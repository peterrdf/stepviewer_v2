#ifndef _OPEN_GL_CIS2_VIEW_H_
#define _OPEN_GL_CIS2_VIEW_H_

#include "CIS2Representation.h"
#include "_oglUtils.h"

// ************************************************************************************************
class CCIS2Model;

// ************************************************************************************************
// Open GL View
class CCIS2OpenGLView 
	: public _oglView
{

private: // Members

	// Mouse
	CPoint m_ptStartMousePosition;
	CPoint m_ptPrevMousePosition;

	// Selection
	_oglSelectionFramebuffer* m_pInstanceSelectionFrameBuffer;
	CCIS2Instance* m_pPointedInstance;
	CCIS2Instance* m_pSelectedInstance;

	// Materials
	_material* m_pSelectedInstanceMaterial;
	_material* m_pPointedInstanceMaterial;

public: // Methods
	
	// ctor/dtor
	CCIS2OpenGLView(CWnd * pWnd);
	virtual ~CCIS2OpenGLView();

	// _oglRendererSettings
	virtual _controller* getController() const override;

	// _oglView
	virtual void _load(_model* pModel) override;
	virtual void _draw(CDC* pDC) override;

	// _view	
	virtual void onWorldDimensionsChanged() override;
	virtual void onInstanceSelected(_view* pSender) override;
	virtual void onInstancesEnabledStateChanged(_view* pSender) override;
	virtual void onApplicationPropertyChanged(_view* pSender, enumApplicationProperty enApplicationProperty) override;

protected: // Methods

	virtual void onControllerChanged() override;

private: // Methods
	
	// UI
	void DrawFaces(_model* pM, bool bTransparent);
	void DrawConceptualFacesPolygons(_model* pM);
	void DrawLines(_model* pM);
	void DrawPoints(_model* pM);
	
	// Selection
	void DrawInstancesFrameBuffer();
	
	// Mouse
	void OnMouseMoveEvent(UINT nFlags, CPoint point);
};

#endif // _OPEN_GL_CIS2_VIEW_H_
