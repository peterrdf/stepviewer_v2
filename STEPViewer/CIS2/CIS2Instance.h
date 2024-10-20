#ifndef CIS2_INSTANCE_H
#define CIS2_INSTANCE_H

#include "_oglUtils.h"
#include "InstanceBase.h"

#include "engine.h"

#include <cstddef>
#include <string>
using namespace std;

// ************************************************************************************************
enum class enumCIS2InstanceType
{
	Instance = 0,
	DesignPart = 1,
	Reperesentation = 2,
};

// ************************************************************************************************
class CCIS2Instance
	: public _geometry
	, public CInstanceBase
{

private: // Members

	enumCIS2InstanceType m_enCIS2InstanceType;

	// Metadata
	SdaiInstance m_iSdaiInstance;
	ExpressID m_iExpressID;
	
	// UI
	bool m_bReferenced;

public: // Methods
	
	CCIS2Instance(int64_t iID, SdaiInstance iSdaiInstance, enumCIS2InstanceType enCIS2InstanceType);
	virtual ~CCIS2Instance();

	enumCIS2InstanceType getType() const { return m_enCIS2InstanceType; }

	// _geometry
	virtual OwlModel getOwlModel() const override;
//	virtual void calculateGetBufferSize(int64_t* piVertexBufferSize, int64_t* piIndexBufferSize) override;

	// CInstanceBase
	virtual SdaiInstance GetSdaiInstance() const { return m_iSdaiInstance; }
	SdaiModel GetSdaiModel() const { return sdaiGetInstanceModel(GetSdaiInstance()); }
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

#endif // CIS2_INSTANCE_H
