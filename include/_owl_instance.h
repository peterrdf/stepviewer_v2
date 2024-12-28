#pragma once

#include "_instance.h"
#include "_owl_geometry.h"

// ************************************************************************************************
class _owl_instance : public _instance
{

public:  // Methods

	_owl_instance(int64_t iID, _geometry* pGeometry, _matrix4x3* pTransformationMatrix)
		: _instance(iID, pGeometry, pTransformationMatrix)
	{
	}

	virtual ~_owl_instance()
	{
	}

	static void buildInstanceNames(OwlModel owlModel, OwlInstance owlInstance, wstring& strName, wstring& strUniqueName)
	{
		ASSERT(owlModel != 0);
		ASSERT(owlInstance != 0);

		OwlClass iClassInstance = GetInstanceClass(owlInstance);
		ASSERT(iClassInstance != 0);

		wchar_t* szClassName = nullptr;
		GetNameOfClassW(iClassInstance, &szClassName);

		wchar_t* szName = nullptr;
		GetNameOfInstanceW(owlInstance, &szName);

		if (szName == nullptr)
		{
			RdfProperty iTagProperty = GetPropertyByName(owlModel, "tag");
			if (iTagProperty != 0)
			{
				SetCharacterSerialization(owlModel, 0, 0, false);

				int64_t iCard = 0;
				wchar_t** szValue = nullptr;
				GetDatatypeProperty(owlInstance, iTagProperty, (void**)&szValue, &iCard);

				if (iCard == 1)
				{
					szName = szValue[0];
				}

				SetCharacterSerialization(owlModel, 0, 0, true);
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
			swprintf(szUniqueName, 200, L"#%lld (%s)", owlInstance, szClassName);
		}

		strUniqueName = szUniqueName;
	}

public: // Properties

	OwlInstance getOwlInstance() const { return getGeometryAs<_owl_geometry>()->getOwlInstance(); }

	wstring getName() const
	{
		CString strName;
		strName.Format(_T("#%lld"), getOwlInstance());

		strName += L" ";
		strName += getClassName();

		return (LPCWSTR)strName;
	}

	static wstring getName(OwlInstance owlInstance)
	{
		CString strName;
		strName.Format(_T("#%lld"), owlInstance);

		strName += L" ";
		strName += getClassName(owlInstance);

		return (LPCWSTR)strName;
	}

	OwlClass getClassInstance() const
	{
		return getClassInstance(getOwlInstance());
	}

	static OwlClass getClassInstance(OwlInstance owlInstance)
	{
		return GetInstanceClass(owlInstance);
	}

	const wchar_t* getClassName() const
	{
		return getClassName(getOwlInstance());
	}

	static const wchar_t* getClassName(OwlInstance owlInstance)
	{
		wchar_t* szClassName = nullptr;
		GetNameOfClassW(GetInstanceClass(owlInstance), &szClassName);

		return szClassName;
	}

	static double getDoubleProperty(OwlInstance iInstance, char* szPropertyName)
	{
		double* pdValues = nullptr;
		int64_t	iCard = 0;
		GetDatatypeProperty(
			iInstance,
			GetPropertyByName(GetModel(iInstance), szPropertyName),
			(void**)&pdValues,
			&iCard);

		return (iCard == 1) ? pdValues[0] : 0.;
	}

	static int64_t getObjectProperty(OwlInstance iInstance, char* szPropertyName)
	{
		int64_t* piValues = nullptr;
		int64_t	iCard = 0;
		GetObjectProperty(
			iInstance,
			GetPropertyByName(GetModel(iInstance), szPropertyName),
			&piValues,
			&iCard);

		return (iCard == 1) ? piValues[0] : 0;
	}
};