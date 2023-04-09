#pragma once

#include "ViewBase.h"
#include "_oglUtils.h"

// ------------------------------------------------------------------------------------------------
// Mouse support
enum class enumMouseEvent : int
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
	: public CViewBase
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

