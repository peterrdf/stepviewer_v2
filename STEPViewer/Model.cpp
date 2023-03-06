#include "stdafx.h"

#include "Model.h"

// ------------------------------------------------------------------------------------------------
CModel::CModel(enumModelType enModelType)
	: m_enModelType(enModelType)
{
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ CModel::~CModel()
{
}

// ------------------------------------------------------------------------------------------------
enumModelType CModel::GetType() const
{
	return m_enModelType;
}