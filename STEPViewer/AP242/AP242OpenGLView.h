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

protected: // Methods

	// _view
	virtual void onControllerChanged() override;
};

#endif // _OPEN_GL_STEP_VIEW_H_
