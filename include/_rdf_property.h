#pragma once

#include <string>
#include <vector>
#include <map>
using namespace std;

// ************************************************************************************************
class _rdf_property
{

private: // Members	

	RdfProperty m_rdfProperty;

public: // Methods

	_rdf_property(RdfProperty rdfProperty);
	virtual ~_rdf_property();

	RdfProperty GetInstance() const { return m_rdfProperty; }
	RdfPropertyType GetType() const { return GetType(m_rdfProperty); }
	static RdfPropertyType GetType(RdfProperty rdfProperty) { return GetPropertyType(rdfProperty); }
	wstring GetTypeName() { return GetTypeName(GetType()); }
	static wstring GetTypeName(RdfPropertyType iType);
	wchar_t* GetName() const;

	wstring GetRange(vector<int64_t>& vecRestrictionClasses) const { return GetRange(m_rdfProperty, vecRestrictionClasses); }
	static wstring GetRange(RdfProperty rdfProperty, vector<OwlClass>& vecRestrictionClasses);
	wstring GetCardinality(int64_t rdfProperty) const { return GetCardinality(rdfProperty, GetInstance()); }
	static wstring GetCardinality(OwlInstance owlInstance, RdfProperty rdfProperty);
};

// ************************************************************************************************
class _rdf_property_collection
{

private: // Members

	vector<_rdf_property*> m_vecProperties;

public: // Methods

	_rdf_property_collection();
	virtual ~_rdf_property_collection();

	vector<_rdf_property*>& Properties() { return m_vecProperties; }
};

// ************************************************************************************************
class _rdf_property_provider
{

private: // Members

	map<OwlInstance, _rdf_property_collection*> m_mapPropertyCollections;

public: // Methods

	_rdf_property_provider();
	virtual ~_rdf_property_provider();

	_rdf_property_collection* GetPropertyCollection(OwlInstance iInstance);

private: // Methods

	_rdf_property_collection* LoadPropertyCollection(OwlInstance iInstance);
};