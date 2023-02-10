#pragma once

#include "Generic.h"
#include "STEPInstance.h"

// ------------------------------------------------------------------------------------------------
class CProductDefinition;

// ------------------------------------------------------------------------------------------------
class CProductInstance : public CSTEPInstance
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

	// CSTEPInstance
	virtual int64_t _getInstance() const;
	virtual bool _hasGeometry() const;
	virtual bool _isEnabled() const;
	virtual wstring _getName() const;

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

