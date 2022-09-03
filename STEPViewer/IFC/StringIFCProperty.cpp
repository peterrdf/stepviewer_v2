#include "stdafx.h"
#include "StringIFCProperty.h"

// ------------------------------------------------------------------------------------------------
CStringIFCProperty::CStringIFCProperty(int64_t iInstance)
	: CIFCProperty(iInstance)
{
	m_iType = TYPE_CHAR_DATATYPE;
}

// ------------------------------------------------------------------------------------------------
CStringIFCProperty::~CStringIFCProperty()
{
}
