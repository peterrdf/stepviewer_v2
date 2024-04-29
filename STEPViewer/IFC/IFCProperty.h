#pragma once

#include "IFCUnit.h"

#include <string>
#include <vector>
#include <map>

using namespace std;

// ************************************************************************************************
class CIFCProperty
{

private: // Members	

	wstring m_strName;
	wstring m_strValue;

public: // Methods

	CIFCProperty(const wstring& strName, const wstring& strValue);
	virtual ~CIFCProperty();

	static bool HasProperties(SdaiModel iModel, SdaiInstance iInstance);
	static wstring GetPropertySingleValue(SdaiInstance iPropertySingleValueInstance);

	wstring GetName() const;
	wstring GetValue() const;
};

// ************************************************************************************************
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

// ************************************************************************************************
class CIFCPropertySetCollection
{

private: // Members

	vector<CIFCPropertySet*> m_vecPropertySets;

public: // Methods

	CIFCPropertySetCollection();
	virtual ~CIFCPropertySetCollection();

	vector<CIFCPropertySet*>& PropertySets();
};

// ************************************************************************************************
class CIFCPropertyProvider
{

private: // Members

	SdaiModel m_iModel;
	CIFCUnitProvider* m_pUnitProvider;
	map<SdaiInstance, CIFCPropertySetCollection*> m_mapPropertyCollections;

public: // Methods

	CIFCPropertyProvider(SdaiModel SdaiModel, CIFCUnitProvider* pUnitProvider);
	virtual ~CIFCPropertyProvider();

	CIFCPropertySetCollection* GetPropertySetCollection(SdaiInstance iInstance);

private: // Methods

	CIFCPropertySetCollection* LoadPropertyCollection(SdaiInstance iInstance);

	void LoadProperties(SdaiInstance iInstance, CIFCPropertySetCollection* pPropertySetCollection);
	void LoadRelDefinesByProperties(SdaiInstance iIsDefinedByPropertiesInstance, CIFCPropertySetCollection* pPropertySetCollection);
	void LoadPropertySet(SdaiInstance iPropertySetInstance, CIFCPropertySetCollection* pPropertySetCollection);
	void LoadRelDefinesByType(SdaiInstance iRelDefinesByTypeInstance, CIFCPropertySetCollection* pPropertySetCollection);
	void LoadQuantites(SdaiInstance iElementQuantityInstance, CIFCPropertySetCollection* pPropertySetCollection);
	void LoadIFCQuantityLength(SdaiInstance iQuantityInstance, CIFCPropertySet* pPropertySet);
	void LoadIFCQuantityArea(SdaiInstance iQuantityInstance, CIFCPropertySet* pPropertySet);
	void LoadIFCQuantityVolume(SdaiInstance iQuantityInstance, CIFCPropertySet* pPropertySet);
	void LoadIFCQuantityCount(SdaiInstance iQuantityInstance, CIFCPropertySet* pPropertySet);
	void LoadIFCQuantityWeight(SdaiInstance iQuantityInstance, CIFCPropertySet* pPropertySet);
	void LoadIFCQuantityTime(SdaiInstance iQuantityInstance, CIFCPropertySet* pPropertySet);

	wstring GetPropertyName(SdaiInstance iInstance) const;
};