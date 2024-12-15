#include "stdafx.h"
#include "CIS2Geometry.h"

// ************************************************************************************************
CCIS2Geometry::CCIS2Geometry(OwlInstance owlInstance, SdaiInstance sdaiInstance, enumCIS2GeometryType enCIS2GeometryType)
	: _ap_geometry(owlInstance, sdaiInstance)
	, m_enCIS2GeometryType(enCIS2GeometryType)
{
}

/*virtual*/ CCIS2Geometry::~CCIS2Geometry()
{}
