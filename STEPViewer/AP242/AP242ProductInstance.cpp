#include "stdafx.h"
#include "AP242ProductInstance.h"
#include "AP242ProductDefinition.h"

// ************************************************************************************************
CAP242ProductInstance::CAP242ProductInstance(int64_t iID, _geometry* pGeometry, _matrix4x3* pTransformationMatrix)
	: _ap_instance(iID, pGeometry, pTransformationMatrix)
{
}

CAP242ProductInstance::~CAP242ProductInstance()
{
}
