#pragma once

#include <string>

using namespace std;

// ------------------------------------------------------------------------------------------------
class CInstanceBase
{

public: // Methods
	
	CInstanceBase();
	virtual ~CInstanceBase();

	virtual int64_t GetInstance() const PURE;
	virtual bool HasGeometry() const PURE;
	virtual bool IsEnabled() const PURE;

	wstring GetName() const;
	static wstring GetName(int64_t iInstance);

	int64_t GetClass() const;
	static int64_t GetClass(int64_t iInstance);
	const wchar_t* GetClassName() const;
	static const wchar_t* GetClassName(int64_t iInstance);

	int64_t GetEntity() const;
	static int64_t GetEntity(int64_t iInstance);
	const wchar_t* GetEntityName() const;
	static const wchar_t* GetEntityName(int64_t iInstance);
};
