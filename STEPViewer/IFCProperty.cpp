#include "stdafx.h"
#include "IFCProperty.h"
#include "IFCUnit.h"

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

	return pPropertySetCollection;
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

// ------------------------------------------------------------------------------------------------
void CIFCPropertyProvider::LoadPropertySet(int64_t iIFCPropertySetInstance)
{
	ASSERT(iIFCPropertySetInstance != 0);

	wchar_t* szName = nullptr;
	sdaiGetAttrBN(iIFCPropertySetInstance, "Name", sdaiUNICODE, &szName);

	wchar_t* szDescription = nullptr;
	sdaiGetAttrBN(iIFCPropertySetInstance, "Description", sdaiUNICODE, &szDescription);

	wstring strItem;
	if ((szName != nullptr) && (wcslen(szName) > 0))
	{
		strItem = szName;
	}
	else
	{
		strItem = L"<empty>";
	}

	if ((szDescription != nullptr) && (wcslen(szDescription) > 0))
	{
		strItem += L" (";
		strItem += szDescription;
		strItem += L")";
	}

	/*
	* Property set
	*/
	/*TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = hParent;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY_SET;
	tvInsertStruct.item.lParam = NULL;

	HTREEITEM hPropertySet = m_pTreeView->InsertItem(&tvInsertStruct);*/

	int64_t* piIFCHasPropertiesInstances = nullptr;
	sdaiGetAttrBN(iIFCPropertySetInstance, "HasProperties", sdaiAGGR, &piIFCHasPropertiesInstances);

	if (piIFCHasPropertiesInstances == nullptr)
	{
		return;
	}

	const int64_t iIFCPropertySingleValueEntity = sdaiGetEntity(m_iModel, "IFCPROPERTYSINGLEVALUE");

	int64_t iIFCHasPropertiesInstancesCount = sdaiGetMemberCount(piIFCHasPropertiesInstances);
	for (int64_t i = 0; i < iIFCHasPropertiesInstancesCount; ++i)
	{
		int64_t iIFCHasPropertiesInstance = 0;
		engiGetAggrElement(piIFCHasPropertiesInstances, i, sdaiINSTANCE, &iIFCHasPropertiesInstance);

		szName = nullptr;
		sdaiGetAttrBN(iIFCHasPropertiesInstance, "Name", sdaiUNICODE, &szName);

		szDescription = nullptr;
		sdaiGetAttrBN(iIFCHasPropertiesInstance, "Description", sdaiUNICODE, &szDescription);

		strItem = L"";
		if ((szName != nullptr) && (wcslen(szName) > 0))
		{
			strItem = szName;
		}
		else
		{
			strItem = L"<empty>";
		}

		if ((szDescription != nullptr) && (wcslen(szDescription) > 0))
		{
			strItem += L" ('";
			strItem += szDescription;
			strItem += L"')";
		}

		if (sdaiGetInstanceType(iIFCHasPropertiesInstance) == iIFCPropertySingleValueEntity)
		{
			wstring strValue = CIFCUnit::GetPropertyValue(iIFCHasPropertiesInstance);
			if (strValue.empty())
			{
				strValue = L"<empty>";
			}

			strItem += L" = ";
			strItem += strValue;
		}

		/*tvInsertStruct.hParent = hPropertySet;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
		tvInsertStruct.item.lParam = NULL;

		m_pTreeView->InsertItem(&tvInsertStruct);*/
	} // for  (int64_t i = ...
}

// ------------------------------------------------------------------------------------------------
void CIFCPropertyProvider::LoadRelDefinesByType(int64_t iIFCIsDefinedByInstance)
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
			LoadQuantites(iIFCHasPropertySetInstance);
		}
		else
			if (sdaiGetInstanceType(iIFCHasPropertySetInstance) == iIFCPropertySetEntity)
			{
				LoadPropertySet(iIFCHasPropertySetInstance);
			}
			else
			{
				ASSERT(false);
			}
	} // for (int64_t i = ...
}

// ------------------------------------------------------------------------------------------------
void CIFCPropertyProvider::LoadQuantites(int64_t iIFCPropertySetInstance)
{
	ASSERT(iIFCPropertySetInstance != 0);

	wchar_t* szName = nullptr;
	sdaiGetAttrBN(iIFCPropertySetInstance, "Name", sdaiUNICODE, &szName);

	wchar_t* szDescription = nullptr;
	sdaiGetAttrBN(iIFCPropertySetInstance, "Description", sdaiUNICODE, &szDescription);

	wstring strItem;
	if ((szName != nullptr) && (wcslen(szName) > 0))
	{
		strItem = szName;
	}
	else
	{
		strItem = L"<empty>";
	}

	if ((szDescription != nullptr) && (wcslen(szDescription) > 0))
	{
		strItem += L" (";
		strItem += szDescription;
		strItem += L")";
	}

	/*
	* Property set
	*/
	/*TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = hParent;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY_SET;
	tvInsertStruct.item.lParam = NULL;

	HTREEITEM hPropertySet = m_pTreeView->InsertItem(&tvInsertStruct);*/

	const int64_t iIFCQuantityLength_TYPE = sdaiGetEntity(m_iModel, "IFCQUANTITYLENGTH");
	const int64_t iIFCQuantityArea_TYPE = sdaiGetEntity(m_iModel, "IFCQUANTITYAREA");
	const int64_t iIFCQuantityVolume_TYPE = sdaiGetEntity(m_iModel, "IFCQUANTITYVOLUME");
	const int64_t iIFCQuantityCount_TYPE = sdaiGetEntity(m_iModel, "IFCQUANTITYCOUNT");
	const int64_t iIFCQuantityWeigth_TYPE = sdaiGetEntity(m_iModel, "IFCQUANTITYWEIGHT");
	const int64_t iIFCQuantityTime_TYPE = sdaiGetEntity(m_iModel, "IFCQUANTITYTIME");

	int64_t* piIFCQuantities = nullptr;
	sdaiGetAttrBN(iIFCPropertySetInstance, "Quantities", sdaiAGGR, &piIFCQuantities);

	int64_t iIFCQuantitiesCount = sdaiGetMemberCount(piIFCQuantities);
	for (int64_t i = 0; i < iIFCQuantitiesCount; ++i)
	{
		int64_t iIFCQuantityInstance = 0;
		engiGetAggrElement(piIFCQuantities, i, sdaiINSTANCE, &iIFCQuantityInstance);

		if (sdaiGetInstanceType(iIFCQuantityInstance) == iIFCQuantityLength_TYPE)
		{
			LoadIFCQuantityLength(iIFCQuantityInstance);
		}
		else
			if (sdaiGetInstanceType(iIFCQuantityInstance) == iIFCQuantityArea_TYPE)
			{
				LoadIFCQuantityArea(iIFCQuantityInstance);
			}
			else
				if (sdaiGetInstanceType(iIFCQuantityInstance) == iIFCQuantityVolume_TYPE)
				{
					LoadIFCQuantityVolume(iIFCQuantityInstance);
				}
				else
					if (sdaiGetInstanceType(iIFCQuantityInstance) == iIFCQuantityCount_TYPE)
					{
						LoadIFCQuantityCount(iIFCQuantityInstance);
					}
					else
						if (sdaiGetInstanceType(iIFCQuantityInstance) == iIFCQuantityWeigth_TYPE)
						{
							LoadIFCQuantityWeight(iIFCQuantityInstance);
						}
						else
							if (sdaiGetInstanceType(iIFCQuantityInstance) == iIFCQuantityTime_TYPE)
							{
								LoadIFCQuantityTime(iIFCQuantityInstance);
							}
							else
							{
								ASSERT(FALSE); // TODO
							}
	} // for (int64_t i = ...
}

// ------------------------------------------------------------------------------------------------
void CIFCPropertyProvider::LoadIFCQuantityLength(int_t iIFCQuantity)
{
	wstring strQuantity = m_pUnitProvider->GetQuantityLength(iIFCQuantity);
	//pModel->LoadIFCQuantityLength(iIFCQuantity, strQuantity);

	/*
	* Quantity
	*/
	/*TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = hParent;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = (LPWSTR)strQuantity.c_str();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
	tvInsertStruct.item.lParam = NULL;

	m_pTreeView->InsertItem(&tvInsertStruct);*/
}

// ------------------------------------------------------------------------------------------------
void CIFCPropertyProvider::LoadIFCQuantityArea(int_t iIFCQuantity)
{
	wstring strQuantity = m_pUnitProvider->GetQuantityArea(iIFCQuantity);
}

// ------------------------------------------------------------------------------------------------
void CIFCPropertyProvider::LoadIFCQuantityVolume(int_t iIFCQuantity)
{
	wstring strQuantity = m_pUnitProvider->GetQuantityVolume(iIFCQuantity);
}

// ------------------------------------------------------------------------------------------------
void CIFCPropertyProvider::LoadIFCQuantityCount(int_t iIFCQuantity)
{
	wstring strQuantity = m_pUnitProvider->GetQuantityCount(iIFCQuantity);
}

// ------------------------------------------------------------------------------------------------
void CIFCPropertyProvider::LoadIFCQuantityWeight(int_t iIFCQuantity)
{
	wstring strQuantity = m_pUnitProvider->GetQuantityWeight(iIFCQuantity);
}

// ------------------------------------------------------------------------------------------------
void CIFCPropertyProvider::LoadIFCQuantityTime(int_t iIFCQuantity)
{
	wstring strQuantity = m_pUnitProvider->GetQuantityTime(iIFCQuantity);
}
