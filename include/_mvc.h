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
	map<SdaiInstance, _geometry*> m_mapGeometries;
	// 1...*
	vector<_instance*> m_vecInstances;
	map<int64_t, _instance*> m_mapID2Instance;

	// http://rdf.bg/gkdoc/CP64/SetVertexBufferOffset.html
	bool m_bUpdteVertexBuffers;
	double m_dVertexBuffersOffsetX;
	double m_dVertexBuffersOffsetY;
	double m_dVertexBuffersOffsetZ;
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

	_model()
		: m_strPath(L"")
		, m_vecGeometries()
		, m_mapGeometries()
		, m_vecInstances()
		, m_mapID2Instance()
		, m_bUpdteVertexBuffers(true)
		, m_dVertexBuffersOffsetX(0.)
		, m_dVertexBuffersOffsetY(0.)
		, m_dVertexBuffersOffsetZ(0.)
		, m_dOriginalBoundingSphereDiameter(2.)
		, m_fXmin(-1.f)
		, m_fXmax(1.f)
		, m_fYmin(-1.f)
		, m_fYmax(1.f)
		, m_fZmin(-1.f)
		, m_fZmax(1.f)
		, m_fBoundingSphereDiameter(2.f)
	{}

	virtual ~_model()
	{
		clean();
	}

	virtual OwlModel getOwlInstance() const PURE;

	void scale()
	{
		// World
		m_dOriginalBoundingSphereDiameter = 2.;
		m_fBoundingSphereDiameter = 2.f;

		// Min/Max
		m_fXmin = FLT_MAX;
		m_fXmax = -FLT_MAX;
		m_fYmin = FLT_MAX;
		m_fYmax = -FLT_MAX;
		m_fZmin = FLT_MAX;
		m_fZmax = -FLT_MAX;

		for (auto pGeometry : m_vecGeometries)
		{
			if (!pGeometry->hasGeometry())
			{
				continue;
			}

			auto itInstance = pGeometry->getInstances();
			for (auto pInstance : pGeometry->getInstances())
			{
				pGeometry->calculateMinMaxTransform(
					pInstance,
					m_fXmin, m_fXmax,
					m_fYmin, m_fYmax,
					m_fZmin, m_fZmax);
			}
		} // for (auto pGeometry : ...

		if ((m_fXmin == FLT_MAX) ||
			(m_fXmax == -FLT_MAX) ||
			(m_fYmin == FLT_MAX) ||
			(m_fYmax == -FLT_MAX) ||
			(m_fZmin == FLT_MAX) ||
			(m_fZmax == -FLT_MAX))
		{
			// TODO: new status bar for messages

			return;
		}

		// World
		m_fBoundingSphereDiameter = m_fXmax - m_fXmin;
		m_fBoundingSphereDiameter = fmax(m_fBoundingSphereDiameter, m_fYmax - m_fYmin);
		m_fBoundingSphereDiameter = fmax(m_fBoundingSphereDiameter, m_fZmax - m_fZmin);

		m_dOriginalBoundingSphereDiameter = m_fBoundingSphereDiameter;

		TRACE(L"\n*** Scale I *** => Xmin/max, Ymin/max, Zmin/max: %.16f, %.16f, %.16f, %.16f, %.16f, %.16f",
			m_fXmin,
			m_fXmax,
			m_fYmin,
			m_fYmax,
			m_fZmin,
			m_fZmax);
		TRACE(L"\n*** Scale, Bounding sphere I *** =>  %.16f", m_fBoundingSphereDiameter);

		// Scale
		for (auto pGeometry : m_vecGeometries)
		{
			if (!pGeometry->hasGeometry())
			{
				continue;
			}

			pGeometry->scale(m_fBoundingSphereDiameter / 2.f);
		}

		// Min/Max
		m_fXmin = FLT_MAX;
		m_fXmax = -FLT_MAX;
		m_fYmin = FLT_MAX;
		m_fYmax = -FLT_MAX;
		m_fZmin = FLT_MAX;
		m_fZmax = -FLT_MAX;

		for (auto pGeometry : m_vecGeometries)
		{
			if (!pGeometry->hasGeometry())
			{
				continue;
			}

			auto itInstance = pGeometry->getInstances();
			for (auto pInstance : pGeometry->getInstances())
			{
				if (!pInstance->getEnable())
				{
					continue;
				}

				pGeometry->calculateMinMaxTransform(
					pInstance,
					m_fXmin, m_fXmax,
					m_fYmin, m_fYmax,
					m_fZmin, m_fZmax);
			}
		} // for (auto pGeometry : ...

		if ((m_fXmin == FLT_MAX) ||
			(m_fXmax == -FLT_MAX) ||
			(m_fYmin == FLT_MAX) ||
			(m_fYmax == -FLT_MAX) ||
			(m_fZmin == FLT_MAX) ||
			(m_fZmax == -FLT_MAX))
		{
			// TODO: new status bar for messages

			return;
		}

		// World
		m_fBoundingSphereDiameter = m_fXmax - m_fXmin;
		m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fYmax - m_fYmin);
		m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fZmax - m_fZmin);

		TRACE(L"\n*** Scale II *** => Xmin/max, Ymin/max, Zmin/max: %.16f, %.16f, %.16f, %.16f, %.16f, %.16f",
			m_fXmin,
			m_fXmax,
			m_fYmin,
			m_fYmax,
			m_fZmin,
			m_fZmax);
		TRACE(L"\n*** Scale, Bounding sphere II *** =>  %.16f", m_fBoundingSphereDiameter);
	}

	virtual void zoomTo(_instance* pInstance)
	{
		assert(pInstance != nullptr);
		assert(pInstance->getGeometry() != nullptr);

		// World
		m_fBoundingSphereDiameter = 2.f;

		// Min/Max
		m_fXmin = FLT_MAX;
		m_fXmax = -FLT_MAX;
		m_fYmin = FLT_MAX;
		m_fYmax = -FLT_MAX;
		m_fZmin = FLT_MAX;
		m_fZmax = -FLT_MAX;

		pInstance->getGeometry()->calculateMinMaxTransform(
			pInstance,
			m_fXmin, m_fXmax,
			m_fYmin, m_fYmax,
			m_fZmin, m_fZmax);

		if ((m_fXmin == FLT_MAX) ||
			(m_fXmax == -FLT_MAX) ||
			(m_fYmin == FLT_MAX) ||
			(m_fYmax == -FLT_MAX) ||
			(m_fZmin == FLT_MAX) ||
			(m_fZmax == -FLT_MAX))
		{
			m_fXmin = -1.f;
			m_fXmax = 1.f;
			m_fYmin = -1.f;
			m_fYmax = 1.f;
			m_fZmin = -1.f;
			m_fZmax = 1.f;
		}

		m_fBoundingSphereDiameter = m_fXmax - m_fXmin;
		m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fYmax - m_fYmin);
		m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fZmax - m_fZmin);
	}

	virtual void zoomOut()
	{
		// World
		m_fBoundingSphereDiameter = 2.f;

		// Min/Max
		m_fXmin = FLT_MAX;
		m_fXmax = -FLT_MAX;
		m_fYmin = FLT_MAX;
		m_fYmax = -FLT_MAX;
		m_fZmin = FLT_MAX;
		m_fZmax = -FLT_MAX;

		for (auto pGeometry : m_vecGeometries)
		{
			for (auto pInstance : pGeometry->getInstances())
			{
				if (!pInstance->getEnable())
				{
					continue;
				}

				pGeometry->calculateMinMaxTransform(
					pInstance,
					m_fXmin, m_fXmax,
					m_fYmin, m_fYmax,
					m_fZmin, m_fZmax);
			}
		}

		if ((m_fXmin == FLT_MAX) ||
			(m_fXmax == -FLT_MAX) ||
			(m_fYmin == FLT_MAX) ||
			(m_fYmax == -FLT_MAX) ||
			(m_fZmin == FLT_MAX) ||
			(m_fZmax == -FLT_MAX))
		{
			m_fXmin = -1.f;
			m_fXmax = 1.f;
			m_fYmin = -1.f;
			m_fYmax = 1.f;
			m_fZmin = -1.f;
			m_fZmax = 1.f;
		}

		m_fBoundingSphereDiameter = m_fXmax - m_fXmin;
		m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fYmax - m_fYmin);
		m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fZmax - m_fZmin);
	}

	virtual _instance* LoadInstance(OwlInstance /*owlInstance*/) { assert(false); return nullptr; };

protected: // Methods

	virtual void clean()
	{
		for (auto pGeometry : m_vecGeometries)
		{
			delete pGeometry;
		}
		m_vecGeometries.clear();
		m_mapGeometries.clear();

		for (auto pInstance : m_vecInstances)
		{
			delete pInstance;
		}
		m_vecInstances.clear();
		m_mapID2Instance.clear();
	}

public: // Methods
	 
	_geometry* geGeometryByInstance(SdaiInstance sdaiInstance)
	{
		auto itGeometry = m_mapGeometries.find(sdaiInstance);
		if (itGeometry != m_mapGeometries.end())
		{
			return itGeometry->second;
		}

		return nullptr;
	}

	_instance* getInstanceByID(int64_t iID) const
	{
		auto itInstance = m_mapID2Instance.find(iID);
		if (itInstance == m_mapID2Instance.end())
		{
			return nullptr;
		}

		return itInstance->second;
	}

	void getVertexBuffersOffset(double& dVertexBuffersOffsetX, double& dVertexBuffersOffsetY, double& dVertexBuffersOffsetZ) const
	{
		dVertexBuffersOffsetX = m_dVertexBuffersOffsetX;
		dVertexBuffersOffsetY = m_dVertexBuffersOffsetY;
		dVertexBuffersOffsetZ = m_dVertexBuffersOffsetZ;
	}

	void getWorldDimensions(float& fXmin, float& fXmax, float& fYmin, float& fYmax, float& fZmin, float& fZmax) const
	{
		fXmin = m_fXmin;
		fXmax = m_fXmax;
		fYmin = m_fYmin;
		fYmax = m_fYmax;
		fZmin = m_fZmin;
		fZmax = m_fZmax;
	}

	template<typename T>
	T* as()
	{
		return dynamic_cast<T*>(this);
	}

public: // Properties

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

	virtual void OnModelChanged() {}
	virtual void OnModelUpdated() {}
	virtual void OnWorldDimensionsChanged() {}
	virtual void OnShowMetaInformation() {}

	// Controller
	void SetController(_controller* pController);

	// Events

	virtual void OnTargetInstanceChanged(_view* pSender);
	virtual void OnInstanceSelected(_view* pSender);
	virtual void OnInstancesEnabledStateChanged(_view* pSender);
	virtual void OnInstanceAttributeEdited(_view* pSender, SdaiInstance iInstance, SdaiAttr pAttribute);
	virtual void OnViewRelations(_view* pSender, SdaiInstance iInstance);
	//virtual void OnViewRelations(_view* pSender, CEntity* pEntity);
	virtual void OnApplicationPropertyChanged(_view* pSender, enumApplicationProperty enApplicationProperty);

protected: // Methods

	// Events
	virtual void OnControllerChanged();

	// Controller
	_controller* GetController() const;

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
	void OnInstancesEnabledStateChanged(_view* pSender);
	void OnApplicationPropertyChanged(_view* pSender, enumApplicationProperty enApplicationProperty);
	void OnViewRelations(_view* pSender, SdaiInstance iInstance);
	//void OnViewRelations(_view* pSender, CEntity* pEntity);
	void OnInstanceAttributeEdited(_view* pSender, SdaiInstance iInstance, SdaiAttr pAttribute);

public: // Methods

	_controller();
	virtual ~_controller();

public: // Properties

	_model* getModel() const { return m_pModel; }
	_settings_storage* getSettingsStorage() const { return m_pSettingsStorage; }
};

