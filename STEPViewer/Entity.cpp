#include "stdafx.h"
#include "Entity.h"
#include "stepengine.h"

#include <algorithm>

// ------------------------------------------------------------------------------------------------
CEntity::CEntity(int_t iModel, int_t iEntity, int_t iAttributesCount, int_t iInstancesCount)
	: m_iEntity(iEntity)
	, m_strName(L"")
	, m_pParent(nullptr)
	, m_iAttributesCount(iAttributesCount)
	, m_vecAttributes()
	, m_iInstancesCount(iInstancesCount)
	, m_vecSubTypes()
	, m_vecInstances()
{
	// Name ***************************************************************************************
	wchar_t* szName = CEntity::GetName(m_iEntity);
	// ********************************************************************************************

	m_strName = szName;

	// Attributes *********************************************************************************
	for (int_t iIndex = 0; iIndex < m_iAttributesCount; iIndex++)
	{
		wchar_t	* szArgumentName = 0;
		engiGetEntityArgumentName(m_iEntity, iIndex, sdaiUNICODE, (const char**)&szArgumentName);

		m_vecAttributes.push_back(szArgumentName);
	}
	// ********************************************************************************************

	// Instances **********************************************************************************
	int_t* piInstances = sdaiGetEntityExtent(iModel, m_iEntity);
	iInstancesCount = sdaiGetMemberCount(piInstances);

	int_t iIndex = 0;
	while (iIndex < iInstancesCount) {
		int_t iInstance = 0;
		sdaiGetAggrByIndex(piInstances, iIndex++, sdaiINSTANCE, &iInstance);

		m_vecInstances.push_back(iInstance);
	}
	// ********************************************************************************************
}

// ------------------------------------------------------------------------------------------------
CEntity::~CEntity()
{
}

// ------------------------------------------------------------------------------------------------
int_t CEntity::GetEntity() const
{
	return m_iEntity;
}

// ------------------------------------------------------------------------------------------------
const wchar_t* CEntity::GetName() const
{
	return m_strName.c_str();
}

// ------------------------------------------------------------------------------------------------
/*static*/ wchar_t* CEntity::GetName(int_t iEntity)
{
	wchar_t* szName = nullptr;
	engiGetEntityName(iEntity, sdaiUNICODE, (const char**)&szName);

	return szName;
}

// ------------------------------------------------------------------------------------------------
CEntity* CEntity::GetParent() const
{
	return m_pParent;
}

// ------------------------------------------------------------------------------------------------
void CEntity::SetParent(CEntity * pParent)
{
	m_pParent = pParent;
}

// ------------------------------------------------------------------------------------------------
bool CEntity::HasParent() const
{
	return m_pParent != nullptr;
}

// ------------------------------------------------------------------------------------------------
int_t CEntity::GetAttributesCount() const
{
	return m_iAttributesCount;
}

// ------------------------------------------------------------------------------------------------
const vector<wstring>& CEntity::GetAttributes() const
{
	return m_vecAttributes;
}

// ------------------------------------------------------------------------------------------------
bool CEntity::IsAttributeInherited(const wstring& strAttribute) const
{
	if (m_pParent != nullptr)
	{
		const vector<wstring> & vecParentAttributes = m_pParent->GetAttributes();

		return find(vecParentAttributes.begin(), vecParentAttributes.end(), strAttribute) != vecParentAttributes.end();
	}

	return false;
}

// ------------------------------------------------------------------------------------------------
int_t CEntity::GetInstancesCount() const
{
	return m_iInstancesCount;
}

// ------------------------------------------------------------------------------------------------
void CEntity::AddSubType(CEntity* pEntity)
{
	m_vecSubTypes.push_back(pEntity);
}

// ------------------------------------------------------------------------------------------------
const vector<CEntity*>& CEntity::GetSubTypes() const
{
	return m_vecSubTypes;
}

// ------------------------------------------------------------------------------------------------
const vector<int_t>& CEntity::GetInstances() const
{
	return m_vecInstances;
}

// ------------------------------------------------------------------------------------------------
void CEntity::PostProcessing()
{
	if (!m_vecSubTypes.empty())
	{
		std::sort(m_vecSubTypes.begin(), m_vecSubTypes.end(), SORT_ENTITIES());
	}
}


// ------------------------------------------------------------------------------------------------
CEntityProvider::CEntityProvider(SdaiModel iModel)
	: m_iModel(iModel)
	, m_mapEntities()
{
	ASSERT(m_iModel != 0);

	Load();
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ CEntityProvider::~CEntityProvider()
{
	for (auto itEntity : m_mapEntities)
	{
		delete itEntity.second;
	}
}

// ------------------------------------------------------------------------------------------------
void CEntityProvider::Load()
{
	SdaiInteger iEntitiesCount = engiGetEntityCount(m_iModel);

	/*
	* Retrieve the Entities
	*/
	SdaiInteger i = 0;
	while (i < iEntitiesCount)
	{
		SdaiEntity iEntity = engiGetEntityElement(m_iModel, i);
		int_t iAttributesCount = engiGetEntityNoArguments(iEntity);
		SdaiInteger iInstancesCount = sdaiGetMemberCount(sdaiGetEntityExtent(m_iModel, iEntity));

		auto pEntity = new CEntity(m_iModel, iEntity, iAttributesCount, iInstancesCount);
		ASSERT(m_mapEntities.find(iEntity) == m_mapEntities.end());
		m_mapEntities[iEntity] = pEntity;

		i++;
	}

	/*
	* Connect the Entities
	*/	
	for (auto itEntity : m_mapEntities)
	{
		int_t iParentEntity = engiGetEntityParent(itEntity.first);
		if (iParentEntity == 0)
		{
			continue;
		}

		auto itParentEntity = m_mapEntities.find(iParentEntity);
		ASSERT(itParentEntity != m_mapEntities.end());

		itEntity.second->SetParent(itParentEntity->second);

		itParentEntity->second->AddSubType(itEntity.second);
	} // for (; itEntities != ...	

	/*
	* Post-processing
	*/
	for (auto itEntity : m_mapEntities)
	{
		itEntity.second->PostProcessing();
	} // for (; itEntities != ...	
}

// ------------------------------------------------------------------------------------------------
const map<int_t, CEntity*>& CEntityProvider::GetEntities() const
{
	return m_mapEntities;
}