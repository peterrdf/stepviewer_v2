#pragma once

#include "_ap_model.h"
#include "CIS2DesignPart.h"
#include "CIS2Representation.h"

#include <string>
#include <map>
#include <set>
using namespace std;

// ************************************************************************************************
class CCIS2Model : public _ap_model
{

private: // Members

	// Load
	bool m_bLoadInstancesOnDemand;

public: // Methods

	CCIS2Model(bool bLoadInstancesOnDemand = false);
	virtual ~CCIS2Model();

protected: // Methods

	// _model
	virtual _instance* loadInstance(int64_t iInstance) override;

	// _ap_model
	virtual void attachModelCore() override;

private: // Methods

	void LodDesignParts();
	void LoadRepresentations();
	_geometry* LoadGeometry(SdaiInstance sdaiInstance, enumCIS2GeometryType enCIS2GeometryType, int_t iCircleSegments);
};

