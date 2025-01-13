#pragma once

#include "_mvc.h"
#include "_ifc_model.h"
#include "_ap242_model.h"
#include "CIS2Model.h"

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include "zip.h"
#pragma comment(lib, "libz-static.lib")
#pragma comment(lib, "libzip-static.lib")

const int_t	BLOCK_LENGTH_READ = 20000; // MAX: 65535

FILE* myFileRead = nullptr;


int_t	__stdcall	ReadCallBackFunction(unsigned char* content)
{
	if (myFileRead == nullptr || feof(myFileRead)) {
		return	-1;
	}

	int_t	size = fread(content, 1, BLOCK_LENGTH_READ, myFileRead);

	return	size;
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
			string strIFCFileName = pathModel.stem().string();
			strIFCFileName += ".ifc";

			int iError = 0;
			zip* pZip = zip_open(pathModel.string().c_str(), 0, &iError);
			if (iError == 0)
			{
				struct zip_stat zipStat;
				zip_stat_init(&zipStat);
				zip_stat(pZip, strIFCFileName.c_str(), 0, &zipStat);

				unsigned char* szContent = new unsigned char[zipStat.size];

				zip_file* pZipFile = zip_fopen(pZip, strIFCFileName.c_str(), 0);
				zip_fread(pZipFile, szContent, zipStat.size);
				zip_fclose(pZipFile);
				zip_close(pZip);

				auto sdaiModel = engiOpenModelByArray(0, szContent, (int_t)zipStat.size, (const char*)1);

				delete[] szContent;

				auto pModel = new _ifc_model(bMultipleModels);
				pModel->attachModel(szModel, sdaiModel, bMultipleModels ? pController->getModel() : nullptr);

				return pModel;
			} // if (iError == 0)

			return nullptr;
		} // IFCZIP

		/*
		* STEPZIP
		*/
		if (pathModel.extension().string() == ".stpz")
		{
			string strSTEPFileName = pathModel.stem().string();

			int iError = 0;
			zip* pZip = zip_open(pathModel.string().c_str(), 0, &iError);
			if (iError == 0)
			{
				struct zip_stat zipStat;
				zip_stat_init(&zipStat);
				zip_stat(pZip, strSTEPFileName.c_str(), 0, &zipStat);

				unsigned char* szContent = new unsigned char[zipStat.size];

				zip_file* pZipFile = zip_fopen(pZip, strSTEPFileName.c_str(), 0);
				zip_fread(pZipFile, szContent, zipStat.size);
				zip_fclose(pZipFile);
				zip_close(pZip);

				auto sdaiModel = engiOpenModelByArray(0, szContent, (int_t)zipStat.size, (const char*)1);

				delete[] szContent;

				ASSERT(!bMultipleModels); // Not supported!

				auto pModel = new _ap242_model();
				pModel->attachModel(szModel, sdaiModel, nullptr);

				return pModel;
			} // if (iError == 0)
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
};

