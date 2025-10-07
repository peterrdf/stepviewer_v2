#include "_host.h"
#include "_ap242_model_structure.h"

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
	, m_pProjectNode(nullptr)

{
	assert(m_pModel != nullptr);
}

/*virtual*/ _ap242_model_structure::~_ap242_model_structure()
{
	clean();
}

#ifdef _DEBUG
void _ifc_model_structure::print(int iLevel/* = 0*/, _ifc_node* pNode/* = nullptr*/)
{
	if (pNode == nullptr) {
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
	}
}
#endif

void _ap242_model_structure::build()
{

}

void _ap242_model_structure::clean()
{
	if (m_pProjectNode != nullptr) {
		delete m_pProjectNode;
		m_pProjectNode = nullptr;
	}
}