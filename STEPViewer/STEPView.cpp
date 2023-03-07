#include "stdafx.h"
#include "STEPView.h"
#include "Controller.h"

// ------------------------------------------------------------------------------------------------
CSTEPView::CSTEPView()
	: m_pController(nullptr)
{
}

// ------------------------------------------------------------------------------------------------
CSTEPView::~CSTEPView()
{
}

// ------------------------------------------------------------------------------------------------
void CSTEPView::SetController(CController* pController)
{
	ASSERT(pController != nullptr);

	m_pController = pController;

	OnControllerChanged();
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CSTEPView::OnModelChanged()
{
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CSTEPView::OnWorldDimensionsChanged()
{
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CSTEPView::OnShowMetaInformation()
{
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CSTEPView::OnInstanceSelected(CSTEPView* /*pSender*/)
{
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CSTEPView::OnInstancesEnabledStateChanged(CSTEPView* /*pSender*/)
{
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CSTEPView::OnViewRelations(CSTEPView* /*pSender*/, int64_t /*iInstance*/)
{
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CSTEPView::OnViewRelations(CSTEPView* /*pSender*/, CEntity* /*pEntity*/)
{
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CSTEPView::OnApplicationPropertyChanged(CSTEPView* /*pSender*/, enumApplicationProperty /*enApplicationProperty*/)
{
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CSTEPView::OnControllerChanged()
{
}

// ------------------------------------------------------------------------------------------------
CController* CSTEPView::GetController() const
{
	return m_pController;
}
