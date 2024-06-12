#include "stdafx.h"
#include "ViewBase.h"
#include "Controller.h"

// ------------------------------------------------------------------------------------------------
CViewBase::CViewBase()
	: m_pController(nullptr)
{
}

// ------------------------------------------------------------------------------------------------
CViewBase::~CViewBase()
{
}

// ------------------------------------------------------------------------------------------------
void CViewBase::SetController(CController* pController)
{
	ASSERT(pController != nullptr);

	m_pController = pController;

	OnControllerChanged();
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CViewBase::OnModelChanged()
{}

/*virtual*/ void CViewBase::OnModelUpdated()
{}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CViewBase::OnWorldDimensionsChanged()
{}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CViewBase::OnShowMetaInformation()
{}

/*virtual*/ void CViewBase::OnTargetInstanceChanged(CViewBase* pSender)
{}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CViewBase::OnInstanceSelected(CViewBase* /*pSender*/)
{}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CViewBase::OnInstancesEnabledStateChanged(CViewBase* /*pSender*/)
{}

/*virtual*/ void CViewBase::OnInstanceAttributeEdited(CViewBase* pSender, SdaiInstance iInstance, SdaiAttr pAttribute)
{}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CViewBase::OnViewRelations(CViewBase* /*pSender*/, SdaiInstance /*iInstance*/)
{}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CViewBase::OnViewRelations(CViewBase* /*pSender*/, CEntity* /*pEntity*/)
{}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CViewBase::OnApplicationPropertyChanged(CViewBase* /*pSender*/, enumApplicationProperty /*enApplicationProperty*/)
{}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CViewBase::OnControllerChanged()
{}

// ------------------------------------------------------------------------------------------------
CController* CViewBase::GetController() const
{
	return m_pController;
}
