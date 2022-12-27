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
	virtual void OnContextMenu(CWnd* pWnd, CPoint point);

	// CSTEPView
	virtual void OnWorldDimensionsChanged();
	virtual void Reset();
	virtual void OnInstanceSelected(CSTEPView* pSender);
	virtual void OnInstancePropertySelected();
	virtual void OnInstancesEnabledStateChanged(CSTEPView* pSender);
	virtual void OnInstanceEnabledStateChanged(CSTEPView* pSender, CProductInstance* pProductInstance);
	virtual void OnDisableAllButThis(CSTEPView* pSender, CProductInstance* pProductInstance);
	virtual void OnEnableAllInstances(CSTEPView* pSender);

protected: // Methods

	// CSTEPView
	virtual void OnControllerChanged();

private: // Methods

	// Scene
	void DrawClipSpace();
	void DrawCoordinateSystem();

	// --------------------------------------------------------------------------------------------
	// https://docs.microsoft.com/en-us/windows/desktop/api/wingdi/nf-wingdi-wglusefontbitmapsa
	bool DrawTextGDI(const wchar_t* szText, float fX, float fY, float fZ);
	
	void DrawFaces(bool bTransparent);
	void DrawConceptualFacesPolygons();
	void DrawLines();
	void DrawPoints();
	void DrawInstancesFrameBuffer();

	void OnMouseMoveEvent(UINT nFlags, CPoint point);

	void Rotate(float fXSpin, float fYSpin);
	void Zoom(float fZTranslation);
};

#endif // _OPEN_GL_STEP_VIEW_H_
