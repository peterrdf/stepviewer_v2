#pragma once

#include "_mvc.h"
#include "_ifc_model.h"
#include "_ap242_model.h"
#include "CIS2Model.h"

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include "zip.h"
#include "zlib.h"
#pragma comment(lib, "libz-static.lib")
#pragma comment(lib, "libzip-static.lib")

// ************************************************************************************************
const int_t	BLOCK_LENGTH_READ = 20000; // MAX: 65535

// ************************************************************************************************
zip_file* g_pZipFile = nullptr;

// ************************************************************************************************
int_t __stdcall	ReadCallBackFunction(unsigned char* szContent)
{
	if (g_pZipFile == nullptr) 
	{
		return -1;
	}

	return (int_t)zip_fread(g_pZipFile, szContent, BLOCK_LENGTH_READ);
}

// ************************************************************************************************
class CModelFactory
{

public: // Methods

	static _ap_model* Load(_controller* pController, const wchar_t* szModel, bool bMultipleModels)
	{
		fs::path pathModel = szModel;

		/*
		* IFCZIP
		*/		
		if (pathModel.extension().string() == ".ifczip")
		{
			auto sdaiModel = OpenIFCZIPModel(pathModel);
			if (sdaiModel == 0)
			{
				MessageBox(::AfxGetMainWnd()->GetSafeHwnd(), L"Failed to open the model.", L"Error", MB_ICONERROR | MB_OK);

				return nullptr;
			}

			auto pModel = new _ifc_model(bMultipleModels);
			pModel->attachModel(szModel, sdaiModel, bMultipleModels ? pController->getModel() : nullptr);

			return pModel;
		} // IFCZIP

		/*
		* STEPZIP
		*/
		if (pathModel.extension().string() == ".stpz")
		{
			auto sdaiModel = OpenSTEPGZIPModel(pathModel);
			if (sdaiModel == 0)
			{
				MessageBox(::AfxGetMainWnd()->GetSafeHwnd(), L"Failed to open the model.", L"Error", MB_ICONERROR | MB_OK);

				return nullptr;
			}
			
			auto pModel = new _ap242_model();
			pModel->attachModel(szModel, sdaiModel, nullptr);

			return pModel;
		} // STEPZIP

		auto sdaiModel = sdaiOpenModelBNUnicode(0, szModel, L"");
		if (sdaiModel == 0)
		{
			MessageBox(::AfxGetMainWnd()->GetSafeHwnd(), L"Failed to open the model.", L"Error", MB_ICONERROR | MB_OK);

			return nullptr;
		}

		wchar_t* szFileSchema = 0;
		GetSPFFHeaderItem(sdaiModel, 9, 0, sdaiUNICODE, (char**)&szFileSchema);
		if (szFileSchema == nullptr)
		{
			MessageBox(::AfxGetMainWnd()->GetSafeHwnd(), L"Unknown file schema.", L"Error", MB_ICONERROR | MB_OK);

			return nullptr;
		}

		CString strFileSchema = szFileSchema;
		strFileSchema.MakeUpper();

		/*
		* STEP
		*/
		if ((strFileSchema.Find(L"CONFIG_CONTROL_DESIGN") == 0) ||
			(strFileSchema.Find(L"CONFIG_CONTROL_3D_DESIGN") == 0) ||
			(strFileSchema.Find(L"CONFIG_CONTROL_DESIGN_LINE") == 0) ||
			(strFileSchema.Find(L"CONFIGURATION_CONTROL_DESIGN") == 0) ||
			(strFileSchema.Find(L"CONFIGURATION_CONTROL_3D_DESIGN") == 0) ||
			(strFileSchema.Find(L"AUTOMOTIVE_DESIGN") == 0) ||
			(strFileSchema.Find(L"AP203") == 0) || 
			(strFileSchema.Find(L"AP209") == 0) ||
			(strFileSchema.Find(L"AP214") == 0) ||
			(strFileSchema.Find(L"AP242") == 0))
		{
			ASSERT(!bMultipleModels); // Not supported!

			auto pModel = new _ap242_model();
			pModel->attachModel(szModel, sdaiModel, nullptr);

			return pModel;
		}

		/*
		* IFC
		*/
		if (strFileSchema.Find(L"IFC") == 0)
		{
			auto pModel = new _ifc_model(bMultipleModels);
			pModel->attachModel(szModel, sdaiModel, bMultipleModels ? pController->getModel() : nullptr);

			return pModel;
		}

		/*
		* CIS2
		*/
		if (strFileSchema.Find(L"STRUCTURAL_FRAME_SCHEMA") == 0)
		{
			ASSERT(!bMultipleModels); // Not supported!

			auto pModel = new CCIS2Model();
			pModel->attachModel(szModel, sdaiModel, nullptr);

			return pModel;
		}

		return nullptr;
	}

	// ********************************************************************************************
	// zip support
	// ********************************************************************************************
	static SdaiModel OpenIFCZIPModel(const fs::path& pathIfcZip)
	{
		string strIFCFileName = pathIfcZip.stem().string();
		strIFCFileName += ".ifc";

		int iError = 0;
		zip* pZip = zip_open(pathIfcZip.string().c_str(), 0, &iError);
		if (iError != 0)
		{
			return 0;
		}

		struct zip_stat zipStat;
		zip_stat_init(&zipStat);
		zip_stat(pZip, strIFCFileName.c_str(), 0, &zipStat);

		g_pZipFile = zip_fopen(pZip, strIFCFileName.c_str(), 0);
		if (g_pZipFile == nullptr)
		{
			return 0;
		}

		auto sdaiModel = engiOpenModelByStream(0, ReadCallBackFunction, "");

		zip_fclose(g_pZipFile);
		g_pZipFile = nullptr;
		zip_close(pZip);

		return sdaiModel;
	}

	// ********************************************************************************************
	// gzip support
	// https://windrealm.org/tutorials/decompress-gzip-stream.php
	// ********************************************************************************************	
	static bool gzipInflate(const std::string& compressedBytes, std::string& uncompressedBytes) {
		if (compressedBytes.size() == 0) {
			uncompressedBytes = compressedBytes;
			return true;
		}

		uncompressedBytes.clear();

		unsigned full_length = (unsigned)compressedBytes.size();
		unsigned half_length = (unsigned)compressedBytes.size() / 2;

		unsigned uncompLength = full_length;
		char* uncomp = (char*)calloc(sizeof(char), uncompLength);

		z_stream strm;
		strm.next_in = (Bytef*)compressedBytes.c_str();
		strm.avail_in = (unsigned)compressedBytes.size();
		strm.total_out = 0;
		strm.zalloc = Z_NULL;
		strm.zfree = Z_NULL;

		bool done = false;

		if (inflateInit2(&strm, (16 + MAX_WBITS)) != Z_OK) {
			free(uncomp);
			return false;
		}

		while (!done) {
			// If our output buffer is too small  
			if (strm.total_out >= uncompLength) {
				// Increase size of output buffer  
				char* uncomp2 = (char*)calloc(sizeof(char), uncompLength + half_length);
				memcpy(uncomp2, uncomp, uncompLength);
				uncompLength += half_length;
				free(uncomp);
				uncomp = uncomp2;
			}

			strm.next_out = (Bytef*)(uncomp + strm.total_out);
			strm.avail_out = uncompLength - strm.total_out;

			// Inflate another chunk.  
			int err = inflate(&strm, Z_SYNC_FLUSH);
			if (err == Z_STREAM_END) done = true;
			else if (err != Z_OK) {
				break;
			}
		}

		if (inflateEnd(&strm) != Z_OK) {
			free(uncomp);
			return false;
		}

		for (size_t i = 0; i < strm.total_out; ++i) {
			uncompressedBytes += uncomp[i];
		}
		free(uncomp);
		return true;
	}

	/* Reads a file into memory. */
	static bool loadBinaryFile(const std::string& filename, std::string& contents) {
		// Open the gzip file in binary mode  
		FILE* f = fopen(filename.c_str(), "rb");
		if (f == NULL)
			return false;

		// Clear existing bytes in output vector  
		contents.clear();

		// Read all the bytes in the file  
		int c = fgetc(f);
		while (c != EOF) {
			contents += (char)c;
			c = fgetc(f);
		}
		fclose(f);

		return true;
	}

	static SdaiModel OpenSTEPGZIPModel(const fs::path& pathStepZip)
	{
		// Read the gzip file data into memory  
		std::string fileData;
		if (!loadBinaryFile(pathStepZip.string(), fileData)) {
			printf("Error loading input file.");
			return 0;
		}

		std::string data;
		if (!gzipInflate(fileData, data)) {
			printf("Error decompressing file.");
			return 0;
		}

		return engiOpenModelByArray(0, (unsigned char*)data.c_str(), (int_t)data.size(), "");
	}
};

