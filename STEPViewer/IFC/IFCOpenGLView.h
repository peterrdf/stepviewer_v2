#ifndef _OPEN_GL_IFC_VIEW_H_
#define _OPEN_GL_IFC_VIEW_H_

#include "IFCInstance.h"
#include "_oglUtils.h"

// ************************************************************************************************
class CIFCModel;

// ************************************************************************************************
// Open GL View
class CIFCOpenGLView 
	: public _oglView
{

public: // Methods
	
	// ctor/dtor
	CIFCOpenGLView(CWnd * pWnd);
	virtual ~CIFCOpenGLView();

	// _oglRendererSettings
	virtual _controller* getController() const override;

	// _view	
	virtual void OnWorldDimensionsChanged() override;
	virtual void OnInstanceSelected(_view* pSender) override;
	virtual void OnInstancesEnabledStateChanged(_view* pSender) override;
	virtual void OnApplicationPropertyChanged(_view* pSender, enumApplicationProperty enApplicationProperty) override;

protected: // Methods

	virtual void OnControllerChanged() override;
};

#endif // _OPEN_GL_IFC_VIEW_H_
