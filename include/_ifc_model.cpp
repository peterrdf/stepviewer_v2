#include "stdafx.h"
#include "_ifc_model.h"

#include "_ptr.h"
#include "_ifc_geometry.h"

// ************************************************************************************************
#define DEFAULT_CIRCLE_SEGMENTS 36

// ************************************************************************************************
_ifc_model::_ifc_model(bool bLoadInstancesOnDemand/* = false*/)
	: _ap_model(enumAP::IFC)
	, m_bLoadInstancesOnDemand(bLoadInstancesOnDemand)
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
{
}

/*virtual*/ _ifc_model::~_ifc_model()
{
	clean();
}

/*virtual*/ void _ifc_model::attachModelCore() /*override*/
{
	setBRepProperties(getSdaiModel(), 7, 0.9, 0., 20000);

	// Entities
	SdaiEntity ifcObjectEntity = sdaiGetEntity(getSdaiModel(), "IFCOBJECT");
	m_ifcSpaceEntity = sdaiGetEntity(getSdaiModel(), "IFCSPACE");
	m_ifcOpeningElementEntity = sdaiGetEntity(getSdaiModel(), "IFCOPENINGELEMENT");
	m_ifcDistributionElementEntity = sdaiGetEntity(getSdaiModel(), "IFCDISTRIBUTIONELEMENT");
	m_ifcElectricalElementEntity = sdaiGetEntity(getSdaiModel(), "IFCELECTRICALELEMENT");
	m_ifcElementAssemblyEntity = sdaiGetEntity(getSdaiModel(), "IFCELEMENTASSEMBLY");
	m_ifcElementComponentEntity = sdaiGetEntity(getSdaiModel(), "IFCELEMENTCOMPONENT");
	m_ifcEquipmentElementEntity = sdaiGetEntity(getSdaiModel(), "IFCEQUIPMENTELEMENT");
	m_ifcFeatureElementEntity = sdaiGetEntity(getSdaiModel(), "IFCFEATUREELEMENT");
	m_ifcFeatureElementSubtractionEntity = sdaiGetEntity(getSdaiModel(), "IFCFEATUREELEMENTSUBTRACTION");
	m_ifcFurnishingElementEntity = sdaiGetEntity(getSdaiModel(), "IFCFURNISHINGELEMENT");
	m_ifcReinforcingElementEntity = sdaiGetEntity(getSdaiModel(), "IFCREINFORCINGELEMENT");
	m_ifcTransportElementEntity = sdaiGetEntity(getSdaiModel(), "IFCTRANSPORTELEMENT");
	m_ifcVirtualElementEntity = sdaiGetEntity(getSdaiModel(), "IFCVIRTUALELEMENT");

	// Objects & Unreferenced
	if (!m_bLoadInstancesOnDemand)
	{
		RetrieveGeometryRecursively(ifcObjectEntity, DEFAULT_CIRCLE_SEGMENTS);

		RetrieveGeometry("IFCPROJECT", DEFAULT_CIRCLE_SEGMENTS);
		RetrieveGeometry("IFCRELSPACEBOUNDARY", DEFAULT_CIRCLE_SEGMENTS);

		GetObjectsReferencedState();
	}

	m_pUnitProvider = new CIFCUnitProvider(getSdaiModel());
	m_pPropertyProvider = new CIFCPropertyProvider(getSdaiModel(), m_pUnitProvider);
	m_pAttributeProvider = new CIFCAttributeProvider();

	scale();
}

/*virtual*/ void _ifc_model::clean() /*override*/
{
	_ap_model::clean();

	delete m_pUnitProvider;
	m_pUnitProvider = nullptr;

	delete m_pPropertyProvider;
	m_pPropertyProvider = nullptr;

	delete m_pAttributeProvider;
	m_pAttributeProvider = nullptr;
}

/*virtual*/ _ifc_geometry* _ifc_model::createGeometry(OwlInstance owlInstance, SdaiInstance sdaiInstance)
{
	return new _ifc_geometry(owlInstance, sdaiInstance);
}

/*virtual*/ _ap_instance* _ifc_model::createInstance(int64_t iID, _geometry* pGeometry, _matrix4x3* pTransformationMatrix)
{
	return new _ap_instance(iID, pGeometry, pTransformationMatrix);
}

void _ifc_model::GetObjectsReferencedState()
{
	SdaiAggr pAggr = sdaiGetEntityExtentBN(getSdaiModel(), (char*)"IFCPROJECT");

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
		for (auto pGeometry : getGeometries())
		{
			if (!_ptr<_ifc_geometry>(pGeometry)->getIsReferenced())
			{
				pGeometry->enableInstances(false);
			}
		}
	} // if (iIFCProjectInstancesCount > 0)
}

void _ifc_model::GetObjectsReferencedStateRecursively(SdaiInstance sdaiInstance)
{
	ASSERT(sdaiInstance != 0);

	auto pGeometry = getGeometryByInstance(sdaiInstance);
	if (pGeometry != nullptr)
	{
		_ptr<_ifc_geometry>(pGeometry)->m_bIsReferenced = true;

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

void _ifc_model::GetObjectsReferencedStateIsDecomposedBy(SdaiInstance iInstance)
{
	SdaiAggr piIsDecomposedByInstances = nullptr;
	sdaiGetAttrBN(iInstance, "IsDecomposedBy", sdaiAGGR, &piIsDecomposedByInstances);

	if (piIsDecomposedByInstances == nullptr)
	{
		return;
	}

	SdaiEntity iIFCRelAggregatesEntity = sdaiGetEntity(getSdaiModel(), "IFCRELAGGREGATES");

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

void _ifc_model::GetObjectsReferencedStateIsNestedBy(SdaiInstance iInstance)
{
	ASSERT(iInstance != 0);

	SdaiAggr piIsDecomposedByInstances = nullptr;
	sdaiGetAttrBN(iInstance, "IsNestedBy", sdaiAGGR, &piIsDecomposedByInstances);

	if (piIsDecomposedByInstances == nullptr)
	{
		return;
	}

	SdaiEntity iIFCRelNestsEntity = sdaiGetEntity(getSdaiModel(), "IFCRELNESTS");

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

void _ifc_model::GetObjectsReferencedStateContainsElements(SdaiInstance iInstance)
{
	ASSERT(iInstance != 0);

	SdaiAggr piContainsElementsInstances = nullptr;
	sdaiGetAttrBN(iInstance, "ContainsElements", sdaiAGGR, &piContainsElementsInstances);

	if (piContainsElementsInstances == nullptr)
	{
		return;
	}

	SdaiEntity iIFCRelContainedInSpatialStructureEntity = sdaiGetEntity(getSdaiModel(), "IFCRELCONTAINEDINSPATIALSTRUCTURE");

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

void _ifc_model::GetObjectsReferencedStateHasAssignments(SdaiInstance iInstance)
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

void _ifc_model::RetrieveGeometry(const char* szEntityName, int_t iCircleSegements)
{
	SdaiAggr iIFCInstances = sdaiGetEntityExtentBN(getSdaiModel(), (char*)szEntityName);

	int_t iIFCInstancesCount = sdaiGetMemberCount(iIFCInstances);
	if (iIFCInstancesCount == 0)
	{
		return;
	}

	for (int_t i = 0; i < iIFCInstancesCount; ++i)
	{
		SdaiInstance iInstance = 0;
		engiGetAggrElement(iIFCInstances, i, sdaiINSTANCE, &iInstance);

		LoadGeometry(szEntityName, iInstance, iCircleSegements);
	}
}

void _ifc_model::RetrieveGeometryRecursively(int_t iParentEntity, int_t iCircleSegments)
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

	int_t* piInstances = sdaiGetEntityExtent(getSdaiModel(), iParentEntity);
	int_t iIntancesCount = sdaiGetMemberCount(piInstances);

	if (iIntancesCount != 0)
	{
		char* szParenEntityName = nullptr;
		engiGetEntityName(iParentEntity, sdaiSTRING, (const char**)&szParenEntityName);

		RetrieveGeometry(szParenEntityName, iCircleSegments);
	} // if (iIntancesCount != 0)

	iIntancesCount = engiGetEntityCount(getSdaiModel());
	for (int_t i = 0; i < iIntancesCount; i++)
	{
		SdaiEntity iEntity = engiGetEntityElement(getSdaiModel(), i);
		if (engiGetEntityParent(iEntity) == iParentEntity)
		{
			RetrieveGeometryRecursively(iEntity, iCircleSegments);
		}
	}
}

_geometry* _ifc_model::LoadGeometry(const char* szEntityName, SdaiInstance sdaiInstance, int_t iCircleSegments)
{
	_ap_geometry* pGeometry = dynamic_cast<_ap_geometry*>(getGeometryByInstance(sdaiInstance));
	if (pGeometry != nullptr)
	{
		return pGeometry;
	}

	OwlInstance owlInstance = _ap_geometry::buildOwlInstance(sdaiInstance);
	if (owlInstance != 0)
	{
		preLoadInstance(owlInstance);
	}

	// Set up circleSegments()
	if (iCircleSegments != DEFAULT_CIRCLE_SEGMENTS)
	{
		circleSegments(iCircleSegments, 5);
	}

	pGeometry = createGeometry(owlInstance, sdaiInstance);
	addGeometry(pGeometry);

	auto pInstance = createInstance(_model::getNextInstanceID(), pGeometry, nullptr);
	addInstance(pInstance);

	CString strEntity = (LPWSTR)CA2W(szEntityName);
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

	return pGeometry;
}

