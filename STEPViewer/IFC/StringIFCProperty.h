#pragma once

#include "IFCProperty.h"

// ------------------------------------------------------------------------------------------------
class CStringIFCProperty : public CIFCProperty
{

public: // Methods

	// --------------------------------------------------------------------------------------------
	// ctor
	CStringIFCProperty(int64_t iInstance);

	// --------------------------------------------------------------------------------------------
	// dtor
	virtual ~CStringIFCProperty();
};

