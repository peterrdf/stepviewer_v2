#include "stdafx.h"

#include "Instance.h"

// --------------------------------------------------------------------------------------------
CInstance::CInstance()
{
}

// --------------------------------------------------------------------------------------------
/*virtual*/ CInstance::~CInstance()
{
}

// --------------------------------------------------------------------------------------------
int64_t CInstance::_getEntity() const
{
	return sdaiGetInstanceType(_getInstance());
}

// --------------------------------------------------------------------------------------------
const wchar_t* CInstance::_getEntityName() const
{
	wchar_t* szEntity = nullptr;
	engiGetEntityName(_getEntity(), sdaiUNICODE, (char**)&szEntity);

	return szEntity;
}