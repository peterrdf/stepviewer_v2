#include "stdafx.h"
#include "STEPController.h"
#include "STEPModelBase.h"
#include "STEPView.h"

// ------------------------------------------------------------------------------------------------
CSTEPController::CSTEPController()
	: m_pModel(NULL)
	, m_bUpdatingModel(false)
	, m_setViews()
	, m_pSelectedInstance(NULL)
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
	ASSERT(pModel != NULL);

	m_pModel = pModel;

	m_pSelectedInstance = NULL;

	m_bUpdatingModel = true;

	set<CSTEPView*>::iterator itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->OnModelChanged();
	}

	m_bUpdatingModel = false;
}

// ------------------------------------------------------------------------------------------------
void CSTEPController::RegisterView(CSTEPView* pView)
{
	ASSERT(pView != NULL);
	ASSERT(m_setViews.find(pView) == m_setViews.end());

	m_setViews.insert(pView);
}

// ------------------------------------------------------------------------------------------------
void CSTEPController::UnRegisterView(CSTEPView* pView)
{
	ASSERT(pView != NULL);
	ASSERT(m_setViews.find(pView) != m_setViews.end());

	m_setViews.erase(pView);
}

// ------------------------------------------------------------------------------------------------
const set<CSTEPView*> & CSTEPController::GetViews()
{
	return m_setViews;
}

// ------------------------------------------------------------------------------------------------
void CSTEPController::ZoomToInstance(int64_t iInstanceID)
{
	ASSERT(m_pModel != NULL);

	m_pModel->ZoomToInstance(iInstanceID);

	set<CSTEPView*>::iterator itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->OnWorldDimensionsChanged();
	}
}

// ------------------------------------------------------------------------------------------------
void CSTEPController::ScaleAndCenter()
{
	ASSERT(FALSE); // OBSOLETE
	/*m_pModel->ScaleAndCenter();

	set<CSTEPView*>::iterator itView = m_setViews.begin();
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

	set<CSTEPView*>::iterator itView = m_setViews.begin();
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

	set<CSTEPView*>::iterator itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->OnVisibleValuesCountLimitChanged();
	}
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
	set<CSTEPView*>::iterator itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->OnInstancesEnabledStateChanged(pSender);
	}
}

// ------------------------------------------------------------------------------------------------
void CSTEPController::OnInstanceEnabledStateChanged(CSTEPView* pSender, CProductInstance* pProductInstance)
{
	set<CSTEPView*>::iterator itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->OnInstanceEnabledStateChanged(pSender, pProductInstance);
	}
}

// ------------------------------------------------------------------------------------------------
void CSTEPController::OnDisableAllButThis(CSTEPView* pSender, CProductInstance* pProductInstance)
{
	set<CSTEPView*>::iterator itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->OnDisableAllButThis(pSender, pProductInstance);
	}
}

// ------------------------------------------------------------------------------------------------
void CSTEPController::OnEnableAllInstances(CSTEPView* pSender)
{
	set<CSTEPView*>::iterator itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->OnEnableAllInstances(pSender);
	}
}
