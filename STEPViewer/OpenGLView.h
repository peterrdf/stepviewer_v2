#pragma once

#include "STEPView.h"
#include "_oglUtils.h"

const double ZOOM_SPEED_1 = 0.01;
const double ZOOM_SPEED_2 = 0.025;
const double ARROW_SIZE_I = 1.;
const double ARROW_SIZE_II = 1.;

// ------------------------------------------------------------------------------------------------
const float ZOOM_SPEED_MOUSE = 0.025f;
const float ZOOM_SPEED_MOUSE_WHEEL = 0.0125f;
const float ZOOM_SPEED_KEYS = 0.025f;
const float PAN_SPEED_MOUSE = 4.f;
const float PAN_SPEED_KEYS = 40.f;
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
class COpenGLView 
	: public CSTEPView
{

protected: // Members

	// UI
	BOOL m_bShowFaces;
	BOOL m_bShowConceptualFacesPolygons;
	BOOL m_bShowLines;
	GLfloat m_fLineWidth;
	BOOL m_bShowPoints;
	GLfloat m_fPointSize;

public: // Methods
	
	// ctor/dtor
	COpenGLView();
	virtual ~COpenGLView();	

	// UI
	void ShowFaces(BOOL bShow) { m_bShowFaces = bShow; }
	BOOL AreFacesShown() const { return m_bShowFaces; }
	void ShowConceptualFacesPolygons(BOOL bShow) { m_bShowConceptualFacesPolygons = bShow; }
	BOOL AreConceptualFacesPolygonsShown() const { return m_bShowConceptualFacesPolygons; }
	void ShowLines(BOOL bShow) { m_bShowLines = bShow; }
	BOOL AreLinesShown() const { return m_bShowLines; }
	void ShowPoints(BOOL bShow) { m_bShowPoints = bShow; }
	BOOL ArePointsShown() const { return m_bShowPoints; }	

	// Load
	virtual void Load() PURE;

	// Projection/View
	virtual void SetProjection(enumProjection enProjection) PURE;
	virtual enumProjection GetProjection() const PURE;
	virtual void SetView(enumView enView) PURE;
	
	// Events
	virtual void OnMouseEvent(enumMouseEvent enEvent, UINT nFlags, CPoint point) PURE;
	virtual void OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) PURE;
	virtual void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) PURE;

	// Draw
	virtual void Draw(CDC* pDC) PURE;
};

