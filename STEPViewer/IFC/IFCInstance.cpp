#include "stdafx.h"
#include "IFCInstance.h"
#include "IFCModel.h"

#include <math.h>

// ************************************************************************************************
CIFCInstance::CIFCInstance(int64_t iID, SdaiInstance sdaiInstance)
	: _ap_geometry(sdaiInstance)
	, _ap_instance(iID, this, nullptr)
	, m_bReferenced(false)
{
	calculate();
}

CIFCInstance::~CIFCInstance()
{}

/*virtual*/ void CIFCInstance::preCalculate() /*override*/
{
	// Format
	setSTEPFormatSettings();

	// Extra settings
	const int_t flagbit1 = 2;
	setFilter(GetModel(), flagbit1, flagbit1);
	setSegmentation(GetModel(), 16, 0.);
}

/*virtual*/ OwlModel CIFCInstance::getModel() const /*override*/
{
	OwlModel iOwlModel = 0;
	owlGetModel(GetModel(), &iOwlModel);
	ASSERT(iOwlModel != 0);

	return iOwlModel;
}

