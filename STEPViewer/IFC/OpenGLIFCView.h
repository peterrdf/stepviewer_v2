#ifndef _OPEN_GL_IFC_VIEW_H_
#define _OPEN_GL_IFC_VIEW_H_

#include "IFCInstance.h"
#include "_oglUtils.h"
#include "OpenGLView.h"

// ************************************************************************************************
class CIFCModel;

// ************************************************************************************************
// Open GL View
class COpenGLIFCView 
	: public COpenGLView
{

private: // Members

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

	// _oglRendererSettings
	virtual _controller* getController() const override;
	virtual _model* getModel() const override;
	virtual void saveSetting(const string& strName, const string& strValue) override;
	virtual string loadSetting(const string& strName) override;

	// _oglView
	virtual void _load() override;
	virtual void _draw(CDC* pDC) override;

	// CViewBase	
	virtual void OnWorldDimensionsChanged() override;
	virtual void OnInstanceSelected(CViewBase* pSender) override;
	virtual void OnInstancesEnabledStateChanged(CViewBase* pSender) override;
	virtual void OnApplicationPropertyChanged(CViewBase* pSender, enumApplicationProperty enApplicationProperty) override;

protected: // Methods

	virtual void OnControllerChanged() override;

public: // Methods

	// COpenGLView
	virtual void OnMouseEvent(enumMouseEvent enEvent, UINT nFlags, CPoint point) override;

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

#endif // _OPEN_GL_IFC_VIEW_H_
