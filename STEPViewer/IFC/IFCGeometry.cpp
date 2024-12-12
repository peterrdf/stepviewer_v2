#include "stdafx.h"
#include "IFCGeometry.h"
#include "IFCModel.h"

#include <math.h>

// ************************************************************************************************
CIFCGeometry::CIFCGeometry(SdaiInstance sdaiInstance)
	: _ap_geometry(sdaiInstance)
	, m_bReferenced(false)
{
	calculate();
}

/*virtual*/ CIFCGeometry::~CIFCGeometry()
{}

/*virtual*/ void CIFCGeometry::preCalculate() /*override*/
{
	// Format
	setAPFormatSettings();

	// Extra settings
	const int_t flagbit1 = 2;
	setFilter(getSdaiModel(), flagbit1, flagbit1);
	setSegmentation(getSdaiModel(), 16, 0.);
}