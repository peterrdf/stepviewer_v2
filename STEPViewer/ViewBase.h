#pragma once

#include "ProductInstance.h"
#include "IFCInstance.h"
#include "Entity.h"

// ------------------------------------------------------------------------------------------------
class CController;

// ------------------------------------------------------------------------------------------------
enum class enumApplicationProperty : int
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
class CViewBase
{

private: // Members
	
	// Controller
	CController* m_pController;

public: // Methods
	
	// ctor/dtor
	CViewBase();
	virtual ~CViewBase();
	
	// Controller
	void SetController(CController* pController);

	// Events
	virtual void OnModelChanged();
	virtual void OnWorldDimensionsChanged();	
	virtual void OnShowMetaInformation();
	virtual void OnInstanceSelected(CViewBase* pSender);
	virtual void OnInstancesEnabledStateChanged(CViewBase* pSender);
	virtual void OnViewRelations(CViewBase* pSender, int64_t iInstance);
	virtual void OnViewRelations(CViewBase* pSender, CEntity* pEntity);
	virtual void OnApplicationPropertyChanged(CViewBase* pSender, enumApplicationProperty enApplicationProperty);

protected: // Methods

	// Events
	virtual void OnControllerChanged();
	
	// Controller
	CController* GetController() const;	
};

