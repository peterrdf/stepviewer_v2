#include "stdafx.h"
#include "IFCInstance.h"
#include "IFCModel.h"

#include <math.h>

// ************************************************************************************************
CIFCInstance::CIFCInstance(int64_t iID, _geometry* pGeometry, _matrix4x3* pTransformationMatrix)
	: _ap_instance(iID, pGeometry, pTransformationMatrix)
{
}

/*virtual*/ CIFCInstance::~CIFCInstance()
{}


