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
	OwlModel m_iModel;
	
	vector<_geometry*> m_vecGeometries;
	// 1...*
	vector<_instance*> m_vecInstances;

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
		, m_iModel(0)
		, m_vecGeometries()
		, m_vecInstances()
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

protected: // Methods

	virtual void clean()
	{
		for (auto pInstance : m_vecInstances)
		{
			delete pInstance;
		}
		m_vecInstances.clear();

		for (auto pGeometry : m_vecGeometries)
		{
			delete pGeometry;
		}
		m_vecGeometries.clear();
	}


public: // Methods

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

