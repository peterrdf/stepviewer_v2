#ifndef _OPEN_GL_STEP_VIEW_H_
#define _OPEN_GL_STEP_VIEW_H_

#include "_oglUtils.h"

// ************************************************************************************************
class CAP242OpenGLView : public _oglView
{

public: // Methods

	CAP242OpenGLView(CWnd* pWnd);
	virtual ~CAP242OpenGLView();

protected: // Methods

	// _oglRenderer
	virtual void _reset() override;
};

#endif // _OPEN_GL_STEP_VIEW_H_
