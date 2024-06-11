#ifndef IFCINSTANCE_H
#define IFCINSTANCE_H

#include "_oglUtils.h"
#include "InstanceBase.h"

#include "engine.h"

#include <cstddef>
#include <string>
using namespace std;

// ************************************************************************************************
class CIFCInstance
	: public _geometry
	, public CInstanceBase
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

	// CInstanceBase
	virtual SdaiInstance GetInstance() const { return (SdaiInstance)m_iInstance; }
	SdaiModel GetModel() const { return sdaiGetInstanceModel(GetInstance()); }
	virtual bool HasGeometry() const { return _geometry::hasGeometry(); }
	virtual bool IsEnabled() const { return getEnable(); }	
	ExpressID ExpressID() const { return m_iExpressID; }
	bool& Referenced() { return m_bReferenced; }
	
	void CalculateMinMax(
		float& fXmin, float& fXmax, 
		float& fYmin, float& fYmax, 
		float& fZmin, float& fZmax);

	void Scale(float fScaleFactor);

private: // Methods
	
	void Calculate();
};

#endif // IFCINSTANCE_H
