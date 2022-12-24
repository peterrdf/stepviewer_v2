#ifndef _OPEN_GL_STEP_VIEW_H_
#define _OPEN_GL_STEP_VIEW_H_

#include "_oglUtils.h"
#include "Generic.h"
#include "OpenGLView.h"
#include "ProductDefinition.h"

#ifdef _LINUX
#include <wx/wx.h>
#include <wx/glcanvas.h>
#endif // _LINUX

#ifdef _LINUX
#define MK_LBUTTON 1
#define MK_MBUTTON 2
#define MK_RBUTTON 4
#endif

// ------------------------------------------------------------------------------------------------
// Open GL View
class COpenGLSTEPView
	: public _oglRenderer<CProductDefinition>
	, public COpenGLView
{

private: // Members
	
#ifdef _LINUX
    wxGLCanvas * m_pWnd;
#else
	CWnd * m_pWnd;
#endif // _LINUX

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
	BOOL m_bDisableSelectionBuffer;
	CProductInstance* m_pPointedInstance;
	CProductInstance* m_pSelectedInstance;

	// Materials
	_material* m_pSelectedInstanceMaterial;
	_material* m_pPointedInstanceMaterial;

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
	// Handler
	void OnMouseMoveEvent(UINT nFlags, CPoint point);

	// --------------------------------------------------------------------------------------------
	// Rotate
	void Rotate(float fXSpin, float fYSpin);

	// --------------------------------------------------------------------------------------------
	// Zoom
	void Zoom(float fZTranslation);
};

#endif // _OPEN_GL_STEP_VIEW_H_
