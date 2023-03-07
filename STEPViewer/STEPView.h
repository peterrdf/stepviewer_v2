#pragma once

#include "ProductInstance.h"
#include "IFCInstance.h"
#include "Entity.h"

// ------------------------------------------------------------------------------------------------
class CController;

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
	CController* m_pController;

public: // Methods
	
	// ctor/dtor
	CSTEPView();
	virtual ~CSTEPView();
	
	// Controller
	void SetController(CController* pController);

	// Events
	virtual void OnModelChanged();
	virtual void OnWorldDimensionsChanged();	
	virtual void OnShowMetaInformation();
	virtual void OnInstanceSelected(CSTEPView* pSender);
	virtual void OnInstancesEnabledStateChanged(CSTEPView* pSender);
	virtual void OnViewRelations(CSTEPView* pSender, int64_t iInstance);
	virtual void OnViewRelations(CSTEPView* pSender, CEntity* pEntity);
	virtual void OnApplicationPropertyChanged(CSTEPView* pSender, enumApplicationProperty enApplicationProperty);

protected: // Methods

	// Events
	virtual void OnControllerChanged();
	
	// Controller
	CController* GetController() const;	
};

