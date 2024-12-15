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

/*virtual*/ void CCIS2OpenGLView::onWorldDimensionsChanged() /*override*/
{
	auto pModel = getModel();
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	/*
	* Center
	*/
	float fXmin = -1.f;
	float fXmax = 1.f;
	float fYmin = -1.f;
	float fYmax = 1.f;
	float fZmin = -1.f;
	float fZmax = 1.f;
	pModel->getWorldDimensions(fXmin, fXmax, fYmin, fYmax, fZmin, fZmax);

	m_fXTranslation = fXmin;
	m_fXTranslation += (fXmax - fXmin) / 2.f;
	m_fXTranslation = -m_fXTranslation;

	m_fYTranslation = fYmin;
	m_fYTranslation += (fYmax - fYmin) / 2.f;
	m_fYTranslation = -m_fYTranslation;

	m_fZTranslation = fZmin;
	m_fZTranslation += (fZmax - fZmin) / 2.f;
	m_fZTranslation = -m_fZTranslation;
	m_fZTranslation -= (pModel->getBoundingSphereDiameter() * 2.f);

	m_fScaleFactor = pModel->getBoundingSphereDiameter();

	_redraw();
}

/*virtual*/ void CCIS2OpenGLView::onInstanceSelected(_view* pSender) /*override*/
{
	if (pSender == this)
	{
		return;
	}

	if (getController() == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pSelectedInstance = getController()->getSelectedInstance() != nullptr ?
		dynamic_cast<CCIS2Instance*>(getController()->getSelectedInstance()) :
		nullptr;

	if (m_pSelectedInstance != pSelectedInstance)
	{
		m_pSelectedInstance = pSelectedInstance;

		_redraw();
	}
}

/*virtual*/ void CCIS2OpenGLView::onInstancesEnabledStateChanged(_view* pSender) /*override*/
{
	if (pSender == this)
	{
		return;
	}

	_redraw();
}

/*virtual*/ void CCIS2OpenGLView::onApplicationPropertyChanged(_view* pSender, enumApplicationProperty enApplicationProperty) /*override*/
{
	if (pSender == this)
	{
		return;
	}

	switch (enApplicationProperty)
	{
	case enumApplicationProperty::Projection:
	case enumApplicationProperty::View:
	case enumApplicationProperty::ShowFaces:
	case enumApplicationProperty::CullFaces:
	case enumApplicationProperty::ShowConceptualFacesWireframes:
	case enumApplicationProperty::ShowLines:
	case enumApplicationProperty::ShowPoints:
	case enumApplicationProperty::RotationMode:
	case enumApplicationProperty::PointLightingLocation:
	case enumApplicationProperty::AmbientLightWeighting:
	case enumApplicationProperty::SpecularLightWeighting:
	case enumApplicationProperty::DiffuseLightWeighting:
	case enumApplicationProperty::MaterialShininess:
	case enumApplicationProperty::Contrast:
	case enumApplicationProperty::Brightness:
	case enumApplicationProperty::Gamma:
	{
		_redraw();
	}
	break;

	default:
	{
		ASSERT(FALSE); // Internal error!
	}
	break;
	} // switch (enApplicationProperty)
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

