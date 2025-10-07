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

wchar_t* _ap242_node::getGlobalId() const
{
	wchar_t* szGlobalId = nullptr;
	sdaiGetAttrBN(m_sdaiInstance, "GlobalId", sdaiUNICODE, &szGlobalId);
	assert(szGlobalId != nullptr);

	return szGlobalId;
}

// ************************************************************************************************
_ap242_model_structure::_ap242_model_structure(_ap242_model* pModel)
	: m_pModel(pModel)
	, m_vecRootsProducts()

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
	auto pModel = _ptr<_ap242_model>(m_pModel);
	if (pModel == nullptr) {
		return;
	}

	for (auto pGeometry : pModel->getGeometries()) {
		auto pProduct = dynamic_cast<_ap242_product_definition*>(pGeometry);
		if ((pProduct != nullptr) && (pProduct->getRelatedProducts() == 0)) {
			m_vecRootsProducts.push_back(new _ap242_node(pProduct->getSdaiInstance(), __nullptr));
		}
	}

	//#todo
	// Draughitng models
	/*for (auto pDraughtingModel : pModel->getDraughtingModels()) {
		LoadDraughtingModel(pDraughtingModel, hModel);
	}*/
}

void _ap242_model_structure::clean()
{
	for (auto pRoot : m_vecRootsProducts) {
		delete pRoot;
	}
	m_vecRootsProducts.clear();
}