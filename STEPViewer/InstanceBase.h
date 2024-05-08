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

	wstring GetName() const { return GetName(GetInstance()); }
	static wstring GetName(SdaiInstance iInstance);
	OwlClass GetClass() const { return GetClass(GetInstance()); }
	static OwlClass GetClass(int64_t iInstance) { return GetInstanceClass(iInstance); }
	const wchar_t* GetClassName() const { return GetClassName(GetInstance()); }
	static const wchar_t* GetClassName(OwlInstance iInstance);
	SdaiEntity GetEntity() const { return sdaiGetInstanceType(GetInstance()); }
	static SdaiEntity GetEntity(SdaiInstance iInstance) { return sdaiGetInstanceType(iInstance); }
	const wchar_t* GetEntityName() const { return GetEntityName(GetInstance()); }
	static const wchar_t* GetEntityName(SdaiInstance iInstance);
};
