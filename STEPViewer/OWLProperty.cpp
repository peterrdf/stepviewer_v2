#include "stdafx.h"
#include "OWLProperty.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
COWLProperty::COWLProperty(int64_t iInstance)
	: m_iInstance(iInstance)
{
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ COWLProperty::~COWLProperty()
{
}

// ------------------------------------------------------------------------------------------------
int64_t COWLProperty::GetInstance() const
{
	return m_iInstance;
}

// ------------------------------------------------------------------------------------------------
int64_t COWLProperty::GetType() const
{
	return GetType(GetInstance());
}

// ------------------------------------------------------------------------------------------------
/*static*/ int64_t COWLProperty::GetType(int64_t iInstance)
{
	return GetPropertyType(iInstance);
}

// ------------------------------------------------------------------------------------------------
wstring COWLProperty::GetTypeName()
{
	return GetTypeName(GetType());
}

// ------------------------------------------------------------------------------------------------
/*static*/ wstring COWLProperty::GetTypeName(int64_t iType)
{
	wstring strTypeName = iType == OBJECTPROPERTY_TYPE ?
		L"owl:ObjectProperty" :
		L"owl:DatatypeProperty";

	return strTypeName;
}

// ------------------------------------------------------------------------------------------------
wchar_t* COWLProperty::GetName() const
{
	wchar_t* szName = nullptr;
	GetNameOfPropertyW(GetInstance(), &szName);

	return szName;
}

// ------------------------------------------------------------------------------------------------
wstring COWLProperty::GetRange(vector<int64_t>& vecRestrictionClasses) const
{
	return GetRange(GetInstance(), vecRestrictionClasses);
}

// ------------------------------------------------------------------------------------------------
/*static*/ wstring COWLProperty::GetRange(int64_t iInstance, vector<int64_t>& vecRestrictionClasses)
{
	wstring strRange = L"unknown";
	vecRestrictionClasses.clear();

	switch (GetType(iInstance))
	{
		case OBJECTTYPEPROPERTY_TYPE:
		{
			strRange = L"xsd:object";

			int64_t	iRestrictionClassInstance = GetRangeRestrictionsByIterator(iInstance, 0);
			while (iRestrictionClassInstance != 0)
			{
				vecRestrictionClasses.push_back(iRestrictionClassInstance);				

				iRestrictionClassInstance = GetRangeRestrictionsByIterator(iInstance, iRestrictionClassInstance);
			}
		}
		break;

		case DATATYPEPROPERTY_TYPE_BOOLEAN:
		{
			strRange = L"xsd:boolean";
		}
		break;

		case DATATYPEPROPERTY_TYPE_STRING:
		{
			strRange = L"xsd:string";
		}
		break;

		case DATATYPEPROPERTY_TYPE_INTEGER:
		{
			strRange = L"xsd:integer";
		}
		break;

		case DATATYPEPROPERTY_TYPE_DOUBLE:
		{
			strRange = L"xsd:double";
		}
		break;

		case DATATYPEPROPERTY_TYPE_BYTE:
		{
			strRange = L"xsd:integer";
		}
		break;

		default:
		{
			ASSERT(false);
		}
		break;
	} // switch (getType())

	return strRange;
}

// ------------------------------------------------------------------------------------------------
wstring COWLProperty::GetCardinality(int64_t iInstance) const
{
	return GetCardinality(iInstance, GetInstance());
}

// ------------------------------------------------------------------------------------------------
/*static*/ wstring COWLProperty::GetCardinality(int64_t iInstance, int64_t iPropertyInstance)
{
	ASSERT(iInstance != 0);
	ASSERT(iPropertyInstance != 0);

	int64_t iCard = 0;
	switch (GetType(iPropertyInstance))
	{
		case OBJECTTYPEPROPERTY_TYPE:
		{
			int64_t* piInstances = nullptr;
			GetObjectProperty(iInstance, iPropertyInstance, &piInstances, &iCard);
		}
		break;

		case DATATYPEPROPERTY_TYPE_BOOLEAN:
		{
			bool* pbValue = nullptr;
			GetDatatypeProperty(iInstance, iPropertyInstance, (void**)&pbValue, &iCard);
		}
		break;

		case DATATYPEPROPERTY_TYPE_STRING:
		{
			char** szValue = nullptr;
			GetDatatypeProperty(iInstance, iPropertyInstance, (void**)&szValue, &iCard);
		}
		break;

		case DATATYPEPROPERTY_TYPE_INTEGER:
		{
			int64_t* piValue = nullptr;
			GetDatatypeProperty(iInstance, iPropertyInstance, (void**)&piValue, &iCard);
		}
		break;

		case DATATYPEPROPERTY_TYPE_DOUBLE:
		{
			double* pdValue = nullptr;
			GetDatatypeProperty(iInstance, iPropertyInstance, (void**)&pdValue, &iCard);
		}
		break;

		case DATATYPEPROPERTY_TYPE_BYTE:
		{
			BYTE* piValue = nullptr;
			GetDatatypeProperty(iInstance, iPropertyInstance, (void**)&piValue, &iCard);
		}
		break;	

		default:
		{
			ASSERT(false);
		}
		break;
	} // switch (GetType(iPropertyInstance))

	int64_t iInstanceClass = GetInstanceClass(iInstance);
	ASSERT(iInstanceClass != 0);

	int64_t	iMinCard = 0;
	int64_t iMaxCard = 0;
	GetClassPropertyAggregatedCardinalityRestriction(iInstanceClass, iPropertyInstance, &iMinCard, &iMaxCard);

	wchar_t szBuffer[100];
	if ((iMinCard == -1) && (iMaxCard == -1))
	{
		swprintf(szBuffer, 100, L"%lld of [0 - infinity>", iCard);
	}
	else
	{
		if (iMaxCard == -1)
		{
			swprintf(szBuffer, 100, L"%lld of [%lld - infinity>", iCard, iMinCard);
		}
		else
		{
			swprintf(szBuffer, 100, L"%lld of [%lld - %lld]", iCard, iMinCard, iMaxCard);
		}
	}

	return szBuffer;
}
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
COWLPropertyCollection::COWLPropertyCollection()
	: m_vecProperties()
{
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ COWLPropertyCollection::~COWLPropertyCollection()
{
	for (auto pProperty : m_vecProperties)
	{
		delete pProperty;
	}
}

// ------------------------------------------------------------------------------------------------
vector<COWLProperty*>& COWLPropertyCollection::Properties()
{
	return m_vecProperties;
}

// ------------------------------------------------------------------------------------------------
COWLPropertyProvider::COWLPropertyProvider(int64_t iModel)
	: m_iModel(iModel)
	, m_mapPropertyCollections()
{	
	ASSERT(m_iModel != 0);
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ COWLPropertyProvider::~COWLPropertyProvider()
{
	for (auto itPropertyCollection : m_mapPropertyCollections)
	{
		delete itPropertyCollection.second;
	}
}

// ------------------------------------------------------------------------------------------------
COWLPropertyCollection* COWLPropertyProvider::GetPropertyCollection(int64_t iInstance)
{
	if (iInstance == 0)
	{
		ASSERT(FALSE);

		return nullptr;
	}

	auto itPropertyCollection = m_mapPropertyCollections.find(iInstance);
	if (itPropertyCollection != m_mapPropertyCollections.end())
	{
		return itPropertyCollection->second;
	}

	auto pPropertyCollection = LoadPropertyCollection(iInstance);
	m_mapPropertyCollections[iInstance] = pPropertyCollection;

	return pPropertyCollection;
}

// ------------------------------------------------------------------------------------------------
COWLPropertyCollection* COWLPropertyProvider::LoadPropertyCollection(int64_t iInstance)
{
	if (iInstance == 0)
	{
		ASSERT(FALSE);

		return nullptr;
	}

	auto propertyCollection = new COWLPropertyCollection();

	int64_t iPropertyInstance = GetInstancePropertyByIterator(iInstance, 0);
	while (iPropertyInstance != 0)
	{
		propertyCollection->Properties().push_back(new COWLProperty(iPropertyInstance));

		iPropertyInstance = GetInstancePropertyByIterator(iInstance, iPropertyInstance);
	}

	return propertyCollection;
}

// ------------------------------------------------------------------------------------------------
wstring COWLPropertyProvider::GetPropertyName(SdaiInstance iPropertyInstance) const
{
	if (iPropertyInstance == 0)
	{
		ASSERT(FALSE);

		return L"";
	}

	wstring strName;

	wchar_t* szName = nullptr;
	sdaiGetAttrBN(iPropertyInstance, "Name", sdaiUNICODE, &szName);

	if ((szName != nullptr) && (wcslen(szName) > 0))
	{
		strName = szName;
	}
	else
	{
		strName = L"<empty>";
	}

	wchar_t* szDescription = nullptr;
	sdaiGetAttrBN(iPropertyInstance, "Description", sdaiUNICODE, &szDescription);

	if ((szDescription != nullptr) && (wcslen(szDescription) > 0))
	{
		strName += L" (";
		strName += szDescription;
		strName += L")";
	}

	return strName;
}