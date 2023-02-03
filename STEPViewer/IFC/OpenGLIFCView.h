#ifndef _OPEN_GL_IFC_VIEW_H_
#define _OPEN_GL_IFC_VIEW_H_

#include "Generic.h"
#include "IFCInstance.h"
#include "_oglUtils.h"
#include "OpenGLView.h"

// ------------------------------------------------------------------------------------------------
class CIFCModel;

// ------------------------------------------------------------------------------------------------
// Open GL View
class COpenGLIFCView 
	: public _oglRenderer<CIFCInstance>
	, public COpenGLView
{

private: // Members
		
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
	CIFCInstance* m_pPointedInstance;
	CIFCInstance* m_pSelectedInstance;

	// Materials
	_material* m_pSelectedInstanceMaterial;
	_material* m_pPointedInstanceMaterial;

public: // Methods
	
	COpenGLIFCView(CWnd * pWnd);
	virtual ~COpenGLIFCView();

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
	virtual void OnMouseEvent(enumMouseEvent enEvent, UINT nFlags, CPoint point);
	virtual void OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	virtual void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);	
	virtual void SetProjection(enumProjection enProjection);
	virtual enumProjection GetProjection() const;
	
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
	void DrawInstancesFrameBuffer();
	
	// Mouse
	void OnMouseMoveEvent(UINT nFlags, CPoint point);
	void Rotate(float fXAngle, float fYAngle);
	void Zoom(float fZTranslation);
};

#endif // _OPEN_GL_IFC_VIEW_H_
