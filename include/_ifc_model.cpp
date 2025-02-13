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
	, m_mapMapping()
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

	m_mapMapping.clear();

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
	return new _ifc_geometry(owlInstance, sdaiInstance, vector<_ifc_geometry*>());
}

/*virtual*/ _ifc_instance* _ifc_model::createInstance(int64_t iID, _ifc_geometry* pGeometry, _matrix4x3* pTransformationMatrix)
{
	return new _ifc_instance(iID, pGeometry, pTransformationMatrix);
}

void _ifc_model::getObjectsReferencedState()
{
	SdaiAggr sdaiAggr = sdaiGetEntityExtentBN(getSdaiModel(), "IFCPROJECT");
	SdaiInteger iMembersCount = sdaiGetMemberCount(sdaiAggr);
	if (iMembersCount > 0)
	{
		SdaiInstance sdaiProjectInstance = 0;
		engiGetAggrElement(sdaiAggr, 0, sdaiINSTANCE, &sdaiProjectInstance);

		getObjectsReferencedStateIsDecomposedBy(sdaiProjectInstance);
		getObjectsReferencedStateIsNestedBy(sdaiProjectInstance);
		getObjectsReferencedStateContainsElements(sdaiProjectInstance);
		getObjectsReferencedStateHasAssignments(sdaiProjectInstance);
		getObjectsReferencedStateBoundedBy(sdaiProjectInstance);
		getObjectsReferencedStateHasOpenings(sdaiProjectInstance);

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
	assert(sdaiInstance != 0);

	auto pGeometry = getGeometryByInstance(sdaiInstance);
	if (pGeometry != nullptr)
	{
		_ptr<_ifc_geometry>(pGeometry)->m_bIsReferenced = true;

		getObjectsReferencedStateIsDecomposedBy(sdaiInstance);
		getObjectsReferencedStateIsNestedBy(sdaiInstance);
		getObjectsReferencedStateContainsElements(sdaiInstance);
		getObjectsReferencedStateHasAssignments(sdaiInstance);
		getObjectsReferencedStateBoundedBy(sdaiInstance);
		getObjectsReferencedStateHasOpenings(sdaiInstance);
	}
	else
	{
		assert(FALSE);
	}
}

void _ifc_model::getObjectsReferencedStateIsDecomposedBy(SdaiInstance sdaiInstance)
{
	SdaiAggr sdaiIsDecomposedByAggr = nullptr;
	sdaiGetAttrBN(sdaiInstance, "IsDecomposedBy", sdaiAGGR, &sdaiIsDecomposedByAggr);
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

void _ifc_model::getObjectsReferencedStateIsNestedBy(SdaiInstance sdaiInstance)
{
	assert(sdaiInstance != 0);

	SdaiAggr sdaiIsNestedByAggr = nullptr;
	sdaiGetAttrBN(sdaiInstance, "IsNestedBy", sdaiAGGR, &sdaiIsNestedByAggr);
	if (sdaiIsNestedByAggr == nullptr)
	{
		return;
	}

	SdaiEntity sdaiRelNestsEntity = sdaiGetEntity(getSdaiModel(), "IFCRELNESTS");

	SdaiInteger iIsNestedByMembersCount = sdaiGetMemberCount(sdaiIsNestedByAggr);
	for (SdaiInteger i = 0; i < iIsNestedByMembersCount; ++i)
	{
		SdaiInstance sdaiIsNestedByInstance = 0;
		engiGetAggrElement(sdaiIsNestedByAggr, i, sdaiINSTANCE, &sdaiIsNestedByInstance);
		if (sdaiGetInstanceType(sdaiIsNestedByInstance) != sdaiRelNestsEntity)
		{
			continue;
		}

		SdaiAggr sdaiRelatedObjectsAggr = 0;
		sdaiGetAttrBN(sdaiIsNestedByInstance, "RelatedObjects", sdaiAGGR, &sdaiRelatedObjectsAggr);

		SdaiInteger iRelatedObjectsMembersCount = sdaiGetMemberCount(sdaiRelatedObjectsAggr);
		for (SdaiInteger j = 0; j < iRelatedObjectsMembersCount; ++j)
		{
			SdaiInstance sdaiRelatedObjectsInstance = 0;
			engiGetAggrElement(sdaiRelatedObjectsAggr, j, sdaiINSTANCE, &sdaiRelatedObjectsInstance);

			getObjectsReferencedStateRecursively(sdaiRelatedObjectsInstance);
		} // for (SdaiInteger j = ...
	} // for (SdaiInteger i = ...
}

void _ifc_model::getObjectsReferencedStateContainsElements(SdaiInstance sdaiInstance)
{
	assert(sdaiInstance != 0);

	SdaiAggr sdaiContainsElementsAggr = nullptr;
	sdaiGetAttrBN(sdaiInstance, "ContainsElements", sdaiAGGR, &sdaiContainsElementsAggr);
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

void _ifc_model::getObjectsReferencedStateHasAssignments(SdaiInstance sdaiInstance)
{
	assert(sdaiInstance != 0);

	SdaiAggr sdaiContainsElementsAggr = nullptr;
	sdaiGetAttrBN(sdaiInstance, "HasAssignments", sdaiAGGR, &sdaiContainsElementsAggr);
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

void _ifc_model::getObjectsReferencedStateBoundedBy(SdaiInstance sdaiInstance)
{
	assert(sdaiInstance != 0);

	SdaiAggr sdaiBoundedByAggr = nullptr;
	sdaiGetAttrBN(sdaiInstance, "BoundedBy", sdaiAGGR, &sdaiBoundedByAggr);

	if (sdaiBoundedByAggr == nullptr)
	{
		return;
	}

	SdaiInteger iBoundedByInstancesCount = sdaiGetMemberCount(sdaiBoundedByAggr);
	for (SdaiInteger i = 0; i < iBoundedByInstancesCount; ++i)
	{
		SdaiInstance sdaiBoundedByInstance = 0;
		engiGetAggrElement(sdaiBoundedByAggr, i, sdaiINSTANCE, &sdaiBoundedByInstance);

		if (sdaiIsKindOfBN(sdaiBoundedByInstance, "IFCRELSPACEBOUNDARY"))
		{
			getObjectsReferencedStateRecursively(sdaiBoundedByInstance);
		}
	}
}

void _ifc_model::getObjectsReferencedStateHasOpenings(SdaiInstance sdaiInstance)
{
	assert(sdaiInstance != 0);

	SdaiAggr sdaiHasOpeningsAggr = nullptr;
	sdaiGetAttrBN(sdaiInstance, "HasOpenings", sdaiAGGR, &sdaiHasOpeningsAggr);

	if (sdaiHasOpeningsAggr == nullptr)
	{
		return;
	}

	SdaiInteger iHasOpeningsInstancesCount = sdaiGetMemberCount(sdaiHasOpeningsAggr);
	for (SdaiInteger i = 0; i < iHasOpeningsInstancesCount; ++i)
	{
		SdaiInstance sdaiHasOpeningsInstance = 0;
		engiGetAggrElement(sdaiHasOpeningsAggr, i, sdaiINSTANCE, &sdaiHasOpeningsInstance);

		SdaiInstance sdaiRelatedOpeningElementInstance = 0;
		sdaiGetAttrBN(sdaiHasOpeningsInstance, "RelatedOpeningElement", sdaiINSTANCE, &sdaiRelatedOpeningElementInstance);

		getObjectsReferencedStateRecursively(sdaiRelatedOpeningElementInstance);
	}
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

		loadGeometry(szEntityName, sdaiInstance, false, iCircleSegements);
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

_geometry* _ifc_model::loadGeometry(const char* szEntityName, SdaiInstance sdaiInstance, bool bMappedItem, SdaiInteger iCircleSegments)
{
	auto pGeometry = dynamic_cast<_ifc_geometry*>(getGeometryByInstance(sdaiInstance));
	if (pGeometry != nullptr)
	{
		return pGeometry;
	}

	CString strEntity = (LPWSTR)CA2W(szEntityName);
	strEntity.MakeUpper();

	auto mappedItems = RecognizeMappedItems(sdaiInstance);
	if (mappedItems != nullptr)
	{
		int64_t iParenInstanceID = _model::getNextInstanceID();

		vector<_ifc_geometry*> vecMappedGeometries;
		vector<_ifc_instance*> vecMappedInstances;
		for (auto pMappedItem : mappedItems->mappedItems) 
		{
			auto pMappedGeometry = dynamic_cast<_ifc_geometry*>(getGeometryByInstance(pMappedItem->ifcRepresentationInstance));
			if (pMappedGeometry == nullptr)
			{
				pMappedGeometry = dynamic_cast<_ifc_geometry*>(loadGeometry(szEntityName, pMappedItem->ifcRepresentationInstance, true, iCircleSegments));
			}

			vecMappedGeometries.push_back(pMappedGeometry);

			auto pMappedInstance = createInstance(_model::getNextInstanceID(), pMappedGeometry, (_matrix4x3*)pMappedItem->matrixCoordinates);
			addInstance(pMappedInstance);

			pMappedInstance->setEnable(
				(strEntity == L"IFCSPACE") ||
				(strEntity == L"IFCRELSPACEBOUNDARY") ||
				(strEntity == L"IFCOPENINGELEMENT") ||
				(strEntity == L"IFCALIGNMENTVERTICAL") ||
				(strEntity == L"IFCALIGNMENTHORIZONTAL") ||
				(strEntity == L"IFCALIGNMENTSEGMENT") ||
				(strEntity == L"IFCALIGNMENTCANT") ? false : true);

			vecMappedInstances.push_back(pMappedInstance);
		} // for (auto pMappedItem : ...

		// Referenced By Geometry
		pGeometry = new _ifc_geometry(0, sdaiInstance, vecMappedGeometries);
		addGeometry(pGeometry);

		pGeometry->setShow(
			(strEntity == L"IFCRELSPACEBOUNDARY") ||
			(strEntity == L"IFCOPENINGELEMENT") ? false : true);

		// Referenced By Instance
		auto pInstance = createInstance(iParenInstanceID, pGeometry, nullptr);
		addInstance(pInstance);

		pInstance->setEnable(
			(strEntity == L"IFCSPACE") ||
			(strEntity == L"IFCRELSPACEBOUNDARY") ||
			(strEntity == L"IFCOPENINGELEMENT") ||
			(strEntity == L"IFCALIGNMENTVERTICAL") ||
			(strEntity == L"IFCALIGNMENTHORIZONTAL") ||
			(strEntity == L"IFCALIGNMENTSEGMENT") ||
			(strEntity == L"IFCALIGNMENTCANT") ? false : true);

		for (auto pMappedInstance : vecMappedInstances)
		{
			pMappedInstance->m_pOwner = pInstance;
			m_mapMapping[pMappedInstance] = pInstance;
		}

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

	if (!bMappedItem)
	{
		pGeometry->setShow(
			(strEntity == L"IFCRELSPACEBOUNDARY") ||
			(strEntity == L"IFCOPENINGELEMENT") ? false : true);

		auto pInstance = createInstance(_model::getNextInstanceID(), pGeometry, nullptr);
		addInstance(pInstance);
		pInstance->setEnable(
			(strEntity == L"IFCSPACE") ||
			(strEntity == L"IFCRELSPACEBOUNDARY") ||
			(strEntity == L"IFCOPENINGELEMENT") ||
			(strEntity == L"IFCALIGNMENTVERTICAL") ||
			(strEntity == L"IFCALIGNMENTHORIZONTAL") ||
			(strEntity == L"IFCALIGNMENTSEGMENT") ||
			(strEntity == L"IFCALIGNMENTCANT") ? false : true);
	}
	else
	{
		pGeometry->m_bIsMappedItem = true;
		pGeometry->m_bIsReferenced = true;
	}

	// Restore circleSegments()
	if (iCircleSegments != DEFAULT_CIRCLE_SEGMENTS)
	{
		circleSegments(DEFAULT_CIRCLE_SEGMENTS, 5);
	}

	return pGeometry;
}

//#todo#mappeditems
void	_ifc_model::ParseMappedItem(
	SdaiModel						ifcModel,
	SdaiInstance					ifcMappedItemInstance,
	std::vector<STRUCT_INTERNAL*>* pVectorMappedItemData
)
{
	OwlModel	owlModel = 0;
	owlGetModel(ifcModel, &owlModel);

	assert(sdaiGetInstanceType(ifcMappedItemInstance) == sdaiGetEntity(ifcModel, "IFCMAPPEDITEM"));

	SdaiInstance	ifcStyledItemInstance = 0;
	sdaiGetAttrBN(ifcMappedItemInstance, "StyledByItem", sdaiINSTANCE, &ifcStyledItemInstance);

	STRUCT_MATERIAL* material = nullptr;
	if (ifcStyledItemInstance) {
		assert(sdaiGetInstanceType(ifcStyledItemInstance) == sdaiGetEntity(ifcModel, "IFCSTYLEDITEM"));
		OwlInstance		owlInstanceMaterial = 0;
		owlBuildInstance(ifcModel, ifcStyledItemInstance, &owlInstanceMaterial);
		if (owlInstanceMaterial) {
			assert(GetInstanceClass(owlInstanceMaterial) == GetClassByName(owlModel, "Material"));
			material = new STRUCT_MATERIAL;
			material->ambient = GetMaterialColorAmbient(owlInstanceMaterial);
			material->diffuse = GetMaterialColorDiffuse(owlInstanceMaterial);
			material->emissive = GetMaterialColorEmissive(owlInstanceMaterial);
			material->specular = GetMaterialColorSpecular(owlInstanceMaterial);
		}
	}

	SdaiInstance	ifcRepresentationMapInstance = 0;
	sdaiGetAttrBN(ifcMappedItemInstance, "MappingSource", sdaiINSTANCE, &ifcRepresentationMapInstance);

	SdaiInstance	ifcCartesianTransformationOperatorInstance = 0;
	sdaiGetAttrBN(ifcMappedItemInstance, "MappingTarget", sdaiINSTANCE, &ifcCartesianTransformationOperatorInstance);

	OwlInstance		owlInstanceCartesianTransformationOperatorMatrix = 0;
	owlBuildInstance(ifcModel, ifcCartesianTransformationOperatorInstance, &owlInstanceCartesianTransformationOperatorMatrix);

	if (GetInstanceClass(owlInstanceCartesianTransformationOperatorMatrix) == GetClassByName(owlModel, "Transformation"))
		owlInstanceCartesianTransformationOperatorMatrix = GetObjectProperty(owlInstanceCartesianTransformationOperatorMatrix, GetPropertyByName(owlModel, "matrix"));
	assert(GetInstanceClass(owlInstanceCartesianTransformationOperatorMatrix) == GetClassByName(owlModel, "Matrix"));

	SdaiInstance	ifcAxis2PlacementInstance = 0;
	sdaiGetAttrBN(ifcRepresentationMapInstance, "MappingOrigin", sdaiINSTANCE, &ifcAxis2PlacementInstance);

	OwlInstance		owlInstanceAxis2PlacementMatrix = 0;
	owlBuildInstance(ifcModel, ifcAxis2PlacementInstance, &owlInstanceAxis2PlacementMatrix);
	assert(GetInstanceClass(owlInstanceAxis2PlacementMatrix) == GetClassByName(owlModel, "Matrix"));

	SdaiInstance	ifcRepresentationInstance = 0;
	sdaiGetAttrBN(ifcRepresentationMapInstance, "MappedRepresentation", sdaiINSTANCE, &ifcRepresentationInstance);

	OwlInstance		owlInstanceMatrix;
	if (owlInstanceCartesianTransformationOperatorMatrix && owlInstanceAxis2PlacementMatrix) {
		OwlInstance	owlInstanceMatrixMultiplication = CreateInstance(GetClassByName(owlModel, "MatrixMultiplication"));
		SetObjectProperty(owlInstanceMatrixMultiplication, GetPropertyByName(owlModel, "firstMatrix"), owlInstanceCartesianTransformationOperatorMatrix);
		SetObjectProperty(owlInstanceMatrixMultiplication, GetPropertyByName(owlModel, "secondMatrix"), owlInstanceAxis2PlacementMatrix);
		owlInstanceMatrix = owlInstanceMatrixMultiplication;
	}
	else {
		owlInstanceMatrix = owlInstanceCartesianTransformationOperatorMatrix & owlInstanceAxis2PlacementMatrix;
	}

	SdaiAggr	itemsAGGR = nullptr;
	sdaiGetAttrBN(ifcRepresentationInstance, "Items", sdaiAGGR, &itemsAGGR);
	SdaiInteger	noItemsAGGR = sdaiGetMemberCount(itemsAGGR);
	if (noItemsAGGR) {
		for (SdaiInteger index = 0; index < noItemsAGGR; index++) {
			SdaiInstance	ifcRepresentationItemInstance = 0;
			sdaiGetAggrByIndex(itemsAGGR, index, sdaiINSTANCE, &ifcRepresentationItemInstance);

			STRUCT_INTERNAL* mappedItemData = new STRUCT_INTERNAL;
			mappedItemData->owlInstanceMatrix = owlInstanceMatrix;
			mappedItemData->ifcRepresentationInstance = ifcRepresentationItemInstance;
#ifdef _DEBUG
			mappedItemData->ifcMappedItemInstance = ifcMappedItemInstance;
#endif // _DEBUG
			mappedItemData->material = material;
			(*pVectorMappedItemData).push_back(mappedItemData);
		}
	}
	else {
		//		assert(noItemsAGGR == 2);

		STRUCT_INTERNAL* mappedItemData = new STRUCT_INTERNAL;
		mappedItemData->owlInstanceMatrix = owlInstanceMatrix;
		mappedItemData->ifcRepresentationInstance = ifcRepresentationInstance;
#ifdef _DEBUG
		mappedItemData->ifcMappedItemInstance = ifcMappedItemInstance;
#endif // _DEBUG
		mappedItemData->material = material;
		(*pVectorMappedItemData).push_back(mappedItemData);
	}
}

//#todo#mappeditems
STRUCT_IFC_PRODUCT* _ifc_model::RecognizeMappedItems(
	SdaiInstance	ifcProductInstance
)
{
	SdaiModel		ifcModel = sdaiGetInstanceModel(ifcProductInstance);

	assert(ifcModel && ifcProductInstance);

	std::vector<STRUCT_INTERNAL*> vectorMappedItemData;

	{
		//
		//	If this object has opening elements mapped items should be ignored
		//
		SdaiAggr	hasOpeningsAGGR = nullptr;
		sdaiGetAttrBN(ifcProductInstance, "HasOpenings", sdaiAGGR, &hasOpeningsAGGR);
		if (hasOpeningsAGGR && sdaiGetMemberCount(hasOpeningsAGGR))
			return	nullptr;
	}

	//{
		SdaiEntity		ifcMappedItemEntity = sdaiGetEntity(ifcModel, "IFCMAPPEDITEM");

		SdaiInstance	ifcProductRepresentationInstance = 0;
		sdaiGetAttrBN(ifcProductInstance, "Representation", sdaiINSTANCE, &ifcProductRepresentationInstance);

		if (ifcProductRepresentationInstance) {
			SdaiAggr	ifcRepresentationInstanceAGGR = nullptr;
			sdaiGetAttrBN(ifcProductRepresentationInstance, "Representations", sdaiAGGR, &ifcRepresentationInstanceAGGR);

			SdaiInteger	noIfcRepresentationInstanceAGGR = sdaiGetMemberCount(ifcRepresentationInstanceAGGR);
			for (SdaiInteger index = 0; index < noIfcRepresentationInstanceAGGR; index++) {
				SdaiInstance	ifcRepresentationInstance = 0;
				sdaiGetAggrByIndex(ifcRepresentationInstanceAGGR, index, sdaiINSTANCE, &ifcRepresentationInstance);

				char* representationIdentifier = nullptr;
				sdaiGetAttrBN(ifcRepresentationInstance, "RepresentationIdentifier", sdaiSTRING, &representationIdentifier);
				if (Equals(representationIdentifier, "Body")) {
					SdaiAggr	ifcRepresentationItemInstanceAGGR = nullptr;
					sdaiGetAttrBN(ifcRepresentationInstance, "Items", sdaiAGGR, &ifcRepresentationItemInstanceAGGR);

					SdaiInteger	noIfcRepresentationItemInstanceAGGR = sdaiGetMemberCount(ifcRepresentationItemInstanceAGGR);
					for (SdaiInteger i = 0; i < noIfcRepresentationItemInstanceAGGR; i++) {
						SdaiInstance	ifcRepresentationItemInstance = 0;
						sdaiGetAggrByIndex(ifcRepresentationItemInstanceAGGR, i, sdaiINSTANCE, &ifcRepresentationItemInstance);

						if (sdaiGetInstanceType(ifcRepresentationItemInstance) == ifcMappedItemEntity) {
							ParseMappedItem(ifcModel, ifcRepresentationItemInstance, &vectorMappedItemData);
						}
						else
							return	nullptr;
					}
				}
			}
		}
	//}

	if (vectorMappedItemData.size()) {
		STRUCT_IFC_PRODUCT* myIfcProduct = new STRUCT_IFC_PRODUCT();
		myIfcProduct->ifcProductInstance = ifcProductInstance;
#ifdef _DEBUG
		myIfcProduct->ifcProductInstance__expressID = internalGetP21Line(myIfcProduct->ifcProductInstance);
#endif // _DEBUG

		OwlModel	owlModel = 0;
		owlGetModel(ifcModel, &owlModel);

		SdaiInstance	ifcObjectPlacementInstance = 0;
		sdaiGetAttrBN(ifcProductInstance, "ObjectPlacement", sdaiINSTANCE, &ifcObjectPlacementInstance);

		OwlInstance		owlInstanceObjectPlacementMatrix = 0;
		owlBuildInstance(ifcModel, ifcObjectPlacementInstance, &owlInstanceObjectPlacementMatrix);

		assert(GetInstanceClass(owlInstanceObjectPlacementMatrix) == GetClassByName(owlModel, "Matrix") ||
			GetInstanceClass(owlInstanceObjectPlacementMatrix) == GetClassByName(owlModel, "InverseMatrix") ||
			GetInstanceClass(owlInstanceObjectPlacementMatrix) == GetClassByName(owlModel, "MatrixMultiplication"));

		for (auto mappedItemData = vectorMappedItemData.begin(); mappedItemData != vectorMappedItemData.end(); ++mappedItemData) {
			OwlInstance	owlInstanceMatrixMultiplication = CreateInstance(GetClassByName(owlModel, "MatrixMultiplication"));
			SetObjectProperty(owlInstanceMatrixMultiplication, GetPropertyByName(owlModel, "firstMatrix"), (*mappedItemData)->owlInstanceMatrix);
			SetObjectProperty(owlInstanceMatrixMultiplication, GetPropertyByName(owlModel, "secondMatrix"), owlInstanceObjectPlacementMatrix);

			InferenceInstance(owlInstanceMatrixMultiplication);

			double* values = nullptr;
			int64_t	card = 0;
			GetDatatypeProperty(owlInstanceMatrixMultiplication, GetPropertyByName(owlModel, "coordinates"), (void**)&values, &card);
			if (card == 12) {
				STRUCT_MAPPED_ITEM* myMappedItem = new STRUCT_MAPPED_ITEM;
				myMappedItem->ifcRepresentationInstance = (*mappedItemData)->ifcRepresentationInstance;

				//			FILE* fp = 0;
				//			fopen_s(&fp, "d:\\0002\\OUT9871.TXT", "a");
				//			if (fp) {
				//				fprintf(fp, "  %I64i\n", myMappedItem->ifcRepresentationInstance);
				//				fclose(fp);
				//			}
				//			else {
				//				assert(false);
				//			}
			//CNT++;
#ifdef _DEBUG
				myMappedItem->ifcRepresentationInstance__expressID = internalGetP21Line(myMappedItem->ifcRepresentationInstance);
				myMappedItem->ifcMappedItemInstance__expressID = internalGetP21Line((*mappedItemData)->ifcMappedItemInstance);
#endif // _DEBUG

				memcpy(myMappedItem->matrixCoordinates, values, 12 * sizeof(double));

				//				myMappedItem->matrixCoordinatesOpenGL_16[0]  =   myMappedItem->matrixCoordinates[0];
				//				myMappedItem->matrixCoordinatesOpenGL_16[1]  = - myMappedItem->matrixCoordinates[2];
				//				myMappedItem->matrixCoordinatesOpenGL_16[2]  =   myMappedItem->matrixCoordinates[1];
				//				myMappedItem->matrixCoordinatesOpenGL_16[3]  =   0.;

				//				myMappedItem->matrixCoordinatesOpenGL_16[4]  =   myMappedItem->matrixCoordinates[3];
				//				myMappedItem->matrixCoordinatesOpenGL_16[5]  = - myMappedItem->matrixCoordinates[5];
				//				myMappedItem->matrixCoordinatesOpenGL_16[6]  =   myMappedItem->matrixCoordinates[4];
				//				myMappedItem->matrixCoordinatesOpenGL_16[7]  =   0.;

				//				myMappedItem->matrixCoordinatesOpenGL_16[8]  =   myMappedItem->matrixCoordinates[6];
				//				myMappedItem->matrixCoordinatesOpenGL_16[9]  = - myMappedItem->matrixCoordinates[8];
				//				myMappedItem->matrixCoordinatesOpenGL_16[10] =   myMappedItem->matrixCoordinates[7];
				//				myMappedItem->matrixCoordinatesOpenGL_16[11] =   0.;

				//				myMappedItem->matrixCoordinatesOpenGL_16[12] =   myMappedItem->matrixCoordinates[9];
				//				myMappedItem->matrixCoordinatesOpenGL_16[13] = - myMappedItem->matrixCoordinates[11];
				//				myMappedItem->matrixCoordinatesOpenGL_16[14] =   myMappedItem->matrixCoordinates[10];
				//				myMappedItem->matrixCoordinatesOpenGL_16[15] =   1.;

				myIfcProduct->mappedItems.push_back(myMappedItem);
			}
			else {
				assert(false);
				delete	myIfcProduct;
				return	nullptr;
			}
		}

		return	myIfcProduct;
	}

	return	nullptr;
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

