#include "stdafx.h"

#include "_3DUtils.h"
#include "_oglUtils.h"
#include "IFCModel.h"

#include <cwchar>
#include <cfloat>
#include <math.h>

// ************************************************************************************************
#define DEFAULT_CIRCLE_SEGMENTS 36

// ************************************************************************************************
/*static*/ int_t CIFCModel::s_iInstanceID = 1;

// ************************************************************************************************
static uint32_t DEFAULT_COLOR_R = 10;
static uint32_t DEFAULT_COLOR_G = 150;
static uint32_t DEFAULT_COLOR_B = 10;
static uint32_t DEFAULT_COLOR_A = 255;
/*static*/ uint32_t CIFCModel::DEFAULT_COLOR =
	256 * 256 * 256 * DEFAULT_COLOR_R +
	256 * 256 * DEFAULT_COLOR_G +
	256 * DEFAULT_COLOR_B +
	DEFAULT_COLOR_A;

// ************************************************************************************************
CIFCModel::CIFCModel(bool bLoadInstancesOnDemand/* = false*/)
	: CModel(enumModelType::IFC)
	, m_bLoadInstancesOnDemand(bLoadInstancesOnDemand)
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
	, m_vecInstances()
	, m_mapInstances()
	, m_mapID2Instance()
	, m_mapExpressID2Instance()
	, m_pUnitProvider(nullptr)
	, m_pPropertyProvider(nullptr)
	, m_pEntityProvider(nullptr)
	, m_pAttributeProvider(nullptr)
	, m_bUpdteVertexBuffers(true)
{}


CIFCModel::~CIFCModel()
{
	Clean();
}

void CIFCModel::PreLoadInstance(SdaiInstance iInstance)
{
	if (m_bUpdteVertexBuffers)
	{
		_vector3d vecOriginalBBMin;
		_vector3d vecOriginalBBMax;
		if (GetInstanceGeometryClass(iInstance) &&
			GetBoundingBox(
				iInstance,
				(double*)&vecOriginalBBMin,
				(double*)&vecOriginalBBMax))
		{
			TRACE(L"\n*** SetVertexBufferOffset *** => x/y/z: %.16f, %.16f, %.16f",
				-(vecOriginalBBMin.x + vecOriginalBBMax.x) / 2.,
				-(vecOriginalBBMin.y + vecOriginalBBMax.y) / 2.,
				-(vecOriginalBBMin.z + vecOriginalBBMax.z) / 2.);

			// http://rdf.bg/gkdoc/CP64/SetVertexBufferOffset.html
			SetVertexBufferOffset(
				m_iModel,
				-(vecOriginalBBMin.x + vecOriginalBBMax.x) / 2.,
				-(vecOriginalBBMin.y + vecOriginalBBMax.y) / 2.,
				-(vecOriginalBBMin.z + vecOriginalBBMax.z) / 2.);

			// http://rdf.bg/gkdoc/CP64/ClearedExternalBuffers.html
			ClearedExternalBuffers(m_iModel);

			m_bUpdteVertexBuffers = false;
		}
	} // if (m_bUpdteVertexBuffers)
}

/*virtual*/ void CIFCModel::ZoomToInstance(CInstanceBase* pInstance) /*override*/
{
	ASSERT(pInstance != nullptr);

	auto pIFCInstance = dynamic_cast<CIFCInstance*>(pInstance);
	if (pIFCInstance == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	ASSERT(m_mapInstances.find(pIFCInstance->GetInstance()) != m_mapInstances.end());

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

void CIFCModel::Load(const wchar_t* szIFCFile, SdaiModel iModel)
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
	m_strPath = szIFCFile;

	/*
	* Entities
	*/
	SdaiEntity ifcObjectEntity = sdaiGetEntity(GetInstance(), "IFCOBJECT");
	m_ifcProjectEntity = sdaiGetEntity(GetInstance(), "IFCPROJECT");
	m_ifcSpaceEntity = sdaiGetEntity(GetInstance(), "IFCSPACE");
	m_ifcOpeningElementEntity = sdaiGetEntity(GetInstance(), "IFCOPENINGELEMENT");
	m_ifcDistributionElementEntity = sdaiGetEntity(GetInstance(), "IFCDISTRIBUTIONELEMENT");
	m_ifcElectricalElementEntity = sdaiGetEntity(GetInstance(), "IFCELECTRICALELEMENT");
	m_ifcElementAssemblyEntity = sdaiGetEntity(GetInstance(), "IFCELEMENTASSEMBLY");
	m_ifcElementComponentEntity = sdaiGetEntity(GetInstance(), "IFCELEMENTCOMPONENT");
	m_ifcEquipmentElementEntity = sdaiGetEntity(GetInstance(), "IFCEQUIPMENTELEMENT");
	m_ifcFeatureElementEntity = sdaiGetEntity(GetInstance(), "IFCFEATUREELEMENT");
	m_ifcFeatureElementSubtractionEntity = sdaiGetEntity(GetInstance(), "IFCFEATUREELEMENTSUBTRACTION");
	m_ifcFurnishingElementEntity = sdaiGetEntity(GetInstance(), "IFCFURNISHINGELEMENT");
	m_ifcReinforcingElementEntity = sdaiGetEntity(GetInstance(), "IFCREINFORCINGELEMENT");
	m_ifcTransportElementEntity = sdaiGetEntity(GetInstance(), "IFCTRANSPORTELEMENT");
	m_ifcVirtualElementEntity = sdaiGetEntity(GetInstance(), "IFCVIRTUALELEMENT");

	// Objects & Unreferenced
	if (!m_bLoadInstancesOnDemand)
	{
		RetrieveObjectsRecursively(ifcObjectEntity, DEFAULT_CIRCLE_SEGMENTS);

		RetrieveObjects("IFCPROJECT", L"IFCPROJECT", DEFAULT_CIRCLE_SEGMENTS);
		RetrieveObjects("IFCRELSPACEBOUNDARY", L"IFCRELSPACEBOUNDARY", DEFAULT_CIRCLE_SEGMENTS);

		GetObjectsReferencedState();
	}	

	// Units
	m_pUnitProvider = new CIFCUnitProvider(GetInstance());

	// Properties
	m_pPropertyProvider = new CIFCPropertyProvider(GetInstance(), m_pUnitProvider);

	// Entities
	m_pEntityProvider = new CEntityProvider(GetInstance());

	// Attributes
	m_pAttributeProvider = new CIFCAttributeProvider();

	// Helper data structures
	for (auto pInstance : m_vecInstances)
	{
		m_mapID2Instance[pInstance->getID()] = pInstance;
		m_mapExpressID2Instance[pInstance->ExpressID()] = pInstance;
	}

	// Scale
	Scale();
}

/*virtual*/ CInstanceBase* CIFCModel::LoadInstance(OwlInstance iInstance) /*override*/
{
	ASSERT(iInstance != 0);

	m_bUpdteVertexBuffers = true;

	for (auto pInstance : m_vecInstances)
	{
		delete pInstance;
	}
	m_vecInstances.clear();

	m_mapInstances.clear();
	m_mapID2Instance.clear();
	m_mapExpressID2Instance.clear();

	auto pInstance = RetrieveGeometry((SdaiInstance)iInstance, DEFAULT_CIRCLE_SEGMENTS);
	pInstance->setEnable(true);

	m_vecInstances.push_back(pInstance);
	m_mapInstances[(SdaiInstance)iInstance] = pInstance;

	// Helper data structures
	m_mapID2Instance[pInstance->getID()] = pInstance;
	m_mapExpressID2Instance[pInstance->ExpressID()] = pInstance;

	// Scale
	Scale();

	return pInstance;
}

void CIFCModel::Clean()
{
	if (m_iModel != 0)
	{
		sdaiCloseModel((SdaiModel)m_iModel);
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
}

void CIFCModel::Scale()
{
	/* World */
	m_dOriginalBoundingSphereDiameter = 2.;
	m_fBoundingSphereDiameter = 2.f;

	m_fXTranslation = 0.f;
	m_fYTranslation = 0.f;
	m_fZTranslation = 0.f;

	/* Min/Max */
	m_fXmin = FLT_MAX;
	m_fXmax = -FLT_MAX;
	m_fYmin = FLT_MAX;
	m_fYmax = -FLT_MAX;
	m_fZmin = FLT_MAX;
	m_fZmax = -FLT_MAX;

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
		// TODO: new status bar for geometry
		/*::MessageBox(
			::AfxGetMainWnd()->GetSafeHwnd(),
			L"Internal error.", L"Error", MB_ICONERROR | MB_OK);*/

		return;
	}

	/* World */
	m_dOriginalBoundingSphereDiameter = m_fXmax - m_fXmin;
	m_dOriginalBoundingSphereDiameter = max(m_dOriginalBoundingSphereDiameter, m_fYmax - m_fYmin);
	m_dOriginalBoundingSphereDiameter = max(m_dOriginalBoundingSphereDiameter, m_fZmax - m_fZmin);

	m_fBoundingSphereDiameter = m_dOriginalBoundingSphereDiameter;

	TRACE(L"\n*** Scale I *** => Xmin/max, Ymin/max, Zmin/max: %.16f, %.16f, %.16f, %.16f, %.16f, %.16f",
		m_fXmin,
		m_fXmax,
		m_fYmin,
		m_fYmax,
		m_fZmin,
		m_fZmax);
	TRACE(L"\n*** Scale, Bounding sphere I *** =>  %.16f", m_fBoundingSphereDiameter);

	/* Scale */
	itIinstance = m_mapInstances.begin();
	for (; itIinstance != m_mapInstances.end(); itIinstance++)
	{
		itIinstance->second->Scale(m_fBoundingSphereDiameter / 2.f);
	}

	/* Min/Max */
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
		// TODO: new status bar for geometry
		/*::MessageBox(
			::AfxGetMainWnd()->GetSafeHwnd(),
			L"Internal error.", L"Error", MB_ICONERROR | MB_OK);*/

		return;
	}

	/* World */
	m_fBoundingSphereDiameter = m_fXmax - m_fXmin;
	m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fYmax - m_fYmin);
	m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fZmax - m_fZmin);

	TRACE(L"\n*** Scale II *** => Xmin/max, Ymin/max, Zmin/max: %.16f, %.16f, %.16f, %.16f, %.16f, %.16f",
		m_fXmin,
		m_fXmax,
		m_fYmin,
		m_fYmax,
		m_fZmin,
		m_fZmax);
	TRACE(L"\n*** Scale, Bounding sphere II *** =>  %.16f", m_fBoundingSphereDiameter);

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

CIFCInstance* CIFCModel::GetInstanceByID(int64_t iID)
{
	auto itID2Instance = m_mapID2Instance.find(iID);
	if (itID2Instance != m_mapID2Instance.end())
	{
		return itID2Instance->second;
	}

	return nullptr;
}

/*virtual*/ CInstanceBase* CIFCModel::GetInstanceByExpressID(ExpressID iExpressID) const /*override*/
{
	auto itExpressID2Instance = m_mapExpressID2Instance.find(iExpressID);
	if (itExpressID2Instance != m_mapExpressID2Instance.end())
	{
		return itExpressID2Instance->second;
	}

	return nullptr;
}

void CIFCModel::GetInstancesByType(const wchar_t* szType, vector<CIFCInstance*>& vecInstances)
{
	vecInstances.clear();

	CString strTargetType = szType;
	strTargetType.MakeUpper();

	for (auto pInstance : m_vecInstances)
	{
		CString strType = pInstance->GetEntityName();
		strType.MakeUpper();

		if (strType == strTargetType)
		{
			vecInstances.push_back(pInstance);
		}
	}
}

void CIFCModel::RetrieveObjects(const char * szEntityName, const wchar_t * szEntityNameW, int_t iCircleSegements)
{
	SdaiAggr iIFCInstances = sdaiGetEntityExtentBN(GetInstance(), (char *) szEntityName);

	int_t iIFCInstancesCount = sdaiGetMemberCount(iIFCInstances);
	if (iIFCInstancesCount == 0)
    {
        return;
    }	

	for (int_t i = 0; i < iIFCInstancesCount; ++i)
	{
		SdaiInstance iInstance = 0;
		engiGetAggrElement(iIFCInstances, i, sdaiINSTANCE, &iInstance);

		auto pInstance = RetrieveGeometry(iInstance, iCircleSegements);

		CString strEntity = szEntityNameW;
		strEntity.MakeUpper();

		pInstance->setEnable(
			(strEntity == L"IFCSPACE") || 
			(strEntity == L"IFCRELSPACEBOUNDARY") ||
			(strEntity == L"IFCOPENINGELEMENT") ||
			(strEntity == L"IFCALIGNMENTVERTICAL") ||
			(strEntity == L"IFCALIGNMENTHORIZONTAL") ||
			(strEntity == L"IFCALIGNMENTSEGMENT") ||
			(strEntity == L"IFCALIGNMENTCANT") ? false : true);
		
		m_vecInstances.push_back(pInstance);
		m_mapInstances[iInstance] = pInstance;
	}
}

void CIFCModel::GetObjectsReferencedState()
{
	SdaiAggr pAggr = sdaiGetEntityExtentBN((SdaiModel)m_iModel, (char*)"IFCPROJECT");

	SdaiInteger iMembersCount = sdaiGetMemberCount(pAggr);
	if (iMembersCount > 0)
	{
		SdaiInstance iProjectInstance = 0;
		engiGetAggrElement(pAggr, 0, sdaiINSTANCE, &iProjectInstance);

		GetObjectsReferencedStateIsDecomposedBy(iProjectInstance);
		GetObjectsReferencedStateIsNestedBy(iProjectInstance);
		GetObjectsReferencedStateContainsElements(iProjectInstance);

		// Disable Unreferenced instances
		for (auto itInstance : m_mapInstances)
		{
			if (!itInstance.second->Referenced())
			{
				itInstance.second->setEnable(false);
			}
		}
	} // if (iIFCProjectInstancesCount > 0)
}

void CIFCModel::GetObjectsReferencedStateRecursively(SdaiInstance iInstance)
{
	ASSERT(iInstance != 0);

	auto itInstance = m_mapInstances.find(iInstance);
	if (itInstance != m_mapInstances.end())
	{
		itInstance->second->Referenced() = true;

		GetObjectsReferencedStateIsDecomposedBy(iInstance);
		GetObjectsReferencedStateIsNestedBy(iInstance);
		GetObjectsReferencedStateContainsElements(iInstance);
	}
	else
	{
		ASSERT(FALSE);
	}
}

void CIFCModel::GetObjectsReferencedStateIsDecomposedBy(SdaiInstance iInstance)
{
	SdaiAggr piIsDecomposedByInstances = nullptr;
	sdaiGetAttrBN(iInstance, "IsDecomposedBy", sdaiAGGR, &piIsDecomposedByInstances);

	if (piIsDecomposedByInstances == nullptr)
	{
		return;
	}

	SdaiEntity iIFCRelAggregatesEntity = sdaiGetEntity((SdaiModel)m_iModel, "IFCRELAGGREGATES");

	SdaiInteger iIFCIsDecomposedByInstancesCount = sdaiGetMemberCount(piIsDecomposedByInstances);
	for (SdaiInteger i = 0; i < iIFCIsDecomposedByInstancesCount; ++i)
	{
		SdaiInstance iIFCIsDecomposedByInstance = 0;
		engiGetAggrElement(piIsDecomposedByInstances, i, sdaiINSTANCE, &iIFCIsDecomposedByInstance);

		if (sdaiGetInstanceType(iIFCIsDecomposedByInstance) != iIFCRelAggregatesEntity)
		{
			continue;
		}

		SdaiAggr piIFCRelatedObjectsInstances = 0;
		sdaiGetAttrBN(iIFCIsDecomposedByInstance, "RelatedObjects", sdaiAGGR, &piIFCRelatedObjectsInstances);

		SdaiInteger iIFCRelatedObjectsInstancesCount = sdaiGetMemberCount(piIFCRelatedObjectsInstances);
		for (SdaiInteger j = 0; j < iIFCRelatedObjectsInstancesCount; ++j)
		{
			SdaiInstance iRelatedObjectsInstance = 0;
			engiGetAggrElement(piIFCRelatedObjectsInstances, j, sdaiINSTANCE, &iRelatedObjectsInstance);

			GetObjectsReferencedStateRecursively(iRelatedObjectsInstance);
		} // for (int_t j = ...
	} // for (int64_t i = ...
}

void CIFCModel::GetObjectsReferencedStateIsNestedBy(SdaiInstance iInstance)
{
	ASSERT(iInstance != 0);

	SdaiAggr piIsDecomposedByInstances = nullptr;
	sdaiGetAttrBN(iInstance, "IsNestedBy", sdaiAGGR, &piIsDecomposedByInstances);

	if (piIsDecomposedByInstances == nullptr)
	{
		return;
	}

	SdaiEntity iIFCRelNestsEntity = sdaiGetEntity((SdaiModel)m_iModel, "IFCRELNESTS");

	SdaiInteger iIFCIsDecomposedByInstancesCount = sdaiGetMemberCount(piIsDecomposedByInstances);
	for (SdaiInteger i = 0; i < iIFCIsDecomposedByInstancesCount; ++i)
	{
		SdaiInstance iIFCIsDecomposedByInstance = 0;
		engiGetAggrElement(piIsDecomposedByInstances, i, sdaiINSTANCE, &iIFCIsDecomposedByInstance);

		if (sdaiGetInstanceType(iIFCIsDecomposedByInstance) != iIFCRelNestsEntity)
		{
			continue;
		}

		SdaiAggr piIFCRelatedObjectsInstances = 0;
		sdaiGetAttrBN(iIFCIsDecomposedByInstance, "RelatedObjects", sdaiAGGR, &piIFCRelatedObjectsInstances);

		SdaiInteger iIFCRelatedObjectsInstancesCount = sdaiGetMemberCount(piIFCRelatedObjectsInstances);
		for (SdaiInteger j = 0; j < iIFCRelatedObjectsInstancesCount; ++j)
		{
			SdaiInstance iIFCRelatedObjectsInstance = 0;
			engiGetAggrElement(piIFCRelatedObjectsInstances, j, sdaiINSTANCE, &iIFCRelatedObjectsInstance);

			GetObjectsReferencedStateRecursively(iIFCRelatedObjectsInstance);
		} // for (int_t j = ...
	} // for (int64_t i = ...
}

void CIFCModel::GetObjectsReferencedStateContainsElements(SdaiInstance iInstance)
{
	ASSERT(iInstance != 0);

	SdaiAggr piContainsElementsInstances = nullptr;
	sdaiGetAttrBN(iInstance, "ContainsElements", sdaiAGGR, &piContainsElementsInstances);

	if (piContainsElementsInstances == nullptr)
	{
		return;
	}

	SdaiEntity iIFCRelContainedInSpatialStructureEntity = sdaiGetEntity((SdaiModel)m_iModel, "IFCRELCONTAINEDINSPATIALSTRUCTURE");

	SdaiInteger iIFCContainsElementsInstancesCount = sdaiGetMemberCount(piContainsElementsInstances);
	for (SdaiInteger i = 0; i < iIFCContainsElementsInstancesCount; ++i)
	{
		SdaiInstance iIFCContainsElementsInstance = 0;
		engiGetAggrElement(piContainsElementsInstances, i, sdaiINSTANCE, &iIFCContainsElementsInstance);

		if (sdaiGetInstanceType(iIFCContainsElementsInstance) != iIFCRelContainedInSpatialStructureEntity)
		{
			continue;
		}

		SdaiAggr piIFCRelatedElementsInstances = 0;
		sdaiGetAttrBN(iIFCContainsElementsInstance, "RelatedElements", sdaiAGGR, &piIFCRelatedElementsInstances);

		SdaiInteger iIFCRelatedElementsInstancesCount = sdaiGetMemberCount(piIFCRelatedElementsInstances);
		for (SdaiInteger j = 0; j < iIFCRelatedElementsInstancesCount; ++j)
		{
			SdaiInstance iIFCRelatedElementsInstance = 0;
			engiGetAggrElement(piIFCRelatedElementsInstances, j, sdaiINSTANCE, &iIFCRelatedElementsInstance);

			GetObjectsReferencedStateRecursively(iIFCRelatedElementsInstance);
		} // for (int_t j = ...
	} // for (int64_t i = ...
}

void CIFCModel::RetrieveObjectsRecursively(int_t iParentEntity, int_t iCircleSegments)
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

	int_t* piInstances = sdaiGetEntityExtent(GetInstance(), iParentEntity);
	int_t iIntancesCount = sdaiGetMemberCount(piInstances);

	if (iIntancesCount != 0)
	{
		char* szParenEntityName = nullptr;
		engiGetEntityName(iParentEntity, sdaiSTRING, (const char**)&szParenEntityName);

		wchar_t* szParentEntityNameW = CEntity::GetName(iParentEntity);
		RetrieveObjects(szParenEntityName, szParentEntityNameW, iCircleSegments);
	} // if (iIntancesCount != 0)

	iIntancesCount = engiGetEntityCount(GetInstance());
	for (int_t i = 0; i < iIntancesCount; i++)
	{
		SdaiEntity iEntity = engiGetEntityElement(GetInstance(), i);
		if (engiGetEntityParent(iEntity) == iParentEntity)
		{
			RetrieveObjectsRecursively(iEntity, iCircleSegments);
		}
	}
}

CIFCInstance* CIFCModel::RetrieveGeometry(SdaiInstance iInstance, int_t iCircleSegments)
{
	PreLoadInstance(iInstance);		

	// Set up circleSegments()
	if (iCircleSegments != DEFAULT_CIRCLE_SEGMENTS)
	{
		circleSegments(iCircleSegments, 5);
	}

	auto pInstance = new CIFCInstance(s_iInstanceID++, iInstance);	

	// Restore circleSegments()
	if (iCircleSegments != DEFAULT_CIRCLE_SEGMENTS)
	{
		circleSegments(DEFAULT_CIRCLE_SEGMENTS, 5);
	}

	cleanMemory(GetInstance(), 0);

	return pInstance;
}
