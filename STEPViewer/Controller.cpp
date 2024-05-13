#include "stdafx.h"
#include "Controller.h"
#include "Model.h"
#include "ViewBase.h"

// ------------------------------------------------------------------------------------------------
CController::CController()
	: _controller()
	, m_pModel(nullptr)
	, m_bUpdatingModel(false)
	, m_setViews()
	, m_pSelectedInstance(nullptr)
	, m_bScaleAndCenter(FALSE)
{
	wchar_t szAppPath[_MAX_PATH];
	::GetModuleFileName(::GetModuleHandle(nullptr), szAppPath, sizeof(szAppPath));

	fs::path pthExe = szAppPath;
	auto pthRootFolder = pthExe.parent_path();

	wstring strSettingsFile = pthRootFolder.wstring();
	strSettingsFile += L"\\STEPViewer.settings";

	m_pSettingsStorage->loadSettings(strSettingsFile);
}

// ------------------------------------------------------------------------------------------------
CController::~CController()
{}

// ------------------------------------------------------------------------------------------------
CModel* CController::GetModel() const
{
	return m_pModel;
}

// ------------------------------------------------------------------------------------------------
void CController::SetModel(CModel* pModel)
{
	ASSERT(pModel != nullptr);

	m_pModel = pModel;

	m_pSelectedInstance = nullptr;

	m_bUpdatingModel = true;

	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->OnModelChanged();
	}

	m_bUpdatingModel = false;
}

// ------------------------------------------------------------------------------------------------
void CController::RegisterView(CViewBase* pView)
{
	ASSERT(pView != nullptr);
	ASSERT(m_setViews.find(pView) == m_setViews.end());

	m_setViews.insert(pView);
}

// ------------------------------------------------------------------------------------------------
void CController::UnRegisterView(CViewBase* pView)
{
	ASSERT(pView != nullptr);
	ASSERT(m_setViews.find(pView) != m_setViews.end());

	m_setViews.erase(pView);
}

// ------------------------------------------------------------------------------------------------
const set<CViewBase*> & CController::GetViews()
{
	return m_setViews;
}

// ------------------------------------------------------------------------------------------------
void CController::ZoomToInstance()
{
	ASSERT(m_pModel != nullptr);
	ASSERT(m_pSelectedInstance != nullptr);

	m_pModel->ZoomToInstance(m_pSelectedInstance);

	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->OnWorldDimensionsChanged();
	}
}

// ------------------------------------------------------------------------------------------------
void CController::ZoomOut()
{
	ASSERT(m_pModel != nullptr);

	m_pModel->ZoomOut();

	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->OnWorldDimensionsChanged();
	}
}

// ------------------------------------------------------------------------------------------------
void CController::SaveInstance()
{
	ASSERT(m_pModel != nullptr);
	ASSERT(m_pSelectedInstance != nullptr);

	TCHAR szFilters[] = _T("BIN Files (*.bin)|*.bin|All Files (*.*)|*.*||");
	CFileDialog dlgFile(FALSE, _T("bin"), m_pSelectedInstance->GetName().c_str(),
		OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, szFilters);

	if (dlgFile.DoModal() != IDOK)
	{
		return;
	}

	auto pProductInstance = dynamic_cast<CProductInstance*>(m_pSelectedInstance);
	if (pProductInstance != nullptr)
	{
		SdaiModel iSdaiModel = sdaiGetInstanceModel(m_pSelectedInstance->GetInstance());
		ASSERT(iSdaiModel != 0);

		OwlModel iOwlModel = 0;
		owlGetModel(iSdaiModel, &iOwlModel);
		ASSERT(iOwlModel != 0);
		
		OwlInstance	iMatrixInstance = CreateInstance(GetClassByName(iOwlModel, "Matrix"));
		ASSERT(iMatrixInstance != 0);

		vector<double> vecMatrix
		{
			pProductInstance->GetTransformationMatrix()->_11,
			pProductInstance->GetTransformationMatrix()->_12,
			pProductInstance->GetTransformationMatrix()->_13,
			pProductInstance->GetTransformationMatrix()->_21,
			pProductInstance->GetTransformationMatrix()->_22,
			pProductInstance->GetTransformationMatrix()->_23,
			pProductInstance->GetTransformationMatrix()->_31,
			pProductInstance->GetTransformationMatrix()->_32,
			pProductInstance->GetTransformationMatrix()->_33,
			pProductInstance->GetTransformationMatrix()->_41,
			pProductInstance->GetTransformationMatrix()->_42,
			pProductInstance->GetTransformationMatrix()->_43,
		};

		SetDatatypeProperty(
			iMatrixInstance, 
			GetPropertyByName(iOwlModel, "coordinates"), 
			vecMatrix.data(),
			vecMatrix.size());

		OwlInstance iTransformationInstance = CreateInstance(GetClassByName(iOwlModel, "Transformation"));
		ASSERT(iTransformationInstance != 0);

		SetObjectProperty(
			iTransformationInstance,
			GetPropertyByName(iOwlModel, "object"),
			m_pSelectedInstance->GetInstance());

		SetObjectProperty(
			iTransformationInstance, 
			GetPropertyByName(iOwlModel, "matrix"), 
			iMatrixInstance);

		SaveInstanceTreeW(iTransformationInstance, dlgFile.GetPathName());
	}
	else
	{
		SaveInstanceTreeW(m_pSelectedInstance->GetInstance(), dlgFile.GetPathName());
	}
}

// ------------------------------------------------------------------------------------------------
void CController::ScaleAndCenter()
{
	ASSERT(FALSE); // OBSOLETE
	/*m_pModel->ScaleAndCenter();

	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->OnWorldDimensionsChanged();
	}*/
}

// ------------------------------------------------------------------------------------------------
void CController::ShowMetaInformation(CInstanceBase* /*pInstance*/)
{
	ASSERT(FALSE); // OBSOLETE
}

// ------------------------------------------------------------------------------------------------
void CController::SelectInstance(CViewBase* pSender, CInstanceBase* pInstance)
{
	if (m_bUpdatingModel)
	{
		return;
	}

	m_pSelectedInstance = pInstance;

	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->OnInstanceSelected(pSender);
	}
}

// ------------------------------------------------------------------------------------------------
CInstanceBase* CController::GetSelectedInstance() const
{
	return m_pSelectedInstance;
}

// ------------------------------------------------------------------------------------------------
BOOL CController::GetScaleAndCenter() const
{
	return m_bScaleAndCenter;
}

// ------------------------------------------------------------------------------------------------
void CController::SetScaleAndCenter(BOOL bScaleAndCenter)
{
	m_bScaleAndCenter = bScaleAndCenter;
}

// ------------------------------------------------------------------------------------------------
void CController::OnInstancesEnabledStateChanged(CViewBase* pSender)
{
	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->OnInstancesEnabledStateChanged(pSender);
	}
}

// ------------------------------------------------------------------------------------------------
void CController::OnApplicationPropertyChanged(CViewBase* pSender, enumApplicationProperty enApplicationProperty)
{
	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->OnApplicationPropertyChanged(pSender, enApplicationProperty);
	}
}

// ------------------------------------------------------------------------------------------------
void CController::OnViewRelations(CViewBase* pSender, SdaiInstance iInstance)
{
	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->OnViewRelations(pSender, iInstance);
	}
}

// ------------------------------------------------------------------------------------------------
void CController::OnViewRelations(CViewBase* pSender, CEntity* pEntity)
{
	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->OnViewRelations(pSender, pEntity);
	}
}
