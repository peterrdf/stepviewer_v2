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
/*virtual*/ int64_t CProductInstance::_getInstance() const
{
	return m_pProductDefinition != nullptr ?
		m_pProductDefinition->getInstance() :
		0;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ bool CProductInstance::_hasGeometry() const
{
	return m_pProductDefinition != nullptr ?
		m_pProductDefinition->hasGeometry() :
		false;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ bool CProductInstance::_isEnabled() const
{
	return getEnable();
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ wstring CProductInstance::_getName() const
{	
	CString strName;

	if (m_pProductDefinition != nullptr)
	{	
		strName.Format(_T("#%lld"), m_pProductDefinition->getInstance());
		strName += L" ";
		strName += m_pProductDefinition->getName();
	}
	else
	{
		strName.Format(_T("#%lld"), m_iID);
	}

	return (LPCTSTR)strName;
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