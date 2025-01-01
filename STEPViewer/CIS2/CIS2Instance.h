#ifndef CIS2_INSTANCE_H
#define CIS2_INSTANCE_H

#include "_ap_model.h"
#include "_oglUtils.h"

#include "engine.h"

#include <cstddef>
#include <string>
using namespace std;

// ************************************************************************************************
class CCIS2Instance : public _ap_instance
{

public: // Methods
	
	CCIS2Instance(int64_t iID, _geometry* pGeometry, _matrix4x3* pTransformationMatrix);
	virtual ~CCIS2Instance();
};

#endif // CIS2_INSTANCE_H
