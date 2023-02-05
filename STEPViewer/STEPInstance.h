#pragma once

#include <string>

using namespace std;

// ------------------------------------------------------------------------------------------------
class CSTEPInstance
{

public: // Methods
	
	CSTEPInstance();
	virtual ~CSTEPInstance();

	virtual int64_t _getInstance() const PURE;
	virtual bool _hasGeometry() const PURE;
	virtual bool _isEnabled() const PURE;
	virtual wstring _getName() const PURE;
};

