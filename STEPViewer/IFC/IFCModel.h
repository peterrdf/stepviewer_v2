#ifndef IFCFILEPARSER_H
#define IFCFILEPARSER_H

#include "_ifc_mvc.h"

#include "IFCGeometry.h"
#include "IFCInstance.h"

// ************************************************************************************************
class CIFCModel : public _ifc_model
{

public: // Methods
	
	CIFCModel(bool bLoadInstancesOnDemand = false);
	virtual ~CIFCModel();

protected: // Methods

	virtual _ifc_geometry* createGeometry(OwlInstance owlInstance, SdaiInstance sdaiInstance) override;
	virtual _ap_instance* createInstance(int64_t iID, _geometry* pGeometry, _matrix4x3* pTransformationMatrix) override;

};

#endif // IFCFILEPARSER_H
