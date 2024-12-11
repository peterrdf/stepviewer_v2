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

	// Metadata
	ExpressID m_iExpressID;
	
	// UI
	bool m_bReferenced;

public: // Methods
	
	CIFCInstance(int64_t iID, SdaiInstance iSdaiInstance);
	virtual ~CIFCInstance();

	// _geometry
	virtual OwlModel getModel() const override;
	virtual int64_t calculateInstance(int64_t* piVertexBufferSize, int64_t* piIndexBufferSize) override;

	// CInstanceBase
	SdaiModel GetModel() const { return sdaiGetInstanceModel(_ap_geometry::getSdaiInstance()); }
	ExpressID ExpressID() const { return m_iExpressID; }
	bool& Referenced() { return m_bReferenced; }
	
	void CalculateMinMax(
		float& fXmin, float& fXmax, 
		float& fYmin, float& fYmax, 
		float& fZmin, float& fZmax);

private: // Methods
	
	void Calculate();
};

#endif // IFC_INSTANCE_H
