#include "stdafx.h"
#include "STEPItemData.h"

// ------------------------------------------------------------------------------------------------
CSTEPItemData::CSTEPItemData(CSTEPItemData* pParent, int64_t* pInstance, enumSTEPItemDataType enItemDataType)
	: m_pParent(pParent)
	, m_pInstance(pInstance)
	, m_enSTEPItemDataType(enItemDataType)
	, m_hItem(nullptr)
	, m_vecChildren()
	, m_bHasDescendantWithGeometry()
{
	ASSERT(m_pInstance != nullptr);
	ASSERT(m_enSTEPItemDataType != enumSTEPItemDataType::Unknown);
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