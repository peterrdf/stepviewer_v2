#include "stdafx.h"
#include "IFCProperty.h"
#include "IFCUnit.h"

// ------------------------------------------------------------------------------------------------
CIFCProperty::CIFCProperty(const wstring& strName, const wstring& strValue)
	: m_strName(strName)
	, m_strValue(strValue)
{
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ CIFCProperty::~CIFCProperty()
{
}

// ------------------------------------------------------------------------------------------------
/*static*/ bool CIFCProperty::HasProperties(int64_t iModel, int64_t iInstance)
{
	ASSERT(iModel != 0);
	ASSERT(iInstance != 0);

	int64_t* piIFCIsDefinedByInstances = 0;
	sdaiGetAttrBN(iInstance, "IsDefinedBy", sdaiAGGR, &piIFCIsDefinedByInstances);

	if (piIFCIsDefinedByInstances == nullptr)
	{
		return false;
	}

	int64_t iIFCIsDefinedByInstancesCount = sdaiGetMemberCount(piIFCIsDefinedByInstances);
	for (int64_t i = 0; i < iIFCIsDefinedByInstancesCount; ++i)
	{
		int64_t	iIFCIsDefinedByInstance = 0;
		engiGetAggrElement(piIFCIsDefinedByInstances, i, sdaiINSTANCE, &iIFCIsDefinedByInstance);

		if ((sdaiGetInstanceType(iIFCIsDefinedByInstance) == sdaiGetEntity(iModel, "IFCRELDEFINESBYPROPERTIES")) ||
			(sdaiGetInstanceType(iIFCIsDefinedByInstance) == sdaiGetEntity(iModel, "IFCRELDEFINESBYTYPE")))
		{
			return true;
		}
	} // for (int64_t i = ...

	return	false;
}
// ------------------------------------------------------------------------------------------------
/*static*/ wstring CIFCProperty::GetPropertySingleValue(int64_t iIFCPropertySingleValue)
{
	ASSERT(iIFCPropertySingleValue != 0);

	wchar_t* szNominalValueADB = nullptr;
	sdaiGetAttrBN(iIFCPropertySingleValue, "NominalValue", sdaiUNICODE, &szNominalValueADB);

	if (szNominalValueADB == nullptr)
	{
		return L"";
	}

	wchar_t* szUnitADB = nullptr;
	sdaiGetAttrBN(iIFCPropertySingleValue, "Unit", sdaiUNICODE, &szUnitADB);

	wchar_t* szTypePath = (wchar_t*)sdaiGetADBTypePath(szNominalValueADB, 0);
	if (szTypePath == nullptr)
	{
		return szNominalValueADB;
	}

	return L"";
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
CIFCPropertySet::CIFCPropertySet(const wstring& strName)
	: m_strName(strName)
	, m_vecProperties()
{
	ASSERT(!m_strName.empty());
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
wstring CIFCPropertySet::GetName() const
{
	return m_strName;
}

// ------------------------------------------------------------------------------------------------
vector<CIFCProperty*>& CIFCPropertySet::Properties()
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
vector<CIFCPropertySet*>& CIFCPropertySetCollection::PropertySets()
{
	return m_vecPropertySets;
}

// ------------------------------------------------------------------------------------------------
CIFCPropertyProvider::CIFCPropertyProvider(int64_t iModel, CIFCUnitProvider* pUnitProvider)
	: m_iModel(iModel)
	, m_pUnitProvider(pUnitProvider)
	, m_mapPropertyCollections()
{	
	ASSERT(m_iModel != 0);
	ASSERT(m_pUnitProvider != nullptr);
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
CIFCPropertySetCollection* CIFCPropertyProvider::GetPropertySetCollection(int64_t iInstance)
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
	auto pPropertySetCollection = new CIFCPropertySetCollection();
	LoadProperties(iInstance, pPropertySetCollection);

	return pPropertySetCollection;
}

// ------------------------------------------------------------------------------------------------
void CIFCPropertyProvider::LoadProperties(int64_t iInstance, CIFCPropertySetCollection* pPropertySetCollection)
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
			LoadRelDefinesByProperties(iIFCIsDefinedByInstance, pPropertySetCollection);
		}
		else
		{
			if (sdaiGetInstanceType(iIFCIsDefinedByInstance) == iIFCRelDefinesByTypeEntity)
			{
				LoadRelDefinesByType(iIFCIsDefinedByInstance, pPropertySetCollection);
			}
		}
	} // for (int64_t i = ...
}

// ------------------------------------------------------------------------------------------------
void CIFCPropertyProvider::LoadRelDefinesByProperties(int64_t iIFCIsDefinedByInstance, CIFCPropertySetCollection* pPropertySetCollection)
{
	ASSERT(iIFCIsDefinedByInstance != 0);

	const int64_t iIFCElementQuantityEntity = sdaiGetEntity(m_iModel, "IFCELEMENTQUANTITY");
	const int64_t iIFCPropertySetEntity = sdaiGetEntity(m_iModel, "IFCPROPERTYSET");

	int64_t iIFCPropertyDefinitionInstance = 0;
	sdaiGetAttrBN(iIFCIsDefinedByInstance, "RelatingPropertyDefinition", sdaiINSTANCE, &iIFCPropertyDefinitionInstance);

	if (sdaiGetInstanceType(iIFCPropertyDefinitionInstance) == iIFCElementQuantityEntity)
	{
		LoadQuantites(iIFCPropertyDefinitionInstance, pPropertySetCollection);
	}
	else
	{
		if (sdaiGetInstanceType(iIFCPropertyDefinitionInstance) == iIFCPropertySetEntity)
		{
			LoadPropertySet(iIFCPropertyDefinitionInstance, pPropertySetCollection);
		}
	}
}

// ------------------------------------------------------------------------------------------------
void CIFCPropertyProvider::LoadPropertySet(int64_t iIFCPropertySetInstance, CIFCPropertySetCollection* pPropertySetCollection)
{
	ASSERT(iIFCPropertySetInstance != 0);

	/*
	* Property set
	*/
	int64_t* piIFCHasPropertiesInstances = nullptr;
	sdaiGetAttrBN(iIFCPropertySetInstance, "HasProperties", sdaiAGGR, &piIFCHasPropertiesInstances);

	if (piIFCHasPropertiesInstances == nullptr)
	{
		return;
	}

	wstring strItem = GetPropertyName(iIFCPropertySetInstance);

	auto pPropertySet = new CIFCPropertySet(strItem);

	const int64_t iIFCPropertySingleValueEntity = sdaiGetEntity(m_iModel, "IFCPROPERTYSINGLEVALUE");

	int64_t iIFCHasPropertiesInstancesCount = sdaiGetMemberCount(piIFCHasPropertiesInstances);
	for (int64_t i = 0; i < iIFCHasPropertiesInstancesCount; ++i)
	{
		int64_t iIFCHasPropertiesInstance = 0;
		engiGetAggrElement(piIFCHasPropertiesInstances, i, sdaiINSTANCE, &iIFCHasPropertiesInstance);

		strItem = GetPropertyName(iIFCHasPropertiesInstance);

		wstring strValue;
		if (sdaiGetInstanceType(iIFCHasPropertiesInstance) == iIFCPropertySingleValueEntity)
		{
			strValue = CIFCProperty::GetPropertySingleValue(iIFCHasPropertiesInstance);
			if (strValue.empty())
			{
				strValue = L"<empty>";
			}
		}

		auto pProperty = new CIFCProperty(strItem, strValue);
		pPropertySet->Properties().push_back(pProperty);
	} // for  (int64_t i = ...

	pPropertySetCollection->PropertySets().push_back(pPropertySet);
}

// ------------------------------------------------------------------------------------------------
void CIFCPropertyProvider::LoadRelDefinesByType(int64_t iIFCIsDefinedByInstance, CIFCPropertySetCollection* pPropertySetCollection)
{
	ASSERT(iIFCIsDefinedByInstance != 0);

	int64_t iIFCRelatingType = 0;
	sdaiGetAttrBN(iIFCIsDefinedByInstance, "RelatingType", sdaiINSTANCE, &iIFCRelatingType);

	if (iIFCRelatingType == 0)
	{
		return;
	}

	const int64_t iIFCElementQuantityEntity = sdaiGetEntity(m_iModel, "IFCELEMENTQUANTITY");
	const int64_t iIFCPropertySetEntity = sdaiGetEntity(m_iModel, "IFCPROPERTYSET");

	int64_t* piIFCHasPropertySets = nullptr;
	sdaiGetAttrBN(iIFCRelatingType, "HasPropertySets", sdaiAGGR, &piIFCHasPropertySets);

	int64_t iIFCHasPropertySetsCount = sdaiGetMemberCount(piIFCHasPropertySets);
	for (int64_t i = 0; i < iIFCHasPropertySetsCount; ++i)
	{
		int64_t iIFCHasPropertySetInstance = 0;
		engiGetAggrElement(piIFCHasPropertySets, i, sdaiINSTANCE, &iIFCHasPropertySetInstance);
		if (sdaiGetInstanceType(iIFCHasPropertySetInstance) == iIFCElementQuantityEntity)
		{
			LoadQuantites(iIFCHasPropertySetInstance, pPropertySetCollection);
		}
		else
			if (sdaiGetInstanceType(iIFCHasPropertySetInstance) == iIFCPropertySetEntity)
			{
				LoadPropertySet(iIFCHasPropertySetInstance, pPropertySetCollection);
			}
			else
			{
				ASSERT(false);
			}
	} // for (int64_t i = ...
}

// ------------------------------------------------------------------------------------------------
void CIFCPropertyProvider::LoadQuantites(int64_t iIFCPropertySetInstance, CIFCPropertySetCollection* pPropertySetCollection)
{
	ASSERT(iIFCPropertySetInstance != 0);

	wstring strItem = GetPropertyName(iIFCPropertySetInstance);

	/*
	* Property set
	*/
	auto pPropertySet = new CIFCPropertySet(strItem);

	int64_t* piIFCQuantities = nullptr;
	sdaiGetAttrBN(iIFCPropertySetInstance, "Quantities", sdaiAGGR, &piIFCQuantities);

	int64_t iIFCQuantitiesCount = sdaiGetMemberCount(piIFCQuantities);
	for (int64_t i = 0; i < iIFCQuantitiesCount; ++i)
	{
		int64_t iIFCQuantityInstance = 0;
		engiGetAggrElement(piIFCQuantities, i, sdaiINSTANCE, &iIFCQuantityInstance);

		if (sdaiGetInstanceType(iIFCQuantityInstance) == sdaiGetEntity(m_iModel, "IFCQUANTITYLENGTH"))
		{
			LoadIFCQuantityLength(iIFCQuantityInstance, pPropertySet);
		}
		else if (sdaiGetInstanceType(iIFCQuantityInstance) == sdaiGetEntity(m_iModel, "IFCQUANTITYAREA"))
		{
			LoadIFCQuantityArea(iIFCQuantityInstance, pPropertySet);
		}
		else if (sdaiGetInstanceType(iIFCQuantityInstance) == sdaiGetEntity(m_iModel, "IFCQUANTITYVOLUME"))
		{
			LoadIFCQuantityVolume(iIFCQuantityInstance, pPropertySet);
		}
		else if (sdaiGetInstanceType(iIFCQuantityInstance) == sdaiGetEntity(m_iModel, "IFCQUANTITYCOUNT"))
		{
			LoadIFCQuantityCount(iIFCQuantityInstance, pPropertySet);
		} 
		else if (sdaiGetInstanceType(iIFCQuantityInstance) == sdaiGetEntity(m_iModel, "IFCQUANTITYWEIGHT"))
		{
			LoadIFCQuantityWeight(iIFCQuantityInstance, pPropertySet);
		}
		else if (sdaiGetInstanceType(iIFCQuantityInstance) == sdaiGetEntity(m_iModel, "IFCQUANTITYTIME"))
		{
			LoadIFCQuantityTime(iIFCQuantityInstance, pPropertySet);
		}
		else
		{
			ASSERT(FALSE); // TODO
		}
	} // for (int64_t i = ...

	pPropertySetCollection->PropertySets().push_back(pPropertySet);
}

// ------------------------------------------------------------------------------------------------
void CIFCPropertyProvider::LoadIFCQuantityLength(int_t iIFCQuantity, CIFCPropertySet* pPropertySet)
{
	auto prProperty = m_pUnitProvider->GetQuantityLength(iIFCQuantity);

	pPropertySet->Properties().push_back(new CIFCProperty(prProperty.first, prProperty.second));
}

// ------------------------------------------------------------------------------------------------
void CIFCPropertyProvider::LoadIFCQuantityArea(int_t iIFCQuantity, CIFCPropertySet* pPropertySet)
{
	auto prProperty = m_pUnitProvider->GetQuantityArea(iIFCQuantity);

	pPropertySet->Properties().push_back(new CIFCProperty(prProperty.first, prProperty.second));
}

// ------------------------------------------------------------------------------------------------
void CIFCPropertyProvider::LoadIFCQuantityVolume(int_t iIFCQuantity, CIFCPropertySet* pPropertySet)
{
	auto prProperty = m_pUnitProvider->GetQuantityVolume(iIFCQuantity);

	pPropertySet->Properties().push_back(new CIFCProperty(prProperty.first, prProperty.second));
}

// ------------------------------------------------------------------------------------------------
void CIFCPropertyProvider::LoadIFCQuantityCount(int_t iIFCQuantity, CIFCPropertySet* pPropertySet)
{
	auto prProperty = m_pUnitProvider->GetQuantityCount(iIFCQuantity);

	pPropertySet->Properties().push_back(new CIFCProperty(prProperty.first, prProperty.second));
}

// ------------------------------------------------------------------------------------------------
void CIFCPropertyProvider::LoadIFCQuantityWeight(int_t iIFCQuantity, CIFCPropertySet* pPropertySet)
{
	auto prProperty = m_pUnitProvider->GetQuantityWeight(iIFCQuantity);

	pPropertySet->Properties().push_back(new CIFCProperty(prProperty.first, prProperty.second));
}

// ------------------------------------------------------------------------------------------------
void CIFCPropertyProvider::LoadIFCQuantityTime(int_t iIFCQuantity, CIFCPropertySet* pPropertySet)
{
	auto prProperty = m_pUnitProvider->GetQuantityTime(iIFCQuantity);

	pPropertySet->Properties().push_back(new CIFCProperty(prProperty.first, prProperty.second));
}

// ------------------------------------------------------------------------------------------------
wstring CIFCPropertyProvider::GetPropertyName(int64_t iInstance) const
{
	ASSERT(iInstance != 0);

	wchar_t* szName = nullptr;
	sdaiGetAttrBN(iInstance, "Name", sdaiUNICODE, &szName);

	wchar_t* szDescription = nullptr;
	sdaiGetAttrBN(iInstance, "Description", sdaiUNICODE, &szDescription);

	wstring strName;
	if ((szName != nullptr) && (wcslen(szName) > 0))
	{
		strName = szName;
	}
	else
	{
		strName = L"<empty>";
	}

	if ((szDescription != nullptr) && (wcslen(szDescription) > 0))
	{
		strName += L" (";
		strName += szDescription;
		strName += L")";
	}

	return strName;
}