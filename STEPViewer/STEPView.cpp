#include "stdafx.h"
#include "STEPView.h"
#include "STEPController.h"

// ------------------------------------------------------------------------------------------------
CSTEPView::CSTEPView()
	: m_pController(nullptr)
	, m_bShowFaces(TRUE)
	, m_bShowFacesPolygons(FALSE)
	, m_bShowConceptualFacesPolygons(TRUE)
	, m_bShowLines(TRUE)
	, m_fLineWidth(1.f)
	, m_bShowPoints(TRUE)
	, m_fPointSize(1.f)
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
/*virtual*/ void CSTEPView::OnInstanceEnabledStateChanged(CSTEPView* /*pSender*/, CProductInstance* /*pInstance*/)
{
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CSTEPView::OnDisableAllButThis(CSTEPView* /*pSender*/, CProductInstance* /*pInstance*/)
{
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CSTEPView::OnEnableAllInstances(CSTEPView* /*pSender*/)
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
