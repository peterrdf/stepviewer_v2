#pragma once

#include "_ap_instance.h"

// ************************************************************************************************
class CAP242ProductDefinition;
class CAP242Assembly;

// ************************************************************************************************
class CAP242ProductInstance : public _ap_instance
{

private: // Members

	CAP242Assembly* m_pAssembly;

public: // Methods
	
	CAP242ProductInstance(int64_t iID, _geometry* pGeometry, CAP242Assembly* pAssembly, _matrix4x3* pTransformationMatrix);
	virtual ~CAP242ProductInstance();

public: // Properties

	CAP242ProductDefinition* GetProductDefinition() const;
	CAP242Assembly* GetAssembly() const;
};

