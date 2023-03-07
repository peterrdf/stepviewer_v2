#include "stdafx.h"
#include "Controller.h"
#include "Model.h"
#include "STEPView.h"

// ------------------------------------------------------------------------------------------------
CController::CController()
	: m_pModel(nullptr)
	, m_bUpdatingModel(false)
	, m_setViews()
	, m_pSelectedInstance(nullptr)
	, m_iVisibleValuesCountLimit(100)
	, m_bScaleAndCenter(FALSE)
{
}

// ------------------------------------------------------------------------------------------------
CController::~CController()
{
}

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
void CController::RegisterView(CSTEPView* pView)
{
	ASSERT(pView != nullptr);
	ASSERT(m_setViews.find(pView) == m_setViews.end());

	m_setViews.insert(pView);
}

// ------------------------------------------------------------------------------------------------
void CController::UnRegisterView(CSTEPView* pView)
{
	ASSERT(pView != nullptr);
	ASSERT(m_setViews.find(pView) != m_setViews.end());

	m_setViews.erase(pView);
}

// ------------------------------------------------------------------------------------------------
const set<CSTEPView*> & CController::GetViews()
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

	SaveInstanceTreeW(m_pSelectedInstance->GetInstance(), dlgFile.GetPathName());
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
void CController::ShowMetaInformation(CInstance* /*pInstance*/)
{
	ASSERT(FALSE); // OBSOLETE
}

// ------------------------------------------------------------------------------------------------
void CController::SelectInstance(CSTEPView* pSender, CInstance* pInstance)
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
CInstance* CController::GetSelectedInstance() const
{
	return m_pSelectedInstance;
}

// ------------------------------------------------------------------------------------------------
int CController::GetVisibleValuesCountLimit() const
{
	return m_iVisibleValuesCountLimit;
}

// ------------------------------------------------------------------------------------------------
void CController::SetVisibleValuesCountLimit(int iVisibleValuesCountLimit)
{
	m_iVisibleValuesCountLimit = iVisibleValuesCountLimit;
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
void CController::OnInstancesEnabledStateChanged(CSTEPView* pSender)
{
	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->OnInstancesEnabledStateChanged(pSender);
	}
}

// ------------------------------------------------------------------------------------------------
void CController::OnApplicationPropertyChanged(CSTEPView* pSender, enumApplicationProperty enApplicationProperty)
{
	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->OnApplicationPropertyChanged(pSender, enApplicationProperty);
	}
}

// ------------------------------------------------------------------------------------------------
void CController::OnViewRelations(CSTEPView* pSender, CInstance* pInstance)
{
	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->OnViewRelations(pSender, pInstance);
	}
}

// ------------------------------------------------------------------------------------------------
void CController::OnViewRelations(CSTEPView* pSender, CEntity* pEntity)
{
	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->OnViewRelations(pSender, pEntity);
	}
}
