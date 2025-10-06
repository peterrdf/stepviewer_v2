#include "_host.h"
#include "_ifc_model_structure.h"
#include "_ifc_geometry.h"

// ************************************************************************************************
_ifc_node::_ifc_node(SdaiInstance sdaiInstance)
	: m_sdaiInstance(sdaiInstance)
	, m_vecChildren()
{
}

/*virtual*/ _ifc_node::~_ifc_node()
{
	for (auto pChild : m_vecChildren) {
		delete pChild;
	}
}

wchar_t* _ifc_node::getGlobalId() const
{
	wchar_t* szGlobalId = nullptr;
	sdaiGetAttrBN(m_sdaiInstance, "GlobalId", sdaiUNICODE, &szGlobalId);
	assert(szGlobalId != nullptr);

	return szGlobalId;
}

// ************************************************************************************************
_ifc_decomposition_node::_ifc_decomposition_node(SdaiInstance sdaiInstance)
	: _ifc_node(sdaiInstance)
{
}

/*virtual*/ _ifc_decomposition_node::~_ifc_decomposition_node()
{
}

// ************************************************************************************************
_ifc_contains_node::_ifc_contains_node(SdaiInstance sdaiInstance)
	: _ifc_node(sdaiInstance)
{
}

/*virtual*/ _ifc_contains_node::~_ifc_contains_node()
{
}

// ************************************************************************************************
_ifc_model_structure::_ifc_model_structure(_ifc_model* pModel)
	: m_pModel(pModel)
	, m_pRootNode(nullptr)
{
	assert(m_pModel != nullptr);

	build();
}

/*virtual*/ _ifc_model_structure::~_ifc_model_structure()
{
	clean();
}

void _ifc_model_structure::build()
{
	// Clean
	clean();

	//
	// Root
	// 

	SdaiAggr sdaiProjectAggr = sdaiGetEntityExtentBN(m_pModel->getSdaiModel(), "IFCPROJECT");
	SdaiInteger iProjectInstancesCount = sdaiGetMemberCount(sdaiProjectAggr);
	if (iProjectInstancesCount > 0) {
		SdaiInstance sdaiProjectInstance = 0;
		engiGetAggrElement(sdaiProjectAggr, 0, sdaiINSTANCE, &sdaiProjectInstance);
		assert(sdaiProjectInstance != 0);

		loadProjectNode(sdaiProjectInstance);
		//LoadGroups(m_pModel, m_pModel->GetGroupsItems()); //#todo
		//LoadUnreferencedItems(m_pModel, m_pModel->GetUnreferencedItems()); //#todo
	} // if (iProjectInstancesCount > 0)
}

void _ifc_model_structure::loadProjectNode(SdaiInstance sdaiProjectInstance)
{
	assert(sdaiProjectInstance != 0);
	assert(m_pRootNode == nullptr);
	m_pRootNode = new _ifc_node(sdaiProjectInstance);

	auto pGeometry = m_pModel->getGeometryByInstance(sdaiProjectInstance);
	if (pGeometry != nullptr) {
		_ptr<_ifc_geometry> ifcGeometry(pGeometry);
		assert(!ifcGeometry->getIsMappedItem());
		assert(pGeometry->getInstances().size() == 1);

		// decomposition/contains
		loadIsDecomposedBy(m_pRootNode, sdaiProjectInstance);
		loadIsNestedBy(m_pRootNode, sdaiProjectInstance);
		loadContainsElements(m_pRootNode, sdaiProjectInstance);
	}
}

void _ifc_model_structure::loadIsDecomposedBy(_ifc_node* pParent, SdaiInstance sdaiInstance)
{
	assert(pParent != nullptr);
	assert(sdaiInstance != 0);

	SdaiAggr sdaiIsDecomposedByAggr = nullptr;
	sdaiGetAttrBN(sdaiInstance, "IsDecomposedBy", sdaiAGGR, &sdaiIsDecomposedByAggr);

	if (sdaiIsDecomposedByAggr == nullptr) {
		return;
	}

	SdaiEntity sdaiRelAggregatesEntity = sdaiGetEntity(m_pModel->getSdaiModel(), "IFCRELAGGREGATES");

	SdaiInteger iIsDecomposedByInstancesCount = sdaiGetMemberCount(sdaiIsDecomposedByAggr);
	for (SdaiInteger i = 0; i < iIsDecomposedByInstancesCount; ++i) {
		SdaiInstance sdaiIsDecomposedByInstance = 0;
		engiGetAggrElement(sdaiIsDecomposedByAggr, i, sdaiINSTANCE, &sdaiIsDecomposedByInstance);

		if (sdaiGetInstanceType(sdaiIsDecomposedByInstance) != sdaiRelAggregatesEntity) {
			continue;
		}

		auto pDecomposition = new _ifc_decomposition_node(sdaiIsDecomposedByInstance);
		pParent->children().push_back(pDecomposition);

		SdaiAggr sdaiRelatedObjectsAggr = 0;
		sdaiGetAttrBN(sdaiIsDecomposedByInstance, "RelatedObjects", sdaiAGGR, &sdaiRelatedObjectsAggr);

		SdaiInteger iRelatedObjectsInstancesCount = sdaiGetMemberCount(sdaiRelatedObjectsAggr);
		for (SdaiInteger j = 0; j < iRelatedObjectsInstancesCount; ++j) {
			SdaiInstance sdaiRelatedObjectsInstance = 0;
			engiGetAggrElement(sdaiRelatedObjectsAggr, j, sdaiINSTANCE, &sdaiRelatedObjectsInstance);

			loadInstance(pDecomposition, sdaiRelatedObjectsInstance);
		}
	} // for (SdaiInteger i = ...	
}

void _ifc_model_structure::loadIsNestedBy(_ifc_node* pParent, SdaiInstance sdaiInstance)
{
	assert(pParent != nullptr);
	assert(sdaiInstance != 0);

	SdaiAggr sdaiIsNestedByAggr = nullptr;
	sdaiGetAttrBN(sdaiInstance, "IsNestedBy", sdaiAGGR, &sdaiIsNestedByAggr);
	if (sdaiIsNestedByAggr == nullptr) {
		return;
	}

	SdaiEntity sdaiRelNestsEntity = sdaiGetEntity(m_pModel->getSdaiModel(), "IFCRELNESTS");

	SdaiInteger iIsNestedByInstancesCount = sdaiGetMemberCount(sdaiIsNestedByAggr);
	for (SdaiInteger i = 0; i < iIsNestedByInstancesCount; ++i) {
		SdaiInstance sdaiIsNestedByInstance = 0;
		engiGetAggrElement(sdaiIsNestedByAggr, i, sdaiINSTANCE, &sdaiIsNestedByInstance);

		if (sdaiGetInstanceType(sdaiIsNestedByInstance) != sdaiRelNestsEntity) {
			continue;
		}
		auto pDecomposition = new _ifc_decomposition_node(sdaiIsNestedByInstance);
		pParent->children().push_back(pDecomposition);

		SdaiAggr sdaiRelatedObjectsAggr = 0;
		sdaiGetAttrBN(sdaiIsNestedByInstance, "RelatedObjects", sdaiAGGR, &sdaiRelatedObjectsAggr);

		SdaiInteger iRelatedObjectsInstancesCount = sdaiGetMemberCount(sdaiRelatedObjectsAggr);
		for (SdaiInteger j = 0; j < iRelatedObjectsInstancesCount; ++j) {
			SdaiInstance sdaiRelatedObjectsInstance = 0;
			engiGetAggrElement(sdaiRelatedObjectsAggr, j, sdaiINSTANCE, &sdaiRelatedObjectsInstance);

			loadInstance(pDecomposition, sdaiRelatedObjectsInstance);
		}
	} // for (SdaiInteger i = ...	
}

void _ifc_model_structure::loadContainsElements(_ifc_node* pParent, SdaiInstance sdaiInstance)
{
	assert(pParent != nullptr);
	assert(sdaiInstance != 0);

	SdaiAggr sdaiContainsElementsAggr = nullptr;
	sdaiGetAttrBN(sdaiInstance, "ContainsElements", sdaiAGGR, &sdaiContainsElementsAggr);
	if (sdaiContainsElementsAggr == nullptr) {
		return;
	}

	SdaiEntity sdaiRelContainedInSpatialStructureEntity = sdaiGetEntity(m_pModel->getSdaiModel(), "IFCRELCONTAINEDINSPATIALSTRUCTURE");
	SdaiInteger iContainsElementsInstancesCount = sdaiGetMemberCount(sdaiContainsElementsAggr);
	for (SdaiInteger i = 0; i < iContainsElementsInstancesCount; ++i) {
		SdaiInstance sdaiContainsElementsInstance = 0;
		engiGetAggrElement(sdaiContainsElementsAggr, i, sdaiINSTANCE, &sdaiContainsElementsInstance);

		if (sdaiGetInstanceType(sdaiContainsElementsInstance) != sdaiRelContainedInSpatialStructureEntity) {
			continue;
		}

		auto pContains = new _ifc_contains_node(sdaiContainsElementsInstance);
		pParent->children().push_back(pContains);

		SdaiAggr sdaiRelatedElementsInstances = 0;
		sdaiGetAttrBN(sdaiContainsElementsInstance, "RelatedElements", sdaiAGGR, &sdaiRelatedElementsInstances);

		SdaiInteger iIFCRelatedElementsInstancesCount = sdaiGetMemberCount(sdaiRelatedElementsInstances);
		for (SdaiInteger j = 0; j < iIFCRelatedElementsInstancesCount; ++j) {
			SdaiInstance sdaiRelatedElementsInstance = 0;
			engiGetAggrElement(sdaiRelatedElementsInstances, j, sdaiINSTANCE, &sdaiRelatedElementsInstance);

			loadInstance(pContains, sdaiRelatedElementsInstance);
		}
	} // for (SdaiInteger i = ...
}

void _ifc_model_structure::loadInstance(_ifc_node* pParent, SdaiInstance sdaiInstance)
{
	assert(pParent != nullptr);
	assert(sdaiInstance != 0);

	auto pGeometry = m_pModel->getGeometryByInstance(sdaiInstance);
	if (pGeometry != nullptr) {
		_ptr<_ifc_geometry> ifcGeometry(pGeometry);
		assert(!ifcGeometry->getIsMappedItem());
		assert(pGeometry->getInstances().size() == 1);

		_ifc_node* pInstance = new _ifc_node(sdaiInstance);
		pParent->children().push_back(pInstance);

		// decomposition/contains
		loadIsDecomposedBy(pInstance, sdaiInstance);
		loadIsNestedBy(pInstance, sdaiInstance);
		loadContainsElements(pInstance, sdaiInstance);
	}
}

void _ifc_model_structure::clean()
{
	if (m_pRootNode != nullptr) {
		delete m_pRootNode;
		m_pRootNode = nullptr;
	}
}