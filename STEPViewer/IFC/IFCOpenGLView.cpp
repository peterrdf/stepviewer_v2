#include "stdafx.h"

#include "IFCOpenGLView.h"
#include "resource.h"

// ************************************************************************************************
CIFCOpenGLView::CIFCOpenGLView(CWnd* pWnd)
	: _oglView()
{
	_initialize(
		pWnd,
		16,
#ifdef _BLINN_PHONG_SHADERS
		IDR_TEXTFILE_VERTEX_SHADER2,
		IDR_TEXTFILE_FRAGMENT_SHADER2,
#else
		IDR_TEXTFILE_VERTEX_SHADER3,
		IDR_TEXTFILE_FRAGMENT_SHADER3,
#endif
		TEXTFILE,
		false);

	m_bMultiSelect = true;
}

CIFCOpenGLView::~CIFCOpenGLView()
{
}




