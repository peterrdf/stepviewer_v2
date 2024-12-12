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
	: _ap_model(enumAP::IFC)
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
	, m_pUnitProvider(nullptr)
	, m_pPropertyProvider(nullptr)
	, m_pAttributeProvider(nullptr)
	, m_bUpdteVertexBuffers(true)
{}

CIFCModel::~CIFCModel()
{
	clean();
}

/*virtual*/ void CIFCModel::attachModelCore() /*override*/
{
	// Entities
	SdaiEntity ifcObjectEntity = sdaiGetEntity(getSdaiInstance(), "IFCOBJECT");
	m_ifcProjectEntity = sdaiGetEntity(getSdaiInstance(), "IFCPROJECT");
	m_ifcSpaceEntity = sdaiGetEntity(getSdaiInstance(), "IFCSPACE");
	m_ifcOpeningElementEntity = sdaiGetEntity(getSdaiInstance(), "IFCOPENINGELEMENT");
	m_ifcDistributionElementEntity = sdaiGetEntity(getSdaiInstance(), "IFCDISTRIBUTIONELEMENT");
	m_ifcElectricalElementEntity = sdaiGetEntity(getSdaiInstance(), "IFCELECTRICALELEMENT");
	m_ifcElementAssemblyEntity = sdaiGetEntity(getSdaiInstance(), "IFCELEMENTASSEMBLY");
	m_ifcElementComponentEntity = sdaiGetEntity(getSdaiInstance(), "IFCELEMENTCOMPONENT");
	m_ifcEquipmentElementEntity = sdaiGetEntity(getSdaiInstance(), "IFCEQUIPMENTELEMENT");
	m_ifcFeatureElementEntity = sdaiGetEntity(getSdaiInstance(), "IFCFEATUREELEMENT");
	m_ifcFeatureElementSubtractionEntity = sdaiGetEntity(getSdaiInstance(), "IFCFEATUREELEMENTSUBTRACTION");
	m_ifcFurnishingElementEntity = sdaiGetEntity(getSdaiInstance(), "IFCFURNISHINGELEMENT");
	m_ifcReinforcingElementEntity = sdaiGetEntity(getSdaiInstance(), "IFCREINFORCINGELEMENT");
	m_ifcTransportElementEntity = sdaiGetEntity(getSdaiInstance(), "IFCTRANSPORTELEMENT");
	m_ifcVirtualElementEntity = sdaiGetEntity(getSdaiInstance(), "IFCVIRTUALELEMENT");

	// Objects & Unreferenced
	if (!m_bLoadInstancesOnDemand)
	{
		RetrieveObjectsRecursively(ifcObjectEntity, DEFAULT_CIRCLE_SEGMENTS);

		RetrieveObjects("IFCPROJECT", L"IFCPROJECT", DEFAULT_CIRCLE_SEGMENTS);
		RetrieveObjects("IFCRELSPACEBOUNDARY", L"IFCRELSPACEBOUNDARY", DEFAULT_CIRCLE_SEGMENTS);

		GetObjectsReferencedState();
	}

	// Units
	m_pUnitProvider = new CIFCUnitProvider(getSdaiInstance());

	// Properties
	m_pPropertyProvider = new CIFCPropertyProvider(getSdaiInstance(), m_pUnitProvider);

	// Attributes
	m_pAttributeProvider = new CIFCAttributeProvider();

	// Helper data structures
	for (auto pInstance : m_vecInstances)
	{
		m_mapID2Instance[pInstance->getID()] = pInstance;
	}

	// Scale
	Scale();
}

/*virtual*/ void CIFCModel::clean() /*override*/
{
	_ap_model::clean();

	delete m_pUnitProvider;
	m_pUnitProvider = nullptr;

	delete m_pPropertyProvider;
	m_pPropertyProvider = nullptr;

	delete m_pAttributeProvider;
	m_pAttributeProvider = nullptr;
}

/*virtual*/ void CIFCModel::ZoomToInstance(_instance* pInstance) /*override*/
{
	/*ASSERT(pInstance != nullptr);

	auto pIFCInstance = dynamic_cast<CIFCInstance*>(pInstance);
	if (pIFCInstance == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	ASSERT(m_mapInstances.find(pIFCInstance->getSdaiInstance()) != m_mapInstances.end());

	m_fBoundingSphereDiameter = 0.f;

	m_fXmin = FLT_MAX;
	m_fXmax = -FLT_MAX;
	m_fYmin = FLT_MAX;
	m_fYmax = -FLT_MAX;
	m_fZmin = FLT_MAX;
	m_fZmax = -FLT_MAX;

	pIFCInstance->calculateMinMax(m_fXmin, m_fXmax, m_fYmin, m_fYmax, m_fZmin, m_fZmax);

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
	m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fZmax - m_fZmin);*/
}

/*virtual*/ void CIFCModel::ZoomOut() /*override*/
{
	//m_fBoundingSphereDiameter = 0.f;

	//m_fXmin = FLT_MAX;
	//m_fXmax = -FLT_MAX;
	//m_fYmin = FLT_MAX;
	//m_fYmax = -FLT_MAX;
	//m_fZmin = FLT_MAX;
	//m_fZmax = -FLT_MAX;

	//auto itInstance = m_mapInstances.begin();
	//for (; itInstance != m_mapInstances.end(); itInstance++)
	//{
	//	if (!itInstance->second->_instance::getEnable())
	//	{
	//		continue;
	//	}

	//	itInstance->second->calculateMinMax(m_fXmin, m_fXmax, m_fYmin, m_fYmax, m_fZmin, m_fZmax);
	//}

	//if ((m_fXmin == FLT_MAX) ||
	//	(m_fXmax == -FLT_MAX) ||
	//	(m_fYmin == FLT_MAX) ||
	//	(m_fYmax == -FLT_MAX) ||
	//	(m_fZmin == FLT_MAX) ||
	//	(m_fZmax == -FLT_MAX))
	//{
	//	m_fXmin = -1.;
	//	m_fXmax = 1.;
	//	m_fYmin = -1.;
	//	m_fYmax = 1.;
	//	m_fZmin = -1.;
	//	m_fZmax = 1.;
	//}

	///*
	//* World
	//*/
	//m_fBoundingSphereDiameter = m_fXmax - m_fXmin;
	//m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fYmax - m_fYmin);
	//m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fZmax - m_fZmin);
}

/*virtual*/ _instance* CIFCModel::LoadInstance(OwlInstance iInstance) /*override*/
{
	ASSERT(FALSE); //#todo
	return nullptr;
	//ASSERT(iInstance != 0);

	//m_bUpdteVertexBuffers = true;

	//for (auto pInstance : m_vecInstances)
	//{
	//	delete pInstance;
	//}
	//m_vecInstances.clear();

	//m_mapInstances.clear();
	//m_mapID2Instance.clear();

	//auto pInstance = LoadGeometry(SdaiInstance)iInstance, DEFAULT_CIRCLE_SEGMENTS);
	//pInstance->_instance::setEnable(true);

	//m_vecInstances.push_back(pInstance);
	//m_mapInstances[(SdaiInstance)iInstance] = pInstance;

	//// Helper data structures
	//m_mapID2Instance[pInstance->getID()] = pInstance;

	//// Scale
	//Scale();

	//return pInstance;
}

void CIFCModel::Scale()
{
	scale();
}

void CIFCModel::GetInstancesByType(const wchar_t* szType, vector<_ap_instance*>& vecInstances)
{
//	vecInstances.clear();
//
//	CString strTargetType = szType;
//	strTargetType.MakeUpper();
//
//	for (auto pInstance : m_vecInstances)
//	{
//		auto pAPInstance = dynamic_cast<_ap_instance*>(pInstance);
//		ASSERT(pAPInstance != nullptr);
//
//		CString strType = pAPInstance->GetEntityName();
//		strType.MakeUpper();
//
//		if (strType == strTargetType)
//		{
//			vecInstances.push_back(pAPInstance);
//		}
//	}
}

void CIFCModel::RetrieveObjects(const char * szEntityName, const wchar_t * szEntityNameW, int_t iCircleSegements)
{
	SdaiAggr iIFCInstances = sdaiGetEntityExtentBN(getSdaiInstance(), (char *) szEntityName);

	int_t iIFCInstancesCount = sdaiGetMemberCount(iIFCInstances);
	if (iIFCInstancesCount == 0)
    {
        return;
    }	

	for (int_t i = 0; i < iIFCInstancesCount; ++i)
	{
		SdaiInstance iInstance = 0;
		engiGetAggrElement(iIFCInstances, i, sdaiINSTANCE, &iInstance);

		LoadGeometry(szEntityNameW, iInstance, iCircleSegements);
	}
}

void CIFCModel::GetObjectsReferencedState()
{
	SdaiAggr pAggr = sdaiGetEntityExtentBN(getSdaiInstance(), (char*)"IFCPROJECT");

	SdaiInteger iMembersCount = sdaiGetMemberCount(pAggr);
	if (iMembersCount > 0)
	{
		SdaiInstance iProjectInstance = 0;
		engiGetAggrElement(pAggr, 0, sdaiINSTANCE, &iProjectInstance);

		GetObjectsReferencedStateIsDecomposedBy(iProjectInstance);
		GetObjectsReferencedStateIsNestedBy(iProjectInstance);
		GetObjectsReferencedStateContainsElements(iProjectInstance);
		GetObjectsReferencedStateHasAssignments(iProjectInstance);

		// Disable Unreferenced instances
		for (auto pGeometry : m_vecGeometries)
		{
			if (!dynamic_cast<CIFCGeometry*>(pGeometry)->Referenced())
			{
				pGeometry->enableInstances(false);
			}
		}
	} // if (iIFCProjectInstancesCount > 0)
}

void CIFCModel::GetObjectsReferencedStateRecursively(SdaiInstance sdaiInstance)
{
	ASSERT(sdaiInstance != 0);

	auto pGeometry = geGeometryByInstance(sdaiInstance);
	if (pGeometry != nullptr)
	{
		dynamic_cast<CIFCGeometry*>(pGeometry)->Referenced() = true;

		GetObjectsReferencedStateIsDecomposedBy(sdaiInstance);
		GetObjectsReferencedStateIsNestedBy(sdaiInstance);
		GetObjectsReferencedStateContainsElements(sdaiInstance);
		GetObjectsReferencedStateHasAssignments(sdaiInstance);
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

	SdaiEntity iIFCRelAggregatesEntity = sdaiGetEntity(getSdaiInstance(), "IFCRELAGGREGATES");

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

	SdaiEntity iIFCRelNestsEntity = sdaiGetEntity(getSdaiInstance(), "IFCRELNESTS");

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

	SdaiEntity iIFCRelContainedInSpatialStructureEntity = sdaiGetEntity(getSdaiInstance(), "IFCRELCONTAINEDINSPATIALSTRUCTURE");

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

void CIFCModel::GetObjectsReferencedStateHasAssignments(SdaiInstance iInstance)
{
	ASSERT(iInstance != 0);

	SdaiAggr piContainsElementsInstances = nullptr;
	sdaiGetAttrBN(iInstance, "HasAssignments", sdaiAGGR, &piContainsElementsInstances);

	if (piContainsElementsInstances == nullptr)
	{
		return;
	}

	SdaiInteger iIFCContainsElementsInstancesCount = sdaiGetMemberCount(piContainsElementsInstances);
	for (SdaiInteger i = 0; i < iIFCContainsElementsInstancesCount; ++i)
	{
		SdaiInstance iIFCContainsElementsInstance = 0;
		engiGetAggrElement(piContainsElementsInstances, i, sdaiINSTANCE, &iIFCContainsElementsInstance);

		SdaiInstance iIFCRelatingProductInstance = 0;
		sdaiGetAttrBN(iIFCContainsElementsInstance, "RelatingProduct", sdaiINSTANCE, &iIFCRelatingProductInstance);

		if (sdaiIsKindOfBN(iIFCRelatingProductInstance, "IFCPRODUCT"))
			GetObjectsReferencedStateRecursively(iIFCRelatingProductInstance);
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

	int_t* piInstances = sdaiGetEntityExtent(getSdaiInstance(), iParentEntity);
	int_t iIntancesCount = sdaiGetMemberCount(piInstances);

	if (iIntancesCount != 0)
	{
		char* szParenEntityName = nullptr;
		engiGetEntityName(iParentEntity, sdaiSTRING, (const char**)&szParenEntityName);

		wchar_t* szParentEntityNameW = CEntity::GetName(iParentEntity);
		RetrieveObjects(szParenEntityName, szParentEntityNameW, iCircleSegments);
	} // if (iIntancesCount != 0)

	iIntancesCount = engiGetEntityCount(getSdaiInstance());
	for (int_t i = 0; i < iIntancesCount; i++)
	{
		SdaiEntity iEntity = engiGetEntityElement(getSdaiInstance(), i);
		if (engiGetEntityParent(iEntity) == iParentEntity)
		{
			RetrieveObjectsRecursively(iEntity, iCircleSegments);
		}
	}
}

_geometry* CIFCModel::LoadGeometry(const wchar_t* szEntityName, SdaiInstance sdaiInstance, int_t iCircleSegments)
{
	auto itGeometry = m_mapGeometries.find(sdaiInstance);
	if (itGeometry != m_mapGeometries.end())
	{
		return itGeometry->second;
	}

	preLoadInstance(sdaiInstance);		

	// Set up circleSegments()
	if (iCircleSegments != DEFAULT_CIRCLE_SEGMENTS)
	{
		circleSegments(iCircleSegments, 5);
	}

	auto pGeometry = new CIFCGeometry(sdaiInstance);
	m_vecGeometries.push_back(pGeometry);
	m_mapGeometries[sdaiInstance] = pGeometry;

	ASSERT(m_mapExpressID2Geometry.find(pGeometry->getExpressID()) == m_mapExpressID2Geometry.end());
	m_mapExpressID2Geometry[pGeometry->getExpressID()] = pGeometry;

	auto pInstance = new CIFCInstance(s_iInstanceID++, pGeometry, nullptr);
	m_vecInstances.push_back(pInstance);

	ASSERT(m_mapID2Instance.find(pInstance->getID()) == m_mapID2Instance.end());
	m_mapID2Instance[pInstance->getID()] = pInstance;

	pGeometry->addInstance(pInstance);

	CString strEntity = szEntityName;
	strEntity.MakeUpper();

	pInstance->_instance::setEnable(
		(strEntity == L"IFCSPACE") ||
		(strEntity == L"IFCRELSPACEBOUNDARY") ||
		(strEntity == L"IFCOPENINGELEMENT") ||
		(strEntity == L"IFCALIGNMENTVERTICAL") ||
		(strEntity == L"IFCALIGNMENTHORIZONTAL") ||
		(strEntity == L"IFCALIGNMENTSEGMENT") ||
		(strEntity == L"IFCALIGNMENTCANT") ? false : true);

	// Restore circleSegments()
	if (iCircleSegments != DEFAULT_CIRCLE_SEGMENTS)
	{
		circleSegments(DEFAULT_CIRCLE_SEGMENTS, 5);
	}

	cleanMemory(getSdaiInstance(), 0);

	return pGeometry;
}
