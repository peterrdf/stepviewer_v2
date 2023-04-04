#include "stdafx.h"

#include "OpenGLSTEPView.h"
#include "Controller.h"
#include "STEPModel.h"

#include "Resource.h"

#include <chrono>

#ifdef _LINUX
#include <cfloat>
#include <GL/gl.h>
#include <GL/glext.h>
#endif // _LINUX


// ------------------------------------------------------------------------------------------------
#ifdef _LINUX
COpenGLSTEPView::COpenGLSTEPView(wxGLCanvas * pWnd)
#else
COpenGLSTEPView::COpenGLSTEPView(CWnd* pWnd)
#endif // _LINUX
	: COpenGLView()	
	, _oglRenderer()
	, m_ptStartMousePosition(-1, -1)
	, m_ptPrevMousePosition(-1, -1)
	, m_pInstanceSelectionFrameBuffer(new _oglSelectionFramebuffer())
	, m_pPointedInstance(nullptr)
	, m_pSelectedInstance(nullptr)
	, m_pSelectedInstanceMaterial(nullptr)
	, m_pPointedInstanceMaterial(nullptr)
	, m_hFont(nullptr)
{
	_initialize(
		pWnd,
		16,
		IDR_TEXTFILE_VERTEX_SHADER2,
		IDR_TEXTFILE_FRAGMENT_SHADER2,
		TEXTFILE,
		false);

	/*
	* Default
	*/
	m_pSelectedInstanceMaterial = new _material();
	m_pSelectedInstanceMaterial->init(
		1.f, 0.f, 0.f,
		1.f, 0.f, 0.f,
		1.f, 0.f, 0.f,
		1.f, 0.f, 0.f,
		1.f,
		nullptr);

	/*
	* Default
	*/
	m_pPointedInstanceMaterial = new _material();
	m_pPointedInstanceMaterial->init(
		.33f, .33f, .33f,
		.33f, .33f, .33f,
		.33f, .33f, .33f,
		.33f, .33f, .33f,
		.66f,
		nullptr);

	/*
	* Font
	*/
	m_hFont = CreateFont(-12,           // Height Of Font
		0,                              // Width Of Font
		0,                              // Angle Of Escapement
		0,                              // Orientation Angle
		FW_EXTRALIGHT,                  // Font Weight
		FALSE,                          // Italic
		FALSE,                          // Underline
		FALSE,                          // Strikeout
		ANSI_CHARSET,                   // Character Set Identifier
		OUT_TT_PRECIS,                  // Output Precision
		CLIP_DEFAULT_PRECIS,            // Clipping Precision
		ANTIALIASED_QUALITY,            // Output Quality
		FF_DONTCARE | VARIABLE_PITCH,   // Family And Pitch
		(LPCWSTR)L"Arial");             // Font Name
	ASSERT(m_hFont != nullptr);
}

// ------------------------------------------------------------------------------------------------
COpenGLSTEPView::~COpenGLSTEPView()
{
	GetController()->UnRegisterView(this);

	delete m_pInstanceSelectionFrameBuffer;

	_destroy();

	delete m_pSelectedInstanceMaterial;
	delete m_pPointedInstanceMaterial;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void COpenGLSTEPView::OnWorldDimensionsChanged()  /*override*/
{
	auto pController = GetController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	if (pController->GetModel() == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pModel = pController->GetModel()->As<CSTEPModel>();
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
	pModel->GetWorldDimensions(fXmin, fXmax, fYmin, fYmax, fZmin, fZmax);

	m_fXTranslation = fXmin;
	m_fXTranslation += (fXmax - fXmin) / 2.f;
	m_fXTranslation = -m_fXTranslation;

	m_fYTranslation = fYmin;
	m_fYTranslation += (fYmax - fYmin) / 2.f;
	m_fYTranslation = -m_fYTranslation;

	m_fZTranslation = fZmin;
	m_fZTranslation += (fZmax - fZmin) / 2.f;
	m_fZTranslation = -m_fZTranslation;

	m_fZTranslation -= (pModel->GetBoundingSphereDiameter() * 2.f);

#ifdef _LINUX
	m_pWnd->Refresh(false);
#else
	_redraw();
#endif // _LINUX
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void COpenGLSTEPView::OnInstanceSelected(CSTEPView* pSender)  /*override*/
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
		dynamic_cast<CProductInstance*>(GetController()->GetSelectedInstance()) :
		nullptr;

	if (m_pSelectedInstance != pSelectedInstance)
	{
		m_pSelectedInstance = pSelectedInstance;

#ifdef _LINUX
		m_pWnd->Refresh(false);
#else
		_redraw();
#endif // _LINUX
	}
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void COpenGLSTEPView::OnInstancesEnabledStateChanged(CSTEPView* pSender)  /*override*/
{
	if (pSender == this)
	{
		return;
	}

	/*
	* Restore the selection
	*/
	OnInstanceSelected(nullptr);

#ifdef _LINUX
	m_pWnd->Refresh(false);
#else
	_redraw();
#endif // _LINUX
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void COpenGLSTEPView::OnApplicationPropertyChanged(CSTEPView* pSender, enumApplicationProperty enApplicationProperty) /*override*/
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
		case enumApplicationProperty::ShowConceptualFacesWireframes:
		case enumApplicationProperty::ShowLines:
		case enumApplicationProperty::ShowPoints:
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

// ------------------------------------------------------------------------------------------------
/*virtual*/ void COpenGLSTEPView::OnControllerChanged() /*override*/
{
	ASSERT(GetController() != nullptr);

	GetController()->RegisterView(this);
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void COpenGLSTEPView::Load() /*override*/
{
#ifdef _LINUX
	m_pOGLContext->SetCurrent(*m_pWnd);
#else
	BOOL bResult = m_pOGLContext->makeCurrent();
	VERIFY(bResult);
#endif // _LINUX

	// OpenGL buffers
	m_oglBuffers.clear();

	m_pInstanceSelectionFrameBuffer->encoding().clear();
	m_pPointedInstance = nullptr;
	m_pSelectedInstance = nullptr;

	CController* pController = GetController();
	ASSERT(pController != nullptr);

	if (pController->GetModel() == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pModel = pController->GetModel()->As<CSTEPModel>();
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
	pModel->GetWorldDimensions(fXmin, fXmax, fYmin, fYmax, fZmin, fZmax);

	/*
	* Bounding sphere diameter
	*/
	float fBoundingSphereDiameter = fXmax - fXmin;
	fBoundingSphereDiameter = fmaxf(fBoundingSphereDiameter, fYmax - fYmin);
	fBoundingSphereDiameter = fmaxf(fBoundingSphereDiameter, fZmax - fZmin);

	m_fXTranslation = fXmin;
	m_fXTranslation += (fXmax - fXmin) / 2.f;
	m_fXTranslation = -m_fXTranslation;

	m_fYTranslation = fYmin;
	m_fYTranslation += (fYmax - fYmin) / 2.f;
	m_fYTranslation = -m_fYTranslation;

	m_fZTranslation = fZmin;
	m_fZTranslation += (fZmax - fZmin) / 2.f;
	m_fZTranslation = -m_fZTranslation;

	m_fZTranslation -= (pModel->GetBoundingSphereDiameter() * 2.f);

	// Limits
	GLsizei VERTICES_MAX_COUNT = _oglUtils::getVerticesCountLimit(GEOMETRY_VBO_VERTEX_LENGTH * sizeof(float));
	GLsizei INDICES_MAX_COUNT = _oglUtils::getIndicesCountLimit();	

	// Data
	auto& mapDefinitions = pModel->GetDefinitions();

	// VBO
	GLuint iVerticesCount = 0;
	vector<CProductDefinition*> vecProductDefinitionsCohort;

	// IBO - Conceptual faces
	GLuint iConcFacesIndicesCount = 0;
	vector<_cohort*> vecConcFacesCohorts;

	// IBO - Conceptual face polygons
	GLuint iConcFacePolygonsIndicesCount = 0;
	vector<_cohort*> vecConcFacePolygonsCohorts;

	// IBO - Lines
	GLuint iLinesIndicesCount = 0;
	vector<_cohort*> vecLinesCohorts;

	// IBO - Points
	GLuint iPointsIndicesCount = 0;
	vector<_cohort*> vecPointsCohorts;

	for (auto itProductDefinitions = mapDefinitions.begin(); itProductDefinitions != mapDefinitions.end(); itProductDefinitions++)
	{
		auto pDefinition = itProductDefinitions->second;
		if (pDefinition->GetVerticesCount() == 0)
		{
			continue;
		}

		/******************************************************************************************
		* Geometry
		*/

		/**
		* VBO - Conceptual faces, polygons, etc.
		*/
		if (((int_t)iVerticesCount + pDefinition->GetVerticesCount()) > (int_t)VERTICES_MAX_COUNT)
		{
			if (m_oglBuffers.createInstancesCohort(vecProductDefinitionsCohort, m_pOGLProgram) != iVerticesCount)
			{
				ASSERT(FALSE);

				return;
			}

			iVerticesCount = 0;
			vecProductDefinitionsCohort.clear();
		}

		/*
		* IBO - Conceptual faces
		*/
		for (size_t iFacesCohort = 0; iFacesCohort < pDefinition->ConcFacesCohorts().size(); iFacesCohort++)
		{
			if ((int_t)(iConcFacesIndicesCount + pDefinition->ConcFacesCohorts()[iFacesCohort]->indices().size()) > (int_t)INDICES_MAX_COUNT)
			{
				if (m_oglBuffers.createIBO(vecConcFacesCohorts) != iConcFacesIndicesCount)
				{
					ASSERT(FALSE);

					return;
				}

				iConcFacesIndicesCount = 0;
				vecConcFacesCohorts.clear();
			}

			iConcFacesIndicesCount += (GLsizei)pDefinition->ConcFacesCohorts()[iFacesCohort]->indices().size();
			vecConcFacesCohorts.push_back(pDefinition->ConcFacesCohorts()[iFacesCohort]);
		}

		/*
		* IBO - Conceptual face polygons
		*/
		for (size_t iConcFacePolygonsCohort = 0; iConcFacePolygonsCohort < pDefinition->ConcFacePolygonsCohorts().size(); iConcFacePolygonsCohort++)
		{
			if ((int_t)(iConcFacePolygonsIndicesCount + pDefinition->ConcFacePolygonsCohorts()[iConcFacePolygonsCohort]->indices().size()) > (int_t)INDICES_MAX_COUNT)
			{
				if (m_oglBuffers.createIBO(vecConcFacePolygonsCohorts) != iConcFacePolygonsIndicesCount)
				{
					ASSERT(FALSE);

					return;
				}

				iConcFacePolygonsIndicesCount = 0;
				vecConcFacePolygonsCohorts.clear();
			}

			iConcFacePolygonsIndicesCount += (GLsizei)pDefinition->ConcFacePolygonsCohorts()[iConcFacePolygonsCohort]->indices().size();
			vecConcFacePolygonsCohorts.push_back(pDefinition->ConcFacePolygonsCohorts()[iConcFacePolygonsCohort]);
		}

		/*
		* IBO - Lines
		*/
		for (size_t iLinesCohort = 0; iLinesCohort < pDefinition->LinesCohorts().size(); iLinesCohort++)
		{
			if ((int_t)(iLinesIndicesCount + pDefinition->LinesCohorts()[iLinesCohort]->indices().size()) > (int_t)INDICES_MAX_COUNT)
			{
				if (m_oglBuffers.createIBO(vecLinesCohorts) != iLinesIndicesCount)
				{
					ASSERT(FALSE);

					return;
				}

				iLinesIndicesCount = 0;
				vecLinesCohorts.clear();
			}

			iLinesIndicesCount += (GLsizei)pDefinition->LinesCohorts()[iLinesCohort]->indices().size();
			vecLinesCohorts.push_back(pDefinition->LinesCohorts()[iLinesCohort]);
		}

		/*
		* IBO - Points
		*/
		for (size_t iPointsCohort = 0; iPointsCohort < pDefinition->PointsCohorts().size(); iPointsCohort++)
		{
			if ((int_t)(iPointsIndicesCount + pDefinition->PointsCohorts()[iPointsCohort]->indices().size()) > (int_t)INDICES_MAX_COUNT)
			{
				if (m_oglBuffers.createIBO(vecPointsCohorts) != iPointsIndicesCount)
				{
					ASSERT(FALSE);

					return;
				}

				iPointsIndicesCount = 0;
				vecPointsCohorts.clear();
			}

			iPointsIndicesCount += (GLsizei)pDefinition->PointsCohorts()[iPointsCohort]->indices().size();
			vecPointsCohorts.push_back(pDefinition->PointsCohorts()[iPointsCohort]);
		}

		iVerticesCount += (GLsizei)pDefinition->GetVerticesCount();
		vecProductDefinitionsCohort.push_back(pDefinition);
	} // for (auto itProductDefinitions = ...

	/******************************************************************************************
	* Geometry
	*/

	/*
	* VBO - Conceptual faces, polygons, etc.
	*/
	if (iVerticesCount > 0)
	{
		if (m_oglBuffers.createInstancesCohort(vecProductDefinitionsCohort, m_pOGLProgram) != iVerticesCount)
		{
			ASSERT(FALSE);

			return;
		}

		iVerticesCount = 0;
		vecProductDefinitionsCohort.clear();
	} // if (iVerticesCount > 0)	

	/*
	* IBO - Conceptual faces
	*/
	if (iConcFacesIndicesCount > 0)
	{
		if (m_oglBuffers.createIBO(vecConcFacesCohorts) != iConcFacesIndicesCount)
		{
			ASSERT(FALSE);

			return;
		}

		iConcFacesIndicesCount = 0;
		vecConcFacesCohorts.clear();
	}

	/*
	* IBO - Conceptual face polygons
	*/
	if (iConcFacePolygonsIndicesCount > 0)
	{
		if (m_oglBuffers.createIBO(vecConcFacePolygonsCohorts) != iConcFacePolygonsIndicesCount)
		{
			ASSERT(FALSE);

			return;
		}

		iConcFacePolygonsIndicesCount = 0;
		vecConcFacePolygonsCohorts.clear();
	}

	/*
	* IBO - Lines
	*/
	if (iLinesIndicesCount > 0)
	{
		if (m_oglBuffers.createIBO(vecLinesCohorts) != iLinesIndicesCount)
		{
			ASSERT(FALSE);

			return;
		}

		iLinesIndicesCount = 0;
		vecLinesCohorts.clear();
	}

	/*
	* IBO - Points
	*/
	if (iPointsIndicesCount > 0)
	{
		if (m_oglBuffers.createIBO(vecPointsCohorts) != iPointsIndicesCount)
		{
			ASSERT(FALSE);

			return;
		}

		iPointsIndicesCount = 0;
		vecPointsCohorts.clear();
	}

#ifdef _LINUX
	m_pWnd->Refresh(false);
#else
	_redraw();
#endif // _LINUX
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void COpenGLSTEPView::SetProjection(enumProjection enProjection) /*override*/
{
	_setProjection(enProjection);
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ enumProjection COpenGLSTEPView::GetProjection() const /*override*/
{
	return _getProjection();
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void COpenGLSTEPView::SetView(enumView enView) /*override*/
{
	_setView(enView);
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void COpenGLSTEPView::OnMouseEvent(enumMouseEvent enEvent, UINT nFlags, CPoint point) /*override*/
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

#ifdef _LINUX
				m_pWnd->Refresh(false);
#else
				_redraw();
#endif // _LINUX

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

// ------------------------------------------------------------------------------------------------
/*virtual*/ void COpenGLSTEPView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) /*override*/
{
	UNREFERENCED_PARAMETER(nFlags);
	UNREFERENCED_PARAMETER(pt);

	_zoom(
		(float)zDelta < 0.f ?
		-abs(m_fZTranslation) * ZOOM_SPEED_MOUSE_WHEEL :
		abs(m_fZTranslation) * ZOOM_SPEED_MOUSE_WHEEL);
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void COpenGLSTEPView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) /*override*/
{
	UNREFERENCED_PARAMETER(nRepCnt);
	UNREFERENCED_PARAMETER(nFlags);

	CRect rcClient;
	m_pWnd->GetClientRect(&rcClient);

	switch (nChar)
	{
		case VK_UP:
		{
			_move(
				0.f,
				PAN_SPEED_KEYS * (1.f / rcClient.Height()));
		}
		break;

		case VK_DOWN:
		{
			_move(
				0.f,
				-(PAN_SPEED_KEYS * (1.f / rcClient.Height())));
		}
		break;

		case VK_LEFT:
		{
			_move(
				-(PAN_SPEED_KEYS * (1.f / rcClient.Width())),
				0.f);
		}
		break;

		case VK_RIGHT:
		{
			_move(
				PAN_SPEED_KEYS * (1.f / rcClient.Width()),
				0.f);
		}
		break;
	} // switch (nChar)
}

// ------------------------------------------------------------------------------------------------
#ifdef _LINUX
void COpenGLSTEPView::Draw(wxPaintDC * pDC)
#else
/*virtual*/ void COpenGLSTEPView::Draw(CDC* pDC) /*override*/
#endif // _LINUX
{
	VERIFY(pDC != nullptr);

	auto pController = GetController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	if (pController->GetModel() == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pModel = pController->GetModel()->As<CSTEPModel>();
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	CRect rcClient;
	m_pWnd->GetClientRect(&rcClient);

	int iWidth = rcClient.Width();
	int iHeight = rcClient.Height();

	if ((iWidth < 20) || (iHeight < 20))
	{
		return;
	}

	float fXmin = -1.f;
	float fXmax = 1.f;
	float fYmin = -1.f;
	float fYmax = 1.f;
	float fZmin = -1.f;
	float fZmax = 1.f;
	pModel->GetWorldDimensions(fXmin, fXmax, fYmin, fYmax, fZmin, fZmax);

	_prepare(
		iWidth, iHeight,
		fXmin, fXmax,
		fYmin, fYmax,
		fZmin, fZmax);

	/*
	* Non-transparent faces
	*/
	DrawFaces(false);

	/*
	* Transparent faces
	*/
	DrawFaces(true);

	/*
	* Conceptual faces polygons
	*/
	DrawConceptualFacesPolygons();

	/*
	* Lines
	*/
	DrawLines();

	/*
	* Points
	*/
	DrawPoints();

	/*
	* End
	*/
#ifdef _LINUX
	m_pWnd->SwapBuffers();
#else
	SwapBuffers(*pDC);
#endif // _LINUX	

	/*
	*Selection support
	*/
	DrawInstancesFrameBuffer();
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::DrawFaces(bool bTransparent)
{
	if (!m_bShowFaces)
	{
		return;
	}

	auto pController = GetController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	if (pController->GetModel() == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pModel = pController->GetModel()->As<CSTEPModel>();
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	if (pModel->GetDefinitions().empty())
	{
		return;
	}

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	if (bTransparent)
	{
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	m_pOGLProgram->_enableBlinnPhongModel(true);

	float fXTranslation = 0.f;
	float fYTranslation = 0.f;
	float fZTranslation = 0.f;
	pModel->GetWorldTranslations(fXTranslation, fYTranslation, fZTranslation);

	for (auto itCohort : m_oglBuffers.instancesCohorts())
	{
		glBindVertexArray(itCohort.first);

		for (auto pDefinition : itCohort.second)
		{
			if (pDefinition->ConcFacesCohorts().empty())
			{
				continue;
			}

			auto& vecInstances = pDefinition->GetInstances();
			for (size_t iInstance = 0; iInstance < vecInstances.size(); iInstance++)
			{
				auto pInstance = vecInstances[iInstance];
				if (!pInstance->GetEnable())
				{
					continue;
				}

				/*
				* Transformation Matrix
				*/
				glm::mat4 matTransformation = glm::make_mat4((GLdouble*)pInstance->GetTransformationMatrix());

				/*
				* Model-View Matrix
				*/
				glm::mat4 matModelView = m_matModelView;
				matModelView = glm::translate(matModelView, glm::vec3(fXTranslation, fYTranslation, fZTranslation));
				matModelView = matModelView * matTransformation;
				matModelView = glm::translate(matModelView, glm::vec3(-fXTranslation, -fYTranslation, -fZTranslation));

				m_pOGLProgram->_setModelViewMatrix(matModelView);

				for (size_t iCohort = 0; iCohort < pDefinition->ConcFacesCohorts().size(); iCohort++)
				{
					auto pCohort = pDefinition->ConcFacesCohorts()[iCohort];

					const _material* pMaterial =
						pInstance == m_pSelectedInstance ? m_pSelectedInstanceMaterial :
						pInstance == m_pPointedInstance ? m_pPointedInstanceMaterial :
						pCohort->getMaterial();

					if (bTransparent)
					{
						if (pMaterial->getA() == 1.0)
						{
							continue;
						}
					}
					else
					{
						if (pMaterial->getA() < 1.0)
						{
							continue;
						}
					}

					m_pOGLProgram->_setMaterial(pMaterial);

					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pCohort->ibo());
					glDrawElementsBaseVertex(GL_TRIANGLES,
						(GLsizei)pCohort->indices().size(),
						GL_UNSIGNED_INT,
						(void*)(sizeof(GLuint) * pCohort->iboOffset()),
						pDefinition->VBOOffset());
				}
			} // for (size_t iInstance = ...			
		} // for (auto pDefinition ...

		glBindVertexArray(0);
	} // for (auto itCohort ...

	if (bTransparent)
	{
		glDisable(GL_BLEND);
	}

	// Restore Model-View Matrix
	m_pOGLProgram->_setModelViewMatrix(m_matModelView);

	_oglUtils::checkForErrors();

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	TRACE(L"\n*** DrawFaces() : %lld [탎]", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());	
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::DrawConceptualFacesPolygons()
{
	if (!m_bShowConceptualFacesPolygons)
	{
		return;
	}

	auto pController = GetController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	if (pController->GetModel() == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pModel = pController->GetModel()->As<CSTEPModel>();
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	if (pModel->GetDefinitions().empty())
	{
		return;
	}

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	m_pOGLProgram->_enableBlinnPhongModel(false);
	m_pOGLProgram->_setAmbientColor(0.f, 0.f, 0.f);
	m_pOGLProgram->_setTransparency(1.f);

	float fXTranslation = 0.f;
	float fYTranslation = 0.f;
	float fZTranslation = 0.f;
	pModel->GetWorldTranslations(fXTranslation, fYTranslation, fZTranslation);	

	for (auto itCohort : m_oglBuffers.instancesCohorts())
	{
		glBindVertexArray(itCohort.first);

		for (auto pDefinition : itCohort.second)
		{
			if (pDefinition->ConcFacePolygonsCohorts().empty())
			{
				continue;
			}

			auto& vecInstances = pDefinition->GetInstances();
			for (size_t iInstance = 0; iInstance < vecInstances.size(); iInstance++)
			{
				auto pInstance = vecInstances[iInstance];
				if (!pInstance->GetEnable())
				{
					continue;
				}

				/*
				* Transformation Matrix
				*/
				glm::mat4 matTransformation = glm::make_mat4((GLdouble*)pInstance->GetTransformationMatrix());

				/*
				* Model-View Matrix
				*/
				glm::mat4 matModelView = m_matModelView;
				matModelView = glm::translate(matModelView, glm::vec3(fXTranslation, fYTranslation, fZTranslation));
				matModelView = matModelView * matTransformation;
				matModelView = glm::translate(matModelView, glm::vec3(-fXTranslation, -fYTranslation, -fZTranslation));

				m_pOGLProgram->_setModelViewMatrix(matModelView);
				
				for (size_t iCohort = 0; iCohort < pDefinition->ConcFacePolygonsCohorts().size(); iCohort++)
				{
					_cohort* pCohort = pDefinition->ConcFacePolygonsCohorts()[iCohort];

					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pCohort->ibo());
					glDrawElementsBaseVertex(GL_LINES,
						(GLsizei)pCohort->indices().size(),
						GL_UNSIGNED_INT,
						(void*)(sizeof(GLuint) * pCohort->iboOffset()),
						pDefinition->VBOOffset());
				}
			} // for (size_t iInstance = ...			
		} // for (auto pDefinition ...

		glBindVertexArray(0);
	} // for (auto itCohort ...

	// Restore Model-View Matrix
	m_pOGLProgram->_setModelViewMatrix(m_matModelView);

	_oglUtils::checkForErrors();

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	TRACE(L"\n*** DrawConceptualFacesPolygons() : %lld [탎]", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::DrawLines()
{
	if (!m_bShowLines)
	{
		return;
	}

	auto pController = GetController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	if (pController->GetModel() == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pModel = pController->GetModel()->As<CSTEPModel>();
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	if (pModel->GetDefinitions().empty())
	{
		return;
	}

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	m_pOGLProgram->_enableBlinnPhongModel(false);
	m_pOGLProgram->_setAmbientColor(0.f, 0.f, 0.f);
	m_pOGLProgram->_setTransparency(1.f);

	float fXTranslation = 0.f;
	float fYTranslation = 0.f;
	float fZTranslation = 0.f;
	pModel->GetWorldTranslations(fXTranslation, fYTranslation, fZTranslation);

	for (auto itCohort : m_oglBuffers.instancesCohorts())
	{
		glBindVertexArray(itCohort.first);

		for (auto pDefinition : itCohort.second)
		{
			if (pDefinition->LinesCohorts().empty())
			{
				continue;
			}

			auto& vecInstances = pDefinition->GetInstances();
			for (size_t iInstance = 0; iInstance < vecInstances.size(); iInstance++)
			{
				auto pInstance = vecInstances[iInstance];
				if (!pInstance->GetEnable())
				{
					continue;
				}

				/*
				* Transformation Matrix
				*/
				glm::mat4 matTransformation = glm::make_mat4((GLdouble*)pInstance->GetTransformationMatrix());

				/*
				* Model-View Matrix
				*/
				glm::mat4 matModelView = m_matModelView;
				matModelView = glm::translate(matModelView, glm::vec3(fXTranslation, fYTranslation, fZTranslation));
				matModelView = matModelView * matTransformation;
				matModelView = glm::translate(matModelView, glm::vec3(-fXTranslation, -fYTranslation, -fZTranslation));

				m_pOGLProgram->_setModelViewMatrix(matModelView);

				for (size_t iCohort = 0; iCohort < pDefinition->LinesCohorts().size(); iCohort++)
				{
					_cohort* pCohort = pDefinition->LinesCohorts()[iCohort];

					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pCohort->ibo());
					glDrawElementsBaseVertex(GL_LINES,
						(GLsizei)pCohort->indices().size(),
						GL_UNSIGNED_INT,
						(void*)(sizeof(GLuint) * pCohort->iboOffset()),
						pDefinition->VBOOffset());
				}
			} // for (size_t iInstance = ...			
		} // for (auto pDefinition ...

		glBindVertexArray(0);
	} // for (auto itCohort ...

	// Restore Model-View Matrix
	m_pOGLProgram->_setModelViewMatrix(m_matModelView);

	_oglUtils::checkForErrors();

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	TRACE(L"\n*** DrawLines() : %lld [탎]", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::DrawPoints()
{
	if (!m_bShowPoints)
	{
		return;
	}

	auto pController = GetController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	if (pController->GetModel() == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pModel = pController->GetModel()->As<CSTEPModel>();
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	if (pModel->GetDefinitions().empty())
	{
		return;
	}

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	glEnable(GL_PROGRAM_POINT_SIZE);

	m_pOGLProgram->_enableBlinnPhongModel(false);
	m_pOGLProgram->_setAmbientColor(0.f, 0.f, 0.f);

	float fXTranslation = 0.f;
	float fYTranslation = 0.f;
	float fZTranslation = 0.f;
	pModel->GetWorldTranslations(fXTranslation, fYTranslation, fZTranslation);

	for (auto itCohort : m_oglBuffers.instancesCohorts())
	{
		glBindVertexArray(itCohort.first);

		for (auto pDefinition : itCohort.second)
		{
			if (pDefinition->PointsCohorts().empty())
			{
				continue;
			}

			auto& vecInstances = pDefinition->GetInstances();
			for (auto pInstance : vecInstances)
			{
				if (!pInstance->GetEnable())
				{
					continue;
				}

				/*
				* Transformation Matrix
				*/
				glm::mat4 matTransformation = glm::make_mat4((GLdouble*)pInstance->GetTransformationMatrix());

				/*
				* Model-View Matrix
				*/
				glm::mat4 matModelView = m_matModelView;
				matModelView = glm::translate(matModelView, glm::vec3(fXTranslation, fYTranslation, fZTranslation));
				matModelView = matModelView * matTransformation;
				matModelView = glm::translate(matModelView, glm::vec3(-fXTranslation, -fYTranslation, -fZTranslation));

				m_pOGLProgram->_setModelViewMatrix(matModelView);

				for (auto pCohort : pDefinition->PointsCohorts())
				{
					const _material* pMaterial =
						pInstance == m_pSelectedInstance ? m_pSelectedInstanceMaterial :
						pInstance == m_pPointedInstance ? m_pPointedInstanceMaterial :
						pCohort->getMaterial();

					m_pOGLProgram->_setAmbientColor(
						pMaterial->getDiffuseColor().r(),
						pMaterial->getDiffuseColor().g(),
						pMaterial->getDiffuseColor().b());

					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pCohort->ibo());
					glDrawElementsBaseVertex(GL_POINTS,
						(GLsizei)pCohort->indices().size(),
						GL_UNSIGNED_INT,
						(void*)(sizeof(GLuint) * pCohort->iboOffset()),
						pDefinition->VBOOffset());
				}
			} // for (auto pInstance ...
		} // for (auto pDefinition ...

		glBindVertexArray(0);
	} // for (auto itCohort ...

	// Restore Model-View Matrix
	m_pOGLProgram->_setModelViewMatrix(m_matModelView);

	_oglUtils::checkForErrors();

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	TRACE(L"\n*** DrawPoints() : %lld [탎]", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::DrawInstancesFrameBuffer()
{
	CController * pController = GetController();
	ASSERT(pController != nullptr);

	if (pController->GetModel() == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pModel = pController->GetModel()->As<CSTEPModel>();
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	if (pModel->GetDefinitions().empty())
	{
		return;
	}

	/*
	* Create a frame buffer
	*/

	int iWidth = 0;
	int iHeight = 0;

#ifdef _LINUX
	const wxSize szClient = m_pWnd->GetClientSize();

	iWidth = szClient.GetWidth();
	iHeight = szClient.GetHeight();
#else
	CRect rcClient;
	m_pWnd->GetClientRect(&rcClient);

	iWidth = rcClient.Width();
	iHeight = rcClient.Height();
#endif // _LINUX

	if ((iWidth < 20) || (iHeight < 20))
	{
		return;
	}

	BOOL bResult = m_pOGLContext->makeCurrent();
	VERIFY(bResult);

	m_pInstanceSelectionFrameBuffer->create();

	/*
	* Selection colors
	*/
	if (m_pInstanceSelectionFrameBuffer->encoding().empty())
	{
		auto& mapDefinitions = pModel->GetDefinitions();
		for (auto itDefinition = mapDefinitions.begin(); 
			itDefinition != mapDefinitions.end(); 
			itDefinition++)
		{
			if (itDefinition->second->GetTriangles().empty())
			{
				continue;
			}

			auto& vecInstances = itDefinition->second->GetInstances();
			for (size_t iInstance = 0; iInstance < vecInstances.size(); iInstance++)
			{
				auto pInstance = vecInstances[iInstance];
				if (!pInstance->GetEnable())
				{
					continue;
				}

				float fR, fG, fB;
				_i64RGBCoder::encode(pInstance->GetID(), fR, fG, fB);

				m_pInstanceSelectionFrameBuffer->encoding()[pInstance->GetID()] = _color(fR, fG, fB);
			}
		}
	} // if (m_pInstanceSelectionFrameBuffer->encoding().empty())

	/*
	* Draw
	*/

	m_pInstanceSelectionFrameBuffer->bind();

	glViewport(0, 0, BUFFER_SIZE, BUFFER_SIZE);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set up the parameters
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	m_pOGLProgram->_enableBlinnPhongModel(false);
	m_pOGLProgram->_setTransparency(1.f);

	float fXTranslation = 0.f;
	float fYTranslation = 0.f;
	float fZTranslation = 0.f;
	pModel->GetWorldTranslations(fXTranslation, fYTranslation, fZTranslation);

	for (auto itCohort : m_oglBuffers.instancesCohorts())
	{
		glBindVertexArray(itCohort.first);

		for (auto pDefinition : itCohort.second)
		{
			if (pDefinition->GetTriangles().empty())
			{
				continue;
			}

			auto& vecInstances = pDefinition->GetInstances();
			for (size_t iInstance = 0; iInstance < vecInstances.size(); iInstance++)
			{
				auto pInstance = vecInstances[iInstance];
				if (!pInstance->GetEnable())
				{
					continue;
				}

				/*
				* Transformation Matrix
				*/
				glm::mat4 matTransformation = glm::make_mat4((GLdouble*)pInstance->GetTransformationMatrix());

				/*
				* Model-View Matrix
				*/
				glm::mat4 matModelView = m_matModelView;
				matModelView = glm::translate(matModelView, glm::vec3(fXTranslation, fYTranslation, fZTranslation));
				matModelView = matModelView * matTransformation;
				matModelView = glm::translate(matModelView, glm::vec3(-fXTranslation, -fYTranslation, -fZTranslation));

				m_pOGLProgram->_setModelViewMatrix(matModelView);

				for (size_t iCohort = 0; iCohort < pDefinition->ConcFacesCohorts().size(); iCohort++)
				{
					auto pCohort = pDefinition->ConcFacesCohorts()[iCohort];

					auto itSelectionColor = m_pInstanceSelectionFrameBuffer->encoding().find(pInstance->GetID());
					ASSERT(itSelectionColor != m_pInstanceSelectionFrameBuffer->encoding().end());

					m_pOGLProgram->_setAmbientColor(
						itSelectionColor->second.r(),
						itSelectionColor->second.g(),
						itSelectionColor->second.b());

					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pCohort->ibo());
					glDrawElementsBaseVertex(GL_TRIANGLES,
						(GLsizei)pCohort->indices().size(),
						GL_UNSIGNED_INT,
						(void*)(sizeof(GLuint) * pCohort->iboOffset()),
						pDefinition->VBOOffset());
				}
			} // for (size_t iInstance = ...			
		} // for (auto pDefinition ...

		glBindVertexArray(0);
	} // for (auto itCohort ...

	// Restore Model-View Matrix
	m_pOGLProgram->_setModelViewMatrix(m_matModelView);

	m_pInstanceSelectionFrameBuffer->unbind();

	_oglUtils::checkForErrors();
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::OnMouseMoveEvent(UINT nFlags, CPoint point)
{
	auto pController = GetController();
	if (pController->GetModel() == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	if (pController->GetModel() == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pModel = pController->GetModel()->As<CSTEPModel>();
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

#ifdef _LINUX
		m_pOGLContext->SetCurrent(*m_pWnd);

		const wxSize szClient = m_pWnd->GetClientSize();

		iWidth = szClient.GetWidth();
		iHeight = szClient.GetHeight();
#else
		BOOL bResult = m_pOGLContext->makeCurrent();
		VERIFY(bResult);

		CRect rcClient;
		m_pWnd->GetClientRect(&rcClient);

		iWidth = rcClient.Width();
		iHeight = rcClient.Height();
#endif // _LINUX

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

		CProductInstance* pPointedInstance = nullptr;
		if (arPixels[3] != 0)
		{
			int64_t iInstanceID = _i64RGBCoder::decode(arPixels[0], arPixels[1], arPixels[2]);
			pPointedInstance = pModel->getProductInstanceByID(iInstanceID);
			ASSERT(pPointedInstance != nullptr);
		}

		if (m_pPointedInstance != pPointedInstance)
		{
			m_pPointedInstance = pPointedInstance;

#ifdef _LINUX
			m_pWnd->Refresh(false);
#else
			_redraw();
#endif // _LINUX
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
		float fXAngle = ((float)point.y - (float)m_ptPrevMousePosition.y);
		float fYAngle = ((float)point.x - (float)m_ptPrevMousePosition.x);

		if (abs(fXAngle) >= abs(fYAngle) * ROTATION_SENSITIVITY)
		{
			fYAngle = 0.;
		}
		else
		{
			if (abs(fYAngle) >= abs(fXAngle) * ROTATION_SENSITIVITY)
			{
				fXAngle = 0.;
			}
		}

		_rotate(
			fXAngle * ROTATION_SPEED, 
			fYAngle * ROTATION_SPEED);

		m_ptPrevMousePosition = point;

		return;
	}

	/*
	* Zoom
	*/
	if ((nFlags & MK_MBUTTON) == MK_MBUTTON)
	{
		_zoom(
			point.y - m_ptPrevMousePosition.y > 0 ? 
			-abs(m_fZTranslation) * ZOOM_SPEED_MOUSE : 
			abs(m_fZTranslation) * ZOOM_SPEED_MOUSE);

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

		_move(
			PAN_SPEED_MOUSE* (((float)point.x - (float)m_ptPrevMousePosition.x) / rcClient.Width()),
			-(PAN_SPEED_MOUSE * (((float)point.y - (float)m_ptPrevMousePosition.y) / rcClient.Height())));

		m_ptPrevMousePosition = point;

		return;
	}
}

