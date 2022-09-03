#pragma once

#include "IFCProperty.h"

// ------------------------------------------------------------------------------------------------
class CIntIFCProperty : public CIFCProperty
{

public: // Methods

	// --------------------------------------------------------------------------------------------
	// ctor
	CIntIFCProperty(int64_t iInstance);

	// --------------------------------------------------------------------------------------------
	// dtor
	virtual ~CIntIFCProperty();
};

