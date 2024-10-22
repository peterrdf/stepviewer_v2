#pragma once

#include <string>
#include <vector>
#include <map>

using namespace std;

// ************************************************************************************************
class COWLProperty
{

private: // Members	

	RdfProperty m_iInstance;

public: // Methods

	COWLProperty(RdfProperty iInstance);
	virtual ~COWLProperty();

	RdfProperty GetInstance() const { return m_iInstance; }
	RdfPropertyType GetType() const { return GetType(m_iInstance); }
	static RdfPropertyType GetType(RdfProperty iInstance) { return GetPropertyType(iInstance); }
	wstring GetTypeName() { return GetTypeName(GetType()); }
	static wstring GetTypeName(RdfPropertyType iType);
	wchar_t* GetName() const;

	wstring GetRange(vector<int64_t>& vecRestrictionClasses) const { return GetRange(m_iInstance, vecRestrictionClasses); }
	static wstring GetRange(RdfProperty iInstance, vector<OwlClass>& vecRestrictionClasses);
	wstring GetCardinality(int64_t iInstance) const { return GetCardinality(iInstance, GetInstance()); }
	static wstring GetCardinality(OwlInstance iInstance, RdfProperty iPropertyInstance);
};

// ************************************************************************************************
class COWLPropertyCollection
{

private: // Members

	vector<COWLProperty*> m_vecProperties;

public: // Methods

	COWLPropertyCollection();
	virtual ~COWLPropertyCollection();

	vector<COWLProperty*>& Properties() { return m_vecProperties; }
};

// ************************************************************************************************
class COWLPropertyProvider
{

private: // Members

	map<OwlInstance, COWLPropertyCollection*> m_mapPropertyCollections;

public: // Methods

	COWLPropertyProvider();
	virtual ~COWLPropertyProvider();

	COWLPropertyCollection* GetPropertyCollection(OwlInstance iInstance);

private: // Methods

	COWLPropertyCollection* LoadPropertyCollection(OwlInstance iInstance);
};