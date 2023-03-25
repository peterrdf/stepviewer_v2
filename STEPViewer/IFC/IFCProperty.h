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

	CIFCProperty(const wstring& strName, const wstring& strValue);
	virtual ~CIFCProperty();

	static bool HasProperties(int64_t iModel, int64_t iInstance);
	static wstring GetPropertySingleValue(int64_t iIFCPropertySingleValue);

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

	CIFCPropertySet(const wstring& strName);
	virtual ~CIFCPropertySet();

	wstring GetName() const;
	vector<CIFCProperty*>& Properties();
};

// ------------------------------------------------------------------------------------------------
class CIFCPropertySetCollection
{

private: // Members

	vector<CIFCPropertySet*> m_vecPropertySets;

public: // Methods

	CIFCPropertySetCollection();
	virtual ~CIFCPropertySetCollection();

	vector<CIFCPropertySet*>& PropertySets();
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

	CIFCPropertySetCollection* GetPropertySetCollection(int64_t iInstance);

private: // Methods

	CIFCPropertySetCollection* LoadPropertyCollection(int64_t iInstance);

	void LoadProperties(int64_t iInstance, CIFCPropertySetCollection* pPropertySetCollection);
	void LoadRelDefinesByProperties(int64_t iIFCIsDefinedByInstance, CIFCPropertySetCollection* pPropertySetCollection);
	void LoadPropertySet(int64_t iIFCPropertySetInstance, CIFCPropertySetCollection* pPropertySetCollection);
	void LoadRelDefinesByType(int64_t iIFCIsDefinedByInstance, CIFCPropertySetCollection* pPropertySetCollection);
	void LoadQuantites(int64_t iIFCPropertySetInstance, CIFCPropertySetCollection* pPropertySetCollection);
	void LoadIFCQuantityLength(int_t iIFCQuantity, CIFCPropertySet* pPropertySet);
	void LoadIFCQuantityArea(int_t iIFCQuantity, CIFCPropertySet* pPropertySet);
	void LoadIFCQuantityVolume(int_t iIFCQuantity, CIFCPropertySet* pPropertySet);
	void LoadIFCQuantityCount(int_t iIFCQuantity, CIFCPropertySet* pPropertySet);
	void LoadIFCQuantityWeight(int_t iIFCQuantity, CIFCPropertySet* pPropertySet);
	void LoadIFCQuantityTime(int_t iIFCQuantity, CIFCPropertySet* pPropertySet);	

	wstring GetPropertyName(int64_t iInstance) const;
};