#ifndef _OPEN_GL_IFC_VIEW_H_
#define _OPEN_GL_IFC_VIEW_H_

#include "Generic.h"
#include "IFCObject.h"
#include "_oglUtils.h"
#include "OpenGLView.h"

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
	CIFCObject* GetPickedIFCObject() const;

	// --------------------------------------------------------------------------------------------
	void ZoomToExtent(CIFCObject* pIFCObject);

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
};

#endif // _OPEN_GL_IFC_VIEW_H_
