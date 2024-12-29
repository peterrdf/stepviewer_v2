#pragma once

#include "_mvc.h"

#include "_ifc_mvc.h"
#include "AP242Model.h"
#include "CIS2Model.h"

// ************************************************************************************************
class CModelFactory
{

public: // Methods

	static _ap_model* Load(_controller* pController, const wchar_t* szPath)
	{
		auto sdaiModel = sdaiOpenModelBNUnicode(0, szPath, L"");
		if (sdaiModel == 0)
		{
			MessageBox(::AfxGetMainWnd()->GetSafeHwnd(), L"Failed to open the model.", L"Error", MB_ICONERROR | MB_OK);

			return nullptr;
		}

		wchar_t* fileSchema = 0;
		GetSPFFHeaderItem(sdaiModel, 9, 0, sdaiUNICODE, (char**)&fileSchema);

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
			auto pModel = new CAP242Model();
			if (!pController->getModels().empty())
			{
				pModel->setWorld(pController->getModels()[0]);
			}			
			pModel->attachModel(szPath, sdaiModel);

			return pModel;
		}

		/*
		* IFC
		*/
		if (strFileSchema.Find(L"IFC") == 0)
		{
			auto pModel = new _ifc_model();
			if (!pController->getModels().empty())
			{
				pModel->setWorld(pController->getModels()[0]);
			}
			pModel->attachModel(szPath, sdaiModel);

			return pModel;
		}

		/*
		* CIS2
		*/
		if (strFileSchema.Find(L"STRUCTURAL_FRAME_SCHEMA") == 0)
		{
			auto pModel = new CCIS2Model();
			if (!pController->getModels().empty())
			{
				pModel->setWorld(pController->getModels()[0]);
			}
			pModel->attachModel(szPath, sdaiModel);

			return pModel;
		}

		return nullptr;
	}
};

