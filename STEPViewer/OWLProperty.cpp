#include "stdafx.h"
#include "OWLProperty.h"

// ************************************************************************************************
COWLProperty::COWLProperty(RdfProperty iInstance)
	: m_iInstance(iInstance)
{}

/*virtual*/ COWLProperty::~COWLProperty()
{}

/*static*/ wstring COWLProperty::GetTypeName(RdfPropertyType iType)
{
	wstring strTypeName = iType == OBJECTPROPERTY_TYPE ?
		L"owl:ObjectProperty" :
		L"owl:DatatypeProperty";

	return strTypeName;
}

wchar_t* COWLProperty::GetName() const
{
	wchar_t* szName = nullptr;
	GetNameOfPropertyW(m_iInstance, &szName);

	return szName;
}

/*static*/ wstring COWLProperty::GetRange(RdfProperty iInstance, vector<OwlClass>& vecRestrictionClasses)
{
	wstring strRange = L"unknown";
	vecRestrictionClasses.clear();

	switch (GetType(iInstance))
	{
		case OBJECTTYPEPROPERTY_TYPE:
		{
			strRange = L"xsd:object";

			OwlClass iRestrictionClassInstance = GetRangeRestrictionsByIterator(iInstance, 0);
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

/*static*/ wstring COWLProperty::GetCardinality(OwlInstance iInstance, RdfProperty iPropertyInstance)
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

	OwlClass iInstanceClass = GetInstanceClass(iInstance);
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

// ************************************************************************************************
COWLPropertyCollection::COWLPropertyCollection()
	: m_vecProperties()
{}

/*virtual*/ COWLPropertyCollection::~COWLPropertyCollection()
{
	for (auto pProperty : m_vecProperties)
	{
		delete pProperty;
	}
}

// ************************************************************************************************
COWLPropertyProvider::COWLPropertyProvider()
	: m_mapPropertyCollections()
{}

/*virtual*/ COWLPropertyProvider::~COWLPropertyProvider()
{
	for (auto itPropertyCollection : m_mapPropertyCollections)
	{
		delete itPropertyCollection.second;
	}
}

COWLPropertyCollection* COWLPropertyProvider::GetPropertyCollection(OwlInstance iInstance)
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

COWLPropertyCollection* COWLPropertyProvider::LoadPropertyCollection(OwlInstance iInstance)
{
	if (iInstance == 0)
	{
		ASSERT(FALSE);

		return nullptr;
	}

	auto propertyCollection = new COWLPropertyCollection();

	RdfProperty iPropertyInstance = GetInstancePropertyByIterator(iInstance, 0);
	while (iPropertyInstance != 0)
	{
		propertyCollection->Properties().push_back(new COWLProperty(iPropertyInstance));

		iPropertyInstance = GetInstancePropertyByIterator(iInstance, iPropertyInstance);
	}

	return propertyCollection;
}