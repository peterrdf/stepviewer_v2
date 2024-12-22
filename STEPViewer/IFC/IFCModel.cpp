#include "stdafx.h"
#include "IFCModel.h"

// ************************************************************************************************
CIFCModel::CIFCModel(bool bLoadInstancesOnDemand/* = false*/)
	: _ifc_model()	
{}

CIFCModel::~CIFCModel()
{	
}

/*virtual*/ _ifc_geometry* CIFCModel::createGeometry(OwlInstance owlInstance, SdaiInstance sdaiInstance) /*override*/
{
	return new CIFCGeometry(owlInstance, sdaiInstance);
}

/*virtual*/ _ap_instance* CIFCModel::createInstance(int64_t iID, _geometry* pGeometry, _matrix4x3* pTransformationMatrix) /*override*/
{
	return new CIFCInstance(iID, pGeometry, pTransformationMatrix);
}


