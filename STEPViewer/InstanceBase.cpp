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
	return GetName(GetInstance());
}

// --------------------------------------------------------------------------------------------
/*static*/ wstring CInstanceBase::GetName(SdaiInstance iInstance)
{
	wstring strUniqueName;

	int64_t iExpressID = internalGetP21Line(iInstance);
	if (iExpressID != 0)
	{
		CString strID;
		strID.Format(_T("#%lld"), iExpressID);

		strUniqueName = strID;
		strUniqueName += L" ";
		strUniqueName += GetEntityName(iInstance);
	}
	else
	{
		strUniqueName = GetClassName(iInstance);
	}	

	wchar_t* szName = nullptr;
	sdaiGetAttrBN((SdaiInstance)iInstance, "Name", sdaiUNICODE, &szName);

	if ((szName != nullptr) && (wcslen(szName) > 0))
	{
		strUniqueName += L" '";
		strUniqueName += szName;
		strUniqueName += L"'";
	}

	wchar_t* szDescription = nullptr;
	sdaiGetAttrBN((SdaiInstance)iInstance, "Description", sdaiUNICODE, &szDescription);

	if ((szDescription != nullptr) && (wcslen(szDescription) > 0))
	{
		strUniqueName += L" (";
		strUniqueName += szDescription;
		strUniqueName += L")";
	}

	return strUniqueName;
}

// --------------------------------------------------------------------------------------------
OwlClass CInstanceBase::GetClass() const
{
	return GetClass(GetInstance());
}

// --------------------------------------------------------------------------------------------
/*static*/ OwlClass CInstanceBase::GetClass(OwlInstance iInstance)
{
	return GetInstanceClass(iInstance);
}

// --------------------------------------------------------------------------------------------
const wchar_t* CInstanceBase::GetClassName() const
{
	return GetClassName(GetInstance());
}

// --------------------------------------------------------------------------------------------
/*static*/ const wchar_t* CInstanceBase::GetClassName(OwlInstance iInstance)
{
	wchar_t* szClassName = nullptr;
	GetNameOfClassW(GetInstanceClass(iInstance), &szClassName);

	return szClassName;
}

// --------------------------------------------------------------------------------------------
SdaiEntity CInstanceBase::GetEntity() const
{
	return sdaiGetInstanceType(GetInstance());
}

// --------------------------------------------------------------------------------------------
/*static*/ SdaiEntity CInstanceBase::GetEntity(SdaiInstance iInstance)
{
	return sdaiGetInstanceType(iInstance);
}

// --------------------------------------------------------------------------------------------
const wchar_t* CInstanceBase::GetEntityName() const
{
	return GetEntityName(GetInstance());
}

// --------------------------------------------------------------------------------------------
/*static*/ const wchar_t* CInstanceBase::GetEntityName(SdaiInstance iInstance)
{
	wchar_t* szEntityName = nullptr;
	engiGetEntityName(GetEntity(iInstance), sdaiUNICODE, (const char**)&szEntityName);

	return szEntityName != nullptr ? szEntityName : L"";
}

// --------------------------------------------------------------------------------------------
/*static*/ void CInstanceBase::BuildInstanceNames(OwlModel iModel, OwlInstance iInstance, wstring& strName, wstring& strUniqueName)
{
	ASSERT(iModel != 0);
	ASSERT(iInstance != 0);

	OwlClass iClassInstance = GetInstanceClass(iInstance);
	ASSERT(iClassInstance != 0);

	wchar_t* szClassName = nullptr;
	GetNameOfClassW(iClassInstance, &szClassName);

	wchar_t* szName = nullptr;
	GetNameOfInstanceW(iInstance, &szName);

	if (szName == nullptr)
	{
		RdfProperty iTagProperty = GetPropertyByName(iModel, "tag");
		if (iTagProperty != 0)
		{
			SetCharacterSerialization(iModel, 0, 0, false);

			int64_t iCard = 0;
			wchar_t** szValue = nullptr;
			GetDatatypeProperty(iInstance, iTagProperty, (void**)&szValue, &iCard);

			if (iCard == 1)
			{
				szName = szValue[0];
			}

			SetCharacterSerialization(iModel, 0, 0, true);
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
		swprintf(szUniqueName, 200, L"#%lld (%s)", iInstance, szClassName);
	}

	strUniqueName = szUniqueName;
}