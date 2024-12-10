#include "stdafx.h"
#include "AP242ProductInstance.h"
#include "AP242ProductDefinition.h"

// ************************************************************************************************
CAP242ProductInstance::CAP242ProductInstance(int64_t iID, _geometry* pGeometry, _matrix4x3* pTransformationMatrix)
	: CInstanceBase(iID, pGeometry, pTransformationMatrix)
	, m_iID(iID)
	, m_pProductDefinition((CAP242ProductDefinition*)pGeometry)
	, m_pTransformationMatrix(nullptr)
	, m_bEnable(true)
{
	ASSERT(m_iID != 0);
	ASSERT(m_pProductDefinition != nullptr);

	m_pTransformationMatrix = new _matrix4x4();
	_matrix4x4Identity(m_pTransformationMatrix);

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

CAP242ProductInstance::~CAP242ProductInstance()
{
	delete m_pTransformationMatrix;
}

/*virtual*/  bool CAP242ProductInstance::HasGeometry() const 
{ 
	return m_pProductDefinition->hasGeometry(); 
}

void CAP242ProductInstance::Scale(float fScaleFactor)
{
	m_pTransformationMatrix->_41 /= fScaleFactor;
	m_pTransformationMatrix->_42 /= fScaleFactor;
	m_pTransformationMatrix->_43 /= fScaleFactor;
}
