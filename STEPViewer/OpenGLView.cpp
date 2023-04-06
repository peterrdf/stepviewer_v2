#include "stdafx.h"
#include "OpenGLView.h"


// ------------------------------------------------------------------------------------------------
COpenGLView::COpenGLView()
	: CViewBase()
	, m_bShowFaces(TRUE)
	, m_bShowConceptualFacesPolygons(TRUE)
	, m_bShowLines(TRUE)
	, m_fLineWidth(1.f)
	, m_bShowPoints(TRUE)
	, m_fPointSize(1.f)
{
}

/*virtual*/ COpenGLView::~COpenGLView()
{
}
