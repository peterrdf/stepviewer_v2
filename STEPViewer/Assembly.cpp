#include "stdafx.h"
#include "Assembly.h"
#include "ProductDefinition.h"

//-------------------------------------------------------------------------------------------------
CAssembly::CAssembly()
	: m_iExpressID(0)
	, m_strId(L"")
	, m_strName(L"")
	, m_strDescription(L"")
	, m_pRelatingProductDefinition(nullptr)
	, m_pRelatedProductDefinition(nullptr)
{
}

//-------------------------------------------------------------------------------------------------
/*virtual*/ CAssembly::~CAssembly()
{
}

// ------------------------------------------------------------------------------------------------
ExpressID CAssembly::GetExpressID() const
{
	return m_iExpressID;
}

// ------------------------------------------------------------------------------------------------
const wchar_t* CAssembly::GetId() const
{
	return m_strId.c_str();
}

// ------------------------------------------------------------------------------------------------
const wchar_t* CAssembly::GetName() const
{
	return m_strName.c_str();
}

// ------------------------------------------------------------------------------------------------
const wchar_t* CAssembly::GetDescription() const
{
	return m_strDescription.c_str();
}

// ------------------------------------------------------------------------------------------------
CProductDefinition* CAssembly::GetRelatingProductDefinition() const
{
	return m_pRelatingProductDefinition;
}

// ------------------------------------------------------------------------------------------------
CProductDefinition* CAssembly::GetRelatedProductDefinition() const
{
	return m_pRelatedProductDefinition;
}