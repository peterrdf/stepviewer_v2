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
COpenGLLight::COpenGLLight(GLenum enLight)
{
	m_bIsEnabled = false;

	m_enLight = enLight;

	m_arAmbient[0] = 0.f;
	m_arAmbient[1] = 0.f;
	m_arAmbient[2] = 0.f;
	m_arAmbient[3] = 1.f;

	m_arDiffuse[0] = 1.f;
	m_arDiffuse[1] = 1.f;
	m_arDiffuse[2] = 1.f;
	m_arDiffuse[3] = 1.f;

	m_arSpecular[0] = 1.f;
	m_arSpecular[1] = 1.f;
	m_arSpecular[2] = 1.f;
	m_arSpecular[3] = 1.f;

	m_arPosition[0] = 1.f;
	m_arPosition[1] = 1.f;
	m_arPosition[2] = 1.f;
	m_arPosition[3] = 0.f; // directional light
}

// ------------------------------------------------------------------------------------------------
COpenGLLight::COpenGLLight(const COpenGLLight & light)
{
	m_bIsEnabled = light.m_bIsEnabled;

	m_enLight = light.m_enLight;

	m_arAmbient[0] = light.m_arAmbient[0];
	m_arAmbient[1] = light.m_arAmbient[1];
	m_arAmbient[2] = light.m_arAmbient[2];
	m_arAmbient[3] = light.m_arAmbient[3];

	m_arDiffuse[0] = light.m_arDiffuse[0];
	m_arDiffuse[1] = light.m_arDiffuse[1];
	m_arDiffuse[2] = light.m_arDiffuse[2];
	m_arDiffuse[3] = light.m_arDiffuse[3];

	m_arSpecular[0] = light.m_arSpecular[0];
	m_arSpecular[1] = light.m_arSpecular[1];
	m_arSpecular[2] = light.m_arSpecular[2];
	m_arSpecular[3] = light.m_arSpecular[3];

	m_arPosition[0] = light.m_arPosition[0];
	m_arPosition[1] = light.m_arPosition[1];
	m_arPosition[2] = light.m_arPosition[2];
	m_arPosition[3] = light.m_arPosition[3];
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ COpenGLLight::~COpenGLLight()
{
}

// ------------------------------------------------------------------------------------------------
bool COpenGLLight::isEnabled() const
{
	return m_bIsEnabled;
}

// ------------------------------------------------------------------------------------------------
void COpenGLLight::enable(bool bEnable)
{
	m_bIsEnabled = bEnable;
}

// ------------------------------------------------------------------------------------------------
GLenum COpenGLLight::getLight() const
{
	return m_enLight;
}

// ------------------------------------------------------------------------------------------------
GLfloat * COpenGLLight::getAmbient() const
{
	return (GLfloat *)m_arAmbient;
}

// ------------------------------------------------------------------------------------------------
void COpenGLLight::setAmbient(GLfloat fR, GLfloat fG, GLfloat fB, GLfloat fW/* = 1.f*/)
{
	m_arAmbient[0] = fR;
	m_arAmbient[1] = fG;
	m_arAmbient[2] = fB;
	m_arAmbient[3] = fW;
}

// ------------------------------------------------------------------------------------------------
GLfloat * COpenGLLight::getDiffuse() const
{
	return (GLfloat *)m_arDiffuse;
}

// ------------------------------------------------------------------------------------------------
void COpenGLLight::setDiffuse(GLfloat fR, GLfloat fG, GLfloat fB, GLfloat fW/* = 1.f*/)
{
	m_arDiffuse[0] = fR;
	m_arDiffuse[1] = fG;
	m_arDiffuse[2] = fB;
	m_arDiffuse[3] = fW;
}

// ------------------------------------------------------------------------------------------------
GLfloat * COpenGLLight::getSpecular() const
{
	return (GLfloat *)m_arSpecular;
}

// ------------------------------------------------------------------------------------------------
void COpenGLLight::setSpecular(GLfloat fR, GLfloat fG, GLfloat fB, GLfloat fW/* = 1.f*/)
{
	m_arSpecular[0] = fR;
	m_arSpecular[1] = fG;
	m_arSpecular[2] = fB;
	m_arSpecular[3] = fW;
}

// ------------------------------------------------------------------------------------------------
GLfloat * COpenGLLight::getPosition() const
{
	return (GLfloat *)m_arPosition;
}

// ------------------------------------------------------------------------------------------------
void COpenGLLight::setPosition(GLfloat fX, GLfloat fY, GLfloat fZ, GLfloat fW)
{
	m_arPosition[0] = fX;
	m_arPosition[1] = fY;
	m_arPosition[2] = fZ;
	m_arPosition[3] = fW;
}

// ------------------------------------------------------------------------------------------------
#define SELECTION_BUFFER_SIZE 512

// ------------------------------------------------------------------------------------------------
#ifdef _LINUX
COpenGLSTEPView::COpenGLSTEPView(wxGLCanvas * pWnd)
#else
COpenGLSTEPView::COpenGLSTEPView(CWnd * pWnd)
#endif // _LINUX
	: COpenGLView()
	, m_pWnd(pWnd)
	, m_bShowFaces(TRUE)
	, m_bShowFacesPolygons(FALSE)
	, m_bShowConceptualFacesPolygons(TRUE)
	, m_bShowLines(TRUE)
	, m_fLineWidth(1.f)
	, m_bShowPoints(TRUE)
	, m_fPointSize(1.f)
	, m_bShowBoundingBoxes(FALSE)
	, m_bShowNormalVectors(FALSE)
	, m_bShowTangenVectors(FALSE)
	, m_bShowBiNormalVectors(FALSE)
	, m_bScaleVectors(FALSE)
	, m_bDisableSelectionBuffer(FALSE)
	, m_pOGLContext(NULL)
	, m_vecOGLLights()
	//, m_vecFacesVBOs()
	//, m_vecFacesIBOs()
	//, m_vecWireframesVBOs()
	//, m_vecWireframesIBOs()
	, m_fXAngle(30.0f)
	, m_fYAngle(30.0f)
	, m_fXTranslation(0.0f)
	, m_fYTranslation(0.0f)
	, m_fZTranslation(-5.0f)
	, m_ptStartMousePosition(-1, -1)
	, m_ptPrevMousePosition(-1, -1)
	, m_iInstanceSelectionFrameBuffer(0)
	, m_iInstanceSelectionTextureBuffer(0)
	, m_iInstanceSelectionDepthRenderBuffer(0)
	, m_iFaceSelectionFrameBuffer(0)
	, m_iFaceSelectionTextureBuffer(0)
	, m_iFaceSelectionDepthRenderBuffer(0)
	, m_mapInstancesSelectionColors()
	, m_pPointedInstance(NULL)
	, m_pSelectedInstance(NULL)
	, m_ptSelectedPoint(-1, -1)
	//, m_mapFacesSelectionColors()
	, m_iPointedFace(-1)
	, m_veCSTEPDrawMetaData()
	, m_vecIBOs()
	, m_pSelectedInstanceMaterial(NULL)
	, m_pPointedInstanceMaterial(NULL)
	, m_hFont(NULL)
{
	ASSERT(m_pWnd != NULL);

#ifdef _LINUX
    m_pOGLContext = new wxGLContext(m_pWnd);
#else
    m_pOGLContext = new COpenGLContext(*(m_pWnd->GetDC()));
#endif // _LINUX

	/*
	* Light model
	*/
	// http://www.glprogramming.com/red/chapter05.html#name4
	m_arLightModelAmbient[0] = .2f;
	m_arLightModelAmbient[1] = .2f;
	m_arLightModelAmbient[2] = .2f;
	m_arLightModelAmbient[3] = 1.f;
	m_bLightModelLocalViewer = true;
	m_bLightModel2Sided = false;

	/*
	* Lights: GL_LIGHT0 - GL_LIGHT7
	*/
	for (int iLight = 0; iLight < 8; iLight++)
	{
		m_vecOGLLights.push_back(COpenGLLight(GL_LIGHT0 + iLight));
	}

	/*
	* Enable GL_LIGHT0
	*/
	m_vecOGLLights[0].enable(true);

	m_arSelectedPoint[0] = -FLT_MAX;
	m_arSelectedPoint[1] = -FLT_MAX;
	m_arSelectedPoint[2] = -FLT_MAX;

	m_arCamera[0] = -FLT_MAX;
	m_arCamera[1] = -FLT_MAX;
	m_arCamera[2] = -FLT_MAX;

	/*
	* Default
	*/
	m_pSelectedInstanceMaterial = new CSTEPMaterial();
	m_pSelectedInstanceMaterial->set(
		1, 0, 0,  // ambient
		1, 0, 0,  // diffuse
		1, 0, 0,  // emissive
		1, 0, 0,  //specular
		1,        // transparency	
		NULL);	  // texture

	/*
	* Default
	*/
	m_pPointedInstanceMaterial = new CSTEPMaterial();
	m_pPointedInstanceMaterial->set(
		.33f, .33f, .33f,  // ambient
		.33f, .33f, .33f,  // diffuse
		.33f, .33f, .33f,  // emissive
		.33f, .33f, .33f,  //specular
		1,                 // transparency	
		NULL);	           // texture

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

#ifdef _USE_SHADERS
	m_pOGLContext->MakeCurrent();
	
	//m_pProgram = new CBinnPhongOGLPipeline();

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

	m_modelViewMatrix = glm::identity<glm::mat4>();
#endif // _USE_SHADERS
}

// ------------------------------------------------------------------------------------------------
COpenGLSTEPView::~COpenGLSTEPView()
{
	GetController()->UnRegisterView(this);

	if (m_iInstanceSelectionFrameBuffer != 0)
	{
		glDeleteFramebuffers(1, &m_iInstanceSelectionFrameBuffer);
		m_iInstanceSelectionFrameBuffer = 0;
	}

	if (m_iInstanceSelectionTextureBuffer != 0)
	{
		glDeleteTextures(1, &m_iInstanceSelectionTextureBuffer);
		m_iInstanceSelectionTextureBuffer = 0;
	}

	if (m_iInstanceSelectionDepthRenderBuffer != 0)
	{
		glDeleteRenderbuffers(1, &m_iInstanceSelectionDepthRenderBuffer);
		m_iInstanceSelectionDepthRenderBuffer = 0;
	}

	if (m_iFaceSelectionFrameBuffer != 0)
	{
		glDeleteFramebuffers(1, &m_iFaceSelectionFrameBuffer);
		m_iFaceSelectionFrameBuffer = 0;
	}

	if (m_iFaceSelectionTextureBuffer != 0)
	{
		glDeleteTextures(1, &m_iFaceSelectionTextureBuffer);
		m_iFaceSelectionTextureBuffer = 0;
	}

	if (m_iFaceSelectionDepthRenderBuffer != 0)
	{
		glDeleteRenderbuffers(1, &m_iFaceSelectionDepthRenderBuffer);
		m_iFaceSelectionDepthRenderBuffer = 0;
	}	

	/*
	* VBO
	*/
	for (size_t iDrawMetaData = 0; iDrawMetaData < m_veCSTEPDrawMetaData.size(); iDrawMetaData++)
	{
		delete m_veCSTEPDrawMetaData[iDrawMetaData];
	}
	m_veCSTEPDrawMetaData.clear();

	/*
	* IBO
	*/
	for (size_t iIBO = 0; iIBO < m_vecIBOs.size(); iIBO++)
	{
		glDeleteBuffers(1, &(m_vecIBOs[iIBO]));
	}
	m_vecIBOs.clear();

	if (m_hFont != NULL)
	{
		BOOL bResult = DeleteObject(m_hFont);
		VERIFY(bResult);

		m_hFont = NULL;
	}

	delete m_pSelectedInstanceMaterial;
	delete m_pPointedInstanceMaterial;

#ifdef _USE_SHADERS
	m_pOGLContext->MakeCurrent();

#ifdef _USE_SHADERS
	m_pProgram->DetachShader(m_pVertSh);
	m_pProgram->DetachShader(m_pFragSh);

	delete m_pProgram;
	m_pProgram = NULL;

	delete m_pVertSh;
	m_pVertSh = NULL;
	delete m_pFragSh;
	m_pFragSh = NULL;
#endif // _USE_SHADERS

	//delete m_pProgram;
#endif // _USE_SHADERS

	if (m_pOGLContext != NULL)
	{
		delete m_pOGLContext;
		m_pOGLContext = NULL;
	}

	ReleaseBuffers();
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
void COpenGLSTEPView::ShowBoundingBoxes(BOOL bShow)
{
	m_bShowBoundingBoxes = bShow;

#ifdef _LINUX
    m_pWnd->Refresh(false);
#else
    m_pWnd->RedrawWindow();
#endif // _LINUX
}

// ------------------------------------------------------------------------------------------------
BOOL COpenGLSTEPView::AreBoundingBoxesShown() const
{
	return m_bShowBoundingBoxes;
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::ShowNormalVectors(BOOL bShow)
{
	m_bShowNormalVectors = bShow;

#ifdef _LINUX
    m_pWnd->Refresh(false);
#else
    m_pWnd->RedrawWindow();
#endif // _LINUX
}

// ------------------------------------------------------------------------------------------------
BOOL COpenGLSTEPView::AreNormalVectorsShown() const
{
	return m_bShowNormalVectors;
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::ShowTangentVectors(BOOL bShow)
{
	m_bShowTangenVectors = bShow;

#ifdef _LINUX
    m_pWnd->Refresh(false);
#else
    m_pWnd->RedrawWindow();
#endif // _LINUX
}

// ------------------------------------------------------------------------------------------------
BOOL COpenGLSTEPView::AreTangentVectorsShown() const
{
	return m_bShowTangenVectors;
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::ShowBiNormalVectors(BOOL bShow)
{
	m_bShowBiNormalVectors = bShow;

#ifdef _LINUX
    m_pWnd->Refresh(false);
#else
    m_pWnd->RedrawWindow();
#endif // _LINUX
}

// ------------------------------------------------------------------------------------------------
BOOL COpenGLSTEPView::AreBiNormalVectorsShown() const
{
	return m_bShowBiNormalVectors;
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::ScaleVectors(BOOL bShow)
{
	m_bScaleVectors = bShow;

#ifdef _LINUX
    m_pWnd->Refresh(false);
#else
    m_pWnd->RedrawWindow();
#endif // _LINUX
}

// ------------------------------------------------------------------------------------------------
BOOL COpenGLSTEPView::AreVectorsScaled() const
{
	return m_bScaleVectors;
}

// ------------------------------------------------------------------------------------------------
GLfloat * COpenGLSTEPView::GetLightModelAmbient() const
{
	return (GLfloat *)m_arLightModelAmbient;
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::SetLightModelAmbient(GLfloat fR, GLfloat fG, GLfloat fB, GLfloat fW/* = 1.f*/)
{
	m_arLightModelAmbient[0] = fR;
	m_arLightModelAmbient[1] = fG;
	m_arLightModelAmbient[2] = fB;
	m_arLightModelAmbient[3] = fW;

#ifdef _LINUX
    m_pWnd->Refresh(false);
#else
    m_pWnd->RedrawWindow();
#endif // _LINUX
}

// ------------------------------------------------------------------------------------------------
bool COpenGLSTEPView::GetLightModelLocalViewer() const
{
	return m_bLightModelLocalViewer;
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::SetLightModelLocalViewer(bool bLocalViewer)
{
	m_bLightModelLocalViewer = bLocalViewer;

#ifdef _LINUX
    m_pWnd->Refresh(false);
#else
    m_pWnd->RedrawWindow();
#endif // _LINUX
}

// ------------------------------------------------------------------------------------------------
bool COpenGLSTEPView::GetLightModel2Sided() const
{
	return m_bLightModel2Sided;
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::SetLightModel2Sided(bool b2Sided)
{
	m_bLightModel2Sided = b2Sided;

#ifdef _LINUX
    m_pWnd->Refresh(false);
#else
    m_pWnd->RedrawWindow();
#endif // _LINUX
}

// ------------------------------------------------------------------------------------------------
const vector<COpenGLLight> & COpenGLSTEPView::GetOGLLights() const
{
	return m_vecOGLLights;
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::SetOGLLight(int iLight, const COpenGLLight & light)
{
	ASSERT((iLight >= 0)& (iLight < (int)m_vecOGLLights.size()));

	m_vecOGLLights[iLight] = light;

#ifdef _LINUX
    m_pWnd->Refresh(false);
#else
    m_pWnd->RedrawWindow();
#endif // _LINUX
}

bool gluInvertMatrix(const double m[16], double invOut[16])
{
	double inv[16], det;
	int i;

	inv[0] = m[5] * m[10] * m[15] -
		m[5] * m[11] * m[14] -
		m[9] * m[6] * m[15] +
		m[9] * m[7] * m[14] +
		m[13] * m[6] * m[11] -
		m[13] * m[7] * m[10];

	inv[4] = -m[4] * m[10] * m[15] +
		m[4] * m[11] * m[14] +
		m[8] * m[6] * m[15] -
		m[8] * m[7] * m[14] -
		m[12] * m[6] * m[11] +
		m[12] * m[7] * m[10];

	inv[8] = m[4] * m[9] * m[15] -
		m[4] * m[11] * m[13] -
		m[8] * m[5] * m[15] +
		m[8] * m[7] * m[13] +
		m[12] * m[5] * m[11] -
		m[12] * m[7] * m[9];

	inv[12] = -m[4] * m[9] * m[14] +
		m[4] * m[10] * m[13] +
		m[8] * m[5] * m[14] -
		m[8] * m[6] * m[13] -
		m[12] * m[5] * m[10] +
		m[12] * m[6] * m[9];

	inv[1] = -m[1] * m[10] * m[15] +
		m[1] * m[11] * m[14] +
		m[9] * m[2] * m[15] -
		m[9] * m[3] * m[14] -
		m[13] * m[2] * m[11] +
		m[13] * m[3] * m[10];

	inv[5] = m[0] * m[10] * m[15] -
		m[0] * m[11] * m[14] -
		m[8] * m[2] * m[15] +
		m[8] * m[3] * m[14] +
		m[12] * m[2] * m[11] -
		m[12] * m[3] * m[10];

	inv[9] = -m[0] * m[9] * m[15] +
		m[0] * m[11] * m[13] +
		m[8] * m[1] * m[15] -
		m[8] * m[3] * m[13] -
		m[12] * m[1] * m[11] +
		m[12] * m[3] * m[9];

	inv[13] = m[0] * m[9] * m[14] -
		m[0] * m[10] * m[13] -
		m[8] * m[1] * m[14] +
		m[8] * m[2] * m[13] +
		m[12] * m[1] * m[10] -
		m[12] * m[2] * m[9];

	inv[2] = m[1] * m[6] * m[15] -
		m[1] * m[7] * m[14] -
		m[5] * m[2] * m[15] +
		m[5] * m[3] * m[14] +
		m[13] * m[2] * m[7] -
		m[13] * m[3] * m[6];

	inv[6] = -m[0] * m[6] * m[15] +
		m[0] * m[7] * m[14] +
		m[4] * m[2] * m[15] -
		m[4] * m[3] * m[14] -
		m[12] * m[2] * m[7] +
		m[12] * m[3] * m[6];

	inv[10] = m[0] * m[5] * m[15] -
		m[0] * m[7] * m[13] -
		m[4] * m[1] * m[15] +
		m[4] * m[3] * m[13] +
		m[12] * m[1] * m[7] -
		m[12] * m[3] * m[5];

	inv[14] = -m[0] * m[5] * m[14] +
		m[0] * m[6] * m[13] +
		m[4] * m[1] * m[14] -
		m[4] * m[2] * m[13] -
		m[12] * m[1] * m[6] +
		m[12] * m[2] * m[5];

	inv[3] = -m[1] * m[6] * m[11] +
		m[1] * m[7] * m[10] +
		m[5] * m[2] * m[11] -
		m[5] * m[3] * m[10] -
		m[9] * m[2] * m[7] +
		m[9] * m[3] * m[6];

	inv[7] = m[0] * m[6] * m[11] -
		m[0] * m[7] * m[10] -
		m[4] * m[2] * m[11] +
		m[4] * m[3] * m[10] +
		m[8] * m[2] * m[7] -
		m[8] * m[3] * m[6];

	inv[11] = -m[0] * m[5] * m[11] +
		m[0] * m[7] * m[9] +
		m[4] * m[1] * m[11] -
		m[4] * m[3] * m[9] -
		m[8] * m[1] * m[7] +
		m[8] * m[3] * m[5];

	inv[15] = m[0] * m[5] * m[10] -
		m[0] * m[6] * m[9] -
		m[4] * m[1] * m[10] +
		m[4] * m[2] * m[9] +
		m[8] * m[1] * m[6] -
		m[8] * m[2] * m[5];

	det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

	if (det == 0)
		return false;

	det = 1.0 / det;

	for (i = 0; i < 16; i++)
		invOut[i] = inv[i] * det;

	return true;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void COpenGLSTEPView::Load()
{
#ifdef _LINUX
	m_pOGLContext->SetCurrent(*m_pWnd);
#else
	BOOL bResult = m_pOGLContext->MakeCurrent();
	VERIFY(bResult);
#endif // _LINUX

	ReleaseBuffers();

	m_mapInstancesSelectionColors.clear();
	m_pPointedInstance = NULL;
	m_pSelectedInstance = NULL;
	m_arSelectedPoint[0] = -FLT_MAX;
	m_arSelectedPoint[1] = -FLT_MAX;
	m_arSelectedPoint[2] = -FLT_MAX;
	//m_mapFacesSelectionColors.clear();
	m_iPointedFace = -1;

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

	/*
	* VBOs
	*/
	for (size_t iDrawMetaData = 0; iDrawMetaData < m_veCSTEPDrawMetaData.size(); iDrawMetaData++)
	{
		delete m_veCSTEPDrawMetaData[iDrawMetaData];
	}
	m_veCSTEPDrawMetaData.clear();

	/*
	* IBO
	*/
	for (size_t iIBO = 0; iIBO < m_vecIBOs.size(); iIBO++)
	{
		glDeleteBuffers(1, &(m_vecIBOs[iIBO]));
	}
	m_vecIBOs.clear();

	// Limits
	GLsizei GEOMETRY_VERTICES_MAX_COUNT = COpenGL::GetGeometryVerticesCountLimit();
	GLsizei INDICES_MAX_COUNT = COpenGL::GetIndicesCountLimit();

	const map<int_t, CProductDefinition*>& mapProductDefinitions = pModel->getProductDefinitions();

	// VBO
	GLuint iVerticesCount = 0;
	vector<CProductDefinition*> vecProductDefinitionsGroup;

	// IBO - Materials
	GLuint iMaterialsIndicesCount = 0;
	vector<CSTEPGeometryWithMaterial*> vecSTEPMaterialsGroup;

	// IBO - Lines
	GLuint iLinesIndicesCount = 0;
	vector<CLinesCohort*> vecLinesCohorts;

	// IBO - Points
	GLuint iPointsIndicesCount = 0;
	vector<CPointsCohort*> vecPointsCohorts;

	// IBO - Conceptual Faces
	GLuint iConceptualFacesIndicesCount = 0;
	vector<CWireframesCohort*> vecConceptualFacesCohorts;

	map<int_t, CProductDefinition*>::const_iterator itProductDefinitions = mapProductDefinitions.begin();
	for (; itProductDefinitions != mapProductDefinitions.end(); itProductDefinitions++)
	{
		CProductDefinition* pProductDefinition = itProductDefinitions->second;
		if (pProductDefinition->getVerticesCount() == 0)
		{
			continue;
		}

		/******************************************************************************************
		* Geometry
		*/

		/**
		* VBO - Conceptual faces, wireframes, etc.
		*/
		if (((int_t)iVerticesCount + pProductDefinition->getVerticesCount()) > (int_t)GEOMETRY_VERTICES_MAX_COUNT)
		{
			ASSERT(!vecProductDefinitionsGroup.empty());

			int_t iCohortVerticesCount = 0;
			float* pVertices = GetVertices(vecProductDefinitionsGroup, iCohortVerticesCount);
			if ((iCohortVerticesCount == 0) || (pVertices == nullptr))
			{
				ASSERT(0);

				return;
			}

			ASSERT(iCohortVerticesCount == iVerticesCount);

			GLuint iVBO = 0;
			glGenBuffers(1, &iVBO);

			ASSERT(iVBO != 0);

			glBindBuffer(GL_ARRAY_BUFFER, iVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * iVerticesCount * GEOMETRY_VBO_VERTEX_LENGTH, pVertices, GL_STATIC_DRAW);

			TRACE(L"\nVBO VERTICES: %d", iVerticesCount);

			/*
			* Store VBO/offset
			*/
			GLsizei iVBOOffset = 0;
			for (size_t iProductDefinition = 0; iProductDefinition < vecProductDefinitionsGroup.size(); iProductDefinition++)
			{
				vecProductDefinitionsGroup[iProductDefinition]->VBO() = iVBO;
				vecProductDefinitionsGroup[iProductDefinition]->VBOOffset() = iVBOOffset;

				iVBOOffset += (GLsizei)vecProductDefinitionsGroup[iProductDefinition]->getVerticesCount();
			} // for (size_t iProductDefinition = ...

			delete[] pVertices;

			glBindBuffer(GL_ARRAY_BUFFER, 0);

			COpenGL::Check4Errors();

			CSTEPDrawMetaData* pDrawMetaData = new CSTEPDrawMetaData(mdtGeometry);
			pDrawMetaData->AddGroup(iVBO, vecProductDefinitionsGroup);

			m_veCSTEPDrawMetaData.push_back(pDrawMetaData);

			iVerticesCount = 0;
			vecProductDefinitionsGroup.clear();
		} // if (((int_t)iVerticesCount + pProductDefinition->getVerticesCount()) > ...		

		/*
		* IBO - Materials
		*/
		for (size_t iMaterial = 0; iMaterial < pProductDefinition->conceptualFacesMaterials().size(); iMaterial++)
		{
			if ((int_t)(iMaterialsIndicesCount + pProductDefinition->conceptualFacesMaterials()[iMaterial]->getIndicesCount()) > (int_t)INDICES_MAX_COUNT)
			{
				ASSERT(!vecSTEPMaterialsGroup.empty());

				GLuint iIBO = 0;
				glGenBuffers(1, &iIBO);

				ASSERT(iIBO != 0);

				m_vecIBOs.push_back(iIBO);

				int_t iGroupIndicesCount = 0;
				unsigned int* pIndices = GetMaterialsIndices(vecSTEPMaterialsGroup, iGroupIndicesCount);
				if ((iGroupIndicesCount == 0) || (pIndices == nullptr))
				{
					ASSERT(0);

					return;
				}

				ASSERT(iMaterialsIndicesCount == iGroupIndicesCount);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iIBO);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * iMaterialsIndicesCount, pIndices, GL_STATIC_DRAW);

				delete[] pIndices;

				/*
				* Store IBO/offset
				*/
				GLsizei iIBOOffset = 0;
				for (size_t iMaterial2 = 0; iMaterial2 < vecSTEPMaterialsGroup.size(); iMaterial2++)
				{
					vecSTEPMaterialsGroup[iMaterial2]->IBO() = iIBO;
					vecSTEPMaterialsGroup[iMaterial2]->IBOOffset() = iIBOOffset;

					iIBOOffset += (GLsizei)vecSTEPMaterialsGroup[iMaterial2]->getIndicesCount();
				}

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

				COpenGL::Check4Errors();

				iMaterialsIndicesCount = 0;
				vecSTEPMaterialsGroup.clear();
			} // if ((int_t)(iMaterialsIndicesCount + ...	

			iMaterialsIndicesCount += (GLsizei)pProductDefinition->conceptualFacesMaterials()[iMaterial]->getIndicesCount();
			vecSTEPMaterialsGroup.push_back(pProductDefinition->conceptualFacesMaterials()[iMaterial]);
		} // for (size_t iMaterial = ...	

		/*
		* IBO - Lines
		*/
		for (size_t iLinesCohort = 0; iLinesCohort < pProductDefinition->linesCohorts().size(); iLinesCohort++)
		{
			if ((int_t)(iLinesIndicesCount + pProductDefinition->linesCohorts()[iLinesCohort]->getIndicesCount()) > (int_t)INDICES_MAX_COUNT)
			{
				ASSERT(!vecLinesCohorts.empty());

				GLuint iIBO = 0;
				glGenBuffers(1, &iIBO);

				ASSERT(iIBO != 0);

				m_vecIBOs.push_back(iIBO);

				int_t iCohortIndicesCount = 0;
				unsigned int* pIndices = GetLinesCohortsIndices(vecLinesCohorts, iCohortIndicesCount);
				if ((iCohortIndicesCount == 0) || (pIndices == NULL))
				{
					ASSERT(0);

					return;
				}

				ASSERT(iLinesIndicesCount == iCohortIndicesCount);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iIBO);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * iLinesIndicesCount, pIndices, GL_STATIC_DRAW);

				delete[] pIndices;

				/*
				* Store IBO/offset
				*/
				GLsizei iIBOOffset = 0;
				for (size_t iLinesCohort2 = 0; iLinesCohort2 < vecLinesCohorts.size(); iLinesCohort2++)
				{
					vecLinesCohorts[iLinesCohort2]->IBO() = iIBO;
					vecLinesCohorts[iLinesCohort2]->IBOOffset() = iIBOOffset;

					iIBOOffset += (GLsizei)vecLinesCohorts[iLinesCohort2]->getIndicesCount();
				} // for (size_t iLinesCohort2 = ...				

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

				COpenGL::Check4Errors();

				iLinesIndicesCount = 0;
				vecLinesCohorts.clear();
			} // if ((int_t)(iLinesIndicesCount + ...	

			iLinesIndicesCount += (GLsizei)pProductDefinition->linesCohorts()[iLinesCohort]->getIndicesCount();
			vecLinesCohorts.push_back(pProductDefinition->linesCohorts()[iLinesCohort]);
		} // for (size_t iLinesCohort = ...	

		/*
		* IBO - Points
		*/
		for (size_t iPointsCohort = 0; iPointsCohort < pProductDefinition->pointsCohorts().size(); iPointsCohort++)
		{
			if ((int_t)(iPointsIndicesCount + pProductDefinition->pointsCohorts()[iPointsCohort]->getIndicesCount()) > (int_t)INDICES_MAX_COUNT)
			{
				ASSERT(!vecPointsCohorts.empty());

				GLuint iIBO = 0;
				glGenBuffers(1, &iIBO);

				ASSERT(iIBO != 0);

				m_vecIBOs.push_back(iIBO);

				int_t iCohortIndicesCount = 0;
				unsigned int* pIndices = GetPointsCohortsIndices(vecPointsCohorts, iCohortIndicesCount);
				if ((iCohortIndicesCount == 0) || (pIndices == NULL))
				{
					ASSERT(0);

					return;
				}

				ASSERT(iPointsIndicesCount == iCohortIndicesCount);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iIBO);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * iPointsIndicesCount, pIndices, GL_STATIC_DRAW);

				delete[] pIndices;

				/*
				* Store IBO/offset
				*/
				GLsizei iIBOOffset = 0;
				for (size_t iPointsCohort2 = 0; iPointsCohort2 < vecPointsCohorts.size(); iPointsCohort2++)
				{
					vecPointsCohorts[iPointsCohort2]->IBO() = iIBO;
					vecPointsCohorts[iPointsCohort2]->IBOOffset() = iIBOOffset;

					iIBOOffset += (GLsizei)vecPointsCohorts[iPointsCohort2]->getIndicesCount();
				} // for (size_t iPointsCohort2 = ...				

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

				COpenGL::Check4Errors();

				iPointsIndicesCount = 0;
				vecPointsCohorts.clear();
			} // if ((int_t)(iPointsIndicesCount + ...	

			iPointsIndicesCount += (GLsizei)pProductDefinition->pointsCohorts()[iPointsCohort]->getIndicesCount();
			vecPointsCohorts.push_back(pProductDefinition->pointsCohorts()[iPointsCohort]);
		} // for (size_t iPointsCohort = ...	

		/*
		* IBO - Conceptual Faces
		*/
		for (size_t iConceptualFacesCohort = 0; iConceptualFacesCohort < pProductDefinition->conceptualFacesCohorts().size(); iConceptualFacesCohort++)
		{
			if ((int_t)(iConceptualFacesIndicesCount + pProductDefinition->conceptualFacesCohorts()[iConceptualFacesCohort]->getIndicesCount()) > (int_t)INDICES_MAX_COUNT)
			{
				ASSERT(!vecConceptualFacesCohorts.empty());

				GLuint iIBO = 0;
				glGenBuffers(1, &iIBO);

				ASSERT(iIBO != 0);

				m_vecIBOs.push_back(iIBO);

				int_t iCohortIndicesCount = 0;
				unsigned int* pIndices = GetWireframesCohortsIndices(vecConceptualFacesCohorts, iCohortIndicesCount);
				if ((iCohortIndicesCount == 0) || (pIndices == NULL))
				{
					ASSERT(0);

					return;
				}

				ASSERT(iConceptualFacesIndicesCount == iCohortIndicesCount);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iIBO);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * iConceptualFacesIndicesCount, pIndices, GL_STATIC_DRAW);

				delete[] pIndices;

				/*
				* Store IBO/offset
				*/
				GLsizei iIBOOffset = 0;
				for (size_t iConceptualFacesCohort2 = 0; iConceptualFacesCohort2 < vecConceptualFacesCohorts.size(); iConceptualFacesCohort2++)
				{
					vecConceptualFacesCohorts[iConceptualFacesCohort2]->IBO() = iIBO;
					vecConceptualFacesCohorts[iConceptualFacesCohort2]->IBOOffset() = iIBOOffset;

					iIBOOffset += (GLsizei)vecConceptualFacesCohorts[iConceptualFacesCohort2]->getIndicesCount();
				} // for (size_t iConceptualFacesCohort2 = ...				

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

				COpenGL::Check4Errors();

				iConceptualFacesIndicesCount = 0;
				vecConceptualFacesCohorts.clear();
			} // if ((int_t)(iConceptualFacesIndicesCount + ...	

			iConceptualFacesIndicesCount += (GLsizei)pProductDefinition->conceptualFacesCohorts()[iConceptualFacesCohort]->getIndicesCount();
			vecConceptualFacesCohorts.push_back(pProductDefinition->conceptualFacesCohorts()[iConceptualFacesCohort]);
		} // for (size_t iConceptualFacesCohort = ...			

		iVerticesCount += (GLsizei)pProductDefinition->getVerticesCount();
		vecProductDefinitionsGroup.push_back(pProductDefinition);
	} // for (; itProductDefinitions != ...

	/******************************************************************************************
	* Geometry
	*/

	/*
	* VBO - Conceptual faces, wireframes, etc.
	*/
	if (iVerticesCount > 0)
	{
		ASSERT(!vecProductDefinitionsGroup.empty());

		int_t iCohortVerticesCount = 0;
		float* pVertices = GetVertices(vecProductDefinitionsGroup, iCohortVerticesCount);
		if ((iCohortVerticesCount == 0) || (pVertices == nullptr))
		{
			ASSERT(0);

			return;
		}

		ASSERT(iCohortVerticesCount == iVerticesCount);

		GLuint iVBO = 0;
		glGenBuffers(1, &iVBO);

		ASSERT(iVBO != 0);

		glBindBuffer(GL_ARRAY_BUFFER, iVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * iVerticesCount * GEOMETRY_VBO_VERTEX_LENGTH, pVertices, GL_STATIC_DRAW);

		TRACE(L"\nVBO VERTICES: %d", iVerticesCount);

		/*
		* Store VBO/offset
		*/
		GLsizei iVBOOffset = 0;
		for (size_t iProductDefinition = 0; iProductDefinition < vecProductDefinitionsGroup.size(); iProductDefinition++)
		{
			vecProductDefinitionsGroup[iProductDefinition]->VBO() = iVBO;
			vecProductDefinitionsGroup[iProductDefinition]->VBOOffset() = iVBOOffset;

			iVBOOffset += (GLsizei)vecProductDefinitionsGroup[iProductDefinition]->getVerticesCount();
		} // for (size_t iProductDefinition = ...

		delete[] pVertices;

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		COpenGL::Check4Errors();

		CSTEPDrawMetaData* pDrawMetaData = new CSTEPDrawMetaData(mdtGeometry);
		pDrawMetaData->AddGroup(iVBO, vecProductDefinitionsGroup);

		m_veCSTEPDrawMetaData.push_back(pDrawMetaData);

		iVerticesCount = 0;
		vecProductDefinitionsGroup.clear();
	} // if (iVerticesCount > 0)	

	/*
	* IBO - Materials
	*/
	if (iMaterialsIndicesCount > 0)
	{
		ASSERT(!vecSTEPMaterialsGroup.empty());

		GLuint iIBO = 0;
		glGenBuffers(1, &iIBO);

		ASSERT(iIBO != 0);

		m_vecIBOs.push_back(iIBO);

		int_t iGroupIndicesCount = 0;
		unsigned int* pIndices = GetMaterialsIndices(vecSTEPMaterialsGroup, iGroupIndicesCount);
		if ((iGroupIndicesCount == 0) || (pIndices == nullptr))
		{
			ASSERT(0);

			return;
		}

		ASSERT(iMaterialsIndicesCount == iGroupIndicesCount);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iIBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * iMaterialsIndicesCount, pIndices, GL_STATIC_DRAW);

		delete[] pIndices;

		/*
		* Store IBO/offset
		*/
		GLsizei iIBOOffset = 0;
		for (size_t iMaterial2 = 0; iMaterial2 < vecSTEPMaterialsGroup.size(); iMaterial2++)
		{
			vecSTEPMaterialsGroup[iMaterial2]->IBO() = iIBO;
			vecSTEPMaterialsGroup[iMaterial2]->IBOOffset() = iIBOOffset;

			iIBOOffset += (GLsizei)vecSTEPMaterialsGroup[iMaterial2]->getIndicesCount();
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		COpenGL::Check4Errors();

		iMaterialsIndicesCount = 0;
		vecSTEPMaterialsGroup.clear();
	} // if (iMaterialsIndicesCount > 0)

	/*
	* IBO - Lines
	*/
	if (iLinesIndicesCount > 0)
	{
		ASSERT(!vecLinesCohorts.empty());

		GLuint iIBO = 0;
		glGenBuffers(1, &iIBO);

		ASSERT(iIBO != 0);

		m_vecIBOs.push_back(iIBO);

		int_t iCohortIndicesCount = 0;
		unsigned int* pIndices = GetLinesCohortsIndices(vecLinesCohorts, iCohortIndicesCount);
		if ((iCohortIndicesCount == 0) || (pIndices == NULL))
		{
			ASSERT(0);

			return;
		}

		ASSERT(iLinesIndicesCount == iCohortIndicesCount);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iIBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * iLinesIndicesCount, pIndices, GL_STATIC_DRAW);

		delete[] pIndices;

		/*
		* Store IBO/offset
		*/
		GLsizei iIBOOffset = 0;
		for (size_t iLinesCohort2 = 0; iLinesCohort2 < vecLinesCohorts.size(); iLinesCohort2++)
		{
			vecLinesCohorts[iLinesCohort2]->IBO() = iIBO;
			vecLinesCohorts[iLinesCohort2]->IBOOffset() = iIBOOffset;

			iIBOOffset += (GLsizei)vecLinesCohorts[iLinesCohort2]->getIndicesCount();
		} // for (size_t iLinesCohort2 = ...				

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		COpenGL::Check4Errors();

		iLinesIndicesCount = 0;
		vecLinesCohorts.clear();
	} // if (iLinesIndicesCount > 0)		

	if (iPointsIndicesCount > 0)
	{
		ASSERT(!vecPointsCohorts.empty());

		GLuint iIBO = 0;
		glGenBuffers(1, &iIBO);

		ASSERT(iIBO != 0);

		m_vecIBOs.push_back(iIBO);

		int_t iCohortIndicesCount = 0;
		unsigned int* pIndices = GetPointsCohortsIndices(vecPointsCohorts, iCohortIndicesCount);
		if ((iCohortIndicesCount == 0) || (pIndices == NULL))
		{
			ASSERT(0);

			return;
		}

		ASSERT(iPointsIndicesCount == iCohortIndicesCount);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iIBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * iPointsIndicesCount, pIndices, GL_STATIC_DRAW);

		delete[] pIndices;

		/*
		* Store IBO/offset
		*/
		GLsizei iIBOOffset = 0;
		for (size_t iPointsCohort2 = 0; iPointsCohort2 < vecPointsCohorts.size(); iPointsCohort2++)
		{
			vecPointsCohorts[iPointsCohort2]->IBO() = iIBO;
			vecPointsCohorts[iPointsCohort2]->IBOOffset() = iIBOOffset;

			iIBOOffset += (GLsizei)vecPointsCohorts[iPointsCohort2]->getIndicesCount();
		} // for (size_t iPointsCohort2 = ...				

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		COpenGL::Check4Errors();

		iPointsIndicesCount = 0;
		vecPointsCohorts.clear();
	} // if (iPointsIndicesCount > 0)

	/*
	* IBO - Conceptual Faces
	*/
	if (iConceptualFacesIndicesCount > 0)
	{
		ASSERT(!vecConceptualFacesCohorts.empty());

		GLuint iIBO = 0;
		glGenBuffers(1, &iIBO);

		ASSERT(iIBO != 0);

		m_vecIBOs.push_back(iIBO);

		int_t iCohortIndicesCount = 0;
		unsigned int* pIndices = GetWireframesCohortsIndices(vecConceptualFacesCohorts, iCohortIndicesCount);
		if ((iCohortIndicesCount == 0) || (pIndices == NULL))
		{
			ASSERT(0);

			return;
		}

		ASSERT(iConceptualFacesIndicesCount == iCohortIndicesCount);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iIBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * iConceptualFacesIndicesCount, pIndices, GL_STATIC_DRAW);

		delete[] pIndices;

		/*
		* Store IBO/offset
		*/
		GLsizei iIBOOffset = 0;
		for (size_t iConceptualFacesCohort2 = 0; iConceptualFacesCohort2 < vecConceptualFacesCohorts.size(); iConceptualFacesCohort2++)
		{
			vecConceptualFacesCohorts[iConceptualFacesCohort2]->IBO() = iIBO;
			vecConceptualFacesCohorts[iConceptualFacesCohort2]->IBOOffset() = iIBOOffset;

			iIBOOffset += (GLsizei)vecConceptualFacesCohorts[iConceptualFacesCohort2]->getIndicesCount();
		} // for (size_t iConceptualFacesCohort2 = ...				

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		COpenGL::Check4Errors();

		iConceptualFacesIndicesCount = 0;
		vecConceptualFacesCohorts.clear();
	} // if (iConceptualFacesIndicesCount > 0)

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
	CRect rcClient;
	m_pWnd->GetClientRect(&rcClient);

	iWidth = rcClient.Width();
	iHeight = rcClient.Height();
#endif // _LINUX

	if ((iWidth < 20) || (iHeight < 20))
	{
		return;
	}	

#ifdef _USE_SHADERS
	BOOL bResult = m_pOGLContext->MakeCurrent();
	VERIFY(bResult);

#ifdef _ENABLE_OPENGL_DEBUG
	m_pOGLContext->EnableDebug();
#endif

	m_pProgram->Use();

	glViewport(0, 0, iWidth, iHeight);

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set up the parameters
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glShadeModel(GL_SMOOTH);

	/*
	* Light
	*/
	glProgramUniform3f(
		m_pProgram->GetID(),
		m_pProgram->getPointLightingLocation(), 
		0.f, 
		0.f, 
		10000.f);

	/*
	* Shininess
	*/
	glProgramUniform1f(
		m_pProgram->GetID(), 
		m_pProgram->getMaterialShininess(), 
		30.f);

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

	glm::mat4 projectionMatrix = glm::frustum<GLdouble>(-fW, fW, -fH, fH, zNear, zFar);

	glProgramUniformMatrix4fv(
		m_pProgram->GetID(), 
		m_pProgram->getPMatrix(), 
		1, 
		false, 
		value_ptr(projectionMatrix));

	/*
	* Model-View Matrix
	*/
	m_modelViewMatrix = glm::identity<glm::mat4>();
	m_modelViewMatrix = glm::translate(m_modelViewMatrix, glm::vec3(m_fXTranslation, m_fYTranslation, m_fZTranslation));

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

	m_modelViewMatrix = glm::translate(m_modelViewMatrix, glm::vec3(-fXTranslation, -fYTranslation, -fZTranslation));

	m_modelViewMatrix = glm::rotate(m_modelViewMatrix, m_fXAngle, glm::vec3(1.0f, 0.0f, 0.0f));
	m_modelViewMatrix = glm::rotate(m_modelViewMatrix, m_fYAngle, glm::vec3(0.0f, 1.0f, 0.0f));

	m_modelViewMatrix = glm::translate(m_modelViewMatrix, glm::vec3(fXTranslation, fYTranslation, fZTranslation));

	glProgramUniformMatrix4fv(
		m_pProgram->GetID(), 
		m_pProgram->getMVMatrix(), 
		1, 
		false, 
		glm::value_ptr(m_modelViewMatrix));

	/*
	* Normal Matrix
	*/
	glm::mat4 normalMatrix = m_modelViewMatrix;
	normalMatrix = glm::inverse(normalMatrix);
	normalMatrix = glm::transpose(normalMatrix);

	glProgramUniformMatrix4fv(
		m_pProgram->GetID(), 
		m_pProgram->getNMatrix(), 
		1, 
		false, 
		value_ptr(normalMatrix));
	
	/*
	* Binn-Phong Model
	*/
	//glBindProgramPipeline(m_pProgram->getPipeline());

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
	* Scene
	*/
	DrawCoordinateSystem();

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
#else
	/*
	* Convert the selected point in Open GL coordinates
	*/
	//if (m_ptSelectedPoint != CPoint(-1, -1))
	//{
	//	GetOGLPos(m_ptSelectedPoint.x, m_ptSelectedPoint.y, -FLT_MAX, m_arSelectedPoint[0], m_arSelectedPoint[1], m_arSelectedPoint[2]);

	//	/*
	//	* Project/unproject the camera
	//	*/
	//	GLdouble dOutX, dOutY, dOutZ;
	//	OGLProject(m_fXTranslation, m_fYTranslation, m_fZTranslation, dOutX, dOutY, dOutZ);

	//	GetOGLPos((int)dOutX, (int)dOutY, (float)dOutZ, m_arCamera[0], m_arCamera[1], m_arCamera[2]);

	//	m_ptSelectedPoint = CPoint(-1, -1);
	//}	

	glEnable(GL_COLOR_MATERIAL);

	glEnable(GL_FRAMEBUFFER_SRGB);

	glShadeModel(GL_SMOOTH);

	glEnable(GL_LIGHTING);

	/*
	* Light model, http://www.glprogramming.com/red/chapter05.html
	*/
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, m_arLightModelAmbient);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, m_bLightModelLocalViewer ? GL_TRUE : GL_FALSE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, m_bLightModel2Sided ? GL_TRUE : GL_FALSE);

	/*
	* Lights
	*/
	for (size_t iLight = 0; iLight < m_vecOGLLights.size(); iLight++)
	{
		if (!m_vecOGLLights[iLight].isEnabled())
		{
			glDisable(m_vecOGLLights[iLight].getLight());

			continue;
		}

		glEnable(m_vecOGLLights[iLight].getLight());
		glLightfv(m_vecOGLLights[iLight].getLight(), GL_AMBIENT, m_vecOGLLights[iLight].getAmbient());
		glLightfv(m_vecOGLLights[iLight].getLight(), GL_DIFFUSE, m_vecOGLLights[iLight].getDiffuse());
		glLightfv(m_vecOGLLights[iLight].getLight(), GL_SPECULAR, m_vecOGLLights[iLight].getSpecular());
		glLightfv(m_vecOGLLights[iLight].getLight(), GL_POSITION, m_vecOGLLights[iLight].getPosition());
	}

	// Shininess between 0 and 128
	float fShininess = 64.f;
	glMaterialfv(GL_FRONT, GL_SHININESS, &fShininess);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

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

	glFrustum(-fW, fW, -fH, fH, zNear, zFar);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(m_fXTranslation, m_fYTranslation, m_fZTranslation);

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

	glTranslatef(-fXTranslation, -fYTranslation, -fZTranslation);

	glRotatef(m_fXAngle, 1.0f, 0.0f, 0.0f);
	glRotatef(m_fYAngle, 0.0f, 1.0f, 0.0f);

	glTranslatef(fXTranslation, fYTranslation, fZTranslation);

	/*
	Non-transparent faces
	*/
	DrawFaces(false);

	/*
	Transparent faces
	*/
	DrawFaces(true);

	/*
	Pointed instance
	*/
	//DrawPointedInstance();

	/*
	Pointed face
	*/
	//DrawPointedFace();

	/*
	Faces polygons
	*/
	//DrawFacesPolygons();

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
	Bounding boxes
	*/
	//DrawBoundingBoxes();

	/*
	Normal vectors
	*/
	//DrawNormalVectors();

	/*
	Tangent vectors
	*/
	//DrawTangentVectors();

	/*
	Bi-Normal vectors
	*/
	//DrawBiNormalVectors();

	/*
	* Selected point plane/view plane
	*/
	/*if ((m_arSelectedPoint[0] != -FLT_MAX) && (m_arSelectedPoint[1] != -FLT_MAX) && (m_arSelectedPoint[2] != -FLT_MAX) &&
		(m_arCamera[0] != -FLT_MAX) && (m_arCamera[1] != -FLT_MAX) && (m_arCamera[2] != -FLT_MAX))
	{
		glDisable(GL_LIGHTING);

		glLineWidth(m_fLineWidth);
		glColor3f(.0f, .0f, .0f);

		glBegin(GL_LINES);

		const float fStep = (fXmax - fXmin) / 10.f;

		for (int i = 0; i < 5; i++)
		{
			glVertex3f(m_arSelectedPoint[0] + (i * fStep), m_arSelectedPoint[1], m_arSelectedPoint[2]);
			glVertex3f(m_arCamera[0] + (i * fStep), m_arCamera[1], -m_arCamera[2]);
		}

		for (int i = 0; i < 5; i++)
		{
			glVertex3f(m_arSelectedPoint[0] - (i * fStep), m_arSelectedPoint[1], m_arSelectedPoint[2]);
			glVertex3f(m_arCamera[0] - (i * fStep), m_arCamera[1], -m_arCamera[2]);
		}

		glEnd();

		glEnable(GL_LIGHTING);

		COpenGL::Check4Errors();
	}*/

	/*
	Clip space
	*/
	// DISABLED
	//DrawClipSpace();

	/*
	Scene
	*/
	DrawCoordinateSystem();

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
	//DrawFacesFrameBuffer();
#endif	
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
				m_iPointedFace = -1;

				m_ptSelectedPoint = m_pSelectedInstance != NULL ? point : CPoint(-1, -1);

				m_arSelectedPoint[0] = -FLT_MAX;
				m_arSelectedPoint[1] = -FLT_MAX;
				m_arSelectedPoint[2] = -FLT_MAX;

				//m_mapFacesSelectionColors.clear();

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
	m_fXAngle = 30.0f;
	m_fYAngle = 30.0f;
	m_fXTranslation = 0.0f;
	m_fYTranslation = 0.0f;
	m_fZTranslation = -5.0f;

	m_bShowFaces = TRUE;
	m_bShowFacesPolygons = FALSE;
	m_bShowConceptualFacesPolygons = TRUE;
	m_bShowLines = TRUE;
	m_bShowPoints = TRUE;
	m_bShowBoundingBoxes = FALSE;
	m_bShowNormalVectors = FALSE;
	m_bShowTangenVectors = FALSE;
	m_bShowBiNormalVectors = FALSE;
	m_bScaleVectors = FALSE;

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
		m_iPointedFace = -1;

		m_pSelectedInstance = pSelectedInstance;

		m_arSelectedPoint[0] = -FLT_MAX;
		m_arSelectedPoint[1] = -FLT_MAX;
		m_arSelectedPoint[2] = -FLT_MAX;

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
float* COpenGLSTEPView::GetVertices(const vector<CProductDefinition*>& vecProductDefinitions, int_t& iVerticesCount)
{
	iVerticesCount = 0;
	for (size_t iProductDefinition = 0; iProductDefinition < vecProductDefinitions.size(); iProductDefinition++)
	{
		iVerticesCount += vecProductDefinitions[iProductDefinition]->getVerticesCount();
	}

	float* pVertices = new float[iVerticesCount * GEOMETRY_VBO_VERTEX_LENGTH];

	int_t iOffset = 0;
	for (size_t iProductDefinition = 0; iProductDefinition < vecProductDefinitions.size(); iProductDefinition++)
	{
		float* pVBOVertices = vecProductDefinitions[iProductDefinition]->BuildVBOVertexBuffer();

		memcpy((float*)pVertices + iOffset, pVBOVertices,
			vecProductDefinitions[iProductDefinition]->getVerticesCount() * GEOMETRY_VBO_VERTEX_LENGTH * sizeof(float));

		delete[] pVBOVertices;

		iOffset += vecProductDefinitions[iProductDefinition]->getVerticesCount() * GEOMETRY_VBO_VERTEX_LENGTH;
	}

	return pVertices;
}

// ------------------------------------------------------------------------------------------------
unsigned int* COpenGLSTEPView::GetMaterialsIndices(const vector<CSTEPGeometryWithMaterial*> & vecSTEPMaterials, int_t & iIndicesCount)
{
	iIndicesCount = 0;
	for (size_t iMaterial = 0; iMaterial < vecSTEPMaterials.size(); iMaterial++)
	{
		iIndicesCount += vecSTEPMaterials[iMaterial]->getIndicesCount();
	}

	unsigned int* pIndices = new unsigned int[iIndicesCount];

	int_t iOffset = 0;
	for (size_t iMaterial = 0; iMaterial < vecSTEPMaterials.size(); iMaterial++)
	{
		memcpy((unsigned int*)pIndices + iOffset, vecSTEPMaterials[iMaterial]->getIndices(),
			vecSTEPMaterials[iMaterial]->getIndicesCount() * sizeof(unsigned int));

		iOffset += vecSTEPMaterials[iMaterial]->getIndicesCount();
	}

	return pIndices;
}

// ------------------------------------------------------------------------------------------------
unsigned int* COpenGLSTEPView::GetLinesCohortsIndices(const vector<CLinesCohort*>& vecLinesCohorts, int_t& iIndicesCount)
{
	iIndicesCount = 0;
	for (size_t iCohort = 0; iCohort < vecLinesCohorts.size(); iCohort++)
	{
		iIndicesCount += vecLinesCohorts[iCohort]->getIndicesCount();
	}

	unsigned int* pIndices = new unsigned int[iIndicesCount];

	int_t iOffset = 0;
	for (size_t iCohort = 0; iCohort < vecLinesCohorts.size(); iCohort++)
	{
		memcpy((unsigned int*)pIndices + iOffset, vecLinesCohorts[iCohort]->getIndices(),
			vecLinesCohorts[iCohort]->getIndicesCount() * sizeof(unsigned int));

		iOffset += vecLinesCohorts[iCohort]->getIndicesCount();
	}

	return pIndices;
}

// ------------------------------------------------------------------------------------------------
unsigned int* COpenGLSTEPView::GetPointsCohortsIndices(const vector<CPointsCohort*>& vecPointsCohorts, int_t& iIndicesCount)
{
	iIndicesCount = 0;
	for (size_t iCohort = 0; iCohort < vecPointsCohorts.size(); iCohort++)
	{
		iIndicesCount += vecPointsCohorts[iCohort]->getIndicesCount();
	}

	unsigned int* pIndices = new unsigned int[iIndicesCount];

	int_t iOffset = 0;
	for (size_t iCohort = 0; iCohort < vecPointsCohorts.size(); iCohort++)
	{
		memcpy((unsigned int*)pIndices + iOffset, vecPointsCohorts[iCohort]->getIndices(),
			vecPointsCohorts[iCohort]->getIndicesCount() * sizeof(unsigned int));

		iOffset += vecPointsCohorts[iCohort]->getIndicesCount();
	}

	return pIndices;
}

// ------------------------------------------------------------------------------------------------
unsigned int* COpenGLSTEPView::GetWireframesCohortsIndices(const vector<CWireframesCohort*>& vecWireframesCohorts, int_t& iIndicesCount)
{
	iIndicesCount = 0;
	for (size_t iCohort = 0; iCohort < vecWireframesCohorts.size(); iCohort++)
	{
		iIndicesCount += vecWireframesCohorts[iCohort]->getIndicesCount();
	}

	unsigned int* pIndices = new unsigned int[iIndicesCount];

	int_t iOffset = 0;
	for (size_t iCohort = 0; iCohort < vecWireframesCohorts.size(); iCohort++)
	{
		if (vecWireframesCohorts[iCohort]->getIndicesCount() == 0)
		{
			continue;
		}

		memcpy((unsigned int*)pIndices + iOffset, vecWireframesCohorts[iCohort]->getIndices(),
			vecWireframesCohorts[iCohort]->getIndicesCount() * sizeof(unsigned int));

		iOffset += vecWireframesCohorts[iCohort]->getIndicesCount();
	}

	return pIndices;
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::ReleaseBuffers()
{
	/*for (size_t iBuffers = 0; iBuffers < m_vecFacesVBOs.size(); iBuffers++)
	{
		glDeleteBuffers(1, &m_vecFacesVBOs[iBuffers]);
	}
	m_vecFacesVBOs.clear();

	for (size_t iBuffers = 0; iBuffers < m_vecFacesIBOs.size(); iBuffers++)
	{
		delete m_vecFacesIBOs[iBuffers];
	}
	m_vecFacesIBOs.clear();

	for (size_t iBuffers = 0; iBuffers < m_vecWireframesVBOs.size(); iBuffers++)
	{
		glDeleteBuffers(1, &m_vecWireframesVBOs[iBuffers]);
	}
	m_vecWireframesVBOs.clear();

	for (size_t iBuffers = 0; iBuffers < m_vecWireframesIBOs.size(); iBuffers++)
	{
		delete m_vecWireframesIBOs[iBuffers];
	}
	m_vecWireframesIBOs.clear();*/
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::DrawClipSpace()
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

	COpenGL::Check4Errors();
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::DrawCoordinateSystem()
{
	const float ARROW_SIZE = 1.5f;

#ifdef _USE_SHADERS
	glProgramUniform1f(
		m_pProgram->GetID(),
		m_pProgram->geUseBinnPhongModel(),
		0.f);

	glProgramUniformMatrix4fv(
		m_pProgram->GetID(),
		m_pProgram->getMVMatrix(),
		1, 
		false, 
		glm::value_ptr(m_modelViewMatrix));
	
	glProgramUniform1f(
		m_pProgram->GetID(),
		m_pProgram->getTransparency(),
		1.f);

	// X axis
	glProgramUniform3f(
		m_pProgram->GetID(),
		m_pProgram->getMaterialAmbientColor(),
		1.f, 
		0.f, 
		0.f);

	glBegin(GL_LINES);
	glVertex3d(0., 0., 0.);
	glVertex3d(0. + ARROW_SIZE, 0., 0.);
	glEnd();
	DrawTextGDI(L"X", ARROW_SIZE + (ARROW_SIZE * 0.05f), 0.f, 0.f);

	// Y axis
	glProgramUniform3f(
		m_pProgram->GetID(),
		m_pProgram->getMaterialAmbientColor(),
		0.f, 
		1.f, 
		0.f);

	glBegin(GL_LINES);
	glVertex3d(0., 0., 0.);
	glVertex3d(0., 0. + ARROW_SIZE, 0.);
	glEnd();
	DrawTextGDI(L"Y", 0.f, ARROW_SIZE + (ARROW_SIZE * 0.05f), 0.f);

	// Z axis
	glProgramUniform3f(
		m_pProgram->GetID(),
		m_pProgram->getMaterialAmbientColor(),
		0.f, 
		0.f, 
		1.f);

	glBegin(GL_LINES);
	glVertex3d(0., 0., 0.);
	glVertex3d(0., 0., 0. + ARROW_SIZE);
	glEnd();
	DrawTextGDI(L"Z", 0.f, 0.f, ARROW_SIZE + (ARROW_SIZE * 0.05f));
#else
	glDisable(GL_LIGHTING);

	glLineWidth(1.0);
	glColor3f(0.0f, 0.0f, 0.0f);	

	glLineWidth(2.0);

	// X axis
	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex3d(0., 0., 0.);
	glVertex3d(0. + ARROW_SIZE, 0., 0.);
	glEnd();
	DrawTextGDI(L"X", ARROW_SIZE + (ARROW_SIZE * 0.05f), 0.f, 0.f);

	// Y axis
	glColor3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex3d(0., 0., 0.);
	glVertex3d(0., 0. + ARROW_SIZE, 0.);
	glEnd();
	DrawTextGDI(L"Y", 0.f, ARROW_SIZE + (ARROW_SIZE * 0.05f), 0.f);

	// Z axis
	glColor3f(0.0f, 0.0f, 1.0f);
	glBegin(GL_LINES);
	glVertex3d(0., 0., 0.);
	glVertex3d(0., 0., 0. + ARROW_SIZE);
	glEnd();
	DrawTextGDI(L"Z", 0.f, 0.f, ARROW_SIZE + (ARROW_SIZE * 0.05f));

	glEnable(GL_LIGHTING);
#endif // _USE_SHADERS

	COpenGL::Check4Errors();
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

	float fXTranslation = 0.f;
	float fYTranslation = 0.f;
	float fZTranslation = 0.f;
	pModel->GetWorldTranslations(fXTranslation, fYTranslation, fZTranslation);	

	if (bTransparent)
	{
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

#ifdef _USE_SHADERS
	glProgramUniform1f(
		m_pProgram->GetID(),
		m_pProgram->geUseBinnPhongModel(),
		1.f);

	for (size_t iDrawMetaData = 0; iDrawMetaData < m_veCSTEPDrawMetaData.size(); iDrawMetaData++)
	{
		if (m_veCSTEPDrawMetaData[iDrawMetaData]->GetType() != mdtGeometry)
		{
			continue;
		}

		const map<GLuint, vector<CProductDefinition*>>& mapGroups = m_veCSTEPDrawMetaData[iDrawMetaData]->getVBOGroups();

		map<GLuint, vector<CProductDefinition*>>::const_iterator itGroups = mapGroups.begin();
		for (; itGroups != mapGroups.end(); itGroups++)
		{
			bool bBindVBO = true;			

			for (size_t iObject = 0; iObject < itGroups->second.size(); iObject++)
			{
				CProductDefinition* pProductDefinition = itGroups->second[iObject];

				const vector<CProductInstance*>& vecProductInstances = pProductDefinition->getProductInstances();
				for (size_t iInstance = 0; iInstance < vecProductInstances.size(); iInstance++)
				{
					auto pProductInstance = vecProductInstances[iInstance];

					if (!pProductInstance->getEnable())
					{
						continue;
					}

					if (pProductDefinition->conceptualFacesMaterials().empty())
					{
						continue;
					}		

					bool bTransform = true;					

					/*
					* Conceptual faces
					*/
					GLuint iCurrentIBO = 0;
					for (size_t iGeometryWithMaterial = 0; iGeometryWithMaterial < pProductDefinition->conceptualFacesMaterials().size(); iGeometryWithMaterial++)
					{
						CSTEPGeometryWithMaterial* pGeometryWithMaterial = pProductDefinition->conceptualFacesMaterials()[iGeometryWithMaterial];

						const CSTEPMaterial* pMaterial =
							pProductInstance == m_pSelectedInstance ? m_pSelectedInstanceMaterial :
							pProductInstance == m_pPointedInstance ? m_pPointedInstanceMaterial :
							pGeometryWithMaterial->getMaterial();

						if (bTransparent)
						{
							if (pMaterial->A() == 1.0)
							{
								continue;
							}
						}
						else
						{
							if (pMaterial->A() < 1.0)
							{
								continue;
							}
						}

						if (bBindVBO)
						{
							glBindBuffer(GL_ARRAY_BUFFER, itGroups->first);
							glVertexAttribPointer(m_pProgram->getVertexPosition(), 3, GL_FLOAT, false, sizeof(GLfloat) * GEOMETRY_VBO_VERTEX_LENGTH, 0);
							glEnableVertexAttribArray(m_pProgram->getVertexPosition());
							glVertexAttribPointer(m_pProgram->getVertexNormal(), 3, GL_FLOAT, false, sizeof(GLfloat) * GEOMETRY_VBO_VERTEX_LENGTH, (void*)(sizeof(GLfloat) * 3));
							glEnableVertexAttribArray(m_pProgram->getVertexNormal());

							bBindVBO = false;
						}

						if (bTransform)
						{
							/*
							* Transformation Matrix
							*/
							glm::mat4 transformationMatrix = glm::make_mat4((GLdouble*)pProductInstance->getTransformationMatrix());

							/*
							* Model-View Matrix
							*/
							glm::mat4 modelViewMatrix = m_modelViewMatrix;
							modelViewMatrix = glm::translate(modelViewMatrix, glm::vec3(fXTranslation, fYTranslation, fZTranslation));
							modelViewMatrix = modelViewMatrix * transformationMatrix;
							modelViewMatrix = glm::translate(modelViewMatrix, glm::vec3(-fXTranslation, -fYTranslation, -fZTranslation));

							glProgramUniformMatrix4fv(
								m_pProgram->GetID(),
								m_pProgram->getMVMatrix(),
								1,
								false,
								glm::value_ptr(modelViewMatrix));

							/*
							* Normal Matrix
							*/
							glm::mat4 normalMatrix = modelViewMatrix;
							normalMatrix = glm::inverse(normalMatrix);
							normalMatrix = glm::transpose(normalMatrix);

							glProgramUniformMatrix4fv(
								m_pProgram->GetID(),
								m_pProgram->getNMatrix(),
								1,
								false,
								value_ptr(normalMatrix));

							bTransform = false;
						}

						/*
						* Material - Ambient color
						*/
						glProgramUniform3f(m_pProgram->GetID(),
							m_pProgram->getMaterialAmbientColor(),
							pMaterial->getAmbientColor().R(),
							pMaterial->getAmbientColor().G(),
							pMaterial->getAmbientColor().B());

						/*
						* Material - Transparency
						*/
						glProgramUniform1f(
							m_pProgram->GetID(),
							m_pProgram->getTransparency(),
							pMaterial->A());

						/*
						* Material - Diffuse color
						*/
						glProgramUniform3f(m_pProgram->GetID(),
							m_pProgram->getMaterialDiffuseColor(),
							pMaterial->getDiffuseColor().R() / 2.f,
							pMaterial->getDiffuseColor().G() / 2.f,
							pMaterial->getDiffuseColor().B() / 2.f);

						/*
						* Material - Specular color
						*/
						glProgramUniform3f(m_pProgram->GetID(),
							m_pProgram->getMaterialSpecularColor(),
							pMaterial->getSpecularColor().R() / 2.f,
							pMaterial->getSpecularColor().G() / 2.f,
							pMaterial->getSpecularColor().B() / 2.f);

						/*
						* Material - Emissive color
						*/
						glProgramUniform3f(m_pProgram->GetID(),
							m_pProgram->getMaterialEmissiveColor(),
							pMaterial->getEmissiveColor().R() / 3.f,
							pMaterial->getEmissiveColor().G() / 3.f,
							pMaterial->getEmissiveColor().B() / 3.f);

						if (iCurrentIBO != pGeometryWithMaterial->IBO())
						{
							glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pGeometryWithMaterial->IBO());

							iCurrentIBO = pGeometryWithMaterial->IBO();
						}						

						glDrawElementsBaseVertex(GL_TRIANGLES,
							(GLsizei)pGeometryWithMaterial->getIndicesCount(),
							GL_UNSIGNED_INT,
							(void*)(sizeof(GLuint) * pGeometryWithMaterial->IBOOffset()),
							pProductDefinition->VBOOffset());

						//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
					} // for (size_t iMaterial = ...
				} // for (size_t iInstance = ...
			} // for (size_t iObject = ...

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		} // for (; itGroups != ...
	} // for (size_t iDrawMetaData = ...

	glDisableVertexAttribArray(m_pProgram->getVertexNormal());
#else
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	for (size_t iDrawMetaData = 0; iDrawMetaData < m_veCSTEPDrawMetaData.size(); iDrawMetaData++)
	{
		if (m_veCSTEPDrawMetaData[iDrawMetaData]->GetType() != mdtGeometry)
		{
			continue;
		}

		const map<GLuint, vector<CProductDefinition*>>& mapGroups = m_veCSTEPDrawMetaData[iDrawMetaData]->getVBOGroups();

		map<GLuint, vector<CProductDefinition*>>::const_iterator itGroups = mapGroups.begin();
		for (; itGroups != mapGroups.end(); itGroups++)
		{
			glBindBuffer(GL_ARRAY_BUFFER, itGroups->first);
			glVertexPointer(3, GL_FLOAT, sizeof(GLfloat) * GEOMETRY_VBO_VERTEX_LENGTH, NULL);
			glNormalPointer(GL_FLOAT, sizeof(GLfloat) * GEOMETRY_VBO_VERTEX_LENGTH, (float*)(sizeof(GLfloat) * 3));

			for (size_t iObject = 0; iObject < itGroups->second.size(); iObject++)
			{
				CProductDefinition* pProductDefinition = itGroups->second[iObject];

				/*
				* Transformations
				*/
				GLint iMatrixMode = 0;
				glGetIntegerv(GL_MATRIX_MODE, &iMatrixMode);

				glMatrixMode(GL_MODELVIEW);

				const vector<CProductInstance*>& vecProductInstances = pProductDefinition->getProductInstances();
				for (size_t iInstance = 0; iInstance < vecProductInstances.size(); iInstance++)
				{
					auto pProductInstance = vecProductInstances[iInstance];

					if (!pProductInstance->getEnable())
					{
						continue;
					}

					glPushMatrix();
					glTranslatef(fXTranslation, fYTranslation, fZTranslation);
					glMultMatrixd((GLdouble*)pProductInstance->getTransformationMatrix());
					glTranslatef(-fXTranslation, -fYTranslation, -fZTranslation);

					/*
					* Conceptual faces
					*/
					for (size_t iGeometryWithMaterial = 0; iGeometryWithMaterial < pProductDefinition->conceptualFacesMaterials().size(); iGeometryWithMaterial++)
					{
						CSTEPGeometryWithMaterial* pGeometryWithMaterial = pProductDefinition->conceptualFacesMaterials()[iGeometryWithMaterial];

						const CSTEPMaterial* pMaterial =
							pProductInstance == m_pSelectedInstance ? m_pSelectedInstanceMaterial :
							pProductInstance == m_pPointedInstance ? m_pPointedInstanceMaterial :
							pGeometryWithMaterial->getMaterial();

						if (bTransparent)
						{
							if (pMaterial->A() == 1.0)
							{
								continue;
							}
						}
						else
						{
							if (pMaterial->A() < 1.0)
							{
								continue;
							}
						}

						/*
						* Material - Ambient color
						*/
						glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT);
						glColor4f(
							pMaterial->getAmbientColor().R(),
							pMaterial->getAmbientColor().G(),
							pMaterial->getAmbientColor().B(),
							pMaterial->A());

						/*
						* Material - Diffuse color
						*/
						glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
						glColor4f(
							pMaterial->getDiffuseColor().R() / 2.f,
							pMaterial->getDiffuseColor().G() / 2.f,
							pMaterial->getDiffuseColor().B() / 2.f,
							pMaterial->A());

						/*
						* Material - Specular color
						*/
						glColorMaterial(GL_FRONT_AND_BACK, GL_SPECULAR);
						glColor4f(
							pMaterial->getSpecularColor().R() / 2.f,
							pMaterial->getSpecularColor().G() / 2.f,
							pMaterial->getSpecularColor().B() / 2.f,
							pMaterial->A());

						/*
						* Material - Emissive color
						*/
						glColorMaterial(GL_FRONT_AND_BACK, GL_EMISSION);
						glColor4f(
							pMaterial->getEmissiveColor().R() / 3.f,
							pMaterial->getEmissiveColor().G() / 3.f,
							pMaterial->getEmissiveColor().B() / 3.f,
							pMaterial->A());

						glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pGeometryWithMaterial->IBO());

						glDrawElementsBaseVertex(GL_TRIANGLES,
							(GLsizei)pGeometryWithMaterial->getIndicesCount(),
							GL_UNSIGNED_INT,
							(void*)(sizeof(GLuint) * pGeometryWithMaterial->IBOOffset()),
							pProductDefinition->VBOOffset());

						//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
					} // for (size_t iMaterial = ...

					glPopMatrix();
				} // for (size_t iInstance = ...

				glMatrixMode(iMatrixMode);
			} // for (size_t iObject = ...

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		} // for (; itGroups != ...
	} // for (size_t iDrawMetaData = ...

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
#endif // _USE_SHADERS

	if (bTransparent)
	{
		glDisable(GL_BLEND);
	}

	COpenGL::Check4Errors();

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	TRACE(L"\n*** DrawFaces() : %lld [탎]", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());	
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::DrawFacesPolygons()
{	
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

	float fXTranslation = 0.f;
	float fYTranslation = 0.f;
	float fZTranslation = 0.f;
	pModel->GetWorldTranslations(fXTranslation, fYTranslation, fZTranslation);

#ifdef _USE_SHADERS
	glProgramUniform1f(
		m_pProgram->GetID(),
		m_pProgram->geUseBinnPhongModel(),
		0.f);

	glProgramUniform3f(
		m_pProgram->GetID(),
		m_pProgram->getMaterialAmbientColor(),
		0.f, 
		0.f, 
		0.f);

	glProgramUniform1f(
		m_pProgram->GetID(),
		m_pProgram->getTransparency(),
		1.f);

	for (size_t iDrawMetaData = 0; iDrawMetaData < m_veCSTEPDrawMetaData.size(); iDrawMetaData++)
	{
		if (m_veCSTEPDrawMetaData[iDrawMetaData]->GetType() != mdtGeometry)
		{
			continue;
		}

		const map<GLuint, vector<CProductDefinition*>>& mapGroups = m_veCSTEPDrawMetaData[iDrawMetaData]->getVBOGroups();

		map<GLuint, vector<CProductDefinition*>>::const_iterator itGroups = mapGroups.begin();
		for (; itGroups != mapGroups.end(); itGroups++)
		{
			bool bBindVBO = true;			

			for (size_t iObject = 0; iObject < itGroups->second.size(); iObject++)
			{
				CProductDefinition* pProductDefinition = itGroups->second[iObject];

				const vector<CProductInstance*>& vecProductInstances = pProductDefinition->getProductInstances();
				for (size_t iInstance = 0; iInstance < vecProductInstances.size(); iInstance++)
				{
					auto pProductInstance = vecProductInstances[iInstance];

					if (!pProductInstance->getEnable())
					{
						continue;
					}

					if (pProductDefinition->conceptualFacesCohorts().empty())
					{
						continue;
					}

					if (bBindVBO)
					{
						glBindBuffer(GL_ARRAY_BUFFER, itGroups->first);
						glVertexAttribPointer(m_pProgram->getVertexPosition(), 3, GL_FLOAT, false, sizeof(GLfloat) * GEOMETRY_VBO_VERTEX_LENGTH, 0);
						glEnableVertexAttribArray(m_pProgram->getVertexPosition());

						bBindVBO = false;
					}

					/*
					* Transformation Matrix
					*/
					glm::mat4 transformationMatrix = glm::make_mat4((GLdouble*)pProductInstance->getTransformationMatrix());

					/*
					* Model-View Matrix
					*/
					glm::mat4 modelViewMatrix = m_modelViewMatrix;
					modelViewMatrix = glm::translate(modelViewMatrix, glm::vec3(fXTranslation, fYTranslation, fZTranslation));
					modelViewMatrix = modelViewMatrix * transformationMatrix;
					modelViewMatrix = glm::translate(modelViewMatrix, glm::vec3(-fXTranslation, -fYTranslation, -fZTranslation));

					glProgramUniformMatrix4fv(
						m_pProgram->GetID(), 
						m_pProgram->getMVMatrix(), 
						1, 
						false, 
						glm::value_ptr(modelViewMatrix));

					/*
					* Wireframes
					*/
					GLuint iCurrentIBO = 0;
					for (size_t iWireframesCohort = 0; iWireframesCohort < pProductDefinition->conceptualFacesCohorts().size(); iWireframesCohort++)
					{
						CWireframesCohort* pWireframesCohort = pProductDefinition->conceptualFacesCohorts()[iWireframesCohort];

						if (iCurrentIBO != pWireframesCohort->IBO())
						{
							glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pWireframesCohort->IBO());

							iCurrentIBO = pWireframesCohort->IBO();
						}						

						glDrawElementsBaseVertex(GL_LINES,
							(GLsizei)pWireframesCohort->getIndicesCount(),
							GL_UNSIGNED_INT,
							(void*)(sizeof(GLuint) * pWireframesCohort->IBOOffset()),
							pProductDefinition->VBOOffset());

						//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
					} // for (size_t iWireframesCohort = ...
				} // for (size_t iInstance = ...
			} // for (size_t iObject = ...

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		} // for (; itGroups != ...
	} // for (size_t iDrawMetaData = ...
#else
	glDisable(GL_LIGHTING);

	glLineWidth(1.0);
	glColor4f(0.0f, 0.0f, 0.0f, 1.0);

	glEnableClientState(GL_VERTEX_ARRAY);

	for (size_t iDrawMetaData = 0; iDrawMetaData < m_veCSTEPDrawMetaData.size(); iDrawMetaData++)
	{
		if (m_veCSTEPDrawMetaData[iDrawMetaData]->GetType() != mdtGeometry)
		{
			continue;
		}

		const map<GLuint, vector<CProductDefinition*>>& mapGroups = m_veCSTEPDrawMetaData[iDrawMetaData]->getVBOGroups();

		map<GLuint, vector<CProductDefinition*>>::const_iterator itGroups = mapGroups.begin();
		for (; itGroups != mapGroups.end(); itGroups++)
		{
			glBindBuffer(GL_ARRAY_BUFFER, itGroups->first);
			glVertexPointer(3, GL_FLOAT, sizeof(GLfloat) * GEOMETRY_VBO_VERTEX_LENGTH, NULL);

			for (size_t iObject = 0; iObject < itGroups->second.size(); iObject++)
			{
				CProductDefinition* pProductDefinition = itGroups->second[iObject];

				/*
				* Transformations
				*/
				GLint iMatrixMode = 0;
				glGetIntegerv(GL_MATRIX_MODE, &iMatrixMode);

				glMatrixMode(GL_MODELVIEW);

				const vector<CProductInstance*>& vecProductInstances = pProductDefinition->getProductInstances();
				for (size_t iInstance = 0; iInstance < vecProductInstances.size(); iInstance++)
				{
					auto pProductInstance = vecProductInstances[iInstance];

					if (!pProductInstance->getEnable())
					{
						continue;
					}

					glPushMatrix();
					glTranslatef(fXTranslation, fYTranslation, fZTranslation);
					glMultMatrixd((GLdouble*)pProductInstance->getTransformationMatrix());
					glTranslatef(-fXTranslation, -fYTranslation, -fZTranslation);

					/*
					* Wireframes
					*/
					for (size_t iWireframesCohort = 0; iWireframesCohort < pProductDefinition->conceptualFacesCohorts().size(); iWireframesCohort++)
					{
						CWireframesCohort* pWireframesCohort = pProductDefinition->conceptualFacesCohorts()[iWireframesCohort];

						glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pWireframesCohort->IBO());

						glDrawElementsBaseVertex(GL_LINES,
							(GLsizei)pWireframesCohort->getIndicesCount(),
							GL_UNSIGNED_INT,
							(void*)(sizeof(GLuint) * pWireframesCohort->IBOOffset()),
							pProductDefinition->VBOOffset());

						//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
					} // for (size_t iWireframesCohort = ...

					glPopMatrix();
				} // for (size_t iInstance = ...				

				glMatrixMode(iMatrixMode);
			} // for (size_t iObject = ...

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		} // for (; itGroups != ...
	} // for (size_t iDrawMetaData = ...

	glDisableClientState(GL_VERTEX_ARRAY);

	glEnable(GL_LIGHTING);
#endif // _USE_SHADERS

	COpenGL::Check4Errors();

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

	float fXTranslation = 0.f;
	float fYTranslation = 0.f;
	float fZTranslation = 0.f;
	pModel->GetWorldTranslations(fXTranslation, fYTranslation, fZTranslation);

#ifdef _USE_SHADERS
	glProgramUniform1f(
		m_pProgram->GetID(),
		m_pProgram->geUseBinnPhongModel(),
		0.f);

	glProgramUniform3f(
		m_pProgram->GetID(),
		m_pProgram->getMaterialAmbientColor(),
		0.f, 
		0.f, 
		0.f);

	glProgramUniform1f(
		m_pProgram->GetID(),
		m_pProgram->getTransparency(),
		1.f);

	for (size_t iDrawMetaData = 0; iDrawMetaData < m_veCSTEPDrawMetaData.size(); iDrawMetaData++)
	{
		if (m_veCSTEPDrawMetaData[iDrawMetaData]->GetType() != mdtGeometry)
		{
			continue;
		}

		const map<GLuint, vector<CProductDefinition*>>& mapGroups = m_veCSTEPDrawMetaData[iDrawMetaData]->getVBOGroups();

		map<GLuint, vector<CProductDefinition*>>::const_iterator itGroups = mapGroups.begin();
		for (; itGroups != mapGroups.end(); itGroups++)
		{
			bool bBindVBO = true;			

			for (size_t iObject = 0; iObject < itGroups->second.size(); iObject++)
			{
				CProductDefinition* pProductDefinition = itGroups->second[iObject];				

				const vector<CProductInstance*>& vecProductInstances = pProductDefinition->getProductInstances();
				for (size_t iInstance = 0; iInstance < vecProductInstances.size(); iInstance++)
				{
					auto pProductInstance = vecProductInstances[iInstance];

					if (!pProductInstance->getEnable())
					{
						continue;
					}

					if (pProductDefinition->linesCohorts().empty())
					{
						continue;
					}

					if (bBindVBO)
					{
						glBindBuffer(GL_ARRAY_BUFFER, itGroups->first);
						glVertexAttribPointer(m_pProgram->getVertexPosition(), 3, GL_FLOAT, false, sizeof(GLfloat) * GEOMETRY_VBO_VERTEX_LENGTH, 0);
						glEnableVertexAttribArray(m_pProgram->getVertexPosition());

						bBindVBO = false;
					}

					/*
					* Transformation Matrix
					*/
					glm::mat4 transformationMatrix = glm::make_mat4((GLdouble*)pProductInstance->getTransformationMatrix());

					/*
					* Model-View Matrix
					*/
					glm::mat4 modelViewMatrix = m_modelViewMatrix;
					modelViewMatrix = glm::translate(modelViewMatrix, glm::vec3(fXTranslation, fYTranslation, fZTranslation));
					modelViewMatrix = modelViewMatrix * transformationMatrix;
					modelViewMatrix = glm::translate(modelViewMatrix, glm::vec3(-fXTranslation, -fYTranslation, -fZTranslation));

					glProgramUniformMatrix4fv(
						m_pProgram->GetID(), 
						m_pProgram->getMVMatrix(), 
						1, 
						false, 
						glm::value_ptr(modelViewMatrix));

					/*
					* Lines
					*/
					GLuint iCurrentIBO = 0;
					for (size_t iLinesCohort = 0; iLinesCohort < pProductDefinition->linesCohorts().size(); iLinesCohort++)
					{
						CLinesCohort* pLinesCohort = pProductDefinition->linesCohorts()[iLinesCohort];

						if (iCurrentIBO != pLinesCohort->IBO())
						{
							glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pLinesCohort->IBO());

							iCurrentIBO = pLinesCohort->IBO();
						}

						glDrawElementsBaseVertex(GL_LINES,
							(GLsizei)pLinesCohort->getIndicesCount(),
							GL_UNSIGNED_INT,
							(void*)(sizeof(GLuint) * pLinesCohort->IBOOffset()),
							pProductDefinition->VBOOffset());

						//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
					} // for (size_t iLinesCohort = ...
				} // for (size_t iInstance = ...
			} // for (size_t iObject = ...

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		} // for (; itGroups != ...
	} // for (size_t iDrawMetaData = ...
#else
	glDisable(GL_LIGHTING);

	glLineWidth(m_fLineWidth);
	glColor4f(0.0f, 0.0f, 0.0f, 1.0);

	glEnableClientState(GL_VERTEX_ARRAY);

	for (size_t iDrawMetaData = 0; iDrawMetaData < m_veCSTEPDrawMetaData.size(); iDrawMetaData++)
	{
		if (m_veCSTEPDrawMetaData[iDrawMetaData]->GetType() != mdtGeometry)
		{
			continue;
		}

		const map<GLuint, vector<CProductDefinition*>>& mapGroups = m_veCSTEPDrawMetaData[iDrawMetaData]->getVBOGroups();

		map<GLuint, vector<CProductDefinition*>>::const_iterator itGroups = mapGroups.begin();
		for (; itGroups != mapGroups.end(); itGroups++)
		{
			glBindBuffer(GL_ARRAY_BUFFER, itGroups->first);
			glVertexPointer(3, GL_FLOAT, sizeof(GLfloat) * GEOMETRY_VBO_VERTEX_LENGTH, NULL);

			for (size_t iObject = 0; iObject < itGroups->second.size(); iObject++)
			{
				CProductDefinition* pProductDefinition = itGroups->second[iObject];

				/*
				* Transformations
				*/
				GLint iMatrixMode = 0;
				glGetIntegerv(GL_MATRIX_MODE, &iMatrixMode);

				glMatrixMode(GL_MODELVIEW);

				const vector<CProductInstance*>& vecProductInstances = pProductDefinition->getProductInstances();
				for (size_t iInstance = 0; iInstance < vecProductInstances.size(); iInstance++)
				{
					auto pProductInstance = vecProductInstances[iInstance];

					if (!pProductInstance->getEnable())
					{
						continue;
					}

					glPushMatrix();
					glTranslatef(fXTranslation, fYTranslation, fZTranslation);
					glMultMatrixd((GLdouble*)pProductInstance->getTransformationMatrix());
					glTranslatef(-fXTranslation, -fYTranslation, -fZTranslation);

					/*
					* Lines
					*/
					for (size_t iLinesCohort = 0; iLinesCohort < pProductDefinition->linesCohorts().size(); iLinesCohort++)
					{
						CLinesCohort* pLinesCohort = pProductDefinition->linesCohorts()[iLinesCohort];

						glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pLinesCohort->IBO());

						glDrawElementsBaseVertex(GL_LINES,
							(GLsizei)pLinesCohort->getIndicesCount(),
							GL_UNSIGNED_INT,
							(void*)(sizeof(GLuint) * pLinesCohort->IBOOffset()),
							pProductDefinition->VBOOffset());

						//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
					} // for (size_t iLinesCohort = ...

					glPopMatrix();
				} // for (size_t iInstance = ...	

				glMatrixMode(iMatrixMode);
			} // for (size_t iObject = ...

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		} // for (; itGroups != ...
	} // for (size_t iDrawMetaData = ...

	glDisableClientState(GL_VERTEX_ARRAY);

	glEnable(GL_LIGHTING);
#endif // #ifdef _USE_SHADERS	

	COpenGL::Check4Errors();

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

	float fXTranslation = 0.f;
	float fYTranslation = 0.f;
	float fZTranslation = 0.f;
	pModel->GetWorldTranslations(fXTranslation, fYTranslation, fZTranslation);

#ifdef _USE_SHADERS
	glProgramUniform1f(
		m_pProgram->GetID(),
		m_pProgram->geUseBinnPhongModel(),
		0.f);

	glProgramUniform3f(
		m_pProgram->GetID(),
		m_pProgram->getMaterialAmbientColor(),
		0.f, 
		0.f, 
		0.f);

	glProgramUniform1f(
		m_pProgram->GetID(),
		m_pProgram->getTransparency(),
		1.f);

	for (size_t iDrawMetaData = 0; iDrawMetaData < m_veCSTEPDrawMetaData.size(); iDrawMetaData++)
	{
		if (m_veCSTEPDrawMetaData[iDrawMetaData]->GetType() != mdtGeometry)
		{
			continue;
		}

		const map<GLuint, vector<CProductDefinition*>>& mapGroups = m_veCSTEPDrawMetaData[iDrawMetaData]->getVBOGroups();

		map<GLuint, vector<CProductDefinition*>>::const_iterator itGroups = mapGroups.begin();
		for (; itGroups != mapGroups.end(); itGroups++)
		{
			bool bBindVBO = true;			

			for (size_t iObject = 0; iObject < itGroups->second.size(); iObject++)
			{
				CProductDefinition* pProductDefinition = itGroups->second[iObject];

				const vector<CProductInstance*>& vecProductInstances = pProductDefinition->getProductInstances();
				for (size_t iInstance = 0; iInstance < vecProductInstances.size(); iInstance++)
				{
					auto pProductInstance = vecProductInstances[iInstance];

					if (!pProductInstance->getEnable())
					{
						continue;
					}

					if (pProductDefinition->pointsCohorts().empty())
					{
						continue;
					}

					if (bBindVBO)
					{
						glBindBuffer(GL_ARRAY_BUFFER, itGroups->first);
						glVertexAttribPointer(m_pProgram->getVertexPosition(), 3, GL_FLOAT, false, sizeof(GLfloat) * GEOMETRY_VBO_VERTEX_LENGTH, 0);
						glEnableVertexAttribArray(m_pProgram->getVertexPosition());

						bBindVBO = false;
					}

					/*
					* Transformation Matrix
					*/
					glm::mat4 transformationMatrix = glm::make_mat4((GLdouble*)pProductInstance->getTransformationMatrix());

					/*
					* Model-View Matrix
					*/
					glm::mat4 modelViewMatrix = m_modelViewMatrix;
					modelViewMatrix = glm::translate(modelViewMatrix, glm::vec3(fXTranslation, fYTranslation, fZTranslation));
					modelViewMatrix = modelViewMatrix * transformationMatrix;
					modelViewMatrix = glm::translate(modelViewMatrix, glm::vec3(-fXTranslation, -fYTranslation, -fZTranslation));

					glProgramUniformMatrix4fv(
						m_pProgram->GetID(), 
						m_pProgram->getMVMatrix(),
						1, 
						false, 
						glm::value_ptr(modelViewMatrix));

					/*
					* Points
					*/
					GLuint iCurrentIBO = 0;
					for (size_t iPointsCohort = 0; iPointsCohort < pProductDefinition->pointsCohorts().size(); iPointsCohort++)
					{
						CPointsCohort* pPointsCohort = pProductDefinition->pointsCohorts()[iPointsCohort];

						if (iCurrentIBO != pPointsCohort->IBO())
						{
							glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pPointsCohort->IBO());

							iCurrentIBO = pPointsCohort->IBO();
						}						

						glDrawElementsBaseVertex(GL_POINTS,
							(GLsizei)pPointsCohort->getIndicesCount(),
							GL_UNSIGNED_INT,
							(void*)(sizeof(GLuint) * pPointsCohort->IBOOffset()),
							pProductDefinition->VBOOffset());

						//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
					} // for (size_t iPointsCohort = ...
				} // for (size_t iInstance = ...
			} // for (size_t iObject = ...

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		} // for (; itGroups != ...
	} // for (size_t iDrawMetaData = ...
#else
	glDisable(GL_LIGHTING);

	glPointSize(m_fPointSize);
	glColor4f(0.0f, 0.0f, 0.0f, 1.0);

	glEnableClientState(GL_VERTEX_ARRAY);

	for (size_t iDrawMetaData = 0; iDrawMetaData < m_veCSTEPDrawMetaData.size(); iDrawMetaData++)
	{
		if (m_veCSTEPDrawMetaData[iDrawMetaData]->GetType() != mdtGeometry)
		{
			continue;
		}

		const map<GLuint, vector<CProductDefinition*>>& mapGroups = m_veCSTEPDrawMetaData[iDrawMetaData]->getVBOGroups();

		map<GLuint, vector<CProductDefinition*>>::const_iterator itGroups = mapGroups.begin();
		for (; itGroups != mapGroups.end(); itGroups++)
		{
			glBindBuffer(GL_ARRAY_BUFFER, itGroups->first);
			glVertexPointer(3, GL_FLOAT, sizeof(GLfloat) * GEOMETRY_VBO_VERTEX_LENGTH, NULL);

			for (size_t iObject = 0; iObject < itGroups->second.size(); iObject++)
			{
				CProductDefinition* pProductDefinition = itGroups->second[iObject];

				/*
				* Transformations
				*/
				GLint iMatrixMode = 0;
				glGetIntegerv(GL_MATRIX_MODE, &iMatrixMode);

				glMatrixMode(GL_MODELVIEW);

				const vector<CProductInstance*>& vecProductInstances = pProductDefinition->getProductInstances();
				for (size_t iInstance = 0; iInstance < vecProductInstances.size(); iInstance++)
				{
					auto pProductInstance = vecProductInstances[iInstance];

					if (!pProductInstance->getEnable())
					{
						continue;
					}

					glPushMatrix();
					glTranslatef(fXTranslation, fYTranslation, fZTranslation);
					glMultMatrixd((GLdouble*)pProductInstance->getTransformationMatrix());
					glTranslatef(-fXTranslation, -fYTranslation, -fZTranslation);

					/*
					* Points
					*/
					for (size_t iPointsCohort = 0; iPointsCohort < pProductDefinition->pointsCohorts().size(); iPointsCohort++)
					{
						CPointsCohort* pPointsCohort = pProductDefinition->pointsCohorts()[iPointsCohort];

						glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pPointsCohort->IBO());

						glDrawElementsBaseVertex(GL_POINTS,
							(GLsizei)pPointsCohort->getIndicesCount(),
							GL_UNSIGNED_INT,
							(void*)(sizeof(GLuint) * pPointsCohort->IBOOffset()),
							pProductDefinition->VBOOffset());

						//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
					} // for (size_t iPointsCohort = ...

					glPopMatrix();
				} // for (size_t iInstance = ...

				glMatrixMode(iMatrixMode);
			} // for (size_t iObject = ...

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		} // for (; itGroups != ...
	} // for (size_t iDrawMetaData = ...

	glDisableClientState(GL_VERTEX_ARRAY);

	glEnable(GL_LIGHTING);
#endif // _USE_SHADERS	

	COpenGL::Check4Errors();

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	TRACE(L"\n*** DrawPoints() : %lld [탎]", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::DrawInstancesFrameBuffer()
{
	if (m_bDisableSelectionBuffer)
	{
		return;
	}

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

	if (pModel->getProductDefinitions().empty())
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

	BOOL bResult = m_pOGLContext->MakeCurrent();
	VERIFY(bResult);

	if (m_iInstanceSelectionFrameBuffer == 0)
	{
		ASSERT(m_iInstanceSelectionTextureBuffer == 0);
		ASSERT(m_iInstanceSelectionDepthRenderBuffer == 0);

		/*
		* Frame buffer
		*/
		glGenFramebuffers(1, &m_iInstanceSelectionFrameBuffer);
		ASSERT(m_iInstanceSelectionFrameBuffer != 0);

		glBindFramebuffer(GL_FRAMEBUFFER, m_iInstanceSelectionFrameBuffer);

		/*
		* Texture buffer
		*/
		glGenTextures(1, &m_iInstanceSelectionTextureBuffer);
		ASSERT(m_iInstanceSelectionTextureBuffer != 0);

		glBindTexture(GL_TEXTURE_2D, m_iInstanceSelectionTextureBuffer);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SELECTION_BUFFER_SIZE, SELECTION_BUFFER_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		glBindTexture(GL_TEXTURE_2D, 0);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_iInstanceSelectionTextureBuffer, 0);

		/*
		* Depth buffer
		*/
		glGenRenderbuffers(1, &m_iInstanceSelectionDepthRenderBuffer);
		ASSERT(m_iInstanceSelectionDepthRenderBuffer != 0);

		glBindRenderbuffer(GL_RENDERBUFFER, m_iInstanceSelectionDepthRenderBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SELECTION_BUFFER_SIZE, SELECTION_BUFFER_SIZE);

		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_iInstanceSelectionDepthRenderBuffer);

		GLenum arDrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, arDrawBuffers);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		COpenGL::Check4Errors();
	} // if (m_iInstanceSelectionFrameBuffer == 0)

	/*
	* Selection colors
	*/
	if (m_mapInstancesSelectionColors.empty())
	{
		const float STEP = 1.0f / 255.0f;

		const map<int64_t, CProductInstance *> & mapProductInstances = pModel->getProductInstances();

		map<int64_t, CProductInstance*>::const_iterator itProductInstances = mapProductInstances.begin();
		for (; itProductInstances != mapProductInstances.end(); itProductInstances++)
		{
			CProductInstance* pProductInstance = itProductInstances->second;			

			const vector<pair<int64_t, int64_t> > & vecTriangles = pProductInstance->getProductDefinition()->getTriangles();
			if (vecTriangles.empty())
			{
				continue;
			}

			float fR = floorf((float)pProductInstance->getID() / (255.0f * 255.0f));
			if (fR >= 1.0f)
			{
				fR *= STEP;
			}

			float fG = floorf((float)pProductInstance->getID() / 255.0f);
			if (fG >= 1.0f)
			{
				fG *= STEP;
			}

			float fB = (float)(pProductInstance->getID() % 255);
			fB *= STEP;

			ASSERT(m_mapInstancesSelectionColors.find(pProductInstance->getID()) == m_mapInstancesSelectionColors.end());
			m_mapInstancesSelectionColors[pProductInstance->getID()] = CSTEPColor(fR, fG, fB);
		} // for (; itProductInstances != ...
	} // if (m_mapInstancesSelectionColors.empty())

	/*
	* Draw
	*/

	glBindFramebuffer(GL_FRAMEBUFFER, m_iInstanceSelectionFrameBuffer);

	glViewport(0, 0, SELECTION_BUFFER_SIZE, SELECTION_BUFFER_SIZE);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set up the parameters
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

#ifdef _USE_SHADERS
	glProgramUniform1f(
		m_pProgram->GetID(),
		m_pProgram->geUseBinnPhongModel(),
		0.f);

	glProgramUniform1f(
		m_pProgram->GetID(),
		m_pProgram->getTransparency(),
		1.f);

	float fXTranslation = 0.f;
	float fYTranslation = 0.f;
	float fZTranslation = 0.f;
	pModel->GetWorldTranslations(fXTranslation, fYTranslation, fZTranslation);

	for (size_t iDrawMetaData = 0; iDrawMetaData < m_veCSTEPDrawMetaData.size(); iDrawMetaData++)
	{
		if (m_veCSTEPDrawMetaData[iDrawMetaData]->GetType() != mdtGeometry)
		{
			continue;
		}

		const map<GLuint, vector<CProductDefinition*>>& mapGroups = m_veCSTEPDrawMetaData[iDrawMetaData]->getVBOGroups();

		map<GLuint, vector<CProductDefinition*>>::const_iterator itGroups = mapGroups.begin();
		for (; itGroups != mapGroups.end(); itGroups++)
		{
			bool bBindVBO = true;			

			for (size_t iObject = 0; iObject < itGroups->second.size(); iObject++)
			{
				CProductDefinition* pProductDefinition = itGroups->second[iObject];

				const vector<CProductInstance*>& vecProductInstances = pProductDefinition->getProductInstances();
				for (size_t iInstance = 0; iInstance < vecProductInstances.size(); iInstance++)
				{
					auto pProductInstance = vecProductInstances[iInstance];

					if (!pProductInstance->getEnable())
					{
						continue;
					}

					if (pProductDefinition->conceptualFacesMaterials().empty())
					{
						continue;
					}

					if (bBindVBO)
					{
						glBindBuffer(GL_ARRAY_BUFFER, itGroups->first);
						glVertexAttribPointer(m_pProgram->getVertexPosition(), 3, GL_FLOAT, false, sizeof(GLfloat)* GEOMETRY_VBO_VERTEX_LENGTH, 0);
						glEnableVertexAttribArray(m_pProgram->getVertexPosition());

						bBindVBO = false;
					}

					/*
					* Transformation Matrix
					*/
					glm::mat4 transformationMatrix = glm::make_mat4((GLdouble*)pProductInstance->getTransformationMatrix());

					/*
					* Model-View Matrix
					*/
					glm::mat4 modelViewMatrix = m_modelViewMatrix;
					modelViewMatrix = glm::translate(modelViewMatrix, glm::vec3(fXTranslation, fYTranslation, fZTranslation));
					modelViewMatrix = modelViewMatrix * transformationMatrix;
					modelViewMatrix = glm::translate(modelViewMatrix, glm::vec3(-fXTranslation, -fYTranslation, -fZTranslation));

					glProgramUniformMatrix4fv(
						m_pProgram->GetID(),
						m_pProgram->getMVMatrix(),
						1, 
						false, 
						glm::value_ptr(modelViewMatrix));

					/*
					* Ambient color
					*/
					map<int64_t, CSTEPColor>::iterator itSelectionColor = m_mapInstancesSelectionColors.find(pProductInstance->getID());
					ASSERT(itSelectionColor != m_mapInstancesSelectionColors.end());

					/*
					* Material - Ambient color
					*/
					glProgramUniform3f(
						m_pProgram->GetID(),
						m_pProgram->getMaterialAmbientColor(),
						itSelectionColor->second.R(),
						itSelectionColor->second.G(),
						itSelectionColor->second.B());

					/*
					* Conceptual faces
					*/
					GLuint iCurrentIBO = 0;
					for (size_t iMaterial = 0; iMaterial < pProductDefinition->conceptualFacesMaterials().size(); iMaterial++)
					{
						CSTEPGeometryWithMaterial* pGeometryWithMaterial = pProductDefinition->conceptualFacesMaterials()[iMaterial];						

						if (iCurrentIBO != pGeometryWithMaterial->IBO())
						{
							glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pGeometryWithMaterial->IBO());

							iCurrentIBO = pGeometryWithMaterial->IBO();
						}						

						glDrawElementsBaseVertex(GL_TRIANGLES,
							(GLsizei)pGeometryWithMaterial->getIndicesCount(),
							GL_UNSIGNED_INT,
							(void*)(sizeof(GLuint) * pGeometryWithMaterial->IBOOffset()),
							pProductDefinition->VBOOffset());

						//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);						
					} // for (size_t iMaterial = ...
				} // for (size_t iInstance = ...
			} // for (size_t iObject = ...

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		} // for (; itGroups != ...
	} // for (size_t iDrawMetaData = ...
#else
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_FLAT);

	glDisable(GL_LIGHTING);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

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

	glFrustum(-fW, fW, -fH, fH, zNear, zFar);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(m_fXTranslation, m_fYTranslation, m_fZTranslation);

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

	glTranslatef(-fXTranslation, -fYTranslation, -fZTranslation);

	glRotatef(m_fXAngle, 1.0f, 0.0f, 0.0f);
	glRotatef(m_fYAngle, 0.0f, 1.0f, 0.0f);

	glTranslatef(fXTranslation, fYTranslation, fZTranslation);

	fXTranslation = 0.f;
	fYTranslation = 0.f;
	fZTranslation = 0.f;
	pModel->GetWorldTranslations(fXTranslation, fYTranslation, fZTranslation);

	/*
	* Draw
	*/
	glEnableClientState(GL_VERTEX_ARRAY);

	for (size_t iDrawMetaData = 0; iDrawMetaData < m_veCSTEPDrawMetaData.size(); iDrawMetaData++)
	{
		if (m_veCSTEPDrawMetaData[iDrawMetaData]->GetType() != mdtGeometry)
		{
			continue;
		}

		const map<GLuint, vector<CProductDefinition*>>& mapGroups = m_veCSTEPDrawMetaData[iDrawMetaData]->getVBOGroups();

		map<GLuint, vector<CProductDefinition*>>::const_iterator itGroups = mapGroups.begin();
		for (; itGroups != mapGroups.end(); itGroups++)
		{
			glBindBuffer(GL_ARRAY_BUFFER, itGroups->first);
			glVertexPointer(3, GL_FLOAT, sizeof(GLfloat) * GEOMETRY_VBO_VERTEX_LENGTH, NULL);

			for (size_t iObject = 0; iObject < itGroups->second.size(); iObject++)
			{
				CProductDefinition* pProductDefinition = itGroups->second[iObject];

				/*
				* Transformations
				*/
				GLint iMatrixMode = 0;
				glGetIntegerv(GL_MATRIX_MODE, &iMatrixMode);

				glMatrixMode(GL_MODELVIEW);

				const vector<CProductInstance*>& vecProductInstances = pProductDefinition->getProductInstances();
				for (size_t iInstance = 0; iInstance < vecProductInstances.size(); iInstance++)
				{
					auto pProductInstance = vecProductInstances[iInstance];

					if (!pProductInstance->getEnable())
					{
						continue;
					}

					glPushMatrix();
					glTranslatef(fXTranslation, fYTranslation, fZTranslation);
					glMultMatrixd((GLdouble*)pProductInstance->getTransformationMatrix());
					glTranslatef(-fXTranslation, -fYTranslation, -fZTranslation);

					/*
					* Ambient color
					*/
					map<int64_t, CSTEPColor>::iterator itSelectionColor = m_mapInstancesSelectionColors.find(pProductInstance->getID());
					ASSERT(itSelectionColor != m_mapInstancesSelectionColors.end());

					/*
					* Conceptual faces
					*/
					for (size_t iMaterial = 0; iMaterial < pProductDefinition->conceptualFacesMaterials().size(); iMaterial++)
					{
						CSTEPGeometryWithMaterial* pGeometryWithMaterial = pProductDefinition->conceptualFacesMaterials()[iMaterial];

						glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT);
						glColor4f(
							itSelectionColor->second.R(),
							itSelectionColor->second.G(),
							itSelectionColor->second.B(),
							1.f);

						glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pGeometryWithMaterial->IBO());

						glDrawElementsBaseVertex(GL_TRIANGLES,
							(GLsizei)pGeometryWithMaterial->getIndicesCount(),
							GL_UNSIGNED_INT,
							(void*)(sizeof(GLuint) * pGeometryWithMaterial->IBOOffset()),
							pProductDefinition->VBOOffset());

						//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);						
					} // for (size_t iMaterial = ...

					glPopMatrix();
				} // for (size_t iInstance = ...				

				glMatrixMode(iMatrixMode);
			} // for (size_t iObject = ...

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		} // for (; itGroups != ...
	} // for (size_t iDrawMetaData = ...

	glDisableClientState(GL_VERTEX_ARRAY);

	glEnable(GL_LIGHTING);
#endif // _USE_SHADERS	

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	COpenGL::Check4Errors();
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::DrawFacesFrameBuffer()
{
	ASSERT(0); // todo

//	if (m_pSelectedInstance == NULL)
//	{
//		return;
//	}
//
//	CSTEPController * pController = GetController();
//	ASSERT(pController != NULL);
//
//	CSTEPModel * pModel = pController->GetModel();
//	if (pModel == NULL)
//	{
//		return;
//	}
//
//	/*
//	* Create a frame buffer
//	*/
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
//	if (m_iFaceSelectionFrameBuffer == 0)
//	{
//		ASSERT(m_iFaceSelectionTextureBuffer == 0);
//		ASSERT(m_iFaceSelectionDepthRenderBuffer == 0);
//
//		/*
//		* Frame buffer
//		*/
//		glGenFramebuffers(1, &m_iFaceSelectionFrameBuffer);
//		ASSERT(m_iFaceSelectionFrameBuffer != 0);
//
//		glBindFramebuffer(GL_FRAMEBUFFER, m_iFaceSelectionFrameBuffer);
//
//		/*
//		* Texture buffer
//		*/
//		glGenTextures(1, &m_iFaceSelectionTextureBuffer);
//		ASSERT(m_iFaceSelectionTextureBuffer != 0);
//
//		glBindTexture(GL_TEXTURE_2D, m_iFaceSelectionTextureBuffer);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//
//		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SELECTION_BUFFER_SIZE, SELECTION_BUFFER_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
//
//		glBindTexture(GL_TEXTURE_2D, 0);
//
//		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_iFaceSelectionTextureBuffer, 0);
//
//		/*
//		* Depth buffer
//		*/
//		glGenRenderbuffers(1, &m_iFaceSelectionDepthRenderBuffer);
//		ASSERT(m_iFaceSelectionDepthRenderBuffer != 0);
//
//		glBindRenderbuffer(GL_RENDERBUFFER, m_iFaceSelectionDepthRenderBuffer);
//		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SELECTION_BUFFER_SIZE, SELECTION_BUFFER_SIZE);
//
//		glBindRenderbuffer(GL_RENDERBUFFER, 0);
//
//		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_iFaceSelectionDepthRenderBuffer);
//
//		GLenum arDrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
//		glDrawBuffers(1, arDrawBuffers);
//
//		glBindFramebuffer(GL_FRAMEBUFFER, 0);
//
//		COpenGL::Check4Errors();
//	} // if (m_iFaceSelectionFrameBuffer == 0)
//
//	/*
//	* Selection colors
//	*/
//	if (m_mapFacesSelectionColors.empty())
//	{
//		const float STEP = 1.0f / 255.0f;
//
//		const vector<pair<int64_t, int64_t> > & vecTriangles = m_pSelectedInstance->getTriangles();
//		ASSERT(!vecTriangles.empty());
//
//		for (size_t iTriangle = 0; iTriangle < vecTriangles.size(); iTriangle++)
//		{
//			float fR = floor((float)iTriangle / (255.0f * 255.0f));
//			if (fR >= 1.0f)
//			{
//				fR *= STEP;
//			}
//
//			float fG = floor((float)iTriangle / 255.0f);
//			if (fG >= 1.0f)
//			{
//				fG *= STEP;
//			}
//
//			float fB = (float)(iTriangle % 255);
//			fB *= STEP;
//
//			m_mapFacesSelectionColors[iTriangle] = CSTEPColor(fR, fG, fB);
//		} // for (size_t iTriangle = ...
//	} // if (m_mapFacesSelectionColors.empty())
//
//	/*
//	* Draw
//	*/
//
//	glBindFramebuffer(GL_FRAMEBUFFER, m_iFaceSelectionFrameBuffer);
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
//	glEnable(GL_COLOR_MATERIAL);
//
//	glShadeModel(GL_FLAT);
//
//	glDisable(GL_LIGHTING);
//
//	glMatrixMode(GL_PROJECTION);
//	glLoadIdentity();
//
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
//	glFrustum(-fW, fW, -fH, fH, zNear, zFar);
//
//	glMatrixMode(GL_MODELVIEW);
//	glLoadIdentity();
//
//	glTranslatef(m_fXTranslation, m_fYTranslation, m_fZTranslation);
//
//	float fXmin = -1.f;
//	float fXmax = 1.f;
//	float fYmin = -1.f;
//	float fYmax = 1.f;
//	float fZmin = -1.f;
//	float fZmax = 1.f;
//	pModel->GetWorldDimensions(fXmin, fXmax, fYmin, fYmax, fZmin, fZmax);
//
//	float fXTranslation = fXmin;
//	fXTranslation += (fXmax - fXmin) / 2.f;
//	fXTranslation = -fXTranslation;
//
//	float fYTranslation = fYmin;
//	fYTranslation += (fYmax - fYmin) / 2.f;
//	fYTranslation = -fYTranslation;
//
//	float fZTranslation = fZmin;
//	fZTranslation += (fZmax - fZmin) / 2.f;
//	fZTranslation = -fZTranslation;
//
//	glTranslatef(-fXTranslation, -fYTranslation, -fZTranslation);
//
//	glRotatef(m_fXAngle, 1.0f, 0.0f, 0.0f);
//	glRotatef(m_fYAngle, 0.0f, 1.0f, 0.0f);
//
//	glTranslatef(fXTranslation, fYTranslation, fZTranslation);
//
//	const vector<pair<int64_t, int64_t> > & vecTriangles = m_pSelectedInstance->getTriangles();
//	ASSERT(!vecTriangles.empty());
//
//	for (size_t iTriangle = 0; iTriangle < vecTriangles.size(); iTriangle++)
//	{
//		map<int64_t, CSTEPColor>::iterator itSelectionColor = m_mapFacesSelectionColors.find(iTriangle);
//		ASSERT(itSelectionColor != m_mapFacesSelectionColors.end());
//
//		// Ambient color
//		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT);
//		glColor4f(
//			itSelectionColor->second.R(),
//			itSelectionColor->second.G(),
//			itSelectionColor->second.B(),
//			1.f);
//
//		glBegin(GL_TRIANGLES);
//
//		for (int64_t iIndex = vecTriangles[iTriangle].first; iIndex < vecTriangles[iTriangle].first + vecTriangles[iTriangle].second; iIndex++)
//		{
//			glNormal3f(
//				m_pSelectedInstance->getVertices()[(m_pSelectedInstance->getIndices()[iIndex] * VERTEX_LENGTH) + 3],
//				m_pSelectedInstance->getVertices()[(m_pSelectedInstance->getIndices()[iIndex] * VERTEX_LENGTH) + 4],
//				m_pSelectedInstance->getVertices()[(m_pSelectedInstance->getIndices()[iIndex] * VERTEX_LENGTH) + 5]);
//
//			glVertex3f(
//				m_pSelectedInstance->getVertices()[(m_pSelectedInstance->getIndices()[iIndex] * VERTEX_LENGTH)],
//				m_pSelectedInstance->getVertices()[(m_pSelectedInstance->getIndices()[iIndex] * VERTEX_LENGTH) + 1],
//				m_pSelectedInstance->getVertices()[(m_pSelectedInstance->getIndices()[iIndex] * VERTEX_LENGTH) + 2]);
//		} // for (size_t iIndex = ...
//
//		glEnd();
//	} // for (size_t iTriangle = ...
//
//	glEnable(GL_LIGHTING);
//
//	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
//
//	COpenGL::Check4Errors();
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::DrawPointedFace()
{
	ASSERT(0); // todo

	//if (m_pSelectedInstance == NULL)
	//{
	//	return;
	//}

	//if (m_iPointedFace == -1)
	//{
	//	return;
	//}

	///*
	//* Triangles
	//*/
	//const vector<pair<int64_t, int64_t> >& vecTriangles = m_pSelectedInstance->getTriangles();
	//ASSERT(!vecTriangles.empty());
	//ASSERT((m_iPointedFace >= 0) && (m_iPointedFace < (int64_t)vecTriangles.size()));

	//// Ambient color
	//glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT);
	//glColor4f(
	//	0.f,
	//	1.f,
	//	0.f,
	//	1.f);

	//// Diffuse color
	//glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
	//glColor4f(
	//	0.f,
	//	1.f,
	//	0.f,
	//	1.f);

	//// Specular color
	//glColorMaterial(GL_FRONT_AND_BACK, GL_SPECULAR);
	//glColor4f(
	//	0.f,
	//	1.f,
	//	0.f,
	//	1.f);

	//// Emissive color
	//glColorMaterial(GL_FRONT_AND_BACK, GL_EMISSION);
	//glColor4f(
	//	0.f,
	//	1.f,
	//	0.f,
	//	1.f);

	//glBegin(GL_TRIANGLES);

	//for (int64_t iIndex = vecTriangles[m_iPointedFace].first; iIndex < vecTriangles[m_iPointedFace].first + vecTriangles[m_iPointedFace].second; iIndex++)
	//{
	//	glNormal3f(
	//		m_pSelectedInstance->getVertices()[(m_pSelectedInstance->getIndices()[iIndex] * VERTEX_LENGTH) + 3],
	//		m_pSelectedInstance->getVertices()[(m_pSelectedInstance->getIndices()[iIndex] * VERTEX_LENGTH) + 4],
	//		m_pSelectedInstance->getVertices()[(m_pSelectedInstance->getIndices()[iIndex] * VERTEX_LENGTH) + 5]);

	//	glVertex3f(
	//		m_pSelectedInstance->getVertices()[(m_pSelectedInstance->getIndices()[iIndex] * VERTEX_LENGTH)],
	//		m_pSelectedInstance->getVertices()[(m_pSelectedInstance->getIndices()[iIndex] * VERTEX_LENGTH) + 1],
	//		m_pSelectedInstance->getVertices()[(m_pSelectedInstance->getIndices()[iIndex] * VERTEX_LENGTH) + 2]);
	//} // for (size_t iIndex = ...

	//glEnd();

	///*
	//* Lines
	//*/
	//if (GetKeyState(VK_CONTROL) & 0x8000)
	//{
	//	glDisable(GL_LIGHTING);

	//	glLineWidth(1.0);
	//	glColor4f(0.0f, 0.0f, 0.0f, 1.0);

	//	glBegin(GL_LINES);

	//	for (int64_t iIndex = vecTriangles[m_iPointedFace].first; iIndex < vecTriangles[m_iPointedFace].first + vecTriangles[m_iPointedFace].second; iIndex++)
	//	{
	//		glVertex3f(
	//			m_pSelectedInstance->getVertices()[(m_pSelectedInstance->getIndices()[iIndex] * VERTEX_LENGTH)],
	//			m_pSelectedInstance->getVertices()[(m_pSelectedInstance->getIndices()[iIndex] * VERTEX_LENGTH) + 1],
	//			m_pSelectedInstance->getVertices()[(m_pSelectedInstance->getIndices()[iIndex] * VERTEX_LENGTH) + 2]);
	//	} // for (size_t iIndex = ...

	//	glEnd();

	//	glEnable(GL_LIGHTING);
	//} // if (GetKeyState(VK_CONTROL) & 0x8000)

	//COpenGL::Check4Errors();
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
		if (!pModel->getProductDefinitions().empty() && m_iInstanceSelectionFrameBuffer != 0)
		{
			int iWidth = 0;
			int iHeight = 0;

#ifdef _LINUX
			m_pOGLContext->SetCurrent(*m_pWnd);

			const wxSize szClient = m_pWnd->GetClientSize();

			iWidth = szClient.GetWidth();
			iHeight = szClient.GetHeight();
#else
			BOOL bResult = m_pOGLContext->MakeCurrent();
			VERIFY(bResult);

			CRect rcClient;
			m_pWnd->GetClientRect(&rcClient);

			iWidth = rcClient.Width();
			iHeight = rcClient.Height();
#endif // _LINUX

			GLubyte arPixels[4];
			memset(arPixels, 0, sizeof(GLubyte) * 4);

			double dX = (double)point.x * ((double)SELECTION_BUFFER_SIZE / (double)iWidth);
			double dY = ((double)iHeight - (double)point.y) * ((double)SELECTION_BUFFER_SIZE / (double)iHeight);

			glBindFramebuffer(GL_FRAMEBUFFER, m_iInstanceSelectionFrameBuffer);

			glReadPixels(
				(GLint)dX,
				(GLint)dY,
				1, 1,
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				arPixels);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

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

// ------------------------------------------------------------------------------------------------
// http://nehe.gamedev.net/article/using_gluunproject/16013/
void COpenGLSTEPView::GetOGLPos(int iX, int iY, float fDepth, GLfloat & fX, GLfloat & fY, GLfloat & fZ) const
{
	GLint arViewport[4];
	GLdouble arModelView[16];
	GLdouble arProjection[16];
	GLfloat fWinX, fWinY, fWinZ;

	glGetDoublev(GL_MODELVIEW_MATRIX, arModelView);
	glGetDoublev(GL_PROJECTION_MATRIX, arProjection);
	glGetIntegerv(GL_VIEWPORT, arViewport);

	fWinX = (float)iX;
	fWinY = (float)arViewport[3] - (float)iY;

	if (fDepth == -FLT_MAX)
	{
		glReadPixels(iX, int(fWinY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &fWinZ);
	}
	else
	{
		fWinZ = fDepth;
	}

	GLdouble dX, dY, dZ;
	GLint iResult = gluUnProject(fWinX, fWinY, fWinZ, arModelView, arProjection, arViewport, &dX, &dY, &dZ);
#ifdef _LINUX
    ASSERT(iResult == GL_TRUE);
#else
    VERIFY(iResult == GL_TRUE);
#endif // _LINUX

	fX = (GLfloat)dX;
	fY = (GLfloat)dY;
	fZ = (GLfloat)dZ;
}

// ------------------------------------------------------------------------------------------------
void COpenGLSTEPView::OGLProject(GLdouble dInX, GLdouble dInY, GLdouble dInZ, GLdouble & dOutX, GLdouble & dOutY, GLdouble & dOutZ) const
{
	GLint arViewport[4];
	GLdouble arModelView[16];
	GLdouble arProjection[16];

	glGetDoublev(GL_MODELVIEW_MATRIX, arModelView);
	glGetDoublev(GL_PROJECTION_MATRIX, arProjection);
	glGetIntegerv(GL_VIEWPORT, arViewport);

	GLint iResult = gluProject(dInX, dInY, dInZ, arModelView, arProjection, arViewport, &dOutX, &dOutY, &dOutZ);
#ifdef _LINUX
    ASSERT(iResult == GL_TRUE);
#else
    VERIFY(iResult == GL_TRUE);
#endif // _LINUX
}
