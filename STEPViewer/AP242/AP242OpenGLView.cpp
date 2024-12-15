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

/*virtual*/ void CAP242OpenGLView::onWorldDimensionsChanged()  /*override*/
{
	auto pModel = getModel();
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

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

/*virtual*/ void CAP242OpenGLView::onInstanceSelected(_view* pSender)  /*override*/
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
		dynamic_cast<CAP242ProductInstance*>(getController()->getSelectedInstance()) :
		nullptr;

	if (m_pSelectedInstance != pSelectedInstance)
	{
		m_pSelectedInstance = pSelectedInstance;

		_redraw();
	}
}

/*virtual*/ void CAP242OpenGLView::onInstancesEnabledStateChanged(_view* pSender)  /*override*/
{
	if (pSender == this)
	{
		return;
	}

	/*
	* Restore the selection
	*/
	onInstanceSelected(nullptr);

	_redraw();
}

/*virtual*/ void CAP242OpenGLView::onApplicationPropertyChanged(_view* pSender, enumApplicationProperty enApplicationProperty) /*override*/
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

/*virtual*/ void CAP242OpenGLView::onControllerChanged() /*override*/
{
	ASSERT(getController() != nullptr);

	getController()->registerView(this);

	loadSettings();
}



