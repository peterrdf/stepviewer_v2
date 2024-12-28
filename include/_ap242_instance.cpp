#include "stdafx.h"

#include "_ap242_instance.h"
#include "_ap242_geometry.h"
#include "_ap242_product_definition.h"

// ************************************************************************************************
_ap242_instance::_ap242_instance(int64_t iID, _ap242_geometry* pGeometry, _matrix4x3* pTransformationMatrix)
	: _ap_instance(iID, pGeometry, pTransformationMatrix)
{
}

/*virtual*/ _ap242_instance::~_ap242_instance()
{
}

_ap242_product_definition* _ap242_instance::GetProductDefinition() const
{
	return getGeometryAs<_ap242_product_definition>();
}
