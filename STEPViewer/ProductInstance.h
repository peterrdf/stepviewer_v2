#pragma once

#include "Generic.h"
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
	OGLMATRIX* m_pTransformationMatrix;

	bool m_bEnable;	

public: // Methods
	
	CProductInstance(int64_t iID, CProductDefinition* pProductDefinition, MATRIX* pTransformationMatrix);
	virtual ~CProductInstance();

	// CInstanceBase
	virtual SdaiInstance GetInstance() const;
	virtual bool HasGeometry() const;
	virtual bool IsEnabled() const { return GetEnable(); }

	void Scale(float fScaleFactor);
	
	int64_t GetID() const { return m_iID; }
	CProductDefinition* GetProductDefinition() const;
	OGLMATRIX* GetTransformationMatrix() const { return m_pTransformationMatrix; }
	bool GetEnable() const { return m_bEnable; }
	void SetEnable(bool bEnable) { m_bEnable = bEnable; }
};

