#pragma once

#include "engine.h"
#include "_settings_storage.h"

#include <string>
using namespace std;

// ************************************************************************************************
class _model
{

protected: // Members

	wstring m_strPath;
	SdaiModel m_iSdaiModel;
	OwlModel m_iOwlModel;

public: // Methods

	_model()
		: m_strPath(L"")
		, m_iSdaiModel(0)
		, m_iOwlModel(0)
	{}

	virtual ~_model()
	{}

	const wchar_t* getPath() const { return m_strPath.c_str(); }
	virtual OwlModel getOwlModel() const { return m_iOwlModel; }
	uint64_t getVertexLength() const { return SetFormat(getOwlModel()) / sizeof(float); }
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

