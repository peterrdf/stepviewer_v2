#include "stdafx.h"
#include "AP242DraughtingModel.h"

// ************************************************************************************************
CAP242DraughtingModel::CAP242DraughtingModel(SdaiInstance sdaiInstance)
	: m_sdaiInstance(sdaiInstance)
{
	ASSERT(m_sdaiInstance != 0);
}

CAP242DraughtingModel::~CAP242DraughtingModel()
{
}

