#include "stdafx.h"

#include "InstanceBase.h"

// --------------------------------------------------------------------------------------------
CInstanceBase::CInstanceBase()
{
}

// --------------------------------------------------------------------------------------------
/*virtual*/ CInstanceBase::~CInstanceBase()
{
}

// --------------------------------------------------------------------------------------------
wstring CInstanceBase::GetName() const
{
	return GetName(GetSdaiInstance());
}

// --------------------------------------------------------------------------------------------
/*static*/ wstring CInstanceBase::GetName(SdaiInstance iSdaiInstance)
{
	wstring strUniqueName;

	int64_t iExpressID = internalGetP21Line(iSdaiInstance);
	if (iExpressID != 0)
	{
		CString strID;
		strID.Format(_T("#%lld"), iExpressID);

		strUniqueName = strID;
		strUniqueName += L" ";
		strUniqueName += GetEntityName(iSdaiInstance);
	}
//	else
//	{
//		strUniqueName = GetClassName(iSdaiInstance);
//	}	

	wchar_t* szName = nullptr;
	sdaiGetAttrBN(iSdaiInstance, "Name", sdaiUNICODE, &szName);

	if ((szName != nullptr) && (wcslen(szName) > 0))
	{
		strUniqueName += L" '";
		strUniqueName += szName;
		strUniqueName += L"'";
	}

	wchar_t* szDescription = nullptr;
	sdaiGetAttrBN(iSdaiInstance, "Description", sdaiUNICODE, &szDescription);

	if ((szDescription != nullptr) && (wcslen(szDescription) > 0))
	{
		strUniqueName += L" (";
		strUniqueName += szDescription;
		strUniqueName += L")";
	}

	return strUniqueName;
}

SdaiModel CInstanceBase::GetSdaiModel() const
{
	return sdaiGetInstanceModel(GetSdaiInstance());
}

// --------------------------------------------------------------------------------------------
SdaiEntity CInstanceBase::GetEntity() const
{
	return GetEntity(GetSdaiInstance());
}

// --------------------------------------------------------------------------------------------
/*static*/ SdaiEntity CInstanceBase::GetEntity(SdaiInstance iSdaiInstance)
{
	return sdaiGetInstanceType(iSdaiInstance);
}

// --------------------------------------------------------------------------------------------
//const wchar_t* CInstanceBase::GetClassName() const
//{
//	return GetClassName(GetInstance());
//}

// --------------------------------------------------------------------------------------------
// *static*/ const wchar_t* CInstanceBase::GetClassName(OwlInstance iInstance)
//{
//	wchar_t* szClassName = nullptr;
//	GetNameOfClassW(GetInstanceClass(iInstance), &szClassName);
//
//	return szClassName;
//}
/*
// --------------------------------------------------------------------------------------------
SdaiEntity CInstanceBase::GetEntity() const
{
	return sdaiGetInstanceType(GetSdaiInstance());
}

// --------------------------------------------------------------------------------------------
/*static* / SdaiEntity CInstanceBase::GetEntity(SdaiInstance iSdaiInstance)
{
	return sdaiGetInstanceType(iSdaiInstance);
}	//	*/

// --------------------------------------------------------------------------------------------
const wchar_t* CInstanceBase::GetEntityName() const
{
	return GetEntityName(GetSdaiInstance());
}

// --------------------------------------------------------------------------------------------
/*static*/ const wchar_t* CInstanceBase::GetEntityName(SdaiEntity iSdaiEntity)
{
	wchar_t	* szEntityName = nullptr;
	engiGetEntityName(iSdaiEntity, sdaiUNICODE, (const char**) &szEntityName);

	return szEntityName != nullptr ? szEntityName : L"";
}

// --------------------------------------------------------------------------------------------
/*static*/ void CInstanceBase::BuildOwlInstanceName(OwlInstance iOwlInstance, wstring& strName, wstring& strUniqueName)
{
	OwlModel iOwlModel = GetModel(iOwlInstance);

	ASSERT(iOwlModel != 0);
	ASSERT(iOwlInstance != 0);

	OwlClass iClassInstance = GetInstanceClass(iOwlInstance);
	ASSERT(iClassInstance != 0);

	wchar_t* szClassName = nullptr;
	GetNameOfClassW(iClassInstance, &szClassName);

	wchar_t* szName = nullptr;
	GetNameOfInstanceW(iOwlInstance, &szName);

	if (szName == nullptr)
	{
		RdfProperty iTagProperty = GetPropertyByName(iOwlModel, "tag");
		if (iTagProperty != 0)
		{
			SetCharacterSerialization(iOwlModel, 0, 0, false);

			int64_t iCard = 0;
			wchar_t	** szValue = nullptr;
			GetDatatypeProperty(iOwlInstance, iTagProperty, (void**) &szValue, &iCard);

			if (iCard == 1)
			{
				szName = szValue[0];
			}

			SetCharacterSerialization(iOwlModel, 0, 0, true);
		}
	} // if (szName == nullptr)

	wchar_t szUniqueName[200];

	if (szName != nullptr)
	{
		strName = szName;
		swprintf(szUniqueName, 200, L"%s (%s)", szName, szClassName);
	}
	else
	{
		strName = szClassName;
		swprintf(szUniqueName, 200, L"#%lld (%s)", iOwlInstance, szClassName);
	}

	strUniqueName = szUniqueName;
}