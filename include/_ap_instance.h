#pragma once

#include "_geometry.h"

// ************************************************************************************************
class _ap_instance : public _instance
{

private: // Members

	SdaiInstance m_sdaiInstance;
	ExpressID m_expressID;

public:  // Methods

	_ap_instance(int64_t iID, _geometry* pGeometry, SdaiInstance sdaiInstance, _matrix4x3* pTransformationMatrix)
		: _instance(iID, pGeometry, pTransformationMatrix)
		, m_sdaiInstance(sdaiInstance)
		, m_expressID(internalGetP21Line(m_sdaiInstance))
	{
		assert(m_sdaiInstance != 0);
		assert(m_expressID != 0);
	}

	virtual ~_ap_instance()
	{
	}

	SdaiInstance getSdaiInstance() const { return m_sdaiInstance; } 
	ExpressID getExpressID() const { return m_expressID; }
};