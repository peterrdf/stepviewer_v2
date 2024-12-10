#pragma once

#include "_ap_geometry.h"

// ************************************************************************************************
class _ap_instance : public _instance
{

public:  // Methods

	_ap_instance(int64_t iID, _geometry* pGeometry, _matrix4x3* pTransformationMatrix)
		: _instance(iID, pGeometry, pTransformationMatrix)
	{
	}

	virtual ~_ap_instance()
	{
	}

	SdaiInstance getSdaiInstance() const { return getGeometryAs<_ap_geometry>()->getSdaiInstance(); }
	ExpressID getExpressID() const { return getGeometryAs<_ap_geometry>()->getEnable(); }
	OwlInstance getOwlInstance() const { return getGeometryAs<_ap_geometry>()->getInstance(); }
};