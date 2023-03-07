#pragma once

#include "Generic.h"
#include "Instance.h"

// ------------------------------------------------------------------------------------------------
class CProductDefinition;

// ------------------------------------------------------------------------------------------------
class CProductInstance : public CInstance
{

	// --------------------------------------------------------------------------------------------
	friend class CProductDefinition;

private: // Members

	// --------------------------------------------------------------------------------------------
	int_t m_iID;

	// --------------------------------------------------------------------------------------------
	CProductDefinition* m_pProductDefinition;

	// --------------------------------------------------------------------------------------------
	OGLMATRIX* m_pTransformationMatrix;

	// --------------------------------------------------------------------------------------------
	bool m_bEnable;	

public: // Methods
	
	CProductInstance(int_t iID, CProductDefinition* pDefinition, MATRIX* pTransformationMatrix);
	virtual ~CProductInstance();

	// CInstance
	virtual int64_t GetInstance() const;
	virtual bool HasGeometry() const;
	virtual bool IsEnabled() const;
	virtual wstring GetName() const;

	// --------------------------------------------------------------------------------------------
	int_t getID() const;

	// --------------------------------------------------------------------------------------------
	CProductDefinition* getProductDefinition() const;

	// --------------------------------------------------------------------------------------------
	OGLMATRIX* getTransformationMatrix() const;

	// --------------------------------------------------------------------------------------------
	bool getEnable() const;

	// --------------------------------------------------------------------------------------------
	void setEnable(bool bEnable);

	// --------------------------------------------------------------------------------------------
	void ScaleAndCenter(float fResoltuion);
};

