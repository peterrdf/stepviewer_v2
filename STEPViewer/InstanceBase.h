#pragma once

#include <string>
using namespace std;

#include "_ap_instance.h"

// ************************************************************************************************
class CInstanceBase : public _ap_instance
{

public: // Methods
	
	CInstanceBase(int64_t iID, _geometry* pGeometry, _matrix4x3* pTransformationMatrix);
	virtual ~CInstanceBase();
};