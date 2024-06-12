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

	OwlClass GetClass() const;
	static OwlClass GetClass(OwlInstance iInstance);
	const wchar_t* GetClassName() const;
	static const wchar_t* GetClassName(OwlInstance iInstance);

	SdaiEntity GetEntity() const;
	static SdaiEntity GetEntity(SdaiInstance iInstance);
	const wchar_t* GetEntityName() const;
	static const wchar_t* GetEntityName(SdaiInstance iInstance);

	static void BuildInstanceNames(OwlModel iModel, OwlInstance iInstance, wstring& strName, wstring& strUniqueName);
};