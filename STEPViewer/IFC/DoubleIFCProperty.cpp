#include "stdafx.h"
#include "DoubleIFCProperty.h"

// ------------------------------------------------------------------------------------------------
CDoubleIFCProperty::CDoubleIFCProperty(int64_t iInstance)
	: CIFCProperty(iInstance)
{
	m_iType = TYPE_DOUBLE_DATATYPE;
}

// ------------------------------------------------------------------------------------------------
CDoubleIFCProperty::~CDoubleIFCProperty()
{
}
