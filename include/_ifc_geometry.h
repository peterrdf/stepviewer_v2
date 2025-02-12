#pragma once

#include "_ap_geometry.h"

// ************************************************************************************************
class _ifc_geometry : public _ap_geometry
{
	friend class _ifc_model;

private: // Members

	vector<_ifc_geometry*> m_vecMappedItems;

	bool m_bIsReferenced;

public: // Methods

	_ifc_geometry(OwlInstance owlInstance, SdaiInstance sdaiInstance, const vector<_ifc_geometry*>& vecMappedItems);
	virtual ~_ifc_geometry();

protected: // Methods

	// _geometry
	virtual void preCalculate() override;
	virtual void postCalculate() override;
	virtual bool hasGeometry() const override;

public: // Properties

	bool getIsReferenced() const { return m_bIsReferenced; }
};