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

public: // Methods

	_model()
		: m_strPath(L"")
		, m_iModel(0)
		, m_vecGeometries()
		, m_vecInstances()
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
	}

public: // Properties

	const wchar_t* getPath() const { return m_strPath.c_str(); }
	virtual OwlModel getInstance() const { return m_iModel; }
	uint64_t getVertexLength() const { return SetFormat(getInstance()) / sizeof(float); }

	const vector<_geometry*>& getGeometries() const { return m_vecGeometries; }
	const vector<_instance*>& getInstances() const { return m_vecInstances; }
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

