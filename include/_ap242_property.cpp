#include "_host.h"
#include "_ap242_property.h"
#include "_ap_geometry.h"

#include <cwctype>
#include <algorithm>
using namespace std;

// ************************************************************************************************
_ap242_property::_ap242_property(SdaiInstance sdaiInstance, const wstring& strName, const wstring& strValue)
	: m_sdaiInstance(sdaiInstance)
	, m_strEntityName(_ap_geometry::getEntityName(sdaiInstance))
	, m_strName(strName)
	, m_strValue(strValue)
{
	assert(!m_strName.empty());
	assert(m_sdaiInstance != 0);
}

/*virtual*/ _ap242_property::~_ap242_property()
{}

/*static*/ wstring _ap242_property::getPropertySingleValue(SdaiInstance sdaiPropertySingleValueInstance)
{
	assert(sdaiPropertySingleValueInstance != 0);

	wchar_t* szNominalValueADB = nullptr;
	sdaiGetAttrBN(sdaiPropertySingleValueInstance, "NominalValue", sdaiUNICODE, &szNominalValueADB);

	return szNominalValueADB != nullptr ? szNominalValueADB : L"";
}

/*static*/ pair<wstring, wstring> _ap242_property::getValueTypes(SdaiInstance sdaiInstance)
{
	wstring strIfcValueType;
	wstring strValueType;

	wchar_t* szEntityName = nullptr;
	engiGetEntityName(sdaiGetInstanceType(sdaiInstance), sdaiUNICODE, (const char**)&szEntityName);

	wstring strEntity = szEntityName;
	std::transform(strEntity.begin(), strEntity.end(), strEntity.begin(), ::towupper);
	if (strEntity == L"IFCPROPERTYSINGLEVALUE") {
		SdaiAttr sdaiNominalValueAttr = sdaiGetAttrDefinition(sdaiGetInstanceType(sdaiInstance), "NominalValue");
		assert(sdaiNominalValueAttr != nullptr);

		SdaiPrimitiveType sdaiPrimitiveType = engiGetAttrType(sdaiNominalValueAttr);
		if ((sdaiPrimitiveType & engiTypeFlagAggr) ||
			(sdaiPrimitiveType & engiTypeFlagAggrOption)) {
			sdaiPrimitiveType = sdaiAGGR;
		}

		SdaiADB pADB = nullptr;
		if (sdaiGetAttr(
			sdaiInstance,
			sdaiNominalValueAttr,
			sdaiPrimitiveType,
			&pADB)) {
			strIfcValueType = (const wchar_t*)sdaiGetADBTypePath(pADB, sdaiUNICODE);

			SdaiPrimitiveType adbType = sdaiGetADBType(pADB);
			if ((adbType == sdaiINTEGER) ||
				(adbType == sdaiREAL) ||
				(adbType == sdaiNUMBER)) {
				strValueType = L"number";
			}
			else {
				if (adbType == sdaiSTRING) {
					strValueType = L"string";
				}
				else if (adbType == sdaiBOOLEAN) {
					strValueType = L"boolean";
				}
				else if (adbType == sdaiENUM) {
					strValueType = L"enum";
				}
				else {
					strValueType = L"object"; // complex type
				}
			}
		}
		else {
			assert(false); // Failed to get NominalValue attribute
		}
	} // if (strEntity == "IFCPROPERTYSINGLEVALUE")
	else {
		assert(sdaiIsKindOfBN(sdaiInstance, "IfcPhysicalQuantity"));

		strIfcValueType = szEntityName;
		strValueType = L"number";
	}

	return pair <wstring, wstring>(strIfcValueType, strValueType);
}

// ************************************************************************************************
_ap242_property_collection::_ap242_property_collection()
	: m_vecProperties()
{}

/*virtual*/ _ap242_property_collection::~_ap242_property_collection()
{
	for (auto pProperty : m_vecProperties) {
		delete pProperty;
	}
}

// ************************************************************************************************
_ap242_property_provider::_ap242_property_provider(SdaiModel sdaiModel)
	: m_sdaiModel(sdaiModel)
	, m_mapPropertyCollections()
{
	assert(m_sdaiModel != 0);
}

/*virtual*/ _ap242_property_provider::~_ap242_property_provider()
{
	for (auto itPropertyCollection : m_mapPropertyCollections) {
		delete itPropertyCollection.second;
	}
}

_ap242_property_collection* _ap242_property_provider::getPropertyCollection(SdaiInstance sdaiInstance)
{
	if (sdaiInstance == 0) {
		assert(false);
		return nullptr;
	}

	auto itPropertyCollection = m_mapPropertyCollections.find(sdaiInstance);
	if (itPropertyCollection != m_mapPropertyCollections.end()) {
		return itPropertyCollection->second;
	}

	auto pPropertyCollection = loadPropertyCollection(sdaiInstance);
	m_mapPropertyCollections[sdaiInstance] = pPropertyCollection;

	return pPropertyCollection;
}

_ap242_property_collection* _ap242_property_provider::loadPropertyCollection(SdaiInstance sdaiInstance)
{
	auto pPropertyCollection = new _ap242_property_collection();
	loadProperties(sdaiInstance, pPropertyCollection);

	return pPropertyCollection;
}

void _ap242_property_provider::loadProperties(SdaiInstance sdaiInstance, _ap242_property_collection* pPropertyCollection)
{
	assert(sdaiInstance != 0);

	SdaiAggr sdaiPropertyDefinitionAggr = sdaiGetEntityExtentBN(m_sdaiModel, "PROPERTY_DEFINITION");
	SdaiInteger iPropertyDefinitionsCount = sdaiGetMemberCount(sdaiPropertyDefinitionAggr);
	for (SdaiInteger i = 0; i < iPropertyDefinitionsCount; i++) {
		SdaiInstance sdaiPropertyDefinitionInstance = 0;
		sdaiGetAggrByIndex(sdaiPropertyDefinitionAggr, i, sdaiINSTANCE, &sdaiPropertyDefinitionInstance);

		SdaiInstance sdaiDefinitionInstance = 0;
		sdaiGetAttrBN(sdaiPropertyDefinitionInstance, "definition", sdaiINSTANCE, &sdaiDefinitionInstance);
		if (sdaiDefinitionInstance == sdaiInstance) {
			char* szName = nullptr;
			sdaiGetAttrBN(sdaiPropertyDefinitionInstance, "name", sdaiSTRING, &szName);
			assert(szName != nullptr);

			CString strValue;
			strValue.Format(L"property (#%i = PROPERTY_DEFINITION( ... ))", (int)internalGetP21Line(sdaiPropertyDefinitionInstance));

			//auto pProperty = new _ap242_property(sdaiPropertyDefinitionInstance, szName, (LPCTSTR)strValue);

			/*auto pPropertyGroup = new CMFCPropertyGridProperty(strValue);
			pInstanceGroup->AddSubItem(pPropertyGroup);

			

			auto pProperty = new CMFCPropertyGridProperty(L"name", (_variant_t)szName, L"name");
			pProperty->AllowEdit(false);
			pPropertyGroup->AddSubItem(pProperty);

			char* szDescription = nullptr;
			sdaiGetAttrBN(sdaiPropertyDefinitionInstance, "description", sdaiSTRING, &szDescription);

			pProperty = new CMFCPropertyGridProperty(L"description", (_variant_t)szName, L"description");
			pProperty->AllowEdit(false);
			pPropertyGroup->AddSubItem(pProperty);*/

			//
			//	Lookup value (not using inverse relations)
			//
			SdaiAggr sdaiPropertyDefinitionRepresentationAggr = sdaiGetEntityExtentBN(m_sdaiModel, "PROPERTY_DEFINITION_REPRESENTATION");
			SdaiInteger	iPropertyDefinitionRepresentationsCount = sdaiGetMemberCount(sdaiPropertyDefinitionRepresentationAggr);
			for (SdaiInteger j = 0; j < iPropertyDefinitionRepresentationsCount; j++) {
				SdaiInstance sdaiPropertyDefinitionRepresentationInstance = 0;
				sdaiGetAggrByIndex(sdaiPropertyDefinitionRepresentationAggr, j, sdaiINSTANCE, &sdaiPropertyDefinitionRepresentationInstance);

				SdaiInstance sdaiPDRDefinitionInstance = 0;
				sdaiGetAttrBN(sdaiPropertyDefinitionRepresentationInstance, "definition", sdaiINSTANCE, &sdaiPDRDefinitionInstance);
				if (sdaiPDRDefinitionInstance == sdaiPropertyDefinitionInstance) {
					SdaiInstance sdaiUsedRepresentationInstance = 0;
					sdaiGetAttrBN(sdaiPropertyDefinitionRepresentationInstance, "used_representation", sdaiINSTANCE, &sdaiUsedRepresentationInstance);

					SdaiAggr sdaiItemsAggr = nullptr;
					sdaiGetAttrBN(sdaiUsedRepresentationInstance, "items", sdaiAGGR, &sdaiItemsAggr);
					SdaiInteger	iItemsCount = sdaiGetMemberCount(sdaiItemsAggr);
					for (SdaiInteger k = 0; k < iItemsCount; k++) {
						SdaiInstance sdaiItemInstance = 0;
						sdaiGetAggrByIndex(sdaiItemsAggr, k, sdaiINSTANCE, &sdaiItemInstance);

						if (sdaiGetInstanceType(sdaiItemInstance) == sdaiGetEntity(m_sdaiModel, "DESCRIPTIVE_REPRESENTATION_ITEM")) {
							/*szDescription = nullptr;
							sdaiGetAttrBN(sdaiItemInstance, "description", sdaiSTRING, &szDescription);

							pProperty = new CMFCPropertyGridProperty(L"value", (_variant_t)szDescription, L"value");
							pProperty->AllowEdit(false);
							pPropertyGroup->AddSubItem(pProperty);*/
						} // DESCRIPTIVE_REPRESENTATION_ITEM
						else if (sdaiGetInstanceType(sdaiItemInstance) == sdaiGetEntity(m_sdaiModel, "VALUE_REPRESENTATION_ITEM")) {
							SdaiADB sdaiValueComponentADB = nullptr;
							sdaiGetAttrBN(sdaiItemInstance, "value_component", sdaiADB, &sdaiValueComponentADB);

							const char* szTypePath = sdaiGetADBTypePath(sdaiValueComponentADB, 0);
							switch (sdaiGetADBType(sdaiValueComponentADB)) {
								case sdaiINTEGER:
									{
										SdaiInteger iValue = 0;
										sdaiGetADBValue(sdaiValueComponentADB, sdaiINTEGER, (void*)&iValue);

										strValue.Format(L"%i [%s]", (int)iValue, (LPCTSTR)CA2W(szTypePath));

										/*pProperty = new CMFCPropertyGridProperty(L"value", (_variant_t)(LPCTSTR)strValue, L"value");
										pProperty->AllowEdit(false);
										pPropertyGroup->AddSubItem(pProperty);*/
									}
									break;

								case sdaiREAL:
									{
										double dValue = 0;
										sdaiGetADBValue(sdaiValueComponentADB, sdaiREAL, (void*)&dValue);

										strValue.Format(L"%f [%s]", dValue, (LPCTSTR)CA2W(szTypePath));

										/*pProperty = new CMFCPropertyGridProperty(L"value", (_variant_t)(LPCTSTR)strValue, L"value");
										pProperty->AllowEdit(false);
										pPropertyGroup->AddSubItem(pProperty);*/
									}
									break;

								case sdaiSTRING:
									{
										char* szValue = nullptr;
										sdaiGetADBValue(sdaiValueComponentADB, sdaiSTRING, (void*)&szValue);

										strValue.Format(L"%s [%s]", (LPCWSTR)CA2W(szValue), (LPCTSTR)CA2W(szTypePath));

										/*pProperty = new CMFCPropertyGridProperty(L"value", (_variant_t)(LPCTSTR)strValue, L"value");
										pProperty->AllowEdit(false);
										pPropertyGroup->AddSubItem(pProperty);*/
									}
									break;

								default:
									assert(false);
									break;
							} // switch (sdaiGetADBType(valueComponentADB))
						} // VALUE_REPRESENTATION_ITEM
					} // for (SdaiInteger k = ...
				} // if (sdaiPDRDefinitionInstance == ...
			} // for (SdaiInteger j = ...
		} // if (sdaiDefinitionInstance == ... 
	} // for (SdaiInteger i = ...
}
