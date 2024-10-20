#include "stdafx.h"
#include "STEPItemData.h"

// ------------------------------------------------------------------------------------------------
CSTEPItemData::CSTEPItemData(CSTEPItemData* pParent, SdaiInstance pSdaiInstance, enumSTEPItemDataType enItemDataType)
	: m_pParent(pParent)
	, m_pSdaiInstance(pSdaiInstance)
	, m_pAssembly()
	, m_pAP242Model()
	, m_pProductDefinition()
	, m_pProductInstance()
	, m_enSTEPItemDataType(enItemDataType)
	, m_hItem(nullptr)
	, m_vecChildren()
	, m_bHasDescendantWithGeometry()
{
	ASSERT(m_pSdaiInstance != 0);
	ASSERT(m_enSTEPItemDataType != enumSTEPItemDataType::ProductInstance);
}

// ------------------------------------------------------------------------------------------------
CSTEPItemData::CSTEPItemData(CSTEPItemData* pParent, CAssembly* pAssembly, enumSTEPItemDataType enItemDataType)
	: m_pParent(pParent)
	, m_pSdaiInstance()
	, m_pAssembly(pAssembly)
	, m_pAP242Model()
	, m_pProductDefinition()
	, m_pProductInstance()
	, m_enSTEPItemDataType(enItemDataType)
	, m_hItem(nullptr)
	, m_vecChildren()
	, m_bHasDescendantWithGeometry()
{
	ASSERT(m_pAssembly != 0);
	ASSERT(m_enSTEPItemDataType == enumSTEPItemDataType::Assembly);
}

// ------------------------------------------------------------------------------------------------
CSTEPItemData::CSTEPItemData(CSTEPItemData* pParent, CAP242Model* pAP242Model, enumSTEPItemDataType enItemDataType)
	: m_pParent(pParent)
	, m_pSdaiInstance()
	, m_pAssembly()
	, m_pAP242Model(pAP242Model)
	, m_pProductDefinition()
	, m_pProductInstance()
	, m_enSTEPItemDataType(enItemDataType)
	, m_hItem(nullptr)
	, m_vecChildren()
	, m_bHasDescendantWithGeometry()
{
	ASSERT(m_pAP242Model != 0);
	ASSERT(m_enSTEPItemDataType == enumSTEPItemDataType::Model);
}

// ------------------------------------------------------------------------------------------------
CSTEPItemData::CSTEPItemData(CSTEPItemData* pParent, CProductDefinition* pProductDefinition, enumSTEPItemDataType enItemDataType)
	: m_pParent(pParent)
	, m_pSdaiInstance()
	, m_pAssembly()
	, m_pAP242Model()
	, m_pProductDefinition(pProductDefinition)
	, m_pProductInstance()
	, m_enSTEPItemDataType(enItemDataType)
	, m_hItem(nullptr)
	, m_vecChildren()
	, m_bHasDescendantWithGeometry()
{
	ASSERT(m_pProductDefinition != 0);
	ASSERT(m_enSTEPItemDataType == enumSTEPItemDataType::ProductDefinition);
}

// ------------------------------------------------------------------------------------------------
CSTEPItemData::CSTEPItemData(CSTEPItemData* pParent, CProductInstance* pProductInstance, enumSTEPItemDataType enItemDataType)
	: m_pParent(pParent)
	, m_pSdaiInstance()
	, m_pAssembly()
	, m_pAP242Model()
	, m_pProductDefinition()
	, m_pProductInstance(pProductInstance)
	, m_enSTEPItemDataType(enItemDataType)
	, m_hItem(nullptr)
	, m_vecChildren()
	, m_bHasDescendantWithGeometry()
{
	ASSERT(m_pProductInstance != 0);
	ASSERT(m_enSTEPItemDataType == enumSTEPItemDataType::ProductInstance);
}

// ------------------------------------------------------------------------------------------------
CSTEPItemData::~CSTEPItemData()
{
}

// ------------------------------------------------------------------------------------------------
CSTEPItemData* CSTEPItemData::getParent()
{
	return m_pParent;
}

// ------------------------------------------------------------------------------------------------
enumSTEPItemDataType CSTEPItemData::getType() const
{
	return m_enSTEPItemDataType;
}

// ------------------------------------------------------------------------------------------------
HTREEITEM& CSTEPItemData::treeItem()
{
	return m_hItem;
}

// ------------------------------------------------------------------------------------------------
vector<CSTEPItemData*>& CSTEPItemData::children()
{
	return m_vecChildren;
}