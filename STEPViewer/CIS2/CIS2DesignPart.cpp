#include "stdafx.h"
#include "CIS2DesignPart.h"
#include "CIS2Model.h"

#include <math.h>

// ************************************************************************************************
CCIS2DesignPart::CCIS2DesignPart(OwlInstance owlInstance, SdaiInstance sdaiInstance, MultiThreadOwlModelWrapper multiThreadOwlModelWrapper)
	: CCIS2Geometry(owlInstance, sdaiInstance, enumCIS2GeometryType::DesignPart, multiThreadOwlModelWrapper)
{}

CCIS2DesignPart::~CCIS2DesignPart()
{}