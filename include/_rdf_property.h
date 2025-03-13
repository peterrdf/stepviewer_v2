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

	wstring getRangeAsString() const { return getRangeAsString(getRdfProperty()); }
	static wstring getRangeAsString(RdfProperty rdfProperty);
	wstring getCardinality(OwlInstance owlInstance) const { return getCardinality(owlInstance, getRdfProperty()); }
	static wstring getCardinality(OwlInstance owlInstance, RdfProperty rdfProperty);
	void getCardinalityRestriction(OwlInstance owlInstance, int64_t& iMinCard, int64_t& iMaxCard) const { return getCardinalityRestriction(owlInstance, getRdfProperty(), iMinCard, iMaxCard); }
	static void getCardinalityRestriction(OwlInstance owlInstance, RdfProperty rdfProperty, int64_t& iMinCard, int64_t& iMaxCard);
	void getRangeRestrictions(vector<OwlClass>& vecRestrictionClasses) const { getRangeRestrictions(getRdfProperty(), vecRestrictionClasses); };
	static void getRangeRestrictions(RdfProperty rdfProperty, vector<OwlClass>& vecRestrictionClasses);

public: // Properties

	RdfProperty getRdfProperty() const { return m_rdfProperty; }
	RdfPropertyType getType() const { return getType(m_rdfProperty); }
	static RdfPropertyType getType(RdfProperty rdfProperty) { return GetPropertyType(rdfProperty); }
	wstring getTypeAsString() { return getTypeAsString(getType()); }
	static wstring getTypeAsString(RdfPropertyType rdfPropertyType);
	const wchar_t* getName() const;
};

// ************************************************************************************************
class _rdf_property_collection
{

private: // Members

	vector<_rdf_property*> m_vecProperties;

public: // Methods

	_rdf_property_collection();
	virtual ~_rdf_property_collection();

public: // Properties

	vector<_rdf_property*>& properties() { return m_vecProperties; }
};

// ************************************************************************************************
class _rdf_property_provider
{

private: // Members

	map<OwlInstance, _rdf_property_collection*> m_mapPropertyCollections;

public: // Methods

	_rdf_property_provider();
	virtual ~_rdf_property_provider();

	_rdf_property_collection* getPropertyCollection(OwlInstance owlInstance);

private: // Methods

	_rdf_property_collection* loadPropertyCollection(OwlInstance owlInstance);
};
