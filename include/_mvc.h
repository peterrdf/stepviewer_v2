#pragma once

#include "_geometry.h"
#include "_settings_storage.h"

#include <set>
#include <string>
using namespace std;

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
class _model
{

protected: // Members

	wstring m_strPath;

	vector<_geometry*> m_vecGeometries;	
	// 1...*
	vector<_instance*> m_vecInstances;
	map<int64_t, _instance*> m_mapID2Instance;

	// http://rdf.bg/gkdoc/CP64/SetVertexBufferOffset.html
	bool m_bUpdteVertexBuffers;
	double m_dOriginalBoundingSphereDiameter;	

	// World's dimensions
	float m_fXmin;
	float m_fXmax;
	float m_fYmin;
	float m_fYmax;
	float m_fZmin;
	float m_fZmax;
	float m_fBoundingSphereDiameter;

public: // Methods

	_model();
	virtual ~_model();

	template<typename T>
	T* as()
	{
		return dynamic_cast<T*>(this);
	}

	virtual _instance* loadInstance(OwlInstance /*owlInstance*/) { assert(false); return nullptr; };

	void scale();
	virtual void zoomTo(_instance* pInstance);
	virtual void zoomOut();

	void getWorldDimensions(float& fXmin, float& fXmax, float& fYmin, float& fYmax, float& fZmin, float& fZmax) const;
	_instance* getInstanceByID(int64_t iID) const;	

protected: // Methods

	virtual void clean();

public: // Properties

	virtual OwlModel getOwlInstance() const PURE;

	const wchar_t* getPath() const { return m_strPath.c_str(); }
	uint64_t getVertexLength() const { return SetFormat(getOwlInstance()) / sizeof(float); }

	const vector<_geometry*>& getGeometries() const { return m_vecGeometries; }
	const vector<_instance*>& getInstances() const { return m_vecInstances; }

	double getOriginalBoundingSphereDiameter() const { return m_dOriginalBoundingSphereDiameter; }
	float getBoundingSphereDiameter() const { return m_fBoundingSphereDiameter; }	
};

// ************************************************************************************************
class _controller;

// ************************************************************************************************
class _view
{

protected: // Members

	_controller* m_pController;

public: // Methods

	_view()
		: m_pController(nullptr)
	{
	}

	virtual ~_view()
	{
	}

	// Model
	template<class Model>
	Model* GetModel()
	{
		auto pController = GetController();
		if (pController == nullptr)
		{
			return nullptr;
		}

		return pController->GetModel()->as<Model>();
	}

	// Events	
	virtual void onModelChanged() {}
	virtual void onModelUpdated() {}
	virtual void onWorldDimensionsChanged() {}
	virtual void onShowMetaInformation() {}
	virtual void onTargetInstanceChanged(_view* pSender) {}
	virtual void onInstanceSelected(_view* pSender) {}
	virtual void onInstancesEnabledStateChanged(_view* pSender) {}
	virtual void onInstanceAttributeEdited(_view* pSender, SdaiInstance iInstance, SdaiAttr pAttribute) {}
	virtual void onViewRelations(_view* pSender, SdaiInstance iInstance) {}
	//virtual void onViewRelations(_view* pSender, CEntity* pEntity) {}
	virtual void onApplicationPropertyChanged(_view* pSender, enumApplicationProperty enApplicationProperty) {}
	virtual void onControllerChanged() {}

public: // Properties

	void SetController(_controller* pController)
	{
		ASSERT(pController != nullptr);

		m_pController = pController;

		onControllerChanged();
	}

	_controller* GetController() const { return m_pController; }	
};

// ************************************************************************************************
class _controller
{

private: // Members

protected: // Members

	_model* m_pModel;
	set<_view*> m_setViews;
	_settings_storage* m_pSettingsStorage;	

private: // Members	

	bool m_bUpdatingModel; // Updating model - disable all notifications

	// Target
	_instance* m_pTargetInstance;

	// Selection
	_instance* m_pSelectedInstance;

	// UI properties
	BOOL m_bScaleAndCenter;

public: // Methods

	_model* GetModel() const;
	void SetModel(_model* pModel);
	_instance* LoadInstance(OwlInstance iInstance);

	// Events
	void RegisterView(_view* pView);
	void UnRegisterView(_view* pView);

	const set<_view*>& GetViews();
	template <class T>
	T* GetView()
	{
		set<_view*>::const_iterator itView = m_setViews.begin();
		for (; itView != m_setViews.end(); itView++)
		{
			T* pView = dynamic_cast<T*>(*itView);
			if (pView != nullptr)
			{
				return pView;
			}
		}

		return nullptr;
	}

	// Zoom
	void ZoomToInstance();
	void ZoomOut();

	// Save
	virtual void SaveInstance() PURE;
	void SaveInstance(OwlInstance iInstance);

	// [-1, 1]
	void ScaleAndCenter();

	// Events
	void ShowMetaInformation(_instance* pInstance);
	void SetTargetInstance(_view* pSender, _instance* pInstance);
	_instance* GetTargetInstance() const;
	void SelectInstance(_view* pSender, _instance* pInstance);
	_instance* GetSelectedInstance() const;

	// UI
	BOOL GetScaleAndCenter() const;
	void SetScaleAndCenter(BOOL bScaleAndCenter);

	// Events
	void onInstancesEnabledStateChanged(_view* pSender);
	void onApplicationPropertyChanged(_view* pSender, enumApplicationProperty enApplicationProperty);
	void onViewRelations(_view* pSender, SdaiInstance iInstance);
	//void onViewRelations(_view* pSender, CEntity* pEntity);
	void onInstanceAttributeEdited(_view* pSender, SdaiInstance iInstance, SdaiAttr pAttribute);

public: // Methods

	_controller();
	virtual ~_controller();

public: // Properties

	_model* getModel() const { return m_pModel; }
	_settings_storage* getSettingsStorage() const { return m_pSettingsStorage; }
};

