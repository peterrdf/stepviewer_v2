#ifndef IFC_INSTANCE_H
#define IFC_INSTANCE_H

#include "_oglUtils.h"
#include "_ap_mvc.h"

#include "engine.h"

#include <cstddef>
#include <string>
using namespace std;

// ************************************************************************************************
class CIFCInstance : public _ap_instance
{

public: // Methods
	
	CIFCInstance(int64_t iID, _geometry* pGeometry, _matrix4x3* pTransformationMatrix);
	virtual ~CIFCInstance();
};

#endif // IFC_INSTANCE_H
