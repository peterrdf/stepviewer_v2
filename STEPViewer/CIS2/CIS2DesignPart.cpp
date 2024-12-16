#include "stdafx.h"
#include "CIS2DesignPart.h"
#include "CIS2Model.h"

#include <math.h>

// ************************************************************************************************
CCIS2DesignPart::CCIS2DesignPart(OwlInstance owlInstance, SdaiInstance sdaiInstance)
	: CCIS2Geometry(owlInstance, sdaiInstance, enumCIS2GeometryType::DesignPart)
{}

CCIS2DesignPart::~CCIS2DesignPart()
{}