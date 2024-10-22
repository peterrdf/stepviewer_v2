#include "stdafx.h"
#include "AP242ItemData.h"

// ------------------------------------------------------------------------------------------------
CAP242ItemData::CAP242ItemData(CAP242ItemData* pParent, SdaiInstance pSdaiInstance, enumSTEPItemDataType enItemDataType)
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
CAP242ItemData::CAP242ItemData(CAP242ItemData* pParent, CAP242Assembly* pAssembly, enumSTEPItemDataType enItemDataType)
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
CAP242ItemData::CAP242ItemData(CAP242ItemData* pParent, CAP242Model* pAP242Model, enumSTEPItemDataType enItemDataType)
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
CAP242ItemData::CAP242ItemData(CAP242ItemData* pParent, CAP242ProductDefinition* pProductDefinition, enumSTEPItemDataType enItemDataType)
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
CAP242ItemData::CAP242ItemData(CAP242ItemData* pParent, CAP242ProductInstance* pProductInstance, enumSTEPItemDataType enItemDataType)
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