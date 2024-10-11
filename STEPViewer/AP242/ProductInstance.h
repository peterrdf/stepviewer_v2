#pragma once

#include "_geometry.h"
#include "InstanceBase.h"

// ************************************************************************************************
class CProductDefinition;

// ************************************************************************************************
class CProductInstance 
	: public CInstanceBase
{
	friend class CProductDefinition;

private: // Members

	int64_t m_iID;
	CProductDefinition* m_pProductDefinition;
	_matrix4x4* m_pTransformationMatrix;

	bool m_bEnable;	

public: // Methods
	
	CProductInstance(int64_t iID, CProductDefinition* pProductDefinition, _matrix4x3* pTransformationMatrix);
	virtual ~CProductInstance();

	// CInstanceBase
	virtual SdaiInstance GetInstance() const;
	virtual bool HasGeometry() const;
	virtual bool IsEnabled() const { return GetEnable(); }

	void Scale(float fScaleFactor);
	
	int64_t GetID() const { return m_iID; }
	CProductDefinition* GetProductDefinition() const { return m_pProductDefinition; }
	_matrix4x4* GetTransformationMatrix() const { return m_pTransformationMatrix; }
	bool GetEnable() const { return m_bEnable; }
	void SetEnable(bool bEnable) { m_bEnable = bEnable; }
};

