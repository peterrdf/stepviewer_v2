#pragma once

#include "IFCProperty.h"
#include <vector>

using namespace std;

// ------------------------------------------------------------------------------------------------
class CObjectIFCProperty : public CIFCProperty
{

private: // Members

	// --------------------------------------------------------------------------------------------
	// Restrictions
	vector<int64_t> m_vecRestrictions;


public: // Methods

	// --------------------------------------------------------------------------------------------
	// ctor
	CObjectIFCProperty(int64_t iInstance);

	// --------------------------------------------------------------------------------------------
	// dtor
	virtual ~CObjectIFCProperty();

	// --------------------------------------------------------------------------------------------
	// Getter
	const vector<int64_t> & getRestrictions();
};

