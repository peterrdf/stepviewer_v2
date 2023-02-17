#ifndef _OPEN_GL_STEP_VIEW_H_
#define _OPEN_GL_STEP_VIEW_H_

#include "_oglUtils.h"
#include "Generic.h"
#include "OpenGLView.h"
#include "ProductDefinition.h"

// ------------------------------------------------------------------------------------------------
// Open GL View
class COpenGLSTEPView
	: public COpenGLView
	, public _oglRenderer<CProductDefinition>
{

private: // Members	

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

	// ctor/dtor
	COpenGLSTEPView(CWnd* pWnd);
	virtual ~COpenGLSTEPView();

	// CSTEPView	
	virtual void Redraw() override { _redraw(); }

	// CSTEPView
	virtual void OnWorldDimensionsChanged();
	virtual void OnInstanceSelected(CSTEPView* pSender);
	virtual void OnInstancePropertySelected();
	virtual void OnInstancesEnabledStateChanged(CSTEPView* pSender);
	virtual void OnInstanceEnabledStateChanged(CSTEPView* pSender, CProductInstance* pInstance);
	virtual void OnDisableAllButThis(CSTEPView* pSender, CProductInstance* pInstance);
	virtual void OnEnableAllInstances(CSTEPView* pSender);

protected: // Methods

	// CSTEPView
	virtual void OnControllerChanged();

public: // Methods

	// COpenGLView
	virtual void Load();	

	// COpenGLView
	virtual void SetProjection(enumProjection enProjection);
	virtual enumProjection GetProjection() const;
	virtual void SetView(enumView enView);

	// COpenGLView
	virtual void OnMouseEvent(enumMouseEvent enEvent, UINT nFlags, CPoint point);
	virtual void OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	virtual void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);	

	// COpenGLView
	virtual void Draw(CDC* pDC);

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

#endif // _OPEN_GL_STEP_VIEW_H_
