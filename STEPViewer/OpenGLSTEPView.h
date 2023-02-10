#ifndef _OPEN_GL_STEP_VIEW_H_
#define _OPEN_GL_STEP_VIEW_H_

#include "_oglUtils.h"
#include "Generic.h"
#include "OpenGLView.h"
#include "ProductDefinition.h"

// ------------------------------------------------------------------------------------------------
// Open GL View
class COpenGLSTEPView
	: public _oglRenderer<CProductDefinition>
	, public COpenGLView
{

private: // Members

	// UI
	BOOL m_bShowFaces;
	BOOL m_bShowFacesPolygons;
	BOOL m_bShowConceptualFacesPolygons;
	BOOL m_bShowLines;
	GLfloat m_fLineWidth;
	BOOL m_bShowPoints;
	GLfloat m_fPointSize;

	// Mouse
	CPoint m_ptStartMousePosition;
	CPoint m_ptPrevMousePosition;

	// Selection
	_oglSelectionFramebuffer* m_pInstanceSelectionFrameBuffer;
	CProductInstance* m_pPointedInstance;
	CProductInstance* m_pSelectedInstance;

	// Materials
	_material* m_pSelectedInstanceMaterial;
	_material* m_pPointedInstanceMaterial;

	// --------------------------------------------------------------------------------------------
	// Handle
	HFONT m_hFont;

public: // Methods

	COpenGLSTEPView(CWnd* pWnd);
	virtual ~COpenGLSTEPView();
	
	void ShowFaces(BOOL bShow);
	BOOL AreFacesShown() const;
	void ShowFacesPolygons(BOOL bShow);
	BOOL AreFacesPolygonsShown() const;
	void ShowConceptualFacesPolygons(BOOL bShow);
	BOOL AreConceptualFacesPolygonsShown() const;
	void ShowLines(BOOL bShow);
	BOOL AreLinesShown() const;
	void SetLineWidth(GLfloat fWidth);
	GLfloat GetLineWidth() const;
	void ShowPoints(BOOL bShow);
	BOOL ArePointsShown() const;
	void SetPointSize(GLfloat fSize);
	GLfloat GetPointSize() const;

	virtual void Load();

	virtual void Draw(CDC* pDC);

	// COpenGLView
	virtual void OnMouseEvent(enumMouseEvent enEvent, UINT nFlags, CPoint point);
	virtual void OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	virtual void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual void SetProjection(enumProjection enProjection);
	virtual enumProjection GetProjection() const;	
	virtual void SetView(enumView enView);
	virtual void OnContextMenu(CWnd* pWnd, CPoint point);

	// CSTEPView
	virtual void OnWorldDimensionsChanged();
	virtual void Reset();
	virtual void OnInstanceSelected(CSTEPView* pSender);
	virtual void OnInstancePropertySelected();
	virtual void OnInstancesEnabledStateChanged(CSTEPView* pSender);
	virtual void OnInstanceEnabledStateChanged(CSTEPView* pSender, CProductInstance* pInstance);
	virtual void OnDisableAllButThis(CSTEPView* pSender, CProductInstance* pInstance);
	virtual void OnEnableAllInstances(CSTEPView* pSender);

protected: // Methods

	// CSTEPView
	virtual void OnControllerChanged();

private: // Methods
	
	void DrawFaces(bool bTransparent);
	void DrawConceptualFacesPolygons();
	void DrawLines();
	void DrawPoints();
	void DrawInstancesFrameBuffer();

	void OnMouseMoveEvent(UINT nFlags, CPoint point);

	void Rotate(float fXAngle, float fYAngle);
	void Zoom(float fZTranslation);
};

#endif // _OPEN_GL_STEP_VIEW_H_
