#ifndef _OPEN_GL_IFC_VIEW_H_
#define _OPEN_GL_IFC_VIEW_H_

#include "IFCInstance.h"
#include "_oglUtils.h"
#include "OpenGLView.h"

// ************************************************************************************************
class CIFCModel;

// ************************************************************************************************
// Open GL View
class CIFCOpenGLView 
	: public COpenGLView
{

private: // Members

	// Mouse
	CPoint m_ptStartMousePosition;
	CPoint m_ptPrevMousePosition;

public: // Methods
	
	// ctor/dtor
	CIFCOpenGLView(CWnd * pWnd);
	virtual ~CIFCOpenGLView();

	// _oglRendererSettings
	virtual _controller* getController() const override;
	virtual _model* getModel() const override;
	virtual void saveSetting(const string& strName, const string& strValue) override;
	virtual string loadSetting(const string& strName) override;

	// _oglView
	virtual bool _preDraw(_model* pModel) override;
	virtual void _postDraw(_model* pModel) override;

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
	
	// Selection
	void DrawInstancesFrameBuffer();
	
	// Mouse
	void OnMouseMoveEvent(UINT nFlags, CPoint point);
};

#endif // _OPEN_GL_IFC_VIEW_H_
