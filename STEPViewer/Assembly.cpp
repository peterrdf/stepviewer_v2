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
int_t CAssembly::getExpressID() const
{
	return m_iExpressID;
}

// ------------------------------------------------------------------------------------------------
const wchar_t* CAssembly::getId() const
{
	return m_strId.c_str();
}

// ------------------------------------------------------------------------------------------------
const wchar_t* CAssembly::getName() const
{
	return m_strName.c_str();
}

// ------------------------------------------------------------------------------------------------
const wchar_t* CAssembly::getDescription() const
{
	return m_strDescription.c_str();
}

// ------------------------------------------------------------------------------------------------
CProductDefinition* CAssembly::getRelatingProductDefinition() const
{
	return m_pRelatingProductDefinition;
}

// ------------------------------------------------------------------------------------------------
CProductDefinition* CAssembly::getRelatedProductDefinition() const
{
	return m_pRelatedProductDefinition;
}