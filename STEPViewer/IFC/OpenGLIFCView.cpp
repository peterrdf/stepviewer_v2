#include "stdafx.h"

#include "OpenGLIFCView.h"
#include "STEPController.h"
#include "IFCModel.h"
#include "resource.h"

#include <assert.h>

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

	////	CTime tmBuildOGLBuffersStart = CTime::GetCurrentTime();

	//BOOL bResult = m_pOGLContext->MakeCurrent();
	//VERIFY(bResult);

	//auto pController = GetController();
	//ASSERT(pController != NULL);

	//if (pController->GetModel() == nullptr)
	//{
	//	ASSERT(FALSE);

	//	return;
	//}

	//auto pModel = pController->GetModel()->As<CIFCModel>();
	//if (pModel == nullptr)
	//{
	//	ASSERT(FALSE);

	//	return;
	//}

	//float fXmin = -1.f;
	//float fXmax = 1.f;
	//float fYmin = -1.f;
	//float fYmax = 1.f;
	//float fZmin = -1.f;
	//float fZmax = 1.f;
	//pModel->GetWorldDimensions(fXmin, fXmax, fYmin, fYmax, fZmin, fZmax);

	///*
	//* Bounding sphere diameter
	//*/
	//float fBoundingSphereDiameter = fXmax - fXmin;
	//fBoundingSphereDiameter = fmax(fBoundingSphereDiameter, fYmax - fYmin);
	//fBoundingSphereDiameter = fmax(fBoundingSphereDiameter, fZmax - fZmin);

	//m_dScaleFactor = fBoundingSphereDiameter;

	//GLsizei VERTICES_MAX_COUNT = COpenGL::GetGeometryVerticesCountLimit();
	//GLsizei INDICES_MAX_COUNT = COpenGL::GetIndicesCountLimit();

	//const vector<CIFCObject*>& vecIFCObjects = pModel->getIFCObjects();

	//// VBO
	//GLuint iVerticesCount = 0;
	//vector<CIFCObject*> vecIFCObjectsGroup;

	//// IBO - Materials
	//GLuint iFacesIndicesCount = 0;
	//vector<CIFCGeometryWithMaterial*> vecIFCMaterialsGroup;

	//// IBO - Lines
	//GLuint iLinesIndicesCount = 0;
	//vector<CLinesCohort*> vecLinesCohorts;

	//// IBO - Wireframes
	//GLuint iWireframesIndicesCount = 0;
	//vector<CWireframesCohort*> vecWireframesCohorts;

	//for (size_t iIFCObject = 0; iIFCObject < vecIFCObjects.size(); iIFCObject++)
	//{
	//	CIFCObject* pIFCObject = vecIFCObjects[iIFCObject];

	//	if (!pIFCObject->hasGeometry())
	//	{
	//		// skip the objects without geometry
	//		continue;
	//	}

	//	/*
	//	* VBO - Conceptual faces, wireframes, etc.
	//	*/
	//	if (((int_t)iVerticesCount + pIFCObject->verticesCount()) > (int_t)VERTICES_MAX_COUNT)
	//	{
	//		assert(!vecIFCObjectsGroup.empty());

	//		int_t iCohortVerticesCount = 0;
	//		float* pVertices = pModel->GetVertices(vecIFCObjectsGroup, iCohortVerticesCount);
	//		if ((iCohortVerticesCount == 0) || (pVertices == nullptr))
	//		{
	//			ASSERT(0);

	//			return;
	//		}

	//		ASSERT(iCohortVerticesCount == iVerticesCount);

	//		CIFCObject::Scale(pVertices, iVerticesCount, fXmin, fXmax, fYmin, fYmax, fZmin, fZmax, fBoundingSphereDiameter);

	//		GLuint iVBO = 0;
	//		glGenBuffers(1, &iVBO);

	//		ASSERT(iVBO != 0);

	//		glBindBuffer(GL_ARRAY_BUFFER, iVBO);
	//		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * iVerticesCount * GEOMETRY_VBO_VERTEX_LENGTH, pVertices, GL_STATIC_DRAW);

	//		/*
	//		* Store VBO/offset
	//		*/
	//		GLsizei iVBOOffset = 0;
	//		for (iIFCObject = 0; iIFCObject < vecIFCObjectsGroup.size(); iIFCObject++)
	//		{
	//			vecIFCObjectsGroup[iIFCObject]->VBO() = iVBO;
	//			vecIFCObjectsGroup[iIFCObject]->VBOOffset() = iVBOOffset;

	//			// Update min/max
	//			vecIFCObjectsGroup[iIFCObject]->CalculateMinMaxValues(pVertices + (iVBOOffset * GEOMETRY_VBO_VERTEX_LENGTH), vecIFCObjectsGroup[iIFCObject]->verticesCount());

	//			// Update minimum bounding box
	//			vecIFCObjectsGroup[iIFCObject]->UpdateBBox(fXmin, fXmax, fYmin, fYmax, fZmin, fZmax, fBoundingSphereDiameter);

	//			iVBOOffset += (GLsizei)vecIFCObjectsGroup[iIFCObject]->verticesCount();
	//		} // for (size_t iIFCObject = ...

	//		delete[] pVertices;

	//		glBindBuffer(GL_ARRAY_BUFFER, 0);

	//		_oglUtils::checkForErrors();

	//		CIFCDrawMetaData* pDrawMetaData = new CIFCDrawMetaData();
	//		pDrawMetaData->AddGroup(iVBO, vecIFCObjectsGroup);

	//		m_vecIFCDrawMetaData.push_back(pDrawMetaData);

	//		iVerticesCount = 0;
	//		vecIFCObjectsGroup.clear();
	//	} // if (((int_t)iVerticesCount + pIFCObject->verticesCount()) > ...

	//	/*
	//	* IBO - Materials
	//	*/
	//	for (size_t iMaterial = 0; iMaterial < pIFCObject->conceptualFacesMaterials().size(); iMaterial++)
	//	{
	//		if ((int_t)(iFacesIndicesCount + pIFCObject->conceptualFacesMaterials()[iMaterial]->getIndicesCount()) > (int_t)INDICES_MAX_COUNT)
	//		{
	//			assert(!vecIFCMaterialsGroup.empty());

	//			GLuint iIBO = 0;
	//			glGenBuffers(1, &iIBO);

	//			ASSERT(iIBO != 0);

	//			m_vecIBOs.push_back(iIBO);

	//			int_t iGroupIndicesCount = 0;
	//			unsigned int* pIndices = pModel->GetMaterialsIndices(vecIFCMaterialsGroup, iGroupIndicesCount);
	//			if ((iGroupIndicesCount == 0) || (pIndices == nullptr))
	//			{
	//				ASSERT(0);

	//				return;
	//			}

	//			ASSERT(iFacesIndicesCount == iGroupIndicesCount);

	//			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iIBO);
	//			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * iFacesIndicesCount, pIndices, GL_STATIC_DRAW);

	//			delete[] pIndices;

	//			/*
	//			* Store IBO/offset
	//			*/
	//			GLsizei iIBOOffset = 0;
	//			for (size_t iMaterial2 = 0; iMaterial2 < vecIFCMaterialsGroup.size(); iMaterial2++)
	//			{
	//				vecIFCMaterialsGroup[iMaterial2]->IBO() = iIBO;
	//				vecIFCMaterialsGroup[iMaterial2]->IBOOffset() = iIBOOffset;

	//				iIBOOffset += (GLsizei)vecIFCMaterialsGroup[iMaterial2]->getIndicesCount();
	//			}

	//			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//			_oglUtils::checkForErrors();

	//			iFacesIndicesCount = 0;
	//			vecIFCMaterialsGroup.clear();
	//		} // if ((int_t)(iFacesIndicesCount + ...	

	//		iFacesIndicesCount += (GLsizei)pIFCObject->conceptualFacesMaterials()[iMaterial]->getIndicesCount();
	//		vecIFCMaterialsGroup.push_back(pIFCObject->conceptualFacesMaterials()[iMaterial]);
	//	} // for (size_t iMaterial = ...	

	//	/*
	//	* IBO - Lines
	//	*/
	//	for (size_t iLinesCohort = 0; iLinesCohort < pIFCObject->linesCohorts().size(); iLinesCohort++)
	//	{
	//		if ((int_t)(iLinesIndicesCount + pIFCObject->linesCohorts()[iLinesCohort]->getIndicesCount()) > (int_t)INDICES_MAX_COUNT)
	//		{
	//			assert(!vecLinesCohorts.empty());

	//			GLuint iIBO = 0;
	//			glGenBuffers(1, &iIBO);

	//			ASSERT(iIBO != 0);

	//			m_vecIBOs.push_back(iIBO);

	//			int_t iCohortIndicesCount = 0;
	//			unsigned int* pIndices = pModel->GetLinesCohortsIndices(vecLinesCohorts, iCohortIndicesCount);
	//			if ((iCohortIndicesCount == 0) || (pIndices == NULL))
	//			{
	//				ASSERT(0);

	//				return;
	//			}

	//			ASSERT(iLinesIndicesCount == iCohortIndicesCount);


	//			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iIBO);
	//			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * iLinesIndicesCount, pIndices, GL_STATIC_DRAW);

	//			delete[] pIndices;

	//			/*
	//			* Store IBO/offset
	//			*/
	//			GLsizei iIBOOffset = 0;
	//			for (size_t iLinesCohort2 = 0; iLinesCohort2 < vecLinesCohorts.size(); iLinesCohort2++)
	//			{
	//				vecLinesCohorts[iLinesCohort2]->IBO() = iIBO;
	//				vecLinesCohorts[iLinesCohort2]->IBOOffset() = iIBOOffset;

	//				iIBOOffset += (GLsizei)vecLinesCohorts[iLinesCohort2]->getIndicesCount();
	//			} // for (size_t iLinesCohort2 = ...				

	//			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//			_oglUtils::checkForErrors();

	//			iLinesIndicesCount = 0;
	//			vecLinesCohorts.clear();
	//		} // if ((int_t)(iLinesIndicesCount + ...	

	//		iLinesIndicesCount += (GLsizei)pIFCObject->linesCohorts()[iLinesCohort]->getIndicesCount();
	//		vecLinesCohorts.push_back(pIFCObject->linesCohorts()[iLinesCohort]);
	//	} // for (size_t iLinesCohort = ...	

	//	/*
	//	* IBO - Wireframes
	//	*/
	//	for (size_t iWireframesCohort = 0; iWireframesCohort < pIFCObject->wireframesCohorts().size(); iWireframesCohort++)
	//	{
	//		if ((int_t)(iWireframesIndicesCount + pIFCObject->wireframesCohorts()[iWireframesCohort]->getIndicesCount()) > (int_t)INDICES_MAX_COUNT)
	//		{
	//			assert(!vecWireframesCohorts.empty());

	//			GLuint iIBO = 0;
	//			glGenBuffers(1, &iIBO);

	//			ASSERT(iIBO != 0);

	//			m_vecIBOs.push_back(iIBO);

	//			int_t iCohortIndicesCount = 0;
	//			unsigned int* pIndices = pModel->GetWireframesCohortsIndices(vecWireframesCohorts, iCohortIndicesCount);
	//			if ((iCohortIndicesCount == 0) || (pIndices == NULL))
	//			{
	//				ASSERT(0);

	//				return;
	//			}

	//			ASSERT(iWireframesIndicesCount == iCohortIndicesCount);


	//			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iIBO);
	//			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * iWireframesIndicesCount, pIndices, GL_STATIC_DRAW);

	//			delete[] pIndices;

	//			/*
	//			* Store IBO/offset
	//			*/
	//			GLsizei iIBOOffset = 0;
	//			for (size_t iWireframesCohort2 = 0; iWireframesCohort2 < vecWireframesCohorts.size(); iWireframesCohort2++)
	//			{
	//				vecWireframesCohorts[iWireframesCohort2]->IBO() = iIBO;
	//				vecWireframesCohorts[iWireframesCohort2]->IBOOffset() = iIBOOffset;

	//				iIBOOffset += (GLsizei)vecWireframesCohorts[iWireframesCohort2]->getIndicesCount();
	//			} // for (size_t iWireframesCohort2 = ...				

	//			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//			_oglUtils::checkForErrors();

	//			iWireframesIndicesCount = 0;
	//			vecWireframesCohorts.clear();
	//		} // if ((int_t)(iWireframesIndicesCount + ...	

	//		iWireframesIndicesCount += (GLsizei)pIFCObject->wireframesCohorts()[iWireframesCohort]->getIndicesCount();
	//		vecWireframesCohorts.push_back(pIFCObject->wireframesCohorts()[iWireframesCohort]);
	//	} // for (size_t iWireframesCohort = ...	

	//	iVerticesCount += (GLsizei)pIFCObject->verticesCount();
	//	vecIFCObjectsGroup.push_back(pIFCObject);
	//} // for (size_t iIFCObject = ...

	///*
	//* VBO - Conceptual faces, wireframes, etc.
	//*/
	//if (iVerticesCount > 0)
	//{
	//	assert(!vecIFCObjectsGroup.empty());

	//	int_t iCohortVerticesCount = 0;
	//	float* pVertices = pModel->GetVertices(vecIFCObjectsGroup, iCohortVerticesCount);
	//	if ((iCohortVerticesCount == 0) || (pVertices == nullptr))
	//	{
	//		ASSERT(0);

	//		return;
	//	}

	//	ASSERT(iCohortVerticesCount == iVerticesCount);

	//	CIFCObject::Scale(pVertices, iVerticesCount, fXmin, fXmax, fYmin, fYmax, fZmin, fZmax, fBoundingSphereDiameter);

	//	GLuint iVBO = 0;
	//	glGenBuffers(1, &iVBO);

	//	ASSERT(iVBO != 0);

	//	glBindBuffer(GL_ARRAY_BUFFER, iVBO);
	//	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * iVerticesCount * GEOMETRY_VBO_VERTEX_LENGTH, pVertices, GL_STATIC_DRAW);

	//	/*
	//	* Store VBO/offset
	//	*/
	//	GLsizei iVBOOffset = 0;
	//	for (size_t iIFCObject = 0; iIFCObject < vecIFCObjectsGroup.size(); iIFCObject++)
	//	{
	//		vecIFCObjectsGroup[iIFCObject]->VBO() = iVBO;
	//		vecIFCObjectsGroup[iIFCObject]->VBOOffset() = iVBOOffset;

	//		// Update min/max
	//		vecIFCObjectsGroup[iIFCObject]->CalculateMinMaxValues(pVertices + (iVBOOffset * GEOMETRY_VBO_VERTEX_LENGTH), vecIFCObjectsGroup[iIFCObject]->verticesCount());

	//		// Update minimum bounding box
	//		vecIFCObjectsGroup[iIFCObject]->UpdateBBox(fXmin, fXmax, fYmin, fYmax, fZmin, fZmax, fBoundingSphereDiameter);

	//		iVBOOffset += (GLsizei)vecIFCObjectsGroup[iIFCObject]->verticesCount();
	//	} // for (size_t iIFCObject = ...

	//	delete[] pVertices;

	//	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//	_oglUtils::checkForErrors();

	//	CIFCDrawMetaData* pDrawMetaData = new CIFCDrawMetaData();
	//	pDrawMetaData->AddGroup(iVBO, vecIFCObjectsGroup);

	//	m_vecIFCDrawMetaData.push_back(pDrawMetaData);

	//	iVerticesCount = 0;
	//	vecIFCObjectsGroup.clear();
	//} // if (iVerticesCount > 0)

	///*
	//* IBO - Materials
	//*/
	//if (iFacesIndicesCount > 0)
	//{
	//	assert(!vecIFCMaterialsGroup.empty());

	//	GLuint iIBO = 0;
	//	glGenBuffers(1, &iIBO);

	//	ASSERT(iIBO != 0);

	//	m_vecIBOs.push_back(iIBO);

	//	int_t iGroupIndicesCount = 0;
	//	unsigned int* pIndices = pModel->GetMaterialsIndices(vecIFCMaterialsGroup, iGroupIndicesCount);
	//	if ((iGroupIndicesCount == 0) || (pIndices == nullptr))
	//	{
	//		ASSERT(0);

	//		return;
	//	}

	//	ASSERT(iFacesIndicesCount == iGroupIndicesCount);

	//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iIBO);
	//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * iFacesIndicesCount, pIndices, GL_STATIC_DRAW);

	//	delete[] pIndices;

	//	/*
	//	* Store IBO/offset
	//	*/
	//	GLsizei iIBOOffset = 0;
	//	for (size_t iMaterial2 = 0; iMaterial2 < vecIFCMaterialsGroup.size(); iMaterial2++)
	//	{
	//		vecIFCMaterialsGroup[iMaterial2]->IBO() = iIBO;
	//		vecIFCMaterialsGroup[iMaterial2]->IBOOffset() = iIBOOffset;

	//		iIBOOffset += (GLsizei)vecIFCMaterialsGroup[iMaterial2]->getIndicesCount();
	//	}

	//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//	_oglUtils::checkForErrors();

	//	iFacesIndicesCount = 0;
	//	vecIFCMaterialsGroup.clear();
	//} // if (iFacesIndicesCount > 0)

	///*
	//* IBO - Lines
	//*/
	//if (iLinesIndicesCount > 0)
	//{
	//	assert(!vecLinesCohorts.empty());

	//	GLuint iIBO = 0;
	//	glGenBuffers(1, &iIBO);

	//	ASSERT(iIBO != 0);

	//	m_vecIBOs.push_back(iIBO);

	//	int_t iCohortIndicesCount = 0;
	//	unsigned int* pIndices = pModel->GetLinesCohortsIndices(vecLinesCohorts, iCohortIndicesCount);
	//	if ((iCohortIndicesCount == 0) || (pIndices == NULL))
	//	{
	//		ASSERT(0);

	//		return;
	//	}

	//	ASSERT(iLinesIndicesCount == iCohortIndicesCount);


	//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iIBO);
	//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * iLinesIndicesCount, pIndices, GL_STATIC_DRAW);

	//	delete[] pIndices;

	//	/*
	//	* Store IBO/offset
	//	*/
	//	GLsizei iIBOOffset = 0;
	//	for (size_t iLinesCohort2 = 0; iLinesCohort2 < vecLinesCohorts.size(); iLinesCohort2++)
	//	{
	//		vecLinesCohorts[iLinesCohort2]->IBO() = iIBO;
	//		vecLinesCohorts[iLinesCohort2]->IBOOffset() = iIBOOffset;

	//		iIBOOffset += (GLsizei)vecLinesCohorts[iLinesCohort2]->getIndicesCount();
	//	} // for (size_t iLinesCohort2 = ...				

	//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//	_oglUtils::checkForErrors();

	//	iLinesIndicesCount = 0;
	//	vecLinesCohorts.clear();
	//} // if (iLinesIndicesCount > 0)		

	///*
	//* IBO - Wireframes
	//*/
	//if (iWireframesIndicesCount > 0)
	//{
	//	assert(!vecWireframesCohorts.empty());

	//	GLuint iIBO = 0;
	//	glGenBuffers(1, &iIBO);

	//	ASSERT(iIBO != 0);

	//	m_vecIBOs.push_back(iIBO);

	//	int_t iCohortIndicesCount = 0;
	//	unsigned int* pIndices = pModel->GetWireframesCohortsIndices(vecWireframesCohorts, iCohortIndicesCount);
	//	if ((iCohortIndicesCount == 0) || (pIndices == NULL))
	//	{
	//		ASSERT(0);

	//		return;
	//	}

	//	ASSERT(iWireframesIndicesCount == iCohortIndicesCount);


	//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iIBO);
	//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * iWireframesIndicesCount, pIndices, GL_STATIC_DRAW);

	//	delete[] pIndices;

	//	/*
	//	* Store IBO/offset
	//	*/
	//	GLsizei iIBOOffset = 0;
	//	for (size_t iWireframesCohort2 = 0; iWireframesCohort2 < vecWireframesCohorts.size(); iWireframesCohort2++)
	//	{
	//		vecWireframesCohorts[iWireframesCohort2]->IBO() = iIBO;
	//		vecWireframesCohorts[iWireframesCohort2]->IBOOffset() = iIBOOffset;

	//		iIBOOffset += (GLsizei)vecWireframesCohorts[iWireframesCohort2]->getIndicesCount();
	//	} // for (size_t iWireframesCohort2 = ...				

	//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//	_oglUtils::checkForErrors();

	//	iWireframesIndicesCount = 0;
	//	vecWireframesCohorts.clear();
	//} // if (iWireframesIndicesCount > 0)

//	CTime tmBuildOGLBuffersEnd = CTime::GetCurrentTime();

	// #####pController->FireOnProgress(90, 0, 0);

//	CTimeSpan tsBuildOGLBuffers = tmBuildOGLBuffersEnd - tmBuildOGLBuffersStart;
//	AfxMessageBox(CString(_T("Building openGL buffers: ")) + tsBuildOGLBuffers.Format(_T("Total days: %D, hours: %H, mins: %M, secs: %S")));

	m_pWnd->RedrawWindow();
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void COpenGLIFCView::Draw(CDC* pDC)
{
	CRect rcClient;
	m_pWnd->GetClientRect(&rcClient);

	int iWidth = rcClient.Width();
	int iHeight = rcClient.Height();

	if ((iWidth < 20) || (iHeight < 20))
	{
		return;
	}

//	BOOL bResult = m_pOGLContext->MakeCurrent();
//	VERIFY(bResult);
//
//#ifdef _ENABLE_OPENGL_DEBUG
//	m_pOGLContext->EnableDebug();
//#endif
//
//	m_pProgram->Use();
//
//	glViewport(0, 0, iWidth, iHeight);
//
//	glClearColor(1.0, 1.0, 1.0, 1.0);
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//	// Set up the parameters
//	glEnable(GL_DEPTH_TEST);
//	glDepthFunc(GL_LEQUAL);
//
//	glShadeModel(GL_SMOOTH);
//
//	/*
//	* Light
//	*/
//	glProgramUniform3f(
//		m_pProgram->GetID(),
//		m_pProgram->getPointLightingLocation(),
//		0.f,
//		0.f,
//		10000.f);
//
//	/*
//	* Shininess
//	*/
//	glProgramUniform1f(
//		m_pProgram->GetID(),
//		m_pProgram->getMaterialShininess(),
//		30.f);
//
//	/*
//	* Projection Matrix
//	*/
//	// fovY     - Field of vision in degrees in the y direction
//	// aspect   - Aspect ratio of the viewport
//	// zNear    - The near clipping distance
//	// zFar     - The far clipping distance
//	GLdouble fovY = 45.0;
//	GLdouble aspect = (GLdouble)iWidth / (GLdouble)iHeight;
//	GLdouble zNear = 0.001;
//	GLdouble zFar = 1000000.0;
//
//	GLdouble fH = tan(fovY / 360 * M_PI) * zNear;
//	GLdouble fW = fH * aspect;
//
//	switch (m_enProjectionType)
//	{
//		case ptPerspective:
//		{
//			glFrustum(-fW, fW, -fH, fH, zNear, zFar);
//		}
//		break;
//
//		case ptIsometric:
//		{
//			glOrtho(-1.5, 1.5, -1.5, 1.5, zNear, zFar);
//		}
//		break;
//
//		default:
//		{
//			ASSERT(FALSE);
//		}
//		break;
//	}
//
//	glm::mat4 projectionMatrix = glm::frustum<GLdouble>(-fW, fW, -fH, fH, zNear, zFar);
//
//	glProgramUniformMatrix4fv(
//		m_pProgram->GetID(),
//		m_pProgram->getPMatrix(),
//		1,
//		false,
//		value_ptr(projectionMatrix));
//
//	/*
//	* Model-View Matrix
//	*/
//	m_modelViewMatrix = glm::identity<glm::mat4>();
//	m_modelViewMatrix = glm::translate(m_modelViewMatrix, glm::vec3(m_dXTranslation, m_dYTranslation, m_dZTranslation));
//
//	m_modelViewMatrix = glm::translate(m_modelViewMatrix, glm::vec3(m_dOriginX, m_dOriginY, m_dOriginZ));
//
//	m_modelViewMatrix = glm::rotate(m_modelViewMatrix, (float)m_dXAngle, glm::vec3(1.0f, 0.0f, 0.0f));
//	m_modelViewMatrix = glm::rotate(m_modelViewMatrix, (float)m_dYAngle, glm::vec3(0.0f, 1.0f, 0.0f));
//
//	m_modelViewMatrix = glm::translate(m_modelViewMatrix, glm::vec3(-m_dOriginX, -m_dOriginY, -m_dOriginZ));
//
//	glProgramUniformMatrix4fv(
//		m_pProgram->GetID(),
//		m_pProgram->getMVMatrix(),
//		1,
//		false,
//		glm::value_ptr(m_modelViewMatrix));
//
//	/*
//	* Normal Matrix
//	*/
//	glm::mat4 normalMatrix = m_modelViewMatrix;
//	normalMatrix = glm::inverse(normalMatrix);
//	normalMatrix = glm::transpose(normalMatrix);
//
//	glProgramUniformMatrix4fv(
//		m_pProgram->GetID(),
//		m_pProgram->getNMatrix(),
//		1,
//		false,
//		value_ptr(normalMatrix));

	/*
	* Non-transparent faces
	*/
	//DrawFaces(false);

	/*
	* Transparent faces
	*/
	//DrawFaces(true);	

	/*
	* Wireframes
	*/
	//DrawWireframes();

	/*
	* Lines
	*/
	//DrawLines();

	/*
	* Scene
	*/
	//DrawScene((float)ARROW_SIZE_I, (float)ARROW_SIZE_II);	

	SwapBuffers(*pDC);

	/*
	* Selection support
	*/
	//DrawFacesFrameBuffer();
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

				m_pWnd->RedrawWindow();

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

	m_pWnd->RedrawWindow();
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
	ASSERT(pController != NULL);

	if (pController->GetModel() == nullptr)
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

	if (bTransparent)
	{
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	//glProgramUniform1f(
	//	m_pProgram->GetID(),
	//	m_pProgram->geUseBinnPhongModel(),
	//	1.f);

	//for (size_t iDrawMetaData = 0; iDrawMetaData < m_vecIFCDrawMetaData.size(); iDrawMetaData++)
	//{
	//	const map<GLuint, vector<CIFCObject*>>& mapGroups = m_vecIFCDrawMetaData[iDrawMetaData]->getGroups();

	//	map<GLuint, vector<CIFCObject*>>::const_iterator itGroups = mapGroups.begin();
	//	for (; itGroups != mapGroups.end(); itGroups++)
	//	{
	//		GLsizei iOffset = 0;
	//		for (size_t iObject = 0; iObject < itGroups->second.size(); iObject++)
	//		{
	//			CIFCObject* pIFCObject = itGroups->second[iObject];
	//			if (!pIFCObject->visible__() || !pIFCObject->AreFacesShown() ||
	//				(m_bDetailsViewMode ? pModel->getSubSelection() != NULL ? pModel->getSubSelection() != pIFCObject : !pIFCObject->selected() : false))
	//			{
	//				iOffset += (GLsizei)pIFCObject->verticesCount();

	//				continue;
	//			}

	//			/*
	//			* Conceptual faces
	//			*/
	//			for (size_t iGeometryWithMaterial = 0; iGeometryWithMaterial < pIFCObject->conceptualFacesMaterials().size(); iGeometryWithMaterial++)
	//			{
	//				CIFCGeometryWithMaterial* pGeometryWithMaterial = pIFCObject->conceptualFacesMaterials()[iGeometryWithMaterial];

	//				const CIFCMaterial* pMaterial =
	//					/*Picked?*/ (pModel == m_pPickedIFCObjectModel) && (pIFCObject == m_pPickedIFCObject) ? pModel->getOnMouseOverMaterial()
	//					/*Selected?*/ : pIFCObject->selected() && !m_bDetailsViewMode ? pModel->getSelectionMaterial() :
	//					/*User-defined?*/pIFCObject->getUserDefinedMaterial() != NULL ? pIFCObject->getUserDefinedMaterial() :
	//					/*Use conceptual face material*/pGeometryWithMaterial->getMaterial();

	//				if (bTransparent)
	//				{
	//					if (pMaterial->A() == 1.0)
	//					{
	//						continue;
	//					}
	//				}
	//				else
	//				{
	//					if (pMaterial->A() < 1.0)
	//					{
	//						continue;
	//					}
	//				}

	//				/*
	//				* VBO
	//				*/
	//				glBindBuffer(GL_ARRAY_BUFFER, itGroups->first);
	//				glVertexAttribPointer(m_pProgram->getVertexPosition(), 3, GL_FLOAT, false, sizeof(GLfloat) * GEOMETRY_VBO_VERTEX_LENGTH, 0);
	//				glEnableVertexAttribArray(m_pProgram->getVertexPosition());
	//				glVertexAttribPointer(m_pProgram->getVertexNormal(), 3, GL_FLOAT, false, sizeof(GLfloat) * GEOMETRY_VBO_VERTEX_LENGTH, (void*)(sizeof(GLfloat) * 3));
	//				glEnableVertexAttribArray(m_pProgram->getVertexNormal());

	//				/*
	//				* Material - Ambient color
	//				*/
	//				glProgramUniform3f(m_pProgram->GetID(),
	//					m_pProgram->getMaterialAmbientColor(),
	//					pMaterial->getAmbientColor().R(),
	//					pMaterial->getAmbientColor().G(),
	//					pMaterial->getAmbientColor().B());

	//				/*
	//				* Material - Transparency
	//				*/
	//				glProgramUniform1f(
	//					m_pProgram->GetID(),
	//					m_pProgram->getTransparency(),
	//					pMaterial->A());

	//				/*
	//				* Material - Diffuse color
	//				*/
	//				glProgramUniform3f(m_pProgram->GetID(),
	//					m_pProgram->getMaterialDiffuseColor(),
	//					pMaterial->getDiffuseColor().R() / 2.f,
	//					pMaterial->getDiffuseColor().G() / 2.f,
	//					pMaterial->getDiffuseColor().B() / 2.f);

	//				/*
	//				* Material - Specular color
	//				*/
	//				glProgramUniform3f(m_pProgram->GetID(),
	//					m_pProgram->getMaterialSpecularColor(),
	//					pMaterial->getSpecularColor().R() / 2.f,
	//					pMaterial->getSpecularColor().G() / 2.f,
	//					pMaterial->getSpecularColor().B() / 2.f);

	//				/*
	//				* Material - Emissive color
	//				*/
	//				glProgramUniform3f(m_pProgram->GetID(),
	//					m_pProgram->getMaterialEmissiveColor(),
	//					pMaterial->getEmissiveColor().R() / 3.f,
	//					pMaterial->getEmissiveColor().G() / 3.f,
	//					pMaterial->getEmissiveColor().B() / 3.f);

	//				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pGeometryWithMaterial->IBO());
	//				
	//				glDrawElementsBaseVertex(GL_TRIANGLES,
	//					(GLsizei)pGeometryWithMaterial->getIndicesCount(),
	//					GL_UNSIGNED_INT,
	//					(void*)(sizeof(GLuint) * pGeometryWithMaterial->IBOOffset()),
	//					iOffset);
	//				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//			} // for (size_t iMaterial = ...

	//			iOffset += (GLsizei)pIFCObject->verticesCount();
	//		} // for (size_t iObject = ...

	//		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//		glBindBuffer(GL_ARRAY_BUFFER, 0);
	//	} // for (; itGroups != ...
	//} // for (size_t iDrawMetaData = ...

	//glDisableVertexAttribArray(m_pProgram->getVertexNormal());

	//if (bTransparent)
	//{
	//	glDisable(GL_BLEND);
	//}

	_oglUtils::checkForErrors();
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::DrawLines()
{
	if (!m_bShowLines)
	{
		return;
	}

	auto pController = GetController();
	ASSERT(pController != NULL);

	if (pController->GetModel() == nullptr)
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

	//for (size_t iDrawMetaData = 0; iDrawMetaData < m_vecIFCDrawMetaData.size(); iDrawMetaData++)
	//{
	//	const map<GLuint, vector<CIFCObject*>>& mapGroups = m_vecIFCDrawMetaData[iDrawMetaData]->getGroups();

	//	map<GLuint, vector<CIFCObject*>>::const_iterator itGroups = mapGroups.begin();
	//	for (; itGroups != mapGroups.end(); itGroups++)
	//	{
	//		GLsizei iOffset = 0;
	//		for (size_t iObject = 0; iObject < itGroups->second.size(); iObject++)
	//		{
	//			CIFCObject* pIFCObject = itGroups->second[iObject];
	//			if (!pIFCObject->visible__() || !pIFCObject->AreLinesShown() ||
	//				(m_bDetailsViewMode ? pModel->getSubSelection() != NULL ? pModel->getSubSelection() != pIFCObject : !pIFCObject->selected() : false))
	//			{
	//				iOffset += (GLsizei)pIFCObject->verticesCount();

	//				continue;
	//			}

	//			/*
	//			* VBO
	//			*/
	//			glBindBuffer(GL_ARRAY_BUFFER, itGroups->first);
	//			glVertexAttribPointer(m_pProgram->getVertexPosition(), 3, GL_FLOAT, false, sizeof(GLfloat) * GEOMETRY_VBO_VERTEX_LENGTH, 0);
	//			glEnableVertexAttribArray(m_pProgram->getVertexPosition());

	//			/*
	//			* Lines
	//			*/
	//			for (size_t iLinesCohort = 0; iLinesCohort < pIFCObject->linesCohorts().size(); iLinesCohort++)
	//			{
	//				CLinesCohort* pLinesCohort = pIFCObject->linesCohorts()[iLinesCohort];

	//				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pLinesCohort->IBO());

	//				glDrawElementsBaseVertex(GL_LINES,
	//					(GLsizei)pLinesCohort->getIndicesCount(),
	//					GL_UNSIGNED_INT,
	//					(void*)(sizeof(GLuint) * pLinesCohort->IBOOffset()),
	//					iOffset);
	//				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//			} // for (size_t iLinesCohort = ...

	//			iOffset += (GLsizei)pIFCObject->verticesCount();
	//		} // for (size_t iObject = ...

	//		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//		glBindBuffer(GL_ARRAY_BUFFER, 0);
	//	} // for (; itGroups != ...
	//} // for (size_t iDrawMetaData = ...

	_oglUtils::checkForErrors();
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::DrawConceptualFacesPolygons()
{
	if (!m_bShowConceptualFacesPolygons)
	{
		return;
	}	

	auto pController = GetController();
	ASSERT(pController != NULL);

	if (pController->GetModel() == nullptr)
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

	//for (size_t iDrawMetaData = 0; iDrawMetaData < m_vecIFCDrawMetaData.size(); iDrawMetaData++)
	//{
	//	const map<GLuint, vector<CIFCObject*>>& mapGroups = m_vecIFCDrawMetaData[iDrawMetaData]->getGroups();

	//	map<GLuint, vector<CIFCObject*>>::const_iterator itGroups = mapGroups.begin();
	//	for (; itGroups != mapGroups.end(); itGroups++)
	//	{
	//		GLsizei iOffset = 0;
	//		for (size_t iObject = 0; iObject < itGroups->second.size(); iObject++)
	//		{
	//			CIFCObject* pIFCObject = itGroups->second[iObject];
	//			if (!pIFCObject->visible__() || !pIFCObject->AreFacesShown() ||
	//				(m_bDetailsViewMode ? pModel->getSubSelection() != NULL ? pModel->getSubSelection() != pIFCObject : !pIFCObject->selected() : false))
	//			{
	//				iOffset += (GLsizei)pIFCObject->verticesCount();

	//				continue;
	//			}

	//			/*
	//			* VBO
	//			*/
	//			glBindBuffer(GL_ARRAY_BUFFER, itGroups->first);
	//			glVertexAttribPointer(m_pProgram->getVertexPosition(), 3, GL_FLOAT, false, sizeof(GLfloat) * GEOMETRY_VBO_VERTEX_LENGTH, 0);
	//			glEnableVertexAttribArray(m_pProgram->getVertexPosition());

	//			/*
	//			* Wireframes
	//			*/
	//			for (size_t iWireframesCohort = 0; iWireframesCohort < pIFCObject->wireframesCohorts().size(); iWireframesCohort++)
	//			{
	//				CWireframesCohort* pWireframesCohort = pIFCObject->wireframesCohorts()[iWireframesCohort];

	//				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pWireframesCohort->IBO());

	//				glDrawElementsBaseVertex(GL_LINES,
	//					(GLsizei)pWireframesCohort->getIndicesCount(),
	//					GL_UNSIGNED_INT,
	//					(void*)(sizeof(GLuint) * pWireframesCohort->IBOOffset()),
	//					iOffset);
	//				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//			} // for (size_t iWireframesCohort = ...

	//			/*
	//			* Picked edge
	//			*/
	//			//if ((m_enViewMode == vmMeasureEdge) && (pIFCObject == m_pPickedIFCObject) && !m_setPickedIFCObjectEdges.empty())
	//			//{
	//			//	glDisable(GL_DEPTH_TEST);

	//			//	for (size_t iWireframesCohort = 0; iWireframesCohort < pIFCObject->wireframesCohorts().size(); iWireframesCohort++)
	//			//	{
	//			//		CWireframesCohort* pWireframesCohort = pIFCObject->wireframesCohorts()[iWireframesCohort];

	//			//		for (int_t iLine = 0; iLine < pWireframesCohort->getIndicesCount() / 2; iLine++)
	//			//		{
	//			//			set<int_t>::iterator itEdge = m_setPickedIFCObjectEdges.find(iLineID++);
	//			//			if (itEdge != m_setPickedIFCObjectEdges.end())
	//			//			{
	//			//				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pWireframesCohort->IBO());

	//			//				glLineWidth(3.0);
	//			//				glColor4f(1.f, 0.f, 0.f, 1.0);

	//			//				/*
	//			//				* Draw
	//			//				*/
	//			//				glDrawElementsBaseVertex(GL_LINES,
	//			//					(GLsizei)2,
	//			//					GL_UNSIGNED_INT,
	//			//					(void*)(sizeof(GLuint) * (pWireframesCohort->IBOOffset() + (iLine * 2))),
	//			//					iOffset);

	//			//				/*
	//			//				* Read the indices
	//			//				*/
	//			//				/*GLuint indices[2];
	//			//				glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER,
	//			//					sizeof(GLuint) * (pWireframesCohort->IBOOffset() + (iLine * 2)),
	//			//					2 * sizeof(GLuint),
	//			//					indices);*/

	//			//					/*
	//			//					* Read the vertices
	//			//					*/
	//			//					/*GLfloat vertex1[GEOMETRY_VBO_VERTEX_LENGTH];
	//			//					glGetBufferSubData(GL_ARRAY_BUFFER,
	//			//						sizeof(GLfloat) * ((iOffset + indices[0]) * GEOMETRY_VBO_VERTEX_LENGTH),
	//			//						GEOMETRY_VBO_VERTEX_LENGTH * sizeof(GLfloat), vertex1);

	//			//					GLfloat vertex2[GEOMETRY_VBO_VERTEX_LENGTH];
	//			//					glGetBufferSubData(GL_ARRAY_BUFFER,
	//			//						sizeof(GLfloat) * ((iOffset + indices[1]) * GEOMETRY_VBO_VERTEX_LENGTH),
	//			//						GEOMETRY_VBO_VERTEX_LENGTH * sizeof(GLfloat), vertex2);*/

	//			//						//double dLength = sqrt(pow(vertex1[0] - vertex2[0], 2.) + pow(vertex1[1] - vertex2[1], 2.) + pow(vertex1[2] - vertex2[2], 2.));
	//			//						//dLength = (m_dScaleFactor * dLength) / 2.;

	//			//						//dSum += dLength;

	//			//				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//			//				glLineWidth(1.0);
	//			//				glColor4f(0.f, 0.f, 0.f, 1.0);
	//			//			} // if (itEdge != ...
	//			//		} // for (int_t iLine = ...
	//			//	} // for (size_t iWireframesCohort = ...

	//			//	glEnable(GL_DEPTH_TEST);
	//			//} // if ((m_enViewMode == vmMeasureEdge) && ...

	//			iOffset += (GLsizei)pIFCObject->verticesCount();
	//		} // for (size_t iObject = ...

	//		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//		glBindBuffer(GL_ARRAY_BUFFER, 0);
	//	} // for (; itGroups != ...
	//} // for (size_t iDrawMetaData = ...

	_oglUtils::checkForErrors();
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::DrawFacesFrameBuffer()
{
	if (!m_bShowFaces)
	{
		return;
	}	

	//auto pController = GetController();
	//ASSERT(pController != NULL);

	//if (pController->GetModel() == nullptr)
	//{
	//	ASSERT(FALSE);

	//	return;
	//}

	//auto pModel = pController->GetModel()->As<CIFCModel>();
	//if (pModel == nullptr)
	//{
	//	ASSERT(FALSE);

	//	return;
	//}

	//CRect rcClient;
	//m_pWnd->GetClientRect(&rcClient);

	//if ((rcClient.Width() < 100) || (rcClient.Height() < 100))
	//{
	//	return;
	//}

	///*
	//* Frame buffer
	//*/
	//if (m_iSelectionFrameBuffer == 0)
	//{
	//	assert(m_iSelectionTextureBuffer == 0);
	//	assert(m_iSelectionDepthRenderBuffer == 0);

	//	/*
	//	* Frame buffer
	//	*/
	//	glGenFramebuffers(1, &m_iSelectionFrameBuffer);
	//	assert(m_iSelectionFrameBuffer != 0);

	//	glBindFramebuffer(GL_FRAMEBUFFER, m_iSelectionFrameBuffer);

	//	/*
	//	* Texture buffer
	//	*/
	//	glGenTextures(1, &m_iSelectionTextureBuffer);
	//	assert(m_iSelectionTextureBuffer != 0);

	//	glBindTexture(GL_TEXTURE_2D, m_iSelectionTextureBuffer);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SELECTION_BUFFER_SIZE, SELECTION_BUFFER_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	//	glBindTexture(GL_TEXTURE_2D, 0);

	//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_iSelectionTextureBuffer, 0);

	//	/*
	//	* Depth buffer
	//	*/
	//	glGenRenderbuffers(1, &m_iSelectionDepthRenderBuffer);
	//	assert(m_iSelectionDepthRenderBuffer != 0);

	//	glBindRenderbuffer(GL_RENDERBUFFER, m_iSelectionDepthRenderBuffer);
	//	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SELECTION_BUFFER_SIZE, SELECTION_BUFFER_SIZE);

	//	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	//	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_iSelectionDepthRenderBuffer);

	//	GLenum arDrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	//	glDrawBuffers(1, arDrawBuffers);

	//	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//	_oglUtils::checkForErrors();
	//} // if (m_iSelectionFrameBuffer == 0)

	/*
	* Scene
	*/
	//glBindFramebuffer(GL_FRAMEBUFFER, m_iSelectionFrameBuffer);

	//glViewport(0, 0, SELECTION_BUFFER_SIZE, SELECTION_BUFFER_SIZE);

	//glClearColor(0.0, 0.0, 0.0, 0.0);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set up the parameters
	//glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LEQUAL);	

	//glProgramUniform1f(
	//	m_pProgram->GetID(),
	//	m_pProgram->geUseBinnPhongModel(),
	//	0.f);

	//glProgramUniform1f(
	//	m_pProgram->GetID(),
	//	m_pProgram->getTransparency(),
	//	1.f);

	//for (size_t iDrawMetaData = 0; iDrawMetaData < m_vecIFCDrawMetaData.size(); iDrawMetaData++)
	//{
	//	const map<GLuint, vector<CIFCObject*>>& mapGroups = m_vecIFCDrawMetaData[iDrawMetaData]->getGroups();

	//	map<GLuint, vector<CIFCObject*>>::const_iterator itGroups = mapGroups.begin();
	//	for (; itGroups != mapGroups.end(); itGroups++)
	//	{
	//		GLsizei iOffset = 0;
	//		for (size_t iObject = 0; iObject < itGroups->second.size(); iObject++)
	//		{
	//			CIFCObject* pIFCObject = itGroups->second[iObject];
	//			if (!pIFCObject->visible__() || !pIFCObject->AreFacesShown() ||
	//				(m_bDetailsViewMode ? pModel->getSubSelection() != NULL ? pModel->getSubSelection() != pIFCObject : !pIFCObject->selected() : false))
	//			{
	//				iOffset += (GLsizei)pIFCObject->verticesCount();

	//				continue;
	//			}

	//			glBindBuffer(GL_ARRAY_BUFFER, itGroups->first);
	//			glVertexAttribPointer(m_pProgram->getVertexPosition(), 3, GL_FLOAT, false, sizeof(GLfloat) * GEOMETRY_VBO_VERTEX_LENGTH, 0);
	//			glEnableVertexAttribArray(m_pProgram->getVertexPosition());

	//			/*
	//			* Conceptual faces
	//			*/
	//			for (size_t iMaterial = 0; iMaterial < pIFCObject->conceptualFacesMaterials().size(); iMaterial++)
	//			{
	//				CIFCGeometryWithMaterial* pGeometryWithMaterial = pIFCObject->conceptualFacesMaterials()[iMaterial];

	//				/*
	//				* Ambient color
	//				*/
	//				/*
	//				* Material - Ambient color
	//				*/
	//				glProgramUniform3f(
	//					m_pProgram->GetID(),
	//					m_pProgram->getMaterialAmbientColor(),
	//					pIFCObject->rgbID()->R(),
	//					pIFCObject->rgbID()->G(),
	//					pIFCObject->rgbID()->B());

	//				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pGeometryWithMaterial->IBO());

	//				glDrawElementsBaseVertex(GL_TRIANGLES,
	//					(GLsizei)pGeometryWithMaterial->getIndicesCount(),
	//					GL_UNSIGNED_INT,
	//					(void*)(sizeof(GLuint) * pGeometryWithMaterial->IBOOffset()),
	//					iOffset);
	//				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//			} // for (size_t iMaterial = ...

	//			iOffset += (GLsizei)pIFCObject->verticesCount();
	//		} // for (size_t iObject = ...

	//		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//		glBindBuffer(GL_ARRAY_BUFFER, 0);
	//	} // for (; itGroups != ...
	//} // for (size_t iDrawMetaData = ...

	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

	_oglUtils::checkForErrors();
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::OnMouseMoveEvent(UINT nFlags, CPoint point)
{
	auto pController = GetController();
	ASSERT(pController != NULL);

	if (pController->GetModel() == nullptr)
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
	//if (m_bShowFaces && (m_iSelectionFrameBuffer != 0))
	//{
	//	//m_pOGLContext->MakeCurrent();

	//	CRect rcClient;
	//	m_pWnd->GetClientRect(&rcClient);

	//	glBindFramebuffer(GL_FRAMEBUFFER, m_iSelectionFrameBuffer);
	//	
	//	GLubyte arPixels[4];
	//	memset(arPixels, 0, sizeof(GLubyte) * 4);

	//	double dX = (double)point.x * ((double)SELECTION_BUFFER_SIZE / (double)rcClient.Width());
	//	double dY = ((double)rcClient.Height() - (double)point.y) * ((double)SELECTION_BUFFER_SIZE / (double)rcClient.Height());

	//	CIFCModel * pPickedIFCObjectModel = NULL;
	//	CIFCObject * pPickedIFCObject = NULL;

	//	glReadPixels(
	//		(GLint)dX,
	//		(GLint)dY,
	//		1, 1,
	//		GL_RGBA,
	//		GL_UNSIGNED_BYTE,
	//		arPixels);

	//	if (arPixels[3] != 0)
	//	{
	//		int_t iObjectID =
	//			(arPixels[0/*R*/] * (255 * 255)) +
	//			(arPixels[1/*G*/] * 255) +
	//			 arPixels[2/*B*/];

	//		pPickedIFCObjectModel = pModel;
	//		pPickedIFCObject = pModel->getIFCObject(iObjectID);
	//		
	//		ASSERT(pPickedIFCObjectModel != NULL);
	//		ASSERT(pPickedIFCObject != NULL);
	//	} // if (arPixels[3] != 0)

	//	glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//if ((m_pPickedIFCObjectModel != pPickedIFCObjectModel) || (m_pPickedIFCObject != pPickedIFCObject))
		//{
		//	m_pPickedIFCObjectModel = pPickedIFCObjectModel;
		//	m_pPickedIFCObject = pPickedIFCObject;
		//	m_setPickedIFCObjectEdges.clear();
		//	m_iPickedIFCObjectFace = -1;
		//	
		//	/*if ((m_pPickedIFCObject != NULL) && (m_enViewMode == vmMeasureVolume))
		//	{
		//		double dVolume = m_pPickedIFCObject->volume();
		//		GetController()->FireOnMeasureVolumeEvent(point.x, point.y, dVolume);

		//		CString strTooltip;
		//		strTooltip.Format(_T("%.2f"), dVolume);

		//		CString strTitle;
		//		VERIFY(strTitle.LoadStringW(IDS_VOLUME));

		//		const CIFCUnit * pUnit = m_pPickedIFCObjectModel->getUnit(L"VOLUMEUNIT");
		//		if ((pUnit != NULL) && !pUnit->getUnit().empty())
		//		{
		//			strTooltip += L" ";
		//			strTooltip += pUnit->getUnit().c_str();
		//		}

		//		ShowTooltip(strTitle, strTooltip);
		//	}*/

		//	m_pWnd->RedrawWindow();
		//}
	//} // if (m_bShowFaces && ...

	/*
	* Measures - Edges (Conceptual Faces Polygons)
	*/
	//if ((m_enViewMode == vmMeasureEdge) && !m_bInteractionInProgress && (m_pPickedIFCObjectModel != NULL) && (m_pPickedIFCObject != NULL))
	//{
	//	m_pOGLContext->MakeCurrent();

	//	CRect rcClient;
	//	m_pWnd->GetClientRect(&rcClient);

	//	glBindFramebuffer(GL_FRAMEBUFFER, m_iWireframesFrameBuffer);

	//	// Region - width
	//	const int REGION_WIDTH = 20;
	//	// Region - height
	//	const int REGION_HEIGHT = 10;
	//	// Region
	//	const int REGION_SIZE = REGION_WIDTH * REGION_HEIGHT;

	//	GLubyte arPixels[4 * REGION_SIZE];
	//	memset(arPixels, 0, sizeof(GLubyte) * 4 * REGION_SIZE);

	//	int x = point.x;
	//	int y = point.y;

	//	// Center of the region
	//	x -= REGION_WIDTH / 2;
	//	y += REGION_HEIGHT / 2;

	//	double dX = (double)x * ((double)SELECTION_BUFFER_SIZE / (double)rcClient.Width());		
	//	double dY = ((double)rcClient.Height() - (double)y) * ((double)SELECTION_BUFFER_SIZE / (double)rcClient.Height());

	//	glReadPixels(
	//		(GLint)dX,
	//		(GLint)dY,
	//		REGION_WIDTH, REGION_HEIGHT,
	//		GL_RGBA,
	//		GL_UNSIGNED_BYTE,
	//		arPixels);

	//	int_t iPickedIFCObjectEdge = -1;

	//	for (int iPixel = 0; iPixel < REGION_SIZE; iPixel++)
	//	{
	//		if ((arPixels[(iPixel * 4) + 3] == 255/*conceptual face or edge*/) &&
	//			((arPixels[(iPixel * 4) + 0] != 0) || (arPixels[(iPixel * 4) + 1] != 0) || (arPixels[(iPixel * 4) + 2] != 0)))
	//		{
	//			iPickedIFCObjectEdge = 
	//				(arPixels[(iPixel * 4) + 0/*R*/] * (255 * 255)) +
	//				(arPixels[(iPixel * 4) + 1/*G*/] * 255) +
	//				arPixels[(iPixel * 4) + 2/*B*/];

	//			break;
	//		} // if ((arPixels[3] == 255) ...
	//	} // for (int iPixel = ...

	//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//	
	//	if (iPickedIFCObjectEdge != -1)
	//	{
	//		ASSERT(iPickedIFCObjectEdge > 0); // Expecting 1-based index

	//		if (GetKeyState(VK_CONTROL) & 0x8000)
	//		{
	//			set<int_t>::iterator itEdge = m_setPickedIFCObjectEdges.find(iPickedIFCObjectEdge);
	//			if (itEdge == m_setPickedIFCObjectEdges.end())
	//			{
	//				m_setPickedIFCObjectEdges.insert(iPickedIFCObjectEdge);

	//				m_pWnd->RedrawWindow();

	//				GetController()->FireOnMeasureEdgeEvent(point.x, point.y, GetPickedEdgesLength());

	//				CString strTooltip;
	//				strTooltip.Format(_T("%.2f"), GetPickedEdgesLength());

	//				CString strTitle;
	//				VERIFY(strTitle.LoadStringW(IDS_LENGTH));

	//				const CIFCUnit * pUnit = m_pPickedIFCObjectModel->getUnit(L"LENGTHUNIT");
	//				if ((pUnit != NULL) && !pUnit->getUnit().empty())
	//				{
	//					strTooltip += L" ";
	//					strTooltip += pUnit->getUnit().c_str();
	//				}

	//				ShowTooltip(strTitle, strTooltip);
	//			}
	//		}
	//		else
	//		{
	//			m_setPickedIFCObjectEdges.clear();
	//			m_setPickedIFCObjectEdges.insert(iPickedIFCObjectEdge);

	//			m_pWnd->RedrawWindow();

	//			GetController()->FireOnMeasureEdgeEvent(point.x, point.y, GetPickedEdgesLength());

	//			CString strTooltip;
	//			strTooltip.Format(_T("%.2f"), GetPickedEdgesLength());

	//			CString strTitle;
	//			VERIFY(strTitle.LoadStringW(IDS_LENGTH));

	//			const CIFCUnit * pUnit = m_pPickedIFCObjectModel->getUnit(L"LENGTHUNIT");
	//			if ((pUnit != NULL) && !pUnit->getUnit().empty())
	//			{
	//				strTooltip += L" ";
	//				strTooltip += pUnit->getUnit().c_str();
	//			}

	//			ShowTooltip(strTitle, strTooltip);
	//		}
	//	} // if (iPickedIFCObjectEdge != -1)
	//} // if ((m_enViewMode == vmMeasureEdge) && ...

	/*
	* Measures - Area (Conceptual Faces)
	*/
	//if ((m_enViewMode == vmMeasureArea) && !m_bInteractionInProgress && (m_pPickedIFCObjectModel != NULL) && (m_pPickedIFCObject != NULL))
	//{
	//	m_pOGLContext->MakeCurrent();

	//	CRect rcClient;
	//	m_pWnd->GetClientRect(&rcClient);

	//	glBindFramebuffer(GL_FRAMEBUFFER, m_iFacesFrameBuffer);		

	//	GLubyte arPixels[4];
	//	memset(arPixels, 0, sizeof(GLubyte) * 4);

	//	double dX = (double)point.x * ((double)SELECTION_BUFFER_SIZE / (double)rcClient.Width());
	//	double dY = ((double)rcClient.Height() - (double)point.y) * ((double)SELECTION_BUFFER_SIZE / (double)rcClient.Height());

	//	glReadPixels(
	//		(GLint)dX,
	//		(GLint)dY,
	//		1, 1,
	//		GL_RGBA,
	//		GL_UNSIGNED_BYTE,
	//		arPixels);

	//	int_t iPickedIFCObjectFace = -1;

	//	if (arPixels[3] != 0)
	//	{
	//		iPickedIFCObjectFace =
	//			(arPixels[0/*R*/] * (255 * 255)) +
	//			(arPixels[1/*G*/] * 255) +
	//			arPixels[2/*B*/];
	//	} // if (arPixels[3] != 0)

	//	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//	if (m_iPickedIFCObjectFace != iPickedIFCObjectFace)
	//	{
	//		m_iPickedIFCObjectFace = iPickedIFCObjectFace;

	//		if (m_iPickedIFCObjectFace != -1)
	//		{
	//			ASSERT(m_iPickedIFCObjectFace > 0); // Expecting 1-based index
	//			TRACE(_T("\nFace: %d"), m_iPickedIFCObjectFace);

	//			double dArea = m_pPickedIFCObject->getConceptualFaceArea(m_iPickedIFCObjectFace - 1);
	//			GetController()->FireOnMeasureAreaEvent(point.x, point.y, dArea);

	//			CString strTooltip;
	//			strTooltip.Format(_T("%.2f"), dArea);

	//			CString strTitle;
	//			VERIFY(strTitle.LoadStringW(IDS_AREA));

	//			const CIFCUnit * pUnit = m_pPickedIFCObjectModel->getUnit(L"AREAUNIT");
	//			if ((pUnit != NULL) && !pUnit->getUnit().empty())
	//			{
	//				strTooltip += L" ";
	//				strTooltip += pUnit->getUnit().c_str();
	//			}

	//			ShowTooltip(strTitle, strTooltip);
	//		}

	//		m_pWnd->RedrawWindow();
	//	}
	//} // if ((m_enViewMode == vmMeasureArea) && ...

	//if (m_ptPrevMousePosition.x == -1)
	//{
	//	return;
	//}

	///*
	//* Rotate
	//*/
	//if ((nFlags & MK_LBUTTON) == MK_LBUTTON)
	//{
	//	double dXAngle = ((double)point.y - (double)m_ptPrevMousePosition.y);
	//	double dYAngle = ((double)point.x - (double)m_ptPrevMousePosition.x);

	//	const double ROTATION_SPEED = 1. / 350.;
	//	const double ROTATION_SENSITIVITY = 1.1;

	//	if (abs(dXAngle) >= abs(dYAngle) * ROTATION_SENSITIVITY)
	//	{
	//		dYAngle = 0.;
	//	}
	//	else
	//	{
	//		if (abs(dYAngle) >= abs(dXAngle) * ROTATION_SENSITIVITY)
	//		{
	//			dXAngle = 0.;
	//		}
	//	}

	//	Rotate(dXAngle * ROTATION_SPEED, dYAngle * ROTATION_SPEED);

	//	m_ptPrevMousePosition = point;

	//	return;
	//}

	///*
	//* Zoom
	//*/
	//if ((nFlags & MK_MBUTTON) == MK_MBUTTON)
	//{
	//	/*Zoom(point.y - m_ptPrevMousePosition.y > 0 ? -abs(m_dZTranslation) * ZOOM_SPEED_2 : abs(m_dZTranslation) * ZOOM_SPEED_2);

	//	m_ptPrevMousePosition = point;*/

	//	return;
	//}

	///*
	//* Move
	//*/
	//if ((nFlags & MK_RBUTTON) == MK_RBUTTON)
	//{
	//	/*CRect rcClient;
	//	m_pWnd->GetClientRect(&rcClient);

	//	m_dXTranslation += 4.f * (((double)point.x - (double)m_ptPrevMousePosition.x) / rcClient.Width());
	//	m_dYTranslation -= 4.f * (((double)point.y - (double)m_ptPrevMousePosition.y) / rcClient.Height());

	//	m_pWnd->RedrawWindow();

	//	m_ptPrevMousePosition = point;*/

	//	return;
	//}
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::Rotate(double dXSpin, double dYSpin)
{
	//// Rotate
	//m_dXAngle += dXSpin * (180. / M_PI);
	//if (m_dXAngle > 360.0)
	//{
	//	m_dXAngle = m_dXAngle - 360.0f;
	//}
	//else
	//{
	//	if (m_dXAngle < 0.0)
	//	{
	//		m_dXAngle = m_dXAngle + 360.0f;
	//	}
	//}

	//m_dYAngle += dYSpin * (180. / M_PI);
	//if (m_dYAngle > 360.0)
	//{
	//	m_dYAngle = m_dYAngle - 360.0f;
	//}
	//else
	//{
	//	if (m_dYAngle < 0.0)
	//	{
	//		m_dYAngle = m_dYAngle + 360.0f;
	//	}
	//}

	//m_pWnd->RedrawWindow();
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::Zoom(double dZTranslation)
{
	/*m_dZTranslation += dZTranslation;

	m_pWnd->RedrawWindow();*/
}

