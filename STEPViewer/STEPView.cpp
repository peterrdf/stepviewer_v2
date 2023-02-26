#include "stdafx.h"
#include "STEPView.h"
#include "STEPController.h"

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
void CSTEPView::SetController(CSTEPController* pController)
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
/*virtual*/ void CSTEPView::OnInstancePropertySelected()
{
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CSTEPView::OnInstancesEnabledStateChanged(CSTEPView* /*pSender*/)
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
CSTEPController* CSTEPView::GetController() const
{
	return m_pController;
}
