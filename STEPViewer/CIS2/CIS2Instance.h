#ifndef CIS2_INSTANCE_H
#define CIS2_INSTANCE_H

#include "_ap_mvc.h"
#include "_oglUtils.h"

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
	: public _ap_geometry
	, public _ap_instance
{

private: // Members

	enumCIS2InstanceType m_enCIS2InstanceType;

	// Metadata
	ExpressID m_iExpressID;
	
	// UI
	bool m_bReferenced;

public: // Methods
	
	CCIS2Instance(int64_t iID, SdaiInstance iSdaiInstance, enumCIS2InstanceType enCIS2InstanceType);
	virtual ~CCIS2Instance();

	enumCIS2InstanceType getType() const { return m_enCIS2InstanceType; }

	// _geometry
	virtual OwlModel getOwlModel() const override;

	// CInstanceBase
	SdaiModel GetModel() const { return sdaiGetInstanceModel(_ap_geometry::getSdaiInstance()); }
	ExpressID ExpressID() const { return m_iExpressID; }
	bool& Referenced() { return m_bReferenced; }
};

#endif // CIS2_INSTANCE_H
