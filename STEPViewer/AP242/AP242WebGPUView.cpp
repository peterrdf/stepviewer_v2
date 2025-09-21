#include "stdafx.h"

#include "AP242WebGPUView.h"
#include "Resource.h"

// ************************************************************************************************
CAP242WebGPUView::CAP242WebGPUView(CWnd* pWnd)
	: _webgpuView()
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

	// Default settings
	m_strCullFaces = CULL_FACES_FRONT;
}

CAP242WebGPUView::~CAP242WebGPUView()
{
}

/*virtual*/ void CAP242WebGPUView::_reset() /*override*/
{
	_webgpuRenderer::_reset();

	m_strCullFaces = CULL_FACES_FRONT;
}