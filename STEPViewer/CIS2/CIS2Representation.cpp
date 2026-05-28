#include "stdafx.h"
#include "CIS2Representation.h"
#include "CIS2Model.h"

#include <math.h>

// ************************************************************************************************
CCIS2Representation::CCIS2Representation(OwlInstance owlInstance, SdaiInstance sdaiInstance, MultiThreadOwlModelWrapper multiThreadOwlModelWrapper)
	: CCIS2Geometry(owlInstance, sdaiInstance, enumCIS2GeometryType::Reperesentation, multiThreadOwlModelWrapper)
{}

/*virtual*/ CCIS2Representation::~CCIS2Representation()
{}