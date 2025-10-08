#include "_host.h"
#include "_ap242_model_structure.h"

#include "_ap242_product_definition.h"
#include "_ap242_instance.h"
#include "_ap242_draughting_model.h"

// ************************************************************************************************
_ap242_node::_ap242_node(SdaiInstance sdaiInstance, _ap242_node* pParentNode)
	: m_sdaiInstance(sdaiInstance)
	, m_pParent(pParentNode)
	, m_vecChildren()	
{}

/*virtual*/ _ap242_node::~_ap242_node()
{
	for (auto pChild : m_vecChildren) {
		delete pChild;
	}
}

// ************************************************************************************************
_ap242_model_structure::_ap242_model_structure(_ap242_model* pModel)
	: m_pModel(pModel)
	, m_vecRootProducts()
	, m_mapInstanceIterators()

{
	assert(m_pModel != nullptr);
}

/*virtual*/ _ap242_model_structure::~_ap242_model_structure()
{
	clean();
}

#ifdef _DEBUG
void _ap242_model_structure::print(int iLevel/* = 0*/, _ap242_node* pNode/* = nullptr*/)
{
	//#todo
	/*if (pNode == nullptr) {
		pNode = m_pProjectNode;
	}
	if (pNode == nullptr) {
		return;
	}
	for (int i = 0; i < iLevel; ++i) {
		TRACE("  ");
	}
	wchar_t* szGlobalId = pNode->getGlobalId();
	if (szGlobalId != nullptr) {
		TRACE(L"%s\n", szGlobalId);
	}
	else {
		TRACE("(null)\n");
	}
	for (auto pChild : pNode->children()) {
		print(iLevel + 1, pChild);
	}*/
}
#endif

void _ap242_model_structure::build()
{
	// Clean
	clean();

	//
	// Roots
	// 

	for (auto pGeometry : m_pModel->getGeometries()) {
		auto pProduct = dynamic_cast<_ap242_product_definition*>(pGeometry);
		if ((pProduct != nullptr) && (pProduct->getRelatedProducts() == 0)) {
			m_vecRootProducts.push_back(new _ap242_node(pProduct->getSdaiInstance(), __nullptr));
			loadProductNode(m_vecRootProducts.back(), pProduct);
		}
	}
	
	// Draughitng models #todo
	/*for (auto pDraughtingModel : pModel->getDraughtingModels()) {
		LoadDraughtingModel(pDraughtingModel, hModel);
	}*/
}

void _ap242_model_structure::loadProductNode(_ap242_node* pParentNode, _ap242_product_definition* pProduct)
{
	assert(pParentNode != nullptr);
	assert(pProduct != nullptr);

	// Iterator
	_instance_iterator* pInstanceIterator = nullptr;
	auto itInstanceIterator = m_mapInstanceIterators.find(pProduct);
	if (itInstanceIterator == m_mapInstanceIterators.end()) {
		pInstanceIterator = new _instance_iterator(pProduct->getInstances());
		m_mapInstanceIterators[pProduct] = pInstanceIterator;
	}
	else {
		pInstanceIterator = itInstanceIterator->second;
	}

	// Assemblies
	for (auto itExpressID2Assembly : m_pModel->getExpressID2Assembly()) {
		if (itExpressID2Assembly.second->getRelatingProductDefinition() == pProduct) {
			pParentNode->children().push_back(new _ap242_node(itExpressID2Assembly.second->getSdaiInstance(), pParentNode));
			//loadProductNode(pParentNode->children().back(), itExpressID2Assembly.second->getRelatedProductDefinition());
		}
	}
}

void _ap242_model_structure::clean()
{
	for (auto pRoot : m_vecRootProducts) {
		delete pRoot;
	}
	m_vecRootProducts.clear();

	for (auto itInstanceIterator : m_mapInstanceIterators) {
		delete itInstanceIterator.second;
	}
	m_mapInstanceIterators.clear();
}