#ifndef CIS2_DESIGN_PART_H
#define CIS2_DESIGN_PART_H

#include "CIS2Geometry.h"

// ************************************************************************************************
class CCIS2DesignPart
	: public CCIS2Geometry
{

public: // Methods
	
	CCIS2DesignPart(OwlInstance owlInstance, SdaiInstance sdaiInstance);
	virtual ~CCIS2DesignPart();
};

#endif // CIS2_DESIGN_PART_H
