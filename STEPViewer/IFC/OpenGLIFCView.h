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
	: public COpenGLView
	, public _oglRenderer<CIFCInstance>
{

private: // Members

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
	
	// ctor/dtor
	COpenGLIFCView(CWnd * pWnd);
	virtual ~COpenGLIFCView();

	// CSTEPView	
	virtual void Redraw() override { _redraw();  }

	// CSTEPView	
	virtual void OnWorldDimensionsChanged();
	virtual void OnInstanceSelected(CSTEPView* pSender);
	virtual void OnInstancesEnabledStateChanged(CSTEPView* pSender);

protected: // Methods

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
};

#endif // _OPEN_GL_IFC_VIEW_H_
