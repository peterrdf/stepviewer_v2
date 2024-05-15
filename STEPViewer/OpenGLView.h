#pragma once

#include "ViewBase.h"
#include "_oglUtils.h"

// ************************************************************************************************
class COpenGLView 
	: public _oglView
	, public CViewBase
{


public: // Methods
	
	// ctor/dtor
	COpenGLView();
	virtual ~COpenGLView();	
		
	// Events
	virtual void OnMouseEvent(enumMouseEvent enEvent, UINT nFlags, CPoint point) PURE;
};

