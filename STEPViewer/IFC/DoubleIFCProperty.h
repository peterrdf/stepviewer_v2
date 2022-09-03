#pragma once

#include "IFCProperty.h"

// ------------------------------------------------------------------------------------------------
class CDoubleIFCProperty : public CIFCProperty
{

public: // Methods

	// --------------------------------------------------------------------------------------------
	// ctor
	CDoubleIFCProperty(int64_t iInstance);

	// --------------------------------------------------------------------------------------------
	// dtor
	virtual ~CDoubleIFCProperty();
};

