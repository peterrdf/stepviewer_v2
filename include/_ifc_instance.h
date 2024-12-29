#pragma once

#include "_ap_instance.h"

// ************************************************************************************************
class _ifc_geometry;

// ************************************************************************************************
class _ifc_instance : public _ap_instance
{

public:  // Methods

	_ifc_instance(int64_t iID, _ifc_geometry* pGeometry, _matrix4x3* pTransformationMatrix);
	virtual ~_ifc_instance();
};