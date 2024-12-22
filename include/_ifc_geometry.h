#pragma once

#include "_ap_mvc.h"

// ************************************************************************************************
class _ifc_model;

// ************************************************************************************************
class _ifc_geometry : public _ap_geometry
{
	friend class _ifc_model;

private: // Members

	bool m_bIsReferenced;

public: // Methods

	_ifc_geometry(OwlInstance owlInstance, SdaiInstance sdaiInstance);
	virtual ~_ifc_geometry();

protected: // Methods

	// _geometry
	virtual void preCalculate() override;
	virtual void postCalculate() override;

public: // Properties

	bool getIsReferenced() const { return m_bIsReferenced; }
};

