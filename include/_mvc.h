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
	OwlModel m_iModel;

public: // Methods

	_model()
		: m_strPath(L"")
		, m_iModel(0)
	{}

	virtual ~_model()
	{}

	const wchar_t* getPath() const { return m_strPath.c_str(); }
	virtual OwlModel getInstance() const { return m_iModel; }
	uint64_t getVertexLength() const { return SetFormat(getInstance()) / sizeof(float); }
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

