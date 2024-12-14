#include "_geometry.h"

// ************************************************************************************************
class _owl_geometry : public _geometry
{

public: // Methods

	_owl_geometry(OwlInstance owlInstance)
		: _geometry(owlInstance)
	{
		assert(m_iInstance != 0);
	}

	virtual ~_owl_geometry()
	{
	}

	// _geometry
	virtual OwlModel getOwlModel() const override
	{
		return GetModel(m_iInstance);
	}
};