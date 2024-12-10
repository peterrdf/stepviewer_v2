#pragma once

#include "_ap_instance.h"
#include "InstanceBase.h"

// ************************************************************************************************
class CAP242ProductDefinition;

// ************************************************************************************************
class CAP242ProductInstance 
	: public CInstanceBase	
{
	friend class CAP242ProductDefinition;

private: // Members

	int64_t m_iID;
	CAP242ProductDefinition* m_pProductDefinition;
	_matrix4x4* m_pTransformationMatrix;

	bool m_bEnable;	

public: // Methods
	
	CAP242ProductInstance(int64_t iID, _geometry* pGeometry, _matrix4x3* pTransformationMatrix);
	virtual ~CAP242ProductInstance();

	// CInstanceBase
	virtual bool HasGeometry() const;
	virtual bool IsEnabled() const { return GetEnable(); }

	void Scale(float fScaleFactor);
	
	int64_t GetID() const { return m_iID; }
	CAP242ProductDefinition* GetProductDefinition() const { return m_pProductDefinition; }
	_matrix4x4* GetTransformationMatrix() const { return m_pTransformationMatrix; }
	bool GetEnable() const { return m_bEnable; }
	void SetEnable(bool bEnable) { m_bEnable = bEnable; }
};

