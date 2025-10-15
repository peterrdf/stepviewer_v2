#include "_host.h"
#include "_rdf_property.h"
#include "displayName.h"

// ************************************************************************************************
_rdf_property::_rdf_property(RdfProperty rdfProperty)
	: m_rdfProperty(rdfProperty)
	, m_strName(L"")
{
	assert(m_rdfProperty != 0);

	m_strName = DisplayName(m_rdfProperty);
}

/*virtual*/ _rdf_property::~_rdf_property()
{
}

/*static*/ wstring _rdf_property::getRangeAsString(RdfProperty rdfProperty)
{
	assert(rdfProperty != 0);

	wstring strRange = L"unknown";

	switch (getType(rdfProperty)) {
		case OBJECTTYPEPROPERTY_TYPE:
		{
			strRange = L"xsd:object";
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

		case DATATYPEPROPERTY_TYPE_CHAR_ARRAY:
		{
			strRange = L"xsd:string";
		}
		break;

		case DATATYPEPROPERTY_TYPE_WCHAR_T_ARRAY:
		{
			strRange = L"xsd:string";
		}
		break;

		case DATATYPEPROPERTY_TYPE_DOUBLE:
		{
			strRange = L"xsd:double";
		}
		break;

		case DATATYPEPROPERTY_TYPE_INTEGER:
		{
			strRange = L"xsd:integer";
		}
		break;

		case DATATYPEPROPERTY_TYPE_BYTE:
		{
			strRange = L"xsd:integer";
		}
		break;

		default:
		{
			assert(false);
		}
		break;
	} // switch (getType())

	return strRange;
}

/*static*/ wstring _rdf_property::getCardinality(OwlInstance owlInstance, RdfProperty rdfProperty)
{
	assert(owlInstance != 0);
	assert(rdfProperty != 0);

	int64_t iCard = 0;
	switch (getType(rdfProperty)) {
		case OBJECTPROPERTY_TYPE:
		{
			int64_t* piInstances = nullptr;
			GetObjectProperty(owlInstance, rdfProperty, &piInstances, &iCard);
		}
		break;

		case DATATYPEPROPERTY_TYPE_BOOLEAN:
		{
			bool* pbValue = nullptr;
			GetDatatypeProperty(owlInstance, rdfProperty, (void**)&pbValue, &iCard);
		}
		break;

		case DATATYPEPROPERTY_TYPE_STRING:
		{
			wchar_t** szValue = nullptr;
			SetCharacterSerialization(GetModel(owlInstance), 0, 0, false);
			GetDatatypeProperty(owlInstance, rdfProperty, (void**)&szValue, &iCard);
			SetCharacterSerialization(GetModel(owlInstance), 0, 0, true);
		}
		break;

		case DATATYPEPROPERTY_TYPE_CHAR_ARRAY:
		{
			char** szValue = nullptr;
			GetDatatypeProperty(owlInstance, rdfProperty, (void**)&szValue, &iCard);
		}
		break;

		case DATATYPEPROPERTY_TYPE_WCHAR_T_ARRAY:
		{
			wchar_t** szValue = nullptr;
			GetDatatypeProperty(owlInstance, rdfProperty, (void**)&szValue, &iCard);
		}
		break;

		case DATATYPEPROPERTY_TYPE_DOUBLE:
		{
			double* pdValue = nullptr;
			GetDatatypeProperty(owlInstance, rdfProperty, (void**)&pdValue, &iCard);
		}
		break;

		case DATATYPEPROPERTY_TYPE_INTEGER:
		{
			int64_t* piValue = nullptr;
			GetDatatypeProperty(owlInstance, rdfProperty, (void**)&piValue, &iCard);
		}
		break;

		default:
		{
			assert(false);
		}
		break;
	} // switch (getType(rdfProperty))

	int64_t	iMinCard = 0;
	int64_t iMaxCard = 0;
	getCardinalityRestriction(owlInstance, rdfProperty, iMinCard, iMaxCard);

	wchar_t szBuffer[100];
	if ((iMinCard == -1) && (iMaxCard == -1)) {
		swprintf(szBuffer, 100, L"%lld of [0 - infinity>", iCard);
	} else {
		if (iMaxCard == -1) {
			swprintf(szBuffer, 100, L"%lld of [%lld - infinity>", iCard, iMinCard);
		} else {
			swprintf(szBuffer, 100, L"%lld of [%lld - %lld]", iCard, iMinCard, iMaxCard);
		}
	}

	return szBuffer;
}

/*static*/ void _rdf_property::getCardinalityRestriction(OwlInstance owlInstance, RdfProperty rdfProperty, int64_t& iMinCard, int64_t& iMaxCard)
{
	assert(owlInstance != 0);
	assert(rdfProperty != 0);

	OwlClass owlClass = GetInstanceClass(owlInstance);
	assert(owlClass != 0);

	iMinCard = -1;
	iMaxCard = -1;
	GetClassPropertyAggregatedCardinalityRestriction(owlClass, rdfProperty, &iMinCard, &iMaxCard);
}

/*static*/ void _rdf_property::getRangeRestrictions(RdfProperty rdfProperty, vector<OwlClass>& vecRestrictionClasses)
{
	vecRestrictionClasses.clear();

	OwlClass owlClass = GetRangeRestrictionsByIterator(rdfProperty, 0);
	while (owlClass != 0) {
		vecRestrictionClasses.push_back(owlClass);

		owlClass = GetRangeRestrictionsByIterator(rdfProperty, owlClass);
	}
}

/*static*/ wstring _rdf_property::getTypeAsString(RdfPropertyType rdfPropertyType)
{
	wstring strTypeName = rdfPropertyType == OBJECTPROPERTY_TYPE ?
		L"owl:ObjectProperty" :
		L"owl:DatatypeProperty";

	return strTypeName;
}

const wchar_t* _rdf_property::getName() const
{
	return m_strName.c_str();
}

// ************************************************************************************************
_rdf_property_collection::_rdf_property_collection()
	: m_vecProperties()
{
}

/*virtual*/ _rdf_property_collection::~_rdf_property_collection()
{
	for (auto pProperty : m_vecProperties) {
		delete pProperty;
	}
}

// ************************************************************************************************
_rdf_property_provider::_rdf_property_provider()
	: m_mapPropertyCollections()
{
}

/*virtual*/ _rdf_property_provider::~_rdf_property_provider()
{
	for (auto itPropertyCollection : m_mapPropertyCollections) {
		delete itPropertyCollection.second;
	}
}

_rdf_property_collection* _rdf_property_provider::getPropertyCollection(OwlInstance owlInstance)
{
	if (owlInstance == 0) {
		assert(FALSE);

		return nullptr;
	}

	auto itPropertyCollection = m_mapPropertyCollections.find(owlInstance);
	if (itPropertyCollection != m_mapPropertyCollections.end()) {
		return itPropertyCollection->second;
	}

	auto pPropertyCollection = loadPropertyCollection(owlInstance);
	m_mapPropertyCollections[owlInstance] = pPropertyCollection;

	return pPropertyCollection;
}

_rdf_property_collection* _rdf_property_provider::loadPropertyCollection(OwlInstance owlInstance)
{
	if (owlInstance == 0) {
		assert(FALSE);

		return nullptr;
	}

	auto propertyCollection = new _rdf_property_collection();

	RdfProperty rdfProperty = GetInstancePropertyByIterator(owlInstance, 0);
	while (rdfProperty != 0) {
		propertyCollection->properties().push_back(new _rdf_property(rdfProperty));

		rdfProperty = GetInstancePropertyByIterator(owlInstance, rdfProperty);
	}

	return propertyCollection;
}
