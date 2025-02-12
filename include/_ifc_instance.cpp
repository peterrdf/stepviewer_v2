#include "_host.h"
#include "_ifc_instance.h"
#include "_ifc_geometry.h"

// ************************************************************************************************
_ifc_instance::_ifc_instance(int64_t iID, _ifc_geometry* pGeometry, _matrix4x3* pTransformationMatrix)
	: _ap_instance(iID, pGeometry, pTransformationMatrix)
	, m_pReferencedBy(nullptr)
{
}

/*virtual*/ _ifc_instance::~_ifc_instance()
{
}

