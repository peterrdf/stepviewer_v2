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

	// Load
	virtual void Load() PURE;
		
	// Events
	virtual void OnMouseEvent(enumMouseEvent enEvent, UINT nFlags, CPoint point) PURE;
	
	// Draw
	virtual void Draw(CDC* pDC) PURE;
};

