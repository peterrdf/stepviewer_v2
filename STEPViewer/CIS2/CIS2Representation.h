#ifndef CIS2_REPRESENTAION_H
#define CIS2_REPRESENTAION_H

#include "_oglUtils.h"
#include "InstanceBase.h"

#include "engine.h"

#include <cstddef>
#include <string>
using namespace std;

// ************************************************************************************************
class CCIS2Representation
	: public _geometry
	, public CInstanceBase
{

private: // Members

	// Metadata
	ExpressID m_iExpressID;
	
	// UI
	bool m_bReferenced;

public: // Methods
	
	CCIS2Representation(int64_t iID, SdaiInstance iSdaiInstance);
	virtual ~CCIS2Representation();

	// _geometry
	virtual OwlModel getModel() const override;
	virtual int64_t calculateInstance(int64_t* piVertexBufferSize, int64_t* piIndexBufferSize) override;

	// CInstanceBase
	virtual SdaiInstance GetInstance() const { return (SdaiInstance)m_iInstance; }
	SdaiModel GetModel() const { return sdaiGetInstanceModel(GetInstance()); }
	virtual bool HasGeometry() const { return _geometry::hasGeometry(); }
	virtual bool IsEnabled() const { return getEnable(); }	
	ExpressID ExpressID() const { return m_iExpressID; }
	bool& Referenced() { return m_bReferenced; }
	
	void CalculateMinMax(
		float& fXmin, float& fXmax, 
		float& fYmin, float& fYmax, 
		float& fZmin, float& fZmax);

	void Scale(float fScaleFactor);

private: // Methods
	
	void Calculate();
};

#endif // CIS2_REPRESENTAION_H
