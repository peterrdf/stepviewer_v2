#pragma once

#include <string>
#include <vector>
#include <map>

using namespace std;

// ------------------------------------------------------------------------------------------------
class COWLProperty
{

private: // Members	

	int64_t m_iInstance;

public: // Methods

	COWLProperty(int64_t iInstance);
	virtual ~COWLProperty();

	int64_t GetInstance() const;

	int64_t GetType() const;
	static int64_t GetType(int64_t iInstance);

	wstring GetTypeName();
	static wstring GetTypeName(int64_t iType);

	wchar_t* GetName() const;

	wstring GetRange() const;
	static wstring GetRange(int64_t iInstance);
};

// ------------------------------------------------------------------------------------------------
class COWLPropertyCollection
{

private: // Members

	vector<COWLProperty*> m_vecProperties;

public: // Methods

	COWLPropertyCollection();
	virtual ~COWLPropertyCollection();

	vector<COWLProperty*>& Properties();
};

// ------------------------------------------------------------------------------------------------
class COWLPropertyProvider
{

private: // Members

	int64_t m_iModel;
	map<int64_t, COWLPropertyCollection*> m_mapPropertyCollections;

public: // Methods

	COWLPropertyProvider(int64_t iModel);
	virtual ~COWLPropertyProvider();

	COWLPropertyCollection* GetPropertyCollection(int64_t iInstance);

private: // Methods

	COWLPropertyCollection* LoadPropertyCollection(int64_t iInstance);

	wstring GetPropertyName(int64_t iPropertyInstance) const;
};