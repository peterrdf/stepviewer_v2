#include "stdafx.h"
#include "CIS2Geometry.h"

// ************************************************************************************************
CCIS2Geometry::CCIS2Geometry(OwlInstance owlInstance, SdaiInstance sdaiInstance, enumCIS2GeometryType enCIS2GeometryType, MultiThreadOwlModelWrapper multiThreadOwlModelWrapper)
	: _ap_geometry(owlInstance, sdaiInstance, multiThreadOwlModelWrapper)
	, m_enCIS2GeometryType(enCIS2GeometryType)
{
	calculate();
}

/*virtual*/ CCIS2Geometry::~CCIS2Geometry()
{}

/*virtual*/ void CCIS2Geometry::preCalculate() /*override*/
{
	// Format
	setAPFormatSettings();

	// Extra settings
	const int_t flagbit1 = 2;
	setFilter(getSdaiModel(), flagbit1, flagbit1);
	setSegmentation(getSdaiModel(), 16, 0.);
}