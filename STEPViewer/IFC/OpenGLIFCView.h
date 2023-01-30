#ifndef _OPEN_GL_IFC_VIEW_H_
#define _OPEN_GL_IFC_VIEW_H_

#include "Generic.h"
#include "IFCObject.h"
#include "_oglUtils.h"
#include "OpenGLView.h"

// ------------------------------------------------------------------------------------------------
class CIFCModel;

// ------------------------------------------------------------------------------------------------
// Open GL View
class COpenGLIFCView 
	: public _oglRenderer<CIFCObject>
	, public COpenGLView
{

private: // Members	
	
	CWnd* m_pWnd;

	// Projection
	enumProjection  m_enProjection;
		
	// UI
	BOOL m_bShowFaces;	
	BOOL m_bShowConceptualFacesPolygons;
	BOOL m_bShowLines;
	BOOL m_bShowPoints;

	// --------------------------------------------------------------------------------------------
	// Mouse
	CPoint m_ptStartMousePosition;
	CPoint m_ptPrevMousePosition;

	// Selection
	_oglSelectionFramebuffer* m_pInstanceSelectionFrameBuffer;
	CIFCObject* m_pPointedInstance;
	CIFCObject* m_pSelectedInstance;

	// Materials
	_material* m_pSelectedInstanceMaterial;
	_material* m_pPointedInstanceMaterial;

public: // Methods
	
	COpenGLIFCView(CWnd * pWnd);
	virtual ~COpenGLIFCView();

	// Projection
	enumProjection GetProjection() const;
	void SetProjection(enumProjection enProjection);

	// View
	void SetView(enumView enView);

	// UI
	void ShowFaces(BOOL bShow);
	BOOL AreFacesShown();
	void ShowConceptualFacesPolygons(BOOL bShow);
	BOOL AreConceptualFacesPolygonsShown();
	void ShowLines(BOOL bShow);
	BOOL AreLinesShown();	
	void ShowPoints(BOOL bShow);
	BOOL ArePointsShown();
	
	// COpenGLView
	virtual void Load();
	virtual void Draw(CDC * pDC);
	void OnMouseEvent(enumMouseEvent enEvent, UINT nFlags, CPoint point);
	void OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	
	// CSTEPView
	virtual void OnInstancesEnabledStateChanged(CSTEPView* pSender);
	virtual void OnInstanceSelected(CSTEPView* pSender);
	virtual void OnControllerChanged();	

private: // Methods
	
	// UI
	void DrawFaces(bool bTransparent);
	void DrawConceptualFacesPolygons();
	void DrawLines();
	void DrawPoints();
	
	// Selection
	void DrawFacesFrameBuffer();
	
	// Mouse
	void OnMouseMoveEvent(UINT nFlags, CPoint point);
	void Rotate(double fXSpin, double fYSpin);
	void Zoom(double dZTranslation);
};

#endif // _OPEN_GL_IFC_VIEW_H_
