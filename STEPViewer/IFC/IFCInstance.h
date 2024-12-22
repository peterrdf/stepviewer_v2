#ifndef IFC_INSTANCE_H
#define IFC_INSTANCE_H

#include "_ap_mvc.h"

// ************************************************************************************************
class CIFCInstance : public _ap_instance
{

public: // Methods
	
	CIFCInstance(int64_t iID, _geometry* pGeometry, _matrix4x3* pTransformationMatrix);
	virtual ~CIFCInstance();
};

#endif // IFC_INSTANCE_H
