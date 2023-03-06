#include "stdafx.h"

#include "_oglUtils.h"
#include "IFCModel.h"

#include <cwchar>
#include <cfloat>
#include <math.h>

// ------------------------------------------------------------------------------------------------
/*static*/ int_t CIFCModel::s_iInstanceID = 1;

// ------------------------------------------------------------------------------------------------
CIFCModel::CIFCModel()
	: CModel(enumModelType::IFC)
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
	, m_pUnitProvider(nullptr)
	, m_pPropertyProvider(nullptr)
	, m_pEntityProvider(nullptr)
	, m_mapClasses()
{
}

// ------------------------------------------------------------------------------------------------
CIFCModel::~CIFCModel()
{
	Clean();
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ const wchar_t* CIFCModel::GetModelName() const  /*override*/
{
	return m_strIFCFile.c_str();
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ int64_t CIFCModel::GetInstance() const /*override*/
{
	return m_iModel;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ CEntityProvider* CIFCModel::GetEntityProvider() const /*override*/
{
	return m_pEntityProvider;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CIFCModel::ZoomToInstance(CInstance* pInstance) /*override*/
{
	ASSERT(pInstance != nullptr);

	auto pIFCInstance = dynamic_cast<CIFCInstance*>(pInstance);
	if (pIFCInstance == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	ASSERT(m_mapInstances.find(pIFCInstance->getInstance()) != m_mapInstances.end());

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

	pIFCInstance->CalculateMinMax(m_fXmin, m_fXmax, m_fYmin, m_fYmax, m_fZmin, m_fZmax);

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
/*virtual*/ void CIFCModel::ZoomOut() /*override*/
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

	auto itInstance = m_mapInstances.begin();
	for (; itInstance != m_mapInstances.end(); itInstance++)
	{
		if (!itInstance->second->getEnable())
		{
			continue;
		}

		itInstance->second->CalculateMinMax(m_fXmin, m_fXmax, m_fYmin, m_fYmax, m_fZmin, m_fZmax);
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
	ASSERT(szIFCFile != nullptr);
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
	RetrieveObjects("IFCRELSPACEBOUNDARY", L"IFCRELSPACEBOUNDARY", DEFAULT_CIRCLE_SEGMENTS);

	/*
	* Units
	*/
	m_pUnitProvider = new CIFCUnitProvider(m_iModel);

	/*
	* Properties
	*/
	m_pPropertyProvider = new CIFCPropertyProvider(m_iModel, m_pUnitProvider);

	/*
	* Entities
	*/
	m_pEntityProvider = new CEntityProvider(m_iModel);

	/*
	* Classes
	*/
	LoadClasses();

	/*
	* Helper data structures
	*/
	for (auto pInstance : m_vecInstances)
	{
		m_mapID2Instance[pInstance->ID()] = pInstance;
		m_mapExpressID2Instance[pInstance->expressID()] = pInstance;
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

	for (auto pInstance : m_vecInstances)
	{
		delete pInstance;
	}
	m_vecInstances.clear();

	delete m_pUnitProvider;
	m_pUnitProvider = nullptr;

	delete m_pPropertyProvider;
	m_pPropertyProvider = nullptr;

	delete m_pEntityProvider;
	m_pEntityProvider = nullptr;

	auto itClass = m_mapClasses.begin();
	for (; itClass != m_mapClasses.end(); itClass++)
	{
		delete itClass->second;
	}
	m_mapClasses.clear();
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::ScaleAndCenter()
{
	/*
	* Min/Max
	*/
	m_fXmin = FLT_MAX;
	m_fXmax = -FLT_MAX;
	m_fYmin = FLT_MAX;
	m_fYmax = -FLT_MAX;
	m_fZmin = FLT_MAX;
	m_fZmax = -FLT_MAX;

	m_fBoundingSphereDiameter = 0.f;

	m_fXTranslation = 0.f;
	m_fYTranslation = 0.f;
	m_fZTranslation = 0.f;

	auto itIinstance = m_mapInstances.begin();
	for (; itIinstance != m_mapInstances.end(); itIinstance++)
	{
		itIinstance->second->CalculateMinMax(
			m_fXmin, m_fXmax, 
			m_fYmin, m_fYmax, 
			m_fZmin, m_fZmax);
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

	TRACE(L"\n*** Scale and Center I *** => Xmin/max, Ymin/max, Zmin/max: %.16f, %.16f, %.16f, %.16f, %.16f, %.16f",
		m_fXmin,
		m_fXmax,
		m_fYmin,
		m_fYmax,
		m_fZmin,
		m_fZmax);
	TRACE(L"\n*** Scale and Center, Bounding sphere I *** =>  %.16f",
		m_fBoundingSphereDiameter);

	bool bScale = true;
	if (((m_fXmax - m_fXmin) / m_fBoundingSphereDiameter) <= 0.0001)
	{
		bScale = false;
	}
	else if (((m_fYmax - m_fYmin) / m_fBoundingSphereDiameter) <= 0.0001)
	{
		bScale = false;
	}
	else if (((m_fZmax - m_fZmin) / m_fBoundingSphereDiameter) <= 0.0001)
	{
		bScale = false;
	}

	if (!bScale)
	{
		CString strWarning = L"'Scale' algorithm cannot be used.\n";
		strWarning += L"Please, use 'Zoom to/extent' to explore the model.";

		::MessageBox(::AfxGetMainWnd()->GetSafeHwnd(), strWarning, L"Warning", MB_ICONWARNING | MB_OK);
	}

	/*
	* Scale and Center
	*/
	itIinstance = m_mapInstances.begin();
	for (; itIinstance != m_mapInstances.end(); itIinstance++)
	{
		itIinstance->second->ScaleAndCenter(
			m_fXmin, m_fXmax, 
			m_fYmin, m_fYmax, 
			m_fZmin, m_fZmax, 
			m_fBoundingSphereDiameter,
			bScale);
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
			itIinstance->second->CalculateMinMax(
				m_fXmin, m_fXmax, 
				m_fYmin, m_fYmax, 
				m_fZmin, m_fZmax);
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

	TRACE(L"\n*** Scale and Center II *** => Xmin/max, Ymin/max, Zmin/max: %.16f, %.16f, %.16f, %.16f, %.16f, %.16f",
		m_fXmin,
		m_fXmax,
		m_fYmin,
		m_fYmax,
		m_fZmin,
		m_fZmax);
	TRACE(L"\n*** Scale and Center, Bounding sphere II *** =>  %.16f",
		m_fBoundingSphereDiameter);

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
	if (bScale)
	{
		m_fXTranslation /= (m_fBoundingSphereDiameter / 2.0f);
		m_fYTranslation /= (m_fBoundingSphereDiameter / 2.0f);
		m_fZTranslation /= (m_fBoundingSphereDiameter / 2.0f);
	}
}

// ------------------------------------------------------------------------------------------------
int_t CIFCModel::getModel() const
{
	return m_iModel;
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
CIFCUnitProvider* CIFCModel::GetUnitProvider() const
{
	return m_pUnitProvider;
}

// ------------------------------------------------------------------------------------------------
CIFCPropertyProvider* CIFCModel::GetPropertyProvider() const
{
	return m_pPropertyProvider;
}

// ------------------------------------------------------------------------------------------------
const map<int64_t, CIFCClass *>& CIFCModel::GetClasses() const
{
	return m_mapClasses;
}

// ------------------------------------------------------------------------------------------------
CIFCInstance* CIFCModel::GetInstanceByID(int_t iID)
{
	auto itID2Instance = m_mapID2Instance.find(iID);
	if (itID2Instance != m_mapID2Instance.end())
	{
		return itID2Instance->second;
	}

	return nullptr;
}

// ------------------------------------------------------------------------------------------------
CIFCInstance* CIFCModel::GetInstanceByExpressID(int64_t iExpressID)
{
	auto itExpressID2Instance = m_mapExpressID2Instance.find(iExpressID);
	if (itExpressID2Instance != m_mapExpressID2Instance.end())
	{
		return itExpressID2Instance->second;
	}

	return nullptr;
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

	ASSERT(FALSE); // TODO: use CEntity

	// Entities Count
	//archive << m_mapEntities.size();

	//map<int_t, CIFCEntity*>::const_iterator itEntity = m_mapEntities.begin();
	//for (; itEntity != m_mapEntities.end(); itEntity++)
	//{
	//	// Entity Name
	//	archive << CString(itEntity->second->getName());

	//	const set<wstring>& setIgnoredAttributes = itEntity->second->getIgnoredAttributes();

	//	// Ignored Attributes Count
	//	archive << setIgnoredAttributes.size();

	//	set<wstring>::const_iterator itAttribute = setIgnoredAttributes.begin();
	//	for (; itAttribute != setIgnoredAttributes.end(); itAttribute++)
	//	{
	//		// Attribute Name
	//		archive << CString(itAttribute->c_str());
	//	}
	//} // for (; itEntity != ...
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

	ASSERT(FALSE); // TODO: use CEntity

	//map<int_t, CIFCEntity*>::iterator itEntity = m_mapEntities.begin();
	//for (; itEntity != m_mapEntities.end(); itEntity++)
	//{
	//	CIFCEntity* pEntity = itEntity->second;

	//	map<wstring, vector<wstring>>::iterator itIgnoredAttributes = mapIgnoredAttributes.find(pEntity->getName());
	//	if (itIgnoredAttributes != mapIgnoredAttributes.end())
	//	{
	//		for (size_t iIgnoredAttribute = 0; iIgnoredAttribute < itIgnoredAttributes->second.size(); iIgnoredAttribute++)
	//		{
	//			if (!pEntity->isAttributeIgnored(itIgnoredAttributes->second[iIgnoredAttribute]))
	//			{
	//				pEntity->ignoreAttribute(itIgnoredAttributes->second[iIgnoredAttribute], true);
	//			}
	//		}
	//	}
	//} // for (; itEntity != ...
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::RetrieveObjects(const char * szEntityName, const wchar_t * szEntityNameW, int_t iCircleSegements)
{
	int_t * iIFCInstances = sdaiGetEntityExtentBN(m_iModel, (char *) szEntityName);

	int_t iIFCInstancesCount = sdaiGetMemberCount(iIFCInstances);
	if (iIFCInstancesCount == 0)
    {
        return;
    }	

	for (int_t i = 0; i < iIFCInstancesCount; ++i)
	{
		int_t iInstance = 0;
		engiGetAggrElement(iIFCInstances, i, sdaiINSTANCE, &iInstance);

		wchar_t* szInstanceGUIDW = nullptr;
		sdaiGetAttrBN(iInstance, "GlobalId", sdaiUNICODE, &szInstanceGUIDW);

		auto pInstance = RetrieveGeometry(szInstanceGUIDW, iInstance, iCircleSegements);
		pInstance->ID() = s_iInstanceID++;

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

	int_t* piInstances = sdaiGetEntityExtent(m_iModel, iParentEntity);
	int_t iIntancesCount = sdaiGetMemberCount(piInstances);

	if (iIntancesCount != 0)
	{
		char* szParenEntityName = nullptr;
		engiGetEntityName(iParentEntity, sdaiSTRING, &szParenEntityName);

		wchar_t* szParentEntityNameW = nullptr;
		engiGetEntityName(iParentEntity, sdaiUNICODE, (char **)&szParentEntityNameW);

		RetrieveObjects(szParenEntityName, szParentEntityNameW, iCircleSegments);

		if (iParentEntity == m_ifcProjectEntity) {
			for (int_t i = 0; i < iIntancesCount; i++) {
				int_t iInstance = 0;
				engiGetAggrElement(piInstances, i, sdaiINSTANCE, &iInstance);

				wchar_t* szInstanceGUIDW = nullptr;
				sdaiGetAttrBN(iInstance, "GlobalId", sdaiUNICODE, &szInstanceGUIDW);

				CIFCInstance * pInstance = RetrieveGeometry(szInstanceGUIDW, iInstance, iCircleSegments);
				pInstance->ID() = s_iInstanceID++;

				CString strEntity = szParentEntityNameW;
				strEntity.MakeUpper();

				pInstance->setEnable((strEntity != "IFCSPACE") && (strEntity != "IFCRELSPACEBOUNDARY") && (strEntity != "IFCOPENINGELEMENT"));

				m_vecInstances.push_back(pInstance);
				m_mapInstances[iInstance] = pInstance;
			}
		}
	} // if (iIntancesCount != 0)

	iIntancesCount = engiGetEntityCount(m_iModel);
	for (int_t i = 0; i < iIntancesCount; i++)
	{
		int_t iEntity = engiGetEntityElement(m_iModel, i);
		if (engiGetEntityParent(iEntity) == iParentEntity)
		{
			RetrieveObjects__depth(iEntity, iCircleSegments, depth + 1);
		}
	}
}

// ------------------------------------------------------------------------------------------------
CIFCInstance* CIFCModel::RetrieveGeometry(const wchar_t* szInstanceGUIDW, int_t iInstance, int_t iCircleSegments)
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
	setting += flagbit13;    // CONCEPTUAL FACE POLYGONS ON
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

	auto pInstance = new CIFCInstance(this, iInstance, szInstanceGUIDW);

	ASSERT(pInstance->m_pVertexBuffer == nullptr);
	pInstance->m_pVertexBuffer = new _vertices_f();

	ASSERT(pInstance->m_pIndexBuffer == nullptr);
	pInstance->m_pIndexBuffer = new _indices_i32();

	CalculateInstance(iInstance, &pInstance->m_pVertexBuffer->size(), &pInstance->m_pIndexBuffer->size(), nullptr);
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
			nullptr : pInstance->concFacePolygonsCohorts()[pInstance->concFacePolygonsCohorts().size() - 1];

		/*
		* Create the cohort
		*/
		if (pCohort == nullptr)
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
		if (pCohort == nullptr)
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
void CIFCModel::LoadClasses()
{
	int64_t	iClassInstance = GetClassesByIterator(m_iModel, 0);
	while (iClassInstance != 0)
	{
		m_mapClasses[iClassInstance] = new CIFCClass(iClassInstance);

		iClassInstance = GetClassesByIterator(m_iModel, iClassInstance);
	} // while (iClassInstance != 0)
}
