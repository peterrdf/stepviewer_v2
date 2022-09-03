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
	// Dimensions
	float m_fXmin;
	float m_fXmax;
	float m_fYmin;
	float m_fYmax;
	float m_fZmin;
	float m_fZmax;

	// --------------------------------------------------------------------------------------------
	OGLMATRIX* m_pTransformationMatrix;

	// --------------------------------------------------------------------------------------------
	bool m_bEnable;	

public: // Methods

	// --------------------------------------------------------------------------------------------
	CProductInstance(int_t iID, CProductDefinition* pProductDefinition, MATRIX* pTransformationMatrix);

	// --------------------------------------------------------------------------------------------
	virtual ~CProductInstance();

	// --------------------------------------------------------------------------------------------
	int_t getID() const;

	// --------------------------------------------------------------------------------------------
	CProductDefinition* getProductDefinition() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	void GetDimensions(float& fXmin, float& fXmax, float& fYmin, float& fYmax, float& fZmin, float& fZmax) const;

	// --------------------------------------------------------------------------------------------
	OGLMATRIX* getTransformationMatrix() const;

	// --------------------------------------------------------------------------------------------
	bool getEnable() const;

	// --------------------------------------------------------------------------------------------
	void setEnable(bool bEnable);

	// --------------------------------------------------------------------------------------------
	void ScaleAndCenter(float fXmin, float fXmax, float fYmin, float fYmax, float fZmin, float fZmax, float fResoltuion);
};

