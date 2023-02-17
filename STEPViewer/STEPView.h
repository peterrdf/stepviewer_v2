#pragma once

#include "ProductInstance.h"

// ------------------------------------------------------------------------------------------------
class CSTEPController;

// ------------------------------------------------------------------------------------------------
// View - MVC
class CSTEPView
{

private: // Members
	
	// Controller
	CSTEPController* m_pController;

protected: // Members

	// UI
	BOOL m_bShowFaces;
	BOOL m_bShowFacesPolygons;
	BOOL m_bShowConceptualFacesPolygons;
	BOOL m_bShowLines;
	GLfloat m_fLineWidth;
	BOOL m_bShowPoints;
	GLfloat m_fPointSize;

public: // Methods
	
	// ctor/dtor
	CSTEPView();
	virtual ~CSTEPView();
	
	// Controller
	void SetController(CSTEPController* pController);

	// UI
	void ShowFaces(BOOL bShow) { m_bShowFaces = bShow; Redraw(); }
	BOOL AreFacesShown() const { return m_bShowFaces; }
	void ShowFacesPolygons(BOOL bShow) { m_bShowFacesPolygons = bShow; Redraw(); }
	BOOL AreFacesPolygonsShown() const { return m_bShowFacesPolygons; };
	void ShowConceptualFacesPolygons(BOOL bShow) { m_bShowConceptualFacesPolygons = bShow; Redraw(); }
	BOOL AreConceptualFacesPolygonsShown() const { return m_bShowConceptualFacesPolygons; }
	void ShowLines(BOOL bShow) { m_bShowLines = bShow; Redraw(); }
	BOOL AreLinesShown() const { return m_bShowLines; }
	void SetLineWidth(GLfloat fWidth) { m_fLineWidth = fWidth; Redraw(); }
	GLfloat GetLineWidth() const { return m_fLineWidth; }
	void ShowPoints(BOOL bShow) { m_bShowPoints = bShow; Redraw(); }
	BOOL ArePointsShown() const { return m_bShowPoints; }
	void SetPointSize(GLfloat fSize) { m_fPointSize = fSize; Redraw(); }
	GLfloat GetPointSize() const { return m_fPointSize; }

	// UI
	virtual void Redraw() { ASSERT(FALSE); /*TODO*/ }

	// Events
	virtual void OnModelChanged();
	virtual void OnWorldDimensionsChanged();	
	virtual void OnShowMetaInformation();
	virtual void OnInstanceSelected(CSTEPView* pSender);
	virtual void OnInstancePropertySelected();
	virtual void OnInstancesEnabledStateChanged(CSTEPView* pSender);
	virtual void OnInstanceEnabledStateChanged(CSTEPView* pSender, CProductInstance* pInstance);
	virtual void OnDisableAllButThis(CSTEPView* pSender, CProductInstance* pInstance);
	virtual void OnEnableAllInstances(CSTEPView* pSender);

protected: // Methods

	// Events
	virtual void OnControllerChanged();
	
	// Controller
	CSTEPController* GetController() const;	
};

