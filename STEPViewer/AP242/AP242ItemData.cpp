#include "stdafx.h"
#include "AP242ItemData.h"

// ------------------------------------------------------------------------------------------------
CAP242ItemData::CAP242ItemData(CAP242ItemData* pParent, int64_t* pInstance, enumSTEPItemDataType enItemDataType)
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
CAP242ItemData::~CAP242ItemData()
{
}

// ------------------------------------------------------------------------------------------------
CAP242ItemData* CAP242ItemData::getParent()
{
	return m_pParent;
}

// ------------------------------------------------------------------------------------------------
enumSTEPItemDataType CAP242ItemData::getType() const
{
	return m_enSTEPItemDataType;
}

// ------------------------------------------------------------------------------------------------
HTREEITEM& CAP242ItemData::treeItem()
{
	return m_hItem;
}

// ------------------------------------------------------------------------------------------------
vector<CAP242ItemData*>& CAP242ItemData::children()
{
	return m_vecChildren;
}