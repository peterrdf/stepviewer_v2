#ifndef _OPEN_GL_IFC_VIEW_H_
#define _OPEN_GL_IFC_VIEW_H_

#include "STEPView.h"
#include "IFCObject.h"
#include "OpenGLView.h"

#include "vec3.hpp"
#include "vec4.hpp"
#include "mat4x4.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

#define _USE_MATH_DEFINES
#include <math.h>


#include <set>

using namespace std;

#include "Point3D.h"

// ------------------------------------------------------------------------------------------------
// One instance per model
class CIFCDrawMetaData
{

private: // Members

	// --------------------------------------------------------------------------------------------
	// VBO : IFCObject-s
	map<GLuint, vector<CIFCObject *>> m_mapVBO2IFCObjects;

public: // Methods

	// --------------------------------------------------------------------------------------------
	// ctor
	CIFCDrawMetaData()
	{
	}

	// --------------------------------------------------------------------------------------------
	// dtor
	~CIFCDrawMetaData()
	{
		map<GLuint, vector<CIFCObject *>>::iterator itVBO2IFCObjects = m_mapVBO2IFCObjects.begin();
		for (; itVBO2IFCObjects != m_mapVBO2IFCObjects.end(); itVBO2IFCObjects++)
		{
			glDeleteBuffers(1, &(itVBO2IFCObjects->first));
		}
	}

	// --------------------------------------------------------------------------------------------
	// Adds a group of IFCObject-s that share VBO
	void AddGroup(GLuint iVBO, const vector<CIFCObject *>& vecIFCObjects)
	{
		map<GLuint, vector<CIFCObject *>>::iterator itVBO2IFCObjects = m_mapVBO2IFCObjects.find(iVBO);
		ASSERT(itVBO2IFCObjects == m_mapVBO2IFCObjects.end());

		m_mapVBO2IFCObjects[iVBO] = vecIFCObjects;
	}

	// --------------------------------------------------------------------------------------------
	// Getter
	const map<GLuint, vector<CIFCObject *>> & getGroups() const
	{
		return m_mapVBO2IFCObjects;
	}
};

// ------------------------------------------------------------------------------------------------
class CIFCModel;

// ------------------------------------------------------------------------------------------------
enum class enumProjection
{
	Perspective = 0,
	Isometric = 1,
};

// ------------------------------------------------------------------------------------------------
// Open GL View
class COpenGLIFCView : public COpenGLView
{

private: // Members

	// --------------------------------------------------------------------------------------------
	// Window
	CWnd * m_pWnd;

	glm::mat4 m_modelViewMatrix;

	// --------------------------------------------------------------------------------------------
	// Projection
	enumProjection m_enProjection;

	// --------------------------------------------------------------------------------------------
	// CIFCDrawMetaData-s
	vector<CIFCDrawMetaData *> m_vecIFCDrawMetaData;

	// --------------------------------------------------------------------------------------------
	// Shared IBOs
	vector<GLuint> m_vecIBOs;

	// --------------------------------------------------------------------------------------------
	// Faces
	BOOL m_bShowFaces;

	// --------------------------------------------------------------------------------------------
	// Lines
	BOOL m_bShowLines;

	// --------------------------------------------------------------------------------------------
	// Wireframes
	BOOL m_bShowWireframes;

	// --------------------------------------------------------------------------------------------
	// Rotation - X; radians
	double m_dXAngle;

	// --------------------------------------------------------------------------------------------
	// Rotation - Y; radians
	double m_dYAngle;

	// --------------------------------------------------------------------------------------------
	// Translation - X
	double m_dXTranslation;

	// --------------------------------------------------------------------------------------------
	// Translation - Y
	double m_dYTranslation;

	// --------------------------------------------------------------------------------------------
	// Translation - Z
	double m_dZTranslation;

	// --------------------------------------------------------------------------------------------
	// Scale
	double m_dScaleFactor;

	// --------------------------------------------------------------------------------------------
	// Mouse position
	CPoint m_ptPrevMousePosition;	

	// --------------------------------------------------------------------------------------------
	// Interaction 
	bool m_bInteractionInProgress;

	// --------------------------------------------------------------------------------------------
	// Selection support
	GLuint m_iSelectionFrameBuffer;

	// --------------------------------------------------------------------------------------------
	// Selection support
	GLuint m_iSelectionTextureBuffer;

	// --------------------------------------------------------------------------------------------
	// Selection support
	GLuint m_iSelectionDepthRenderBuffer;

	// --------------------------------------------------------------------------------------------
	// Measures support
	GLuint m_iWireframesFrameBuffer;

	// --------------------------------------------------------------------------------------------
	// Measures support
	GLuint m_iWireframesTextureBuffer;

	// --------------------------------------------------------------------------------------------
	// Measures support
	GLuint m_iWireframesDepthRenderBuffer;

	// --------------------------------------------------------------------------------------------
	// Measures support
	GLuint m_iFacesFrameBuffer;

	// --------------------------------------------------------------------------------------------
	// Measures support
	GLuint m_iFacesTextureBuffer;

	// --------------------------------------------------------------------------------------------
	// Measures support
	GLuint m_iFacesDepthRenderBuffer;

	// --------------------------------------------------------------------------------------------
	// Picked IFC object
	CIFCObject * m_pPickedIFCObject;

	// --------------------------------------------------------------------------------------------
	// Picked IFC object - Model
	CIFCModel * m_pPickedIFCObjectModel;

	// --------------------------------------------------------------------------------------------
	// Picked IFC object - Edges
	set<int_t> m_setPickedIFCObjectEdges;

	// --------------------------------------------------------------------------------------------
	// Picked IFC object - Conceptual Face
	int_t m_iPickedIFCObjectFace;

	// --------------------------------------------------------------------------------------------
	// Picked IFC object - Point
	CPoint3D m_pickedPoint3D;

	// --------------------------------------------------------------------------------------------
	// Picked IFC object - Point
	CPoint3D m_selectedPoint3D;

	// --------------------------------------------------------------------------------------------
	// View Origin	
	CPoint3D m_viewOriginPoint3D;

	// --------------------------------------------------------------------------------------------
	// View Origin	
	CPoint3D m_XArrowVector;

	// --------------------------------------------------------------------------------------------
	// View Origin	
	CPoint3D m_YArrowVector;

	// --------------------------------------------------------------------------------------------
	// View Origin	
	CPoint3D m_ZArrowVector;

public: // Methods

	// --------------------------------------------------------------------------------------------
	// ctor
	COpenGLIFCView(CWnd * pWnd);

	// --------------------------------------------------------------------------------------------
	// dtor
	virtual ~COpenGLIFCView();

	// --------------------------------------------------------------------------------------------
	// Projection
	enumProjection GetProjection() const;

	// --------------------------------------------------------------------------------------------
	// Projection
	void SetProjection(enumProjection enProjection);

	// --------------------------------------------------------------------------------------------
	// Faces
	void ShowFaces(BOOL bShow);

	// --------------------------------------------------------------------------------------------
	// Faces 
	BOOL AreFacesShown();

	// --------------------------------------------------------------------------------------------
	// Lines
	void ShowLines(BOOL bShow);

	// --------------------------------------------------------------------------------------------
	// Lines 
	BOOL AreLinesShown();

	// --------------------------------------------------------------------------------------------
	// Wireframes
	void ShowWireframes(BOOL bShow);

	// --------------------------------------------------------------------------------------------
	// Wireframes 
	BOOL AreWireframesShown();

	// --------------------------------------------------------------------------------------------
	// COpenGLView
	virtual void Load();

	// --------------------------------------------------------------------------------------------
	// COpenGLView
	virtual void Draw(CDC * pDC);

	// --------------------------------------------------------------------------------------------
	// COpenGLView
	void OnMouseEvent(enumMouseEvent enEvent, UINT nFlags, CPoint point);

	// --------------------------------------------------------------------------------------------
	// CSTEPView
	virtual void OnInstancesEnabledStateChanged(CSTEPView* pSender);

	// --------------------------------------------------------------------------------------------
	// CSTEPView
	virtual void OnInstanceSelected(CSTEPView* pSender);

	// --------------------------------------------------------------------------------------------
	// Handles the events
	void OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

	// --------------------------------------------------------------------------------------------
	// CSTEPView
	virtual void OnControllerChanged();

	// --------------------------------------------------------------------------------------------
	void SetOrientation(LONG iOrientation);

	// TEMP?
	double m_dOriginX;
	double m_dOriginY;
	double m_dOriginZ;

	// ------------------------------------------------------------------------------------------------
	void ZoomToSet(double minX, double maxX, double minY, double maxY, double minZ, double maxZ);

	// --------------------------------------------------------------------------------------------
	void SetQuatRotation(DOUBLE dW, DOUBLE dXRadians, DOUBLE dYRadians, DOUBLE dZRadians);

	// --------------------------------------------------------------------------------------------
	void GetQuatRotation(DOUBLE* pdW, DOUBLE* pdXRadians, DOUBLE* pdYRadians, DOUBLE* pdZRadians);

	// --------------------------------------------------------------------------------------------
	void SetRotation(DOUBLE dXDegrees, DOUBLE dYDegrees, DOUBLE dZDegrees);

	// --------------------------------------------------------------------------------------------
	void SetCoordinateSize(DOUBLE arrowLengthI, DOUBLE arrowLengthII);

	// --------------------------------------------------------------------------------------------
	void GetRotation(DOUBLE* pdXDegrees, DOUBLE* pdYDegrees, DOUBLE* pdZDegrees);

	// --------------------------------------------------------------------------------------------
	void GetTranslation(DOUBLE* pdX, DOUBLE* pdY, DOUBLE* pdZ);

	// --------------------------------------------------------------------------------------------
	void SetTranslation(DOUBLE dX, DOUBLE dY, DOUBLE dZ);

	// --------------------------------------------------------------------------------------------
	CIFCObject* GetPickedIFCObject() const;

	// --------------------------------------------------------------------------------------------
	CPoint3D& GetPickedPoint();

	// --------------------------------------------------------------------------------------------
	void ZoomToExtent(CIFCObject* pIFCObject);

	// --------------------------------------------------------------------------------------------
	void ZoomToPickedPoint();

private: // Methods

	// ------------------------------------------------------------------------------------------------
	void ResetView();

	// ------------------------------------------------------------------------------------------------
	// Faces
	void DrawFaces(bool bTransparent);

	// ------------------------------------------------------------------------------------------------
	// Lines
	void DrawLines();

	// ------------------------------------------------------------------------------------------------
	// Wireframes
	void DrawWireframes();

	// ------------------------------------------------------------------------------------------------
	// Selection support
	void DrawFacesFrameBuffer();

	// ------------------------------------------------------------------------------------------------
	// Handler
	void OnMouseMoveEvent(UINT nFlags, CPoint point);

	// ------------------------------------------------------------------------------------------------
	// Rotate
	void Rotate(double fXSpin, double fYSpin);

	// ------------------------------------------------------------------------------------------------
	// Zoom
	void Zoom(double dZTranslation);

	// --------------------------------------------------------------------------------------------
	void ZoomToCoreCalculate(CIFCObject * pIFCObject, double * pMinX, double * pMaxX, double * pMinY, double * pMaxY, double * pMinZ, double * pMaxZ, bool * pInitialized);

	// --------------------------------------------------------------------------------------------
	void COpenGLIFCView::ZoomToCoreSet(double minX, double maxX, double minY, double maxY, double minZ, double maxZ);

	// --------------------------------------------------------------------------------------------
	void CenterToCore(CIFCObject * pIFCObject);
};

#endif // _OPEN_GL_IFC_VIEW_H_
