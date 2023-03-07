#pragma once

#include <string>

using namespace std;

// ------------------------------------------------------------------------------------------------
class CInstance
{

public: // Methods
	
	CInstance();
	virtual ~CInstance();

	virtual int64_t GetInstance() const PURE;
	virtual bool HasGeometry() const PURE;
	virtual bool IsEnabled() const PURE;
	virtual wstring GetName() const PURE;

	int64_t GetEntity() const;
	static int64_t GetEntity(int64_t iInstance);
	const wchar_t* GetEntityName() const;
	static const wchar_t* GetEntityName(int64_t iInstance);


};
