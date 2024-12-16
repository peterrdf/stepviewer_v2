#include "stdafx.h"

#include "_oglUtils.h"

// ************************************************************************************************
_oglView::_oglView()
	: m_ptStartMousePosition(-1, -1)
	, m_ptPrevMousePosition(-1, -1)
	, m_pInstanceSelectionFrameBuffer(new _oglSelectionFramebuffer())
	, m_pPointedInstance(nullptr)
	, m_pSelectedInstance(nullptr)
{
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

/*virtual*/ _oglView::~_oglView()
{
	_view::getController()->unRegisterView(this);

	_destroy();

	delete m_pInstanceSelectionFrameBuffer;
}

/*virtual*/ void _oglView::onWorldDimensionsChanged() /*override*/
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

/*virtual*/ void _oglView::onInstanceSelected(_view* pSender)  /*override*/
{
	if (pSender == this)
	{
		return;
	}

	if (_view::getController() == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pSelectedInstance = _view::getController()->getSelectedInstance();
	if (m_pSelectedInstance != pSelectedInstance)
	{
		m_pSelectedInstance = pSelectedInstance;

		_redraw();
	}
}

/*virtual*/ void _oglView::onInstancesEnabledStateChanged(_view* pSender) /*override*/
{
	if (pSender == this)
	{
		return;
	}

	_redraw();
}

/*virtual*/ void _oglView::onApplicationPropertyChanged(_view* pSender, enumApplicationProperty enApplicationProperty) /*override*/
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

/*virtual*/ void _oglView::_load(_model* pModel)
{
	assert(pModel != nullptr);

	BOOL bResult = m_pOGLContext->makeCurrent();
	VERIFY(bResult);

	// OpenGL buffers
	m_oglBuffers.clear();

	m_pInstanceSelectionFrameBuffer->encoding().clear();
	m_pPointedInstance = nullptr;
	m_pSelectedInstance = nullptr;

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

	// Limits
	GLsizei VERTICES_MAX_COUNT = _oglUtils::getVerticesCountLimit(GEOMETRY_VBO_VERTEX_LENGTH * sizeof(float));
	GLsizei INDICES_MAX_COUNT = _oglUtils::getIndicesCountLimit();

	// Data
	auto& vecGeometries = pModel->getGeometries();

	// VBO
	GLuint iVerticesCount = 0;
	vector<_geometry*> vecGeometriesCohort;

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

	for (auto pGeometry : vecGeometries)
	{
		if (pGeometry->getVerticesCount() == 0)
		{
			continue;
		}

		// VBO - Conceptual faces, polygons, etc.
		if (((int_t)iVerticesCount + pGeometry->getVerticesCount()) > (int_t)VERTICES_MAX_COUNT)
		{
			if (m_oglBuffers.createCohort(vecGeometriesCohort, m_pOGLProgram) != iVerticesCount)
			{
				ASSERT(FALSE);

				return;
			}

			iVerticesCount = 0;
			vecGeometriesCohort.clear();
		}

		// IBO - Conceptual faces
		for (size_t iCohort = 0; iCohort < pGeometry->concFacesCohorts().size(); iCohort++)
		{
			if ((int_t)(iConcFacesIndicesCount + pGeometry->concFacesCohorts()[iCohort]->indices().size()) > (int_t)INDICES_MAX_COUNT)
			{
				if (m_oglBuffers.createIBO(vecConcFacesCohorts) != iConcFacesIndicesCount)
				{
					ASSERT(FALSE);

					return;
				}

				iConcFacesIndicesCount = 0;
				vecConcFacesCohorts.clear();
			}

			iConcFacesIndicesCount += (GLsizei)pGeometry->concFacesCohorts()[iCohort]->indices().size();
			vecConcFacesCohorts.push_back(pGeometry->concFacesCohorts()[iCohort]);
		}

		//  IBO - Conceptual face polygons
		for (size_t iCohort = 0; iCohort < pGeometry->concFacePolygonsCohorts().size(); iCohort++)
		{
			if ((int_t)(iConcFacePolygonsIndicesCount + pGeometry->concFacePolygonsCohorts()[iCohort]->indices().size()) > (int_t)INDICES_MAX_COUNT)
			{
				if (m_oglBuffers.createIBO(vecConcFacePolygonsCohorts) != iConcFacePolygonsIndicesCount)
				{
					ASSERT(FALSE);

					return;
				}

				iConcFacePolygonsIndicesCount = 0;
				vecConcFacePolygonsCohorts.clear();
			}

			iConcFacePolygonsIndicesCount += (GLsizei)pGeometry->concFacePolygonsCohorts()[iCohort]->indices().size();
			vecConcFacePolygonsCohorts.push_back(pGeometry->concFacePolygonsCohorts()[iCohort]);
		}

		// IBO - Lines
		for (size_t iCohort = 0; iCohort < pGeometry->linesCohorts().size(); iCohort++)
		{
			if ((int_t)(iLinesIndicesCount + pGeometry->linesCohorts()[iCohort]->indices().size()) > (int_t)INDICES_MAX_COUNT)
			{
				if (m_oglBuffers.createIBO(vecLinesCohorts) != iLinesIndicesCount)
				{
					ASSERT(FALSE);

					return;
				}

				iLinesIndicesCount = 0;
				vecLinesCohorts.clear();
			}

			iLinesIndicesCount += (GLsizei)pGeometry->linesCohorts()[iCohort]->indices().size();
			vecLinesCohorts.push_back(pGeometry->linesCohorts()[iCohort]);
		}

		//  IBO - Points
		for (size_t iCohort = 0; iCohort < pGeometry->pointsCohorts().size(); iCohort++)
		{
			if ((int_t)(iPointsIndicesCount + pGeometry->pointsCohorts()[iCohort]->indices().size()) > (int_t)INDICES_MAX_COUNT)
			{
				if (m_oglBuffers.createIBO(vecPointsCohorts) != iPointsIndicesCount)
				{
					ASSERT(FALSE);

					return;
				}

				iPointsIndicesCount = 0;
				vecPointsCohorts.clear();
			}

			iPointsIndicesCount += (GLsizei)pGeometry->pointsCohorts()[iCohort]->indices().size();
			vecPointsCohorts.push_back(pGeometry->pointsCohorts()[iCohort]);
		}

		iVerticesCount += (GLsizei)pGeometry->getVerticesCount();
		vecGeometriesCohort.push_back(pGeometry);
	} // for (auto pGeometry : vecGeometries)

	//  VBO - Conceptual faces, polygons, etc.
	if (iVerticesCount > 0)
	{
		if (m_oglBuffers.createCohort(vecGeometriesCohort, m_pOGLProgram) != iVerticesCount)
		{
			ASSERT(FALSE);

			return;
		}

		iVerticesCount = 0;
		vecGeometriesCohort.clear();
	} // if (iVerticesCount > 0)	

	//  IBO - Conceptual faces
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

	//  IBO - Conceptual face polygons
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

	//  IBO - Lines
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

	//  IBO - Points
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

/*virtual*/ bool _oglView::_preDraw(_model* pModel)
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

/*virtual*/ void _oglView::_draw(CDC* pDC)
{
	auto pModel = getModel();
	if (pModel == nullptr)
	{
		return;
	}

	if (!_preDraw(pModel))
	{
		return;
	}

	// Scene
	_drawFaces(pModel, false);
	_drawFaces(pModel, true);
	_drawConceptualFacesPolygons(pModel);
	_drawLines(pModel);
	_drawPoints(pModel);

	// OpenGL
	SwapBuffers(*pDC);

	_postDraw(pModel);
}

/*virtual*/ void _oglView::_postDraw(_model* pModel)
{
	_drawInstancesFrameBuffer(pModel);
}

/*virtual*/ void _oglView::_drawFaces(_model* pModel, bool bTransparent)
{
	if (pModel == nullptr)
	{
		return;
	}

	if (!getShowFaces(pModel))
	{
		return;
	}

	if (pModel->getGeometries().empty())
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
	GetVertexBufferOffset(pModel->getOwlInstance(), (double*)&vecVertexBufferOffset);

	float dScaleFactor = (float)pModel->getOriginalBoundingSphereDiameter() / 2.f;
	float fXTranslation = (float)vecVertexBufferOffset.x / dScaleFactor;
	float fYTranslation = (float)vecVertexBufferOffset.y / dScaleFactor;
	float fZTranslation = (float)vecVertexBufferOffset.z / dScaleFactor;

	for (auto itCohort : m_oglBuffers.cohorts())
	{
		glBindVertexArray(itCohort.first);

		for (auto pGeometry : itCohort.second)
		{
			if (pGeometry->concFacesCohorts().empty())
			{
				continue;
			}

			for (auto pInstance : pGeometry->getInstances())
			{
				if (!pInstance->getEnable())
				{
					continue;
				}

				// Transformation Matrix
				glm::mat4 matTransformation = glm::make_mat4((GLdouble*)pInstance->getTransformationMatrix());

				// Model-View Matrix
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
				for (size_t iCohort = 0; iCohort < pGeometry->concFacesCohorts().size(); iCohort++)
				{
					auto pCohort = pGeometry->concFacesCohorts()[iCohort];

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
						pGeometry->VBOOffset());
				}
			} // auto pInstance : ...			
		} // for (auto pGeometry : ...

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

void _oglView::_drawConceptualFacesPolygons(_model* pModel)
{
	if (pModel == nullptr)
	{
		return;
	}

	if (!getShowConceptualFacesPolygons(pModel))
	{
		return;
	}

	if (pModel->getGeometries().empty())
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
	GetVertexBufferOffset(pModel->getOwlInstance(), (double*)&vecVertexBufferOffset);

	float dScaleFactor = (float)pModel->getOriginalBoundingSphereDiameter() / 2.f;
	float fXTranslation = (float)vecVertexBufferOffset.x / dScaleFactor;
	float fYTranslation = (float)vecVertexBufferOffset.y / dScaleFactor;
	float fZTranslation = (float)vecVertexBufferOffset.z / dScaleFactor;

	for (auto itCohort : m_oglBuffers.cohorts())
	{
		glBindVertexArray(itCohort.first);

		for (auto pGeometry : itCohort.second)
		{
			if (pGeometry->concFacePolygonsCohorts().empty())
			{
				continue;
			}

			for (auto pInstance : pGeometry->getInstances())
			{
				if (!pInstance->getEnable())
				{
					continue;
				}

				// Transformation Matrix
				glm::mat4 matTransformation = glm::make_mat4((GLdouble*)pInstance->getTransformationMatrix());

				// Model-View Matrix
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
				for (size_t iCohort = 0; iCohort < pGeometry->concFacePolygonsCohorts().size(); iCohort++)
				{
					_cohort* pCohort = pGeometry->concFacePolygonsCohorts()[iCohort];

					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pCohort->IBO());
					glDrawElementsBaseVertex(GL_LINES,
						(GLsizei)pCohort->indices().size(),
						GL_UNSIGNED_INT,
						(void*)(sizeof(GLuint) * pCohort->IBOOffset()),
						pGeometry->VBOOffset());
				}
			} // for (size_t iInstance = ...			
		} // for (auto pGeometry ...

		glBindVertexArray(0);
	} // for (auto itCohort ...

	// Restore Model-View Matrix
	m_pOGLProgram->_setModelViewMatrix(m_matModelView);

	_oglUtils::checkForErrors();

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	TRACE(L"\n*** DrawConceptualFacesPolygons() : %lld [탎]", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());
}

void _oglView::_drawLines(_model* pModel)
{
	if (pModel == nullptr)
	{
		return;
	}

	if (!getShowLines(pModel))
	{
		return;
	}

	if (pModel->getGeometries().empty())
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
	GetVertexBufferOffset(pModel->getOwlInstance(), (double*)&vecVertexBufferOffset);

	float dScaleFactor = (float)pModel->getOriginalBoundingSphereDiameter() / 2.f;
	float fXTranslation = (float)vecVertexBufferOffset.x / dScaleFactor;
	float fYTranslation = (float)vecVertexBufferOffset.y / dScaleFactor;
	float fZTranslation = (float)vecVertexBufferOffset.z / dScaleFactor;

	for (auto itCohort : m_oglBuffers.cohorts())
	{
		glBindVertexArray(itCohort.first);

		for (auto pGeometry : itCohort.second)
		{
			if (pGeometry->linesCohorts().empty())
			{
				continue;
			}

			for (auto pInstance : pGeometry->getInstances())
			{
				if (!pInstance->getEnable())
				{
					continue;
				}

				// Transformation Matrix
				glm::mat4 matTransformation = glm::make_mat4((GLdouble*)pInstance->getTransformationMatrix());

				// Model-View Matrix
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
				for (size_t iCohort = 0; iCohort < pGeometry->linesCohorts().size(); iCohort++)
				{
					_cohort* pCohort = pGeometry->linesCohorts()[iCohort];

					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pCohort->IBO());
					glDrawElementsBaseVertex(GL_LINES,
						(GLsizei)pCohort->indices().size(),
						GL_UNSIGNED_INT,
						(void*)(sizeof(GLuint) * pCohort->IBOOffset()),
						pGeometry->VBOOffset());
				}
			} // for (size_t iInstance = ...			
		} // for (auto pGeometry ...

		glBindVertexArray(0);
	} // for (auto itCohort ...

	// Restore Model-View Matrix
	m_pOGLProgram->_setModelViewMatrix(m_matModelView);

	_oglUtils::checkForErrors();

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	TRACE(L"\n*** DrawLines() : %lld [탎]", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());
}

void _oglView::_drawPoints(_model* pModel)
{
	if (pModel == nullptr)
	{
		return;
	}

	if (!getShowPoints(pModel))
	{
		return;
	}

	if (pModel->getGeometries().empty())
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
	GetVertexBufferOffset(pModel->getOwlInstance(), (double*)&vecVertexBufferOffset);

	float dScaleFactor = (float)pModel->getOriginalBoundingSphereDiameter() / 2.f;
	float fXTranslation = (float)vecVertexBufferOffset.x / dScaleFactor;
	float fYTranslation = (float)vecVertexBufferOffset.y / dScaleFactor;
	float fZTranslation = (float)vecVertexBufferOffset.z / dScaleFactor;

	for (auto itCohort : m_oglBuffers.cohorts())
	{
		glBindVertexArray(itCohort.first);

		for (auto pGeometry : itCohort.second)
		{
			if (pGeometry->pointsCohorts().empty())
			{
				continue;
			}

			for (auto pInstance : pGeometry->getInstances())
			{
				if (!pInstance->getEnable())
				{
					continue;
				}

				// Transformation Matrix
				glm::mat4 matTransformation = glm::make_mat4((GLdouble*)pInstance->getTransformationMatrix());

				// Model-View Matrix
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
				for (auto pCohort : pGeometry->pointsCohorts())
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
						pGeometry->VBOOffset());
				}
			} // for (auto pInstance ...
		} // for (auto pGeometry ...

		glBindVertexArray(0);
	} // for (auto itCohort ...

	// Restore Model-View Matrix
	m_pOGLProgram->_setModelViewMatrix(m_matModelView);

	_oglUtils::checkForErrors();

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	TRACE(L"\n*** DrawPoints() : %lld [탎]", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());
}

void _oglView::_drawInstancesFrameBuffer(_model* pModel)
{
	if (pModel == nullptr)
	{
		return;
	}

	if (pModel->getGeometries().empty())
	{
		return;
	}

	//
	// Create a frame buffer
	//

	BOOL bResult = m_pOGLContext->makeCurrent();
	VERIFY(bResult);

	m_pInstanceSelectionFrameBuffer->create();

	// Selection colors
	if (m_pInstanceSelectionFrameBuffer->encoding().empty())
	{
		for (auto pGeometry : pModel->getGeometries())
		{
			if (pGeometry->getTriangles().empty())
			{
				continue;
			}

			for (auto pInstance : pGeometry->getInstances())
			{
				float fR, fG, fB;
				_i64RGBCoder::encode(pInstance->getID(), fR, fG, fB);

				m_pInstanceSelectionFrameBuffer->encoding()[pInstance->getID()] = _color(fR, fG, fB);
			}
		}
	} // if (m_pInstanceSelectionFrameBuffer->encoding().empty())

	//
	// Draw
	//

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
	GetVertexBufferOffset(pModel->getOwlInstance(), (double*)&vecVertexBufferOffset);

	float dScaleFactor = (float)pModel->getOriginalBoundingSphereDiameter() / 2.f;
	float fXTranslation = (float)vecVertexBufferOffset.x / dScaleFactor;
	float fYTranslation = (float)vecVertexBufferOffset.y / dScaleFactor;
	float fZTranslation = (float)vecVertexBufferOffset.z / dScaleFactor;

	for (auto itCohort : m_oglBuffers.cohorts())
	{
		glBindVertexArray(itCohort.first);

		for (auto pGeometry : itCohort.second)
		{
			if (pGeometry->getTriangles().empty())
			{
				continue;
			}

			for (auto pInstance : pGeometry->getInstances())
			{
				if (!pInstance->getEnable())
				{
					continue;
				}

				// Transformation Matrix
				glm::mat4 matTransformation = glm::make_mat4((GLdouble*)pInstance->getTransformationMatrix());

				// Model-View Matrix
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
				for (size_t iCohort = 0; iCohort < pGeometry->concFacesCohorts().size(); iCohort++)
				{
					auto pCohort = pGeometry->concFacesCohorts()[iCohort];

					auto itSelectionColor = m_pInstanceSelectionFrameBuffer->encoding().find(pInstance->getID());
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
						pGeometry->VBOOffset());
				}
			} // for (size_t iInstance = ...			
		} // for (auto pGeometry ...

		glBindVertexArray(0);
	} // for (auto itCohort ...

	// Restore Model-View Matrix
	m_pOGLProgram->_setModelViewMatrix(m_matModelView);

	m_pInstanceSelectionFrameBuffer->unbind();

	_oglUtils::checkForErrors();
}

void _oglView::_onMouseMoveEvent(UINT nFlags, CPoint point)
{
	_model* pModel = getModel();
	if (pModel == nullptr)
	{
		return;
	}

	// Selection
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

		_instance* pPointedInstance = nullptr;
		if (arPixels[3] != 0)
		{
			int64_t iInstanceID = _i64RGBCoder::decode(arPixels[0], arPixels[1], arPixels[2]);
			pPointedInstance = pModel->getInstanceByID(iInstanceID);
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

	// Rotate
	if ((nFlags & MK_LBUTTON) == MK_LBUTTON)
	{
		_rotateMouseLButton(
			(float)point.y - (float)m_ptPrevMousePosition.y,
			(float)point.x - (float)m_ptPrevMousePosition.x);

		m_ptPrevMousePosition = point;

		return;
	}

	// Zoom
	if ((nFlags & MK_MBUTTON) == MK_MBUTTON)
	{
		_zoomMouseMButton(point.y - m_ptPrevMousePosition.y);

		m_ptPrevMousePosition = point;

		return;
	}

	// Move
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

void _oglView::_onMouseEvent(enumMouseEvent enEvent, UINT nFlags, CPoint point)
{
	if (enEvent == enumMouseEvent::LBtnUp)
	{
		if (point == m_ptStartMousePosition)
		{
			if (m_pSelectedInstance != m_pPointedInstance)
			{
				m_pSelectedInstance = m_pPointedInstance;

				_redraw();

				_view::getController()->selectInstance(this, m_pSelectedInstance);
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
		assert(false);
		break;
	} // switch (enEvent)
}