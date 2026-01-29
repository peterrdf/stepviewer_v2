#ifndef _OPEN_GL_IFC_VIEW_H_
#define _OPEN_GL_IFC_VIEW_H_

#include "_oglUtils.h"

// ************************************************************************************************
// Open GL View
class CIFCOpenGLView : public _oglView
{

public: // Methods
	
	// ctor/dtor
	CIFCOpenGLView(CWnd * pWnd);
	virtual ~CIFCOpenGLView();


protected: // Methods

	// _oglView
	virtual void _postDraw() override;

	void DrawBoundingBoxes(_model* pModel);
};

#endif // _OPEN_GL_IFC_VIEW_H_
