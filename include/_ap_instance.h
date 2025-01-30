#pragma once

#include "_instance.h"
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

public: // Properties

	SdaiInstance getSdaiInstance() const { return getGeometryAs<_ap_geometry>()->getSdaiInstance(); }
	ExpressID getExpressID() const { return getGeometryAs<_ap_geometry>()->getExpressID(); }

	virtual wstring getName() const override
	{
		return getName(getSdaiInstance());
	}

	static wstring getName(SdaiInstance sdaiInstance)
	{
		wstring strUniqueName;

		int64_t iExpressID = internalGetP21Line(sdaiInstance);
		if (iExpressID != 0)
		{
			CString strID;
			strID.Format(_T("#%lld"), iExpressID);

			strUniqueName = strID;
			strUniqueName += L" ";
			strUniqueName += getEntityName(sdaiInstance);
		}

		wchar_t* szName = nullptr;
		sdaiGetAttrBN(sdaiInstance, "Name", sdaiUNICODE, &szName);

		if ((szName != nullptr) && (wcslen(szName) > 0))
		{
			strUniqueName += L" '";
			strUniqueName += szName;
			strUniqueName += L"'";
		}

		wchar_t* szDescription = nullptr;
		sdaiGetAttrBN(sdaiInstance, "Description", sdaiUNICODE, &szDescription);

		if ((szDescription != nullptr) && (wcslen(szDescription) > 0))
		{
			strUniqueName += L" (";
			strUniqueName += szDescription;
			strUniqueName += L")";
		}

		return strUniqueName;
	}	

	SdaiEntity getSdaiEntity() const
	{
		return getSdaiEntity(getSdaiInstance());
	}

	static SdaiEntity getSdaiEntity(SdaiInstance sdaiInstance)
	{
		return sdaiGetInstanceType(sdaiInstance);
	}

	const wchar_t* getEntityName() const
	{
		return getEntityName(getSdaiInstance());
	}

	static const wchar_t* getEntityName(SdaiInstance sdaiInstance)
	{
		wchar_t* szEntityName = nullptr;
		engiGetEntityName(getSdaiEntity(sdaiInstance), sdaiUNICODE, (const char**)&szEntityName);

		return szEntityName;
	}
};