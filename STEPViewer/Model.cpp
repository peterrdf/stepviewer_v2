#include "stdafx.h"

#include "Model.h"

// ------------------------------------------------------------------------------------------------
CModel::CModel(enumModelType enModelType)
	: m_enModelType(enModelType)
	, m_iModel(0)
	, m_strFilePath(L"")
	, m_fXmin(-1.f)
	, m_fXmax(1.f)
	, m_fYmin(-1.f)
	, m_fYmax(1.f)
	, m_fZmin(-1.f)
	, m_fZmax(1.f)
	, m_fBoundingSphereDiameter(1.f)
	, m_fXTranslation(0.f)
	, m_fYTranslation(0.f)
	, m_fZTranslation(0.f)
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

// ------------------------------------------------------------------------------------------------
const wchar_t* CModel::GetModelName() const
{
	return m_strFilePath.c_str();
}

// ------------------------------------------------------------------------------------------------
int64_t CModel::GetInstance() const
{
	return m_iModel;
}

// ------------------------------------------------------------------------------------------------
void CModel::GetWorldDimensions(float& fXmin, float& fXmax, float& fYmin, float& fYmax, float& fZmin, float& fZmax) const
{
	fXmin = m_fXmin;
	fXmax = m_fXmax;
	fYmin = m_fYmin;
	fYmax = m_fYmax;
	fZmin = m_fZmin;
	fZmax = m_fZmax;
}

// ------------------------------------------------------------------------------------------------
void CModel::GetWorldTranslations(float& fXTranslation, float& fYTranslation, float& fZTranslation) const
{
	fXTranslation = m_fXTranslation;
	fYTranslation = m_fYTranslation;
	fZTranslation = m_fZTranslation;
}

// ------------------------------------------------------------------------------------------------
float CModel::GetBoundingSphereDiameter() const
{
	return m_fBoundingSphereDiameter;
}
