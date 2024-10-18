#include "stdafx.h"
#include "CIS2Representation.h"
#include "CIS2Model.h"

#include <math.h>

// ************************************************************************************************
CCIS2Representation::CCIS2Representation(int64_t iID, SdaiInstance iSdaiInstance)
	: CCIS2Instance(iID, iSdaiInstance, enumCIS2InstanceType::Reperesentation)
{}

/*virtual*/ CCIS2Representation::~CCIS2Representation()
{}