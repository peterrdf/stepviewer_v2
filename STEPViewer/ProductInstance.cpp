#include "stdafx.h"
#include "ProductInstance.h"
#include "ProductDefinition.h"

// ------------------------------------------------------------------------------------------------
CProductInstance::CProductInstance(int_t iID, CProductDefinition* pProductDefinition, MATRIX* pTransformationMatrix)
	: CSTEPInstance()
	, m_iID(iID)
	, m_pProductDefinition(pProductDefinition)
	, m_pTransformationMatrix(nullptr)
	, m_bEnable(true)
	, m_fXmin(-1.f)
	, m_fXmax(1.f)
	, m_fYmin(-1.f)
	, m_fYmax(1.f)
	, m_fZmin(-1.f)
	, m_fZmax(1.f)
{
	ASSERT(m_iID != 0);
	ASSERT(pProductDefinition != nullptr);

	m_pTransformationMatrix = new OGLMATRIX();
	OGLMatrixIdentity(m_pTransformationMatrix);

	if (pTransformationMatrix != nullptr) 
	{
		m_pTransformationMatrix->_11 = pTransformationMatrix->_11;
		m_pTransformationMatrix->_12 = pTransformationMatrix->_12;
		m_pTransformationMatrix->_13 = pTransformationMatrix->_13;
		m_pTransformationMatrix->_21 = pTransformationMatrix->_21;
		m_pTransformationMatrix->_22 = pTransformationMatrix->_22;
		m_pTransformationMatrix->_23 = pTransformationMatrix->_23;
		m_pTransformationMatrix->_31 = pTransformationMatrix->_31;
		m_pTransformationMatrix->_32 = pTransformationMatrix->_32;
		m_pTransformationMatrix->_33 = pTransformationMatrix->_33;
		m_pTransformationMatrix->_41 = pTransformationMatrix->_41;
		m_pTransformationMatrix->_42 = pTransformationMatrix->_42;
		m_pTransformationMatrix->_43 = pTransformationMatrix->_43;
	}
}

// ------------------------------------------------------------------------------------------------
CProductInstance::~CProductInstance()
{
	delete m_pTransformationMatrix;
}

// ------------------------------------------------------------------------------------------------
int_t CProductInstance::getID() const
{
	return m_iID;
}

// ------------------------------------------------------------------------------------------------
CProductDefinition* CProductInstance::getProductDefinition() const
{
	return m_pProductDefinition;
}

// ------------------------------------------------------------------------------------------------
void CProductInstance::GetDimensions(float& fXmin, float& fXmax, float& fYmin, float& fYmax, float& fZmin, float& fZmax) const
{
	fXmin = m_fXmin;
	fXmax = m_fXmax;
	fYmin = m_fYmin;
	fYmax = m_fYmax;
	fZmin = m_fZmin;
	fZmax = m_fZmax;
}

// ------------------------------------------------------------------------------------------------
OGLMATRIX* CProductInstance::getTransformationMatrix() const
{
	return m_pTransformationMatrix;
}

// ------------------------------------------------------------------------------------------------
bool CProductInstance::getEnable() const
{
	return m_bEnable;
}

// ------------------------------------------------------------------------------------------------
void CProductInstance::setEnable(bool bEnable)
{
	m_bEnable = bEnable;
}

// ------------------------------------------------------------------------------------------------
void CProductInstance::ScaleAndCenter(float fXmin, float fXmax, float fYmin, float fYmax, float fZmin, float fZmax, float fResoltuion)
{
	/*
	* Dimensions
	*/

	// [0.0 -> X/Y/Zmin + X/Y/Zmax]
	m_fXmin -= fXmin;
	m_fYmin -= fYmin;
	m_fZmin -= fZmin;
	m_fXmax -= fXmin;
	m_fYmax -= fYmin;
	m_fZmax -= fZmin;

	// center
	m_fXmin -= ((fXmax - fXmin) / 2.0f);
	m_fYmin -= ((fYmax - fYmin) / 2.0f);
	m_fZmin -= ((fZmax - fZmin) / 2.0f);
	m_fXmax -= ((fXmax - fXmin) / 2.0f);
	m_fYmax -= ((fYmax - fYmin) / 2.0f);
	m_fZmax -= ((fZmax - fZmin) / 2.0f);

	// [-1.0 -> 1.0]
	m_fXmin /= (fResoltuion / 2.0f);
	m_fYmin /= (fResoltuion / 2.0f);
	m_fZmin /= (fResoltuion / 2.0f);
	m_fXmax /= (fResoltuion / 2.0f);
	m_fYmax /= (fResoltuion / 2.0f);
	m_fZmax /= (fResoltuion / 2.0f);

	/*
	* Transformation
	*/

	// [-1.0 -> 1.0]
	m_pTransformationMatrix->_41 /= (fResoltuion / 2.0f);
	m_pTransformationMatrix->_42 /= (fResoltuion / 2.0f);
	m_pTransformationMatrix->_43 /= (fResoltuion / 2.0f);
}