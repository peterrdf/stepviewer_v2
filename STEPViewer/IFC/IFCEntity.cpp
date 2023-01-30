#include "stdafx.h"
#include "IFCEntity.h"
#include "ifcengine.h"

#include <algorithm>

// ------------------------------------------------------------------------------------------------
CIFCEntity::CIFCEntity(int_t iModel, int_t iEntity, int_t iAttributesCount, int_t iInstancesCount)
	: m_iEntity(iEntity)
	, m_strName(L"")
	, m_pParent(NULL)
	, m_iAttributesCount(iAttributesCount)
	, m_vecAttributes()
	, m_setIgnoredAttributes()
	, m_iInstancesCount(iInstancesCount)
	, m_vecSubTypes()
	, m_vecInstances()
{
	// Name ***************************************************************************************
	wchar_t	* szName = NULL;
	engiGetEntityName(m_iEntity, sdaiUNICODE, (char **)&szName);
	// ********************************************************************************************

	m_strName = szName;

	// Attributes *********************************************************************************
	for (int_t iIndex = 0; iIndex < m_iAttributesCount; iIndex++)
	{
		wchar_t	* szArgumentName = 0;
		engiGetEntityArgumentName(m_iEntity, iIndex, sdaiUNICODE, (char **)&szArgumentName);

		m_vecAttributes.push_back(szArgumentName);
	}
	// ********************************************************************************************

	// Instances **********************************************************************************
	int_t * ifcObjects = sdaiGetEntityExtent(iModel, m_iEntity);
	int_t ifcObjectsCount = sdaiGetMemberCount(ifcObjects);

	int_t iIndex = 0;
	while (iIndex < ifcObjectsCount) {
		int_t ifcObject = 0;
		engiGetAggrElement(ifcObjects, iIndex++, sdaiINSTANCE, &ifcObject);

		m_vecInstances.push_back(ifcObject);
	}
	// ********************************************************************************************
}

// ------------------------------------------------------------------------------------------------
CIFCEntity::~CIFCEntity()
{
}

// ------------------------------------------------------------------------------------------------
int_t CIFCEntity::getEntity() const
{
	return m_iEntity;
}

// ------------------------------------------------------------------------------------------------
const wchar_t* CIFCEntity::getName() const
{
	return m_strName.c_str();
}

// ------------------------------------------------------------------------------------------------
CIFCEntity * CIFCEntity::getParent() const
{
	return m_pParent;
}

// ------------------------------------------------------------------------------------------------
void CIFCEntity::setParent(CIFCEntity * pParent)
{
	m_pParent = pParent;
}

// ------------------------------------------------------------------------------------------------
bool CIFCEntity::hasParent() const
{
	return m_pParent != NULL;
}

// ------------------------------------------------------------------------------------------------
int_t CIFCEntity::getAttributesCount() const
{
	return m_iAttributesCount;
}

// ------------------------------------------------------------------------------------------------
const vector<wstring> & CIFCEntity::getAttributes() const
{
	return m_vecAttributes;
}

// ------------------------------------------------------------------------------------------------
const set<wstring>& CIFCEntity::getIgnoredAttributes() const
{
	return m_setIgnoredAttributes;
}

// ------------------------------------------------------------------------------------------------
void CIFCEntity::ignoreAttribute(const wstring& strAttribute, bool bIgnore)
{
	set<wstring>::iterator itAttribute = m_setIgnoredAttributes.find(strAttribute);

	if (bIgnore)
	{
		ASSERT(itAttribute == m_setIgnoredAttributes.end());

		m_setIgnoredAttributes.insert(strAttribute);
	}
	else
	{
		ASSERT(itAttribute != m_setIgnoredAttributes.end());

		m_setIgnoredAttributes.erase(itAttribute);
	}
}

// ------------------------------------------------------------------------------------------------
bool CIFCEntity::isAttributeIgnored(const wstring& strAttribute) const
{
	return m_setIgnoredAttributes.find(strAttribute) != m_setIgnoredAttributes.end();
}

// ------------------------------------------------------------------------------------------------
bool CIFCEntity::isAttributeInherited(const wstring & strAttribute) const
{
	if (m_pParent != NULL)
	{
		const vector<wstring> & vecParentAttributes = m_pParent->getAttributes();

		return find(vecParentAttributes.begin(), vecParentAttributes.end(), strAttribute) != vecParentAttributes.end();
	}

	return false;
}

// ------------------------------------------------------------------------------------------------
int_t CIFCEntity::getInstancesCount() const
{
	return m_iInstancesCount;
}

// ------------------------------------------------------------------------------------------------
void CIFCEntity::addSubType(CIFCEntity * pEntity)
{
	m_vecSubTypes.push_back(pEntity);
}

// ------------------------------------------------------------------------------------------------
const vector<CIFCEntity *> & CIFCEntity::getSubTypes() const
{
	return m_vecSubTypes;
}

// ------------------------------------------------------------------------------------------------
const vector<int_t> & CIFCEntity::getInstances() const
{
	return m_vecInstances;
}

// ------------------------------------------------------------------------------------------------
void CIFCEntity::postProcessing()
{
	if (!m_vecSubTypes.empty())
	{
		std::sort(m_vecSubTypes.begin(), m_vecSubTypes.end(), SORT_IFC_ENTITIES());
	}
}