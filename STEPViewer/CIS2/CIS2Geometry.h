#ifndef CIS2_GEOMETRY_H
#define CIS2_GEOMETRY_H

#include "_ap_mvc.h"
#include "_oglUtils.h"

#include "engine.h"

#include <cstddef>
#include <string>
using namespace std;

// ************************************************************************************************
enum class enumCIS2GeometryType
{
	Unknown = 0,
	DesignPart = 1,
	Reperesentation = 2,
};

// ************************************************************************************************
class CCIS2Geometry : public _ap_geometry
{

private: // Members

	enumCIS2GeometryType m_enCIS2GeometryType;

public: // Methods
	
	CCIS2Geometry(OwlInstance owlInstance, SdaiInstance sdaiInstance, enumCIS2GeometryType enCIS2GeometryType);
	virtual ~CCIS2Geometry();

public: // Properties

	enumCIS2GeometryType getType() const { return m_enCIS2GeometryType; }
};

#endif // CIS2_GEOMETRY_H
