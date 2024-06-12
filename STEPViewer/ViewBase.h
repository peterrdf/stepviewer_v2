#pragma once

#include "IFCInstance.h"
#include "Entity.h"

// ************************************************************************************************
class CController;
class CModel;

// ************************************************************************************************
enum class enumApplicationProperty : int
{
	Projection,
	View,
	ShowFaces,
	CullFaces,
	ShowFacesWireframes,
	ShowConceptualFacesWireframes,
	ShowLines,
	ShowPoints,
	ShowNormalVectors,
	ShowTangenVectors,
	ShowBiNormalVectors,
	ScaleVectors,
	ShowBoundingBoxes,
	RotationMode,
	ShowCoordinateSystem,
	CoordinateSystemType,
	ShowNavigator,
	PointLightingLocation,
	AmbientLightWeighting,
	SpecularLightWeighting,
	DiffuseLightWeighting,
	MaterialShininess,
	Contrast,
	Brightness,
	Gamma,
	VisibleValuesCountLimit,
	ScalelAndCenter,
};

// ************************************************************************************************
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
	virtual void OnModelUpdated();
	virtual void OnWorldDimensionsChanged();	
	virtual void OnShowMetaInformation();
	virtual void OnTargetInstanceChanged(CViewBase* pSender);
	virtual void OnInstanceSelected(CViewBase* pSender);
	virtual void OnInstancesEnabledStateChanged(CViewBase* pSender);
	virtual void OnInstanceAttributeEdited(CViewBase* pSender, SdaiInstance iInstance, SdaiAttr pAttribute);
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

