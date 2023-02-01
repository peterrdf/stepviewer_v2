#include "stdafx.h"

#include "OpenGLIFCView.h"
#include "STEPController.h"
#include "IFCModel.h"
#include "resource.h"

#include <assert.h>
#include <chrono>

// ------------------------------------------------------------------------------------------------
COpenGLIFCView::COpenGLIFCView(CWnd * pWnd)
	: COpenGLView()
	, m_pWnd(pWnd)
	, m_enProjection(enumProjection::Perspective)
	, m_bShowFaces(TRUE)	
	, m_bShowConceptualFacesPolygons(TRUE)
	, m_bShowLines(TRUE)
	, m_bShowPoints(TRUE)
	, m_ptStartMousePosition(-1, -1)
	, m_ptPrevMousePosition(-1, -1)
	, m_pInstanceSelectionFrameBuffer(new _oglSelectionFramebuffer())
	, m_pPointedInstance(NULL)
	, m_pSelectedInstance(NULL)
	, m_pSelectedInstanceMaterial(NULL)
	, m_pPointedInstanceMaterial(NULL)
{
	ASSERT(m_pWnd != NULL);	

	_initialize(
		*(m_pWnd->GetDC()),
		16,
		IDR_TEXTFILE_VERTEX_SHADER2,
		IDR_TEXTFILE_FRAGMENT_SHADER2,
		TEXTFILE,
		false);

	m_pSelectedInstanceMaterial = new _material();
	m_pSelectedInstanceMaterial->init(
		1.f, 0.f, 0.f,
		1.f, 0.f, 0.f,
		1.f, 0.f, 0.f,
		1.f, 0.f, 0.f,
		1.f,
		nullptr);

	m_pPointedInstanceMaterial = new _material();
	m_pPointedInstanceMaterial->init(
		.33f, .33f, .33f,
		.33f, .33f, .33f,
		.33f, .33f, .33f,
		.33f, .33f, .33f,
		.66f,
		nullptr);
}

// ------------------------------------------------------------------------------------------------
COpenGLIFCView::~COpenGLIFCView()
{
	GetController()->UnRegisterView(this);	

	delete m_pInstanceSelectionFrameBuffer;

	delete m_pSelectedInstanceMaterial;
	m_pSelectedInstanceMaterial = NULL;

	delete m_pPointedInstanceMaterial;
	m_pPointedInstanceMaterial = NULL;
}

// ------------------------------------------------------------------------------------------------
enumProjection COpenGLIFCView::GetProjection() const
{
	return m_enProjection;
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::SetProjection(enumProjection enProjection)
{
	m_enProjection = enProjection;

	m_pWnd->RedrawWindow();
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::SetView(enum enumView enView)
{
	switch (enView)
	{
		case enumView::Front:
		{
			m_fXAngle = 0.;
			m_fYAngle = 0.;
		}
		break;

		case enumView::Right:
		{
			m_fXAngle = 0.;
			m_fYAngle = -90.;
		}
		break;

		case enumView::Top:
		{
			m_fXAngle = 90.;
			m_fYAngle = 0.;
		}
		break;

		case enumView::Back:
		{
			m_fXAngle = 0.;
			m_fYAngle = -180.;
		}
		break;

		case enumView::Left:
		{
			m_fXAngle = 0.;
			m_fYAngle = 90.;
		}
		break;

		case enumView::Bottom:
		{
			m_fXAngle = -90.;
			m_fYAngle = 0.;
		}
		break;

		default:
		{
			ASSERT(FALSE);
		}
		break;
	} // switch (enView)

	m_pWnd->RedrawWindow();
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::ShowFaces(BOOL bShow)
{
	m_bShowFaces = bShow;

	m_pWnd->RedrawWindow();
}

// ------------------------------------------------------------------------------------------------
BOOL COpenGLIFCView::AreFacesShown()
{
	return m_bShowFaces;
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::ShowConceptualFacesPolygons(BOOL bShow)
{
	m_bShowConceptualFacesPolygons = bShow;

	m_pWnd->RedrawWindow();
}

// ------------------------------------------------------------------------------------------------
BOOL COpenGLIFCView::AreConceptualFacesPolygonsShown()
{
	return m_bShowConceptualFacesPolygons;
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::ShowLines(BOOL bShow)
{
	m_bShowLines = bShow;

	m_pWnd->RedrawWindow();
}

// ------------------------------------------------------------------------------------------------
BOOL COpenGLIFCView::AreLinesShown()
{
	return m_bShowLines;
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::ShowPoints(BOOL bShow)
{
	m_bShowPoints = bShow;

	m_pWnd->RedrawWindow();
}

// ------------------------------------------------------------------------------------------------
BOOL COpenGLIFCView::ArePointsShown()
{
	return m_bShowPoints;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void COpenGLIFCView::Load()
{
	CWaitCursor waitCursor;

#ifdef _LINUX
	m_pOGLContext->SetCurrent(*m_pWnd);
#else
	BOOL bResult = m_pOGLContext->makeCurrent();
	VERIFY(bResult);
#endif // _LINUX

	// OpenGL buffers
	m_oglBuffers.clear();

	m_pInstanceSelectionFrameBuffer->encoding().clear();
	m_pPointedInstance = NULL;
	m_pSelectedInstance = NULL;

	auto pController = GetController();
	if (pController == NULL)
	{
		ASSERT(FALSE);

		return;
	}

	if (pController->GetModel() == NULL)
	{
		ASSERT(FALSE);

		return;
	}

	auto pModel = pController->GetModel()->As<CIFCModel>();
	if (pModel == NULL)
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
	fBoundingSphereDiameter = fmax(fBoundingSphereDiameter, fYmax - fYmin);
	fBoundingSphereDiameter = fmax(fBoundingSphereDiameter, fZmax - fZmin);

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

	auto& mapInstances = pModel->GetInstances();

	// VBO
	GLuint iVerticesCount = 0;
	vector<CIFCInstance*> vecInstancesCohort;

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

	for (auto itIinstance = mapInstances.begin(); itIinstance != mapInstances.end(); itIinstance++)
	{
		auto pInstance = itIinstance->second;
		if (pInstance->getVerticesCount() == 0)
		{
			continue;
		}

		/******************************************************************************************
		* Geometry
		*/

		/**
		* VBO - Conceptual faces, polygons, etc.
		*/
		if (((int_t)iVerticesCount + pInstance->getVerticesCount()) > (int_t)VERTICES_MAX_COUNT)
		{
			if (m_oglBuffers.createInstancesCohort(vecInstancesCohort, m_pOGLProgram) != iVerticesCount)
			{
				assert(false);

				return;
			}

			iVerticesCount = 0;
			vecInstancesCohort.clear();
		}

		/*
		* IBO - Conceptual faces
		*/
		for (size_t iFacesCohort = 0; iFacesCohort < pInstance->concFacesCohorts().size(); iFacesCohort++)
		{
			if ((int_t)(iConcFacesIndicesCount + pInstance->concFacesCohorts()[iFacesCohort]->indices().size()) > (int_t)INDICES_MAX_COUNT)
			{
				if (m_oglBuffers.createIBO(vecConcFacesCohorts) != iConcFacesIndicesCount)
				{
					assert(false);

					return;
				}

				iConcFacesIndicesCount = 0;
				vecConcFacesCohorts.clear();
			}

			iConcFacesIndicesCount += (GLsizei)pInstance->concFacesCohorts()[iFacesCohort]->indices().size();
			vecConcFacesCohorts.push_back(pInstance->concFacesCohorts()[iFacesCohort]);
		}

		/*
		* IBO - Conceptual face polygons
		*/
		for (size_t iConcFacePolygonsCohort = 0; iConcFacePolygonsCohort < pInstance->concFacePolygonsCohorts().size(); iConcFacePolygonsCohort++)
		{
			if ((int_t)(iConcFacePolygonsIndicesCount + pInstance->concFacePolygonsCohorts()[iConcFacePolygonsCohort]->indices().size()) > (int_t)INDICES_MAX_COUNT)
			{
				if (m_oglBuffers.createIBO(vecConcFacePolygonsCohorts) != iConcFacePolygonsIndicesCount)
				{
					assert(false);

					return;
				}

				iConcFacePolygonsIndicesCount = 0;
				vecConcFacePolygonsCohorts.clear();
			}

			iConcFacePolygonsIndicesCount += (GLsizei)pInstance->concFacePolygonsCohorts()[iConcFacePolygonsCohort]->indices().size();
			vecConcFacePolygonsCohorts.push_back(pInstance->concFacePolygonsCohorts()[iConcFacePolygonsCohort]);
		}

		/*
		* IBO - Lines
		*/
		for (size_t iLinesCohort = 0; iLinesCohort < pInstance->linesCohorts().size(); iLinesCohort++)
		{
			if ((int_t)(iLinesIndicesCount + pInstance->linesCohorts()[iLinesCohort]->indices().size()) > (int_t)INDICES_MAX_COUNT)
			{
				if (m_oglBuffers.createIBO(vecLinesCohorts) != iLinesIndicesCount)
				{
					assert(false);

					return;
				}

				iLinesIndicesCount = 0;
				vecLinesCohorts.clear();
			}

			iLinesIndicesCount += (GLsizei)pInstance->linesCohorts()[iLinesCohort]->indices().size();
			vecLinesCohorts.push_back(pInstance->linesCohorts()[iLinesCohort]);
		}

		/*
		* IBO - Points
		*/
		for (size_t iPointsCohort = 0; iPointsCohort < pInstance->pointsCohorts().size(); iPointsCohort++)
		{
			if ((int_t)(iPointsIndicesCount + pInstance->pointsCohorts()[iPointsCohort]->indices().size()) > (int_t)INDICES_MAX_COUNT)
			{
				if (m_oglBuffers.createIBO(vecPointsCohorts) != iPointsIndicesCount)
				{
					assert(false);

					return;
				}

				iPointsIndicesCount = 0;
				vecPointsCohorts.clear();
			}

			iPointsIndicesCount += (GLsizei)pInstance->pointsCohorts()[iPointsCohort]->indices().size();
			vecPointsCohorts.push_back(pInstance->pointsCohorts()[iPointsCohort]);
		}

		iVerticesCount += (GLsizei)pInstance->getVerticesCount();
		vecInstancesCohort.push_back(pInstance);
	} // for (; itIinstances != ...

	/******************************************************************************************
	* Geometry
	*/

	/*
	* VBO - Conceptual faces, polygons, etc.
	*/
	if (iVerticesCount > 0)
	{
		if (m_oglBuffers.createInstancesCohort(vecInstancesCohort, m_pOGLProgram) != iVerticesCount)
		{
			assert(false);

			return;
		}

		iVerticesCount = 0;
		vecInstancesCohort.clear();
	}

	/*
	* IBO - Conceptual faces
	*/
	if (iConcFacesIndicesCount > 0)
	{
		if (m_oglBuffers.createIBO(vecConcFacesCohorts) != iConcFacesIndicesCount)
		{
			assert(false);

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
			assert(false);

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
			assert(false);

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
			assert(false);

			return;
		}

		iPointsIndicesCount = 0;
		vecPointsCohorts.clear();
	}

#ifdef _LINUX
	m_pWnd->Refresh(false);
#else
	m_pWnd->RedrawWindow();
#endif // _LINUX
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void COpenGLIFCView::Draw(CDC* pDC)
{
	auto pController = GetController();
	if (pController == NULL)
	{
		ASSERT(FALSE);

		return;
	}

	if (pController->GetModel() == NULL)
	{
		return;
	}

	auto pModel = pController->GetModel()->As<CIFCModel>();
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	BOOL bResult = m_pOGLContext->makeCurrent();
	VERIFY(bResult);

#ifdef _ENABLE_OPENGL_DEBUG
	m_pOGLContext->enableDebug();
#endif

	CRect rcClient;
	m_pWnd->GetClientRect(&rcClient);

	int iWidth = rcClient.Width();
	int iHeight = rcClient.Height();

	if ((iWidth < 20) || (iHeight < 20))
	{
		return;
	}

	m_pOGLProgram->use();

	glViewport(0, 0, iWidth, iHeight);

	glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set up the parameters
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	m_pOGLProgram->setPointLightLocation(0.f, 0.f, 10000.f);
	m_pOGLProgram->setMaterialShininess(30.f);

	/*
	* Projection Matrix
	*/
	// fovY     - Field of vision in degrees in the y direction
	// aspect   - Aspect ratio of the viewport
	// zNear    - The near clipping distance
	// zFar     - The far clipping distance
	GLdouble fovY = 45.0;
	GLdouble aspect = (GLdouble)iWidth / (GLdouble)iHeight;
	GLdouble zNear = 0.0001;
	GLdouble zFar = 1000.0;

	GLdouble fH = tan(fovY / 360 * M_PI) * zNear;
	GLdouble fW = fH * aspect;

	// Projection
	switch (m_enProjection)
	{
		case enumProjection::Perspective:
		{
			glm::mat4 matProjection = glm::frustum<GLdouble>(-fW, fW, -fH, fH, zNear, zFar);
			m_pOGLProgram->setProjectionMatrix(matProjection);
		}
		break;

		case enumProjection::Isometric:
		{
			glm::mat4 matProjection = glm::ortho<GLdouble>(-1.5, 1.5, -1.5, 1.5, zNear, zFar);
			m_pOGLProgram->setProjectionMatrix(matProjection);
		}
		break;

		default:
		{
			ASSERT(FALSE);
		}
		break;
	}

	/*
	* Model-View Matrix
	*/
	m_matModelView = glm::identity<glm::mat4>();
	m_matModelView = glm::translate(m_matModelView, glm::vec3(m_fXTranslation, m_fYTranslation, m_fZTranslation));

	float fXmin = -1.f;
	float fXmax = 1.f;
	float fYmin = -1.f;
	float fYmax = 1.f;
	float fZmin = -1.f;
	float fZmax = 1.f;
	pModel->GetWorldDimensions(fXmin, fXmax, fYmin, fYmax, fZmin, fZmax);

	float fXTranslation = fXmin;
	fXTranslation += (fXmax - fXmin) / 2.f;
	fXTranslation = -fXTranslation;

	float fYTranslation = fYmin;
	fYTranslation += (fYmax - fYmin) / 2.f;
	fYTranslation = -fYTranslation;

	float fZTranslation = fZmin;
	fZTranslation += (fZmax - fZmin) / 2.f;
	fZTranslation = -fZTranslation;

	m_matModelView = glm::translate(m_matModelView, glm::vec3(-fXTranslation, -fYTranslation, -fZTranslation));
	m_matModelView = glm::rotate(m_matModelView, m_fXAngle, glm::vec3(1.0f, 0.0f, 0.0f));
	m_matModelView = glm::rotate(m_matModelView, m_fYAngle, glm::vec3(0.0f, 1.0f, 0.0f));
	m_matModelView = glm::translate(m_matModelView, glm::vec3(fXTranslation, fYTranslation, fZTranslation));
	m_pOGLProgram->setModelViewMatrix(m_matModelView);

	/*
	* Normal Matrix
	*/
	glm::mat4 matNormal = m_matModelView;
	matNormal = glm::inverse(matNormal);
	matNormal = glm::transpose(matNormal);
	m_pOGLProgram->setNormalMatrix(matNormal);

	m_pOGLProgram->enableBinnPhongModel(true);

	/*
	Non-transparent faces
	*/
	DrawFaces(false);

	/*
	Transparent faces
	*/
	DrawFaces(true);

	/*
	Conceptual faces polygons
	*/
	DrawConceptualFacesPolygons();

	/*
	Lines
	*/
	DrawLines();

	/*
	Points
	*/
	DrawPoints();

	/*
	End
	*/
#ifdef _LINUX
	m_pWnd->SwapBuffers();
#else
	SwapBuffers(*pDC);
#endif // _LINUX

	/*
	Selection support
	*/
	DrawInstancesFrameBuffer();
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::OnMouseEvent(enumMouseEvent enEvent, UINT nFlags, CPoint point)
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
				m_pWnd->RedrawWindow();
#endif // _LINUX

				ASSERT(GetController() != NULL);

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
void COpenGLIFCView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	UNREFERENCED_PARAMETER(nFlags);
	UNREFERENCED_PARAMETER(pt);

	Zoom((float)zDelta < 0.f ? -abs(m_fZTranslation) * ZOOM_SPEED_MOUSE_WHEEL : abs(m_fZTranslation) * ZOOM_SPEED_MOUSE_WHEEL);
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void COpenGLIFCView::OnInstancesEnabledStateChanged(CSTEPView* pSender)
{
	if (pSender == this)
	{
		return;
	}
		
	m_pWnd->RedrawWindow();
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void COpenGLIFCView::OnInstanceSelected(CSTEPView* pSender)
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

#ifdef _LINUX
		m_pWnd->Refresh(false);
#else
		m_pWnd->RedrawWindow();
#endif // _LINUX
	}
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void COpenGLIFCView::OnControllerChanged()
{
	if (GetController() != NULL)
	{
		GetController()->RegisterView(this);
	}
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::DrawFaces(bool bTransparent)
{
	if (!m_bShowFaces)
	{
		return;
	}

	auto pController = GetController();
	if (pController->GetModel() == NULL)
	{
		ASSERT(FALSE);

		return;
	}

	if (pController->GetModel() == NULL)
	{
		ASSERT(FALSE);

		return;
	}

	auto pModel = pController->GetModel()->As<CIFCModel>();
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}	

	auto begin = std::chrono::steady_clock::now();

	if (bTransparent)
	{
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	m_pOGLProgram->enableBinnPhongModel(true);

	for (auto itCohort : m_oglBuffers.instancesCohorts())
	{
		glBindVertexArray(itCohort.first);

		for (auto pInstance : itCohort.second)
		{
			if (!pInstance->getEnable())
			{
				continue;
			}

			for (auto pConcFacesCohort : pInstance->concFacesCohorts())
			{
				const _material* pMaterial =
					pInstance == m_pSelectedInstance ? m_pSelectedInstanceMaterial :
					pInstance == m_pPointedInstance ? m_pPointedInstanceMaterial :
					pConcFacesCohort->getMaterial();

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

				m_pOGLProgram->setMaterial(pMaterial);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pConcFacesCohort->ibo());
				glDrawElementsBaseVertex(GL_TRIANGLES,
					(GLsizei)pConcFacesCohort->indices().size(),
					GL_UNSIGNED_INT,
					(void*)(sizeof(GLuint) * pConcFacesCohort->iboOffset()),
					pInstance->VBOOffset());
			} // for (auto pConcFacesCohort ...
		} // for (auto pInstance ...

		glBindVertexArray(0);
	} // for (auto itCohort ...

	if (bTransparent)
	{
		glDisable(GL_BLEND);
	}

	_oglUtils::checkForErrors();

	auto end = std::chrono::steady_clock::now();
	TRACE(L"\n*** DrawFaces() : %lld [탎]", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::DrawConceptualFacesPolygons()
{
	if (!m_bShowConceptualFacesPolygons)
	{
		return;
	}

	auto pController = GetController();
	if (pController->GetModel() == NULL)
	{
		ASSERT(FALSE);

		return;
	}

	if (pController->GetModel() == NULL)
	{
		ASSERT(FALSE);

		return;
	}

	auto pModel = pController->GetModel()->As<CIFCModel>();
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	m_pOGLProgram->enableBinnPhongModel(false);
	m_pOGLProgram->setAmbientColor(0.f, 0.f, 0.f);
	m_pOGLProgram->setTransparency(1.f);

	for (auto itCohort : m_oglBuffers.instancesCohorts())
	{
		glBindVertexArray(itCohort.first);

		for (auto pInstance : itCohort.second)
		{
			if (!pInstance->getEnable())
			{
				continue;
			}

			for (auto pCohort : pInstance->concFacePolygonsCohorts())
			{
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pCohort->ibo());
				glDrawElementsBaseVertex(GL_LINES,
					(GLsizei)pCohort->indices().size(),
					GL_UNSIGNED_INT,
					(void*)(sizeof(GLuint) * pCohort->iboOffset()),
					pInstance->VBOOffset());
			}
		} // for (auto pInstance ...

		glBindVertexArray(0);
	} // for (auto itCohort ...

	_oglUtils::checkForErrors();

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	TRACE(L"\n*** DrawConceptualFacesPolygons() : %lld [탎]", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::DrawLines()
{
	if (!m_bShowLines)
	{
		return;
	}

	auto pController = GetController();
	if (pController->GetModel() == NULL)
	{
		ASSERT(FALSE);

		return;
	}

	if (pController->GetModel() == NULL)
	{
		ASSERT(FALSE);

		return;
	}

	auto pModel = pController->GetModel()->As<CIFCModel>();
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto begin = std::chrono::steady_clock::now();

	m_pOGLProgram->enableBinnPhongModel(false);
	m_pOGLProgram->setAmbientColor(0.f, 0.f, 0.f);
	m_pOGLProgram->setTransparency(1.f);

	for (auto itCohort : m_oglBuffers.instancesCohorts())
	{
		glBindVertexArray(itCohort.first);

		for (auto pInstance : itCohort.second)
		{
			if (!pInstance->getEnable())
			{
				continue;
			}

			for (auto pCohort : pInstance->linesCohorts())
			{
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pCohort->ibo());
				glDrawElementsBaseVertex(GL_LINES,
					(GLsizei)pCohort->indices().size(),
					GL_UNSIGNED_INT,
					(void*)(sizeof(GLuint) * pCohort->iboOffset()),
					pInstance->VBOOffset());
			}
		} // for (auto pInstance ...

		glBindVertexArray(0);
	} // for (auto itCohort ...

	_oglUtils::checkForErrors();

	auto end = std::chrono::steady_clock::now();
	TRACE(L"\n*** DrawLines() : %lld [탎]", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::DrawPoints()
{
	if (!m_bShowPoints)
	{
		return;
	}

	auto pController = GetController();
	if (pController->GetModel() == NULL)
	{
		ASSERT(FALSE);

		return;
	}

	if (pController->GetModel() == NULL)
	{
		ASSERT(FALSE);

		return;
	}

	auto pModel = pController->GetModel()->As<CIFCModel>();
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto begin = std::chrono::steady_clock::now();

	glEnable(GL_PROGRAM_POINT_SIZE);

	m_pOGLProgram->enableBinnPhongModel(false);
	m_pOGLProgram->setTransparency(1.f);

	for (auto itCohort : m_oglBuffers.instancesCohorts())
	{
		glBindVertexArray(itCohort.first);

		for (auto itInstance : itCohort.second)
		{
			auto pRDFInstance = itInstance;
			if (!pRDFInstance->getEnable())
			{
				continue;
			}

			for (size_t iPointsCohort = 0; iPointsCohort < pRDFInstance->pointsCohorts().size(); iPointsCohort++)
			{
				auto pCohort = pRDFInstance->pointsCohorts()[iPointsCohort];

				const _material* pMaterial =
					pRDFInstance == m_pSelectedInstance ? m_pSelectedInstanceMaterial :
					pRDFInstance == m_pPointedInstance ? m_pPointedInstanceMaterial :
					pCohort->getMaterial();				

				m_pOGLProgram->setAmbientColor(
					pMaterial->getDiffuseColor().r(),
					pMaterial->getDiffuseColor().g(),
					pMaterial->getDiffuseColor().b());

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pCohort->ibo());
				glDrawElementsBaseVertex(GL_POINTS,
					(GLsizei)pCohort->indices().size(),
					GL_UNSIGNED_INT,
					(void*)(sizeof(GLuint) * pCohort->iboOffset()),
					pRDFInstance->VBOOffset());
			} // for (size_t iPointsCohort = ...		
		} // for (auto itInstance ...

		glBindVertexArray(0);
	} // for (auto itCohort ...

	glDisable(GL_PROGRAM_POINT_SIZE);

	_oglUtils::checkForErrors();

	auto end = std::chrono::steady_clock::now();
	TRACE(L"\n*** DrawPoints() : %lld [탎]", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::DrawInstancesFrameBuffer()
{
	auto pController = GetController();
	if (pController->GetModel() == NULL)
	{
		ASSERT(FALSE);

		return;
	}

	if (pController->GetModel() == NULL)
	{
		ASSERT(FALSE);

		return;
	}

	auto pModel = pController->GetModel()->As<CIFCModel>();
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	/*
	* Create a frame buffer
	*/

	BOOL bResult = m_pOGLContext->makeCurrent();
	VERIFY(bResult);

	m_pInstanceSelectionFrameBuffer->create();

	/*
	* Selection colors
	*/
	if (m_pInstanceSelectionFrameBuffer->encoding().empty())
	{
		auto& mapInstances = pModel->GetInstances();
		for (auto itInstance = mapInstances.begin(); itInstance != mapInstances.end(); itInstance++)
		{
			auto pInstance = itInstance->second;

			auto& vecTriangles = pInstance->getTriangles();
			if (vecTriangles.empty())
			{
				continue;
			}

			float fR, fG, fB;
			_i64RGBCoder::encode(pInstance->ID(), fR, fG, fB);

			m_pInstanceSelectionFrameBuffer->encoding()[pInstance->getInstance()] = _color(fR, fG, fB);
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

	m_pOGLProgram->enableBinnPhongModel(false);
	m_pOGLProgram->setTransparency(1.f);

	for (auto itCohort : m_oglBuffers.instancesCohorts())
	{
		glBindVertexArray(itCohort.first);

		for (auto pInstance : itCohort.second)
		{
			if (!pInstance->getEnable())
			{
				continue;
			}

			auto& vecTriangles = pInstance->getTriangles();
			if (vecTriangles.empty())
			{
				continue;
			}

			auto itSelectionColor = m_pInstanceSelectionFrameBuffer->encoding().find(pInstance->getInstance());
			ASSERT(itSelectionColor != m_pInstanceSelectionFrameBuffer->encoding().end());

			m_pOGLProgram->setAmbientColor(
				itSelectionColor->second.r(),
				itSelectionColor->second.g(),
				itSelectionColor->second.b());

			for (size_t iConcFacesCohort = 0; iConcFacesCohort < pInstance->concFacesCohorts().size(); iConcFacesCohort++)
			{
				auto pConcFacesCohort = pInstance->concFacesCohorts()[iConcFacesCohort];

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pConcFacesCohort->ibo());
				glDrawElementsBaseVertex(GL_TRIANGLES,
					(GLsizei)pConcFacesCohort->indices().size(),
					GL_UNSIGNED_INT,
					(void*)(sizeof(GLuint) * pConcFacesCohort->iboOffset()),
					pInstance->VBOOffset());
			}
		} // for (auto pInstance ...

		glBindVertexArray(0);
	} // for (auto itCohort ...

	m_pInstanceSelectionFrameBuffer->unbind();

	_oglUtils::checkForErrors();
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::OnMouseMoveEvent(UINT nFlags, CPoint point)
{
	auto pController = GetController();
	if (pController->GetModel() == NULL)
	{
		ASSERT(FALSE);

		return;
	}

	if (pController->GetModel() == NULL)
	{
		ASSERT(FALSE);

		return;
	}

	auto pModel = pController->GetModel()->As<CIFCModel>();
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	/*
	* Selection
	*/
	if (((nFlags & MK_LBUTTON) != MK_LBUTTON) && ((nFlags & MK_MBUTTON) != MK_MBUTTON) && ((nFlags & MK_RBUTTON) != MK_RBUTTON))
	{
		/*
		* Select an instance
		*/
		if (m_pInstanceSelectionFrameBuffer->isInitialized())
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

			CIFCInstance* pPointedInstance = NULL;
			if (arPixels[3] != 0)
			{
				int64_t iObjectID = _i64RGBCoder::decode(arPixels[0], arPixels[1], arPixels[2]);
				pPointedInstance = pModel->GetInstanceByID(iObjectID);
				ASSERT(pPointedInstance != NULL);
			}

			if (m_pPointedInstance != pPointedInstance)
			{
				m_pPointedInstance = pPointedInstance;

#ifdef _LINUX
				m_pWnd->Refresh(false);
#else
				m_pWnd->RedrawWindow();
#endif // _LINUX
			}
		} // if (m_pInstanceSelectionFrameBuffer->isInitialized())
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

		Rotate(fXAngle * ROTATION_SPEED, fYAngle * ROTATION_SPEED);

		m_ptPrevMousePosition = point;

		return;
	}

	/*
	* Zoom
	*/
	if ((nFlags & MK_MBUTTON) == MK_MBUTTON)
	{
		Zoom(point.y - m_ptPrevMousePosition.y > 0 ? -abs(m_fZTranslation) * ZOOM_SPEED_MOUSE : abs(m_fZTranslation) * ZOOM_SPEED_MOUSE);

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

		m_fXTranslation += PAN_SPEED_MOUSE * (((float)point.x - (float)m_ptPrevMousePosition.x) / rcClient.Width());
		m_fYTranslation -= PAN_SPEED_MOUSE * (((float)point.y - (float)m_ptPrevMousePosition.y) / rcClient.Height());

#ifdef _LINUX
		m_pWnd->Refresh(false);
#else
		m_pWnd->RedrawWindow();
#endif // _LINUX

		m_ptPrevMousePosition = point;

		return;
	}
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::Rotate(float fXAngle, float fYAngle)
{
	m_fXAngle += fXAngle * (180.f / (float)M_PI);
	if (m_fXAngle > 360.f)
	{
		m_fXAngle -= 360.f;
	}
	else if (m_fXAngle < -360.f)
	{
		m_fXAngle += 360.f;
	}

	m_fYAngle += fYAngle * (180.f / (float)M_PI);
	if (m_fYAngle > 360.f)
	{
		m_fYAngle = m_fYAngle - 360.f;
	}
	else if (m_fYAngle < -360.f)
	{
		m_fYAngle += 360.f;
	}

#ifdef _LINUX
	m_pWnd->Refresh(false);
#else
	m_pWnd->RedrawWindow();
#endif // _LINUX
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::Zoom(float fZTranslation)
{
	m_fZTranslation += fZTranslation;

#ifdef _LINUX
	m_pWnd->Refresh(false);
#else
	m_pWnd->RedrawWindow();
#endif // _LINUX
}

