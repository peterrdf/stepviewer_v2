#include "stdafx.h"
#include "CIS2DesignPart.h"
#include "CIS2Model.h"

#include <math.h>

// ************************************************************************************************
CCIS2DesignPart::CCIS2DesignPart(int64_t iID, SdaiInstance iSdaiInstance)
	: CCIS2Instance(iID, iSdaiInstance, enumCIS2InstanceType::DesignPart)
{}

CCIS2DesignPart::~CCIS2DesignPart()
{}