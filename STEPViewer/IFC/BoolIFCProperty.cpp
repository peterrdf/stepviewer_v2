#include "stdafx.h"
#include "BoolIFCProperty.h"

// ------------------------------------------------------------------------------------------------
CBoolIFCProperty::CBoolIFCProperty(int64_t iInstance)
	: CIFCProperty(iInstance)
{
	m_iType = TYPE_BOOL_DATATYPE;
}

// ------------------------------------------------------------------------------------------------
CBoolIFCProperty::~CBoolIFCProperty()
{
}
