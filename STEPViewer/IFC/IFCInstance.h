#ifndef IFCINSTANCE_H
#define IFCINSTANCE_H

#include "_oglUtils.h"
#include "InstanceBase.h"
#include "Generic.h"

#include "engine.h"
#include "stepengine.h"

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
	virtual bool HasGeometry() const { return _geometry::hasGeometry(); }
	virtual bool IsEnabled() const { return getEnable(); }

	void Calculate();

	ExpressID ExpressID() const { return m_iExpressID; }

	// State
	bool& Referenced() { return m_bReferenced; }
	
	void CalculateMinMax(
		float& fXmin, float& fXmax, 
		float& fYmin, float& fYmax, 
		float& fZmin, float& fZmax);

	void Scale(float fScaleFactor);
};

#endif // IFCINSTANCE_H
