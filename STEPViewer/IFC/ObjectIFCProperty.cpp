#include "stdafx.h"
#include "ObjectIFCProperty.h"

// ------------------------------------------------------------------------------------------------
CObjectIFCProperty::CObjectIFCProperty(int64_t iInstance)
	: CIFCProperty(iInstance)
	, m_vecRestrictions()
{
	m_iType = TYPE_OBJECTTYPE;

	int64_t	iRestrictionsClassInstance = GetRangeRestrictionsByIterator(getInstance(), 0);
	while (iRestrictionsClassInstance != 0)
	{
		char * szRestrictionsClassName = nullptr;
		GetNameOfClass(iRestrictionsClassInstance, &szRestrictionsClassName);

		m_vecRestrictions.push_back(iRestrictionsClassInstance);

		iRestrictionsClassInstance = GetRangeRestrictionsByIterator(getInstance(), iRestrictionsClassInstance);
	} // while (iRestrictionsClassInstance != 0)
}

// ------------------------------------------------------------------------------------------------
CObjectIFCProperty::~CObjectIFCProperty()
{
}

// ------------------------------------------------------------------------------------------------
// Getter
const vector<int64_t> & CObjectIFCProperty::getRestrictions()
{
	return m_vecRestrictions;
}
