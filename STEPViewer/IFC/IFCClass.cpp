#include "stdafx.h"
#include "IFCClass.h"
#include "engine.h"
#include <assert.h>

// ------------------------------------------------------------------------------------------------
CIFCClass::CIFCClass(int64_t iInstance)
	: m_iInstance(iInstance)
	, m_strName(L"")
	, m_vecParentClasses()
	, m_vecAncestorClasses()
	, m_vecPropertyRestrictions()
{
	assert(m_iInstance != 0);

	/*
	* Name
	*/
	char * szClassName = NULL;
	GetNameOfClass(m_iInstance, &szClassName);

	m_strName = CA2W(szClassName);

	/*
	* Parents
	*/
	int64_t iParentClassInstance = GetParentsByIterator(m_iInstance, 0);
	while (iParentClassInstance != 0)
	{
		m_vecParentClasses.push_back(iParentClassInstance);

		m_vecAncestorClasses.push_back(iParentClassInstance);
		GetAncestors(iParentClassInstance, m_vecAncestorClasses);

		char * szParentClassName = NULL;
		GetNameOfClass(iParentClassInstance, &szParentClassName);

		iParentClassInstance = GetParentsByIterator(m_iInstance, iParentClassInstance);
	}
}

// ------------------------------------------------------------------------------------------------
CIFCClass::~CIFCClass()
{
	for (size_t iProperty = 0; iProperty < m_vecPropertyRestrictions.size(); iProperty++)
	{
		delete m_vecPropertyRestrictions[iProperty];
	}
}

// ------------------------------------------------------------------------------------------------
// Getter
int64_t CIFCClass::getInstance() const
{
	return m_iInstance;
}

// ------------------------------------------------------------------------------------------------
// Getter
const wchar_t * CIFCClass::getName() const
{
	return m_strName.c_str();
}

// ------------------------------------------------------------------------------------------------
const vector<int64_t> & CIFCClass::getParentClasses()
{
	return m_vecParentClasses;
}

// ------------------------------------------------------------------------------------------------
const vector<int64_t> & CIFCClass::getAncestorClasses()
{
	return m_vecAncestorClasses;
}

// ------------------------------------------------------------------------------------------------
void CIFCClass::AddPropertyRestriction(CIFCPropertyRestriction * pPropertyRestriction)
{
	assert(pPropertyRestriction != NULL);

	char * szPropertyName = NULL;
	GetNameOfProperty(pPropertyRestriction->getPropertyInstance(), &szPropertyName);

	m_vecPropertyRestrictions.push_back(pPropertyRestriction);
}

// ------------------------------------------------------------------------------------------------
const vector<CIFCPropertyRestriction *> & CIFCClass::getPropertyRestrictions()
{
	return m_vecPropertyRestrictions;
}

// ------------------------------------------------------------------------------------------------
void CIFCClass::GetAncestors(int64_t iClassInstance, vector<int64_t> & vecAncestorClasses)
{
	int64_t iParentClassInstance = GetParentsByIterator(iClassInstance, 0);
	while (iParentClassInstance != 0)
	{
		vecAncestorClasses.push_back(iParentClassInstance);

		GetAncestors(iParentClassInstance, vecAncestorClasses);

		iParentClassInstance = GetParentsByIterator(iClassInstance, iParentClassInstance);
	}
}
