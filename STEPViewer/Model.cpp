#include "stdafx.h"

#include "Model.h"

// ************************************************************************************************
CModel::CModel(enumAPModelType enType)
	: _ap_model(enType)
	, m_fXmin(-1.f)
	, m_fXmax(1.f)
	, m_fYmin(-1.f)
	, m_fYmax(1.f)
	, m_fZmin(-1.f)
	, m_fZmax(1.f)
	, m_dOriginalBoundingSphereDiameter(2.)
	, m_fBoundingSphereDiameter(2.f)
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