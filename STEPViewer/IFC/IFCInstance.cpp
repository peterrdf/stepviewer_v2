#include "stdafx.h"
#include "IFCInstance.h"
#include "IFCModel.h"

#include <math.h>

// ************************************************************************************************
CIFCInstance::CIFCInstance(int64_t iID, SdaiInstance iSdaiInstance)
	: _ap_geometry(iSdaiInstance)
	, _ap_instance(iID, this, nullptr)
	, m_iExpressID(internalGetP21Line(iSdaiInstance))
	, m_bReferenced(false)
{
	ASSERT(iSdaiInstance != 0);
	ASSERT(m_iExpressID != 0);

	calculate();
}

CIFCInstance::~CIFCInstance()
{}

/*virtual*/ OwlModel CIFCInstance::getModel() const /*override*/
{
	OwlModel iOwlModel = 0;
	owlGetModel(GetModel(), &iOwlModel);
	ASSERT(iOwlModel != 0);

	return iOwlModel;
}

