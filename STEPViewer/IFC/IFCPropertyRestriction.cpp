#include "stdafx.h"
#include "IFCPropertyRestriction.h"
#include <assert.h>

// ------------------------------------------------------------------------------------------------
CIFCPropertyRestriction::CIFCPropertyRestriction(int64_t iPropertyInstance, int64_t iMinCard, int64_t iMaxCard)
	: m_iPropertyInstance(iPropertyInstance)
	, m_iMinCard(iMinCard)
	, m_iMaxCard(iMaxCard)
{
	assert(m_iPropertyInstance != 0);
}

// ------------------------------------------------------------------------------------------------
CIFCPropertyRestriction::~CIFCPropertyRestriction()
{
}

// ------------------------------------------------------------------------------------------------
int64_t CIFCPropertyRestriction::getPropertyInstance() const
{
	return m_iPropertyInstance;
}

// --------------------------------------------------------------------------------------------
// Getter
int64_t CIFCPropertyRestriction::getMinCard() const
{
	return m_iMinCard;
}

// --------------------------------------------------------------------------------------------
// Getter
int64_t CIFCPropertyRestriction::getMaxCard() const
{
	return m_iMaxCard;
}
