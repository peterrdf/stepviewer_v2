#pragma once

#include "_ap_mvc.h"

// ************************************************************************************************
class _ap242_draughting_model;
class _ap242_annotation_plane;
class _ap242_draughting_callout;

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
	_ap242_annotation_plane* loadAnnotationPlane(SdaiInstance sdaiInstance);
	_ap242_draughting_callout* loadDraughtingCallout(SdaiInstance sdaiInstance);

public: // Properties

	const vector<_ap242_draughting_model*>& getDraughtingModels() const { return m_vecDraughtingModels; }
};