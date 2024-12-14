#ifndef IFC_GEOMETRY_H
#define IFC_GEOMETRY_H

#include "_oglUtils.h"
#include "_ap_mvc.h"

#include "engine.h"

#include <cstddef>
#include <string>
using namespace std;

// ************************************************************************************************
class CIFCGeometry : public _ap_geometry
{

private: // Members
	
	bool m_bReferenced;

public: // Methods
	
	CIFCGeometry(SdaiInstance sdaiInstance);
	virtual ~CIFCGeometry();

protected: // Methods

	// _geometry
	virtual void preCalculate() override;
	virtual void cleanMemoryCore() override;

public: // Properties

	// _geometry
	virtual OwlInstance getOwlInstance() override;

	bool& Referenced() { return m_bReferenced; }
};

#endif // IFC_GEOMETRY_H
