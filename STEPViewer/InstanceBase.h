#pragma once

#include <string>
using namespace std;

#include "_ap_instance.h"

// ************************************************************************************************
class CInstanceBase : public _ap_instance
{

public: // Methods
	
	CInstanceBase(int64_t iID, _geometry* pGeometry, _matrix4x3* pTransformationMatrix);
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