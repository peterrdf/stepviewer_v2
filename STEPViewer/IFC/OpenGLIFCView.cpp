#include "stdafx.h"

#include "_oglUtils.h"
#include "OpenGLIFCView.h"
#include "STEPController.h"
#include "IFCModel.h"
#include "resource.h"

#include <assert.h>

// ------------------------------------------------------------------------------------------------
#define SELECTION_BUFFER_SIZE 512

const double ZOOM_SPEED_1 = 0.01;
const double ZOOM_SPEED_2 = 0.025;
const double ARROW_SIZE_I = 1.;
const double ARROW_SIZE_II = 1.;

// ------------------------------------------------------------------------------------------------
COpenGLIFCView::COpenGLIFCView(CWnd * pWnd)
	: COpenGLView()
	, m_pWnd(pWnd)
	, m_toolTipCtrl()
	, m_enProjectionType(ptPerspective)
	, m_bDetailsViewMode(FALSE)
	, m_enViewMode(vmExploreModel)
	, m_vecIFCDrawMetaData()
	, m_vecIBOs()
	, m_bShowFaces(TRUE)
	, m_bShowLines(TRUE)
	, m_bShowWireframes(TRUE)
	, m_dXAngle(-75.)
	, m_dYAngle(-30.)
	, m_dXTranslation(0.0f)
	, m_dYTranslation(0.0f)
	, m_dZTranslation(-5.0f)
	, m_dScaleFactor(1.)
	, m_ptPrevMousePosition(-1, -1)
	, m_bInteractionInProgress(false)
	, m_iSelectionFrameBuffer(0)
	, m_iSelectionTextureBuffer(0)
	, m_iSelectionDepthRenderBuffer(0)
	, m_iWireframesFrameBuffer(0)
	, m_iWireframesTextureBuffer(0)
	, m_iWireframesDepthRenderBuffer(0)
	, m_iFacesFrameBuffer(0)
	, m_iFacesTextureBuffer(0)
	, m_iFacesDepthRenderBuffer(0)
	, m_pPickedIFCObject(NULL)
	, m_pPickedIFCObjectModel(NULL)
	, m_setPickedIFCObjectEdges()
	, m_iPickedIFCObjectFace(-1)
	, m_pickedPoint3D(-DBL_MAX, -DBL_MAX, -DBL_MAX)
	, m_selectedPoint3D(-DBL_MAX, -DBL_MAX, -DBL_MAX)
	, m_viewOriginPoint3D(-DBL_MAX, -DBL_MAX, -DBL_MAX)
	, m_XArrowVector(ARROW_SIZE_II, 0., 0.)
	, m_YArrowVector(0., ARROW_SIZE_II, 0.)
	, m_ZArrowVector(0., 0., ARROW_SIZE_II)
	, m_hFont(NULL)
{
	ASSERT(m_pWnd != NULL);

	m_toolTipCtrl.Create(m_pWnd, TTS_NOPREFIX | TTS_ALWAYSTIP);	
	m_toolTipCtrl.SetDelayTime(TTDT_INITIAL, 0);
	m_toolTipCtrl.SetDelayTime(TTDT_AUTOPOP, 30000);
	m_toolTipCtrl.SetDelayTime(TTDT_RESHOW, 30000);
	m_toolTipCtrl.Activate(TRUE);
	m_toolTipCtrl.AddTool(m_pWnd, _T(""));

	/*
	Font
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

	/*
	Origin
	*/
	m_dOriginX = 0.;
	m_dOriginY = 0.;
	m_dOriginZ = 0.;

	/*m_pOGLContext->MakeCurrent();

	m_pProgram = new CBinnPhongGLProgram();
	m_pVertSh = new CGLShader(GL_VERTEX_SHADER);
	m_pFragSh = new CGLShader(GL_FRAGMENT_SHADER);

	if (!m_pVertSh->Load(IDR_TEXTFILE_VERTEX_SHADER2))
		AfxMessageBox(_T("Vertex shader loading error!"));

	if (!m_pFragSh->Load(IDR_TEXTFILE_FRAGMENT_SHADER2))
		AfxMessageBox(_T("Fragment shader loading error!"));

	if (!m_pVertSh->Compile())
		AfxMessageBox(_T("Vertex shader compiling error!"));

	if (!m_pFragSh->Compile())
		AfxMessageBox(_T("Fragment shader compiling error!"));

	m_pProgram->AttachShader(m_pVertSh);
	m_pProgram->AttachShader(m_pFragSh);

	if (!m_pProgram->Link())
		AfxMessageBox(_T("Program linking error!"));

	m_modelViewMatrix = glm::identity<glm::mat4>();*/
}

// ------------------------------------------------------------------------------------------------
COpenGLIFCView::~COpenGLIFCView()
{
	GetController()->UnRegisterView(this);	

	ResetView();

	if (m_hFont != NULL)
	{
		BOOL bResult = DeleteObject(m_hFont);
		VERIFY(bResult);

		m_hFont = NULL;
	}

	//m_pOGLContext->MakeCurrent();

	/*m_pProgram->DetachShader(m_pVertSh);
	m_pProgram->DetachShader(m_pFragSh);

	delete m_pProgram;
	m_pProgram = NULL;

	delete m_pVertSh;
	m_pVertSh = NULL;
	delete m_pFragSh;
	m_pFragSh = NULL;*/

	/*if (m_pOGLContext != NULL)
	{
		delete m_pOGLContext;
		m_pOGLContext = NULL;
	}*/
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::SetDetailsViewMode()
{
	m_bDetailsViewMode = TRUE;

	m_pWnd->RedrawWindow();
}

// ------------------------------------------------------------------------------------------------
enumViewMode COpenGLIFCView::GetViewMode() const
{
	return m_enViewMode;
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::SetViewMode(enumViewMode enViewMode)
{
	m_enViewMode = enViewMode;

	m_selectedPoint3D = CPoint3D(-DBL_MAX, -DBL_MAX, -DBL_MAX);

	m_pWnd->RedrawWindow();
}

// ------------------------------------------------------------------------------------------------
enumProjectionType COpenGLIFCView::GetProjectionType() const
{
	return m_enProjectionType;
}

// ------------------------------------------------------------------------------------------------
// Faces
void COpenGLIFCView::SetProjectionType(enumProjectionType enProjectionType)
{
	m_enProjectionType = enProjectionType;

	m_pWnd->RedrawWindow();
}

// ------------------------------------------------------------------------------------------------
// Faces
void COpenGLIFCView::ShowFaces(BOOL bShow)
{
	m_bShowFaces = bShow;

	m_pWnd->RedrawWindow();
}

// ------------------------------------------------------------------------------------------------
// Faces 
BOOL COpenGLIFCView::AreFacesShown()
{
	return m_bShowFaces;
}

// ------------------------------------------------------------------------------------------------
// Lines
void COpenGLIFCView::ShowLines(BOOL bShow)
{
	m_bShowLines = bShow;

	m_pWnd->RedrawWindow();
}

// ------------------------------------------------------------------------------------------------
// Lines 
BOOL COpenGLIFCView::AreLinesShown()
{
	return m_bShowLines;
}

// ------------------------------------------------------------------------------------------------
// Wireframes
void COpenGLIFCView::ShowWireframes(BOOL bShow)
{
	m_bShowWireframes = bShow;

	m_pWnd->RedrawWindow();
}

// ------------------------------------------------------------------------------------------------
// Wireframes 
BOOL COpenGLIFCView::AreWireframesShown()
{
	return m_bShowWireframes;
}

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
			m_ptPrevMousePosition = point;		

			m_bInteractionInProgress = true;
		}
		break;

		case meLBtnUp:
		case meMBtnUp:
		case meRBtnUp:
		{
			m_ptPrevMousePosition.x = -1;
			m_ptPrevMousePosition.y = -1;

			m_bInteractionInProgress = false;
		}
		break;

		default:
			ASSERT(FALSE);
			break;
	} // switch (enEvent)

	if (m_bInteractionInProgress)
	{
		return;
	}

	/*
	* Picked point
	*/
	double dX = 0.;
	double dY = 0.;
	double dZ = 0.;
	bool bPickedPoint = false;
	m_pickedPoint3D = CPoint3D(-DBL_MAX, -DBL_MAX, -DBL_MAX);
	if (m_pPickedIFCObject != NULL)
	{
		if (GetOGLPos(point.x, point.y, -FLT_MAX, dX, dY, dZ))
		{
			m_pickedPoint3D = CPoint3D(dX, dY, dZ);

			bPickedPoint = true;

			if ((m_enViewMode == vmMeasurePoint) || (m_enViewMode == vmMeasureDistance))
			{
				/*auto pController = GetController();
				ASSERT(pController != NULL);*/

				// #####vector<CIFCModel *> vecModels = pController->GetModels();
				/*CIFCModel * pModel = pController->GetActiveModel();*/

				if (m_enViewMode == vmMeasurePoint)
				{
					/*GetController()->FireOnMouseOver3DPointEvent(
						pModel->getXoffset() + (m_dScaleFactor * dX) / 2.,
						pModel->getYoffset() + (m_dScaleFactor * dY) / 2.,
						pModel->getZoffset() + (m_dScaleFactor * dZ) / 2.);

					CString strTooltip;
					strTooltip.Format(_T("%.2f, %.2f, %.2f"),
						pModel->getXoffset() + (m_dScaleFactor * dX) / 2.,
						pModel->getYoffset() + (m_dScaleFactor * dY) / 2.,
						pModel->getZoffset() + (m_dScaleFactor * dZ) / 2.);

					CString strTitle;
					VERIFY(strTitle.LoadStringW(IDS_POINT));

					ShowTooltip(strTitle, strTooltip);*/
				} // if (m_enViewMode == vmMeasurePoint)				
				else
				{
					//if ((m_enViewMode == vmMeasureDistance) && (m_selectedPoint3D.x() != -DBL_MAX))
					//{
					//	double dDistance = sqrt(
					//		pow(m_selectedPoint3D.x() - m_pickedPoint3D.x(), 2.) + 
					//		pow(m_selectedPoint3D.y() - m_pickedPoint3D.y(), 2.) + 
					//		pow(m_selectedPoint3D.z() - m_pickedPoint3D.z(), 2.));
					//	dDistance = (m_dScaleFactor * dDistance) / 2.;

					//	GetController()->FireOnMeasureDistanceEvent(
					//		pModel->getXoffset() + (m_dScaleFactor * m_selectedPoint3D.x()) / 2.,
					//		pModel->getYoffset() + (m_dScaleFactor * m_selectedPoint3D.y()) / 2.,
					//		pModel->getZoffset() + (m_dScaleFactor * m_selectedPoint3D.z()) / 2.,
					//		pModel->getXoffset() + (m_dScaleFactor * m_pickedPoint3D.x()) / 2.,
					//		pModel->getYoffset() + (m_dScaleFactor * m_pickedPoint3D.y()) / 2.,
					//		pModel->getZoffset() + (m_dScaleFactor * m_pickedPoint3D.z()) / 2.,
					//		dDistance);

					//	CString strTooltip;
					//	strTooltip.Format(_T("%.2f"), dDistance);

					//	CString strTitle;
					//	VERIFY(strTitle.LoadStringW(IDS_DISTANCE));

					//	const CIFCUnit * pUnit = pModel->getUnit(L"LENGTHUNIT");
					//	if ((pUnit != NULL) && !pUnit->getUnit().empty())
					//	{
					//		strTooltip += L" ";
					//		strTooltip += pUnit->getUnit().c_str();
					//	}

					//	ShowTooltip(strTitle, strTooltip);
					//} // if ((m_enViewMode == vmMeasureDistance) && ...
				} // else if (m_enViewMode == vmMeasurePoint)

				m_pWnd->RedrawWindow();
			} // if ((m_enViewMode == vmMeasurePoint) || ...			
		} // if (GetOGLPos(...
		else
		{
			HideTooltip();
		}
	} // if (m_pPickedIFCObject != NULL)
	else
	{
		HideTooltip();
	}

	if (enEvent == meLBtnUp)
	{
		// **************************************************************************************** //
		// Measurement: distance
		//if ((m_enViewMode == vmMeasureDistance) && (GetKeyState(VK_CONTROL) & 0x8000))
		//{
		//	if ((m_pPickedIFCObject != NULL) && bPickedPoint)
		//	{
		//		auto pController = GetController();
		//		ASSERT(pController != NULL);

		//		vector<CIFCModel *> vecModels = pController->GetModels();
		//		CIFCModel * pModel = pController->GetActiveModel();

		//		GetController()->FireOnMouseOver3DPointEvent(
		//			pModel->getXoffset() + (m_dScaleFactor * dX) / 2.,
		//			pModel->getYoffset() + (m_dScaleFactor * dY) / 2.,
		//			pModel->getZoffset() + (m_dScaleFactor * dZ) / 2.);

		//		m_selectedPoint3D = CPoint3D(dX, dY, dZ);

		//		m_pWnd->RedrawWindow();
		//	} // if ((m_pPickedIFCObject != NULL) && ...
		//	else
		//	{
		//		m_selectedPoint3D = CPoint3D(-DBL_MAX, -DBL_MAX, -DBL_MAX);

		//		m_pWnd->RedrawWindow();
		//	}
		//} // if ((m_enViewMode == vmMeasureDistance) && ...
		// **************************************************************************************** //

		// **************************************************************************************** //
		// OnSelectedItemChanged() notification
		if (m_pPickedIFCObject != NULL)
		{
			/*
			* Zoom To/Origin/Select
			*/
			if (GetKeyState(VK_SHIFT) & 0x8000)
			{
				/*
				* Zoom To
				*/
				double minX = 0, maxX = 0, minY = 0, maxY = 0, minZ = 0, maxZ = 0;

				bool initialized = false;
				ZoomToCoreCalculate(m_pPickedIFCObject, &minX, &maxX, &minY, &maxY, &minZ, &maxZ, &initialized);

				if (initialized) {
					ZoomToCoreSet(minX, maxX, minY, maxY, minZ, maxZ);
				}
			} // if (GetKeyState(VK_SHIFT) & 0x8000)
			else
			{
				if (GetKeyState(VK_MENU) & 0x8000)
				{
					/*
					* Change the origin
					*/					
					if (bPickedPoint)
					{
						m_dOriginX = dX;
						m_dOriginY = dY;
						m_dOriginZ = dZ;

						m_dXTranslation = -m_dOriginX;
						m_dYTranslation = -m_dOriginY;

						m_pWnd->RedrawWindow();
					}
				} // if (GetKeyState(VK_MENU) & 0x8000)
				else
				{
					/*
					* Select
					*/
					if (!m_bDetailsViewMode && (GetKeyState(VK_CONTROL) & 0x8000))
					{
						m_pPickedIFCObject->setSelected(m_pPickedIFCObject->getSelected());

						m_pWnd->RedrawWindow();

						GetController()->SelectInstance(this, m_pPickedIFCObject);
					}
				} // else if (GetKeyState(VK_MENU) & 0x8000)				
			} // else if (GetKeyState(VK_SHIFT) & 0x8000)
		} // if (m_pPickedIFCObject != NULL)
		else 
		{
			if (!m_bDetailsViewMode && (GetKeyState(VK_CONTROL) & 0x8000))
			{
				auto pController = GetController();
				ASSERT(pController != nullptr);

				auto pModel = pController->GetModel()->As<CIFCModel>();
				ASSERT(pModel != nullptr);

				const vector<CIFCObject *> & vecIFCObjects = pModel->getIFCObjects();
				for (size_t iIFCObject = 0; iIFCObject < vecIFCObjects.size(); iIFCObject++)
				{
					CIFCObject * pIFCObject = vecIFCObjects[iIFCObject];

					pIFCObject->setSelected(false);
				}

				m_pWnd->RedrawWindow();

				GetController()->SelectInstance(this, nullptr);
			}
		} // else if (m_pPickedIFCObject != NULL)
		// **************************************************************************************** //
	} // if (enEvent == meLBtnUp)
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
void COpenGLIFCView::OnMouseWheel(UINT /*nFlags*/, short zDelta, CPoint /*pt*/)
{
	Zoom(zDelta < 0 ? -abs(m_dZTranslation) * ZOOM_SPEED_1 : abs(m_dZTranslation) * ZOOM_SPEED_1);

	TRACE(L"\nm_dZTranslation: %f", m_dZTranslation);
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void COpenGLIFCView::OnInstanceLoadedEvent(CIFCModel* /*pModel*/, CIFCObject* /*pIFCObject*/)
{
	//if (!m_bDetailsViewMode)
	//{
	//	return;
	//}

	//ASSERT(pModel != NULL);
	//ASSERT(pIFCObject != NULL);

	//if (!pIFCObject->hasGeometry())
	//{
	//	m_pWnd->RedrawWindow();

	//	return;
	//}

	//BOOL bResult = m_pOGLContext->MakeCurrent();
	//ASSERT(bResult);

	//CIFCController* pController = GetController();
	//ASSERT(pController != NULL);

	//vector<CIFCModel*> vecModels = pController->GetModels();

	///*
	//* Calculate world dimensions
	//*/
	//float fXmin = FLT_MAX;
	//float fXmax = -FLT_MAX;
	//float fYmin = FLT_MAX;
	//float fYmax = -FLT_MAX;
	//float fZmin = FLT_MAX;
	//float fZmax = -FLT_MAX;
	//for (size_t iModel = 0; iModel < vecModels.size(); iModel++)
	//{
	//	CIFCModel* pModel = vecModels[iModel];

	//	fXmin = fmin(fXmin, pModel->getXMinMax().first);
	//	fXmax = fmax(fXmax, pModel->getXMinMax().second);
	//	fYmin = fmin(fYmin, pModel->getYMinMax().first);
	//	fYmax = fmax(fYmax, pModel->getYMinMax().second);
	//	fZmin = fmin(fZmin, pModel->getZMinMax().first);
	//	fZmax = fmax(fZmax, pModel->getZMinMax().second);
	//} // for (size_t iModel = ...

	///*
	//* Bounding sphere diameter
	//*/
	//float fBoundingSphereDiameter = fXmax - fXmin;
	//fBoundingSphereDiameter = fmax(fBoundingSphereDiameter, fYmax - fYmin);
	//fBoundingSphereDiameter = fmax(fBoundingSphereDiameter, fZmax - fZmin);

	//map<CIFCModel*, vector<CIFCDrawMetaData*>>::iterator itModel2DrawMetaData = m_vecIFCDrawMetaData.find(pModel);
	//ASSERT(itModel2DrawMetaData != m_vecIFCDrawMetaData.end());

	//vector<CIFCDrawMetaData*> veCIFCDrawMetaData = m_vecIFCDrawMetaData[pModel];

	//GLsizei VERTICES_MAX_COUNT = COpenGL::GetFacesVerticesCountLimit();
	//GLsizei INDICES_MAX_COUNT = COpenGL::GetIndicesCountLimit();

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

	///*
	//* IBO - Materials
	//*/
	//for (size_t iMaterial = 0; iMaterial < pIFCObject->conceptualFacesMaterials().size(); iMaterial++)
	//{
	//	if ((int_t)(iFacesIndicesCount + pIFCObject->conceptualFacesMaterials()[iMaterial]->getIndicesCount()) > (int_t)INDICES_MAX_COUNT)
	//	{
	//		assert(!vecIFCMaterialsGroup.empty());

	//		GLuint iIBO = 0;
	//		glGenBuffers(1, &iIBO);

	//		ASSERT(iIBO != 0);

	//		m_vecIBOs.push_back(iIBO);

	//		int_t iGroupIndicesCount = 0;
	//		unsigned int* pIndices = pModel->GetMaterialsIndices(vecIFCMaterialsGroup, iGroupIndicesCount);
	//		if ((iGroupIndicesCount == 0) || (pIndices == nullptr))
	//		{
	//			ASSERT(0);

	//			return;
	//		}

	//		ASSERT(iFacesIndicesCount == iGroupIndicesCount);

	//		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iIBO);
	//		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * iFacesIndicesCount, pIndices, GL_STATIC_DRAW);

	//		delete[] pIndices;

	//		/*
	//		* Store IBO/offset
	//		*/
	//		GLsizei iIBOOffset = 0;
	//		for (size_t iMaterial2 = 0; iMaterial2 < vecIFCMaterialsGroup.size(); iMaterial2++)
	//		{
	//			vecIFCMaterialsGroup[iMaterial2]->IBO() = iIBO;
	//			vecIFCMaterialsGroup[iMaterial2]->IBOOffset() = iIBOOffset;

	//			iIBOOffset += (GLsizei)vecIFCMaterialsGroup[iMaterial2]->getIndicesCount();
	//		}

	//		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//		_oglUtils::checkForErrors();

	//		iFacesIndicesCount = 0;
	//		vecIFCMaterialsGroup.clear();
	//	} // if ((int_t)(iFacesIndicesCount + ...	

	//	iFacesIndicesCount += (GLsizei)pIFCObject->conceptualFacesMaterials()[iMaterial]->getIndicesCount();
	//	vecIFCMaterialsGroup.push_back(pIFCObject->conceptualFacesMaterials()[iMaterial]);
	//} // for (size_t iMaterial = ...	

	///*
	//* IBO - Lines
	//*/
	//for (size_t iLinesCohort = 0; iLinesCohort < pIFCObject->linesCohorts().size(); iLinesCohort++)
	//{
	//	if ((int_t)(iLinesIndicesCount + pIFCObject->linesCohorts()[iLinesCohort]->getIndicesCount()) > (int_t)INDICES_MAX_COUNT)
	//	{
	//		assert(!vecLinesCohorts.empty());

	//		GLuint iIBO = 0;
	//		glGenBuffers(1, &iIBO);

	//		ASSERT(iIBO != 0);

	//		m_vecIBOs.push_back(iIBO);

	//		int_t iCohortIndicesCount = 0;
	//		unsigned int* pIndices = pModel->GetLinesCohortsIndices(vecLinesCohorts, iCohortIndicesCount);
	//		if ((iCohortIndicesCount == 0) || (pIndices == NULL))
	//		{
	//			ASSERT(0);

	//			return;
	//		}

	//		ASSERT(iLinesIndicesCount == iCohortIndicesCount);


	//		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iIBO);
	//		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * iLinesIndicesCount, pIndices, GL_STATIC_DRAW);

	//		delete[] pIndices;

	//		/*
	//		* Store IBO/offset
	//		*/
	//		GLsizei iIBOOffset = 0;
	//		for (size_t iLinesCohort2 = 0; iLinesCohort2 < vecLinesCohorts.size(); iLinesCohort2++)
	//		{
	//			vecLinesCohorts[iLinesCohort2]->IBO() = iIBO;
	//			vecLinesCohorts[iLinesCohort2]->IBOOffset() = iIBOOffset;

	//			iIBOOffset += (GLsizei)vecLinesCohorts[iLinesCohort2]->getIndicesCount();
	//		} // for (size_t iLinesCohort2 = ...				

	//		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//		_oglUtils::checkForErrors();

	//		iLinesIndicesCount = 0;
	//		vecLinesCohorts.clear();
	//	} // if ((int_t)(iLinesIndicesCount + ...	

	//	iLinesIndicesCount += (GLsizei)pIFCObject->linesCohorts()[iLinesCohort]->getIndicesCount();
	//	vecLinesCohorts.push_back(pIFCObject->linesCohorts()[iLinesCohort]);
	//} // for (size_t iLinesCohort = ...	

	///*
	//* IBO - Wireframes
	//*/
	//for (size_t iWireframesCohort = 0; iWireframesCohort < pIFCObject->wireframesCohorts().size(); iWireframesCohort++)
	//{
	//	if ((int_t)(iWireframesIndicesCount + pIFCObject->wireframesCohorts()[iWireframesCohort]->getIndicesCount()) > (int_t)INDICES_MAX_COUNT)
	//	{
	//		assert(!vecWireframesCohorts.empty());

	//		GLuint iIBO = 0;
	//		glGenBuffers(1, &iIBO);

	//		ASSERT(iIBO != 0);

	//		m_vecIBOs.push_back(iIBO);

	//		int_t iCohortIndicesCount = 0;
	//		unsigned int* pIndices = pModel->GetWireframesCohortsIndices(vecWireframesCohorts, iCohortIndicesCount);
	//		if ((iCohortIndicesCount == 0) || (pIndices == NULL))
	//		{
	//			ASSERT(0);

	//			return;
	//		}

	//		ASSERT(iWireframesIndicesCount == iCohortIndicesCount);


	//		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iIBO);
	//		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * iWireframesIndicesCount, pIndices, GL_STATIC_DRAW);

	//		delete[] pIndices;

	//		/*
	//		* Store IBO/offset
	//		*/
	//		GLsizei iIBOOffset = 0;
	//		for (size_t iWireframesCohort2 = 0; iWireframesCohort2 < vecWireframesCohorts.size(); iWireframesCohort2++)
	//		{
	//			vecWireframesCohorts[iWireframesCohort2]->IBO() = iIBO;
	//			vecWireframesCohorts[iWireframesCohort2]->IBOOffset() = iIBOOffset;

	//			iIBOOffset += (GLsizei)vecWireframesCohorts[iWireframesCohort2]->getIndicesCount();
	//		} // for (size_t iWireframesCohort2 = ...				

	//		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//		_oglUtils::checkForErrors();

	//		iWireframesIndicesCount = 0;
	//		vecWireframesCohorts.clear();
	//	} // if ((int_t)(iWireframesIndicesCount + ...	

	//	iWireframesIndicesCount += (GLsizei)pIFCObject->wireframesCohorts()[iWireframesCohort]->getIndicesCount();
	//	vecWireframesCohorts.push_back(pIFCObject->wireframesCohorts()[iWireframesCohort]);
	//} // for (size_t iWireframesCohort = ...	

	//iVerticesCount += (GLsizei)pIFCObject->verticesCount();
	//vecIFCObjectsGroup.push_back(pIFCObject);

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

	//	veCIFCDrawMetaData.push_back(pDrawMetaData);

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

	//m_vecIFCDrawMetaData[pModel] = veCIFCDrawMetaData;

	//m_pWnd->RedrawWindow();
}

// ------------------------------------------------------------------------------------------------
///*virtual*/ void COpenGLIFCView::OnActiveModelChangedEvent(const CIFCView * pSender)
//{
//	if (pSender == this)
//	{
//		return;
//	}
//
//	m_pWnd->RedrawWindow();
//}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void COpenGLIFCView::OnModelDeleted(CIFCModel * /*pModel*/)
{
	//CWaitCursor waitCursor;

	//BOOL bResult = m_pOGLContext->MakeCurrent();
	//ASSERT(bResult);

	//CIFCController * pController = GetController();
	//ASSERT(pController != NULL);

	///*
	//* Remove the draw meta data
	//*/
	//map<CIFCModel *, vector<CIFCDrawMetaData *>>::iterator itModel2DrawMetaData = m_vecIFCDrawMetaData.find(pModel);
	//ASSERT(itModel2DrawMetaData != m_vecIFCDrawMetaData.end());

	//for (size_t iDrawMetaData = 0; iDrawMetaData < itModel2DrawMetaData->second.size(); iDrawMetaData++)
	//{
	//	delete itModel2DrawMetaData->second[iDrawMetaData];
	//}

	//m_vecIFCDrawMetaData.erase(itModel2DrawMetaData);

	//vector<CIFCModel *> vecModels = pController->GetModels();

	///*
	//* Calculate world dimensions
	//*/
	//float fXmin = FLT_MAX;
	//float fXmax = -FLT_MAX;
	//float fYmin = FLT_MAX;
	//float fYmax = -FLT_MAX;
	//float fZmin = FLT_MAX;
	//float fZmax = -FLT_MAX;
	//for (size_t iModel = 0; iModel < vecModels.size(); iModel++)
	//{
	//	CIFCModel * pModel = vecModels[iModel];

	//	fXmin = fmin(fXmin, pModel->getXMinMax().first);
	//	fXmax = fmax(fXmax, pModel->getXMinMax().second);
	//	fYmin = fmin(fYmin, pModel->getYMinMax().first);
	//	fYmax = fmax(fYmax, pModel->getYMinMax().second);
	//	fZmin = fmin(fZmin, pModel->getZMinMax().first);
	//	fZmax = fmax(fZmax, pModel->getZMinMax().second);
	//} // for (size_t iModel = ...

	///*
	//* Bounding sphere diameter
	//*/
	//float fBoundingSphereDiameter = fXmax - fXmin;
	//fBoundingSphereDiameter = fmax(fBoundingSphereDiameter, fYmax - fYmin);
	//fBoundingSphereDiameter = fmax(fBoundingSphereDiameter, fZmax - fZmin);

	//m_dScaleFactor = fBoundingSphereDiameter;

	//for (size_t iModel = 0; iModel < vecModels.size(); iModel++)
	//{
	//	pModel = vecModels[iModel];

	//	itModel2DrawMetaData = m_vecIFCDrawMetaData.find(pModel);
	//	ASSERT(itModel2DrawMetaData != m_vecIFCDrawMetaData.end());
	//	
	//	/*
	//	* Update VBOs
	//	*/
	//	for (size_t iDrawMetaData = 0; iDrawMetaData < itModel2DrawMetaData->second.size(); iDrawMetaData++)
	//	{
	//		const map<GLuint, vector<CIFCObject *>> & mapGroups = itModel2DrawMetaData->second[iDrawMetaData]->getGroups();

	//		map<GLuint, vector<CIFCObject *>>::const_iterator itGroups = mapGroups.begin();
	//		for (; itGroups != mapGroups.end(); itGroups++)
	//		{
	//			assert(!itGroups->second.empty());

	//			int_t iCohortVerticesCount = 0;
	//			float * pVertices = pModel->GetVertices(itGroups->second, iCohortVerticesCount);
	//			if ((iCohortVerticesCount == 0) || (pVertices == nullptr))
	//			{
	//				ASSERT(0);

	//				return;
	//			}

	//			CIFCObject::Scale(pVertices, iCohortVerticesCount, fXmin, fXmax, fYmin, fYmax, fZmin, fZmax, fBoundingSphereDiameter);

	//			glBindBuffer(GL_ARRAY_BUFFER, itGroups->first);
	//			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * iCohortVerticesCount * GEOMETRY_VBO_VERTEX_LENGTH, pVertices, GL_STATIC_DRAW);

	//			/*
	//			* Update
	//			*/
	//			GLsizei iVBOOffset = 0;
	//			for (size_t iIFCObject = 0; iIFCObject < itGroups->second.size(); iIFCObject++)
	//			{
	//				CIFCObject * pIFCObject = itGroups->second[iIFCObject];

	//				// Update Min/max
	//				pIFCObject->CalculateMinMaxValues(pVertices + (iVBOOffset * GEOMETRY_VBO_VERTEX_LENGTH), pIFCObject->verticesCount());

	//				// Update minimum bounding box
	//				pIFCObject->UpdateBBox(fXmin, fXmax, fYmin, fYmax, fZmin, fZmax, fBoundingSphereDiameter);

	//				iVBOOffset += (GLsizei)pIFCObject->verticesCount();
	//			} // for (size_t iIFCObject = ...

	//			delete[] pVertices;

	//			glBindBuffer(GL_ARRAY_BUFFER, 0);

	//			_oglUtils::checkForErrors();
	//		} // for (; itGroups != ...
	//	} // for (size_t iDrawMetaData = ...
	//} // for (size_t iModel = ...

	//m_pWnd->RedrawWindow();
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void COpenGLIFCView::OnAllModelsDeleted()
{
	ResetView();

	m_pWnd->RedrawWindow();
}

// ------------------------------------------------------------------------------------------------
///*virtual*/ void COpenGLIFCView::OnItemsVisibleStateChangedEvent(const CIFCView * /*pSender*/, const CIFCModel * /*pModel*/, const vector<CIFCObject *> & /*vecItems*/)
//{
//	m_pWnd->RedrawWindow();
//}

// ------------------------------------------------------------------------------------------------
///*virtual*/ void COpenGLIFCView::OnItemsVisibleStateChangedEvent(const CIFCView * /*pSender*/, const CIFCModel * /*pModel*/, const CString & /*strEntity*/)
//{
//	m_pWnd->RedrawWindow();
//}

// ------------------------------------------------------------------------------------------------
///*virtual*/ void COpenGLIFCView::OnSelectInstanceEvent(const CIFCView * pSender, int_t /*iExpressID*/)
//{
//	if (this == pSender)
//	{
//		return;
//	}
//	
//	m_viewOriginPoint3D = CPoint3D(-DBL_MAX, -DBL_MAX, -DBL_MAX);
//
//	m_pWnd->RedrawWindow();
//}

// ------------------------------------------------------------------------------------------------
///*virtual*/ void COpenGLIFCView::OnPointInstanceEvent(const CIFCView* pSender, int_t iExpressID)
//{
//	if (pSender == this)
//	{
//		return;
//	}	
//
//	if (m_bDetailsViewMode)
//	{
//		return;
//	}
//
//	if (iExpressID <= 0)
//	{
//		m_pPickedIFCObjectModel = NULL;
//		m_pPickedIFCObject = NULL;
//
//		return;
//	}
//
//	auto pController = GetController();
//	ASSERT(pController != NULL);
//	
//	CIFCModel* pModel = pController->GetActiveModel();
//	ASSERT(pModel != NULL);
//
//	CIFCObject* pIFCObject = pModel->getIFCObjectByExpressID(iExpressID);
//	if (pIFCObject == NULL)
//	{
//		ASSERT(FALSE);
//
//		return;
//	}
//
//	m_pPickedIFCObjectModel = pModel;
//	m_pPickedIFCObject = pIFCObject;
//
//	m_pWnd->RedrawWindow();
//}

// ------------------------------------------------------------------------------------------------
///*virtual*/ void COpenGLIFCView::OnViewOriginEvent(const CIFCView* pSender, int_t iInstance)
//{
//	if (this == pSender)
//	{
//		return;
//	}
//
//	if (!m_bDetailsViewMode)
//	{
//		return;
//	}
//
//	double transformationMatrix[12];
/////	double startVector[3];
/////	double endVector[3];
/////	GetBoundingBox(iInstance, transformationMatrix, startVector, endVector);
//
//	//
//	//	New call
//	//
//	GetRelativeTransformation(0, iInstance, transformationMatrix);
//
//	auto pController = GetController();
//	ASSERT(pController != NULL);
//
//	vector<CIFCModel*> vecModels = pController->GetModels();
//
//	/*
//	* Calculate world dimensions
//	*/
//	float fXmin = FLT_MAX;
//	float fXmax = -FLT_MAX;
//	float fYmin = FLT_MAX;
//	float fYmax = -FLT_MAX;
//	float fZmin = FLT_MAX;
//	float fZmax = -FLT_MAX;
//	for (size_t iModel = 0; iModel < vecModels.size(); iModel++)
//	{
//		CIFCModel* pModel = vecModels[iModel];
//
//		fXmin = fmin(fXmin, pModel->getXMinMax().first);
//		fXmax = fmax(fXmax, pModel->getXMinMax().second);
//		fYmin = fmin(fYmin, pModel->getYMinMax().first);
//		fYmax = fmax(fYmax, pModel->getYMinMax().second);
//		fZmin = fmin(fZmin, pModel->getZMinMax().first);
//		fZmax = fmax(fZmax, pModel->getZMinMax().second);
//	} // for (size_t iModel = ...
//
//	/*
//	* Bounding sphere diameter
//	*/
//	float fResoltuion = fXmax - fXmin;
//	fResoltuion = fmax(fResoltuion, fYmax - fYmin);
//	fResoltuion = fmax(fResoltuion, fZmax - fZmin);
//
//	double dX = transformationMatrix[9];
//	double dY = transformationMatrix[11];
//	double dZ = -transformationMatrix[10];
//
//	double	x, y, z;
//	GetVertexBufferOffset(vecModels[0]->getModel(), &x, &y, &z);
//
//	dX += x;
//	dY += z;
//	dZ += -y;
//
//	// [0.0 -> X/Y/Zmin + X/Y/Zmax]
//	dX = dX - fXmin;
//	dY = dY - fYmin;
//	dZ = dZ - fZmin;
//
//	// center
//	dX = dX - ((fXmax - fXmin) / 2.0f);
//	dY = dY - ((fYmax - fYmin) / 2.0f);
//	dZ = dZ - ((fZmax - fZmin) / 2.0f);
//
//	// [-1.0 -> 1.0]
//	dX = dX / (fResoltuion / 2.0f);
//	dY = dY / (fResoltuion / 2.0f);
//	dZ = dZ / (fResoltuion / 2.0f);
//
//	m_viewOriginPoint3D = CPoint3D(dX, dY, dZ);
//	m_XArrowVector = CPoint3D(transformationMatrix[0], transformationMatrix[1], -transformationMatrix[2]);
//	m_YArrowVector = CPoint3D(transformationMatrix[3], transformationMatrix[4], -transformationMatrix[5]);
//	m_ZArrowVector = CPoint3D(transformationMatrix[6], transformationMatrix[7], -transformationMatrix[8]);
//
//	m_pWnd->RedrawWindow();
//}

// ------------------------------------------------------------------------------------------------
///*virtual*/ void COpenGLIFCView::OnViewGeometryEvent(const CIFCView* pSender, CIFCObject* /*pIFCObject*/)
//{
//	if (this == pSender)
//	{
//		return;
//	}
//
//	m_viewOriginPoint3D = CPoint3D(-DBL_MAX, -DBL_MAX, -DBL_MAX);
//
//	m_pWnd->RedrawWindow();
//}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void COpenGLIFCView::OnControllerChanged()
{
	if (GetController() != NULL)
	{
		GetController()->RegisterView(this);
	}	
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::SetOrientation(LONG iOrientation)
{
	switch (iOrientation)
	{
	case 1: // front
	{
		m_dXAngle = 0.;
		m_dYAngle = 0.;
	}
	break;

	case 2: // right
	{
		m_dXAngle = 0.;
		m_dYAngle = -90.;
	}
	break;

	case 3: // top
	{
		m_dXAngle = 90.;
		m_dYAngle = 0.;
	}
	break;

	case 4: // back
	{
		m_dXAngle = 0.;
		m_dYAngle = -180.;
	}
	break;

	case 5: // left
	{
		m_dXAngle = 0.;
		m_dYAngle = 90.;
	}
	break;

	case 6: // bottom
	{
		m_dXAngle = -90.;
		m_dYAngle = 0.;
	}
	break;

	default:
	{
		ASSERT(FALSE);
	}
	break;
	} // switch (iOrientation)

	m_dOriginX = m_dOriginY = m_dOriginZ = 0.;

	m_pWnd->RedrawWindow();
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::ZoomToSet(double minX, double maxX, double minY, double maxY, double minZ, double maxZ)
{
	ZoomToCoreSet(minX, maxX, minY, maxY, minZ, maxZ);
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::SetQuatRotation(DOUBLE /*dW*/, DOUBLE /*dXRadians*/, DOUBLE /*dYRadians*/, DOUBLE /*dZRadians*/)
{
	ASSERT(FALSE); // OBSOLETE
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::GetQuatRotation(DOUBLE* /*pdW*/, DOUBLE* /*pdXRadians*/, DOUBLE* /*pdYRadians*/, DOUBLE* /*pdZRadians*/)
{
	ASSERT(FALSE); // OBSOLETE
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::SetRotation(DOUBLE dXDegrees, DOUBLE dYDegrees, DOUBLE /*dZDegrees*/)
{
	m_dXAngle = dXDegrees;
	m_dYAngle = dYDegrees;
	//dZDegrees - UNUSED

	m_pWnd->RedrawWindow();
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::GetRotation(DOUBLE* pdXDegrees, DOUBLE* pdYDegrees, DOUBLE* pdZDegrees)
{
	*pdXDegrees = m_dXAngle;
	*pdYDegrees = m_dYAngle;
	*pdZDegrees = 0.; // UNUSED
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::GetTranslation(DOUBLE* pdX, DOUBLE* pdY, DOUBLE* pdZ)
{
	*pdX = m_dXTranslation;
	*pdY = m_dYTranslation;
	*pdZ = m_dZTranslation;
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::SetTranslation(DOUBLE dX, DOUBLE dY, DOUBLE dZ)
{
	m_dXTranslation = dX;
	m_dYTranslation = dY;
	m_dZTranslation = dZ;

	m_pWnd->RedrawWindow();
}

// ------------------------------------------------------------------------------------------------
CIFCObject* COpenGLIFCView::GetPickedIFCObject() const
{
	return m_pPickedIFCObject;
}

// ------------------------------------------------------------------------------------------------
CPoint3D& COpenGLIFCView::GetPickedPoint()
{
	return m_pickedPoint3D;
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::ZoomToExtent(CIFCObject* pIFCObject)
{
	ASSERT(pIFCObject != NULL);

	double minX = 0, maxX = 0, minY = 0, maxY = 0, minZ = 0, maxZ = 0;

	bool initialized = false;
	ZoomToCoreCalculate(pIFCObject, &minX, &maxX, &minY, &maxY, &minZ, &maxZ, &initialized);

	if (initialized) {
		ZoomToCoreSet(minX, maxX, minY, maxY, minZ, maxZ);
	}
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::ZoomToPickedPoint()
{
	ASSERT(m_pickedPoint3D.x() != -DBL_MAX);

	m_dOriginX = m_pickedPoint3D.x();
	m_dOriginY = m_pickedPoint3D.y();
	m_dOriginZ = m_pickedPoint3D.z();

	m_dXTranslation = -m_dOriginX;
	m_dYTranslation = -m_dOriginY;

	m_pWnd->RedrawWindow();
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::ResetView()
{
	//m_pOGLContext->MakeCurrent();

	//for (size_t iDrawMetaData = 0; iDrawMetaData < m_vecIFCDrawMetaData.size(); iDrawMetaData++)
	//{
	//	delete m_vecIFCDrawMetaData[iDrawMetaData];
	//}

	//m_vecIFCDrawMetaData.clear();

	//if (m_iSelectionFrameBuffer != 0)
	//{
	//	glDeleteFramebuffers(1, &m_iSelectionFrameBuffer);
	//	m_iSelectionFrameBuffer = 0;
	//}

	//if (m_iSelectionTextureBuffer != 0)
	//{
	//	glDeleteTextures(1, &m_iSelectionTextureBuffer);
	//	m_iSelectionTextureBuffer = 0;
	//}

	//if (m_iSelectionDepthRenderBuffer != 0)
	//{
	//	glDeleteRenderbuffers(1, &m_iSelectionDepthRenderBuffer);
	//	m_iSelectionDepthRenderBuffer = 0;
	//}

	//if (m_iWireframesFrameBuffer != 0)
	//{
	//	glDeleteFramebuffers(1, &m_iWireframesFrameBuffer);
	//	m_iWireframesFrameBuffer = 0;
	//}

	//if (m_iWireframesTextureBuffer != 0)
	//{
	//	glDeleteTextures(1, &m_iWireframesTextureBuffer);
	//	m_iWireframesTextureBuffer = 0;
	//}

	//if (m_iWireframesDepthRenderBuffer != 0)
	//{
	//	glDeleteRenderbuffers(1, &m_iWireframesDepthRenderBuffer);
	//	m_iWireframesDepthRenderBuffer = 0;
	//}

	//if (m_iFacesFrameBuffer != 0)
	//{
	//	glDeleteFramebuffers(1, &m_iFacesFrameBuffer);
	//	m_iFacesFrameBuffer = 0;
	//}

	//if (m_iFacesTextureBuffer != 0)
	//{
	//	glDeleteTextures(1, &m_iFacesTextureBuffer);
	//	m_iFacesTextureBuffer = 0;
	//}

	//if (m_iFacesDepthRenderBuffer != 0)
	//{
	//	glDeleteRenderbuffers(1, &m_iFacesDepthRenderBuffer);
	//	m_iFacesDepthRenderBuffer = 0;
	//}

	//for (size_t iIBO = 0; iIBO < m_vecIBOs.size(); iIBO++)
	//{
	//	glDeleteBuffers(1, &(m_vecIBOs[iIBO]));
	//}
	//m_vecIBOs.clear();

	//m_dXAngle = -75.;
	//m_dYAngle = -30.;
	//m_dXTranslation = 0.0f;
	//m_dYTranslation = 0.0f;
	//m_dZTranslation = -5.0f;
	//m_dScaleFactor = 1.;
	//m_ptPrevMousePosition = CPoint(-1, -1);
	//m_bInteractionInProgress = false;
	//m_pPickedIFCObject = NULL;
	//m_pPickedIFCObjectModel = NULL;
	//m_setPickedIFCObjectEdges.clear();
	//m_iPickedIFCObjectFace = -1;
	//m_pickedPoint3D = CPoint3D(-DBL_MAX, -DBL_MAX, -DBL_MAX);
	//m_selectedPoint3D = CPoint3D(-DBL_MAX, -DBL_MAX, -DBL_MAX);
	//m_viewOriginPoint3D = CPoint3D(-DBL_MAX, -DBL_MAX, -DBL_MAX);

	///*
	//Origin
	//*/
	//m_dOriginX = 0.;
	//m_dOriginY = 0.;
	//m_dOriginZ = 0.;
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::DrawScene(float /*arrowSizeI*/, float /*arrowSizeII*/)
{
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

	//if (!m_bDetailsViewMode)
	//{
	//	// X axis
	//	glProgramUniform3f(
	//		m_pProgram->GetID(),
	//		m_pProgram->getMaterialAmbientColor(),
	//		1.f,
	//		0.f,
	//		0.f);

	//	glBegin(GL_LINES);
	//	glVertex3d(m_dOriginX, m_dOriginY, m_dOriginZ);
	//	glVertex3d(m_dOriginX + arrowSizeI, m_dOriginY, m_dOriginZ);
	//	glEnd();

	//	// Y axis
	//	glProgramUniform3f(
	//		m_pProgram->GetID(),
	//		m_pProgram->getMaterialAmbientColor(),
	//		0.f,
	//		1.f,
	//		0.f);

	//	glBegin(GL_LINES);
	//	glVertex3d(m_dOriginX, m_dOriginY, m_dOriginZ);
	//	glVertex3d(m_dOriginX, m_dOriginY + arrowSizeI, m_dOriginZ);
	//	glEnd();

	//	// Z axis
	//	glProgramUniform3f(
	//		m_pProgram->GetID(),
	//		m_pProgram->getMaterialAmbientColor(),
	//		0.f,
	//		0.f,
	//		1.f);

	//	glBegin(GL_LINES);
	//	glVertex3d(m_dOriginX, m_dOriginY, m_dOriginZ);
	//	glVertex3d(m_dOriginX, m_dOriginY, m_dOriginZ + arrowSizeI);
	//	glEnd();

	//	glLineWidth(2.0);

	//	// X axis
	//	glProgramUniform3f(
	//		m_pProgram->GetID(),
	//		m_pProgram->getMaterialAmbientColor(),
	//		1.f,
	//		0.f,
	//		0.f);

	//	glBegin(GL_LINES);
	//	glVertex3d(0., 0., 0.);
	//	glVertex3d(0. + arrowSizeII, 0., 0.);
	//	glEnd();
	//	DrawTextGDI(L"X", arrowSizeII + (arrowSizeII * 0.05f), 0.f, 0.f);

	//	// Y axis
	//	glProgramUniform3f(
	//		m_pProgram->GetID(),
	//		m_pProgram->getMaterialAmbientColor(),
	//		0.f,
	//		1.f,
	//		0.f);

	//	glBegin(GL_LINES);
	//	glVertex3d(0., 0., 0.);
	//	glVertex3d(0., 0. + arrowSizeII, 0.);
	//	glEnd();
	//	DrawTextGDI(L"Y", 0.f, arrowSizeII + (arrowSizeII * 0.05f), 0.f);

	//	// Z axis
	//	glProgramUniform3f(
	//		m_pProgram->GetID(),
	//		m_pProgram->getMaterialAmbientColor(),
	//		0.f,
	//		0.f,
	//		1.f);

	//	glBegin(GL_LINES);
	//	glVertex3d(0., 0., 0.);
	//	glVertex3d(0., 0., 0. + arrowSizeII);
	//	glEnd();
	//	DrawTextGDI(L"Z", 0.f, 0.f, arrowSizeII + (arrowSizeII * 0.05f));
	//} // if (!m_bDetailsViewMode)

	_oglUtils::checkForErrors();
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
void COpenGLIFCView::DrawPickedFace()
{
	if (!m_bShowFaces)
	{
		return;
	}

	// #####

	//if ((m_pPickedIFCObject == NULL) || (m_iPickedIFCObjectFace == -1))
	//{
	//	return;
	//}

	//glDisable(GL_LIGHTING);

	//glEnableClientState(GL_VERTEX_ARRAY);

	//map<CIFCModel *, vector<CIFCDrawMetaData *>>::iterator itModel2DrawMetaData = m_vecIFCDrawMetaData.begin();
	//for (; itModel2DrawMetaData != m_vecIFCDrawMetaData.end(); itModel2DrawMetaData++)
	//{
	//	for (size_t iDrawMetaData = 0; iDrawMetaData < itModel2DrawMetaData->second.size(); iDrawMetaData++)
	//	{
	//		const map<GLuint, vector<CIFCObject *>> & mapGroups = itModel2DrawMetaData->second[iDrawMetaData]->getGroups();

	//		map<GLuint, vector<CIFCObject *>>::const_iterator itGroups = mapGroups.begin();
	//		for (; itGroups != mapGroups.end(); itGroups++)
	//		{
	//			glBindBuffer(GL_ARRAY_BUFFER, itGroups->first);
	//			glVertexPointer(3, GL_FLOAT, sizeof(GLfloat) * GEOMETRY_VBO_VERTEX_LENGTH, NULL);

	//			GLsizei iOffset = 0;
	//			for (size_t iObject = 0; iObject < itGroups->second.size(); iObject++)
	//			{
	//				CIFCObject * pIFCObject = itGroups->second[iObject];
	//				if (pIFCObject != m_pPickedIFCObject)
	//				{
	//					iOffset += (GLsizei)pIFCObject->verticesCount();

	//					continue;
	//				}

	//				/*
	//				* Conceptual faces
	//				*/
	//				for (size_t iMaterial = 0; iMaterial < pIFCObject->conceptualFacesMaterials().size(); iMaterial++)
	//				{
	//					CIFCGeometryWithMaterial * pGeometryWithMaterial = pIFCObject->conceptualFacesMaterials()[iMaterial];

	//					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pGeometryWithMaterial->IBO());

	//					for (size_t iConceptualFace = 0; iConceptualFace < pGeometryWithMaterial->conceptualFaces().size(); iConceptualFace++)
	//					{
	//						int64_t iFaceID = pGeometryWithMaterial->conceptualFaces()[iConceptualFace].index() + 1;							
	//						if (iFaceID != m_iPickedIFCObjectFace)
	//						{
	//							continue;
	//						}

	//						int64_t iStartIndex = pGeometryWithMaterial->conceptualFaces()[iConceptualFace].trianglesStartIndex();
	//						int64_t iIndicesCount = pGeometryWithMaterial->conceptualFaces()[iConceptualFace].trianglesIndicesCount();

	//						/*
	//						* Ambient color
	//						*/
	//						glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT);
	//						glColor4f(
	//							0.f,
	//							0.f,
	//							1.f,
	//							1.0f);

	//						glDrawElementsBaseVertex(GL_TRIANGLES,
	//							(GLsizei)iIndicesCount,
	//							GL_UNSIGNED_INT,
	//							(void *)(sizeof(GLuint) * (pGeometryWithMaterial->IBOOffset() + iStartIndex)),
	//							iOffset);
	//					} // for (size_t iConceptualFace = ...

	//					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//				} // for (size_t iMaterial = ...

	//				iOffset += (GLsizei)pIFCObject->verticesCount();
	//			} // for (size_t iObject = ...

	//			glBindBuffer(GL_ARRAY_BUFFER, 0);
	//		} // for (; itGroups != ...
	//	} // for (size_t iDrawMetaData = ...
	//} // for (; itModel2DrawMetaData != ...

	//glDisableClientState(GL_VERTEX_ARRAY);

	//glEnable(GL_LIGHTING);

	//_oglUtils::checkForErrors();
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::DrawPickedPoint()
{
	if ((m_enViewMode != vmMeasurePoint) || (m_pickedPoint3D.x() == -DBL_MAX))
	{
		return;
	}

	glDisable(GL_DEPTH_TEST);

	glDisable(GL_LIGHTING);
	glColor4f(0.0f, 0.0f, 1.0f, 1.0);

	glPointSize(10.f);

	glBegin(GL_POINTS);
	glVertex3d(
		m_pickedPoint3D.x(),
		m_pickedPoint3D.y(),
		m_pickedPoint3D.z());
	glEnd();

	glEnable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);

	_oglUtils::checkForErrors();
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::DrawInstanceOrigin()
{
	if (m_viewOriginPoint3D.x() == -DBL_MAX)
	{
		return;
	}

	glDisable(GL_LIGHTING);

	glLineWidth(1.0);
	glColor3f(0.0f, 0.0f, 0.0f);

	glLineWidth(2.0);

	glEnable(GL_LINE_STIPPLE);
	glLineStipple(1, 0xAAAA);

	// X axis
	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex3d(m_viewOriginPoint3D.x(), m_viewOriginPoint3D.y(), m_viewOriginPoint3D.z());
	glVertex3d(m_viewOriginPoint3D.x() + m_XArrowVector.x(), m_viewOriginPoint3D.y() + m_XArrowVector.y(), m_viewOriginPoint3D.z() + m_XArrowVector.z());
	glEnd();
	DrawTextGDI(L"X", 
		(float) m_viewOriginPoint3D.x() + 1.05f * (float) m_XArrowVector.x(),
		(float) m_viewOriginPoint3D.y() + 1.05f * (float) m_XArrowVector.y(),
		(float) m_viewOriginPoint3D.z() + 1.05f * (float) m_XArrowVector.z());

	// Y axis
	glColor3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex3d(m_viewOriginPoint3D.x(), m_viewOriginPoint3D.y(), m_viewOriginPoint3D.z());
	glVertex3d(m_viewOriginPoint3D.x() + m_YArrowVector.x(), m_viewOriginPoint3D.y() + m_YArrowVector.y(), m_viewOriginPoint3D.z() + m_YArrowVector.z());
	glEnd();
	DrawTextGDI(L"Y", 
		(float) m_viewOriginPoint3D.x() + 1.05f * (float) m_YArrowVector.x(),
		(float) m_viewOriginPoint3D.y() + 1.05f * (float) m_YArrowVector.y(),
		(float) m_viewOriginPoint3D.z() + 1.05f * (float) m_YArrowVector.z());

	// Z axis
	glColor3f(0.0f, 0.0f, 1.0f);
	glBegin(GL_LINES);
	glVertex3d(m_viewOriginPoint3D.x(), m_viewOriginPoint3D.y(), m_viewOriginPoint3D.z());
	glVertex3d(m_viewOriginPoint3D.x() + m_ZArrowVector.x(), m_viewOriginPoint3D.y() + m_ZArrowVector.y(), m_viewOriginPoint3D.z() + m_ZArrowVector.z());
	glEnd();
	DrawTextGDI(L"Z", 
		(float) m_viewOriginPoint3D.x() + 1.05f * (float) m_ZArrowVector.x(),
		(float) m_viewOriginPoint3D.y() + 1.05f * (float) m_ZArrowVector.y(),
		(float) m_viewOriginPoint3D.z() + 1.05f * (float) m_ZArrowVector.z());

	glDisable(GL_LINE_STIPPLE);
	glEnable(GL_LIGHTING);

	_oglUtils::checkForErrors();
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::DrawDistance()
{
	if ((m_enViewMode != vmMeasureDistance) || (m_selectedPoint3D.x() == -DBL_MAX))
	{
		return;
	}

	glDisable(GL_DEPTH_TEST);

	glDisable(GL_LIGHTING);

	glColor4f(0.0f, 0.0f, 1.0f, 1.0);

	glPointSize(10.f);
	
	glBegin(GL_POINTS);
	/*
	* Start point
	*/
	glVertex3d(
		m_selectedPoint3D.x(),
		m_selectedPoint3D.y(),
		m_selectedPoint3D.z());

	/*
	* End point
	*/
	if (m_pickedPoint3D.x() != -DBL_MAX)
	{
		glVertex3d(
			m_pickedPoint3D.x(),
			m_pickedPoint3D.y(),
			m_pickedPoint3D.z());
	}
	glEnd();

	/*
	* Line
	*/
	glLineWidth(2.0);

	glEnable(GL_LINE_STIPPLE);
	glLineStipple(1, 0xAAAA);

	if (m_pickedPoint3D.x() != -DBL_MAX)
	{
		glBegin(GL_LINES);
		glVertex3d(
			m_selectedPoint3D.x(),
			m_selectedPoint3D.y(),
			m_selectedPoint3D.z());		

		glVertex3d(
			m_pickedPoint3D.x(),
			m_pickedPoint3D.y(),
			m_pickedPoint3D.z());
		glEnd();
	}		

	glDisable(GL_LINE_STIPPLE);

	glEnable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);

	_oglUtils::checkForErrors();
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::DrawBoundingBoxes()
{
	if (!m_bShowFaces || m_bDetailsViewMode)
	{
		return;
	}

	// #####
	//auto pController = GetController();
	//ASSERT(pController != NULL);

	//vector<CIFCModel *> vecModels = pController->GetModels();
	//if (vecModels.empty())
	//{
	//	return;
	//}

	//glDisable(GL_LIGHTING);
	//glLineWidth(2.0);
	//glEnable(GL_LINE_STIPPLE);
	//glLineStipple(1, 0xAAAA);

	//for (size_t iModel = 0; iModel < vecModels.size(); iModel++)
	//{
	//	CIFCModel * pModel = vecModels[iModel];

	//	glColor3f(
	//		pModel->getBoundingBoxMaterial()->getAmbientColor().R(),
	//		pModel->getBoundingBoxMaterial()->getAmbientColor().G(),
	//		pModel->getBoundingBoxMaterial()->getAmbientColor().B());

	//	const vector<CIFCObject *> & vecIFCObjects = pModel->getIFCObjects();
	//	for (size_t iIFCObject = 0; iIFCObject < vecIFCObjects.size(); iIFCObject++)
	//	{
	//		CIFCObject * pIFCObject = vecIFCObjects[iIFCObject];

	//		if (!pIFCObject->selected())
	//		{
	//			continue;
	//		}

	//		// The world is always [-1, 1]
	//		const double PADDING = 0.002 * 2.;

	//		float	* origin__ = pIFCObject->getBBoxOrigin();
	//		float	* xDim__ = pIFCObject->getBBoxX();
	//		float	* yDim__ = pIFCObject->getBBoxY();
	//		float	* zDim__ = pIFCObject->getBBoxZ();

	//		float	origin[3], xDim[3], yDim[3], zDim[3];
	//		origin[0] = origin__[0];
	//		origin[1] = origin__[1];
	//		origin[2] = origin__[2];
	//		xDim[0] = xDim__[0];
	//		xDim[1] = xDim__[1];
	//		xDim[2] = xDim__[2];
	//		yDim[0] = yDim__[0];
	//		yDim[1] = yDim__[1];
	//		yDim[2] = yDim__[2];
	//		zDim[0] = zDim__[0];
	//		zDim[1] = zDim__[1];
	//		zDim[2] = zDim__[2];

	//		float	diffx[3], diffy[3], diffz[3];
	//		diffx[0] = (float)PADDING * (xDim[0]);// - origin[0]);
	//		diffx[1] = (float)PADDING * (xDim[1]);// - origin[1]);
	//		diffx[2] = (float)PADDING * (xDim[2]);// - origin[2]);
	//		diffy[0] = (float)PADDING * (yDim[0]);// - origin[0]);
	//		diffy[1] = (float)PADDING * (yDim[1]);// - origin[1]);
	//		diffy[2] = (float)PADDING * (yDim[2]);// - origin[2]);
	//		diffz[0] = (float)PADDING * (zDim[0]);// - origin[0]);
	//		diffz[1] = (float)PADDING * (zDim[1]);// - origin[1]);
	//		diffz[2] = (float)PADDING * (zDim[2]);// - origin[2]);

	//		origin[0] -= diffx[0] + diffy[0] + diffz[0];
	//		origin[1] -= diffx[1] + diffy[1] + diffz[1];
	//		origin[2] -= diffx[2] + diffy[2] + diffz[2];

	//		xDim[0] += 2 * diffx[0];
	//		xDim[1] += 2 * diffx[1];
	//		xDim[2] += 2 * diffx[2];
	//		yDim[0] += 2 * diffy[0];
	//		yDim[1] += 2 * diffy[1];
	//		yDim[2] += 2 * diffy[2];
	//		zDim[0] += 2 * diffz[0];
	//		zDim[1] += 2 * diffz[1];
	//		zDim[2] += 2 * diffz[2];

	//		glBegin(GL_LINE_LOOP);
	//		glVertex3d(origin[0], origin[1], origin[2]);
	//		glVertex3d(origin[0] + zDim[0], origin[1] + zDim[1], origin[2] + zDim[2]);
	//		glVertex3d(origin[0] + xDim[0] + zDim[0], origin[1] + xDim[1] + zDim[1], origin[2] + xDim[2] + zDim[2]);
	//		glVertex3d(origin[0] + xDim[0], origin[1] + xDim[1], origin[2] + xDim[2]);
	//		glEnd();

	//		glBegin(GL_LINE_LOOP);
	//		glVertex3d(origin[0] + yDim[0], origin[1] + yDim[1], origin[2] + yDim[2]);
	//		glVertex3d(origin[0] + yDim[0] + zDim[0], origin[1] + yDim[1] + zDim[1], origin[2] + yDim[2] + zDim[2]);
	//		glVertex3d(origin[0] + xDim[0] + yDim[0] + zDim[0], origin[1] + xDim[1] + yDim[1] + zDim[1], origin[2] + xDim[2] + yDim[2] + zDim[2]);
	//		glVertex3d(origin[0] + xDim[0] + yDim[0], origin[1] + xDim[1] + yDim[1], origin[2] + xDim[2] + yDim[2]);
	//		glEnd();

	//		glBegin(GL_LINE_LOOP);
	//		glVertex3d(origin[0] + zDim[0], origin[1] + zDim[1], origin[2] + zDim[2]);
	//		glVertex3d(origin[0] + yDim[0] + zDim[0], origin[1] + yDim[1] + zDim[1], origin[2] + yDim[2] + zDim[2]);
	//		glVertex3d(origin[0] + xDim[0] + yDim[0] + zDim[0], origin[1] + xDim[1] + yDim[1] + zDim[1], origin[2] + xDim[2] + yDim[2] + zDim[2]);
	//		glVertex3d(origin[0] + xDim[0] + zDim[0], origin[1] + xDim[1] + zDim[1], origin[2] + xDim[2] + zDim[2]);
	//		glEnd();

	//		glBegin(GL_LINE_LOOP);
	//		glVertex3d(origin[0], origin[1], origin[2]);
	//		glVertex3d(origin[0] + yDim[0], origin[1] + yDim[1], origin[2] + yDim[2]);
	//		glVertex3d(origin[0] + xDim[0] + yDim[0], origin[1] + xDim[1] + yDim[1], origin[2] + xDim[2] + yDim[2]);
	//		glVertex3d(origin[0] + xDim[0], origin[1] + xDim[1], origin[2] + xDim[2]);
	//		glEnd();			
	//	} // for (; itFCObjects != ...
	//} // for (size_t iModel = ...

	//glDisable(GL_LINE_STIPPLE);
	//glEnable(GL_LIGHTING);

	//_oglUtils::checkForErrors();
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
void COpenGLIFCView::DrawWireframes()
{
	if (!m_bShowWireframes)
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

	CRect rcClient;
	m_pWnd->GetClientRect(&rcClient);

	if ((rcClient.Width() < 100) || (rcClient.Height() < 100))
	{
		return;
	}

	/*
	* Frame buffer
	*/
	if (m_iSelectionFrameBuffer == 0)
	{
		assert(m_iSelectionTextureBuffer == 0);
		assert(m_iSelectionDepthRenderBuffer == 0);

		/*
		* Frame buffer
		*/
		glGenFramebuffers(1, &m_iSelectionFrameBuffer);
		assert(m_iSelectionFrameBuffer != 0);

		glBindFramebuffer(GL_FRAMEBUFFER, m_iSelectionFrameBuffer);

		/*
		* Texture buffer
		*/
		glGenTextures(1, &m_iSelectionTextureBuffer);
		assert(m_iSelectionTextureBuffer != 0);

		glBindTexture(GL_TEXTURE_2D, m_iSelectionTextureBuffer);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SELECTION_BUFFER_SIZE, SELECTION_BUFFER_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		glBindTexture(GL_TEXTURE_2D, 0);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_iSelectionTextureBuffer, 0);

		/*
		* Depth buffer
		*/
		glGenRenderbuffers(1, &m_iSelectionDepthRenderBuffer);
		assert(m_iSelectionDepthRenderBuffer != 0);

		glBindRenderbuffer(GL_RENDERBUFFER, m_iSelectionDepthRenderBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SELECTION_BUFFER_SIZE, SELECTION_BUFFER_SIZE);

		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_iSelectionDepthRenderBuffer);

		GLenum arDrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, arDrawBuffers);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		_oglUtils::checkForErrors();
	} // if (m_iSelectionFrameBuffer == 0)

	/*
	* Scene
	*/
	glBindFramebuffer(GL_FRAMEBUFFER, m_iSelectionFrameBuffer);

	glViewport(0, 0, SELECTION_BUFFER_SIZE, SELECTION_BUFFER_SIZE);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
void COpenGLIFCView::DrawConceptualFacesFrameBuffer()
{
	if (m_enViewMode != vmMeasureArea)
	{
		return;
	}

	if (!m_bShowFaces)
	{
		return;
	}

	// #####

	//CRect rcClient;
	//m_pWnd->GetClientRect(&rcClient);

	//if ((rcClient.Width() < 100) || (rcClient.Height() < 100))
	//{
	//	return;
	//}

	///*
	//* Frame buffer
	//*/
	//if (m_iFacesFrameBuffer == 0)
	//{
	//	assert(m_iFacesTextureBuffer == 0);
	//	assert(m_iFacesDepthRenderBuffer == 0);

	//	/*
	//	* Frame buffer
	//	*/
	//	glGenFramebuffers(1, &m_iFacesFrameBuffer);
	//	assert(m_iFacesFrameBuffer != 0);

	//	glBindFramebuffer(GL_FRAMEBUFFER, m_iFacesFrameBuffer);

	//	/*
	//	* Texture buffer
	//	*/
	//	glGenTextures(1, &m_iFacesTextureBuffer);
	//	assert(m_iFacesTextureBuffer != 0);

	//	glBindTexture(GL_TEXTURE_2D, m_iFacesTextureBuffer);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SELECTION_BUFFER_SIZE, SELECTION_BUFFER_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	//	glBindTexture(GL_TEXTURE_2D, 0);

	//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_iFacesTextureBuffer, 0);

	//	/*
	//	* Depth buffer
	//	*/
	//	glGenRenderbuffers(1, &m_iFacesDepthRenderBuffer);
	//	assert(m_iFacesDepthRenderBuffer != 0);

	//	glBindRenderbuffer(GL_RENDERBUFFER, m_iFacesDepthRenderBuffer);
	//	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SELECTION_BUFFER_SIZE, SELECTION_BUFFER_SIZE);

	//	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	//	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_iFacesDepthRenderBuffer);

	//	GLenum arDrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	//	glDrawBuffers(1, arDrawBuffers);

	//	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//	_oglUtils::checkForErrors();
	//} // if (m_iFacesFrameBuffer == 0)

	///*
	//* Scene
	//*/
	//glBindFramebuffer(GL_FRAMEBUFFER, m_iFacesFrameBuffer);

	//glViewport(0, 0, SELECTION_BUFFER_SIZE, SELECTION_BUFFER_SIZE);

	//glClearColor(0.0, 0.0, 0.0, 0.0);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//// Set up the parameters
	//glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LEQUAL);

	//glEnable(GL_COLOR_MATERIAL);

	//glShadeModel(GL_FLAT);

	//glDisable(GL_LIGHTING);

	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();

	//// fovY     - Field of vision in degrees in the y direction
	//// aspect   - Aspect ratio of the viewport
	//// zNear    - The near clipping distance
	//// zFar     - The far clipping distance
	//GLdouble fovY = 45.0;
	//GLdouble aspect = (GLdouble)rcClient.Width() / (GLdouble)rcClient.Height();
	//GLdouble zNear = 0.001;
	//GLdouble zFar = 100000.0;

	//GLdouble fH = tan(fovY / 360 * M_PI) * zNear;
	//GLdouble fW = fH * aspect;

	//glFrustum(-fW, fW, -fH, fH, zNear, zFar);

	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();

	///*
	//* Pan/Zoom
	//*/
	//glTranslated(m_dXTranslation, m_dYTranslation, m_dZTranslation);

	///*
	//* Move the origin
	//*/
	//glTranslated(m_dOriginX, m_dOriginY, m_dOriginZ);

	///*
	//* Rotate
	//*/
	//glRotated(m_dXAngle, 1.0f, 0.0f, 0.0f);
	//glRotated(m_dYAngle, 0.0f, 0.0f, 1.0f);

	///*
	//* Restore the origin
	//*/
	//glTranslated(-m_dOriginX, -m_dOriginY, -m_dOriginZ);

	///*
	//* Draw
	//*/
	//glEnableClientState(GL_VERTEX_ARRAY);	

	//const float STEP = 1.0f / 255.0f;

	//map<CIFCModel *, vector<CIFCDrawMetaData *>>::iterator itModel2DrawMetaData = m_vecIFCDrawMetaData.begin();
	//for (; itModel2DrawMetaData != m_vecIFCDrawMetaData.end(); itModel2DrawMetaData++)
	//{
	//	for (size_t iDrawMetaData = 0; iDrawMetaData < itModel2DrawMetaData->second.size(); iDrawMetaData++)
	//	{
	//		const map<GLuint, vector<CIFCObject *>> & mapGroups = itModel2DrawMetaData->second[iDrawMetaData]->getGroups();

	//		map<GLuint, vector<CIFCObject *>>::const_iterator itGroups = mapGroups.begin();
	//		for (; itGroups != mapGroups.end(); itGroups++)
	//		{
	//			glBindBuffer(GL_ARRAY_BUFFER, itGroups->first);
	//			glVertexPointer(3, GL_FLOAT, sizeof(GLfloat) * GEOMETRY_VBO_VERTEX_LENGTH, NULL);

	//			GLsizei iOffset = 0;
	//			for (size_t iObject = 0; iObject < itGroups->second.size(); iObject++)
	//			{
	//				CIFCObject * pIFCObject = itGroups->second[iObject];
	//				if (pIFCObject != m_pPickedIFCObject)
	//				{
	//					iOffset += (GLsizei)pIFCObject->verticesCount();

	//					continue;
	//				}

	//				/*
	//				* Conceptual faces
	//				*/
	//				for (size_t iMaterial = 0; iMaterial < pIFCObject->conceptualFacesMaterials().size(); iMaterial++)
	//				{
	//					CIFCGeometryWithMaterial * pGeometryWithMaterial = pIFCObject->conceptualFacesMaterials()[iMaterial];

	//					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pGeometryWithMaterial->IBO());

	//					for (size_t iConceptualFace = 0; iConceptualFace < pGeometryWithMaterial->conceptualFaces().size(); iConceptualFace++)
	//					{
	//						int64_t iFaceID = pGeometryWithMaterial->conceptualFaces()[iConceptualFace].index() + 1;
	//						int64_t iStartIndex = pGeometryWithMaterial->conceptualFaces()[iConceptualFace].trianglesStartIndex();
	//						int64_t iIndicesCount = pGeometryWithMaterial->conceptualFaces()[iConceptualFace].trianglesIndicesCount();
	//						
	//						/*
	//						* Unique color
	//						*/
	//						float fR = floor((float)iFaceID / (255.0f * 255.0f));
	//						if (fR >= 1.0f)
	//						{
	//							fR *= STEP;
	//						}

	//						float fG = floor((float)iFaceID / 255.0f);
	//						if (fG >= 1.0f)
	//						{
	//							fG *= STEP;
	//						}

	//						float fB = (float)(iFaceID % 255);
	//						fB *= STEP;

	//						/*
	//						* Ambient color
	//						*/
	//						glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT);
	//						glColor4f(
	//							fR,
	//							fG,
	//							fB,
	//							1.0f);

	//						glDrawElementsBaseVertex(GL_TRIANGLES,
	//							(GLsizei)iIndicesCount,
	//							GL_UNSIGNED_INT,
	//							(void *)(sizeof(GLuint) * (pGeometryWithMaterial->IBOOffset() + iStartIndex)),
	//							iOffset);
	//					} // for (size_t iConceptualFace = ...

	//					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);						
	//				} // for (size_t iMaterial = ...

	//				iOffset += (GLsizei)pIFCObject->verticesCount();
	//			} // for (size_t iObject = ...

	//			glBindBuffer(GL_ARRAY_BUFFER, 0);
	//		} // for (; itGroups != ...
	//	} // for (size_t iDrawMetaData = ...
	//} // for (; itModel2DrawMetaData != ...

	//glDisableClientState(GL_VERTEX_ARRAY);

	//glEnable(GL_LIGHTING);

	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//_oglUtils::checkForErrors();
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::DrawWireframesFrameBuffer()
{
	if (m_enViewMode != vmMeasureEdge)
	{
		return;
	}

	// #####

	//CRect rcClient;
	//m_pWnd->GetClientRect(&rcClient);

	//if ((rcClient.Width() < 100) || (rcClient.Height() < 100))
	//{
	//	return;
	//}

	///*
	//* Frame buffer
	//*/
	//if (m_iWireframesFrameBuffer == 0)
	//{
	//	assert(m_iWireframesTextureBuffer == 0);
	//	assert(m_iWireframesDepthRenderBuffer == 0);

	//	/*
	//	* Frame buffer
	//	*/
	//	glGenFramebuffers(1, &m_iWireframesFrameBuffer);
	//	assert(m_iWireframesFrameBuffer != 0);

	//	glBindFramebuffer(GL_FRAMEBUFFER, m_iWireframesFrameBuffer);

	//	/*
	//	* Texture buffer
	//	*/
	//	glGenTextures(1, &m_iWireframesTextureBuffer);
	//	assert(m_iWireframesTextureBuffer != 0);

	//	glBindTexture(GL_TEXTURE_2D, m_iWireframesTextureBuffer);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SELECTION_BUFFER_SIZE, SELECTION_BUFFER_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	//	glBindTexture(GL_TEXTURE_2D, 0);

	//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_iWireframesTextureBuffer, 0);

	//	/*
	//	* Depth buffer
	//	*/
	//	glGenRenderbuffers(1, &m_iWireframesDepthRenderBuffer);
	//	assert(m_iWireframesDepthRenderBuffer != 0);

	//	glBindRenderbuffer(GL_RENDERBUFFER, m_iWireframesDepthRenderBuffer);
	//	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SELECTION_BUFFER_SIZE, SELECTION_BUFFER_SIZE);

	//	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	//	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_iWireframesDepthRenderBuffer);

	//	GLenum arDrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	//	glDrawBuffers(1, arDrawBuffers);

	//	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//	_oglUtils::checkForErrors();
	//} // if (m_iWireframesFrameBuffer == 0)

	///*
	//* Scene
	//*/
	//glBindFramebuffer(GL_FRAMEBUFFER, m_iWireframesFrameBuffer);

	//glViewport(0, 0, SELECTION_BUFFER_SIZE, SELECTION_BUFFER_SIZE);

	//glClearColor(0.0, 0.0, 0.0, 0.0);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//// Set up the parameters
	//glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LEQUAL);

	//glEnable(GL_COLOR_MATERIAL);

	//glShadeModel(GL_FLAT);

	//glDisable(GL_LIGHTING);

	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();

	//// fovY     - Field of vision in degrees in the y direction
	//// aspect   - Aspect ratio of the viewport
	//// zNear    - The near clipping distance
	//// zFar     - The far clipping distance
	//GLdouble fovY = 45.0;
	//GLdouble aspect = (GLdouble)rcClient.Width() / (GLdouble)rcClient.Height();
	//GLdouble zNear = 0.001;
	//GLdouble zFar = 100000.0;

	//GLdouble fH = tan(fovY / 360 * M_PI) * zNear;
	//GLdouble fW = fH * aspect;

	//glFrustum(-fW, fW, -fH, fH, zNear, zFar);

	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();

	///*
	//* Pan/Zoom
	//*/
	//glTranslated(m_dXTranslation, m_dYTranslation, m_dZTranslation);

	///*
	//* Move the origin
	//*/
	//glTranslated(m_dOriginX, m_dOriginY, m_dOriginZ);

	///*
	//* Rotate
	//*/
	//glRotated(m_dXAngle, 1.0f, 0.0f, 0.0f);
	//glRotated(m_dYAngle, 0.0f, 0.0f, 1.0f);

	///*
	//* Restore the origin
	//*/
	//glTranslated(-m_dOriginX, -m_dOriginY, -m_dOriginZ);

	///*
	//* Draw
	//*/
	//glEnableClientState(GL_VERTEX_ARRAY);

	///*
	//* Conceptual faces with the background color
	//*/
	//map<CIFCModel *, vector<CIFCDrawMetaData *>>::iterator itModel2DrawMetaData = m_vecIFCDrawMetaData.begin();
	//for (; itModel2DrawMetaData != m_vecIFCDrawMetaData.end(); itModel2DrawMetaData++)
	//{
	//	CIFCModel* pIFCModel = itModel2DrawMetaData->first;

	//	for (size_t iDrawMetaData = 0; iDrawMetaData < itModel2DrawMetaData->second.size(); iDrawMetaData++)
	//	{
	//		const map<GLuint, vector<CIFCObject *>> & mapGroups = itModel2DrawMetaData->second[iDrawMetaData]->getGroups();

	//		map<GLuint, vector<CIFCObject *>>::const_iterator itGroups = mapGroups.begin();
	//		for (; itGroups != mapGroups.end(); itGroups++)
	//		{
	//			glBindBuffer(GL_ARRAY_BUFFER, itGroups->first);
	//			glVertexPointer(3, GL_FLOAT, sizeof(GLfloat) * GEOMETRY_VBO_VERTEX_LENGTH, NULL);

	//			GLsizei iOffset = 0;
	//			for (size_t iObject = 0; iObject < itGroups->second.size(); iObject++)
	//			{
	//				CIFCObject * pIFCObject = itGroups->second[iObject];
	//				if ((pIFCObject != m_pPickedIFCObject) || !pIFCObject->visible__() || !pIFCObject->AreFacesShown() || 
	//					(m_bDetailsViewMode ? pIFCModel->getSubSelection() != NULL ? pIFCModel->getSubSelection() != pIFCObject : !pIFCObject->selected() : false))
	//				{
	//					iOffset += (GLsizei)pIFCObject->verticesCount();

	//					continue;
	//				}

	//				/*
	//				* Conceptual faces
	//				*/
	//				for (size_t iMaterial = 0; iMaterial < pIFCObject->conceptualFacesMaterials().size(); iMaterial++)
	//				{
	//					CIFCGeometryWithMaterial * pGeometryWithMaterial = pIFCObject->conceptualFacesMaterials()[iMaterial];

	//					/*
	//					* Ambient color - use the background color
	//					*/
	//					glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT);
	//					glColor4f(
	//						0.f,
	//						0.f,
	//						0.f,
	//						1.f);

	//					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pGeometryWithMaterial->IBO());
	//					glDrawElementsBaseVertex(GL_TRIANGLES,
	//						(GLsizei)pGeometryWithMaterial->getIndicesCount(),
	//						GL_UNSIGNED_INT,
	//						(void *)(sizeof(GLuint) * pGeometryWithMaterial->IBOOffset()),
	//						iOffset);
	//					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//				} // for (size_t iMaterial = ...

	//				iOffset += (GLsizei)pIFCObject->verticesCount();
	//			} // for (size_t iObject = ...

	//			glBindBuffer(GL_ARRAY_BUFFER, 0);
	//		} // for (; itGroups != ...
	//	} // for (size_t iDrawMetaData = ...
	//} // for (; itModel2DrawMetaData != ...

	///*
	//* Wireframes
	//*/

	//glLineWidth(5.0);
	//
	//// One-based index used for the selection frame buffer
	//int_t iLineID = 1;

	//const float STEP = 1.0f / 255.0f;

	//itModel2DrawMetaData = m_vecIFCDrawMetaData.begin();
	//for (; itModel2DrawMetaData != m_vecIFCDrawMetaData.end(); itModel2DrawMetaData++)
	//{
	//	for (size_t iDrawMetaData = 0; iDrawMetaData < itModel2DrawMetaData->second.size(); iDrawMetaData++)
	//	{
	//		const map<GLuint, vector<CIFCObject *>> & mapGroups = itModel2DrawMetaData->second[iDrawMetaData]->getGroups();

	//		map<GLuint, vector<CIFCObject *>>::const_iterator itGroups = mapGroups.begin();
	//		for (; itGroups != mapGroups.end(); itGroups++)
	//		{
	//			glBindBuffer(GL_ARRAY_BUFFER, itGroups->first);
	//			glVertexPointer(3, GL_FLOAT, sizeof(GLfloat) * GEOMETRY_VBO_VERTEX_LENGTH, NULL);

	//			GLsizei iOffset = 0;
	//			for (size_t iObject = 0; iObject < itGroups->second.size(); iObject++)
	//			{
	//				CIFCObject * pIFCObject = itGroups->second[iObject];
	//				if ((pIFCObject != m_pPickedIFCObject) || !pIFCObject->visible__() || !pIFCObject->AreFacesShown())
	//				{
	//					iOffset += (GLsizei)pIFCObject->verticesCount();

	//					continue;
	//				}

	//				/*
	//				* Wireframes
	//				*/
	//				for (size_t iWireframesCohort = 0; iWireframesCohort < pIFCObject->wireframesCohorts().size(); iWireframesCohort++)
	//				{
	//					CWireframesCohort * pWireframesCohort = pIFCObject->wireframesCohorts()[iWireframesCohort];

	//					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pWireframesCohort->IBO());

	//					for (int_t iLine = 0; iLine < pWireframesCohort->getIndicesCount() / 2; iLine++)
	//					{
	//						/*
	//						* Unique color
	//						*/
	//						float fR = floor((float)iLineID / (255.0f * 255.0f));
	//						if (fR >= 1.0f)
	//						{
	//							fR *= STEP;
	//						}

	//						float fG = floor((float)iLineID / 255.0f);
	//						if (fG >= 1.0f)
	//						{
	//							fG *= STEP;
	//						}

	//						float fB = (float)(iLineID % 255);
	//						fB *= STEP;

	//						iLineID++;

	//						glColor4f(fR, fG, fB, 1.0);

	//						/*
	//						* Draw
	//						*/
	//						glDrawElementsBaseVertex(GL_LINES,
	//							(GLsizei)2,
	//							GL_UNSIGNED_INT,
	//							(void *)(sizeof(GLuint) * (pWireframesCohort->IBOOffset() + (iLine * 2))),
	//							iOffset);
	//					} // for (int_t iLine = ...

	//					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//				} // for (size_t iWireframesCohort = ...

	//				iOffset += (GLsizei)pIFCObject->verticesCount();
	//			} // for (size_t iObject = ...

	//			glBindBuffer(GL_ARRAY_BUFFER, 0);
	//		} // for (; itGroups != ...
	//	} // for (size_t iDrawMetaData = ...
	//} // for (; itModel2DrawMetaData != ...

	//glDisableClientState(GL_VERTEX_ARRAY);

	//glEnable(GL_LIGHTING);

	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//_oglUtils::checkForErrors();
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
	if (!m_bInteractionInProgress && m_bShowFaces && (m_iSelectionFrameBuffer != 0))
	{
		//m_pOGLContext->MakeCurrent();

		CRect rcClient;
		m_pWnd->GetClientRect(&rcClient);

		glBindFramebuffer(GL_FRAMEBUFFER, m_iSelectionFrameBuffer);
		
		GLubyte arPixels[4];
		memset(arPixels, 0, sizeof(GLubyte) * 4);

		double dX = (double)point.x * ((double)SELECTION_BUFFER_SIZE / (double)rcClient.Width());
		double dY = ((double)rcClient.Height() - (double)point.y) * ((double)SELECTION_BUFFER_SIZE / (double)rcClient.Height());

		CIFCModel * pPickedIFCObjectModel = NULL;
		CIFCObject * pPickedIFCObject = NULL;

		glReadPixels(
			(GLint)dX,
			(GLint)dY,
			1, 1,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			arPixels);

		if (arPixels[3] != 0)
		{
			int_t iObjectID =
				(arPixels[0/*R*/] * (255 * 255)) +
				(arPixels[1/*G*/] * 255) +
				 arPixels[2/*B*/];

			pPickedIFCObjectModel = pModel;
			pPickedIFCObject = pModel->getIFCObject(iObjectID);
			
			ASSERT(pPickedIFCObjectModel != NULL);
			ASSERT(pPickedIFCObject != NULL);
		} // if (arPixels[3] != 0)

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		if ((m_pPickedIFCObjectModel != pPickedIFCObjectModel) || (m_pPickedIFCObject != pPickedIFCObject))
		{
			m_pPickedIFCObjectModel = pPickedIFCObjectModel;
			m_pPickedIFCObject = pPickedIFCObject;
			m_setPickedIFCObjectEdges.clear();
			m_iPickedIFCObjectFace = -1;
			m_pickedPoint3D = CPoint3D(-DBL_MAX, -DBL_MAX, -DBL_MAX);
			
			/*if ((m_pPickedIFCObject != NULL) && (m_enViewMode == vmMeasureVolume))
			{
				double dVolume = m_pPickedIFCObject->volume();
				GetController()->FireOnMeasureVolumeEvent(point.x, point.y, dVolume);

				CString strTooltip;
				strTooltip.Format(_T("%.2f"), dVolume);

				CString strTitle;
				VERIFY(strTitle.LoadStringW(IDS_VOLUME));

				const CIFCUnit * pUnit = m_pPickedIFCObjectModel->getUnit(L"VOLUMEUNIT");
				if ((pUnit != NULL) && !pUnit->getUnit().empty())
				{
					strTooltip += L" ";
					strTooltip += pUnit->getUnit().c_str();
				}

				ShowTooltip(strTitle, strTooltip);
			}*/

			m_pWnd->RedrawWindow();
		}
	} // if (!m_bInteractionInProgress && ...

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

	if (m_ptPrevMousePosition.x == -1)
	{
		return;
	}

	/*
	* Rotate
	*/
	if ((nFlags & MK_LBUTTON) == MK_LBUTTON)
	{
		double dXAngle = ((double)point.y - (double)m_ptPrevMousePosition.y);
		double dYAngle = ((double)point.x - (double)m_ptPrevMousePosition.x);

		const double ROTATION_SPEED = 1. / 350.;
		const double ROTATION_SENSITIVITY = 1.1;

		if (abs(dXAngle) >= abs(dYAngle) * ROTATION_SENSITIVITY)
		{
			dYAngle = 0.;
		}
		else
		{
			if (abs(dYAngle) >= abs(dXAngle) * ROTATION_SENSITIVITY)
			{
				dXAngle = 0.;
			}
		}

		Rotate(dXAngle * ROTATION_SPEED, dYAngle * ROTATION_SPEED);

		m_ptPrevMousePosition = point;

		return;
	}

	/*
	* Zoom
	*/
	if ((nFlags & MK_MBUTTON) == MK_MBUTTON)
	{
		Zoom(point.y - m_ptPrevMousePosition.y > 0 ? -abs(m_dZTranslation) * ZOOM_SPEED_2 : abs(m_dZTranslation) * ZOOM_SPEED_2);

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

		m_dXTranslation += 4.f * (((double)point.x - (double)m_ptPrevMousePosition.x) / rcClient.Width());
		m_dYTranslation -= 4.f * (((double)point.y - (double)m_ptPrevMousePosition.y) / rcClient.Height());

		m_pWnd->RedrawWindow();

		m_ptPrevMousePosition = point;

		return;
	}
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::Rotate(double dXSpin, double dYSpin)
{
	// Rotate
	m_dXAngle += dXSpin * (180. / M_PI);
	if (m_dXAngle > 360.0)
	{
		m_dXAngle = m_dXAngle - 360.0f;
	}
	else
	{
		if (m_dXAngle < 0.0)
		{
			m_dXAngle = m_dXAngle + 360.0f;
		}
	}

	m_dYAngle += dYSpin * (180. / M_PI);
	if (m_dYAngle > 360.0)
	{
		m_dYAngle = m_dYAngle - 360.0f;
	}
	else
	{
		if (m_dYAngle < 0.0)
		{
			m_dYAngle = m_dYAngle + 360.0f;
		}
	}

	m_pWnd->RedrawWindow();
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::Zoom(double dZTranslation)
{
	m_dZTranslation += dZTranslation;

	m_pWnd->RedrawWindow();
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::ZoomToCoreCalculate(CIFCObject * pIFCObject, double * pMinX, double * pMaxX, double * pMinY, double * pMaxY, double * pMinZ, double * pMaxZ, bool * pInitialized)
{
	if (pIFCObject == NULL)
	{
		ASSERT(FALSE);

		return;
	}

	if (*pInitialized == false) {
		*pMinX = pIFCObject->getXMinMax().first;
		*pMaxX = pIFCObject->getXMinMax().second;

		*pMinY = pIFCObject->getYMinMax().first;
		*pMaxY = pIFCObject->getYMinMax().second;

		*pMinZ = pIFCObject->getZMinMax().first;
		*pMaxZ = pIFCObject->getZMinMax().second;

		*pInitialized = true;
	}
	else {
		if (*pMinX > pIFCObject->getXMinMax().first) {
			*pMinX = pIFCObject->getXMinMax().first;
		}
		if (*pMaxX < pIFCObject->getXMinMax().second) {
			*pMaxX = pIFCObject->getXMinMax().second;
		}

		if (*pMinY > pIFCObject->getYMinMax().first) {
			*pMinY = pIFCObject->getYMinMax().first;
		}
		if (*pMaxY < pIFCObject->getYMinMax().second) {
			*pMaxY = pIFCObject->getYMinMax().second;
		}

		if (*pMinZ > pIFCObject->getZMinMax().first) {
			*pMinZ = pIFCObject->getZMinMax().first;
		}
		if (*pMaxZ < pIFCObject->getZMinMax().second) {
			*pMaxZ = pIFCObject->getZMinMax().second;
		}
	}
}

void COpenGLIFCView::ZoomToCoreSet(double minX, double maxX, double minY, double maxY, double minZ, double maxZ)
{
	m_dOriginX = minX;
	m_dOriginX += (maxX - minX) / 2.f;

	m_dOriginY = minY;
	m_dOriginY += (maxY - minY) / 2.f;

	m_dOriginZ = minZ;
	m_dOriginZ += (maxZ - minZ) / 2.f;

	double dBoundingSphereDiameter = maxX - minX;
	dBoundingSphereDiameter = fmax(dBoundingSphereDiameter, maxY - minY);
	dBoundingSphereDiameter = fmax(dBoundingSphereDiameter, maxZ - minZ);

	m_dXTranslation = -m_dOriginX;
	m_dYTranslation = -m_dOriginY;
	m_dZTranslation = -m_dOriginZ;
	m_dZTranslation -= (dBoundingSphereDiameter * 2.f);

	m_pWnd->RedrawWindow();
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::CenterToCore(CIFCObject * pIFCObject)
{
	if (pIFCObject == NULL)
	{
		ASSERT(FALSE);

		return;
	}

	m_dOriginX = pIFCObject->getXMinMax().first;
	m_dOriginX += (pIFCObject->getXMinMax().second - pIFCObject->getXMinMax().first) / 2.f;

	m_dOriginY = pIFCObject->getYMinMax().first;
	m_dOriginY += (pIFCObject->getYMinMax().second - pIFCObject->getYMinMax().first) / 2.f;

	m_dOriginZ = pIFCObject->getZMinMax().first;
	m_dOriginZ += (pIFCObject->getZMinMax().second - pIFCObject->getZMinMax().first) / 2.f;

	m_dXTranslation = -m_dOriginX;
	m_dYTranslation = -m_dOriginY;

	m_pWnd->RedrawWindow();
}

// ------------------------------------------------------------------------------------------------
// http://nehe.gamedev.net/article/using_gluunproject/16013/
bool COpenGLIFCView::GetOGLPos(int iX, int iY, float fDepth, GLdouble & dX, GLdouble & dY, GLdouble & dZ) const
{
	CRect rcClient;
	m_pWnd->GetClientRect(&rcClient);
	
	GLint arViewport[4] = { 0, 0, rcClient.Width(), rcClient.Height() };
	GLdouble arModelView[16];
	GLdouble arProjection[16];
	GLdouble dWinX, dWinY, dWinZ;

	glGetDoublev(GL_MODELVIEW_MATRIX, arModelView);
	glGetDoublev(GL_PROJECTION_MATRIX, arProjection);
	//glGetIntegerv(GL_VIEWPORT, arViewport);

	dWinX = (double)iX;
	dWinY = (double)arViewport[3] - (double)iY - 1;

	if (fDepth == -FLT_MAX)
	{
		float fWinZ = 0.f;
		glReadPixels(iX, (int)dWinY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &fWinZ);

		dWinZ = fWinZ;

		_oglUtils::checkForErrors();
	}
	else
	{
		dWinZ = fDepth;
	}

	GLint iResult = gluUnProject(dWinX, dWinY, dWinZ, arModelView, arProjection, arViewport, &dX, &dY, &dZ);

	_oglUtils::checkForErrors();

	return iResult == GL_TRUE;
}

// ------------------------------------------------------------------------------------------------
// https://docs.microsoft.com/en-us/windows/desktop/api/wingdi/nf-wingdi-wglusefontbitmapsa
bool COpenGLIFCView::DrawTextGDI(const wchar_t * szText, float fX, float fY, float fZ)
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
double COpenGLIFCView::GetPickedEdgesLength()
{ 
	// All lengths
	double dSum = 0.;

	// #####
	//if ((m_enViewMode == vmMeasureEdge) && (m_pPickedIFCObject != NULL) && !m_setPickedIFCObjectEdges.empty())
	//{
	//	// One-based index used for the selection frame buffer
	//	int_t iLineID = 1;

	//	map<CIFCModel *, vector<CIFCDrawMetaData *>>::iterator itModel2DrawMetaData = m_vecIFCDrawMetaData.begin();
	//	for (; itModel2DrawMetaData != m_vecIFCDrawMetaData.end(); itModel2DrawMetaData++)
	//	{
	//		for (size_t iDrawMetaData = 0; iDrawMetaData < itModel2DrawMetaData->second.size(); iDrawMetaData++)
	//		{
	//			const map<GLuint, vector<CIFCObject *>> & mapGroups = itModel2DrawMetaData->second[iDrawMetaData]->getGroups();

	//			map<GLuint, vector<CIFCObject *>>::const_iterator itGroups = mapGroups.begin();
	//			for (; itGroups != mapGroups.end(); itGroups++)
	//			{
	//				glBindBuffer(GL_ARRAY_BUFFER, itGroups->first);
	//				glVertexPointer(3, GL_FLOAT, sizeof(GLfloat) * GEOMETRY_VBO_VERTEX_LENGTH, NULL);

	//				GLsizei iOffset = 0;
	//				for (size_t iObject = 0; iObject < itGroups->second.size(); iObject++)
	//				{
	//					CIFCObject * pIFCObject = itGroups->second[iObject];
	//					if (!pIFCObject->visible__() || !pIFCObject->AreFacesShown())
	//					{
	//						iOffset += (GLsizei)pIFCObject->verticesCount();

	//						continue;
	//					}
	//					
	//					/*
	//					* Picked edge
	//					*/
	//					if ((m_enViewMode == vmMeasureEdge) && (pIFCObject == m_pPickedIFCObject) && !m_setPickedIFCObjectEdges.empty())
	//					{
	//						for (size_t iWireframesCohort = 0; iWireframesCohort < pIFCObject->wireframesCohorts().size(); iWireframesCohort++)
	//						{
	//							CWireframesCohort * pWireframesCohort = pIFCObject->wireframesCohorts()[iWireframesCohort];

	//							for (int_t iLine = 0; iLine < pWireframesCohort->getIndicesCount() / 2; iLine++)
	//							{
	//								set<int_t>::iterator itEdge = m_setPickedIFCObjectEdges.find(iLineID++);
	//								if (itEdge != m_setPickedIFCObjectEdges.end())
	//								{
	//									glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pWireframesCohort->IBO());

	//									/*
	//									* Read the indices
	//									*/
	//									GLuint indices[2];
	//									glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER,
	//										sizeof(GLuint) * (pWireframesCohort->IBOOffset() + (iLine * 2)),
	//										2 * sizeof(GLuint),
	//										indices);

	//									/*
	//									* Read the vertices
	//									*/
	//									GLfloat vertex1[GEOMETRY_VBO_VERTEX_LENGTH];
	//									glGetBufferSubData(GL_ARRAY_BUFFER,
	//										sizeof(GLfloat) * ((iOffset + indices[0]) * GEOMETRY_VBO_VERTEX_LENGTH),
	//										GEOMETRY_VBO_VERTEX_LENGTH * sizeof(GLfloat), vertex1);

	//									GLfloat vertex2[GEOMETRY_VBO_VERTEX_LENGTH];
	//									glGetBufferSubData(GL_ARRAY_BUFFER,
	//										sizeof(GLfloat) * ((iOffset + indices[1]) * GEOMETRY_VBO_VERTEX_LENGTH),
	//										GEOMETRY_VBO_VERTEX_LENGTH * sizeof(GLfloat), vertex2);

	//									double dLength = sqrt(pow(vertex1[0] - vertex2[0], 2.) + pow(vertex1[1] - vertex2[1], 2.) + pow(vertex1[2] - vertex2[2], 2.));
	//									dLength = (m_dScaleFactor * dLength) / 2.;

	//									dSum += dLength;

	//									glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);										
	//								} // if (itEdge != ...
	//							} // for (int_t iLine = ...
	//						} // for (size_t iWireframesCohort = ...
	//					} // if ((m_enViewMode == vmMeasureEdge) && ...

	//					iOffset += (GLsizei)pIFCObject->verticesCount();
	//				} // for (size_t iObject = ...

	//				glBindBuffer(GL_ARRAY_BUFFER, 0);
	//			} // for (; itGroups != ...
	//		} // for (size_t iDrawMetaData = ...
	//	} // for (; itModel2DrawMetaData != ...
	//} // if ((m_enViewMode == vmMeasureEdge) && ...

	return dSum;
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::ShowTooltip(const CString & strTitle, const CString & strText)
{
	ASSERT(m_toolTipCtrl.GetToolCount() <= 1);

	if (m_toolTipCtrl.GetToolCount() == 1)
	{
		CToolInfo toolInfo;
		m_toolTipCtrl.GetToolInfo(toolInfo, m_pWnd);

		if (strText == toolInfo.lpszText)
		{
			return;
		}

		toolInfo.lpszText = (LPTSTR)(LPCTSTR)strText;
		m_toolTipCtrl.SetToolInfo(&toolInfo);
	} // if (m_toolTipCtrl.GetToolCount() == 1)
	else
	{
		m_toolTipCtrl.AddTool(m_pWnd, strText);
	}

	m_toolTipCtrl.SetTitle(0, strTitle);

	m_toolTipCtrl.Popup();
}

// ------------------------------------------------------------------------------------------------
void COpenGLIFCView::HideTooltip()
{
	ASSERT(m_toolTipCtrl.GetToolCount() <= 1);

	if (m_toolTipCtrl.GetToolCount() == 1)
	{
		m_toolTipCtrl.Pop();
	}
}