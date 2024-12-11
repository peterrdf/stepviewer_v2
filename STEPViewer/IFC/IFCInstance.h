#ifndef IFC_INSTANCE_H
#define IFC_INSTANCE_H

#include "_oglUtils.h"
#include "_ap_mvc.h"

#include "engine.h"

#include <cstddef>
#include <string>
using namespace std;

// ************************************************************************************************
class CIFCInstance
	: public _ap_geometry
	, public _ap_instance
{
	friend class CIFCModel;

private: // Members
	
	// UI
	bool m_bReferenced;

public: // Methods
	
	CIFCInstance(int64_t iID, SdaiInstance iSdaiInstance);
	virtual ~CIFCInstance();

protected: // Methods

	// _geometry
	virtual void preCalculate() override;

public: // Methods

	// _geometry
	virtual OwlModel getModel() const override;

	// CInstanceBase
	SdaiModel GetModel() const { return sdaiGetInstanceModel(_ap_geometry::getSdaiInstance()); }
	bool& Referenced() { return m_bReferenced; }
};

#endif // IFC_INSTANCE_H
