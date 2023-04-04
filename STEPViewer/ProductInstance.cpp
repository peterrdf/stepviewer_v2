#include "stdafx.h"
#include "ProductInstance.h"
#include "ProductDefinition.h"

// ------------------------------------------------------------------------------------------------
CProductInstance::CProductInstance(int_t iID, CProductDefinition* pDefinition, MATRIX* pTransformationMatrix)
	: CInstance()
	, m_iID(iID)
	, m_pProductDefinition(pDefinition)
	, m_pTransformationMatrix(nullptr)
	, m_bEnable(true)
{
	ASSERT(m_iID != 0);
	ASSERT(pDefinition != nullptr);

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
/*virtual*/ int64_t CProductInstance::GetInstance() const
{
	return m_pProductDefinition != nullptr ?
		m_pProductDefinition->GetInstance() :
		0;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ bool CProductInstance::HasGeometry() const
{
	return m_pProductDefinition != nullptr ?
		m_pProductDefinition->HasGeometry() :
		false;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ bool CProductInstance::IsEnabled() const
{
	return GetEnable();
}

// ------------------------------------------------------------------------------------------------
int_t CProductInstance::GetID() const
{
	return m_iID;
}

// ------------------------------------------------------------------------------------------------
CProductDefinition* CProductInstance::GetProductDefinition() const
{
	return m_pProductDefinition;
}

// ------------------------------------------------------------------------------------------------
OGLMATRIX* CProductInstance::GetTransformationMatrix() const
{
	return m_pTransformationMatrix;
}

// ------------------------------------------------------------------------------------------------
bool CProductInstance::GetEnable() const
{
	return m_bEnable;
}

// ------------------------------------------------------------------------------------------------
void CProductInstance::SetEnable(bool bEnable)
{
	m_bEnable = bEnable;
}

// ------------------------------------------------------------------------------------------------
void CProductInstance::ScaleAndCenter(float fResoltuion)
{
	/*
	* Transformation
	*/

	// [-1.0 -> 1.0]
	m_pTransformationMatrix->_41 /= (fResoltuion / 2.0f);
	m_pTransformationMatrix->_42 /= (fResoltuion / 2.0f);
	m_pTransformationMatrix->_43 /= (fResoltuion / 2.0f);
}