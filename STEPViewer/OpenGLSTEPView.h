#ifndef _OPEN_GL_STEP_VIEW_H_
#define _OPEN_GL_STEP_VIEW_H_

#include "Generic.h"
#include "Cohorts.h"
#include "STEPView.h"
#include "ProductDefinition.h"

#ifdef _LINUX
#include <wx/wx.h>
#include <wx/glcanvas.h>
#else
#include "OpenGLContext.h"
#include "OpenGLView.h"
#endif // _LINUX

#define _USE_MATH_DEFINES
#include <math.h>

#include "BinnPhongGLProgram.h"
#include "GLShader.h"

#include "vec3.hpp"
#include "vec4.hpp"
#include "mat4x4.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

#ifdef _LINUX
#define MK_LBUTTON 1
#define MK_MBUTTON 2
#define MK_RBUTTON 4
#endif

// ------------------------------------------------------------------------------------------------
class COpenGLLight
{

private: // Members

	// --------------------------------------------------------------------------------------------
	// Enable/disable
	bool m_bIsEnabled;

	// --------------------------------------------------------------------------------------------
	// OpenGL light, e.g. GL_LIGHT0
	GLenum m_enLight;

	// --------------------------------------------------------------------------------------------
	// Ambient
	GLfloat m_arAmbient[4];

	// --------------------------------------------------------------------------------------------
	// Diffuse
	GLfloat m_arDiffuse[4];

	// --------------------------------------------------------------------------------------------
	// Specular
	GLfloat m_arSpecular[4];

	// --------------------------------------------------------------------------------------------
	// Position
	GLfloat m_arPosition[4];

public: // Methods

	// --------------------------------------------------------------------------------------------
	// ctor
	COpenGLLight(GLenum enLight);

	// --------------------------------------------------------------------------------------------
	// ctor
	COpenGLLight(const COpenGLLight & light);

	// --------------------------------------------------------------------------------------------
	// dtor
	virtual ~COpenGLLight();

	// --------------------------------------------------------------------------------------------
	// Getter
	bool isEnabled() const;

	// --------------------------------------------------------------------------------------------
	// Setter
	void enable(bool bEnable);

	// --------------------------------------------------------------------------------------------
	// Getter
	GLenum getLight() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	GLfloat * getAmbient() const;

	// --------------------------------------------------------------------------------------------
	// Setter
	void setAmbient(GLfloat fR, GLfloat fG, GLfloat fB, GLfloat fW = 1.f);

	// --------------------------------------------------------------------------------------------
	// Getter
	GLfloat * getDiffuse() const;

	// --------------------------------------------------------------------------------------------
	// Setter
	void setDiffuse(GLfloat fR, GLfloat fG, GLfloat fB, GLfloat fW = 1.f);

	// --------------------------------------------------------------------------------------------
	// Getter
	GLfloat * getSpecular() const;

	// --------------------------------------------------------------------------------------------
	// Setter
	void setSpecular(GLfloat fR, GLfloat fG, GLfloat fB, GLfloat fW = 1.f);

	// --------------------------------------------------------------------------------------------
	// Getter
	GLfloat * getPosition() const;

	// --------------------------------------------------------------------------------------------
	// Setter
	void setPosition(GLfloat fX, GLfloat fY, GLfloat fZ, GLfloat fW);
};

// ------------------------------------------------------------------------------------------------
enum enumDrawMetaDataType
{
	mdtGeometry = 0
};

// ------------------------------------------------------------------------------------------------
typedef map<GLuint, vector<CProductDefinition*>> VBOGROUPS;

// ------------------------------------------------------------------------------------------------
// VBOs
class CSTEPDrawMetaData
{

private: // Members	

	// --------------------------------------------------------------------------------------------
	// Type
	enumDrawMetaDataType m_enDrawMetaDataType;

	// --------------------------------------------------------------------------------------------
	// VBO : CProductDefinition-s
	VBOGROUPS m_mapVBO2ProductDefinitions;

public: // Methods

	// --------------------------------------------------------------------------------------------
	// ctor
	CSTEPDrawMetaData(enumDrawMetaDataType enDrawMetaDataType)
		: m_enDrawMetaDataType(enDrawMetaDataType)
		, m_mapVBO2ProductDefinitions()
	{
	}

	// --------------------------------------------------------------------------------------------
	// dtor
	~CSTEPDrawMetaData()
	{
		VBOGROUPS::iterator itVBO2ProductDefinitions = m_mapVBO2ProductDefinitions.begin();
		for (; itVBO2ProductDefinitions != m_mapVBO2ProductDefinitions.end(); itVBO2ProductDefinitions++)
		{
			glDeleteBuffers(1, &(itVBO2ProductDefinitions->first));
		}
	}

	// --------------------------------------------------------------------------------------------
	// Getter
	enumDrawMetaDataType GetType() const
	{
		return m_enDrawMetaDataType;
	}

	// --------------------------------------------------------------------------------------------
	// Adds a group of ProductDefinition-s that share VBO
	void AddGroup(GLuint iVBO, const vector<CProductDefinition*>& vecProductDefinitions)
	{
		VBOGROUPS::iterator itVBO2ProductDefinitions = m_mapVBO2ProductDefinitions.find(iVBO);
		ASSERT(itVBO2ProductDefinitions == m_mapVBO2ProductDefinitions.end());

		m_mapVBO2ProductDefinitions[iVBO] = vecProductDefinitions;
	}

	// --------------------------------------------------------------------------------------------
	// Getter
	const VBOGROUPS& getVBOGroups() const
	{
		return m_mapVBO2ProductDefinitions;
	}
};

// ------------------------------------------------------------------------------------------------
// Open GL View
class COpenGLSTEPView : public COpenGLView
{

private: // Members

	// --------------------------------------------------------------------------------------------
	// Window
#ifdef _LINUX
    wxGLCanvas * m_pWnd;
#else
	CWnd * m_pWnd;
#endif // _LINUX

	CBinnPhongGLProgram* m_pProgram;	               // Program
	CGLShader* m_pVertSh;		       // Vertex shader
	CGLShader* m_pFragSh;

	glm::mat4 m_modelViewMatrix;

	// --------------------------------------------------------------------------------------------
	// Faces
	BOOL m_bShowFaces;

	// --------------------------------------------------------------------------------------------
	// Faces polygons
	BOOL m_bShowFacesPolygons;

	// --------------------------------------------------------------------------------------------
	// Conceptual faces polygons
	BOOL m_bShowConceptualFacesPolygons;

	// --------------------------------------------------------------------------------------------
	// Lines
	BOOL m_bShowLines;

	// --------------------------------------------------------------------------------------------
	// Line width
	GLfloat m_fLineWidth;

	// --------------------------------------------------------------------------------------------
	// Points
	BOOL m_bShowPoints;

	// --------------------------------------------------------------------------------------------
	// Point size
	GLfloat m_fPointSize;

	// --------------------------------------------------------------------------------------------
	// Bounding boxes
	BOOL m_bShowBoundingBoxes;

	// --------------------------------------------------------------------------------------------
	// Normal vectors
	BOOL m_bShowNormalVectors;

	// --------------------------------------------------------------------------------------------
	// Tangent vectors
	BOOL m_bShowTangenVectors;

	// --------------------------------------------------------------------------------------------
	// Bi-normal vectors
	BOOL m_bShowBiNormalVectors;

	// --------------------------------------------------------------------------------------------
	// Scale all vectors
	BOOL m_bScaleVectors;

	// --------------------------------------------------------------------------------------------
	BOOL m_bDisableSelectionBuffer;

	// --------------------------------------------------------------------------------------------
	// OpenGL context
#ifdef _LINUX
    wxGLContext * m_pOGLContext;
#else
	COpenGLContext * m_pOGLContext;
#endif // _LINUX

	// --------------------------------------------------------------------------------------------
	// Light Model - Ambient
	GLfloat m_arLightModelAmbient[4];

	// --------------------------------------------------------------------------------------------
	// Light Model - Local Viewer
	bool m_bLightModelLocalViewer;

	// --------------------------------------------------------------------------------------------
	// Light Model - two-sided lighting
	bool m_bLightModel2Sided;

	// --------------------------------------------------------------------------------------------
	// OpenGL lights
	vector<COpenGLLight> m_vecOGLLights;

	// --------------------------------------------------------------------------------------------
	// Rotation - X
	float m_fXAngle;

	// --------------------------------------------------------------------------------------------
	// Rotation - Y
	float m_fYAngle;

	// --------------------------------------------------------------------------------------------
	// Translation - X
	float m_fXTranslation;

	// --------------------------------------------------------------------------------------------
	// Translation - Y
	float m_fYTranslation;

	// --------------------------------------------------------------------------------------------
	// Translation - Z
	float m_fZTranslation;

	// --------------------------------------------------------------------------------------------
	// Mouse position
	CPoint m_ptStartMousePosition;

	// --------------------------------------------------------------------------------------------
	// Mouse position
	CPoint m_ptPrevMousePosition;

	// --------------------------------------------------------------------------------------------
	// Selection support
	GLuint m_iInstanceSelectionFrameBuffer;

	// --------------------------------------------------------------------------------------------
	// Selection support
	GLuint m_iInstanceSelectionTextureBuffer;

	// --------------------------------------------------------------------------------------------
	// Selection support
	GLuint m_iInstanceSelectionDepthRenderBuffer;

	// --------------------------------------------------------------------------------------------
	// Selection support - (Instance : Color)
	map<int64_t, CSTEPColor> m_mapInstancesSelectionColors;

	// --------------------------------------------------------------------------------------------
	// Pointed instance (mouse move)
	CProductInstance* m_pPointedInstance;

	// --------------------------------------------------------------------------------------------
	// Selected instance (mouse click)
	CProductInstance* m_pSelectedInstance;

	// --------------------------------------------------------------------------------------------
	// Selected point (mouse click) in window coordinates
	CPoint m_ptSelectedPoint;

	// --------------------------------------------------------------------------------------------
	// Selected point (mouse click) in OpenGL coordinates
	GLfloat m_arSelectedPoint[3];

	// --------------------------------------------------------------------------------------------
	// Camera/eye
	GLfloat m_arCamera[3];

	// --------------------------------------------------------------------------------------------
	// Selection support
	GLuint m_iFaceSelectionFrameBuffer;

	// --------------------------------------------------------------------------------------------
	// Selection support
	GLuint m_iFaceSelectionTextureBuffer;

	// --------------------------------------------------------------------------------------------
	// Selection support
	GLuint m_iFaceSelectionDepthRenderBuffer;

	// --------------------------------------------------------------------------------------------
	// Selection support - (Face index : Color) for m_pSelectedInstance
	//map<int64_t, CSTEPColor> m_mapFacesSelectionColors;

	// --------------------------------------------------------------------------------------------
	// Pointed face (mouse move) for m_pSelectedInstance
	int64_t m_iPointedFace;

	// --------------------------------------------------------------------------------------------
	// VBOs
	vector<CSTEPDrawMetaData*> m_veCSTEPDrawMetaData;

	// --------------------------------------------------------------------------------------------
	// Shared IBOs
	vector<GLuint> m_vecIBOs;

	// --------------------------------------------------------------------------------------------
	// Selected instances
	CSTEPMaterial* m_pSelectedInstanceMaterial;

	// --------------------------------------------------------------------------------------------
	// Pointed instances
	CSTEPMaterial* m_pPointedInstanceMaterial;

	// --------------------------------------------------------------------------------------------
	// Handle
	HFONT m_hFont;

public: // Methods

	// --------------------------------------------------------------------------------------------
	// ctor
#ifdef _LINUX
    COpenGLSTEPView(wxGLCanvas * pWnd);
#else
	COpenGLSTEPView(CWnd * pWnd);
#endif //_LINUX

	// --------------------------------------------------------------------------------------------
	// dtor
	virtual ~COpenGLSTEPView();

	// --------------------------------------------------------------------------------------------
	// Setter
	void ShowFaces(BOOL bShow);

	// --------------------------------------------------------------------------------------------
	// Getter
	BOOL AreFacesShown() const;

	// --------------------------------------------------------------------------------------------
	// Setter
	void ShowFacesPolygons(BOOL bShow);

	// --------------------------------------------------------------------------------------------
	// Getter
	BOOL AreFacesPolygonsShown() const;

	// --------------------------------------------------------------------------------------------
	// Setter
	void ShowConceptualFacesPolygons(BOOL bShow);

	// --------------------------------------------------------------------------------------------
	// Getter
	BOOL AreConceptualFacesPolygonsShown() const;

	// --------------------------------------------------------------------------------------------
	// Setter
	void ShowLines(BOOL bShow);

	// --------------------------------------------------------------------------------------------
	// Getter
	BOOL AreLinesShown() const;

	// --------------------------------------------------------------------------------------------
	// Setter
	void SetLineWidth(GLfloat fWidth);

	// --------------------------------------------------------------------------------------------
	// Getter
	GLfloat GetLineWidth() const;

	// --------------------------------------------------------------------------------------------
	// Setter
	void ShowPoints(BOOL bShow);

	// --------------------------------------------------------------------------------------------
	// Getter
	BOOL ArePointsShown() const;

	// --------------------------------------------------------------------------------------------
	// Setter
	void SetPointSize(GLfloat fSize);

	// --------------------------------------------------------------------------------------------
	// Getter
	GLfloat GetPointSize() const;

	// --------------------------------------------------------------------------------------------
	// Setter
	void ShowBoundingBoxes(BOOL bShow);

	// --------------------------------------------------------------------------------------------
	// Getter
	BOOL AreBoundingBoxesShown() const;

	// --------------------------------------------------------------------------------------------
	// Setter
	void ShowNormalVectors(BOOL bShow);

	// --------------------------------------------------------------------------------------------
	// Getter
	BOOL AreNormalVectorsShown() const;

	// --------------------------------------------------------------------------------------------
	// Setter
	void ShowTangentVectors(BOOL bShow);

	// --------------------------------------------------------------------------------------------
	// Getter
	BOOL AreTangentVectorsShown() const;

	// --------------------------------------------------------------------------------------------
	// Setter
	void ShowBiNormalVectors(BOOL bShow);

	// --------------------------------------------------------------------------------------------
	// Getter
	BOOL AreBiNormalVectorsShown() const;

	// --------------------------------------------------------------------------------------------
	// Setter
	void ScaleVectors(BOOL bShow);

	// --------------------------------------------------------------------------------------------
	// Getter
	BOOL AreVectorsScaled() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	GLfloat * GetLightModelAmbient() const;

	// --------------------------------------------------------------------------------------------
	// Setter
	void SetLightModelAmbient(GLfloat fR, GLfloat fG, GLfloat fB, GLfloat fW = 1.f);

	// --------------------------------------------------------------------------------------------
	// Getter
	bool GetLightModelLocalViewer() const;

	// --------------------------------------------------------------------------------------------
	// Setter
	void SetLightModelLocalViewer(bool bLocalViewer);

	// --------------------------------------------------------------------------------------------
	// Getter
	bool GetLightModel2Sided() const;

	// --------------------------------------------------------------------------------------------
	// Setter
	void SetLightModel2Sided(bool b2Sided);

	// --------------------------------------------------------------------------------------------
	// Getter
	const vector<COpenGLLight> & GetOGLLights() const;

	// --------------------------------------------------------------------------------------------
	// Setter
	void SetOGLLight(int iLight, const COpenGLLight & light);

	// --------------------------------------------------------------------------------------------
	// COpenGLView
	virtual void Load();

	// --------------------------------------------------------------------------------------------
	// COpenGLView
#ifdef _LINUX
    void Draw(wxPaintDC * pDC);
#else
	virtual void Draw(CDC * pDC);
#endif // _LINUX

	// --------------------------------------------------------------------------------------------
	// COpenGLView
	virtual void OnMouseEvent(enumMouseEvent enEvent, UINT nFlags, CPoint point);

	// --------------------------------------------------------------------------------------------
	// COpenGLView
	virtual void OnContextMenu(CWnd* pWnd, CPoint point);	

	// --------------------------------------------------------------------------------------------
	// CSTEPView
	virtual void OnWorldDimensionsChanged();	

	// --------------------------------------------------------------------------------------------
	// CSTEPView
	virtual void Reset();

	// --------------------------------------------------------------------------------------------
	// CSTEPView
	virtual void OnInstanceSelected(CSTEPView* pSender);

	// --------------------------------------------------------------------------------------------
	// CSTEPView
	virtual void OnInstancePropertySelected();

	// --------------------------------------------------------------------------------------------
	// CSTEPView
	virtual void OnInstancesEnabledStateChanged(CSTEPView* pSender);

	// --------------------------------------------------------------------------------------------
	// CSTEPView
	virtual void OnInstanceEnabledStateChanged(CSTEPView* pSender, CProductInstance* pProductInstance);

	// --------------------------------------------------------------------------------------------
	// CSTEPView
	virtual void OnDisableAllButThis(CSTEPView* pSender, CProductInstance* pProductInstance);

	// --------------------------------------------------------------------------------------------
	// CSTEPView
	virtual void OnEnableAllInstances(CSTEPView* pSender);

protected: // Methods

	// --------------------------------------------------------------------------------------------
	// Overridden
	virtual void OnControllerChanged();

private: // Methods

	// --------------------------------------------------------------------------------------------
	// Helper
	float* GetVertices(const vector<CProductDefinition*>& vecProductDefinitions, int_t& iVerticesCount);

	// --------------------------------------------------------------------------------------------
	// Helper
	unsigned int* GetMaterialsIndices(const vector<CSTEPGeometryWithMaterial*>& vecIFCMaterials, int_t& iIndicesCount);

	// --------------------------------------------------------------------------------------------
	// Helper
	unsigned int* GetLinesCohortsIndices(const vector<CLinesCohort*>& vecLinesCohorts, int_t& iIndicesCount);

	// --------------------------------------------------------------------------------------------
	// Helper
	unsigned int* GetPointsCohortsIndices(const vector<CPointsCohort*>& vecPointsCohorts, int_t& iIndicesCount);

	// --------------------------------------------------------------------------------------------
	// Helper
	unsigned int* GetWireframesCohortsIndices(const vector<CWireframesCohort*>& vecWireframesCohorts, int_t& iIndicesCount);

	// --------------------------------------------------------------------------------------------
	// Helper
	void ReleaseBuffers();

	// --------------------------------------------------------------------------------------------
	// Scene
	void DrawClipSpace();

	// --------------------------------------------------------------------------------------------
	// Scene
	void DrawCoordinateSystem();

	// --------------------------------------------------------------------------------------------
	// https://docs.microsoft.com/en-us/windows/desktop/api/wingdi/nf-wingdi-wglusefontbitmapsa
	bool DrawTextGDI(const wchar_t* szText, float fX, float fY, float fZ);

	// --------------------------------------------------------------------------------------------
	// Faces
	void DrawFaces(bool bTransparent);

	// --------------------------------------------------------------------------------------------
	// Faces polygons
	void DrawFacesPolygons();

	// --------------------------------------------------------------------------------------------
	// Conceptual faces polygons
	void DrawConceptualFacesPolygons();

	// --------------------------------------------------------------------------------------------
	// Lines
	void DrawLines();

	// --------------------------------------------------------------------------------------------
	// Points
	void DrawPoints();

	// --------------------------------------------------------------------------------------------
	// Selection support
	void DrawInstancesFrameBuffer();

	// --------------------------------------------------------------------------------------------
	// Selection support
	void DrawFacesFrameBuffer();

	// --------------------------------------------------------------------------------------------
	// Selection support
	void DrawPointedFace();

	// --------------------------------------------------------------------------------------------
	// Handler
	void OnMouseMoveEvent(UINT nFlags, CPoint point);

	// --------------------------------------------------------------------------------------------
	// Rotate
	void Rotate(float fXSpin, float fYSpin);

	// --------------------------------------------------------------------------------------------
	// Zoom
	void Zoom(float fZTranslation);

	// --------------------------------------------------------------------------------------------
	// Screen -> Open GL coordinates
	void GetOGLPos(int iX, int iY, float fDepth, GLfloat & fX, GLfloat & fY, GLfloat & fZ) const;

	// --------------------------------------------------------------------------------------------
	// Wrapper for gluProject
	void OGLProject(GLdouble dInX, GLdouble dInY, GLdouble dInZ, GLdouble & dOutX, GLdouble & dOutY, GLdouble & dOutZ) const;
};

#endif // _OPEN_GL_STEP_VIEW_H_
