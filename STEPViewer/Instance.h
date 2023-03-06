#pragma once

#include <string>

using namespace std;

// ------------------------------------------------------------------------------------------------
class CInstance
{

public: // Methods
	
	CInstance();
	virtual ~CInstance();

	virtual int64_t _getInstance() const PURE;
	virtual bool _hasGeometry() const PURE;
	virtual bool _isEnabled() const PURE;
	virtual wstring _getName() const PURE;
	int64_t _getEntity() const;
	const wchar_t* _getEntityName() const;
};
