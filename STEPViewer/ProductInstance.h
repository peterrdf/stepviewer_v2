#pragma once

#include "Generic.h"
#include "InstanceBase.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class CProductDefinition;

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class CProductInstance : public CInstanceBase
{

	friend class CProductDefinition;

private: // Members

	int64_t m_iID;

	CProductDefinition* m_pProductDefinition;
	OGLMATRIX* m_pTransformationMatrix;

	bool m_bEnable;	

public: // Methods
	
	CProductInstance(int64_t iID, CProductDefinition* pDefinition, MATRIX* pTransformationMatrix);
	virtual ~CProductInstance();

	// CInstanceBase
	virtual SdaiInstance GetInstance() const;
	virtual bool HasGeometry() const;
	virtual bool IsEnabled() const;
	
	int64_t GetID() const;

	CProductDefinition* GetProductDefinition() const;
	OGLMATRIX* GetTransformationMatrix() const;

	bool GetEnable() const;
	void SetEnable(bool bEnable);

	void Scale(float fScaleFactor);
};

