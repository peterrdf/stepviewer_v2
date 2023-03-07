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
int64_t CInstance::GetEntity() const
{
	return sdaiGetInstanceType(GetInstance());
}

// --------------------------------------------------------------------------------------------
/*static*/ int64_t  CInstance::GetEntity(int64_t iInstance)
{
	return sdaiGetInstanceType(iInstance);
}

// --------------------------------------------------------------------------------------------
const wchar_t* CInstance::GetEntityName() const
{
	wchar_t* szEntity = nullptr;
	engiGetEntityName(GetEntity(), sdaiUNICODE, (char**)&szEntity);

	return szEntity;
}

// --------------------------------------------------------------------------------------------
/*static*/ const wchar_t* CInstance::GetEntityName(int64_t iInstance)
{
	wchar_t* szEntity = nullptr;
	engiGetEntityName(GetEntity(iInstance), sdaiUNICODE, (char**)&szEntity);

	return szEntity;
}