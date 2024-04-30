#pragma once

#include "ProductInstance.h"
#include "IFCInstance.h"
#include "Entity.h"

// ------------------------------------------------------------------------------------------------
class CController;
class CModel;

// ------------------------------------------------------------------------------------------------
enum class enumApplicationProperty : int
{
	Projection,
	View,
	ShowFaces,
	ShowConceptualFacesWireframes,
	ShowLines,
	ShowPoints,
	RotationMode,
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
	virtual void OnViewRelations(CViewBase* pSender, SdaiInstance iInstance);
	virtual void OnViewRelations(CViewBase* pSender, CEntity* pEntity);
	virtual void OnApplicationPropertyChanged(CViewBase* pSender, enumApplicationProperty enApplicationProperty);

protected: // Methods

	// Events
	virtual void OnControllerChanged();
	
	// Controller
	CController* GetController() const;	

	// Model
	template<class Model>
	Model* GetModel()
	{
		auto pController = GetController();
		if (pController == nullptr)
		{
			return nullptr;
		}

		return pController->GetModel()->As<Model>();
	}
};

