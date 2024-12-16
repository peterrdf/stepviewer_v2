#include "stdafx.h"

#include "CIS2OpenGLView.h"
#include "Controller.h"
#include "CIS2Model.h"
#include "CIS2Instance.h"
#include "resource.h"

// ************************************************************************************************
CCIS2OpenGLView::CCIS2OpenGLView(CWnd* pWnd)
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

CCIS2OpenGLView::~CCIS2OpenGLView()
{
	getController()->unRegisterView(this);	

	_destroy();
}

/*virtual*/ _controller* CCIS2OpenGLView::getController() const /*override*/
{
	return _view::getController();
}

/*virtual*/ void CCIS2OpenGLView::onControllerChanged() /*override*/
{
	auto pController = getController();
	if (pController != nullptr)
	{
		pController->registerView(this);

#ifdef _BLINN_PHONG_SHADERS
		m_pOGLProgram->_setAmbientLightWeighting(
			0.4f,
			0.4f,
			0.4f);
#endif
		pController->onApplicationPropertyChanged(this, enumApplicationProperty::AmbientLightWeighting);

		loadSettings();
	}
}

