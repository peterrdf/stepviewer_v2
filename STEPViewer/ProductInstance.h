#pragma once

#include "Generic.h"
#include "Instance.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class CProductDefinition;

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class CProductInstance : public CInstance
{

	friend class CProductDefinition;

private: // Members

	int_t m_iID;

	CProductDefinition* m_pProductDefinition;
	OGLMATRIX* m_pTransformationMatrix;

	bool m_bEnable;	

public: // Methods
	
	CProductInstance(int_t iID, CProductDefinition* pDefinition, MATRIX* pTransformationMatrix);
	virtual ~CProductInstance();

	// CInstance
	virtual SdaiInstance GetInstance() const;
	virtual bool HasGeometry() const;
	virtual bool IsEnabled() const;
	
	int_t GetID() const;

	CProductDefinition* GetProductDefinition() const;
	OGLMATRIX* GetTransformationMatrix() const;

	bool GetEnable() const;
	void SetEnable(bool bEnable);

	void ScaleAndCenter(float fResoltuion);
};

