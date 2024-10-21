#include "stdafx.h"
#include "AP242Assembly.h"
#include "ProductDefinition.h"

// ************************************************************************************************
CAP242Assembly::CAP242Assembly(SdaiInstance iSdaiInstance, CProductDefinition* pRelatingProductDefinition, CProductDefinition* pRelatedProductDefinition)
	: m_iSdaiInstance(iSdaiInstance)
	, m_iExpressID(internalGetP21Line(iSdaiInstance))
	, m_szId(nullptr)
	, m_szName(nullptr)
	, m_szDescription(nullptr)
	, m_pRelatingProductDefinition(pRelatingProductDefinition)
	, m_pRelatedProductDefinition(pRelatedProductDefinition)
{
	ASSERT(m_iSdaiInstance != 0);
	ASSERT(m_iExpressID != 0);

	sdaiGetAttrBN(iSdaiInstance, "id", sdaiUNICODE, &m_szId);
	sdaiGetAttrBN(iSdaiInstance, "name", sdaiUNICODE, &m_szName);
	sdaiGetAttrBN(iSdaiInstance, "description", sdaiUNICODE, &m_szDescription);

	ASSERT(m_pRelatingProductDefinition != nullptr);
	ASSERT(m_pRelatedProductDefinition != nullptr);
}

/*virtual*/ CAP242Assembly::~CAP242Assembly()
{}