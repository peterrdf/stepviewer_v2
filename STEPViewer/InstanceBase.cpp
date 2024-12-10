#include "stdafx.h"

#include "InstanceBase.h"

// --------------------------------------------------------------------------------------------
CInstanceBase::CInstanceBase(int64_t iID, _geometry* pGeometry, _matrix4x3* pTransformationMatrix)
	: _ap_instance(iID, pGeometry, pTransformationMatrix)
{
}

// --------------------------------------------------------------------------------------------
/*virtual*/ CInstanceBase::~CInstanceBase()
{
}

// --------------------------------------------------------------------------------------------




// --------------------------------------------------------------------------------------------
