#pragma once

#include "_ap_mvc.h"

// ************************************************************************************************
class _ap242_draghting_model;

// ************************************************************************************************
class _ap242_model : public _ap_model
{

private: // Members

	vector<_ap242_draghting_model*> m_vecDraghtingModels;

public: // Methods

	_ap242_model();
	virtual ~_ap242_model();

protected: // Methods

	// _ap_model
	virtual void preLoadInstance(OwlInstance owlInstance) override;
	virtual void clean() override;

	void loadDraughtingModels();

public: // Properties

	const vector<_ap242_draghting_model*>& getDraghtingModels() const { return m_vecDraghtingModels; }
};

// ************************************************************************************************
class _ap242_geometry : public _ap_geometry
{

public: // Methods

	_ap242_geometry(OwlInstance owlInstance, SdaiInstance sdaiInstance);
	virtual ~_ap242_geometry();
};

// ************************************************************************************************
class _annotation_plane;
class _draghting_callout;

// ************************************************************************************************
class _ap242_draghting_model
{

private: // Members

	SdaiInstance m_sdaiInstance;
	wstring m_strName;

	// Reperesentation
	vector<_annotation_plane*> m_vecAnnotationPlanes;
	vector<_draghting_callout*> m_vecDraghtingCallouts;

public: // Methods

	_ap242_draghting_model(SdaiInstance sdaiInstance);
	virtual ~_ap242_draghting_model();

protected: // Methods

	void load();

public: // Properties

	SdaiInstance getSdaiInstance() const { return m_sdaiInstance; }
	const wchar_t* getName() { return m_strName.c_str(); }
};

// ************************************************************************************************
class _annotation_plane
{

private: // Members

	SdaiInstance m_sdaiInstance;
	wstring m_strName;

public: // Methods

	_annotation_plane(SdaiInstance sdaiInstance);
	virtual ~_annotation_plane();

public: // Properties

	SdaiInstance getSdaiInstance() const { return m_sdaiInstance; }
	const wchar_t* getName() { return m_strName.c_str(); }
};

// ************************************************************************************************
class _draghting_callout
{

private: // Members

	SdaiInstance m_sdaiInstance;
	wstring m_strName;

public: // Methods

	_draghting_callout(SdaiInstance sdaiInstance);
	virtual ~_draghting_callout();

public: // Properties

	SdaiInstance getSdaiInstance() const { return m_sdaiInstance; }
	const wchar_t* getName() { return m_strName.c_str(); }
};