#pragma once

#include "IFCProperty.h"

// ------------------------------------------------------------------------------------------------
class CBoolIFCProperty : public CIFCProperty
{

public: // Methods

	// --------------------------------------------------------------------------------------------
	// ctor
	CBoolIFCProperty(int64_t iInstance);

	// --------------------------------------------------------------------------------------------
	// dtor
	virtual ~CBoolIFCProperty();
};

