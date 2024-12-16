#include "stdafx.h"

#include "AP242OpenGLView.h"
#include "Controller.h"
#include "AP242Model.h"
#include "_3DUtils.h"
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

	// Default settings
	_oglRendererSettings::m_bShowLines = FALSE;
	_oglRendererSettings::m_bShowPoints = FALSE;
}

CAP242OpenGLView::~CAP242OpenGLView()
{
	getController()->unRegisterView(this);

	_destroy();
}

/*virtual*/ _controller* CAP242OpenGLView::getController() const /*override*/
{
	return _view::getController();
}

/*virtual*/ void CAP242OpenGLView::onControllerChanged() /*override*/
{
	ASSERT(getController() != nullptr);

	getController()->registerView(this);

	loadSettings();
}



