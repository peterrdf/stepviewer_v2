#include "stdafx.h"

#include "OpenGLSTEPView.h"
#include "Controller.h"
#include "STEPModel.h"
#include "_3DUtils.h"

#include "Resource.h"

#include <chrono>

// ************************************************************************************************
static const int MIN_VIEW_PORT_LENGTH = 100;

// ************************************************************************************************
COpenGLSTEPView::COpenGLSTEPView(CWnd* pWnd)
	: COpenGLView()	
	, m_ptStartMousePosition(-1, -1)
	, m_ptPrevMousePosition(-1, -1)
	, m_pInstanceSelectionFrameBuffer(new _oglSelectionFramebuffer())
	, m_pPointedInstance(nullptr)
	, m_pSelectedInstance(nullptr)
	, m_pSelectedInstanceMaterial(nullptr)
	, m_pPointedInstanceMaterial(nullptr)
{
	_oglRendererSettings::m_bShowLines = FALSE;
	_oglRendererSettings::m_bShowPoints = FALSE;

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
}

COpenGLSTEPView::~COpenGLSTEPView()
{
	GetController()->UnRegisterView(this);

	delete m_pInstanceSelectionFrameBuffer;

	_destroy();

	delete m_pSelectedInstanceMaterial;
	delete m_pPointedInstanceMaterial;
}

/*virtual*/ _controller* COpenGLSTEPView::getController() const /*override*/
{
	return GetController();
}

/*virtual*/ _model* COpenGLSTEPView::getModel() const /*override*/
{
	return GetController()->getModel();
}

/*virtual*/ void COpenGLSTEPView::saveSetting(const string& strName, const string& strValue) /*override*/
{
	GetController()->getSettingsStorage()->setSetting(strName, strValue);
}

/*virtual*/ string COpenGLSTEPView::loadSetting(const string& strName) /*override*/
{
	return GetController()->getSettingsStorage()->getSetting(strName);
}

/*virtual*/ void COpenGLSTEPView::_load() /*override*/
{
	BOOL bResult = m_pOGLContext->makeCurrent();
	VERIFY(bResult);

	// OpenGL buffers
	m_oglBuffers.clear();

	m_pInstanceSelectionFrameBuffer->encoding().clear();
	m_pPointedInstance = nullptr;
	m_pSelectedInstance = nullptr;

	auto pModel = GetModel<CSTEPModel>();
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

	m_fScaleFactor = pModel->GetBoundingSphereDiameter();

	// Limits
	GLsizei VERTICES_MAX_COUNT = _oglUtils::getVerticesCountLimit(GEOMETRY_VBO_VERTEX_LENGTH * sizeof(float));
	GLsizei INDICES_MAX_COUNT = _oglUtils::getIndicesCountLimit();

	// Data
	auto& mapDefinitions = pModel->GetDefinitions();

	// VBO
	GLuint iVerticesCount = 0;
	vector<_geometry*> vecProductDefinitionsCohort;

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
		if (pDefinition->getVerticesCount() == 0)
		{
			continue;
		}

		/******************************************************************************************
		* Geometry
		*/

		/**
		* VBO - Conceptual faces, polygons, etc.
		*/
		if (((int_t)iVerticesCount + pDefinition->getVerticesCount()) > (int_t)VERTICES_MAX_COUNT)
		{
			if (m_oglBuffers.createCohort(vecProductDefinitionsCohort, m_pOGLProgram) != iVerticesCount)
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
		for (size_t iFacesCohort = 0; iFacesCohort < pDefinition->concFacesCohorts().size(); iFacesCohort++)
		{
			if ((int_t)(iConcFacesIndicesCount + pDefinition->concFacesCohorts()[iFacesCohort]->indices().size()) > (int_t)INDICES_MAX_COUNT)
			{
				if (m_oglBuffers.createIBO(vecConcFacesCohorts) != iConcFacesIndicesCount)
				{
					ASSERT(FALSE);

					return;
				}

				iConcFacesIndicesCount = 0;
				vecConcFacesCohorts.clear();
			}

			iConcFacesIndicesCount += (GLsizei)pDefinition->concFacesCohorts()[iFacesCohort]->indices().size();
			vecConcFacesCohorts.push_back(pDefinition->concFacesCohorts()[iFacesCohort]);
		}

		/*
		* IBO - Conceptual face polygons
		*/
		for (size_t iConcFacePolygonsCohort = 0; iConcFacePolygonsCohort < pDefinition->concFacePolygonsCohorts().size(); iConcFacePolygonsCohort++)
		{
			if ((int_t)(iConcFacePolygonsIndicesCount + pDefinition->concFacePolygonsCohorts()[iConcFacePolygonsCohort]->indices().size()) > (int_t)INDICES_MAX_COUNT)
			{
				if (m_oglBuffers.createIBO(vecConcFacePolygonsCohorts) != iConcFacePolygonsIndicesCount)
				{
					ASSERT(FALSE);

					return;
				}

				iConcFacePolygonsIndicesCount = 0;
				vecConcFacePolygonsCohorts.clear();
			}

			iConcFacePolygonsIndicesCount += (GLsizei)pDefinition->concFacePolygonsCohorts()[iConcFacePolygonsCohort]->indices().size();
			vecConcFacePolygonsCohorts.push_back(pDefinition->concFacePolygonsCohorts()[iConcFacePolygonsCohort]);
		}

		/*
		* IBO - Lines
		*/
		for (size_t iLinesCohort = 0; iLinesCohort < pDefinition->linesCohorts().size(); iLinesCohort++)
		{
			if ((int_t)(iLinesIndicesCount + pDefinition->linesCohorts()[iLinesCohort]->indices().size()) > (int_t)INDICES_MAX_COUNT)
			{
				if (m_oglBuffers.createIBO(vecLinesCohorts) != iLinesIndicesCount)
				{
					ASSERT(FALSE);

					return;
				}

				iLinesIndicesCount = 0;
				vecLinesCohorts.clear();
			}

			iLinesIndicesCount += (GLsizei)pDefinition->linesCohorts()[iLinesCohort]->indices().size();
			vecLinesCohorts.push_back(pDefinition->linesCohorts()[iLinesCohort]);
		}

		/*
		* IBO - Points
		*/
		for (size_t iPointsCohort = 0; iPointsCohort < pDefinition->pointsCohorts().size(); iPointsCohort++)
		{
			if ((int_t)(iPointsIndicesCount + pDefinition->pointsCohorts()[iPointsCohort]->indices().size()) > (int_t)INDICES_MAX_COUNT)
			{
				if (m_oglBuffers.createIBO(vecPointsCohorts) != iPointsIndicesCount)
				{
					ASSERT(FALSE);

					return;
				}

				iPointsIndicesCount = 0;
				vecPointsCohorts.clear();
			}

			iPointsIndicesCount += (GLsizei)pDefinition->pointsCohorts()[iPointsCohort]->indices().size();
			vecPointsCohorts.push_back(pDefinition->pointsCohorts()[iPointsCohort]);
		}

		iVerticesCount += (GLsizei)pDefinition->getVerticesCount();
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
		if (m_oglBuffers.createCohort(vecProductDefinitionsCohort, m_pOGLProgram) != iVerticesCount)
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

	_redraw();
}

/*virtual*/ void COpenGLSTEPView::_draw(CDC* pDC) /*override*/
{
	VERIFY(pDC != nullptr);

	auto pModel = GetModel<CSTEPModel>();
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	CRect rcClient;
	m_pWnd->GetClientRect(&rcClient);

	int iWidth = rcClient.Width();
	int iHeight = rcClient.Height();

	if ((iWidth < MIN_VIEW_PORT_LENGTH) || (iHeight < MIN_VIEW_PORT_LENGTH))
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
		0, 0,
		iWidth, iHeight,
		fXmin, fXmax,
		fYmin, fYmax,
		fZmin, fZmax,
		true,
		true);

	/* Non-transparent faces */
	DrawFaces(pModel, false);

	/* Transparent faces */
	DrawFaces(pModel, true);

	/* Conceptual faces polygons */
	DrawConceptualFacesPolygons(pModel);

	/* Lines */
	DrawLines(pModel);

	/* Points */
	DrawPoints(pModel);

	/* End */
	SwapBuffers(*pDC);

	/* Selection support */
	DrawInstancesFrameBuffer();
}

/*virtual*/ void COpenGLSTEPView::OnWorldDimensionsChanged()  /*override*/
{
	auto pModel = GetModel<CSTEPModel>();
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

	m_fScaleFactor = pModel->GetBoundingSphereDiameter();

	_redraw();
}

/*virtual*/ void COpenGLSTEPView::OnInstanceSelected(CViewBase* pSender)  /*override*/
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

		_redraw();
	}
}

/*virtual*/ void COpenGLSTEPView::OnInstancesEnabledStateChanged(CViewBase* pSender)  /*override*/
{
	if (pSender == this)
	{
		return;
	}

	/*
	* Restore the selection
	*/
	OnInstanceSelected(nullptr);

	_redraw();
}

/*virtual*/ void COpenGLSTEPView::OnApplicationPropertyChanged(CViewBase* pSender, enumApplicationProperty enApplicationProperty) /*override*/
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

/*virtual*/ void COpenGLSTEPView::OnControllerChanged() /*override*/
{
	ASSERT(GetController() != nullptr);

	GetController()->RegisterView(this);

	loadSettings();
}

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

void COpenGLSTEPView::DrawFaces(_model* pM, bool bTransparent)
{
	auto pModel = dynamic_cast<CSTEPModel*>(pM);
	if (pModel == nullptr)
	{
		return;
	}

	if (!getShowFaces(pModel))
	{
		return;
	}

	if (pModel->GetDefinitions().empty())
	{
		return;
	}

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	CString strCullFaces = getCullFacesMode(pModel);

	if (bTransparent)
	{
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else
	{
		if ((strCullFaces == CULL_FACES_FRONT) || (strCullFaces == CULL_FACES_BACK))
		{
			glEnable(GL_CULL_FACE);
			glCullFace(strCullFaces == CULL_FACES_FRONT ? GL_FRONT : GL_BACK);
		}
	}

#ifdef _BLINN_PHONG_SHADERS
	m_pOGLProgram->_enableBlinnPhongModel(true);
#else
	m_pOGLProgram->_enableLighting(true);
#endif	

	_vector3d vecVertexBufferOffset;
	GetVertexBufferOffset(pModel->GetInstance(), (double*)&vecVertexBufferOffset);

	float dScaleFactor = (float)pModel->GetOriginalBoundingSphereDiameter() / 2.f;
	float fXTranslation = (float)vecVertexBufferOffset.x / dScaleFactor;
	float fYTranslation = (float)vecVertexBufferOffset.y / dScaleFactor;
	float fZTranslation = (float)vecVertexBufferOffset.z / dScaleFactor;

	for (auto itCohort : m_oglBuffers.cohorts())
	{
		glBindVertexArray(itCohort.first);

		for (auto pDefinition : itCohort.second)
		{
			if (pDefinition->concFacesCohorts().empty())
			{
				continue;
			}

			auto& vecInstances = dynamic_cast<CProductDefinition*>(pDefinition)->GetInstances();
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
#ifdef _BLINN_PHONG_SHADERS
				glm::mat4 matNormal = m_matModelView * matTransformation;
				matNormal = glm::inverse(matNormal);
				matNormal = glm::transpose(matNormal);
				m_pOGLProgram->_setNormalMatrix(matNormal);
#else
				m_pOGLProgram->_setNormalMatrix(matModelView);
#endif

				for (size_t iCohort = 0; iCohort < pDefinition->concFacesCohorts().size(); iCohort++)
				{
					auto pCohort = pDefinition->concFacesCohorts()[iCohort];

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

					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pCohort->IBO());
					glDrawElementsBaseVertex(GL_TRIANGLES,
						(GLsizei)pCohort->indices().size(),
						GL_UNSIGNED_INT,
						(void*)(sizeof(GLuint) * pCohort->IBOOffset()),
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
	else
	{
		if ((strCullFaces == CULL_FACES_FRONT) || (strCullFaces == CULL_FACES_BACK))
		{
			glDisable(GL_CULL_FACE);
		}
	}

	// Restore Model-View Matrix
	m_pOGLProgram->_setModelViewMatrix(m_matModelView);

	_oglUtils::checkForErrors();

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	TRACE(L"\n*** DrawFaces() : %lld [탎]", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());	
}

void COpenGLSTEPView::DrawConceptualFacesPolygons(_model* pM)
{
	auto pModel = dynamic_cast<CSTEPModel*>(pM);
	if (pModel == nullptr)
	{
		return;
	}

	if (!getShowConceptualFacesPolygons(pM))
	{
		return;
	}

	if (pModel->GetDefinitions().empty())
	{
		return;
	}

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

#ifdef _BLINN_PHONG_SHADERS
	m_pOGLProgram->_enableBlinnPhongModel(false);
#else
	m_pOGLProgram->_enableLighting(false);
#endif
	m_pOGLProgram->_setAmbientColor(0.f, 0.f, 0.f);
	m_pOGLProgram->_setTransparency(1.f);

	_vector3d vecVertexBufferOffset;
	GetVertexBufferOffset(pModel->GetInstance(), (double*)&vecVertexBufferOffset);

	float dScaleFactor = (float)pModel->GetOriginalBoundingSphereDiameter() / 2.f;
	float fXTranslation = (float)vecVertexBufferOffset.x / dScaleFactor;
	float fYTranslation = (float)vecVertexBufferOffset.y / dScaleFactor;
	float fZTranslation = (float)vecVertexBufferOffset.z / dScaleFactor;

	for (auto itCohort : m_oglBuffers.cohorts())
	{
		glBindVertexArray(itCohort.first);

		for (auto pDefinition : itCohort.second)
		{
			if (pDefinition->concFacePolygonsCohorts().empty())
			{
				continue;
			}

			auto& vecInstances = dynamic_cast<CProductDefinition*>(pDefinition)->GetInstances();
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
#ifdef _BLINN_PHONG_SHADERS
				glm::mat4 matNormal = m_matModelView * matTransformation;
				matNormal = glm::inverse(matNormal);
				matNormal = glm::transpose(matNormal);
				m_pOGLProgram->_setNormalMatrix(matNormal);
#else
				m_pOGLProgram->_setNormalMatrix(matModelView);
#endif
				
				for (size_t iCohort = 0; iCohort < pDefinition->concFacePolygonsCohorts().size(); iCohort++)
				{
					_cohort* pCohort = pDefinition->concFacePolygonsCohorts()[iCohort];

					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pCohort->IBO());
					glDrawElementsBaseVertex(GL_LINES,
						(GLsizei)pCohort->indices().size(),
						GL_UNSIGNED_INT,
						(void*)(sizeof(GLuint) * pCohort->IBOOffset()),
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

void COpenGLSTEPView::DrawLines(_model* pM)
{
	auto pModel = dynamic_cast<CSTEPModel*>(pM);
	if (pModel == nullptr)
	{
		return;
	}

	if (!getShowLines(pM))
	{
		return;
	}

	if (pModel->GetDefinitions().empty())
	{
		return;
	}

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

#ifdef _BLINN_PHONG_SHADERS
	m_pOGLProgram->_enableBlinnPhongModel(false);
#else
	m_pOGLProgram->_enableLighting(false);
#endif
	m_pOGLProgram->_setAmbientColor(0.f, 0.f, 0.f);
	m_pOGLProgram->_setTransparency(1.f);

	_vector3d vecVertexBufferOffset;
	GetVertexBufferOffset(pModel->GetInstance(), (double*)&vecVertexBufferOffset);

	float dScaleFactor = (float)pModel->GetOriginalBoundingSphereDiameter() / 2.f;
	float fXTranslation = (float)vecVertexBufferOffset.x / dScaleFactor;
	float fYTranslation = (float)vecVertexBufferOffset.y / dScaleFactor;
	float fZTranslation = (float)vecVertexBufferOffset.z / dScaleFactor;

	for (auto itCohort : m_oglBuffers.cohorts())
	{
		glBindVertexArray(itCohort.first);

		for (auto pDefinition : itCohort.second)
		{
			if (pDefinition->linesCohorts().empty())
			{
				continue;
			}

			auto& vecInstances = dynamic_cast<CProductDefinition*>(pDefinition)->GetInstances();
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
#ifdef _BLINN_PHONG_SHADERS
				glm::mat4 matNormal = m_matModelView * matTransformation;
				matNormal = glm::inverse(matNormal);
				matNormal = glm::transpose(matNormal);
				m_pOGLProgram->_setNormalMatrix(matNormal);
#else
				m_pOGLProgram->_setNormalMatrix(matModelView);
#endif

				for (size_t iCohort = 0; iCohort < pDefinition->linesCohorts().size(); iCohort++)
				{
					_cohort* pCohort = pDefinition->linesCohorts()[iCohort];

					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pCohort->IBO());
					glDrawElementsBaseVertex(GL_LINES,
						(GLsizei)pCohort->indices().size(),
						GL_UNSIGNED_INT,
						(void*)(sizeof(GLuint) * pCohort->IBOOffset()),
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

void COpenGLSTEPView::DrawPoints(_model* pM)
{
	auto pModel = dynamic_cast<CSTEPModel*>(pM);
	if (pModel == nullptr)
	{
		return;
	}

	if (!getShowPoints(pM))
	{
		return;
	}

	if (pModel->GetDefinitions().empty())
	{
		return;
	}

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	glEnable(GL_PROGRAM_POINT_SIZE);

#ifdef _BLINN_PHONG_SHADERS
	m_pOGLProgram->_enableBlinnPhongModel(false);
#else
	m_pOGLProgram->_enableLighting(false);
#endif
	m_pOGLProgram->_setAmbientColor(0.f, 0.f, 0.f);

	_vector3d vecVertexBufferOffset;
	GetVertexBufferOffset(pModel->GetInstance(), (double*)&vecVertexBufferOffset);

	float dScaleFactor = (float)pModel->GetOriginalBoundingSphereDiameter() / 2.f;
	float fXTranslation = (float)vecVertexBufferOffset.x / dScaleFactor;
	float fYTranslation = (float)vecVertexBufferOffset.y / dScaleFactor;
	float fZTranslation = (float)vecVertexBufferOffset.z / dScaleFactor;

	for (auto itCohort : m_oglBuffers.cohorts())
	{
		glBindVertexArray(itCohort.first);

		for (auto pDefinition : itCohort.second)
		{
			if (pDefinition->pointsCohorts().empty())
			{
				continue;
			}

			auto& vecInstances = dynamic_cast<CProductDefinition*>(pDefinition)->GetInstances();
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
#ifdef _BLINN_PHONG_SHADERS
				glm::mat4 matNormal = m_matModelView * matTransformation;
				matNormal = glm::inverse(matNormal);
				matNormal = glm::transpose(matNormal);
				m_pOGLProgram->_setNormalMatrix(matNormal);
#else
				m_pOGLProgram->_setNormalMatrix(matModelView);
#endif

				for (auto pCohort : pDefinition->pointsCohorts())
				{
					const _material* pMaterial =
						pInstance == m_pSelectedInstance ? m_pSelectedInstanceMaterial :
						pInstance == m_pPointedInstance ? m_pPointedInstanceMaterial :
						pCohort->getMaterial();

					m_pOGLProgram->_setAmbientColor(
						pMaterial->getDiffuseColor().r(),
						pMaterial->getDiffuseColor().g(),
						pMaterial->getDiffuseColor().b());

					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pCohort->IBO());
					glDrawElementsBaseVertex(GL_POINTS,
						(GLsizei)pCohort->indices().size(),
						GL_UNSIGNED_INT,
						(void*)(sizeof(GLuint) * pCohort->IBOOffset()),
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

void COpenGLSTEPView::DrawInstancesFrameBuffer()
{
	auto pModel = GetModel<CSTEPModel>();
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
	* Validation
	*/
	int iWidth = 0;
	int iHeight = 0;

	CRect rcClient;
	m_pWnd->GetClientRect(&rcClient);

	iWidth = rcClient.Width();
	iHeight = rcClient.Height();

	if ((iWidth < MIN_VIEW_PORT_LENGTH) || (iHeight < MIN_VIEW_PORT_LENGTH))
	{
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
		auto& mapDefinitions = pModel->GetDefinitions();
		for (auto itDefinition = mapDefinitions.begin(); 
			itDefinition != mapDefinitions.end(); 
			itDefinition++)
		{
			if (itDefinition->second->getTriangles().empty())
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

#ifdef _BLINN_PHONG_SHADERS
	m_pOGLProgram->_enableBlinnPhongModel(false);
#else
	m_pOGLProgram->_enableLighting(false);
#endif
	m_pOGLProgram->_setTransparency(1.f);

	_vector3d vecVertexBufferOffset;
	GetVertexBufferOffset(pModel->GetInstance(), (double*)&vecVertexBufferOffset);

	float dScaleFactor = (float)pModel->GetOriginalBoundingSphereDiameter() / 2.f;
	float fXTranslation = (float)vecVertexBufferOffset.x / dScaleFactor;
	float fYTranslation = (float)vecVertexBufferOffset.y / dScaleFactor;
	float fZTranslation = (float)vecVertexBufferOffset.z / dScaleFactor;

	for (auto itCohort : m_oglBuffers.cohorts())
	{
		glBindVertexArray(itCohort.first);

		for (auto pDefinition : itCohort.second)
		{
			if (pDefinition->getTriangles().empty())
			{
				continue;
			}

			auto& vecInstances = dynamic_cast<CProductDefinition*>(pDefinition)->GetInstances();
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
#ifdef _BLINN_PHONG_SHADERS
				glm::mat4 matNormal = m_matModelView * matTransformation;
				matNormal = glm::inverse(matNormal);
				matNormal = glm::transpose(matNormal);
				m_pOGLProgram->_setNormalMatrix(matNormal);
#else
				m_pOGLProgram->_setNormalMatrix(matModelView);
#endif

				for (size_t iCohort = 0; iCohort < pDefinition->concFacesCohorts().size(); iCohort++)
				{
					auto pCohort = pDefinition->concFacesCohorts()[iCohort];

					auto itSelectionColor = m_pInstanceSelectionFrameBuffer->encoding().find(pInstance->GetID());
					ASSERT(itSelectionColor != m_pInstanceSelectionFrameBuffer->encoding().end());

					m_pOGLProgram->_setAmbientColor(
						itSelectionColor->second.r(),
						itSelectionColor->second.g(),
						itSelectionColor->second.b());

					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pCohort->IBO());
					glDrawElementsBaseVertex(GL_TRIANGLES,
						(GLsizei)pCohort->indices().size(),
						GL_UNSIGNED_INT,
						(void*)(sizeof(GLuint) * pCohort->IBOOffset()),
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

void COpenGLSTEPView::OnMouseMoveEvent(UINT nFlags, CPoint point)
{
	auto pModel = GetModel<CSTEPModel>();
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

