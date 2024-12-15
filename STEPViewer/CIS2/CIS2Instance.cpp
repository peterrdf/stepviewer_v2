#include "stdafx.h"
#include "CIS2Instance.h"

// ************************************************************************************************
CCIS2Instance::CCIS2Instance(int64_t iID, _geometry* pGeometry, _matrix4x3* pTransformationMatrix)
	: _ap_instance(iID, pGeometry, pTransformationMatrix)
{
}

/*virtual*/ CCIS2Instance::~CCIS2Instance()
{}


