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

public: // Methods
	
	// ctor/dtor
	CCIS2OpenGLView(CWnd * pWnd);
	virtual ~CCIS2OpenGLView();

	// _oglRendererSettings
	virtual _controller* getController() const override;
};

#endif // _OPEN_GL_CIS2_VIEW_H_
