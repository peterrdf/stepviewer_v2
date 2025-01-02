#include "stdafx.h"
#include "_rdf_property.h"

// ************************************************************************************************
_rdf_property::_rdf_property(RdfProperty rdfProperty)
	: m_rdfProperty(rdfProperty)
{}

/*virtual*/ _rdf_property::~_rdf_property()
{}

/*static*/ wstring _rdf_property::GetTypeName(RdfPropertyType iType)
{
	wstring strTypeName = iType == OBJECTPROPERTY_TYPE ?
		L"owl:ObjectProperty" :
		L"owl:DatatypeProperty";

	return strTypeName;
}

wchar_t* _rdf_property::GetName() const
{
	wchar_t* szName = nullptr;
	GetNameOfPropertyW(m_rdfProperty, &szName);

	return szName;
}

/*static*/ wstring _rdf_property::GetRange(RdfProperty rdfProperty, vector<OwlClass>& vecRestrictionClasses)
{
	wstring strRange = L"unknown";
	vecRestrictionClasses.clear();

	switch (GetType(rdfProperty))
	{
		case OBJECTTYPEPROPERTY_TYPE:
		{
			strRange = L"xsd:object";

			OwlClass iRestrictionClassInstance = GetRangeRestrictionsByIterator(rdfProperty, 0);
			while (iRestrictionClassInstance != 0)
			{
				vecRestrictionClasses.push_back(iRestrictionClassInstance);				

				iRestrictionClassInstance = GetRangeRestrictionsByIterator(rdfProperty, iRestrictionClassInstance);
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

/*static*/ wstring _rdf_property::GetCardinality(OwlInstance iInstance, RdfProperty iPropertyInstance)
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
_rdf_property_collection::_rdf_property_collection()
	: m_vecProperties()
{}

/*virtual*/ _rdf_property_collection::~_rdf_property_collection()
{
	for (auto pProperty : m_vecProperties)
	{
		delete pProperty;
	}
}

// ************************************************************************************************
_rdf_property_provider::_rdf_property_provider()
	: m_mapPropertyCollections()
{}

/*virtual*/ _rdf_property_provider::~_rdf_property_provider()
{
	for (auto itPropertyCollection : m_mapPropertyCollections)
	{
		delete itPropertyCollection.second;
	}
}

_rdf_property_collection* _rdf_property_provider::GetPropertyCollection(OwlInstance iInstance)
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

_rdf_property_collection* _rdf_property_provider::LoadPropertyCollection(OwlInstance iInstance)
{
	if (iInstance == 0)
	{
		ASSERT(FALSE);

		return nullptr;
	}

	auto propertyCollection = new _rdf_property_collection();

	RdfProperty iPropertyInstance = GetInstancePropertyByIterator(iInstance, 0);
	while (iPropertyInstance != 0)
	{
		propertyCollection->Properties().push_back(new _rdf_property(iPropertyInstance));

		iPropertyInstance = GetInstancePropertyByIterator(iInstance, iPropertyInstance);
	}

	return propertyCollection;
}