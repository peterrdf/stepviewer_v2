#include "stdafx.h"
#include "AP242ProductDefinition.h"
#include "AP242Model.h"

// ************************************************************************************************
CAP242ProductDefinition::CAP242ProductDefinition(SdaiInstance sdaiInstance)
	: _ap_geometry(sdaiInstance)
	, m_szId(nullptr)
	, m_szName(nullptr)
	, m_szDescription(nullptr)
	, m_szProductId(nullptr)
	, m_szProductName(nullptr)
	, m_iRelatingProducts(0)
	, m_iRelatedProducts(0)
	, m_iNextInstance(-1)
{
	sdaiGetAttrBN(sdaiInstance, "id", sdaiUNICODE, &m_szId);
	sdaiGetAttrBN(sdaiInstance, "name", sdaiUNICODE, &m_szName);
	sdaiGetAttrBN(sdaiInstance, "description", sdaiUNICODE, &m_szDescription);

	SdaiInstance sdaiFormationInstance = 0;
	sdaiGetAttrBN(sdaiInstance, "formation", sdaiINSTANCE, &sdaiFormationInstance);
	ASSERT(sdaiFormationInstance != 0);

	SdaiInstance sdaiOfProductInstance = 0;
	sdaiGetAttrBN(sdaiFormationInstance, "of_product", sdaiINSTANCE, &sdaiOfProductInstance);

	sdaiGetAttrBN(sdaiOfProductInstance, "id", sdaiUNICODE, &m_szProductId);
	sdaiGetAttrBN(sdaiOfProductInstance, "name", sdaiUNICODE, &m_szProductName);

	calculate();
}

/*virtual*/ CAP242ProductDefinition::~CAP242ProductDefinition()
{}

/*virtual*/ void CAP242ProductDefinition::preCalculate() /*override*/
{
	// Format
	setSTEPFormatSettings();

	// Extra settings
	setSegmentation(getSdaiModel(), 16, 0.);
}

int32_t CAP242ProductDefinition::GetNextInstance()
{
	if (++m_iNextInstance >= (int32_t)m_vecInstances.size())
	{
		m_iNextInstance = 0;
	}

	return m_iNextInstance;
}
