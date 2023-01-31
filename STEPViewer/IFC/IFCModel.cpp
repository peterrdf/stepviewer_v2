#include "stdafx.h"

#include "_oglUtils.h"
#include "IFCModel.h"

#include <cwchar>
#include <cfloat>
#include <math.h>

// ------------------------------------------------------------------------------------------------
/*static*/ int_t CIFCModel::s_iObjectID = 1;

// ------------------------------------------------------------------------------------------------
CIFCModel::CIFCModel()
	: CSTEPModelBase(enumSTEPModelType::IFC)
	, m_strIFCFile(L"")
	, m_iModel(0)
	, m_ifcProjectEntity(0)
	, m_ifcSpaceEntity(0)
	, m_ifcOpeningElementEntity(0)
	, m_ifcDistributionElementEntity(0)
	, m_ifcElectricalElementEntity(0)
	, m_ifcElementAssemblyEntity(0)
	, m_ifcElementComponentEntity(0)
	, m_ifcEquipmentElementEntity(0)
	, m_ifcFeatureElementEntity(0)
	, m_ifcFeatureElementSubtractionEntity(0)
	, m_ifcFurnishingElementEntity(0)
	, m_ifcReinforcingElementEntity(0)
	, m_ifcTransportElementEntity(0)
	, m_ifcVirtualElementEntity(0)
	, m_fXmin(-1.f)
	, m_fXmax(1.f)
	, m_fYmin(-1.f)
	, m_fYmax(1.f)
	, m_fZmin(-1.f)
	, m_fZmax(1.f)
	, m_fBoundingSphereDiameter(1.f)
	, m_fXTranslation(0.f)
	, m_fYTranslation(0.f)
	, m_fZTranslation(0.f)
	, m_vecInstances()
	, m_mapInstances()
	, m_mapID2Instance()
	, m_mapExpressID2Instance()
	, m_mapGUID2Instance()
	, m_mapUnits()
	, m_mapEntities()
	, m_mapClasses()
	, m_mapProperties()
{
}

// ------------------------------------------------------------------------------------------------
CIFCModel::~CIFCModel()
{
	Clean();
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ int64_t CIFCModel::GetInstance() const
{
	return m_iModel;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CIFCModel::ZoomToInstance(int64_t /*iInstanceID*/)
{
	ASSERT(FALSE); // TODO
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::GetWorldDimensions(float& fXmin, float& fXmax, float& fYmin, float& fYmax, float& fZmin, float& fZmax) const
{
	fXmin = m_fXmin;
	fXmax = m_fXmax;
	fYmin = m_fYmin;
	fYmax = m_fYmax;
	fZmin = m_fZmin;
	fZmax = m_fZmax;
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::GetWorldTranslations(float& fXTranslation, float& fYTranslation, float& fZTranslation) const
{
	fXTranslation = m_fXTranslation;
	fYTranslation = m_fYTranslation;
	fZTranslation = m_fZTranslation;
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::Load(const wchar_t* szIFCFile, int64_t iModel)
{
	ASSERT(szIFCFile != NULL);
	ASSERT(iModel != 0);

	/*
	* Memory
	*/
	Clean();

	/*
	* Model
	*/
	m_iModel = iModel;
	m_strIFCFile = szIFCFile;

	/*
	* Entities
	*/
	int_t ifcObjectEntity = sdaiGetEntity(m_iModel, "IFCOBJECT");
	m_ifcProjectEntity = sdaiGetEntity(m_iModel, "IFCPROJECT");
	m_ifcSpaceEntity = sdaiGetEntity(m_iModel, "IFCSPACE");
	m_ifcOpeningElementEntity = sdaiGetEntity(m_iModel, "IFCOPENINGELEMENT");
	m_ifcDistributionElementEntity = sdaiGetEntity(m_iModel, "IFCDISTRIBUTIONELEMENT");
	m_ifcElectricalElementEntity = sdaiGetEntity(m_iModel, "IFCELECTRICALELEMENT");
	m_ifcElementAssemblyEntity = sdaiGetEntity(m_iModel, "IFCELEMENTASSEMBLY");
	m_ifcElementComponentEntity = sdaiGetEntity(m_iModel, "IFCELEMENTCOMPONENT");
	m_ifcEquipmentElementEntity = sdaiGetEntity(m_iModel, "IFCEQUIPMENTELEMENT");
	m_ifcFeatureElementEntity = sdaiGetEntity(m_iModel, "IFCFEATUREELEMENT");
	m_ifcFeatureElementSubtractionEntity = sdaiGetEntity(m_iModel, "IFCFEATUREELEMENTSUBTRACTION");
	m_ifcFurnishingElementEntity = sdaiGetEntity(m_iModel, "IFCFURNISHINGELEMENT");
	m_ifcReinforcingElementEntity = sdaiGetEntity(m_iModel, "IFCREINFORCINGELEMENT");
	m_ifcTransportElementEntity = sdaiGetEntity(m_iModel, "IFCTRANSPORTELEMENT");
	m_ifcVirtualElementEntity = sdaiGetEntity(m_iModel, "IFCVIRTUALELEMENT");

	/*
	* Retrieve the objects recursively
	*/
	RetrieveObjects__depth(ifcObjectEntity, DEFAULT_CIRCLE_SEGMENTS, 0);

	int_t ifcRelSpaceBoundaryEntity = sdaiGetEntity(m_iModel, "IFCRELSPACEBOUNDARY");
	RetrieveObjects(ifcRelSpaceBoundaryEntity, "IFCRELSPACEBOUNDARY", L"IFCRELSPACEBOUNDARY", DEFAULT_CIRCLE_SEGMENTS);

	/*
	* Units
	*/
	LoadUnits();

	/*
	* Entities
	*/
	LoadEntities();

	/*
	* Classes
	*/
	LoadClasses();

	/*
	* Properties
	*/
	LoadProperties();

	/*
	* Helper data structures
	*/
	for (auto pInstance : m_vecInstances)
	{
		m_mapID2Instance[pInstance->ID()] = pInstance;
		m_mapExpressID2Instance[pInstance->expressID()] = pInstance;
		m_mapGUID2Instance[pInstance->getGUID()] = pInstance;
	}

	/**
	* Scale and Center
	*/

	ScaleAndCenter();
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::Clean()
{
	if (m_iModel != 0)
	{
		sdaiCloseModel(m_iModel);
		m_iModel = 0;
	}

	for (size_t iIFCObject = 0; iIFCObject < m_vecInstances.size(); iIFCObject++)
	{
		delete m_vecInstances[iIFCObject];
	}
	m_vecInstances.clear();

	map<wstring, CIFCUnit*>::iterator itUnits = m_mapUnits.begin();
	for (; itUnits != m_mapUnits.end(); itUnits++)
	{
		delete itUnits->second;
	}
	m_mapUnits.clear();

	map<int_t, CIFCEntity*>::iterator itEntities = m_mapEntities.begin();
	for (; itEntities != m_mapEntities.end(); itEntities++)
	{
		delete itEntities->second;
	}
	m_mapEntities.clear();

	map<int64_t, CIFCClass*>::iterator itClass = m_mapClasses.begin();
	for (; itClass != m_mapClasses.end(); itClass++)
	{
		delete itClass->second;
	}
	m_mapClasses.clear();

	map<int64_t, CIFCProperty*>::iterator itProperty = m_mapProperties.begin();
	for (; itProperty != m_mapProperties.end(); itProperty++)
	{
		delete itProperty->second;
	}
	m_mapProperties.clear();
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::ScaleAndCenter()
{
	m_fBoundingSphereDiameter = 0.f;

	m_fXTranslation = 0.f;
	m_fYTranslation = 0.f;
	m_fZTranslation = 0.f;

	m_fXmin = FLT_MAX;
	m_fXmax = -FLT_MAX;
	m_fYmin = FLT_MAX;
	m_fYmax = -FLT_MAX;
	m_fZmin = FLT_MAX;
	m_fZmax = -FLT_MAX;

	auto itIinstance = m_mapInstances.begin();
	for (; itIinstance != m_mapInstances.end(); itIinstance++)
	{
		itIinstance->second->CalculateMinMax(m_fXmin, m_fXmax, m_fYmin, m_fYmax, m_fZmin, m_fZmax);
	}

	if ((m_fXmin == FLT_MAX) ||
		(m_fXmax == -FLT_MAX) ||
		(m_fYmin == FLT_MAX) ||
		(m_fYmax == -FLT_MAX) ||
		(m_fZmin == FLT_MAX) ||
		(m_fZmax == -FLT_MAX))
	{
		m_fXmin = -1.;
		m_fXmax = 1.;
		m_fYmin = -1.;
		m_fYmax = 1.;
		m_fZmin = -1.;
		m_fZmax = 1.;
	}

	m_fBoundingSphereDiameter = m_fXmax - m_fXmin;
	m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fYmax - m_fYmin);
	m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fZmax - m_fZmin);

	itIinstance = m_mapInstances.begin();
	for (; itIinstance != m_mapInstances.end(); itIinstance++)
	{
		itIinstance->second->ScaleAndCenter(m_fXmin, m_fXmax, m_fYmin, m_fYmax, m_fZmin, m_fZmax, m_fBoundingSphereDiameter);
	}

	/*
	* Min/Max
	*/

	m_fXmin = FLT_MAX;
	m_fXmax = -FLT_MAX;
	m_fYmin = FLT_MAX;
	m_fYmax = -FLT_MAX;
	m_fZmin = FLT_MAX;
	m_fZmax = -FLT_MAX;

	itIinstance = m_mapInstances.begin();
	for (; itIinstance != m_mapInstances.end(); itIinstance++)
	{
		if (itIinstance->second->getEnable())
		{
			itIinstance->second->CalculateMinMax(m_fXmin, m_fXmax, m_fYmin, m_fYmax, m_fZmin, m_fZmax);
		}
	}

	if ((m_fXmin == FLT_MAX) ||
		(m_fXmax == -FLT_MAX) ||
		(m_fYmin == FLT_MAX) ||
		(m_fYmax == -FLT_MAX) ||
		(m_fZmin == FLT_MAX) ||
		(m_fZmax == -FLT_MAX))
	{
		m_fXmin = -1.;
		m_fXmax = 1.;
		m_fYmin = -1.;
		m_fYmax = 1.;
		m_fZmin = -1.;
		m_fZmax = 1.;
	}

	/*
	* World
	*/
	m_fBoundingSphereDiameter = m_fXmax - m_fXmin;
	m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fYmax - m_fYmin);
	m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fZmax - m_fZmin);

	/*
	* Translations
	*/

	// [0.0 -> X/Y/Zmin + X/Y/Zmax]
	m_fXTranslation -= m_fXmin;
	m_fYTranslation -= m_fYmin;
	m_fZTranslation -= m_fZmin;

	// center
	m_fXTranslation -= ((m_fXmax - m_fXmin) / 2.0f);
	m_fYTranslation -= ((m_fYmax - m_fYmin) / 2.0f);
	m_fZTranslation -= ((m_fZmax - m_fZmin) / 2.0f);

	// [-1.0 -> 1.0]
	m_fXTranslation /= (m_fBoundingSphereDiameter / 2.0f);
	m_fYTranslation /= (m_fBoundingSphereDiameter / 2.0f);
	m_fZTranslation /= (m_fBoundingSphereDiameter / 2.0f);
}

// ------------------------------------------------------------------------------------------------
int_t CIFCModel::getModel() const
{
	return m_iModel;
}

// ------------------------------------------------------------------------------------------------
const wchar_t* CIFCModel::getModelName() const
{
	return m_strIFCFile.c_str();
}

// ------------------------------------------------------------------------------------------------
float CIFCModel::GetBoundingSphereDiameter() const
{
	return m_fBoundingSphereDiameter;
}

// ------------------------------------------------------------------------------------------------
const map<int64_t, CIFCInstance *>& CIFCModel::GetInstances() const
{
	return m_mapInstances;
}

// ------------------------------------------------------------------------------------------------
const map<wstring, CIFCUnit *>& CIFCModel::GetUnits() const
{
	return m_mapUnits;
}

// ------------------------------------------------------------------------------------------------
const CIFCUnit* CIFCModel::GetUnit(const wchar_t* szUnit) const
{
	map<wstring, CIFCUnit *>::const_iterator itUnit = m_mapUnits.find(szUnit);
	if (itUnit != m_mapUnits.end())
	{
		return itUnit->second;
	}

	return NULL;
}

// ------------------------------------------------------------------------------------------------
const map<int_t, CIFCEntity *>& CIFCModel::GetEntities() const
{
	return m_mapEntities;
}

// ------------------------------------------------------------------------------------------------
const map<int64_t, CIFCClass *>& CIFCModel::GetClasses() const
{
	return m_mapClasses;
}

// ------------------------------------------------------------------------------------------------
const map<int64_t, CIFCProperty *>& CIFCModel::GetProperties() const
{
	return m_mapProperties;
}

// ------------------------------------------------------------------------------------------------
CIFCInstance* CIFCModel::GetInstanceByID(int_t iID)
{
	map<int_t, CIFCInstance *>::iterator itID2IFCObject = m_mapID2Instance.find(iID);
	if (itID2IFCObject != m_mapID2Instance.end())
	{
		return itID2IFCObject->second;
	}

	return NULL;
}

// ------------------------------------------------------------------------------------------------
CIFCInstance* CIFCModel::GetInstanceByExpressID(int64_t iExpressID)
{
	auto itExpressID2IFCObject = m_mapExpressID2Instance.find(iExpressID);
	if (itExpressID2IFCObject != m_mapExpressID2Instance.end())
	{
		return itExpressID2IFCObject->second;
	}

	return NULL;
}

// --------------------------------------------------------------------------------------------
CIFCInstance* CIFCModel::GetInstanceByGUID(const wstring & GUID)
{
	auto itGUID2IFCObject = m_mapGUID2Instance.find(GUID);
	if (itGUID2IFCObject != m_mapGUID2Instance.end())
	{
		return itGUID2IFCObject->second;
	}

	return NULL;
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::LoadIFCQuantityLength(int_t iIFCQuantity, wstring & strQuantity)
{
	wchar_t	* szQuantityName = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Name", sdaiUNICODE, &szQuantityName);

	wchar_t	* szQuantityDescription = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Description", sdaiUNICODE, &szQuantityDescription);

	wchar_t	* szValue = NULL;
	sdaiGetAttrBN(iIFCQuantity, "LengthValue", sdaiUNICODE, &szValue);

	wchar_t	* szUnit = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Unit", sdaiUNICODE, &szUnit);

	strQuantity = szQuantityName;
	strQuantity += L" = ";
	strQuantity += szValue;

	if (szUnit != NULL)
	{
		strQuantity += L" ";
		strQuantity += szUnit;
	} // if (szUnit != NULL)
	else
	{
		map<wstring, CIFCUnit *>::iterator itUnits = m_mapUnits.find(L"LENGTHUNIT");
		if (itUnits != m_mapUnits.end())
		{
			strQuantity += L" ";
			strQuantity += itUnits->second->getName();
		}
	} // else if (szUnit != NULL)	

	if ((szQuantityDescription != NULL) && (wcslen(szQuantityDescription) > 0))
	{
		strQuantity += L" ('";
		strQuantity += szQuantityDescription;
		strQuantity += L"')";
	}
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::LoadIFCQuantityArea(int_t iIFCQuantity, wstring & strQuantity)
{
	wchar_t	* szQuantityName = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Name", sdaiUNICODE, &szQuantityName);

	wchar_t	* szQuantityDescription = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Description", sdaiUNICODE, &szQuantityDescription);

	wchar_t	* szValue = NULL;
	sdaiGetAttrBN(iIFCQuantity, "AreaValue", sdaiUNICODE, &szValue);

	wchar_t	* szUnit = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Unit", sdaiUNICODE, &szUnit);

	strQuantity = szQuantityName;
	strQuantity += L" = ";
	strQuantity += szValue;

	if (szUnit != NULL)
	{
		strQuantity += L" ";
		strQuantity += szUnit;
	} // if (szUnit != NULL)
	else
	{
		map<wstring, CIFCUnit *>::iterator itUnits = m_mapUnits.find(L"AREAUNIT");
		if (itUnits != m_mapUnits.end())
		{
			strQuantity += L" ";
			strQuantity += itUnits->second->getName();
		}
	} // else if (szUnit != NULL)	

	if ((szQuantityDescription != NULL) && (wcslen(szQuantityDescription) > 0))
	{
		strQuantity += L" ('";
		strQuantity += szQuantityDescription;
		strQuantity += L"')";
	}
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::LoadIFCQuantityVolume(int_t iIFCQuantity, wstring & strQuantity)
{
	wchar_t	* szQuantityName = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Name", sdaiUNICODE, &szQuantityName);

	wchar_t	* szQuantityDescription = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Description", sdaiUNICODE, &szQuantityDescription);

	wchar_t	* szValue = NULL;
	sdaiGetAttrBN(iIFCQuantity, "VolumeValue", sdaiUNICODE, &szValue);

	wchar_t	* szUnit = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Unit", sdaiUNICODE, &szUnit);

	strQuantity = szQuantityName;
	strQuantity += L" = ";
	strQuantity += szValue;

	if (szUnit != NULL)
	{
		strQuantity += L" ";
		strQuantity += szUnit;
	} // if (szUnit != NULL)
	else
	{
		map<wstring, CIFCUnit *>::iterator itUnits = m_mapUnits.find(L"VOLUMEUNIT");
		if (itUnits != m_mapUnits.end())
		{
			strQuantity += L" ";
			strQuantity += itUnits->second->getName();
		}
	} // else if (szUnit != NULL)	

	if ((szQuantityDescription != NULL) && (wcslen(szQuantityDescription) > 0))
	{
		strQuantity += L" ('";
		strQuantity += szQuantityDescription;
		strQuantity += L"')";
	}
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::LoadIFCQuantityCount(int_t iIFCQuantity, wstring & strQuantity)
{
	wchar_t	* szQuantityName = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Name", sdaiUNICODE, &szQuantityName);

	wchar_t	* szQuantityDescription = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Description", sdaiUNICODE, &szQuantityDescription);

	wchar_t	* szValue = NULL;
	sdaiGetAttrBN(iIFCQuantity, "CountValue", sdaiUNICODE, &szValue);

	wchar_t	* szUnit = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Unit", sdaiUNICODE, &szUnit);

	strQuantity = szQuantityName;
	strQuantity += L" = ";
	strQuantity += szValue;

	if (szUnit != NULL)
	{
		strQuantity += L" ";
		strQuantity += szUnit;
	} // if (szUnit != NULL)		

	if ((szQuantityDescription != NULL) && (wcslen(szQuantityDescription) > 0))
	{
		strQuantity += L" ('";
		strQuantity += szQuantityDescription;
		strQuantity += L"')";
	}
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::LoadIFCQuantityWeight(int_t iIFCQuantity, wstring & strQuantity)
{
	wchar_t	* szQuantityName = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Name", sdaiUNICODE, &szQuantityName);

	wchar_t	* szQuantityDescription = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Description", sdaiUNICODE, &szQuantityDescription);

	wchar_t	* szValue = NULL;
	sdaiGetAttrBN(iIFCQuantity, "WeigthValue", sdaiUNICODE, &szValue);

	wchar_t	* szUnit = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Unit", sdaiUNICODE, &szUnit);

	strQuantity = szQuantityName;
	strQuantity += L" = ";
	strQuantity += szValue;

	if (szUnit != NULL)
	{
		strQuantity += L" ";
		strQuantity += szUnit;
	} // if (szUnit != NULL)
	else
	{
		map<wstring, CIFCUnit *>::iterator itUnits = m_mapUnits.find(L"MASSUNIT");
		if (itUnits != m_mapUnits.end())
		{
			strQuantity += L" ";
			strQuantity += itUnits->second->getName();
		}
	} // else if (szUnit != NULL)	

	if ((szQuantityDescription != NULL) && (wcslen(szQuantityDescription) > 0))
	{
		strQuantity += L" ('";
		strQuantity += szQuantityDescription;
		strQuantity += L"')";
	}
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::LoadIFCQuantityTime(int_t iIFCQuantity, wstring & strQuantity)
{
	wchar_t	* szQuantityName = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Name", sdaiUNICODE, &szQuantityName);

	wchar_t	* szQuantityDescription = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Description", sdaiUNICODE, &szQuantityDescription);

	wchar_t	* szValue = NULL;
	sdaiGetAttrBN(iIFCQuantity, "TimeValue", sdaiUNICODE, &szValue);

	wchar_t	* szUnit = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Unit", sdaiUNICODE, &szUnit);

	strQuantity = szQuantityName;
	strQuantity += L" = ";
	strQuantity += szValue;

	if (szUnit != NULL)
	{
		strQuantity += L" ";
		strQuantity += szUnit;
	} // if (szUnit != NULL)
	else
	{
		map<wstring, CIFCUnit *>::iterator itUnits = m_mapUnits.find(L"TIMEUNIT");
		if (itUnits != m_mapUnits.end())
		{
			strQuantity += L" ";
			strQuantity += itUnits->second->getName();
		}
	} // else if (szUnit != NULL)	

	if ((szQuantityDescription != NULL) && (wcslen(szQuantityDescription) > 0))
	{
		strQuantity += L" ('";
		strQuantity += szQuantityDescription;
		strQuantity += L"')";
	}
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::SaveSchema()
{
	TCHAR szFilters[] = _T("Schema Files (*.ifcschema)|*.ifcschema|All Files (*.*)|*.*||");

	CFileDialog dlgFile(FALSE, _T("ifcschema"), _T("IFC Schema"),
		OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, szFilters);

	if (dlgFile.DoModal() != IDOK)
	{
		return;
	}

	CString strSchemaFile = dlgFile.GetPathName();

	CFile file(strSchemaFile, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary);
	CArchive archive(&file, CArchive::store);

	const int VERSION = 1;

	// Version
	archive << VERSION;

	// Entities Count
	archive << m_mapEntities.size();

	map<int_t, CIFCEntity*>::const_iterator itEntity = m_mapEntities.begin();
	for (; itEntity != m_mapEntities.end(); itEntity++)
	{
		// Entity Name
		archive << CString(itEntity->second->getName());

		const set<wstring>& setIgnoredAttributes = itEntity->second->getIgnoredAttributes();

		// Ignored Attributes Count
		archive << setIgnoredAttributes.size();

		set<wstring>::const_iterator itAttribute = setIgnoredAttributes.begin();
		for (; itAttribute != setIgnoredAttributes.end(); itAttribute++)
		{
			// Attribute Name
			archive << CString(itAttribute->c_str());
		}
	} // for (; itEntity != ...
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::LoadSchema()
{
	TCHAR szFilters[] = _T("Schema Files (*.ifcschema)|*.ifcschema|All Files (*.*)|*.*||");

	CFileDialog dlgFile(TRUE, _T("ifcschema"), _T("IFC Schema"),
		OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, szFilters);

	if (dlgFile.DoModal() != IDOK)
	{
		return;
	}

	CString strSchemaFile = dlgFile.GetPathName();

	CFile file;
	file.Open(strSchemaFile, CFile::modeRead | CFile::typeBinary);
	CArchive archive(&file, CArchive::load);

	int VERSION = 1;

	// Version
	archive >> VERSION;
	ASSERT(VERSION == 1);

	// Entities Count
	size_t iEntitiesCount = 0;
	archive >> iEntitiesCount;

	map<wstring, vector<wstring>> mapIgnoredAttributes;
	for (size_t iEntity = 0; iEntity < iEntitiesCount; iEntity++)
	{
		CString strEntity;
		archive >> strEntity;

		size_t iIgnoredAttributesCount = 0;
		archive >> iIgnoredAttributesCount;

		vector<wstring> vecIgnoredAttributes;
		for (size_t iIgnoredAttribute = 0; iIgnoredAttribute < iIgnoredAttributesCount; iIgnoredAttribute++)
		{
			CString strAttribute;
			archive >> strAttribute;

			vecIgnoredAttributes.push_back((LPCTSTR)strAttribute);
		}

		if (!vecIgnoredAttributes.empty())
		{
			mapIgnoredAttributes[(LPCTSTR)strEntity] = vecIgnoredAttributes;
		}
	} // for (size_t iEntity = ...

	map<int_t, CIFCEntity*>::iterator itEntity = m_mapEntities.begin();
	for (; itEntity != m_mapEntities.end(); itEntity++)
	{
		CIFCEntity* pEntity = itEntity->second;

		map<wstring, vector<wstring>>::iterator itIgnoredAttributes = mapIgnoredAttributes.find(pEntity->getName());
		if (itIgnoredAttributes != mapIgnoredAttributes.end())
		{
			for (size_t iIgnoredAttribute = 0; iIgnoredAttribute < itIgnoredAttributes->second.size(); iIgnoredAttribute++)
			{
				if (!pEntity->isAttributeIgnored(itIgnoredAttributes->second[iIgnoredAttribute]))
				{
					pEntity->ignoreAttribute(itIgnoredAttributes->second[iIgnoredAttribute], true);
				}
			}
		}
	} // for (; itEntity != ...
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::RetrieveObjects(int_t iEntity, const char * szEntityName, const wchar_t * szEntityNameW, int_t iCircleSegements)
{
	int_t * iIFCObjectInstances = sdaiGetEntityExtentBN(m_iModel, (char *) szEntityName);

	int_t iIFCObjectInstancesCount = sdaiGetMemberCount(iIFCObjectInstances);
	if (iIFCObjectInstancesCount == 0)
    {
        return;
    }	

	for (int_t i = 0; i < iIFCObjectInstancesCount; ++i)
	{
		int_t iInstance = 0;
		engiGetAggrElement(iIFCObjectInstances, i, sdaiINSTANCE, &iInstance);

		wchar_t	* szInstanceGUIDW = nullptr;
		sdaiGetAttrBN(iInstance, "GlobalId", sdaiUNICODE, &szInstanceGUIDW);

		CIFCInstance * pInstance = RetrieveGeometry(szInstanceGUIDW, iEntity, szEntityNameW, iInstance, iCircleSegements);
		pInstance->ID() = s_iObjectID++;

		CString strEntity = szEntityNameW;
		strEntity.MakeUpper();

		pInstance->setEnable((strEntity != "IFCSPACE") && (strEntity != "IFCRELSPACEBOUNDARY") && (strEntity != "IFCOPENINGELEMENT"));
		
		m_vecInstances.push_back(pInstance);
		m_mapInstances[iInstance] = pInstance;
	}
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::RetrieveObjects__depth(int_t iParentEntity, int_t iCircleSegments, int_t depth)
{
	if ((iParentEntity == m_ifcDistributionElementEntity) ||
		(iParentEntity == m_ifcElectricalElementEntity) ||
		(iParentEntity == m_ifcElementAssemblyEntity) ||
		(iParentEntity == m_ifcElementComponentEntity) ||
		(iParentEntity == m_ifcEquipmentElementEntity) ||
		(iParentEntity == m_ifcFeatureElementEntity) ||
		(iParentEntity == m_ifcFurnishingElementEntity) ||
		(iParentEntity == m_ifcTransportElementEntity) ||
		(iParentEntity == m_ifcVirtualElementEntity))
	{
		iCircleSegments = 12;
	}

	if (iParentEntity == m_ifcReinforcingElementEntity)
	{
		iCircleSegments = 6;
	}

	int_t* ifcObjectInstances = sdaiGetEntityExtent(m_iModel, iParentEntity);
	int_t noIfcObjectIntances = sdaiGetMemberCount(ifcObjectInstances);

	if (noIfcObjectIntances != 0)
	{
		char * szParenEntityName = NULL;
		engiGetEntityName(iParentEntity, sdaiSTRING, &szParenEntityName);

		wchar_t	* szParentEntityNameW = NULL;
		engiGetEntityName(iParentEntity, sdaiUNICODE, (char **)&szParentEntityNameW);

		RetrieveObjects(iParentEntity, szParenEntityName, szParentEntityNameW, iCircleSegments);

		if (iParentEntity == m_ifcProjectEntity) {
			for (int_t i = 0; i < noIfcObjectIntances; i++) {
				int_t ifcObjectInstance = 0;
				engiGetAggrElement(ifcObjectInstances, i, sdaiINSTANCE, &ifcObjectInstance);

				wchar_t	* szInstanceGUIDW = nullptr;
				sdaiGetAttrBN(ifcObjectInstance, "GlobalId", sdaiUNICODE, &szInstanceGUIDW);

				CIFCInstance * pInstance = RetrieveGeometry(szInstanceGUIDW, iParentEntity, szParentEntityNameW, ifcObjectInstance, iCircleSegments);
				pInstance->ID() = s_iObjectID++;

				CString strEntity = szParentEntityNameW;
				strEntity.MakeUpper();

				pInstance->setEnable((strEntity != "IFCSPACE") && (strEntity != "IFCRELSPACEBOUNDARY") && (strEntity != "IFCOPENINGELEMENT"));

				m_vecInstances.push_back(pInstance);
				m_mapInstances[ifcObjectInstance] = pInstance;
			}
		}
	} // if (noIfcObjectIntances != 0)

	noIfcObjectIntances = engiGetEntityCount(m_iModel);
	for (int_t i = 0; i < noIfcObjectIntances; i++)
	{
		int_t ifcEntity = engiGetEntityElement(m_iModel, i);
		if (engiGetEntityParent(ifcEntity) == iParentEntity)
		{
			RetrieveObjects__depth(ifcEntity, iCircleSegments, depth + 1);
		}
	}
}

bool	EQUALSUC(const wchar_t * strI, const wchar_t * strII)
{
	if (strI && strII) {
		int i = 0;
		while (strI[i]) {
			wchar_t	strI_UC = (strI[i] >= 'a' && strI[i] <= 'z') ? strI[i] + 'A' - 'a' : strI[i],
					strII_UC = (strII[i] >= 'a' && strII[i] <= 'z') ? strII[i] + 'A' - 'a' : strII[i];
			if (strI_UC != strII_UC) {
				return	false;
			}
			i++;
		}

		if (strII[i] == 0) {
			return	true;
		}
	}

	return	false;
}

// ------------------------------------------------------------------------------------------------
CIFCInstance * CIFCModel::RetrieveGeometry(const wchar_t * szInstanceGUIDW, int_t iEntity, const wchar_t * szEntityNameW, int_t iInstance, int_t iCircleSegments)
{
	/*
	* Set up format
	*/
	int_t setting = 0, mask = 0;
	mask += flagbit2;        // PRECISION (32/64 bit)
	mask += flagbit3;        //	INDEX ARRAY (32/64 bit)
	mask += flagbit5;        // NORMALS
	mask += flagbit6;        // TEXTURE
	mask += flagbit8;        // TRIANGLES
	mask += flagbit9;        // LINES
	mask += flagbit10;       // POINTS
	mask += flagbit13;       // CONCEPTUAL FACE POLYGONS
	mask += flagbit17;       // OPENGL
	mask += flagbit24;		 //	AMBIENT
	mask += flagbit25;		 //	DIFFUSE
	mask += flagbit26;		 //	EMISSIVE
	mask += flagbit27;		 //	SPECULAR

	setting += 0;		     // SINGLE PRECISION (float)
	setting += 0;            // 32 BIT INDEX ARRAY (Int32)
	setting += flagbit5;     // NORMALS ON
	setting += 0;			 // TEXTURE OFF
	setting += flagbit8;     // TRIANGLES ON
	setting += flagbit9;     // LINES ON
	setting += flagbit10;    // POINTS ON
	setting += flagbit13;    // // CONCEPTUAL FACE POLYGONS ON
	setting += 0;		     // OPENGL
	setting += flagbit24;	 //	AMBIENT
	setting += flagbit25;	 //	DIFFUSE
	setting += flagbit26;	 //	EMISSIVE
	setting += flagbit27;	 //	SPECULAR
	setFormat(m_iModel, setting, mask);
	setFilter(m_iModel, flagbit1, flagbit1);

	/*
	* Default color
	*/
	SetDefaultColor(
		m_iModel,
		50 * 256 * 256 * 256 + 50 * 256 * 256 + 50 * 256 + 255,
		50 * 256 * 256 * 256 + 50 * 256 * 256 + 50 * 256 + 255,
		50 * 256 * 256 * 256 + 50 * 256 * 256 + 50 * 256 + 255,
		50 * 256 * 256 * 256 + 50 * 256 * 256 + 50 * 256 + 255
	);

	/*
	* Set up circleSegments()
	*/
	if (iCircleSegments != DEFAULT_CIRCLE_SEGMENTS)
	{
		circleSegments(iCircleSegments, 5);
	}

	auto pInstance = new CIFCInstance(this, iInstance, szInstanceGUIDW, iEntity, szEntityNameW);

	ASSERT(pInstance->m_pVertexBuffer == nullptr);
	pInstance->m_pVertexBuffer = new _vertices_f();

	ASSERT(pInstance->m_pIndexBuffer == nullptr);
	pInstance->m_pIndexBuffer = new _indices_i32();

	CalculateInstance(iInstance, &pInstance->m_pVertexBuffer->size(), &pInstance->m_pIndexBuffer->size(), NULL);
	if ((pInstance->m_pVertexBuffer->size() == 0) || (pInstance->m_pIndexBuffer->size() == 0))
	{
		return pInstance;
	}

	int64_t iOWLModel = 0;
	owlGetModel(m_iModel, &iOWLModel);

	int64_t iOWLInstance = 0;
	owlGetInstance(m_iModel, iInstance, &iOWLInstance);

	/**
	* Retrieves the vertices
	*/
	pInstance->m_pVertexBuffer->vertexLength() = SetFormat(m_iModel, 0, 0) / sizeof(float);

	pInstance->m_pVertexBuffer->data() = new float[pInstance->m_pVertexBuffer->size() * pInstance->m_pVertexBuffer->vertexLength()];
	memset(pInstance->m_pVertexBuffer->data(), 0, pInstance->m_pVertexBuffer->size() * pInstance->m_pVertexBuffer->vertexLength() * sizeof(float));

	UpdateInstanceVertexBuffer(iOWLInstance, pInstance->m_pVertexBuffer->data());

	/**
	* Retrieves the indices
	*/
	pInstance->m_pIndexBuffer->data() = new int32_t[pInstance->m_pIndexBuffer->size()];
	memset(pInstance->m_pIndexBuffer->data(), 0, pInstance->m_pIndexBuffer->size() * sizeof(int32_t));

	UpdateInstanceIndexBuffer(iOWLInstance, pInstance->m_pIndexBuffer->data());

	// MATERIAL : FACE INDEX, START INDEX, INIDCES COUNT, etc.
	MATERIALS mapMaterial2ConcFaces;
	MATERIALS mapMaterial2ConcFacePoints; // MATERIAL : FACE INDEX, START INDEX, INIDCES COUNT, etc.

	//	http://rdf.bg/gkdoc/CP64/GetConceptualFaceCnt.html
	pInstance->m_iConceptualFacesCount = GetConceptualFaceCnt(iInstance);
	for (int64_t iConceptualFace = 0; iConceptualFace < pInstance->m_iConceptualFacesCount; iConceptualFace++)
	{
		//	http://rdf.bg/gkdoc/CP64/GetConceptualFaceEx.html
		int64_t iStartIndexTriangles = 0;
		int64_t iIndicesCountTriangles = 0;
		int64_t iStartIndexLines = 0;
		int64_t iIndicesCountLines = 0;
		int64_t iStartIndexPoints = 0;
		int64_t iIndicesCountPoints = 0;
		int64_t iStartIndexConceptualFacePolygons = 0;
		int64_t iIndicesCountConceptualFacePolygons = 0;
		GetConceptualFaceEx(iInstance, iConceptualFace,
			&iStartIndexTriangles, &iIndicesCountTriangles,
			&iStartIndexLines, &iIndicesCountLines,
			&iStartIndexPoints, &iIndicesCountPoints,
			0, 0,
			&iStartIndexConceptualFacePolygons, &iIndicesCountConceptualFacePolygons);

		if (iIndicesCountTriangles > 0)
		{
			/*
			* Material
			*/
			int32_t iIndexValue = *(pInstance->m_pIndexBuffer->data() + iStartIndexTriangles);
			iIndexValue *= VERTEX_LENGTH;

			float fColor = *(pInstance->m_pVertexBuffer->data() + iIndexValue + 6);
			unsigned int iAmbientColor = *(reinterpret_cast<unsigned int*>(&fColor));
			float fTransparency = (float)(iAmbientColor & (255)) / (float)255;

			fColor = *(pInstance->m_pVertexBuffer->data() + iIndexValue + 7);
			unsigned int iDiffuseColor = *(reinterpret_cast<unsigned int*>(&fColor));

			fColor = *(pInstance->m_pVertexBuffer->data() + iIndexValue + 8);
			unsigned int iEmissiveColor = *(reinterpret_cast<unsigned int*>(&fColor));

			fColor = *(pInstance->m_pVertexBuffer->data() + iIndexValue + 9);
			unsigned int iSpecularColor = *(reinterpret_cast<unsigned int*>(&fColor));

			/*
			* Material
			*/
			_material material(
				iAmbientColor,
				iDiffuseColor,
				iEmissiveColor,
				iSpecularColor,
				fTransparency,
				nullptr);

			auto itMaterial2ConceptualFaces = mapMaterial2ConcFaces.find(material);
			if (itMaterial2ConceptualFaces == mapMaterial2ConcFaces.end())
			{
				mapMaterial2ConcFaces[material] = vector<_face>{ _face(iConceptualFace, iStartIndexTriangles, iIndicesCountTriangles) };
			}
			else
			{
				itMaterial2ConceptualFaces->second.push_back(_face(iConceptualFace, iStartIndexTriangles, iIndicesCountTriangles));
			}
		}

		if (iIndicesCountTriangles > 0)
		{
			pInstance->m_vecTriangles.push_back(_primitives(iStartIndexTriangles, iIndicesCountTriangles));
		}

		if (iIndicesCountConceptualFacePolygons > 0)
		{
			pInstance->m_vecConcFacePolygons.push_back(_primitives(iStartIndexConceptualFacePolygons, iIndicesCountConceptualFacePolygons));
		}

		if (iIndicesCountLines > 0)
		{
			pInstance->m_vecLines.push_back(_primitives(iStartIndexLines, iIndicesCountLines));
		}

		if (iIndicesCountPoints > 0)
		{
			int32_t iIndexValue = *(pInstance->m_pIndexBuffer->data() + iStartIndexTriangles);
			iIndexValue *= VERTEX_LENGTH;

			float fColor = *(pInstance->m_pVertexBuffer->data() + iIndexValue + 6);
			unsigned int iAmbientColor = *(reinterpret_cast<unsigned int*>(&fColor));
			float fTransparency = (float)(iAmbientColor & (255)) / (float)255;

			fColor = *(pInstance->m_pVertexBuffer->data() + iIndexValue + 7);
			unsigned int iDiffuseColor = *(reinterpret_cast<unsigned int*>(&fColor));

			fColor = *(pInstance->m_pVertexBuffer->data() + iIndexValue + 8);
			unsigned int iEmissiveColor = *(reinterpret_cast<unsigned int*>(&fColor));

			fColor = *(pInstance->m_pVertexBuffer->data() + iIndexValue + 9);
			unsigned int iSpecularColor = *(reinterpret_cast<unsigned int*>(&fColor));

			/*
			* Material
			*/
			_material material(
				iAmbientColor,
				iDiffuseColor,
				iEmissiveColor,
				iSpecularColor,
				fTransparency,
				nullptr);

			auto itMaterial2ConcFacePoints = mapMaterial2ConcFacePoints.find(material);
			if (itMaterial2ConcFacePoints == mapMaterial2ConcFacePoints.end())
			{
				mapMaterial2ConcFacePoints[material] = vector<_face>{ _face(iConceptualFace, iStartIndexPoints, iIndicesCountPoints) };
			}
			else
			{
				itMaterial2ConcFacePoints->second.push_back(_face(iConceptualFace, iStartIndexPoints, iIndicesCountPoints));
			}

			pInstance->m_vecPoints.push_back(_primitives(iStartIndexPoints, iIndicesCountPoints));
		} // if (iIndicesCountPoints > 0)
	} // for (int64_t iConceptualFace = ...	

	/*
	* Group the faces
	*/
	auto itMaterial2ConcFaces = mapMaterial2ConcFaces.begin();
	for (; itMaterial2ConcFaces != mapMaterial2ConcFaces.end(); itMaterial2ConcFaces++)
	{
		_facesCohort* pCohort = nullptr;

		for (size_t iConcFace = 0; iConcFace < itMaterial2ConcFaces->second.size(); iConcFace++)
		{
			_face& concFace = itMaterial2ConcFaces->second[iConcFace];

			int_t iStartIndex = concFace.startIndex();
			int_t iIndicesCount = concFace.indicesCount();

			/*
			* Split the conceptual face - isolated case
			*/
			if (iIndicesCount > _oglUtils::getIndicesCountLimit())
			{
				while (iIndicesCount > _oglUtils::getIndicesCountLimit())
				{
					auto pNewCohort = new _facesCohort(itMaterial2ConcFaces->first);
					for (int_t iIndex = iStartIndex;
						iIndex < iStartIndex + _oglUtils::getIndicesCountLimit();
						iIndex++)
					{
						pNewCohort->indices().push_back(pInstance->m_pIndexBuffer->data()[iIndex]);
					}

					pInstance->concFacesCohorts().push_back(pNewCohort);

					/*
					* Update Conceptual face start index
					*/
					concFace.startIndex() = 0;

					// Conceptual faces
					pNewCohort->faces().push_back(concFace);

					iIndicesCount -= _oglUtils::getIndicesCountLimit();
					iStartIndex += _oglUtils::getIndicesCountLimit();
				}

				if (iIndicesCount > 0)
				{
					auto pNewCohort = new _facesCohort(itMaterial2ConcFaces->first);
					for (int_t iIndex = iStartIndex;
						iIndex < iStartIndex + iIndicesCount;
						iIndex++)
					{
						pNewCohort->indices().push_back(pInstance->m_pIndexBuffer->data()[iIndex]);
					}

					pInstance->concFacesCohorts().push_back(pNewCohort);

					/*
					* Update Conceptual face start index
					*/
					concFace.startIndex() = 0;

					// Conceptual faces
					pNewCohort->faces().push_back(concFace);
				}

				continue;
			} // if (iIndicesCount > _oglUtils::GetIndicesCountLimit())	

			/*
			* Create material
			*/
			if (pCohort == nullptr)
			{
				pCohort = new _facesCohort(itMaterial2ConcFaces->first);

				pInstance->concFacesCohorts().push_back(pCohort);
			}

			/*
			* Check the limit
			*/
			if (pCohort->indices().size() + iIndicesCount > _oglUtils::getIndicesCountLimit())
			{
				pCohort = new _facesCohort(itMaterial2ConcFaces->first);

				pInstance->concFacesCohorts().push_back(pCohort);
			}

			/*
			* Update Conceptual face start index
			*/
			concFace.startIndex() = pCohort->indices().size();

			/*
			* Add the indices
			*/
			for (int_t iIndex = iStartIndex;
				iIndex < iStartIndex + iIndicesCount;
				iIndex++)
			{
				pCohort->indices().push_back(pInstance->m_pIndexBuffer->data()[iIndex]);
			}

			// Conceptual faces
			pCohort->faces().push_back(concFace);
		} // for (size_t iConcFace = ...
	} // for (; itMaterial2ConceptualFaces != ...

	/*
	* Group the polygons
	*/
	if (!pInstance->m_vecConcFacePolygons.empty())
	{
		/*
		* Use the last cohort (if any)
		*/
		_cohort* pCohort = pInstance->concFacePolygonsCohorts().empty() ? 
			NULL : pInstance->concFacePolygonsCohorts()[pInstance->concFacePolygonsCohorts().size() - 1];

		/*
		* Create the cohort
		*/
		if (pCohort == NULL)
		{
			pCohort = new _cohort();
			pInstance->concFacePolygonsCohorts().push_back(pCohort);
		}

		for (size_t iFace = 0; iFace < pInstance->m_vecConcFacePolygons.size(); iFace++)
		{
			int_t iStartIndex = pInstance->m_vecConcFacePolygons[iFace].startIndex();
			int_t iIndicesCount = pInstance->m_vecConcFacePolygons[iFace].indicesCount();

			/*
			* Split the conceptual face - isolated case
			*/
			if (iIndicesCount > _oglUtils::getIndicesCountLimit() / 2)
			{
				while (iIndicesCount > _oglUtils::getIndicesCountLimit() / 2)
				{
					pCohort = new _cohort();
					pInstance->concFacePolygonsCohorts().push_back(pCohort);

					int_t iPreviousIndex = -1;
					for (int_t iIndex = iStartIndex;
						iIndex < iStartIndex + _oglUtils::getIndicesCountLimit() / 2;
						iIndex++)
					{
						if (pInstance->m_pIndexBuffer->data()[iIndex] < 0)
						{
							iPreviousIndex = -1;

							continue;
						}

						if (iPreviousIndex != -1)
						{
							pCohort->indices().push_back(pInstance->m_pIndexBuffer->data()[iPreviousIndex]);
							pCohort->indices().push_back(pInstance->m_pIndexBuffer->data()[iIndex]);
						} // if (iPreviousIndex != -1)

						iPreviousIndex = iIndex;
					} // for (int_t iIndex = ...

					iIndicesCount -= _oglUtils::getIndicesCountLimit() / 2;
					iStartIndex += _oglUtils::getIndicesCountLimit() / 2;
				} // while (iIndicesCount > _oglUtils::GetIndicesCountLimit() / 2)

				if (iIndicesCount > 0)
				{
					pCohort = new _cohort();
					pInstance->concFacePolygonsCohorts().push_back(pCohort);

					int_t iPreviousIndex = -1;
					for (int_t iIndex = iStartIndex;
						iIndex < iStartIndex + iIndicesCount;
						iIndex++)
					{
						if (pInstance->m_pIndexBuffer->data()[iIndex] < 0)
						{
							iPreviousIndex = -1;

							continue;
						}

						if (iPreviousIndex != -1)
						{
							pCohort->indices().push_back(pInstance->m_pIndexBuffer->data()[iPreviousIndex]);
							pCohort->indices().push_back(pInstance->m_pIndexBuffer->data()[iIndex]);
						} // if (iPreviousIndex != -1)

						iPreviousIndex = iIndex;
					} // for (int_t iIndex = ...
				}

				continue;
			} // if (iIndicesCount > _oglUtils::GetIndicesCountLimit() / 2)

			/*
			* Check the limit
			*/
			if ((pCohort->indices().size() + (iIndicesCount * 2)) > _oglUtils::getIndicesCountLimit())
			{
				pCohort = new _cohort();
				pInstance->concFacePolygonsCohorts().push_back(pCohort);
			}

			int_t iPreviousIndex = -1;
			for (int_t iIndex = iStartIndex;
				iIndex < iStartIndex + iIndicesCount;
				iIndex++)
			{
				if (pInstance->m_pIndexBuffer->data()[iIndex] < 0)
				{
					iPreviousIndex = -1;

					continue;
				}

				if (iPreviousIndex != -1)
				{
					pCohort->indices().push_back(pInstance->m_pIndexBuffer->data()[iPreviousIndex]);
					pCohort->indices().push_back(pInstance->m_pIndexBuffer->data()[iIndex]);
				} // if (iPreviousIndex != -1)

				iPreviousIndex = iIndex;
			} // for (int_t iIndex = ...
		} // for (size_t iFace = ...

#ifdef _DEBUG
		for (size_t iCohort = 0; iCohort < pInstance->concFacePolygonsCohorts().size(); iCohort++)
		{
			ASSERT(pInstance->concFacePolygonsCohorts()[iCohort]->indices().size() <= _oglUtils::getIndicesCountLimit());
		}
#endif
	} // if (!m_vecConcFacePolygons.empty())

	/*
	* Group the lines
	*/
	if (!pInstance->m_vecLines.empty())
	{
		/*
		* Use the last cohort (if any)
		*/
		auto pCohort = pInstance->linesCohorts().empty() ? 
			nullptr : pInstance->linesCohorts()[pInstance->linesCohorts().size() - 1];

		/*
		* Create the cohort
		*/
		if (pCohort == NULL)
		{
			pCohort = new _cohort();
			pInstance->linesCohorts().push_back(pCohort);
		}

		for (size_t iFace = 0; iFace < pInstance->m_vecLines.size(); iFace++)
		{
			int_t iStartIndex = pInstance->m_vecLines[iFace].startIndex();
			int_t iIndicesCount = pInstance->m_vecLines[iFace].indicesCount();

			/*
			* Check the limit
			*/
			if (pCohort->indices().size() + iIndicesCount > _oglUtils::getIndicesCountLimit())
			{
				pCohort = new _cohort();
				pInstance->linesCohorts().push_back(pCohort);
			}

			for (int_t iIndex = iStartIndex;
				iIndex < iStartIndex + iIndicesCount;
				iIndex++)
			{
				if (pInstance->m_pIndexBuffer->data()[iIndex] < 0)
				{
					continue;
				}

				pCohort->indices().push_back(pInstance->m_pIndexBuffer->data()[iIndex]);
			} // for (int_t iIndex = ...
		} // for (size_t iFace = ...

#ifdef _DEBUG
		for (size_t iCohort = 0; iCohort < pInstance->linesCohorts().size(); iCohort++)
		{
			ASSERT(pInstance->linesCohorts()[iCohort]->indices().size() <= _oglUtils::getIndicesCountLimit());
		}
#endif
	} // if (!m_vecLines.empty())		

	/*
	* Group the points
	*/
	auto itMaterial2ConcFacePoints = mapMaterial2ConcFacePoints.begin();
	for (; itMaterial2ConcFacePoints != mapMaterial2ConcFacePoints.end(); itMaterial2ConcFacePoints++)
	{
		_facesCohort* pCohort = nullptr;

		for (size_t iConcFace = 0; iConcFace < itMaterial2ConcFacePoints->second.size(); iConcFace++)
		{
			_face& concFace = itMaterial2ConcFacePoints->second[iConcFace];

			int_t iStartIndex = concFace.startIndex();
			int_t iIndicesCount = concFace.indicesCount();

			/*
			* Split the conceptual face - isolated case
			*/
			if (iIndicesCount > _oglUtils::getIndicesCountLimit())
			{
				while (iIndicesCount > _oglUtils::getIndicesCountLimit())
				{
					auto pNewCohort = new _facesCohort(itMaterial2ConcFacePoints->first);
					for (int_t iIndex = iStartIndex;
						iIndex < iStartIndex + _oglUtils::getIndicesCountLimit();
						iIndex++)
					{
						pNewCohort->indices().push_back(pInstance->m_pIndexBuffer->data()[iIndex]);
					}

					pInstance->pointsCohorts().push_back(pNewCohort);

					/*
					* Update Conceptual face start index
					*/
					concFace.startIndex() = 0;

					// Conceptual faces
					pNewCohort->faces().push_back(concFace);

					iIndicesCount -= _oglUtils::getIndicesCountLimit();
					iStartIndex += _oglUtils::getIndicesCountLimit();
				}

				if (iIndicesCount > 0)
				{
					auto pNewCohort = new _facesCohort(itMaterial2ConcFacePoints->first);
					for (int_t iIndex = iStartIndex;
						iIndex < iStartIndex + iIndicesCount;
						iIndex++)
					{
						pNewCohort->indices().push_back(pInstance->m_pIndexBuffer->data()[iIndex]);
					}

					pInstance->pointsCohorts().push_back(pNewCohort);

					/*
					* Update Conceptual face start index
					*/
					concFace.startIndex() = 0;

					// Conceptual faces
					pNewCohort->faces().push_back(concFace);
				}

				continue;
			} // if (iIndicesCountTriangles > _oglUtils::GetIndicesCountLimit())	

			/*
			* Create material
			*/
			if (pCohort == nullptr)
			{
				pCohort = new _facesCohort(itMaterial2ConcFacePoints->first);

				pInstance->pointsCohorts().push_back(pCohort);
			}

			/*
			* Check the limit
			*/
			if (pCohort->indices().size() + iIndicesCount > _oglUtils::getIndicesCountLimit())
			{
				pCohort = new _facesCohort(itMaterial2ConcFacePoints->first);

				pInstance->pointsCohorts().push_back(pCohort);
			}

			/*
			* Update Conceptual face start index
			*/
			concFace.startIndex() = pCohort->indices().size();

			/*
			* Add the indices
			*/
			for (int_t iIndex = iStartIndex;
				iIndex < iStartIndex + iIndicesCount;
				iIndex++)
			{
				pCohort->indices().push_back(pInstance->m_pIndexBuffer->data()[iIndex]);
			}

			// Conceptual faces
			pCohort->faces().push_back(concFace);
		} // for (size_t iConcFace = ...
	} // for (; itMaterial2ConceptualFaces != ...

	/*
	* Restore circleSegments()
	*/
	if (iCircleSegments != DEFAULT_CIRCLE_SEGMENTS)
	{
		circleSegments(DEFAULT_CIRCLE_SEGMENTS, 5);
	}

	cleanMemory(m_iModel, 0);

	return pInstance;
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::LoadUnits()
{
	int64_t * iIFCProjectInstances = sdaiGetEntityExtentBN(m_iModel, (char *)"IFCPROJECT");

	int64_t iIFCProjectInstancesCount = sdaiGetMemberCount(iIFCProjectInstances);
	if (iIFCProjectInstancesCount > 0)
	{
		int64_t	iIFCProjectInstance = 0;
		engiGetAggrElement(iIFCProjectInstances, 0, sdaiINSTANCE, &iIFCProjectInstance);

		CIFCUnit::LoadUnits(m_iModel, iIFCProjectInstance, m_mapUnits);
	}
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::LoadEntities()
{
	int_t iEntitiesCount = engiGetEntityCount(m_iModel);

	/*
	* Retrieve the Entities
	*/
	int_t i = 0;
	while (i < iEntitiesCount) 
	{		
		int_t iEntity = engiGetEntityElement(m_iModel, i);
		int_t iAttributesCount = engiGetEntityNoArguments(iEntity);
		int_t iInstancesCount = sdaiGetMemberCount(sdaiGetEntityExtent(m_iModel, iEntity));

		CIFCEntity * pEntity = new CIFCEntity(m_iModel, iEntity, iAttributesCount, iInstancesCount);
		ASSERT(m_mapEntities.find(iEntity) == m_mapEntities.end());
		m_mapEntities[iEntity] = pEntity;

		i++;
	}

	/*
	* Connect the Entities
	*/
	map<int_t, CIFCEntity *>::iterator itEntities = m_mapEntities.begin();
	for (; itEntities != m_mapEntities.end(); itEntities++)
	{
		int_t iParentEntity = engiGetEntityParent(itEntities->first);
		if (iParentEntity == 0)
		{
			continue;
		}		

		map<int_t, CIFCEntity *>::iterator itParentEntity = m_mapEntities.find(iParentEntity);
		ASSERT(itParentEntity != m_mapEntities.end());

		itEntities->second->setParent(itParentEntity->second);

		itParentEntity->second->addSubType(itEntities->second);
	} // for (; itEntities != ...	

	/*
	* Post-processing
	*/
	itEntities = m_mapEntities.begin();
	for (; itEntities != m_mapEntities.end(); itEntities++)
	{
		itEntities->second->postProcessing();
	} // for (; itEntities != ...	
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::LoadClasses()
{
	int64_t	iClassInstance = GetClassesByIterator(m_iModel, 0);
	while (iClassInstance != 0)
	{
		m_mapClasses[iClassInstance] = new CIFCClass(iClassInstance);

		iClassInstance = GetClassesByIterator(m_iModel, iClassInstance);
	} // while (iClassInstance != 0)
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::LoadProperties()
{
	int64_t iPropertyInstance = GetPropertiesByIterator(m_iModel, 0);
	while (iPropertyInstance != 0)
	{
		int64_t iPropertyType = GetPropertyType(iPropertyInstance);
		switch (iPropertyType)
		{
		case TYPE_OBJECTTYPE:
		{
			m_mapProperties[iPropertyInstance] = new CObjectIFCProperty(iPropertyInstance);
		}
		break;

		case TYPE_BOOL_DATATYPE:
		{
			m_mapProperties[iPropertyInstance] = new CBoolIFCProperty(iPropertyInstance);
		}
		break;

		case TYPE_CHAR_DATATYPE:
		{
			m_mapProperties[iPropertyInstance] = new CStringIFCProperty(iPropertyInstance);
		}
		break;

		case TYPE_INT_DATATYPE:
		{
			m_mapProperties[iPropertyInstance] = new CIntIFCProperty(iPropertyInstance);
		}
		break;

		case TYPE_DOUBLE_DATATYPE:
		{
			m_mapProperties[iPropertyInstance] = new CDoubleIFCProperty(iPropertyInstance);
		}
		break;

		default:
			ASSERT(FALSE);
			break;
		} // switch (iPropertyType)

		map<int64_t, CIFCClass *>::iterator itClass = m_mapClasses.begin();
		for (; itClass != m_mapClasses.end(); itClass++)
		{
			int64_t	iMinCard = 0;
			int64_t iMaxCard = 0;
			GetPropertyRestrictions(itClass->first, iPropertyInstance, &iMinCard, &iMaxCard);

			if ((iMinCard == -1) && (iMaxCard == -1))
			{
				continue;
			}

			itClass->second->AddPropertyRestriction(new CIFCPropertyRestriction(iPropertyInstance, iMinCard, iMaxCard));
		} // for (; itClass != ...

		iPropertyInstance = GetPropertiesByIterator(m_iModel, iPropertyInstance);
	} // while (iPropertyInstance != 0)
}

