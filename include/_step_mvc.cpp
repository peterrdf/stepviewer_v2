#include "stdafx.h"
#include "_step_mvc.h"

// ************************************************************************************************
_step_model::_step_model()
	: _ap_model(enumAP::STEP)
{
}

/*virtual*/ _step_model::~_step_model()
{
}

/*virtual*/ void _step_model::preLoadInstance(OwlInstance owlInstance) /*override*/
{
	//#todo
	//setVertexBufferOffset(owlInstance);
}

// ************************************************************************************************
_step_geometry::_step_geometry(OwlInstance owlInstance, SdaiInstance sdaiInstance)
	: _ap_geometry(owlInstance, sdaiInstance)
{
	calculate();
}

/*virtual*/ _step_geometry::~_step_geometry()
{
}