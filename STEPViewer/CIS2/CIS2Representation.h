#ifndef CIS2_REPRESENTAION_H
#define CIS2_REPRESENTAION_H

#include "CIS2Instance.h"

// ************************************************************************************************
class CCIS2Representation
	: public CCIS2Instance
{

public: // Methods
	
	CCIS2Representation(int64_t iID, SdaiInstance iSdaiInstance);
	virtual ~CCIS2Representation();
};

#endif // CIS2_REPRESENTAION_H
