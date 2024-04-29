#include "stdafx.h"
#include "IFCProperty.h"
#include "IFCUnit.h"

// ************************************************************************************************
CIFCProperty::CIFCProperty(const wstring& strName, const wstring& strValue)
	: m_strName(strName)
	, m_strValue(strValue)
{}

/*virtual*/ CIFCProperty::~CIFCProperty()
{}

/*static*/ bool CIFCProperty::HasProperties(SdaiModel iModel, SdaiInstance iInstance)
{
	ASSERT(iModel != 0);
	ASSERT(iInstance != 0);

	SdaiAggr pIsDefinedByAggr = 0;
	sdaiGetAttrBN(iInstance, "IsDefinedBy", sdaiAGGR, &pIsDefinedByAggr);

	if (pIsDefinedByAggr == nullptr)
	{
		return false;
	}

	SdaiInteger iMembersCount = sdaiGetMemberCount(pIsDefinedByAggr);
	for (SdaiInteger iMember = 0; iMember < iMembersCount; iMember++)
	{
		SdaiInstance iAggrInstance = 0;
		engiGetAggrElement(pIsDefinedByAggr, iMember, sdaiINSTANCE, &iAggrInstance);

		if ((sdaiGetInstanceType(iAggrInstance) == sdaiGetEntity(iModel, "IFCRELDEFINESBYPROPERTIES")) ||
			(sdaiGetInstanceType(iAggrInstance) == sdaiGetEntity(iModel, "IFCRELDEFINESBYTYPE")))
		{
			return true;
		}
	} // for (SdaiInteger iMember = ...

	return	false;
}

/*static*/ wstring CIFCProperty::GetPropertySingleValue(SdaiInstance iPropertySingleValueInstance)
{
	ASSERT(iPropertySingleValueInstance != 0);

	wchar_t* szNominalValueADB = nullptr;
	sdaiGetAttrBN(iPropertySingleValueInstance, "NominalValue", sdaiUNICODE, &szNominalValueADB);

	if (szNominalValueADB == nullptr)
	{
		return L"";
	}

	wchar_t* szUnitADB = nullptr;
	sdaiGetAttrBN(iPropertySingleValueInstance, "Unit", sdaiUNICODE, &szUnitADB);

	wchar_t* szTypePath = (wchar_t*)sdaiGetADBTypePath(szNominalValueADB, 0);
	if (szTypePath == nullptr)
	{
		return szNominalValueADB;
	}

	return L"";
}

wstring CIFCProperty::GetName() const
{
	return m_strName;
}

wstring CIFCProperty::GetValue() const
{
	return m_strValue;
}

// ************************************************************************************************
CIFCPropertySet::CIFCPropertySet(const wstring& strName)
	: m_strName(strName)
	, m_vecProperties()
{
	ASSERT(!m_strName.empty());
}

/*virtual*/ CIFCPropertySet::~CIFCPropertySet()
{
	for (auto pProperty : m_vecProperties)
	{
		delete pProperty;
	}
}

wstring CIFCPropertySet::GetName() const
{
	return m_strName;
}

vector<CIFCProperty*>& CIFCPropertySet::Properties()
{
	return m_vecProperties;
}

CIFCPropertySetCollection::CIFCPropertySetCollection()
	: m_vecPropertySets()
{
}

/*virtual*/ CIFCPropertySetCollection::~CIFCPropertySetCollection()
{
	for (auto pPropertySet : m_vecPropertySets)
	{
		delete pPropertySet;
	}
}

vector<CIFCPropertySet*>& CIFCPropertySetCollection::PropertySets()
{
	return m_vecPropertySets;
}

// ************************************************************************************************
CIFCPropertyProvider::CIFCPropertyProvider(SdaiModel iModel, CIFCUnitProvider* pUnitProvider)
	: m_iModel(iModel)
	, m_pUnitProvider(pUnitProvider)
	, m_mapPropertyCollections()
{	
	ASSERT(m_iModel != 0);
	ASSERT(m_pUnitProvider != nullptr);
}

/*virtual*/ CIFCPropertyProvider::~CIFCPropertyProvider()
{
	for (auto itPropertyCollection : m_mapPropertyCollections)
	{
		delete itPropertyCollection.second;
	}
}

CIFCPropertySetCollection* CIFCPropertyProvider::GetPropertySetCollection(SdaiInstance iInstance)
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

CIFCPropertySetCollection* CIFCPropertyProvider::LoadPropertyCollection(SdaiInstance iInstance)
{
	auto pPropertySetCollection = new CIFCPropertySetCollection();
	LoadProperties(iInstance, pPropertySetCollection);

	return pPropertySetCollection;
}

void CIFCPropertyProvider::LoadProperties(SdaiInstance iInstance, CIFCPropertySetCollection* pPropertySetCollection)
{
	SdaiAggr pIsDefinedByAggr = nullptr;
	sdaiGetAttrBN(iInstance, "IsDefinedBy", sdaiAGGR, &pIsDefinedByAggr);

	if (pIsDefinedByAggr == nullptr)
	{
		return;
	}

	SdaiEntity iIFCRelDefinesByTypeEntity = sdaiGetEntity(m_iModel, "IFCRELDEFINESBYTYPE");
	SdaiEntity iIFCRelDefinesByPropertiesEntity = sdaiGetEntity(m_iModel, "IFCRELDEFINESBYPROPERTIES");

	SdaiInteger iMembersCount = sdaiGetMemberCount(pIsDefinedByAggr);
	for (SdaiInteger iMember = 0; iMember < iMembersCount; iMember++)
	{
		SdaiInstance iAggrInstance = 0;
		engiGetAggrElement(pIsDefinedByAggr, iMember, sdaiINSTANCE, &iAggrInstance);

		if (sdaiGetInstanceType(iAggrInstance) == iIFCRelDefinesByPropertiesEntity)
		{
			LoadRelDefinesByProperties(iAggrInstance, pPropertySetCollection);
		}
		else
		{
			if (sdaiGetInstanceType(iAggrInstance) == iIFCRelDefinesByTypeEntity)
			{
				LoadRelDefinesByType(iAggrInstance, pPropertySetCollection);
			}
		}
	} // for (SdaiInteger iMember = ...
}

void CIFCPropertyProvider::LoadRelDefinesByProperties(SdaiInstance iIsDefinedByPropertiesInstance, CIFCPropertySetCollection* pPropertySetCollection)
{
	ASSERT(iIsDefinedByPropertiesInstance != 0);

	SdaiEntity iIFCElementQuantityEntity = sdaiGetEntity(m_iModel, "IFCELEMENTQUANTITY");
	SdaiEntity iIFCPropertySetEntity = sdaiGetEntity(m_iModel, "IFCPROPERTYSET");

	SdaiInstance iRelatingPropertyDefinitionInstance = 0;
	sdaiGetAttrBN(iIsDefinedByPropertiesInstance, "RelatingPropertyDefinition", sdaiINSTANCE, &iRelatingPropertyDefinitionInstance);

	if (sdaiGetInstanceType(iRelatingPropertyDefinitionInstance) == iIFCElementQuantityEntity)
	{
		LoadQuantites(iRelatingPropertyDefinitionInstance, pPropertySetCollection);
	}
	else
	{
		if (sdaiGetInstanceType(iRelatingPropertyDefinitionInstance) == iIFCPropertySetEntity)
		{
			LoadPropertySet(iRelatingPropertyDefinitionInstance, pPropertySetCollection);
		}
	}
}

void CIFCPropertyProvider::LoadPropertySet(SdaiInstance iPropertySetInstance, CIFCPropertySetCollection* pPropertySetCollection)
{
	ASSERT(iPropertySetInstance != 0);

	/*
	* Property set
	*/
	SdaiAggr pHasPropertiesAggr = nullptr;
	sdaiGetAttrBN(iPropertySetInstance, "HasProperties", sdaiAGGR, &pHasPropertiesAggr);

	if (pHasPropertiesAggr == nullptr)
	{
		return;
	}

	wstring strItem = GetPropertyName(iPropertySetInstance);

	auto pPropertySet = new CIFCPropertySet(strItem);

	SdaiEntity iIFCPropertySingleValueEntity = sdaiGetEntity(m_iModel, "IFCPROPERTYSINGLEVALUE");

	SdaiInteger iMembersCount = sdaiGetMemberCount(pHasPropertiesAggr);
	for (SdaiInteger iMember = 0; iMember < iMembersCount; iMember++)
	{
		SdaiInstance iAggrInstance = 0;
		engiGetAggrElement(pHasPropertiesAggr, iMember, sdaiINSTANCE, &iAggrInstance);

		strItem = GetPropertyName(iAggrInstance);

		wstring strValue;
		if (sdaiGetInstanceType(iAggrInstance) == iIFCPropertySingleValueEntity)
		{
			strValue = CIFCProperty::GetPropertySingleValue(iAggrInstance);
			if (strValue.empty())
			{
				strValue = L"<empty>";
			}
		}

		auto pProperty = new CIFCProperty(strItem, strValue);
		pPropertySet->Properties().push_back(pProperty);
	} // for (SdaiInteger iMember = ...

	pPropertySetCollection->PropertySets().push_back(pPropertySet);
}

void CIFCPropertyProvider::LoadRelDefinesByType(SdaiInstance iRelDefinesByTypeInstance, CIFCPropertySetCollection* pPropertySetCollection)
{
	ASSERT(iRelDefinesByTypeInstance != 0);

	SdaiInstance iRelatingTypeInstance = 0;
	sdaiGetAttrBN(iRelDefinesByTypeInstance, "RelatingType", sdaiINSTANCE, &iRelatingTypeInstance);

	if (iRelatingTypeInstance == 0)
	{
		return;
	}

	SdaiEntity iIFCElementQuantityEntity = sdaiGetEntity(m_iModel, "IFCELEMENTQUANTITY");
	SdaiEntity iIFCPropertySetEntity = sdaiGetEntity(m_iModel, "IFCPROPERTYSET");

	SdaiAggr pHasPropertySetsAggr = nullptr;
	sdaiGetAttrBN(iRelatingTypeInstance, "HasPropertySets", sdaiAGGR, &pHasPropertySetsAggr);

	SdaiInteger iMembersCount = sdaiGetMemberCount(pHasPropertySetsAggr);
	for (SdaiInteger iMember = 0; iMember < iMembersCount; iMember++)
	{
		SdaiInstance iAggrInstance = 0;
		engiGetAggrElement(pHasPropertySetsAggr, iMember, sdaiINSTANCE, &iAggrInstance);
		if (sdaiGetInstanceType(iAggrInstance) == iIFCElementQuantityEntity)
		{
			LoadQuantites(iAggrInstance, pPropertySetCollection);
		}
		else if (sdaiGetInstanceType(iAggrInstance) == iIFCPropertySetEntity)
		{
			LoadPropertySet(iAggrInstance, pPropertySetCollection);
		}
	} // for (SdaiInteger iMember = ...
}

void CIFCPropertyProvider::LoadQuantites(SdaiInstance iElementQuantityInstance, CIFCPropertySetCollection* pPropertySetCollection)
{
	ASSERT(iElementQuantityInstance != 0);

	wstring strItem = GetPropertyName(iElementQuantityInstance);

	/*
	* Property set
	*/
	auto pPropertySet = new CIFCPropertySet(strItem);

	SdaiAggr pQuantitiesAggr = nullptr;
	sdaiGetAttrBN(iElementQuantityInstance, "Quantities", sdaiAGGR, &pQuantitiesAggr);

	SdaiInteger iMembersCount = sdaiGetMemberCount(pQuantitiesAggr);
	for (SdaiInteger iMember = 0; iMember < iMembersCount; iMember++)
	{
		SdaiInstance iAggrInstance = 0;
		engiGetAggrElement(pQuantitiesAggr, iMember, sdaiINSTANCE, &iAggrInstance);

		if (sdaiGetInstanceType(iAggrInstance) == sdaiGetEntity(m_iModel, "IFCQUANTITYLENGTH"))
		{
			LoadIFCQuantityLength(iAggrInstance, pPropertySet);
		}
		else if (sdaiGetInstanceType(iAggrInstance) == sdaiGetEntity(m_iModel, "IFCQUANTITYAREA"))
		{
			LoadIFCQuantityArea(iAggrInstance, pPropertySet);
		}
		else if (sdaiGetInstanceType(iAggrInstance) == sdaiGetEntity(m_iModel, "IFCQUANTITYVOLUME"))
		{
			LoadIFCQuantityVolume(iAggrInstance, pPropertySet);
		}
		else if (sdaiGetInstanceType(iAggrInstance) == sdaiGetEntity(m_iModel, "IFCQUANTITYCOUNT"))
		{
			LoadIFCQuantityCount(iAggrInstance, pPropertySet);
		} 
		else if (sdaiGetInstanceType(iAggrInstance) == sdaiGetEntity(m_iModel, "IFCQUANTITYWEIGHT"))
		{
			LoadIFCQuantityWeight(iAggrInstance, pPropertySet);
		}
		else if (sdaiGetInstanceType(iAggrInstance) == sdaiGetEntity(m_iModel, "IFCQUANTITYTIME"))
		{
			LoadIFCQuantityTime(iAggrInstance, pPropertySet);
		}
		else
		{
			ASSERT(FALSE); // TODO
		}
	} // for (SdaiInteger iMember = ...

	pPropertySetCollection->PropertySets().push_back(pPropertySet);
}

void CIFCPropertyProvider::LoadIFCQuantityLength(SdaiInstance iQuantityInstance, CIFCPropertySet* pPropertySet)
{
	auto prProperty = m_pUnitProvider->GetQuantityLength(iQuantityInstance);

	pPropertySet->Properties().push_back(new CIFCProperty(prProperty.first, prProperty.second));
}

void CIFCPropertyProvider::LoadIFCQuantityArea(SdaiInstance iQuantityInstance, CIFCPropertySet* pPropertySet)
{
	auto prProperty = m_pUnitProvider->GetQuantityArea(iQuantityInstance);

	pPropertySet->Properties().push_back(new CIFCProperty(prProperty.first, prProperty.second));
}

void CIFCPropertyProvider::LoadIFCQuantityVolume(SdaiInstance iQuantityInstance, CIFCPropertySet* pPropertySet)
{
	auto prProperty = m_pUnitProvider->GetQuantityVolume(iQuantityInstance);

	pPropertySet->Properties().push_back(new CIFCProperty(prProperty.first, prProperty.second));
}

void CIFCPropertyProvider::LoadIFCQuantityCount(SdaiInstance iQuantityInstance, CIFCPropertySet* pPropertySet)
{
	auto prProperty = m_pUnitProvider->GetQuantityCount(iQuantityInstance);

	pPropertySet->Properties().push_back(new CIFCProperty(prProperty.first, prProperty.second));
}

void CIFCPropertyProvider::LoadIFCQuantityWeight(SdaiInstance iQuantityInstance, CIFCPropertySet* pPropertySet)
{
	auto prProperty = m_pUnitProvider->GetQuantityWeight(iQuantityInstance);

	pPropertySet->Properties().push_back(new CIFCProperty(prProperty.first, prProperty.second));
}

void CIFCPropertyProvider::LoadIFCQuantityTime(SdaiInstance iQuantityInstance, CIFCPropertySet* pPropertySet)
{
	auto prProperty = m_pUnitProvider->GetQuantityTime(iQuantityInstance);

	pPropertySet->Properties().push_back(new CIFCProperty(prProperty.first, prProperty.second));
}

wstring CIFCPropertyProvider::GetPropertyName(SdaiInstance iInstance) const
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