#include "stdafx.h"
#include "Assembly.h"
#include "ProductDefinition.h"

// ************************************************************************************************
CAssembly::CAssembly(SdaiInstance iInstance, CProductDefinition* pRelatingProductDefinition, CProductDefinition* pRelatedProductDefinition)
	: m_iInstance(iInstance)
	, m_iExpressID(internalGetP21Line(iInstance))
	, m_szId(nullptr)
	, m_szName(nullptr)
	, m_szDescription(nullptr)
	, m_pRelatingProductDefinition(pRelatingProductDefinition)
	, m_pRelatedProductDefinition(pRelatedProductDefinition)
{
	ASSERT(m_iInstance != 0);
	ASSERT(m_iExpressID != 0);

	sdaiGetAttrBN(iInstance, "id", sdaiUNICODE, &m_szId);
	sdaiGetAttrBN(iInstance, "name", sdaiUNICODE, &m_szName);
	sdaiGetAttrBN(iInstance, "description", sdaiUNICODE, &m_szDescription);

	ASSERT(m_pRelatingProductDefinition != nullptr);
	ASSERT(m_pRelatedProductDefinition != nullptr);
}

/*virtual*/ CAssembly::~CAssembly()
{}