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

	// CViewBase	
	virtual void OnWorldDimensionsChanged() override;
	virtual void OnInstanceSelected(CViewBase* pSender) override;
	virtual void OnInstancesEnabledStateChanged(CViewBase* pSender) override;
	virtual void OnApplicationPropertyChanged(CViewBase* pSender, enumApplicationProperty enApplicationProperty) override;

protected: // Methods

	virtual void OnControllerChanged() override;

public: // Methods
		
	// COpenGLView
	virtual void Load();

	// COpenGLView
	virtual void SetProjection(enumProjection enProjection) override;
	virtual enumProjection GetProjection() const override;
	virtual void SetView(enumView enView) override;
	virtual enumRotationMode GetRotationMode() const override;
	virtual void SetRotationMode(enumRotationMode enRotationMode) override;
	virtual void OnMouseEvent(enumMouseEvent enEvent, UINT nFlags, CPoint point) override;
	virtual void OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) override;
	virtual void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) override;
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

#endif // _OPEN_GL_IFC_VIEW_H_
