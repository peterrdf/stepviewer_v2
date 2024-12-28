#include "stdafx.h"

#include "_ap242_geometry.h"

// ************************************************************************************************
_ap242_geometry::_ap242_geometry(OwlInstance owlInstance, SdaiInstance sdaiInstance)
	: _ap_geometry(owlInstance, sdaiInstance)
{
	calculate();
}

/*virtual*/ _ap242_geometry::~_ap242_geometry()
{
}