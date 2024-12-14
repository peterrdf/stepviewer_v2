#include "stdafx.h"
#include "CIS2Instance.h"

// ************************************************************************************************
CCIS2Instance::CCIS2Instance(int64_t iID, SdaiInstance iSdaiInstance, enumCIS2InstanceType enCIS2InstanceType)
	: _ap_geometry(iSdaiInstance)
	, _ap_instance(iID, this, nullptr)
	, m_enCIS2InstanceType(enCIS2InstanceType)
	, m_iExpressID(internalGetP21Line(iSdaiInstance))
	, m_bReferenced(false)
{
	ASSERT(m_iExpressID != 0);
}

/*virtual*/ CCIS2Instance::~CCIS2Instance()
{}

/*virtual*/ OwlModel CCIS2Instance::getOwlModel() /*override*/
{
	OwlModel iOwlModel = 0;
	owlGetModel(GetModel(), &iOwlModel);
	ASSERT(iOwlModel != 0);

	return iOwlModel;
}

