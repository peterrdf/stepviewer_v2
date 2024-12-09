#pragma once

#include "_ap_mvc.h"
#include "InstanceBase.h"
#include "Entity.h"

#include <map>

using namespace std;

// ************************************************************************************************
class CModel : public _ap_model
{

public: // Methods

	CModel(enumAP enAP);
	virtual ~CModel()
	{}
	
	virtual void ZoomToInstance(CInstanceBase* pInstance) PURE;
	virtual void ZoomOut() PURE;
	virtual CInstanceBase* LoadInstance(OwlInstance iInstance) PURE;
		
	
};

