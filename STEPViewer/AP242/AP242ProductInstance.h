#pragma once

#include "_ap_instance.h"

// ************************************************************************************************
class CAP242ProductDefinition;

// ************************************************************************************************
class CAP242ProductInstance : public _ap_instance
{

public: // Methods
	
	CAP242ProductInstance(int64_t iID, _geometry* pGeometry, _matrix4x3* pTransformationMatrix);
	virtual ~CAP242ProductInstance();

public: // Properties

	CAP242ProductDefinition* GetProductDefinition() const;
};

