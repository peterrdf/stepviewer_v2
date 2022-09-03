#include "stdafx.h"
#include "STEPView.h"
#include "STEPController.h"

// ------------------------------------------------------------------------------------------------
CSTEPView::CSTEPView()
	: m_pController(NULL)
{
}

// ------------------------------------------------------------------------------------------------
CSTEPView::~CSTEPView()
{
}

// ------------------------------------------------------------------------------------------------
void CSTEPView::SetController(CSTEPController* pController)
{
	ASSERT(pController != NULL);

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
/*virtual*/ void CSTEPView::OnInstanceEnabledStateChanged(CSTEPView* /*pSender*/, CProductInstance* /*pProductInstance*/)
{
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CSTEPView::OnDisableAllButThis(CSTEPView* /*pSender*/, CProductInstance* /*pProductInstance*/)
{
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CSTEPView::OnEnableAllInstances(CSTEPView* /*pSender*/)
{
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CSTEPView::OnVisibleValuesCountLimitChanged()
{
}

// ------------------------------------------------------------------------------------------------
CSTEPController* CSTEPView::GetController() const
{
	return m_pController;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CSTEPView::OnControllerChanged()
{
}

