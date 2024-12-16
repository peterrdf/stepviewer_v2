#ifndef CIS2_REPRESENTAION_H
#define CIS2_REPRESENTAION_H

#include "CIS2Geometry.h"

// ************************************************************************************************
class CCIS2Representation
	: public CCIS2Geometry
{

public: // Methods
	
	CCIS2Representation(OwlInstance owlInstance, SdaiInstance sdaiInstance);
	virtual ~CCIS2Representation();
};

#endif // CIS2_REPRESENTAION_H
