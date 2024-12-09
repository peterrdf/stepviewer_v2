#include "stdafx.h"

#include "Model.h"

// ************************************************************************************************
CModel::CModel(enumAPModelType enType)
	: _ap_model(enType)	
{}

void CModel::GetWorldDimensions(float& fXmin, float& fXmax, float& fYmin, float& fYmax, float& fZmin, float& fZmax) const
{
	fXmin = m_fXmin;
	fXmax = m_fXmax;
	fYmin = m_fYmin;
	fYmax = m_fYmax;
	fZmin = m_fZmin;
	fZmax = m_fZmax;
}