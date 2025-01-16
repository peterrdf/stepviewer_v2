#include "_host.h"
#include "_ifc_model.h"
#include "_ifc_geometry.h"
#include "_ifc_instance.h"
#include "_ptr.h"

// ************************************************************************************************
#define DEFAULT_CIRCLE_SEGMENTS 36

// ************************************************************************************************
_ifc_model::_ifc_model(bool bUseWorldCoordinates /*= false*/, bool bLoadInstancesOnDemand /*= false*/)
	: _ap_model(enumAP::IFC)
	, m_bUseWorldCoordinates(bUseWorldCoordinates)
	, m_bLoadInstancesOnDemand(bLoadInstancesOnDemand)
	, m_sdaiSpaceEntity(0)
	, m_sdaiOpeningElementEntity(0)
	, m_sdaiDistributionElementEntity(0)
	, m_sdaiElectricalElementEntity(0)
	, m_sdaiElementAssemblyEntity(0)
	, m_sdaiElementComponentEntity(0)
	, m_sdaiEquipmentElementEntity(0)
	, m_sdaiFeatureElementEntity(0)
	, m_sdaiFeatureElementSubtractionEntity(0)
	, m_sdaiFurnishingElementEntity(0)
	, m_sdaiReinforcingElementEntity(0)
	, m_sdaiTransportElementEntity(0)
	, m_sdaiVirtualElementEntity(0)
	, m_pUnitProvider(nullptr)
	, m_pPropertyProvider(nullptr)	
{
}

/*virtual*/ _ifc_model::~_ifc_model()
{
	clean();
}

/*virtual*/ _instance* _ifc_model::loadInstance(int64_t iInstance) /*override*/
{ 
	assert(iInstance != 0);
	SdaiInstance sdaiInstance = (SdaiInstance)iInstance;

	clean(false);

	m_bUpdteVertexBuffers = true;

	OwlInstance owlInstance = _ap_geometry::buildOwlInstance(sdaiInstance);
	if (owlInstance != 0)
	{
		preLoadInstance(owlInstance);
	}

	auto pGeometry = createGeometry(owlInstance, sdaiInstance);
	addGeometry(pGeometry);

	auto pInstance = createInstance(_model::getNextInstanceID(), pGeometry, nullptr);
	addInstance(pInstance);

	scale();

	return pInstance;
}

/*virtual*/ void _ifc_model::clean(bool bCloseModel/* = true*/) /*override*/
{
	_ap_model::clean(bCloseModel);

	if (bCloseModel)
	{
		delete m_pUnitProvider;
		m_pUnitProvider = nullptr;

		delete m_pPropertyProvider;
		m_pPropertyProvider = nullptr;
	}
}

/*virtual*/ void _ifc_model::attachModelCore() /*override*/
{
	if (m_bUseWorldCoordinates)
	{
		SetBehavior(getOwlModel(), FLAGBIT(10), FLAGBIT(10));
	}

	setBRepProperties(getSdaiModel(), 7, 0.9, 0., 20000);	

	// Entities
	SdaiEntity sdaiObjectEntity = sdaiGetEntity(getSdaiModel(), "IFCOBJECT");
	m_sdaiSpaceEntity = sdaiGetEntity(getSdaiModel(), "IFCSPACE");
	m_sdaiOpeningElementEntity = sdaiGetEntity(getSdaiModel(), "IFCOPENINGELEMENT");
	m_sdaiDistributionElementEntity = sdaiGetEntity(getSdaiModel(), "IFCDISTRIBUTIONELEMENT");
	m_sdaiElectricalElementEntity = sdaiGetEntity(getSdaiModel(), "IFCELECTRICALELEMENT");
	m_sdaiElementAssemblyEntity = sdaiGetEntity(getSdaiModel(), "IFCELEMENTASSEMBLY");
	m_sdaiElementComponentEntity = sdaiGetEntity(getSdaiModel(), "IFCELEMENTCOMPONENT");
	m_sdaiEquipmentElementEntity = sdaiGetEntity(getSdaiModel(), "IFCEQUIPMENTELEMENT");
	m_sdaiFeatureElementEntity = sdaiGetEntity(getSdaiModel(), "IFCFEATUREELEMENT");
	m_sdaiFeatureElementSubtractionEntity = sdaiGetEntity(getSdaiModel(), "IFCFEATUREELEMENTSUBTRACTION");
	m_sdaiFurnishingElementEntity = sdaiGetEntity(getSdaiModel(), "IFCFURNISHINGELEMENT");
	m_sdaiReinforcingElementEntity = sdaiGetEntity(getSdaiModel(), "IFCREINFORCINGELEMENT");
	m_sdaiTransportElementEntity = sdaiGetEntity(getSdaiModel(), "IFCTRANSPORTELEMENT");
	m_sdaiVirtualElementEntity = sdaiGetEntity(getSdaiModel(), "IFCVIRTUALELEMENT");

	// Objects & Unreferenced
	if (!m_bLoadInstancesOnDemand)
	{
		retrieveGeometryRecursively(sdaiObjectEntity, DEFAULT_CIRCLE_SEGMENTS);

		retrieveGeometry("IFCPROJECT", DEFAULT_CIRCLE_SEGMENTS);
		retrieveGeometry("IFCRELSPACEBOUNDARY", DEFAULT_CIRCLE_SEGMENTS);

		getObjectsReferencedState();
	}

	scale();
}

/*virtual*/ _ifc_geometry* _ifc_model::createGeometry(OwlInstance owlInstance, SdaiInstance sdaiInstance)
{
	return new _ifc_geometry(owlInstance, sdaiInstance);
}

/*virtual*/ _ifc_instance* _ifc_model::createInstance(int64_t iID, _ifc_geometry* pGeometry, _matrix4x3* pTransformationMatrix)
{
	return new _ifc_instance(iID, pGeometry, pTransformationMatrix);
}

void _ifc_model::getObjectsReferencedState()
{
	SdaiAggr sdaiAggr = sdaiGetEntityExtentBN(getSdaiModel(), (char*)"IFCPROJECT");
	SdaiInteger iMembersCount = sdaiGetMemberCount(sdaiAggr);
	if (iMembersCount > 0)
	{
		SdaiInstance sdaiProjectInstance = 0;
		engiGetAggrElement(sdaiAggr, 0, sdaiINSTANCE, &sdaiProjectInstance);

		getObjectsReferencedStateIsDecomposedBy(sdaiProjectInstance);
		getObjectsReferencedStateIsNestedBy(sdaiProjectInstance);
		getObjectsReferencedStateContainsElements(sdaiProjectInstance);
		getObjectsReferencedStateHasAssignments(sdaiProjectInstance);

		// Disable Unreferenced instances
		for (auto pGeometry : getGeometries())
		{
			if (!_ptr<_ifc_geometry>(pGeometry)->getIsReferenced())
			{
				pGeometry->enableInstances(false);
			}
		}
	} // if (iMembersCount > 0)
}

void _ifc_model::getObjectsReferencedStateRecursively(SdaiInstance sdaiInstance)
{
	ASSERT(sdaiInstance != 0);

	auto pGeometry = getGeometryByInstance(sdaiInstance);
	if (pGeometry != nullptr)
	{
		_ptr<_ifc_geometry>(pGeometry)->m_bIsReferenced = true;

		getObjectsReferencedStateIsDecomposedBy(sdaiInstance);
		getObjectsReferencedStateIsNestedBy(sdaiInstance);
		getObjectsReferencedStateContainsElements(sdaiInstance);
		getObjectsReferencedStateHasAssignments(sdaiInstance);
	}
	else
	{
		ASSERT(FALSE);
	}
}

void _ifc_model::getObjectsReferencedStateIsDecomposedBy(SdaiInstance iInstance)
{
	SdaiAggr sdaiIsDecomposedByAggr = nullptr;
	sdaiGetAttrBN(iInstance, "IsDecomposedBy", sdaiAGGR, &sdaiIsDecomposedByAggr);
	if (sdaiIsDecomposedByAggr == nullptr)
	{
		return;
	}

	SdaiEntity sdaiRelAggregatesEntity = sdaiGetEntity(getSdaiModel(), "IFCRELAGGREGATES");

	SdaiInteger iIsDecomposedByMembersCount = sdaiGetMemberCount(sdaiIsDecomposedByAggr);
	for (SdaiInteger i = 0; i < iIsDecomposedByMembersCount; ++i)
	{
		SdaiInstance sdaiIsDecomposedByInstance = 0;
		engiGetAggrElement(sdaiIsDecomposedByAggr, i, sdaiINSTANCE, &sdaiIsDecomposedByInstance);
		if (sdaiGetInstanceType(sdaiIsDecomposedByInstance) != sdaiRelAggregatesEntity)
		{
			continue;
		}

		SdaiAggr sdaiRelatedObjectsAggr = 0;
		sdaiGetAttrBN(sdaiIsDecomposedByInstance, "RelatedObjects", sdaiAGGR, &sdaiRelatedObjectsAggr);

		SdaiInteger iRelatedObjectsMembersCount = sdaiGetMemberCount(sdaiRelatedObjectsAggr);
		for (SdaiInteger j = 0; j < iRelatedObjectsMembersCount; ++j)
		{
			SdaiInstance sdaiRelatedObjectsInstance = 0;
			engiGetAggrElement(sdaiRelatedObjectsAggr, j, sdaiINSTANCE, &sdaiRelatedObjectsInstance);

			getObjectsReferencedStateRecursively(sdaiRelatedObjectsInstance);
		}
	} // for (SdaiInteger i = ...
}

void _ifc_model::getObjectsReferencedStateIsNestedBy(SdaiInstance iInstance)
{
	ASSERT(iInstance != 0);

	SdaiAggr sdaiIsNestedByAggr = nullptr;
	sdaiGetAttrBN(iInstance, "IsNestedBy", sdaiAGGR, &sdaiIsNestedByAggr);
	if (sdaiIsNestedByAggr == nullptr)
	{
		return;
	}

	SdaiEntity sdaiRelNestsEntity = sdaiGetEntity(getSdaiModel(), "IFCRELNESTS");

	SdaiInteger iIsNestedByMembersCount = sdaiGetMemberCount(sdaiIsNestedByAggr);
	for (SdaiInteger i = 0; i < iIsNestedByMembersCount; ++i)
	{
		SdaiInstance sdaiIsDecomposedByInstance = 0;
		engiGetAggrElement(sdaiIsNestedByAggr, i, sdaiINSTANCE, &sdaiIsDecomposedByInstance);
		if (sdaiGetInstanceType(sdaiIsDecomposedByInstance) != sdaiRelNestsEntity)
		{
			continue;
		}

		SdaiAggr sdaiRelatedObjectsAggr = 0;
		sdaiGetAttrBN(sdaiIsDecomposedByInstance, "RelatedObjects", sdaiAGGR, &sdaiRelatedObjectsAggr);

		SdaiInteger iRelatedObjectsMembersCount = sdaiGetMemberCount(sdaiRelatedObjectsAggr);
		for (SdaiInteger j = 0; j < iRelatedObjectsMembersCount; ++j)
		{
			SdaiInstance sdaiRelatedObjectsInstance = 0;
			engiGetAggrElement(sdaiRelatedObjectsAggr, j, sdaiINSTANCE, &sdaiRelatedObjectsInstance);

			getObjectsReferencedStateRecursively(sdaiRelatedObjectsInstance);
		} // for (SdaiInteger j = ...
	} // for (int64_t i = ...
}

void _ifc_model::getObjectsReferencedStateContainsElements(SdaiInstance iInstance)
{
	ASSERT(iInstance != 0);

	SdaiAggr sdaiContainsElementsAggr = nullptr;
	sdaiGetAttrBN(iInstance, "ContainsElements", sdaiAGGR, &sdaiContainsElementsAggr);
	if (sdaiContainsElementsAggr == nullptr)
	{
		return;
	}

	SdaiEntity iIFCRelContainedInSpatialStructureEntity = sdaiGetEntity(getSdaiModel(), "IFCRELCONTAINEDINSPATIALSTRUCTURE");

	SdaiInteger iContainsElementsMembersCount = sdaiGetMemberCount(sdaiContainsElementsAggr);
	for (SdaiInteger i = 0; i < iContainsElementsMembersCount; ++i)
	{
		SdaiInstance sdaiContainsElementsInstance = 0;
		engiGetAggrElement(sdaiContainsElementsAggr, i, sdaiINSTANCE, &sdaiContainsElementsInstance);

		if (sdaiGetInstanceType(sdaiContainsElementsInstance) != iIFCRelContainedInSpatialStructureEntity)
		{
			continue;
		}

		SdaiAggr sdaiRelatedElementsAggr = 0;
		sdaiGetAttrBN(sdaiContainsElementsInstance, "RelatedElements", sdaiAGGR, &sdaiRelatedElementsAggr);

		SdaiInteger iRelatedElementsMembersCount = sdaiGetMemberCount(sdaiRelatedElementsAggr);
		for (SdaiInteger j = 0; j < iRelatedElementsMembersCount; ++j)
		{
			SdaiInstance sdaiRelatedElementsInstance = 0;
			engiGetAggrElement(sdaiRelatedElementsAggr, j, sdaiINSTANCE, &sdaiRelatedElementsInstance);

			getObjectsReferencedStateRecursively(sdaiRelatedElementsInstance);
		} // for (SdaiInteger j = ...
	} // for (SdaiInteger i = ...
}

void _ifc_model::getObjectsReferencedStateHasAssignments(SdaiInstance iInstance)
{
	ASSERT(iInstance != 0);

	SdaiAggr sdaiContainsElementsAggr = nullptr;
	sdaiGetAttrBN(iInstance, "HasAssignments", sdaiAGGR, &sdaiContainsElementsAggr);
	if (sdaiContainsElementsAggr == nullptr)
	{
		return;
	}

	SdaiInteger iContainsElementsMembersCount = sdaiGetMemberCount(sdaiContainsElementsAggr);
	for (SdaiInteger i = 0; i < iContainsElementsMembersCount; ++i)
	{
		SdaiInstance sdaiContainsElementsInstance = 0;
		engiGetAggrElement(sdaiContainsElementsAggr, i, sdaiINSTANCE, &sdaiContainsElementsInstance);

		SdaiInstance sdaiRelatingProductInstance = 0;
		sdaiGetAttrBN(sdaiContainsElementsInstance, "RelatingProduct", sdaiINSTANCE, &sdaiRelatingProductInstance);

		if (sdaiIsKindOfBN(sdaiRelatingProductInstance, "IFCPRODUCT"))
		{
			getObjectsReferencedStateRecursively(sdaiRelatingProductInstance);
		}			
	} // for (SdaiInteger i = ...
}

void _ifc_model::retrieveGeometry(const char* szEntityName, SdaiInteger iCircleSegements)
{
	SdaiAggr sdaiAggr = sdaiGetEntityExtentBN(getSdaiModel(), (char*)szEntityName);
	SdaiInteger iMembersCount = sdaiGetMemberCount(sdaiAggr);
	if (iMembersCount == 0)
	{
		return;
	}

	for (SdaiInteger i = 0; i < iMembersCount; ++i)
	{
		SdaiInstance sdaiInstance = 0;
		engiGetAggrElement(sdaiAggr, i, sdaiINSTANCE, &sdaiInstance);

		loadGeometry(szEntityName, sdaiInstance, iCircleSegements);
	}
}

void _ifc_model::retrieveGeometryRecursively(SdaiEntity sdaiParentEntity, SdaiInteger iCircleSegments)
{
	if ((sdaiParentEntity == m_sdaiDistributionElementEntity) ||
		(sdaiParentEntity == m_sdaiElectricalElementEntity) ||
		(sdaiParentEntity == m_sdaiElementAssemblyEntity) ||
		(sdaiParentEntity == m_sdaiElementComponentEntity) ||
		(sdaiParentEntity == m_sdaiEquipmentElementEntity) ||
		(sdaiParentEntity == m_sdaiFeatureElementEntity) ||
		(sdaiParentEntity == m_sdaiFurnishingElementEntity) ||
		(sdaiParentEntity == m_sdaiTransportElementEntity) ||
		(sdaiParentEntity == m_sdaiVirtualElementEntity))
	{
		iCircleSegments = 12;
	}
	else if (sdaiParentEntity == m_sdaiReinforcingElementEntity)
	{
		iCircleSegments = 6;
	}

	SdaiAggr sdaiAggr = sdaiGetEntityExtent(getSdaiModel(), sdaiParentEntity);
	SdaiInteger iMembersCount = sdaiGetMemberCount(sdaiAggr);
	if (iMembersCount != 0)
	{
		char* szParenEntityName = nullptr;
		engiGetEntityName(sdaiParentEntity, sdaiSTRING, (const char**)&szParenEntityName);

		retrieveGeometry(szParenEntityName, iCircleSegments);
	} // if (iIntancesCount != 0)

	iMembersCount = engiGetEntityCount(getSdaiModel());
	for (SdaiInteger i = 0; i < iMembersCount; i++)
	{
		SdaiEntity sdaiEntity = engiGetEntityElement(getSdaiModel(), i);
		if (engiGetEntityParent(sdaiEntity) == sdaiParentEntity)
		{
			retrieveGeometryRecursively(sdaiEntity, iCircleSegments);
		}
	}
}

_geometry* _ifc_model::loadGeometry(const char* szEntityName, SdaiInstance sdaiInstance, SdaiInteger iCircleSegments)
{
	auto pGeometry = dynamic_cast<_ifc_geometry*>(getGeometryByInstance(sdaiInstance));
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

	pInstance->setEnable(
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

_ifc_unit_provider* _ifc_model::getUnitProvider() 
{ 
	if (m_pUnitProvider == nullptr)
	{
		m_pUnitProvider = new _ifc_unit_provider(getSdaiModel());
	}

	return m_pUnitProvider; 
}

_ifc_property_provider* _ifc_model::getPropertyProvider() 
{ 
	if (m_pPropertyProvider == nullptr)
	{
		m_pPropertyProvider = new _ifc_property_provider(getSdaiModel(), getUnitProvider());
	}

	return m_pPropertyProvider; 
}

