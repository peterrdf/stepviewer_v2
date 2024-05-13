#pragma once

#include "ViewBase.h"
#include "_oglUtils.h"

// ************************************************************************************************
class COpenGLView 
	: public CViewBase
{


public: // Methods
	
	// ctor/dtor
	COpenGLView();
	virtual ~COpenGLView();

	// Load
	virtual void Load() PURE;

	// Projection/View...
	virtual void SetProjection(enumProjection enProjection) PURE;
	virtual enumProjection GetProjection() const PURE;
	virtual void SetView(enumView enView) PURE;
	virtual enumRotationMode GetRotationMode() const PURE;
	virtual void SetRotationMode(enumRotationMode enRotationMode) PURE;

	
	// Events
	virtual void OnMouseEvent(enumMouseEvent enEvent, UINT nFlags, CPoint point) PURE;
	virtual void OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) PURE;
	virtual void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) PURE;

	// Draw
	virtual void Draw(CDC* pDC) PURE;
};

