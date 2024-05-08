#include "stdafx.h"

#include "InstanceBase.h"

CInstanceBase::CInstanceBase()
{}

/*virtual*/ CInstanceBase::~CInstanceBase()
{}

/*static*/ wstring CInstanceBase::GetName(SdaiInstance iInstance)
{
	wstring strUniqueName;

	ExpressID iExpressID = internalGetP21Line(iInstance);	
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

/*static*/ const wchar_t* CInstanceBase::GetClassName(OwlInstance iInstance)
{
	wchar_t* szClassName = nullptr;
	GetNameOfClassW(GetInstanceClass(iInstance), &szClassName);

	return szClassName;
}

/*static*/ const wchar_t* CInstanceBase::GetEntityName(SdaiInstance iInstance)
{
	wchar_t* szEntityName = nullptr;
	engiGetEntityName(GetEntity(iInstance), sdaiUNICODE, (const char**)&szEntityName);

	return szEntityName != nullptr ? szEntityName : L"";
}