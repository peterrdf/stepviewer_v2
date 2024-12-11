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
	
	CIFCGeometry(int64_t iID, SdaiInstance iSdaiInstance);
	virtual ~CIFCGeometry();

public: // Properties

	bool& Referenced() { return m_bReferenced; }

private: // Methods
	
	void Calculate();
};

#endif // IFC_GEOMETRY_H
