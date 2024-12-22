#include "stdafx.h"
#include "IFCGeometry.h"

// ************************************************************************************************
CIFCGeometry::CIFCGeometry(OwlInstance owlInstance, SdaiInstance sdaiInstance)
	: _ifc_geometry(owlInstance, sdaiInstance)
	
{
}

/*virtual*/ CIFCGeometry::~CIFCGeometry()
{}

