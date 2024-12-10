#pragma once

#include "_ap_geometry.h"

// ************************************************************************************************
class _ap_instance : public _instance
{

public:  // Methods

	_ap_instance(int64_t iID, _geometry* pGeometry, _matrix4x3* pTransformationMatrix)
		: _instance(iID, pGeometry, pTransformationMatrix)
	{
	}

	virtual ~_ap_instance()
	{
	}

	SdaiInstance getSdaiInstance() const { return getGeometryAs<_ap_geometry>()->getSdaiInstance(); }
	ExpressID getExpressID() const { return getGeometryAs<_ap_geometry>()->getEnable(); }
	OwlInstance getOwlInstance() const { return getGeometryAs<_ap_geometry>()->getInstance(); }

	wstring GetName() const
	{
		return GetName(getSdaiInstance());
	}

	static wstring GetName(SdaiInstance iInstance)
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

	OwlClass GetClass() const
	{
		return GetClass(getOwlInstance());
	}

	static OwlClass GetClass(OwlInstance iInstance)
	{
		return GetInstanceClass(iInstance);
	}

	const wchar_t* GetClassName() const
	{
		return GetClassName(getOwlInstance());
	}

	static const wchar_t* GetClassName(OwlInstance iInstance)
	{
		wchar_t* szClassName = nullptr;
		GetNameOfClassW(GetInstanceClass(iInstance), &szClassName);

		return szClassName;
	}

	SdaiEntity GetEntity() const
	{
		return GetEntity(getSdaiInstance());
	}

	static SdaiEntity GetEntity(SdaiInstance sdaiInstance)
	{
		return sdaiGetInstanceType(sdaiInstance);
	}

	const wchar_t* GetEntityName() const
	{
		return GetEntityName(getSdaiInstance());
	}

	static const wchar_t* GetEntityName(SdaiInstance sdaiInstance)
	{
		wchar_t* szEntityName = nullptr;
		engiGetEntityName(GetEntity(sdaiInstance), sdaiUNICODE, (const char**)&szEntityName);

		return szEntityName != nullptr ? szEntityName : L"";
	}
};