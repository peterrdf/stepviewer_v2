#pragma once

#include <string>

using namespace std;

// ************************************************************************************************
class CInstanceBase
{

public: // Methods
	
	CInstanceBase();
	virtual ~CInstanceBase();

	virtual SdaiInstance GetSdaiInstance() const PURE;
	virtual bool HasGeometry() const PURE;
	virtual bool IsEnabled() const PURE;

	wstring GetName() const;
	static wstring GetName(SdaiInstance iSdaiInstance);

//	OwlClass GetOwlClass() const;
//	static OwlClass GetOwlClass(OwlInstance iInstance);
//	const wchar_t* GetClassName() const;
//	static const wchar_t* GetClassName(OwlInstance iInstance);

	SdaiModel GetSdaiModel() const;
	SdaiEntity GetEntity() const;
	static SdaiEntity GetEntity(SdaiInstance iSdaiInstance);
	const wchar_t* GetEntityName() const;
	static const wchar_t* GetEntityName(SdaiEntity iSdaiEntity);

	static void BuildOwlInstanceName(OwlInstance iOwlInstance, wstring& strName, wstring& strUniqueName);
};