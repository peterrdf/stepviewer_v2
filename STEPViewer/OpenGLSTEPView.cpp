#include "stdafx.h"

#include "OpenGLSTEPView.h"
#include "STEPController.h"
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
COpenGLSTEPView::COpenGLSTEPView(CWnd * pWnd)
#endif // _LINUX
	: _oglRenderer()
	, COpenGLView()
	, m_pWnd(pWnd)
	, m_bShowFaces(TRUE)
	, m_bShowFacesPolygons(FALSE)
	, m_bShowConceptualFacesPolygons(TRUE)
	, m_bShowLines(TRUE)
	, m_fLineWidth(1.f)
	, m_bShowPoints(TRUE)
	, m_fPointSize(1.f)
	, m_ptStartMousePosition(-1, -1)
	, m_ptPrevMousePosition(-1, -1)
	, m_pInstanceSelectionFrameBuffer(new _oglSelectionFramebuffer())
	, m_bDisableSelectionBuffer(FALSE)
	, m_pPointedInstance(nullptr)
	, m_pSelectedInstance(nullptr)
	, m_pSelectedInstanceMaterial(nullptr)
	, m_pPointedInstanceMaterial(nullptr)
	, m_hFont(nullptr)
{
	ASSERT(m_pWnd != nullptr);

	_initialize(
		*(m_pWnd->GetDC()),
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
	ASSERT(m_hFont != NULL);
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
void COpenGLSTEPView::ShowFaces(BOOL bShow)
{
	m_bShowFaces = bShow;

#ifdef _LINUX
    m_pWnd->Refresh(false);
#else
    m_pWnd->RedrawWindow();
#endif // _LINUX
}

// ------------------------------------------------------------------------------------------------
BOOL COpenGLSTEPView::AreFacesShown() const
{
	return m_bShowFaces;
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::ShowFacesPolygons(BOOL bShow)
{
	m_bShowFacesPolygons = bShow;

#ifdef _LINUX
    m_pWnd->Refresh(false);
#else
    m_pWnd->RedrawWindow();
#endif // _LINUX
}

// ------------------------------------------------------------------------------------------------
BOOL COpenGLSTEPView::AreFacesPolygonsShown() const
{
	return m_bShowFacesPolygons;
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::ShowConceptualFacesPolygons(BOOL bShow)
{
	m_bShowConceptualFacesPolygons = bShow;

#ifdef _LINUX
    m_pWnd->Refresh(false);
#else
    m_pWnd->RedrawWindow();
#endif // _LINUX
}

// ------------------------------------------------------------------------------------------------
BOOL COpenGLSTEPView::AreConceptualFacesPolygonsShown() const
{
	return m_bShowConceptualFacesPolygons;
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::ShowLines(BOOL bShow)
{
	m_bShowLines = bShow;

#ifdef _LINUX
    m_pWnd->Refresh(false);
#else
    m_pWnd->RedrawWindow();
#endif // _LINUX
}

// ------------------------------------------------------------------------------------------------
BOOL COpenGLSTEPView::AreLinesShown() const
{
	return m_bShowLines;
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::SetLineWidth(GLfloat fWidth)
{
	m_fLineWidth = fWidth;

#ifdef _LINUX
    m_pWnd->Refresh(false);
#else
    m_pWnd->RedrawWindow();
#endif // _LINUX
}

// ------------------------------------------------------------------------------------------------
GLfloat COpenGLSTEPView::GetLineWidth() const
{
	return m_fLineWidth;
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::ShowPoints(BOOL bShow)
{
	m_bShowPoints = bShow;

#ifdef _LINUX
    m_pWnd->Refresh(false);
#else
    m_pWnd->RedrawWindow();
#endif // _LINUX
}

// ------------------------------------------------------------------------------------------------
BOOL COpenGLSTEPView::ArePointsShown() const
{
	return m_bShowPoints;
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::SetPointSize(GLfloat fSize)
{
	m_fPointSize = fSize;

#ifdef _LINUX
    m_pWnd->Refresh(false);
#else
    m_pWnd->RedrawWindow();
#endif // _LINUX
}

// ------------------------------------------------------------------------------------------------
GLfloat COpenGLSTEPView::GetPointSize() const
{
	return m_fPointSize;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void COpenGLSTEPView::Load()
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
	m_pPointedInstance = NULL;
	m_pSelectedInstance = NULL;

	CSTEPController* pController = GetController();
	ASSERT(pController != NULL);

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
	const map<int_t, CProductDefinition*>& mapProductDefinitions = pModel->getProductDefinitions();

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

	for (auto itProductDefinitions = mapProductDefinitions.begin(); itProductDefinitions != mapProductDefinitions.end(); itProductDefinitions++)
	{
		auto pProductDefinition = itProductDefinitions->second;
		if (pProductDefinition->getVerticesCount() == 0)
		{
			continue;
		}

		/******************************************************************************************
		* Geometry
		*/

		/**
		* VBO - Conceptual faces, polygons, etc.
		*/
		if (((int_t)iVerticesCount + pProductDefinition->getVerticesCount()) > (int_t)VERTICES_MAX_COUNT)
		{
			if (m_oglBuffers.createInstancesCohort(vecProductDefinitionsCohort, m_pOGLProgram) != iVerticesCount)
			{
				assert(false);

				return;
			}

			iVerticesCount = 0;
			vecProductDefinitionsCohort.clear();
		}

		/*
		* IBO - Conceptual faces
		*/
		for (size_t iFacesCohort = 0; iFacesCohort < pProductDefinition->concFacesCohorts().size(); iFacesCohort++)
		{
			if ((int_t)(iConcFacesIndicesCount + pProductDefinition->concFacesCohorts()[iFacesCohort]->indices().size()) > (int_t)INDICES_MAX_COUNT)
			{
				if (m_oglBuffers.createIBO(vecConcFacesCohorts) != iConcFacesIndicesCount)
				{
					assert(false);

					return;
				}

				iConcFacesIndicesCount = 0;
				vecConcFacesCohorts.clear();
			}

			iConcFacesIndicesCount += (GLsizei)pProductDefinition->concFacesCohorts()[iFacesCohort]->indices().size();
			vecConcFacesCohorts.push_back(pProductDefinition->concFacesCohorts()[iFacesCohort]);
		}

		/*
		* IBO - Conceptual face polygons
		*/
		for (size_t iConcFacePolygonsCohort = 0; iConcFacePolygonsCohort < pProductDefinition->concFacePolygonsCohorts().size(); iConcFacePolygonsCohort++)
		{
			if ((int_t)(iConcFacePolygonsIndicesCount + pProductDefinition->concFacePolygonsCohorts()[iConcFacePolygonsCohort]->indices().size()) > (int_t)INDICES_MAX_COUNT)
			{
				if (m_oglBuffers.createIBO(vecConcFacePolygonsCohorts) != iConcFacePolygonsIndicesCount)
				{
					assert(false);

					return;
				}

				iConcFacePolygonsIndicesCount = 0;
				vecConcFacePolygonsCohorts.clear();
			}

			iConcFacePolygonsIndicesCount += (GLsizei)pProductDefinition->concFacePolygonsCohorts()[iConcFacePolygonsCohort]->indices().size();
			vecConcFacePolygonsCohorts.push_back(pProductDefinition->concFacePolygonsCohorts()[iConcFacePolygonsCohort]);
		}

		/*
		* IBO - Lines
		*/
		for (size_t iLinesCohort = 0; iLinesCohort < pProductDefinition->linesCohorts().size(); iLinesCohort++)
		{
			if ((int_t)(iLinesIndicesCount + pProductDefinition->linesCohorts()[iLinesCohort]->indices().size()) > (int_t)INDICES_MAX_COUNT)
			{
				if (m_oglBuffers.createIBO(vecLinesCohorts) != iLinesIndicesCount)
				{
					assert(false);

					return;
				}

				iLinesIndicesCount = 0;
				vecLinesCohorts.clear();
			}

			iLinesIndicesCount += (GLsizei)pProductDefinition->linesCohorts()[iLinesCohort]->indices().size();
			vecLinesCohorts.push_back(pProductDefinition->linesCohorts()[iLinesCohort]);
		}

		/*
		* IBO - Points
		*/
		for (size_t iPointsCohort = 0; iPointsCohort < pProductDefinition->pointsCohorts().size(); iPointsCohort++)
		{
			if ((int_t)(iPointsIndicesCount + pProductDefinition->pointsCohorts()[iPointsCohort]->indices().size()) > (int_t)INDICES_MAX_COUNT)
			{
				if (m_oglBuffers.createIBO(vecPointsCohorts) != iPointsIndicesCount)
				{
					assert(false);

					return;
				}

				iPointsIndicesCount = 0;
				vecPointsCohorts.clear();
			}

			iPointsIndicesCount += (GLsizei)pProductDefinition->pointsCohorts()[iPointsCohort]->indices().size();
			vecPointsCohorts.push_back(pProductDefinition->pointsCohorts()[iPointsCohort]);
		}

		iVerticesCount += (GLsizei)pProductDefinition->getVerticesCount();
		vecProductDefinitionsCohort.push_back(pProductDefinition);
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
			assert(false);

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
#ifdef _LINUX
void COpenGLSTEPView::Draw(wxPaintDC * pDC)
#else
/*virtual*/ void COpenGLSTEPView::Draw(CDC * pDC)
#endif // _LINUX
{
	VERIFY(pDC != NULL);

	CSTEPController* pController = GetController();
	ASSERT(pController != NULL);

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

#ifdef _ENABLE_OPENGL_DEBUG
	m_pOGLContext->enableDebug();
#endif

	CRect rcClient;
	m_pWnd->GetClientRect(&rcClient);

	iWidth = rcClient.Width();
	iHeight = rcClient.Height();
#endif // _LINUX

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
	GLdouble zNear = 0.001;
	GLdouble zFar = 1000000.0;

	GLdouble fH = tan(fovY / 360 * M_PI) * zNear;
	GLdouble fW = fH * aspect;

	glm::mat4 matProjection = glm::frustum<GLdouble>(-fW, fW, -fH, fH, zNear, zFar);
	m_pOGLProgram->setProjectionMatrix(matProjection);

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
	//DrawPoints();

	/*
	* Scene
	*/
	//DrawCoordinateSystem();

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
	//DrawInstancesFrameBuffer();
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void COpenGLSTEPView::OnMouseEvent(enumMouseEvent enEvent, UINT nFlags, CPoint point)
{
	if (enEvent == meLBtnUp)
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
		case meMove:
		{
			OnMouseMoveEvent(nFlags, point);
		}
		break;

		case meLBtnDown:
		case meMBtnDown:
		case meRBtnDown:
		{
			m_ptStartMousePosition = point;
			m_ptPrevMousePosition = point;
		}
		break;

		case meLBtnUp:
		case meMBtnUp:
		case meRBtnUp:
		{
			m_ptStartMousePosition.x = -1;
			m_ptStartMousePosition.y = -1;
			m_ptPrevMousePosition.x = -1;
			m_ptPrevMousePosition.y = -1;

			if (m_bDisableSelectionBuffer)
			{
				m_bDisableSelectionBuffer = FALSE;

				DrawInstancesFrameBuffer();
			}
		}
		break;

		default:
			ASSERT(FALSE);
			break;
	} // switch (enEvent)
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void COpenGLSTEPView::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	CSTEPController* pController = GetController();
	ASSERT(pController != NULL);

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

	auto pSelectedInstance = GetController()->GetSelectedInstance() != nullptr ? dynamic_cast<CProductInstance*>(GetController()->GetSelectedInstance()) : nullptr;

	CMenu menu;
	VERIFY(menu.LoadMenuW(IDR_MENU_3D_VIEW));

	CMenu* pPopup = menu.GetSubMenu(0);

	pPopup->EnableMenuItem(ID_3DVIEW_ZOOMTO, MF_BYCOMMAND | (pSelectedInstance != nullptr ? MF_ENABLED : MF_DISABLED));

	pPopup->EnableMenuItem(ID_3DVIEW_ENABLE, MF_BYCOMMAND | (pSelectedInstance != nullptr ? MF_ENABLED : MF_DISABLED));
	pPopup->CheckMenuItem(ID_3DVIEW_ENABLE, MF_BYCOMMAND | ((pSelectedInstance != nullptr) && (pSelectedInstance->getEnable()) ? MF_CHECKED : MF_UNCHECKED));

	pPopup->EnableMenuItem(ID_3DVIEW_DISABLEALLBUTTHIS, MF_BYCOMMAND | (pSelectedInstance != nullptr ? MF_ENABLED : MF_DISABLED));

	pPopup->EnableMenuItem(ID_3DVIEW_ENABLEALL, MF_BYCOMMAND | (pSelectedInstance != nullptr ? MF_ENABLED : MF_DISABLED));

	pPopup->EnableMenuItem(ID_3DVIEW_SAVE, MF_BYCOMMAND | (pSelectedInstance != nullptr ? MF_ENABLED : MF_DISABLED));

	UINT uiCommand = pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, point.x, point.y, m_pWnd);
	if (uiCommand == 0)
	{
		return;
	}

	switch (uiCommand)
	{
		case ID_VIEW_RESET:
		{
			Reset();
		}
		break;

		case ID_3DVIEW_ZOOMTO:
		{
			pController->ZoomToInstance(pSelectedInstance->getID());
		}
		break;

		case ID_3DVIEW_ENABLE:
		{
			pSelectedInstance->setEnable(!pSelectedInstance->getEnable());

			pController->OnInstanceEnabledStateChanged(nullptr, pSelectedInstance);
		}
		break;

		case ID_3DVIEW_DISABLEALLBUTTHIS:
		{
			const map<int_t, CProductInstance*>& mapProductInstances = pModel->getProductInstances();

			CProductInstance* pProductInstance = nullptr;
			map<int_t, CProductInstance*>::const_iterator itProductInstance = mapProductInstances.begin();
			for (; itProductInstance != mapProductInstances.end(); itProductInstance++)
			{
				if (itProductInstance->second == pSelectedInstance)
				{
					itProductInstance->second->setEnable(true);

					pProductInstance = itProductInstance->second;

					continue;
				}

				itProductInstance->second->setEnable(false);
			}

			pController->OnDisableAllButThis(nullptr, pProductInstance);
		}
		break;

		case ID_3DVIEW_ENABLEALL:
		{
			const map<int_t, CProductInstance*>& mapProductInstances = pModel->getProductInstances();

			map<int_t, CProductInstance*>::const_iterator itProductInstance = mapProductInstances.begin();
			for (; itProductInstance != mapProductInstances.end(); itProductInstance++)
			{
				itProductInstance->second->setEnable(true);
			}

			pController->OnEnableAllInstances(nullptr);
		}
		break;

		case ID_3DVIEW_SAVE:
		{
			TCHAR szFilters[] = _T("BIN Files (*.bin)|*.bin|All Files (*.*)|*.*||");

			CFileDialog dlgFile(FALSE, _T("bin"), pSelectedInstance->getProductDefinition()->getId(),
				OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, szFilters);

			if (dlgFile.DoModal() != IDOK)
			{
				return;
			}

			SaveInstanceTreeW(pSelectedInstance->getProductDefinition()->getInstance(), dlgFile.GetPathName());
		}
		break;

		default:
		{
			ASSERT(FALSE); // Unknown
		}
		break;
	} // switch (uiCommand)
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void COpenGLSTEPView::OnWorldDimensionsChanged()
{
	CSTEPController* pController = GetController();
	ASSERT(pController != NULL);

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
	m_pWnd->RedrawWindow();
#endif // _LINUX
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void COpenGLSTEPView::Reset()
{
	_reset();

	m_bShowFaces = TRUE;
	m_bShowFacesPolygons = FALSE;
	m_bShowConceptualFacesPolygons = TRUE;
	m_bShowLines = TRUE;
	m_bShowPoints = TRUE;

#ifdef _LINUX
    m_pWnd->Refresh(false);
#else
    m_pWnd->RedrawWindow();
#endif // _LINUX
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void COpenGLSTEPView::OnInstanceSelected(CSTEPView * pSender)
{
	if (pSender == this)
	{
		return;
	}

	ASSERT(GetController() != NULL);

	auto pSelectedInstance = GetController()->GetSelectedInstance() != nullptr ? dynamic_cast<CProductInstance*>(GetController()->GetSelectedInstance()) : nullptr;
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
/*virtual*/ void COpenGLSTEPView::OnInstancePropertySelected()
{
	ASSERT(0); // todo
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void COpenGLSTEPView::OnInstancesEnabledStateChanged(CSTEPView* pSender)
{
	if (pSender == this)
	{
		return;
	}

	/*
	* Restore the selection
	*/
	OnInstanceSelected(NULL);

#ifdef _LINUX
	m_pWnd->Refresh(false);
#else
	m_pWnd->RedrawWindow();
#endif // _LINUX
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void COpenGLSTEPView::OnInstanceEnabledStateChanged(CSTEPView* pSender, CProductInstance* /*pProductInstance*/)
{
	OnInstancesEnabledStateChanged(pSender);
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void COpenGLSTEPView::OnDisableAllButThis(CSTEPView* pSender, CProductInstance* /*pProductInstance*/)
{
	OnInstancesEnabledStateChanged(pSender);
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void COpenGLSTEPView::OnEnableAllInstances(CSTEPView* pSender)
{
	OnInstancesEnabledStateChanged(pSender);
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void COpenGLSTEPView::OnControllerChanged()
{
	ASSERT(GetController() != NULL);

	GetController()->RegisterView(this);
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::DrawClipSpace()
{
	/*CSTEPController* pController = GetController();
	ASSERT(pController != NULL);

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

	glDisable(GL_LIGHTING);

	glLineWidth(m_fLineWidth);
	glColor3f(0.0f, 0.0f, 0.0f);

	float fXmin = -1.f;
	float fXmax = 1.f;
	float fYmin = -1.f;
	float fYmax = 1.f;
	float fZmin = -1.f;
	float fZmax = 1.f;
	if (pModel != NULL)
	{
		pModel->GetWorldDimensions(fXmin, fXmax, fYmin, fYmax, fZmin, fZmax);
	}

	glBegin(GL_LINE_LOOP);
	glVertex3f(fXmax, fYmax, fZmax);
	glVertex3f(fXmin, fYmax, fZmax);
	glVertex3f(fXmin, fYmin, fZmax);
	glVertex3f(fXmax, fYmin, fZmax);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex3f(fXmin, fYmin, fZmin);
	glVertex3f(fXmin, fYmax, fZmin);
	glVertex3f(fXmax, fYmax, fZmin);
	glVertex3f(fXmax, fYmin, fZmin);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex3f(fXmax, fYmax, fZmax);
	glVertex3f(fXmax, fYmax, fZmin);
	glVertex3f(fXmin, fYmax, fZmin);
	glVertex3f(fXmin, fYmax, fZmax);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex3f(fXmin, fYmin, fZmin);
	glVertex3f(fXmax, fYmin, fZmin);
	glVertex3f(fXmax, fYmin, fZmax);
	glVertex3f(fXmin, fYmin, fZmax);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex3f(fXmax, fYmax, fZmax);
	glVertex3f(fXmax, fYmin, fZmax);
	glVertex3f(fXmax, fYmin, fZmin);
	glVertex3f(fXmax, fYmax, fZmin);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex3f(fXmin, fYmin, fZmin);
	glVertex3f(fXmin, fYmin, fZmax);
	glVertex3f(fXmin, fYmax, fZmax);
	glVertex3f(fXmin, fYmax, fZmin);
	glEnd();

	glEnable(GL_LIGHTING);

	_oglUtils::checkForErrors();*/
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::DrawCoordinateSystem()
{
	//const float ARROW_SIZE = 1.5f;

	//glProgramUniform1f(
	//	m_pProgram->GetID(),
	//	m_pProgram->geUseBinnPhongModel(),
	//	0.f);

	//glProgramUniformMatrix4fv(
	//	m_pProgram->GetID(),
	//	m_pProgram->getMVMatrix(),
	//	1, 
	//	false, 
	//	glm::value_ptr(m_modelViewMatrix));
	//
	//glProgramUniform1f(
	//	m_pProgram->GetID(),
	//	m_pProgram->getTransparency(),
	//	1.f);

	//// X axis
	//glProgramUniform3f(
	//	m_pProgram->GetID(),
	//	m_pProgram->getMaterialAmbientColor(),
	//	1.f, 
	//	0.f, 
	//	0.f);

	//glBegin(GL_LINES);
	//glVertex3d(0., 0., 0.);
	//glVertex3d(0. + ARROW_SIZE, 0., 0.);
	//glEnd();
	//DrawTextGDI(L"X", ARROW_SIZE + (ARROW_SIZE * 0.05f), 0.f, 0.f);

	//// Y axis
	//glProgramUniform3f(
	//	m_pProgram->GetID(),
	//	m_pProgram->getMaterialAmbientColor(),
	//	0.f, 
	//	1.f, 
	//	0.f);

	//glBegin(GL_LINES);
	//glVertex3d(0., 0., 0.);
	//glVertex3d(0., 0. + ARROW_SIZE, 0.);
	//glEnd();
	//DrawTextGDI(L"Y", 0.f, ARROW_SIZE + (ARROW_SIZE * 0.05f), 0.f);

	//// Z axis
	//glProgramUniform3f(
	//	m_pProgram->GetID(),
	//	m_pProgram->getMaterialAmbientColor(),
	//	0.f, 
	//	0.f, 
	//	1.f);

	//glBegin(GL_LINES);
	//glVertex3d(0., 0., 0.);
	//glVertex3d(0., 0., 0. + ARROW_SIZE);
	//glEnd();
	//DrawTextGDI(L"Z", 0.f, 0.f, ARROW_SIZE + (ARROW_SIZE * 0.05f));

	//_oglUtils::checkForErrors();
}

// ------------------------------------------------------------------------------------------------
// https://docs.microsoft.com/en-us/windows/desktop/api/wingdi/nf-wingdi-wglusefontbitmapsa
bool COpenGLSTEPView::DrawTextGDI(const wchar_t* szText, float fX, float fY, float fZ)
{
	if (m_hFont == NULL)
	{
		ASSERT(FALSE);

		return false;
	}

	if ((szText == NULL) || (wcslen(szText) == 0))
	{
		ASSERT(FALSE);

		return false;
	}

	// DC
	CDC* pDC = m_pWnd->GetDC();

	// select the font
	HANDLE hOldFont = SelectObject(pDC->GetSafeHdc(), m_hFont);

	// create the bitmap display lists  
	// we're making images of glyphs 0 thru 254  
	// the display list numbering starts at 1000, an arbitrary choice  
	wglUseFontBitmaps(pDC->GetSafeHdc(), 0, 255, 1000);

	glRasterPos3f(fX, fY, fZ); // set start position

	// display a string:  
	// indicate start of glyph display lists  
	glListBase(1000);

	// now draw the characters in a string  
	glCallLists((GLsizei)wcslen(szText), GL_UNSIGNED_SHORT, szText);

	// restore the font
	SelectObject(pDC->GetSafeHdc(), hOldFont);

	m_pWnd->ReleaseDC(pDC);

	return true;
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::DrawFaces(bool bTransparent)
{
	if (!m_bShowFaces)
	{
		return;
	}

	CSTEPController* pController = GetController();
	ASSERT(pController != NULL);

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

	if (pModel->getProductDefinitions().empty())
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

	m_pOGLProgram->enableBinnPhongModel(true);

	float fXTranslation = 0.f;
	float fYTranslation = 0.f;
	float fZTranslation = 0.f;
	pModel->GetWorldTranslations(fXTranslation, fYTranslation, fZTranslation);

	for (auto itCohort : m_oglBuffers.instancesCohorts())
	{
		glBindVertexArray(itCohort.first);

		for (auto pProductDefinition : itCohort.second)
		{
			if (pProductDefinition->concFacesCohorts().empty())
			{
				continue;
			}

			auto vecProductInstances = pProductDefinition->getProductInstances();
			for (size_t iInstance = 0; iInstance < vecProductInstances.size(); iInstance++)
			{
				auto pProductInstance = vecProductInstances[iInstance];
				if (!pProductInstance->getEnable())
				{
					continue;
				}

				/*
				* Transformation Matrix
				*/
				glm::mat4 matTransformation = glm::make_mat4((GLdouble*)pProductInstance->getTransformationMatrix());

				/*
				* Model-View Matrix
				*/
				glm::mat4 matModelView = m_matModelView;
				matModelView = glm::translate(matModelView, glm::vec3(fXTranslation, fYTranslation, fZTranslation));
				matModelView = matModelView * matTransformation;
				matModelView = glm::translate(matModelView, glm::vec3(-fXTranslation, -fYTranslation, -fZTranslation));

				m_pOGLProgram->setModelViewMatrix(matModelView);

				for (size_t iCohort = 0; iCohort < pProductDefinition->concFacesCohorts().size(); iCohort++)
				{
					auto pCohort = pProductDefinition->concFacesCohorts()[iCohort];

					const _material* pMaterial =
						pProductInstance == m_pSelectedInstance ? m_pSelectedInstanceMaterial :
						pProductInstance == m_pPointedInstance ? m_pPointedInstanceMaterial :
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

					m_pOGLProgram->setMaterial(pMaterial);

					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pCohort->ibo());
					glDrawElementsBaseVertex(GL_TRIANGLES,
						(GLsizei)pCohort->indices().size(),
						GL_UNSIGNED_INT,
						(void*)(sizeof(GLuint) * pCohort->iboOffset()),
						pProductDefinition->VBOOffset());
				}
			} // for (size_t iInstance = ...			
		} // for (auto pProductDefinition ...

		glBindVertexArray(0);
	} // for (auto itCohort ...

	if (bTransparent)
	{
		glDisable(GL_BLEND);
	}

	// Restore Model-View Matrix
	m_pOGLProgram->setModelViewMatrix(m_matModelView);

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

	CSTEPController* pController = GetController();
	ASSERT(pController != NULL);

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

	if (pModel->getProductDefinitions().empty())
	{
		return;
	}

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	m_pOGLProgram->enableBinnPhongModel(false);
	m_pOGLProgram->setAmbientColor(0.f, 0.f, 0.f);
	m_pOGLProgram->setTransparency(1.f);

	float fXTranslation = 0.f;
	float fYTranslation = 0.f;
	float fZTranslation = 0.f;
	pModel->GetWorldTranslations(fXTranslation, fYTranslation, fZTranslation);	

	for (auto itCohort : m_oglBuffers.instancesCohorts())
	{
		glBindVertexArray(itCohort.first);

		for (auto pProductDefinition : itCohort.second)
		{
			if (pProductDefinition->concFacePolygonsCohorts().empty())
			{
				continue;
			}

			auto vecProductInstances = pProductDefinition->getProductInstances();
			for (size_t iInstance = 0; iInstance < vecProductInstances.size(); iInstance++)
			{
				auto pProductInstance = vecProductInstances[iInstance];
				if (!pProductInstance->getEnable())
				{
					continue;
				}

				/*
				* Transformation Matrix
				*/
				glm::mat4 matTransformation = glm::make_mat4((GLdouble*)pProductInstance->getTransformationMatrix());

				/*
				* Model-View Matrix
				*/
				glm::mat4 matModelView = m_matModelView;
				matModelView = glm::translate(matModelView, glm::vec3(fXTranslation, fYTranslation, fZTranslation));
				matModelView = matModelView * matTransformation;
				matModelView = glm::translate(matModelView, glm::vec3(-fXTranslation, -fYTranslation, -fZTranslation));

				m_pOGLProgram->setModelViewMatrix(matModelView);
				
				for (size_t iCohort = 0; iCohort < pProductDefinition->concFacePolygonsCohorts().size(); iCohort++)
				{
					_cohort* pCohort = pProductDefinition->concFacePolygonsCohorts()[iCohort];

					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pCohort->ibo());
					glDrawElementsBaseVertex(GL_LINES,
						(GLsizei)pCohort->indices().size(),
						GL_UNSIGNED_INT,
						(void*)(sizeof(GLuint) * pCohort->iboOffset()),
						pProductDefinition->VBOOffset());
				}
			} // for (size_t iInstance = ...			
		} // for (auto pProductDefinition ...

		glBindVertexArray(0);
	} // for (auto itCohort ...

	// Restore Model-View Matrix
	m_pOGLProgram->setModelViewMatrix(m_matModelView);

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

	CSTEPController* pController = GetController();
	ASSERT(pController != NULL);

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

	if (pModel->getProductDefinitions().empty())
	{
		return;
	}

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	m_pOGLProgram->enableBinnPhongModel(false);
	m_pOGLProgram->setAmbientColor(0.f, 0.f, 0.f);
	m_pOGLProgram->setTransparency(1.f);

	float fXTranslation = 0.f;
	float fYTranslation = 0.f;
	float fZTranslation = 0.f;
	pModel->GetWorldTranslations(fXTranslation, fYTranslation, fZTranslation);

	for (auto itCohort : m_oglBuffers.instancesCohorts())
	{
		glBindVertexArray(itCohort.first);

		for (auto pProductDefinition : itCohort.second)
		{
			if (pProductDefinition->linesCohorts().empty())
			{
				continue;
			}

			auto vecProductInstances = pProductDefinition->getProductInstances();
			for (size_t iInstance = 0; iInstance < vecProductInstances.size(); iInstance++)
			{
				auto pProductInstance = vecProductInstances[iInstance];
				if (!pProductInstance->getEnable())
				{
					continue;
				}

				/*
				* Transformation Matrix
				*/
				glm::mat4 matTransformation = glm::make_mat4((GLdouble*)pProductInstance->getTransformationMatrix());

				/*
				* Model-View Matrix
				*/
				glm::mat4 matModelView = m_matModelView;
				matModelView = glm::translate(matModelView, glm::vec3(fXTranslation, fYTranslation, fZTranslation));
				matModelView = matModelView * matTransformation;
				matModelView = glm::translate(matModelView, glm::vec3(-fXTranslation, -fYTranslation, -fZTranslation));

				m_pOGLProgram->setModelViewMatrix(matModelView);

				for (size_t iCohort = 0; iCohort < pProductDefinition->linesCohorts().size(); iCohort++)
				{
					_cohort* pCohort = pProductDefinition->linesCohorts()[iCohort];

					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pCohort->ibo());
					glDrawElementsBaseVertex(GL_LINES,
						(GLsizei)pCohort->indices().size(),
						GL_UNSIGNED_INT,
						(void*)(sizeof(GLuint) * pCohort->iboOffset()),
						pProductDefinition->VBOOffset());
				}
			} // for (size_t iInstance = ...			
		} // for (auto pProductDefinition ...

		glBindVertexArray(0);
	} // for (auto itCohort ...

	// Restore Model-View Matrix
	m_pOGLProgram->setModelViewMatrix(m_matModelView);

	_oglUtils::checkForErrors();

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	TRACE(L"\n*** DrawLines() : %lld [탎]", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::DrawPoints()
{
	//if (!m_bShowPoints)
	//{
	//	return;
	//}

	//CSTEPController* pController = GetController();
	//ASSERT(pController != NULL);

	//if (pController->GetModel() == nullptr)
	//{
	//	ASSERT(FALSE);

	//	return;
	//}

	//auto pModel = pController->GetModel()->As<CSTEPModel>();
	//if (pModel == nullptr)
	//{
	//	ASSERT(FALSE);

	//	return;
	//}

	//if (pModel->getProductDefinitions().empty())
	//{
	//	return;
	//}

	//std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	//float fXTranslation = 0.f;
	//float fYTranslation = 0.f;
	//float fZTranslation = 0.f;
	//pModel->GetWorldTranslations(fXTranslation, fYTranslation, fZTranslation);

	//glProgramUniform1f(
	//	m_pProgram->GetID(),
	//	m_pProgram->geUseBinnPhongModel(),
	//	0.f);

	//glProgramUniform3f(
	//	m_pProgram->GetID(),
	//	m_pProgram->getMaterialAmbientColor(),
	//	0.f, 
	//	0.f, 
	//	0.f);

	//glProgramUniform1f(
	//	m_pProgram->GetID(),
	//	m_pProgram->getTransparency(),
	//	1.f);

	//for (size_t iDrawMetaData = 0; iDrawMetaData < m_veCSTEPDrawMetaData.size(); iDrawMetaData++)
	//{
	//	if (m_veCSTEPDrawMetaData[iDrawMetaData]->GetType() != mdtGeometry)
	//	{
	//		continue;
	//	}

	//	const map<GLuint, vector<CProductDefinition*>>& mapGroups = m_veCSTEPDrawMetaData[iDrawMetaData]->getVBOGroups();

	//	map<GLuint, vector<CProductDefinition*>>::const_iterator itGroups = mapGroups.begin();
	//	for (; itGroups != mapGroups.end(); itGroups++)
	//	{
	//		bool bBindVBO = true;			

	//		for (size_t iObject = 0; iObject < itGroups->second.size(); iObject++)
	//		{
	//			CProductDefinition* pProductDefinition = itGroups->second[iObject];

	//			const vector<CProductInstance*>& vecProductInstances = pProductDefinition->getProductInstances();
	//			for (size_t iInstance = 0; iInstance < vecProductInstances.size(); iInstance++)
	//			{
	//				auto pProductInstance = vecProductInstances[iInstance];

	//				if (!pProductInstance->getEnable())
	//				{
	//					continue;
	//				}

	//				if (pProductDefinition->pointsCohorts().empty())
	//				{
	//					continue;
	//				}

	//				if (bBindVBO)
	//				{
	//					glBindBuffer(GL_ARRAY_BUFFER, itGroups->first);
	//					glVertexAttribPointer(m_pProgram->getVertexPosition(), 3, GL_FLOAT, false, sizeof(GLfloat) * GEOMETRY_VBO_VERTEX_LENGTH, 0);
	//					glEnableVertexAttribArray(m_pProgram->getVertexPosition());

	//					bBindVBO = false;
	//				}

	//				/*
	//				* Transformation Matrix
	//				*/
	//				glm::mat4 transformationMatrix = glm::make_mat4((GLdouble*)pProductInstance->getTransformationMatrix());

	//				/*
	//				* Model-View Matrix
	//				*/
	//				glm::mat4 modelViewMatrix = m_modelViewMatrix;
	//				modelViewMatrix = glm::translate(modelViewMatrix, glm::vec3(fXTranslation, fYTranslation, fZTranslation));
	//				modelViewMatrix = modelViewMatrix * transformationMatrix;
	//				modelViewMatrix = glm::translate(modelViewMatrix, glm::vec3(-fXTranslation, -fYTranslation, -fZTranslation));

	//				glProgramUniformMatrix4fv(
	//					m_pProgram->GetID(), 
	//					m_pProgram->getMVMatrix(),
	//					1, 
	//					false, 
	//					glm::value_ptr(modelViewMatrix));

	//				/*
	//				* Points
	//				*/
	//				GLuint iCurrentIBO = 0;
	//				for (size_t iPointsCohort = 0; iPointsCohort < pProductDefinition->pointsCohorts().size(); iPointsCohort++)
	//				{
	//					CPointsCohort* pPointsCohort = pProductDefinition->pointsCohorts()[iPointsCohort];

	//					if (iCurrentIBO != pPointsCohort->IBO())
	//					{
	//						glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pPointsCohort->IBO());

	//						iCurrentIBO = pPointsCohort->IBO();
	//					}						

	//					glDrawElementsBaseVertex(GL_POINTS,
	//						(GLsizei)pPointsCohort->getIndicesCount(),
	//						GL_UNSIGNED_INT,
	//						(void*)(sizeof(GLuint) * pPointsCohort->IBOOffset()),
	//						pProductDefinition->VBOOffset());

	//					//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//				} // for (size_t iPointsCohort = ...
	//			} // for (size_t iInstance = ...
	//		} // for (size_t iObject = ...

	//		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//		glBindBuffer(GL_ARRAY_BUFFER, 0);
	//	} // for (; itGroups != ...
	//} // for (size_t iDrawMetaData = ...

	//_oglUtils::checkForErrors();

	//std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	//TRACE(L"\n*** DrawPoints() : %lld [탎]", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::DrawInstancesFrameBuffer()
{
//	if (m_bDisableSelectionBuffer)
//	{
//		return;
//	}
//
//	CSTEPController * pController = GetController();
//	ASSERT(pController != NULL);
//
//	if (pController->GetModel() == nullptr)
//	{
//		ASSERT(FALSE);
//
//		return;
//	}
//
//	auto pModel = pController->GetModel()->As<CSTEPModel>();
//	if (pModel == nullptr)
//	{
//		ASSERT(FALSE);
//
//		return;
//	}
//
//	if (pModel->getProductDefinitions().empty())
//	{
//		return;
//	}
//
//	/*
//	* Create a frame buffer
//	*/
//
//	int iWidth = 0;
//	int iHeight = 0;
//
//#ifdef _LINUX
//	const wxSize szClient = m_pWnd->GetClientSize();
//
//	iWidth = szClient.GetWidth();
//	iHeight = szClient.GetHeight();
//#else
//	CRect rcClient;
//	m_pWnd->GetClientRect(&rcClient);
//
//	iWidth = rcClient.Width();
//	iHeight = rcClient.Height();
//#endif // _LINUX
//
//	if ((iWidth < 20) || (iHeight < 20))
//	{
//		return;
//	}
//
//	BOOL bResult = m_pOGLContext->MakeCurrent();
//	VERIFY(bResult);
//
//	if (m_iInstanceSelectionFrameBuffer == 0)
//	{
//		ASSERT(m_iInstanceSelectionTextureBuffer == 0);
//		ASSERT(m_iInstanceSelectionDepthRenderBuffer == 0);
//
//		/*
//		* Frame buffer
//		*/
//		glGenFramebuffers(1, &m_iInstanceSelectionFrameBuffer);
//		ASSERT(m_iInstanceSelectionFrameBuffer != 0);
//
//		glBindFramebuffer(GL_FRAMEBUFFER, m_iInstanceSelectionFrameBuffer);
//
//		/*
//		* Texture buffer
//		*/
//		glGenTextures(1, &m_iInstanceSelectionTextureBuffer);
//		ASSERT(m_iInstanceSelectionTextureBuffer != 0);
//
//		glBindTexture(GL_TEXTURE_2D, m_iInstanceSelectionTextureBuffer);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//
//		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SELECTION_BUFFER_SIZE, SELECTION_BUFFER_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
//
//		glBindTexture(GL_TEXTURE_2D, 0);
//
//		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_iInstanceSelectionTextureBuffer, 0);
//
//		/*
//		* Depth buffer
//		*/
//		glGenRenderbuffers(1, &m_iInstanceSelectionDepthRenderBuffer);
//		ASSERT(m_iInstanceSelectionDepthRenderBuffer != 0);
//
//		glBindRenderbuffer(GL_RENDERBUFFER, m_iInstanceSelectionDepthRenderBuffer);
//		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SELECTION_BUFFER_SIZE, SELECTION_BUFFER_SIZE);
//
//		glBindRenderbuffer(GL_RENDERBUFFER, 0);
//
//		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_iInstanceSelectionDepthRenderBuffer);
//
//		GLenum arDrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
//		glDrawBuffers(1, arDrawBuffers);
//
//		glBindFramebuffer(GL_FRAMEBUFFER, 0);
//
//		_oglUtils::checkForErrors();
//	} // if (m_iInstanceSelectionFrameBuffer == 0)
//
//	/*
//	* Selection colors
//	*/
//	if (m_mapInstancesSelectionColors.empty())
//	{
//		const float STEP = 1.0f / 255.0f;
//
//		const map<int64_t, CProductInstance *> & mapProductInstances = pModel->getProductInstances();
//
//		map<int64_t, CProductInstance*>::const_iterator itProductInstances = mapProductInstances.begin();
//		for (; itProductInstances != mapProductInstances.end(); itProductInstances++)
//		{
//			CProductInstance* pProductInstance = itProductInstances->second;			
//
//			const vector<pair<int64_t, int64_t> > & vecTriangles = pProductInstance->getProductDefinition()->getTriangles();
//			if (vecTriangles.empty())
//			{
//				continue;
//			}
//
//			float fR = floorf((float)pProductInstance->getID() / (255.0f * 255.0f));
//			if (fR >= 1.0f)
//			{
//				fR *= STEP;
//			}
//
//			float fG = floorf((float)pProductInstance->getID() / 255.0f);
//			if (fG >= 1.0f)
//			{
//				fG *= STEP;
//			}
//
//			float fB = (float)(pProductInstance->getID() % 255);
//			fB *= STEP;
//
//			ASSERT(m_mapInstancesSelectionColors.find(pProductInstance->getID()) == m_mapInstancesSelectionColors.end());
//			m_mapInstancesSelectionColors[pProductInstance->getID()] = CSTEPColor(fR, fG, fB);
//		} // for (; itProductInstances != ...
//	} // if (m_mapInstancesSelectionColors.empty())
//
//	/*
//	* Draw
//	*/
//
//	glBindFramebuffer(GL_FRAMEBUFFER, m_iInstanceSelectionFrameBuffer);
//
//	glViewport(0, 0, SELECTION_BUFFER_SIZE, SELECTION_BUFFER_SIZE);
//
//	glClearColor(0.0, 0.0, 0.0, 0.0);
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//	// Set up the parameters
//	glEnable(GL_DEPTH_TEST);
//	glDepthFunc(GL_LEQUAL);
//
//	glProgramUniform1f(
//		m_pProgram->GetID(),
//		m_pProgram->geUseBinnPhongModel(),
//		0.f);
//
//	glProgramUniform1f(
//		m_pProgram->GetID(),
//		m_pProgram->getTransparency(),
//		1.f);
//
//	float fXTranslation = 0.f;
//	float fYTranslation = 0.f;
//	float fZTranslation = 0.f;
//	pModel->GetWorldTranslations(fXTranslation, fYTranslation, fZTranslation);
//
//	for (size_t iDrawMetaData = 0; iDrawMetaData < m_veCSTEPDrawMetaData.size(); iDrawMetaData++)
//	{
//		if (m_veCSTEPDrawMetaData[iDrawMetaData]->GetType() != mdtGeometry)
//		{
//			continue;
//		}
//
//		const map<GLuint, vector<CProductDefinition*>>& mapGroups = m_veCSTEPDrawMetaData[iDrawMetaData]->getVBOGroups();
//
//		map<GLuint, vector<CProductDefinition*>>::const_iterator itGroups = mapGroups.begin();
//		for (; itGroups != mapGroups.end(); itGroups++)
//		{
//			bool bBindVBO = true;			
//
//			for (size_t iObject = 0; iObject < itGroups->second.size(); iObject++)
//			{
//				CProductDefinition* pProductDefinition = itGroups->second[iObject];
//
//				const vector<CProductInstance*>& vecProductInstances = pProductDefinition->getProductInstances();
//				for (size_t iInstance = 0; iInstance < vecProductInstances.size(); iInstance++)
//				{
//					auto pProductInstance = vecProductInstances[iInstance];
//
//					if (!pProductInstance->getEnable())
//					{
//						continue;
//					}
//
//					if (pProductDefinition->conceptualFacesMaterials().empty())
//					{
//						continue;
//					}
//
//					if (bBindVBO)
//					{
//						glBindBuffer(GL_ARRAY_BUFFER, itGroups->first);
//						glVertexAttribPointer(m_pProgram->getVertexPosition(), 3, GL_FLOAT, false, sizeof(GLfloat)* GEOMETRY_VBO_VERTEX_LENGTH, 0);
//						glEnableVertexAttribArray(m_pProgram->getVertexPosition());
//
//						bBindVBO = false;
//					}
//
//					/*
//					* Transformation Matrix
//					*/
//					glm::mat4 transformationMatrix = glm::make_mat4((GLdouble*)pProductInstance->getTransformationMatrix());
//
//					/*
//					* Model-View Matrix
//					*/
//					glm::mat4 modelViewMatrix = m_modelViewMatrix;
//					modelViewMatrix = glm::translate(modelViewMatrix, glm::vec3(fXTranslation, fYTranslation, fZTranslation));
//					modelViewMatrix = modelViewMatrix * transformationMatrix;
//					modelViewMatrix = glm::translate(modelViewMatrix, glm::vec3(-fXTranslation, -fYTranslation, -fZTranslation));
//
//					glProgramUniformMatrix4fv(
//						m_pProgram->GetID(),
//						m_pProgram->getMVMatrix(),
//						1, 
//						false, 
//						glm::value_ptr(modelViewMatrix));
//
//					/*
//					* Ambient color
//					*/
//					map<int64_t, CSTEPColor>::iterator itSelectionColor = m_mapInstancesSelectionColors.find(pProductInstance->getID());
//					ASSERT(itSelectionColor != m_mapInstancesSelectionColors.end());
//
//					/*
//					* Material - Ambient color
//					*/
//					glProgramUniform3f(
//						m_pProgram->GetID(),
//						m_pProgram->getMaterialAmbientColor(),
//						itSelectionColor->second.R(),
//						itSelectionColor->second.G(),
//						itSelectionColor->second.B());
//
//					/*
//					* Conceptual faces
//					*/
//					GLuint iCurrentIBO = 0;
//					for (size_t iMaterial = 0; iMaterial < pProductDefinition->conceptualFacesMaterials().size(); iMaterial++)
//					{
//						CSTEPGeometryWithMaterial* pGeometryWithMaterial = pProductDefinition->conceptualFacesMaterials()[iMaterial];						
//
//						if (iCurrentIBO != pGeometryWithMaterial->IBO())
//						{
//							glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pGeometryWithMaterial->IBO());
//
//							iCurrentIBO = pGeometryWithMaterial->IBO();
//						}						
//
//						glDrawElementsBaseVertex(GL_TRIANGLES,
//							(GLsizei)pGeometryWithMaterial->getIndicesCount(),
//							GL_UNSIGNED_INT,
//							(void*)(sizeof(GLuint) * pGeometryWithMaterial->IBOOffset()),
//							pProductDefinition->VBOOffset());
//
//						//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);						
//					} // for (size_t iMaterial = ...
//				} // for (size_t iInstance = ...
//			} // for (size_t iObject = ...
//
//			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
//			glBindBuffer(GL_ARRAY_BUFFER, 0);
//		} // for (; itGroups != ...
//	} // for (size_t iDrawMetaData = ...
//
//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
//
//	_oglUtils::checkForErrors();
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::OnMouseMoveEvent(UINT nFlags, CPoint point)
{
	CSTEPController * pController = GetController();
	ASSERT(pController != NULL);

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

	/*
	* Selection
	*/
	if (((nFlags & MK_LBUTTON) != MK_LBUTTON) && ((nFlags & MK_MBUTTON) != MK_MBUTTON) && ((nFlags & MK_RBUTTON) != MK_RBUTTON))
	{
		/*
		* Select an instance
		*/
		if (!pModel->getProductDefinitions().empty())// TODO!!! && m_iInstanceSelectionFrameBuffer != 0)
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

			//TODO!!!
			/*double dX = (double)point.x * ((double)SELECTION_BUFFER_SIZE / (double)iWidth);
			double dY = ((double)iHeight - (double)point.y) * ((double)SELECTION_BUFFER_SIZE / (double)iHeight);

			glBindFramebuffer(GL_FRAMEBUFFER, m_iInstanceSelectionFrameBuffer);

			glReadPixels(
				(GLint)dX,
				(GLint)dY,
				1, 1,
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				arPixels);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);*/

			CProductInstance * pPointedInstance = NULL;

			if (arPixels[3] != 0)
			{
				int64_t iObjectID =
					(arPixels[0/*R*/] * (255 * 255)) +
					(arPixels[1/*G*/] * 255) +
					arPixels[2/*B*/];

				pPointedInstance = pModel->getProductInstanceByID(iObjectID);
				ASSERT(pPointedInstance != NULL);
			} // if (arPixels[3] != 0)

			if (m_pPointedInstance != pPointedInstance)
			{
				m_pPointedInstance = pPointedInstance;

#ifdef _LINUX
                m_pWnd->Refresh(false);
#else
                m_pWnd->RedrawWindow();
#endif // _LINUX
			}
		} // if (!pModel->getProductDefinitions().empty() && ...

		/*
		* Select a face
		*/
		//ASSERT(0); // todo
//		if ((m_iFaceSelectionFrameBuffer != 0) && (m_pSelectedInstance != NULL))
//		{
//			int iWidth = 0;
//			int iHeight = 0;
//
//#ifdef _LINUX
//			m_pOGLContext->SetCurrent(*m_pWnd);
//
//			const wxSize szClient = m_pWnd->GetClientSize();
//
//			iWidth = szClient.GetWidth();
//			iHeight = szClient.GetHeight();
//#else
//			BOOL bResult = m_pOGLContext->MakeCurrent();
//			VERIFY(bResult);
//
//			CRect rcClient;
//			m_pWnd->GetClientRect(&rcClient);
//
//			iWidth = rcClient.Width();
//			iHeight = rcClient.Height();
//#endif // _LINUX
//
//			GLubyte arPixels[4];
//			memset(arPixels, 0, sizeof(GLubyte) * 4);
//
//			double dX = (double)point.x * ((double)SELECTION_BUFFER_SIZE / (double)iWidth);
//			double dY = ((double)iHeight - (double)point.y) * ((double)SELECTION_BUFFER_SIZE / (double)iHeight);
//
//			glBindFramebuffer(GL_FRAMEBUFFER, m_iFaceSelectionFrameBuffer);
//
//			glReadPixels(
//				(GLint)dX,
//				(GLint)dY,
//				1, 1,
//				GL_RGBA,
//				GL_UNSIGNED_BYTE,
//				arPixels);
//
//			glBindFramebuffer(GL_FRAMEBUFFER, 0);
//
//			int64_t iPointedFace = -1;
//
//			if (arPixels[3] != 0)
//			{
//				int64_t iObjectID =
//					(arPixels[0/*R*/] * (255 * 255)) +
//					(arPixels[1/*G*/] * 255) +
//					arPixels[2/*B*/];
//
//				iPointedFace = iObjectID;
//				ASSERT(m_mapFacesSelectionColors.find(iPointedFace) != m_mapFacesSelectionColors.end());
//			} // if (arPixels[3] != 0)
//
//			if (m_iPointedFace != iPointedFace)
//			{
//				m_iPointedFace = iPointedFace;
//
//#ifdef _LINUX
//                m_pWnd->Refresh(false);
//#else
//                m_pWnd->RedrawWindow();
//#endif // _LINUX
//			}
//		} // if ((m_iFaceSelectionFrameBuffer != 0) && ...
	} // if (((nFlags & MK_LBUTTON) != MK_LBUTTON) && ...

	if (m_ptPrevMousePosition.x == -1)
	{
		return;
	}

	float fBoundingSphereDiameter = pModel->GetBoundingSphereDiameter();

	/*
	* Rotate
	*/
	if ((nFlags & MK_LBUTTON) == MK_LBUTTON)
	{
		float fXAngle = ((float)point.y - (float)m_ptPrevMousePosition.y);
		float fYAngle = ((float)point.x - (float)m_ptPrevMousePosition.x);

		const float ROTATE_SPEED = 0.075f;

		Rotate(fXAngle * ROTATE_SPEED, fYAngle * ROTATE_SPEED);

		m_ptPrevMousePosition = point;

		return;
	}

	/*
	* Zoom
	*/
	if ((nFlags & MK_MBUTTON) == MK_MBUTTON)
	{
		Zoom(point.y - m_ptPrevMousePosition.y > 0 ? -(fBoundingSphereDiameter * 0.05f) : (fBoundingSphereDiameter * 0.05f));

		m_ptPrevMousePosition = point;

		return;
	}

	/*
	* Move
	*/
	if ((nFlags & MK_RBUTTON) == MK_RBUTTON)
	{
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

		m_fXTranslation += (((float)point.x - (float)m_ptPrevMousePosition.x) / iWidth) * (2.f * fBoundingSphereDiameter);
		m_fYTranslation -= (((float)point.y - (float)m_ptPrevMousePosition.y) / iHeight) * (2.f * fBoundingSphereDiameter);

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
void COpenGLSTEPView::Rotate(float fXSpin, float fYSpin)
{
	m_fXAngle += fXSpin;
	if (m_fXAngle > 360.0)
	{
		m_fXAngle = m_fXAngle - 360.0f;
	}
	else
	{
		if (m_fXAngle < 0.0)
		{
			m_fXAngle = m_fXAngle + 360.0f;
		}
	}

	m_fYAngle += fYSpin;
	if (m_fYAngle > 360.0)
	{
		m_fYAngle = m_fYAngle - 360.0f;
	}
	else
	{
		if (m_fYAngle < 0.0)
		{
			m_fYAngle = m_fYAngle + 360.0f;
		}
	}

	m_bDisableSelectionBuffer = TRUE;

#ifdef _LINUX
    m_pWnd->Refresh(false);
#else
    m_pWnd->RedrawWindow();
#endif // _LINUX	
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::Zoom(float fZTranslation)
{
	m_fZTranslation += fZTranslation;

#ifdef _LINUX
    m_pWnd->Refresh(false);
#else
    m_pWnd->RedrawWindow();
#endif // _LINUX
}

