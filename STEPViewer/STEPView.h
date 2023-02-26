#pragma once

#include "ProductInstance.h"

// ------------------------------------------------------------------------------------------------
class CSTEPController;

// ------------------------------------------------------------------------------------------------
enum class enumApplicationProperty
{
	Projection,
	View,
	ShowFaces,
	ShowConceptualFacesWireframes,
	ShowLines,
	ShowPoints,
	PointLightingLocation,
	AmbientLightWeighting,
	SpecularLightWeighting,
	DiffuseLightWeighting,
	MaterialShininess,
	Contrast,
	Brightness,
	Gamma,
};

// ------------------------------------------------------------------------------------------------
// View - MVC
class CSTEPView
{

private: // Members
	
	// Controller
	CSTEPController* m_pController;

public: // Methods
	
	// ctor/dtor
	CSTEPView();
	virtual ~CSTEPView();
	
	// Controller
	void SetController(CSTEPController* pController);

	// Events
	virtual void OnModelChanged();
	virtual void OnWorldDimensionsChanged();	
	virtual void OnShowMetaInformation();
	virtual void OnInstanceSelected(CSTEPView* pSender);
	virtual void OnInstancePropertySelected();
	virtual void OnInstancesEnabledStateChanged(CSTEPView* pSender);
	virtual void OnApplicationPropertyChanged(CSTEPView* pSender, enumApplicationProperty enApplicationProperty);

protected: // Methods

	// Events
	virtual void OnControllerChanged();
	
	// Controller
	CSTEPController* GetController() const;	
};

