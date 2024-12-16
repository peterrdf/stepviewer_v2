#ifndef _OPEN_GL_IFC_VIEW_H_
#define _OPEN_GL_IFC_VIEW_H_

#include "IFCInstance.h"
#include "_oglUtils.h"

// ************************************************************************************************
// Open GL View
class CIFCOpenGLView : public _oglView
{

public: // Methods
	
	// ctor/dtor
	CIFCOpenGLView(CWnd * pWnd);
	virtual ~CIFCOpenGLView();
};

#endif // _OPEN_GL_IFC_VIEW_H_
