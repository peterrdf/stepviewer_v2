#pragma once

#include <string>

using namespace std;

// ************************************************************************************************
class CInstanceBase
{

public: // Methods
	
	CInstanceBase();
	virtual ~CInstanceBase();

	virtual SdaiInstance GetInstance() const PURE;
	virtual bool HasGeometry() const PURE;
	virtual bool IsEnabled() const PURE;

	wstring GetName() const;
	static wstring GetName(SdaiInstance iInstance);

	int64_t GetClass() const;
	static int64_t GetClass(int64_t iInstance);
	const wchar_t* GetClassName() const;
	static const wchar_t* GetClassName(int64_t iInstance);

	SdaiEntity GetEntity() const;
	static SdaiEntity GetEntity(SdaiInstance iInstance);
	const wchar_t* GetEntityName() const;
	static const wchar_t* GetEntityName(SdaiInstance iInstance);
};
