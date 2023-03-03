#include "stdafx.h"
#include "IFCProperty.h"

// ------------------------------------------------------------------------------------------------
CIFCProperty::CIFCProperty()
	: m_strName(L"")
	, m_strValue(L"")
{
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ CIFCProperty::~CIFCProperty()
{
}

// ------------------------------------------------------------------------------------------------
wstring CIFCProperty::GetName() const
{
	return m_strName;
}

// ------------------------------------------------------------------------------------------------
wstring CIFCProperty::GetValue() const
{
	return m_strValue;
}

// ------------------------------------------------------------------------------------------------
CIFCPropertySet::CIFCPropertySet()
	: m_vecProperties()
{
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ CIFCPropertySet::~CIFCPropertySet()
{
	for (auto pProperty : m_vecProperties)
	{
		delete pProperty;
	}
}

// ------------------------------------------------------------------------------------------------
const vector<CIFCProperty*>& CIFCPropertySet::GetProperties() const
{
	return m_vecProperties;
}

// ------------------------------------------------------------------------------------------------
CIFCPropertySetCollection::CIFCPropertySetCollection()
	: m_vecPropertySets()
{
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ CIFCPropertySetCollection::~CIFCPropertySetCollection()
{
	for (auto pPropertySet : m_vecPropertySets)
	{
		delete pPropertySet;
	}
}

// ------------------------------------------------------------------------------------------------
const vector<CIFCPropertySet*> CIFCPropertySetCollection::GetPropertySets() const
{
	return m_vecPropertySets;
}

// ------------------------------------------------------------------------------------------------
CIFCPropertyProvider::CIFCPropertyProvider(int64_t iModel)
	: m_iModel(iModel)
	, m_mapPropertyCollections()
{	
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ CIFCPropertyProvider::~CIFCPropertyProvider()
{
	for (auto itPropertyCollection : m_mapPropertyCollections)
	{
		delete itPropertyCollection.second;
	}
}

// ------------------------------------------------------------------------------------------------
CIFCPropertySetCollection* CIFCPropertyProvider::GetPropertPropertyCollection(int64_t iInstance)
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

	auto pPropertyCollection = LoadPropertPropertyCollection(iInstance);
	m_mapPropertyCollections[iInstance] = pPropertyCollection;

	return pPropertyCollection;
}

// ------------------------------------------------------------------------------------------------
CIFCPropertySetCollection* CIFCPropertyProvider::LoadPropertPropertyCollection(int64_t iInstance)
{
	ASSERT(FALSE); // todo

	return nullptr;
}

// ------------------------------------------------------------------------------------------------
void CIFCPropertyProvider::LoadProperties(int64_t iInstance)
{
	int64_t* piIFCIsDefinedByInstances = 0;
	sdaiGetAttrBN(iInstance, "IsDefinedBy", sdaiAGGR, &piIFCIsDefinedByInstances);

	if (piIFCIsDefinedByInstances == nullptr)
	{
		return;
	}

	const int64_t iIFCRelDefinesByTypeEntity = sdaiGetEntity(m_iModel, "IFCRELDEFINESBYTYPE");
	const int64_t iIFCRelDefinesByPropertiesEntity = sdaiGetEntity(m_iModel, "IFCRELDEFINESBYPROPERTIES");

	int64_t iIFCIsDefinedByInstancesCount = sdaiGetMemberCount(piIFCIsDefinedByInstances);
	for (int64_t i = 0; i < iIFCIsDefinedByInstancesCount; ++i)
	{
		int64_t	iIFCIsDefinedByInstance = 0;
		engiGetAggrElement(piIFCIsDefinedByInstances, i, sdaiINSTANCE, &iIFCIsDefinedByInstance);

		if (sdaiGetInstanceType(iIFCIsDefinedByInstance) == iIFCRelDefinesByPropertiesEntity)
		{
			LoadRelDefinesByProperties(iIFCIsDefinedByInstance);
		}
		else
		{
			if (sdaiGetInstanceType(iIFCIsDefinedByInstance) == iIFCRelDefinesByTypeEntity)
			{
				LoadRelDefinesByType(iIFCIsDefinedByInstance);
			}
		}
	} // for (int64_t i = ...
}

// ------------------------------------------------------------------------------------------------
void CIFCPropertyProvider::LoadRelDefinesByProperties(int64_t iIFCIsDefinedByInstance)
{
	ASSERT(iIFCIsDefinedByInstance != 0);

	const int64_t iIFCElementQuantityEntity = sdaiGetEntity(m_iModel, "IFCELEMENTQUANTITY");
	const int64_t iIFCPropertySetEntity = sdaiGetEntity(m_iModel, "IFCPROPERTYSET");

	int64_t iIFCPropertyDefinitionInstance = 0;
	sdaiGetAttrBN(iIFCIsDefinedByInstance, "RelatingPropertyDefinition", sdaiINSTANCE, &iIFCPropertyDefinitionInstance);

	if (sdaiGetInstanceType(iIFCPropertyDefinitionInstance) == iIFCElementQuantityEntity)
	{
		LoadQuantites(iIFCPropertyDefinitionInstance);
	}
	else
	{
		if (sdaiGetInstanceType(iIFCPropertyDefinitionInstance) == iIFCPropertySetEntity)
		{
			LoadPropertySet(iIFCPropertyDefinitionInstance);
		}
	}
}