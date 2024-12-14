#include "_owl_geometry.h"

// ************************************************************************************************
class _owl_instance : public _instance
{

public:  // Methods

	_owl_instance(int64_t iID, _geometry* pGeometry, _matrix4x3* pTransformationMatrix)
		: _instance(iID, pGeometry, pTransformationMatrix)
	{
	}

	virtual ~_owl_instance()
	{
	}

public: // Properties

	OwlInstance getOwlInstance() const { return getGeometryAs<_owl_geometry>()->getInstance(); }
};