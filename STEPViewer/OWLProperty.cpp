#include "stdafx.h"
#include "OWLProperty.h"

// ------------------------------------------------------------------------------------------------
COWLProperty::COWLProperty(int64_t iInstance)
	: m_iInstance(iInstance)
{
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ COWLProperty::~COWLProperty()
{
}

// ------------------------------------------------------------------------------------------------
int64_t COWLProperty::GetInstance() const
{
	return m_iInstance;
}

// ------------------------------------------------------------------------------------------------
int64_t COWLProperty::GetType() const
{
	return GetPropertyType(GetInstance());
}

// ------------------------------------------------------------------------------------------------
wstring COWLProperty::GetTypeName()
{
	return GetTypeName(GetType());
}

// ------------------------------------------------------------------------------------------------
/*static*/ wstring COWLProperty::GetTypeName(int64_t iType)
{
	wstring strTypeName = iType == OBJECTPROPERTY_TYPE ?
		L"owl:ObjectProperty" :
		L"owl:DatatypeProperty";

	return strTypeName;
}

// ------------------------------------------------------------------------------------------------
wchar_t* COWLProperty::GetName() const
{
	wchar_t* szName = nullptr;
	GetNameOfPropertyW(GetInstance(), &szName);

	return szName;
}

// ------------------------------------------------------------------------------------------------
COWLPropertyCollection::COWLPropertyCollection()
	: m_vecProperties()
{
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ COWLPropertyCollection::~COWLPropertyCollection()
{
	for (auto pProperty : m_vecProperties)
	{
		delete pProperty;
	}
}

// ------------------------------------------------------------------------------------------------
vector<COWLProperty*>& COWLPropertyCollection::Properties()
{
	return m_vecProperties;
}

// ------------------------------------------------------------------------------------------------
COWLPropertyProvider::COWLPropertyProvider(int64_t iModel)
	: m_iModel(iModel)
	, m_mapPropertyCollections()
{	
	ASSERT(m_iModel != 0);
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ COWLPropertyProvider::~COWLPropertyProvider()
{
	for (auto itPropertyCollection : m_mapPropertyCollections)
	{
		delete itPropertyCollection.second;
	}
}

// ------------------------------------------------------------------------------------------------
COWLPropertyCollection* COWLPropertyProvider::GetPropertyCollection(int64_t iInstance)
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

// ------------------------------------------------------------------------------------------------
COWLPropertyCollection* COWLPropertyProvider::LoadPropertyCollection(int64_t iInstance)
{
	if (iInstance == 0)
	{
		ASSERT(FALSE);

		return nullptr;
	}

	auto propertyCollection = new COWLPropertyCollection();

	int64_t iPropertyInstance = GetInstancePropertyByIterator(iInstance, 0);
	while (iPropertyInstance != 0)
	{
		propertyCollection->Properties().push_back(new COWLProperty(iPropertyInstance));

		iPropertyInstance = GetInstancePropertyByIterator(iInstance, iPropertyInstance);
	}

	return propertyCollection;
}

// ------------------------------------------------------------------------------------------------
wstring COWLPropertyProvider::GetPropertyName(int64_t iPropertyInstance) const
{
	if (iPropertyInstance == 0)
	{
		ASSERT(FALSE);

		return L"";
	}

	wstring strName;

	wchar_t* szName = nullptr;
	sdaiGetAttrBN(iPropertyInstance, "Name", sdaiUNICODE, &szName);

	if ((szName != nullptr) && (wcslen(szName) > 0))
	{
		strName = szName;
	}
	else
	{
		strName = L"<empty>";
	}

	wchar_t* szDescription = nullptr;
	sdaiGetAttrBN(iPropertyInstance, "Description", sdaiUNICODE, &szDescription);

	if ((szDescription != nullptr) && (wcslen(szDescription) > 0))
	{
		strName += L" (";
		strName += szDescription;
		strName += L")";
	}

	return strName;
}