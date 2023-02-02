#pragma once

#include "STEPView.h"

const double ZOOM_SPEED_1 = 0.01;
const double ZOOM_SPEED_2 = 0.025;
const double ARROW_SIZE_I = 1.;
const double ARROW_SIZE_II = 1.;

// ------------------------------------------------------------------------------------------------
const float ZOOM_SPEED_MOUSE = 0.025f;
const float ZOOM_SPEED_MOUSE_WHEEL = 0.0125f;
const float PAN_SPEED_MOUSE = 4.f;
const float PAN_SPEED_KEYS = 4.f;
const float ROTATION_SPEED = 1.f / 2500.f;
const float ROTATION_SENSITIVITY = 0.1f;

// ------------------------------------------------------------------------------------------------
// Mouse support
enum class enumMouseEvent
{
	Move = 0,
	LBtnDown = 1,
	LBtnUp = 2,
	MBtnDown = 3,
	MBtnUp = 4,
	RBtnDown = 5,
	RBtnUp = 6,
};

// ------------------------------------------------------------------------------------------------
class COpenGLView : public CSTEPView
{

public: // Methods

	// --------------------------------------------------------------------------------------------
	COpenGLView();

	// --------------------------------------------------------------------------------------------
	virtual ~COpenGLView();

	// --------------------------------------------------------------------------------------------
	virtual void Load() PURE;

	// --------------------------------------------------------------------------------------------
	virtual void Draw(CDC* pDC) PURE;

	// --------------------------------------------------------------------------------------------
	virtual void OnMouseEvent(enumMouseEvent enEvent, UINT nFlags, CPoint point) PURE;

	// --------------------------------------------------------------------------------------------
	virtual void OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) PURE;

	// --------------------------------------------------------------------------------------------
	virtual void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) PURE;

	// --------------------------------------------------------------------------------------------
	virtual void OnContextMenu(CWnd* pWnd, CPoint point);
};

