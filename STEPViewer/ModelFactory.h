#pragma once

#include "IFCModel.h"
#include "STEPModel.h"


// ------------------------------------------------------------------------------------------------
class CModelFactory
{

public: // Methods

	// --------------------------------------------------------------------------------------------
	static CModel* Load(const wchar_t* szPath)
	{
		auto iModel = sdaiOpenModelBNUnicode(0, szPath, L"");
		if (iModel == 0)
		{
			MessageBox(::AfxGetMainWnd()->GetSafeHwnd(), L"Failed to open the model.", L"Error", MB_ICONERROR | MB_OK);

			return nullptr;
		}

		wchar_t* fileSchema = 0;
		GetSPFFHeaderItem(iModel, 9, 0, sdaiUNICODE, (char**)&fileSchema);

		if (fileSchema == nullptr)
		{
			MessageBox(::AfxGetMainWnd()->GetSafeHwnd(), L"Unknown file schema.", L"Error", MB_ICONERROR | MB_OK);

			return nullptr;
		}

		CString strFileSchema = fileSchema;
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
			auto pModel = new CSTEPModel();
			pModel->Load(szPath, iModel);

			return pModel;
		}

		/*
		* IFC
		*/
		if (strFileSchema.Find(L"IFC") == 0)
		{
			auto pModel = new CIFCModel();
			pModel->Load(szPath, iModel);

			return pModel;
		}

		/*
		* CIS2
		*/
		if (strFileSchema.Find(L"STRUCTURAL_FRAME_SCHEMA") == 0)
		{
			ASSERT(FALSE); // TODO
		}

		return nullptr;
	}
};

