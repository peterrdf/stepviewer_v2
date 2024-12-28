#include "stdafx.h"

#include "_ifc_geometry.h"

// ************************************************************************************************
_ifc_geometry::_ifc_geometry(OwlInstance owlInstance, SdaiInstance sdaiInstance)
	: _ap_geometry(owlInstance, sdaiInstance)
	, m_bIsReferenced(false)
{
	calculate();
}

/*virtual*/ _ifc_geometry::~_ifc_geometry()
{
}

/*virtual*/ void _ifc_geometry::preCalculate() /*override*/
{
	// Format
	setAPFormatSettings();

	// Extra settings
	const int_t flagbit1 = 2;
	setFilter(getSdaiModel(), flagbit1, flagbit1);
	setSegmentation(getSdaiModel(), 16, 0.);
}

/*virtual*/ void _ifc_geometry::postCalculate() /*override*/
{
	cleanCachedGeometry();
}