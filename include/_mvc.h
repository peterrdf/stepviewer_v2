#pragma once

#include "_geometry.h"
#include "_settings_storage.h"

#include <string>
using namespace std;

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

	virtual void ZoomToInstance(_instance* pInstance) PURE;
	virtual void ZoomOut() PURE;
	virtual _instance* LoadInstance(OwlInstance iInstance) PURE;

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
	virtual OwlModel getOwlModel() const PURE;
	uint64_t getVertexLength() const { return SetFormat(getOwlModel()) / sizeof(float); }

	const vector<_geometry*>& getGeometries() const { return m_vecGeometries; }
	const vector<_instance*>& getInstances() const { return m_vecInstances; }

	double getOriginalBoundingSphereDiameter() const { return m_dOriginalBoundingSphereDiameter; }
	float getBoundingSphereDiameter() const { return m_fBoundingSphereDiameter; }	
};

// ************************************************************************************************
class _controller
{

protected: // Members

	_model* m_pModel;
	_settings_storage* m_pSettingsStorage;

public: // Methods

	_controller()
		: m_pModel(nullptr)
		, m_pSettingsStorage(new _settings_storage())
	{}

	virtual ~_controller()
	{
		delete m_pSettingsStorage;
	}

	_model* getModel() const { return m_pModel; }
	_settings_storage* getSettingsStorage() const { return m_pSettingsStorage; }
};

