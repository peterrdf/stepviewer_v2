#include "stdafx.h"
#include "AP242ProductInstance.h"
#include "AP242ProductDefinition.h"
#include "AP242Assembly.h"

// ************************************************************************************************
CAP242ProductInstance::CAP242ProductInstance(int64_t iID, _geometry* pGeometry, CAP242Assembly* pAssembly, _matrix4x3* pTransformationMatrix)
	: _ap_instance(iID, pGeometry, pTransformationMatrix)
	, m_pAssembly(pAssembly)
{
}

CAP242ProductInstance::~CAP242ProductInstance()
{
}

CAP242ProductDefinition* CAP242ProductInstance::GetProductDefinition() const 
{ 
	return getGeometryAs<CAP242ProductDefinition>();
}

CAP242Assembly* CAP242ProductInstance::GetAssembly() const
{
	return m_pAssembly;
}