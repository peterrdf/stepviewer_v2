#pragma once

#include "_ap_mvc.h"

// ************************************************************************************************
class _step_model : public _ap_model
{

public: // Methods

	_step_model();
	virtual ~_step_model();

protected: // Methods

	// _ap_model
	virtual void preLoadInstance(OwlInstance owlInstance) override;
};

// ************************************************************************************************
class _step_geometry : public _ap_geometry
{

public: // Methods

	_step_geometry(OwlInstance owlInstance, SdaiInstance sdaiInstance);
	virtual ~_step_geometry();
};

