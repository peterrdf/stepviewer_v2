#ifndef _OPEN_GL_STEP_VIEW_H_
#define _OPEN_GL_STEP_VIEW_H_

#include "_oglUtils.h"
#include "AP242ProductDefinition.h"

// ************************************************************************************************
// Open GL View
class CAP242OpenGLView
	: public _oglView
{

public: // Methods

	// ctor/dtor
	CAP242OpenGLView(CWnd* pWnd);
	virtual ~CAP242OpenGLView();	

	// _oglRendererSettings
	virtual _controller* getController() const override;

	// _view
	virtual void OnWorldDimensionsChanged() override;
	virtual void OnInstanceSelected(_view* pSender) override;
	virtual void OnInstancesEnabledStateChanged(_view* pSender) override;
	virtual void OnApplicationPropertyChanged(_view* pSender, enumApplicationProperty enApplicationProperty) override;

protected: // Methods

	// _view
	virtual void OnControllerChanged() override;
};

#endif // _OPEN_GL_STEP_VIEW_H_
