#include "stdafx.h"

#include "IFCOpenGLView.h"
#include "Controller.h"
#include "IFCModel.h"
#include "resource.h"

#include <chrono>

// ************************************************************************************************
CIFCOpenGLView::CIFCOpenGLView(CWnd* pWnd)
	: COpenGLView()
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

CIFCOpenGLView::~CIFCOpenGLView()
{
	GetController()->UnRegisterView(this);	

	_destroy();
}

/*virtual*/ _controller* CIFCOpenGLView::getController() const /*override*/
{
	return GetController();
}

/*virtual*/ void CIFCOpenGLView::OnWorldDimensionsChanged() /*override*/
{
	auto pModel = GetModel<CIFCModel>();
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

/*virtual*/ void CIFCOpenGLView::OnInstanceSelected(CViewBase* pSender) /*override*/
{
	if (pSender == this)
	{
		return;
	}

	if (GetController() == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pSelectedInstance = GetController()->GetSelectedInstance() != nullptr ?
		dynamic_cast<CIFCInstance*>(GetController()->GetSelectedInstance()) :
		nullptr;

	if (m_pSelectedInstance != pSelectedInstance)
	{
		m_pSelectedInstance = pSelectedInstance;

		_redraw();
	}
}

/*virtual*/ void CIFCOpenGLView::OnInstancesEnabledStateChanged(CViewBase* pSender) /*override*/
{
	if (pSender == this)
	{
		return;
	}

	_redraw();
}

/*virtual*/ void CIFCOpenGLView::OnApplicationPropertyChanged(CViewBase* pSender, enumApplicationProperty enApplicationProperty) /*override*/
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

/*virtual*/ void CIFCOpenGLView::OnControllerChanged() /*override*/
{
	auto pController = GetController();
	if (pController != nullptr)
	{
		pController->RegisterView(this);

#ifdef _BLINN_PHONG_SHADERS
		m_pOGLProgram->_setAmbientLightWeighting(
			0.4f,
			0.4f,
			0.4f);
#endif
		pController->OnApplicationPropertyChanged(this, enumApplicationProperty::AmbientLightWeighting);

		loadSettings();
	}
}

/*virtual*/ void CIFCOpenGLView::OnMouseEvent(enumMouseEvent enEvent, UINT nFlags, CPoint point) /*override*/
{
	if (enEvent == enumMouseEvent::LBtnUp)
	{
		/*
		* OnSelectedItemChanged() notification
		*/
		if (point == m_ptStartMousePosition)
		{
			if (m_pSelectedInstance != m_pPointedInstance)
			{
				m_pSelectedInstance = m_pPointedInstance;

				_redraw();

				ASSERT(GetController() != nullptr);
				GetController()->SelectInstance(this, m_pSelectedInstance);
			} // if (m_pSelectedInstance != ...
		}
	} // if (enEvent == meLBtnDown)

	switch (enEvent)
	{
		case enumMouseEvent::Move:
		{
			_onMouseMoveEvent(nFlags, point);
		}
		break;

		case enumMouseEvent::LBtnDown:
		case enumMouseEvent::MBtnDown:
		case enumMouseEvent::RBtnDown:
		{
			m_ptStartMousePosition = point;
			m_ptPrevMousePosition = point;
		}
		break;

		case enumMouseEvent::LBtnUp:
		case enumMouseEvent::MBtnUp:
		case enumMouseEvent::RBtnUp:
		{
			m_ptStartMousePosition.x = -1;
			m_ptStartMousePosition.y = -1;
			m_ptPrevMousePosition.x = -1;
			m_ptPrevMousePosition.y = -1;
		}
		break;

		default:
			ASSERT(FALSE);
			break;
	} // switch (enEvent)
}

