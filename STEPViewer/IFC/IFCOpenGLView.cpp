#include "stdafx.h"

#include "IFCOpenGLView.h"
#include "Controller.h"
#include "IFCModel.h"
#include "resource.h"

#include <chrono>

// ************************************************************************************************
static const int MIN_VIEW_PORT_LENGTH = 100;

// ************************************************************************************************
CIFCOpenGLView::CIFCOpenGLView(CWnd* pWnd)
	: COpenGLView()
	, m_ptStartMousePosition(-1, -1)
	, m_ptPrevMousePosition(-1, -1)
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

/*virtual*/ _model* CIFCOpenGLView::getModel() const /*override*/
{
	return GetController()->getModel();
}

/*virtual*/ void CIFCOpenGLView::saveSetting(const string& strName, const string& strValue) /*override*/
{
	GetController()->getSettingsStorage()->setSetting(strName, strValue);
}

/*virtual*/ string CIFCOpenGLView::loadSetting(const string& strName) /*override*/
{
	return GetController()->getSettingsStorage()->getSetting(strName);
}

/*virtual*/ bool CIFCOpenGLView::_preDraw(_model* pModel) /*override*/
{
	if (pModel == nullptr)
	{
		return false;
	}

	CRect rcClient;
	m_pWnd->GetClientRect(&rcClient);

	int iWidth = rcClient.Width();
	int iHeight = rcClient.Height();

	if ((iWidth < MIN_VIEW_PORT_LENGTH) || (iHeight < MIN_VIEW_PORT_LENGTH))
	{
		return false;
	}

	float fXmin = -1.f;
	float fXmax = 1.f;
	float fYmin = -1.f;
	float fYmax = 1.f;
	float fZmin = -1.f;
	float fZmax = 1.f;
	pModel->getWorldDimensions(fXmin, fXmax, fYmin, fYmax, fZmin, fZmax);

	_prepare(
		0, 0,
		iWidth, iHeight,
		fXmin, fXmax,
		fYmin, fYmax,
		fZmin, fZmax,
		true,
		true);

	return true;
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
			OnMouseMoveEvent(nFlags, point);
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

void CIFCOpenGLView::OnMouseMoveEvent(UINT nFlags, CPoint point)
{
	auto pModel = GetModel<CIFCModel>();
	if (pModel == nullptr)
	{
		return;
	}

	/*
	* Selection
	*/
	if (((nFlags & MK_LBUTTON) != MK_LBUTTON) && 
		((nFlags & MK_MBUTTON) != MK_MBUTTON) && 
		((nFlags & MK_RBUTTON) != MK_RBUTTON) &&
		m_pInstanceSelectionFrameBuffer->isInitialized())
	{
		int iWidth = 0;
		int iHeight = 0;

		BOOL bResult = m_pOGLContext->makeCurrent();
		VERIFY(bResult);

		CRect rcClient;
		m_pWnd->GetClientRect(&rcClient);

		iWidth = rcClient.Width();
		iHeight = rcClient.Height();

		GLubyte arPixels[4];
		memset(arPixels, 0, sizeof(GLubyte) * 4);

		double dX = (double)point.x * ((double)BUFFER_SIZE / (double)iWidth);
		double dY = ((double)iHeight - (double)point.y) * ((double)BUFFER_SIZE / (double)iHeight);

		m_pInstanceSelectionFrameBuffer->bind();

		glReadPixels(
			(GLint)dX,
			(GLint)dY,
			1, 1,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			arPixels);

		m_pInstanceSelectionFrameBuffer->unbind();

		CIFCInstance* pPointedInstance = nullptr;
		if (arPixels[3] != 0)
		{
			/*int64_t iInstanceID = _i64RGBCoder::decode(arPixels[0], arPixels[1], arPixels[2]);
			pPointedInstance = pModel->GetInstanceByID((int_t)iInstanceID);
			ASSERT(pPointedInstance != nullptr);*/
		}

		if (m_pPointedInstance != pPointedInstance)
		{
			m_pPointedInstance = pPointedInstance;

			_redraw();
		}
	} // if (((nFlags & MK_LBUTTON) != MK_LBUTTON) && ...

	if (m_ptPrevMousePosition.x == -1)
	{
		return;
	}

	/*
	* Rotate
	*/
	if ((nFlags & MK_LBUTTON) == MK_LBUTTON)
	{
		_rotateMouseLButton(
			(float)point.y - (float)m_ptPrevMousePosition.y,
			(float)point.x - (float)m_ptPrevMousePosition.x);

		m_ptPrevMousePosition = point;

		return;
	}

	/*
	* Zoom
	*/
	if ((nFlags & MK_MBUTTON) == MK_MBUTTON)
	{
		_zoomMouseMButton(point.y - m_ptPrevMousePosition.y);

		m_ptPrevMousePosition = point;

		return;
	}

	/*
	* Move
	*/
	if ((nFlags & MK_RBUTTON) == MK_RBUTTON)
	{
		CRect rcClient;
		m_pWnd->GetClientRect(&rcClient);

		_panMouseRButton(
			((float)point.x - (float)m_ptPrevMousePosition.x) / rcClient.Width(),
			((float)point.y - (float)m_ptPrevMousePosition.y) / rcClient.Height());

		m_ptPrevMousePosition = point;

		return;
	}
}

