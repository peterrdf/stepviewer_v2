#pragma once

#include "_ap_instance.h"

// ************************************************************************************************
class _ifc_instance : public _ap_instance
{

private: // Members

public:  // Methods

	_ifc_instance(_geometry* pGeometry, _matrix4x4* pTransformationMatrix)
		: _ap_instance(pGeometry, pTransformationMatrix)
	{}

	virtual ~_ifc_instance()
	{}
};