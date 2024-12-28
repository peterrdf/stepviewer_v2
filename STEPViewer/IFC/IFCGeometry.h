#ifndef IFC_GEOMETRY_H
#define IFC_GEOMETRY_H

#include "_ifc_geometry.h"

// ************************************************************************************************
class CIFCGeometry : public _ifc_geometry
{

public: // Methods
	
	CIFCGeometry(OwlInstance owlInstance, SdaiInstance sdaiInstance);
	virtual ~CIFCGeometry();
};

#endif // IFC_GEOMETRY_H
