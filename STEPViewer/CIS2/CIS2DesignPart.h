#ifndef CIS2_DESIGN_PART_H
#define CIS2_DESIGN_PART_H

#include "CIS2Instance.h"

// ************************************************************************************************
class CCIS2DesignPart
	: public CCIS2Instance
{

public: // Methods
	
	CCIS2DesignPart(int64_t iID, SdaiInstance iSdaiInstance);
	virtual ~CCIS2DesignPart();
};

#endif // CIS2_DESIGN_PART_H
