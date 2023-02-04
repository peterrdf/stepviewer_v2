#pragma once

// ------------------------------------------------------------------------------------------------
class CSTEPInstance
{

public: // Methods
	
	CSTEPInstance();
	virtual ~CSTEPInstance();

	virtual bool _hasGeometry() const PURE;
	virtual bool _isEnabled() const PURE;
};

