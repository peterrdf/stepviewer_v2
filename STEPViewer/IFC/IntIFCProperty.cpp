#include "stdafx.h"
#include "IntIFCProperty.h"

// ------------------------------------------------------------------------------------------------
CIntIFCProperty::CIntIFCProperty(int64_t iInstance)
	: CIFCProperty(iInstance)
{
	m_iType = TYPE_INT_DATATYPE;
}

// ------------------------------------------------------------------------------------------------
CIntIFCProperty::~CIntIFCProperty()
{
}
