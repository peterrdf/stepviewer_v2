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
	virtual void onWorldDimensionsChanged() override;
	virtual void onInstanceSelected(_view* pSender) override;
	virtual void onInstancesEnabledStateChanged(_view* pSender) override;
	virtual void onApplicationPropertyChanged(_view* pSender, enumApplicationProperty enApplicationProperty) override;

protected: // Methods

	virtual void onControllerChanged() override;
};

#endif // _OPEN_GL_IFC_VIEW_H_
