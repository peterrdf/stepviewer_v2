#pragma once

#include "_ap_geometry.h"

// ************************************************************************************************
class _ifc_geometry : public _ap_geometry
{
	friend class _ifc_model;

private: // Members

	bool m_bUseWorldCoordinates;
	bool m_bIsReferenced;

public: // Methods

	_ifc_geometry(OwlInstance owlInstance, SdaiInstance sdaiInstance, bool bUseWorldCoordinates);
	virtual ~_ifc_geometry();

protected: // Methods

	// _geometry
	virtual void preCalculate() override;
	virtual void postCalculate() override;

public: // Properties

	bool getIsReferenced() const { return m_bIsReferenced; }
};