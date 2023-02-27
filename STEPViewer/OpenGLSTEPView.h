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
	virtual void OnWorldDimensionsChanged() override;
	virtual void OnInstanceSelected(CSTEPView* pSender) override;
	virtual void OnInstancesEnabledStateChanged(CSTEPView* pSender) override;
	virtual void OnApplicationPropertyChanged(CSTEPView* pSender, enumApplicationProperty enApplicationProperty) override;

protected: // Methods

	// CSTEPView
	virtual void OnControllerChanged() override;

public: // Methods

	// COpenGLView
	virtual void Load() override;

	// COpenGLView
	virtual void SetProjection(enumProjection enProjection) override;
	virtual enumProjection GetProjection() const override;
	virtual void SetView(enumView enView) override;

	// COpenGLView
	virtual void OnMouseEvent(enumMouseEvent enEvent, UINT nFlags, CPoint point) override;
	virtual void OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) override;
	virtual void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) override;

	// COpenGLView
	virtual void Draw(CDC* pDC) override;

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
};

#endif // _OPEN_GL_STEP_VIEW_H_
