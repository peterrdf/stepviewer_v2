#include "stdafx.h"
#include "STEPController.h"
#include "STEPModelBase.h"
#include "STEPView.h"

// ------------------------------------------------------------------------------------------------
CSTEPController::CSTEPController()
	: m_pModel(nullptr)
	, m_bUpdatingModel(false)
	, m_setViews()
	, m_pSelectedInstance(nullptr)
	, m_iVisibleValuesCountLimit(100)
	, m_bScaleAndCenter(FALSE)
{
}

// ------------------------------------------------------------------------------------------------
CSTEPController::~CSTEPController()
{
}

// ------------------------------------------------------------------------------------------------
CSTEPModelBase* CSTEPController::GetModel() const
{
	return m_pModel;
}

// ------------------------------------------------------------------------------------------------
void CSTEPController::SetModel(CSTEPModelBase* pModel)
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
void CSTEPController::RegisterView(CSTEPView* pView)
{
	ASSERT(pView != nullptr);
	ASSERT(m_setViews.find(pView) == m_setViews.end());

	m_setViews.insert(pView);
}

// ------------------------------------------------------------------------------------------------
void CSTEPController::UnRegisterView(CSTEPView* pView)
{
	ASSERT(pView != nullptr);
	ASSERT(m_setViews.find(pView) != m_setViews.end());

	m_setViews.erase(pView);
}

// ------------------------------------------------------------------------------------------------
const set<CSTEPView*> & CSTEPController::GetViews()
{
	return m_setViews;
}

// ------------------------------------------------------------------------------------------------
void CSTEPController::ZoomToInstance()
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
void CSTEPController::ZoomOut()
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
void CSTEPController::SaveInstance()
{
	ASSERT(m_pModel != nullptr);
	ASSERT(m_pSelectedInstance != nullptr);

	TCHAR szFilters[] = _T("BIN Files (*.bin)|*.bin|All Files (*.*)|*.*||");
	CFileDialog dlgFile(FALSE, _T("bin"), m_pSelectedInstance->_getName().c_str(),
		OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, szFilters);

	if (dlgFile.DoModal() != IDOK)
	{
		return;
	}

	SaveInstanceTreeW(m_pSelectedInstance->_getInstance(), dlgFile.GetPathName());
}

// ------------------------------------------------------------------------------------------------
void CSTEPController::ScaleAndCenter()
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
void CSTEPController::ShowMetaInformation(CProductInstance* /*pInstance*/)
{
	ASSERT(FALSE); // OBSOLETE
}

// ------------------------------------------------------------------------------------------------
void CSTEPController::SelectInstance(CSTEPView* pSender, CSTEPInstance* pInstance)
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
CSTEPInstance* CSTEPController::GetSelectedInstance() const
{
	return m_pSelectedInstance;
}

// ------------------------------------------------------------------------------------------------
int CSTEPController::GetVisibleValuesCountLimit() const
{
	return m_iVisibleValuesCountLimit;
}

// ------------------------------------------------------------------------------------------------
void CSTEPController::SetVisibleValuesCountLimit(int iVisibleValuesCountLimit)
{
	m_iVisibleValuesCountLimit = iVisibleValuesCountLimit;
}

// ------------------------------------------------------------------------------------------------
BOOL CSTEPController::GetScaleAndCenter() const
{
	return m_bScaleAndCenter;
}

// ------------------------------------------------------------------------------------------------
void CSTEPController::SetScaleAndCenter(BOOL bScaleAndCenter)
{
	m_bScaleAndCenter = bScaleAndCenter;
}

// ------------------------------------------------------------------------------------------------
void CSTEPController::OnInstancesEnabledStateChanged(CSTEPView* pSender)
{
	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->OnInstancesEnabledStateChanged(pSender);
	}
}
