#include "stdafx.h"

#include "STEPModelBase.h"

// ------------------------------------------------------------------------------------------------
CSTEPModelBase::CSTEPModelBase(enumSTEPModelType enSTEPModelType)
	: m_enSTEPModelType(enSTEPModelType)
{
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ CSTEPModelBase::~CSTEPModelBase()
{
}

// ------------------------------------------------------------------------------------------------
enumSTEPModelType CSTEPModelBase::GetType() const
{
	return m_enSTEPModelType;
}