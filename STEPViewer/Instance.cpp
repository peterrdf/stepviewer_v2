#include "stdafx.h"

#include "Instance.h"

// --------------------------------------------------------------------------------------------
CInstance::CInstance()
{
}

// --------------------------------------------------------------------------------------------
/*virtual*/ CInstance::~CInstance()
{
}

// --------------------------------------------------------------------------------------------
wstring CInstance::GetUniqueName() const
{
	return GetUniqueName(GetInstance());
}

// --------------------------------------------------------------------------------------------
/*static*/ wstring CInstance::GetUniqueName(int64_t iInstance)
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
	sdaiGetAttrBN(iInstance, "Name", sdaiUNICODE, &szName);

	if ((szName != nullptr) && (wcslen(szName) > 0))
	{
		strUniqueName += L" '";
		strUniqueName += szName;
		strUniqueName += L"'";
	}

	wchar_t* szDescription = nullptr;
	sdaiGetAttrBN(iInstance, "Description", sdaiUNICODE, &szDescription);

	if ((szDescription != nullptr) && (wcslen(szDescription) > 0))
	{
		strUniqueName += L" (";
		strUniqueName += szDescription;
		strUniqueName += L")";
	}

	return strUniqueName;
}

// --------------------------------------------------------------------------------------------
int64_t CInstance::GetClass() const
{
	return GetClass(GetInstance());
}

// --------------------------------------------------------------------------------------------
/*static*/ int64_t CInstance::GetClass(int64_t iInstance)
{
	return GetInstanceClass(iInstance);
}

// --------------------------------------------------------------------------------------------
const wchar_t* CInstance::GetClassName() const
{
	return GetClassName(GetInstance());
}

// --------------------------------------------------------------------------------------------
/*static*/ const wchar_t* CInstance::GetClassName(int64_t iInstance)
{
	wchar_t* szClassName = nullptr;
	GetNameOfClassW(GetInstanceClass(iInstance), &szClassName);

	return szClassName;
}

// --------------------------------------------------------------------------------------------
int64_t CInstance::GetEntity() const
{
	return sdaiGetInstanceType(GetInstance());
}

// --------------------------------------------------------------------------------------------
/*static*/ int64_t CInstance::GetEntity(int64_t iInstance)
{
	return sdaiGetInstanceType(iInstance);
}

// --------------------------------------------------------------------------------------------
const wchar_t* CInstance::GetEntityName() const
{
	return GetEntityName(GetEntity());
}

// --------------------------------------------------------------------------------------------
/*static*/ const wchar_t* CInstance::GetEntityName(int64_t iInstance)
{
	wchar_t* szEntityName = nullptr;
	engiGetEntityName(GetEntity(iInstance), sdaiUNICODE, (const char**)&szEntityName);

	return szEntityName != nullptr ? szEntityName : L"";
}