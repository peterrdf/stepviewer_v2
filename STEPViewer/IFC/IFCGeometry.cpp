#include "stdafx.h"
#include "IFCGeometry.h"
#include "IFCModel.h"

#include <math.h>

// ************************************************************************************************
CIFCGeometry::CIFCGeometry(int64_t iID, SdaiInstance iSdaiInstance)
	: _ap_geometry(iSdaiInstance)
	, m_bReferenced(false)
{
	calculate();
}

/*virtual*/ CIFCGeometry::~CIFCGeometry()
{}
