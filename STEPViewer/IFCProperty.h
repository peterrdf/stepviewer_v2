#pragma once

#include "IFCUnit.h"

#include <string>
#include <vector>
#include <map>

using namespace std;

// ------------------------------------------------------------------------------------------------
class CIFCProperty
{

private: // Members	

	wstring m_strName;
	wstring m_strValue;

public: // Methods

	CIFCProperty();
	virtual ~CIFCProperty();

	wstring GetName() const;
	wstring GetValue() const;
};

// ------------------------------------------------------------------------------------------------
class CIFCPropertySet
{

private: // Members	

	wstring m_strName;
	vector<CIFCProperty*> m_vecProperties;


public: // Methods

	CIFCPropertySet();
	virtual ~CIFCPropertySet();

	const vector<CIFCProperty*>& GetProperties() const;
};

// ------------------------------------------------------------------------------------------------
class CIFCPropertySetCollection
{

private: // Members

	vector<CIFCPropertySet*> m_vecPropertySets;


public: // Methods

	CIFCPropertySetCollection();
	virtual ~CIFCPropertySetCollection();

	const vector<CIFCPropertySet*> GetPropertySets() const;
};

// ------------------------------------------------------------------------------------------------
class CIFCPropertyProvider
{

private: // Members

	int64_t m_iModel;
	CIFCUnitProvider* m_pUnitProvider;
	map<int64_t, CIFCPropertySetCollection*> m_mapPropertyCollections;

public: // Methods

	CIFCPropertyProvider(int64_t iModel, CIFCUnitProvider* pUnitProvider);
	virtual ~CIFCPropertyProvider();

	CIFCPropertySetCollection* GetPropertPropertyCollection(int64_t iInstance);

protected: // Methods

	CIFCPropertySetCollection* LoadPropertPropertyCollection(int64_t iInstance);	

	void LoadProperties(int64_t iInstance);
	void LoadRelDefinesByProperties(int64_t iIFCIsDefinedByInstance);
	void LoadQuantites(int64_t iIFCPropertySetInstance);
	void LoadIFCQuantityLength(int_t iIFCQuantity);
	void LoadIFCQuantityArea(int_t iIFCQuantity);
	void LoadIFCQuantityVolume(int_t iIFCQuantity);
	void LoadIFCQuantityCount(int_t iIFCQuantity);
	void LoadIFCQuantityWeight(int_t iIFCQuantity);
	void LoadIFCQuantityTime(int_t iIFCQuantity);
	void LoadPropertySet(int64_t iIFCPropertySetInstance);
	void LoadRelDefinesByType(int64_t iIFCIsDefinedByInstance);
};