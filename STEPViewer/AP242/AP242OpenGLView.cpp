#include "stdafx.h"

#include "AP242OpenGLView.h"
#include "Resource.h"

// ************************************************************************************************
CAP242OpenGLView::CAP242OpenGLView(CWnd* pWnd)
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
}

CAP242OpenGLView::~CAP242OpenGLView()
{
}

/*virtual*/ void CAP242OpenGLView::_reset() /*override*/
{
	_oglRenderer::_reset();

	m_strCullFaces = CULL_FACES_FRONT;
}




