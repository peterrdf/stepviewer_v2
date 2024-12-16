#include "_geometry.h"

// ************************************************************************************************
class _owl_geometry : public _geometry
{

public: // Methods

	_owl_geometry(OwlInstance owlInstance)
		: _geometry(owlInstance)
	{
	}

	virtual ~_owl_geometry()
	{
	}
};