#pragma once

#include "_ap_mvc.h"

// ************************************************************************************************
class _ap242_draughting_model;

// ************************************************************************************************
class _ap242_model : public _ap_model
{

private: // Members

	vector<_ap242_draughting_model*> m_vecDraughtingModels;

public: // Methods

	_ap242_model();
	virtual ~_ap242_model();

protected: // Methods

	// _ap_model
	virtual void preLoadInstance(OwlInstance owlInstance) override;
	virtual void clean() override;

	void loadDraughtingModels();

public: // Properties

	const vector<_ap242_draughting_model*>& getDraughtingModels() const { return m_vecDraughtingModels; }
};

// ************************************************************************************************
class _ap242_geometry : public _ap_geometry
{

public: // Methods

	_ap242_geometry(OwlInstance owlInstance, SdaiInstance sdaiInstance);
	virtual ~_ap242_geometry();
};

// ************************************************************************************************
class _ap242_annotation_plane;
class _ap242_draughting_callout;

// ************************************************************************************************
class _ap242_draughting_model
{

private: // Members

	SdaiInstance m_sdaiInstance;
	wstring m_strName;

	// Reperesentation
	vector<_ap242_annotation_plane*> m_vecAnnotationPlanes;
	vector<_ap242_draughting_callout*> m_vecDraughtingCallouts;

public: // Methods

	_ap242_draughting_model(SdaiInstance sdaiInstance);
	virtual ~_ap242_draughting_model();

protected: // Methods

	void load();

public: // Properties

	SdaiInstance getSdaiInstance() const { return m_sdaiInstance; }
	ExpressID getExpressID() const { return internalGetP21Line(m_sdaiInstance); }
	const wchar_t* getName() { return m_strName.c_str(); }

	const vector<_ap242_annotation_plane*>& getAnnotationPlanes() const { return m_vecAnnotationPlanes; }
	const vector<_ap242_draughting_callout*> getDraughtingCallouts() const { return m_vecDraughtingCallouts; }
};

// ************************************************************************************************
class _ap242_annotation_plane
{

private: // Members

	SdaiInstance m_sdaiInstance;
	wstring m_strName;

public: // Methods

	_ap242_annotation_plane(SdaiInstance sdaiInstance);
	virtual ~_ap242_annotation_plane();

public: // Properties

	SdaiInstance getSdaiInstance() const { return m_sdaiInstance; }
	ExpressID getExpressID() const { return internalGetP21Line(m_sdaiInstance); }
	const wchar_t* getName() { return m_strName.c_str(); }
};

// ************************************************************************************************
class _ap242_draughting_callout
{

private: // Members

	SdaiInstance m_sdaiInstance;
	wstring m_strName;

public: // Methods

	_ap242_draughting_callout(SdaiInstance sdaiInstance);
	virtual ~_ap242_draughting_callout();

public: // Properties

	SdaiInstance getSdaiInstance() const { return m_sdaiInstance; }
	ExpressID getExpressID() const { return internalGetP21Line(m_sdaiInstance); }
	const wchar_t* getName() { return m_strName.c_str(); }
};