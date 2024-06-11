#include "stdafx.h"
#include "Entity.h"

#include <algorithm>

// ************************************************************************************************
CEntity::CEntity(SdaiEntity iEntity)
	: m_iEntity(iEntity)
	, m_szName(nullptr)
	, m_pParent(nullptr)
	, m_vecAttributes()
	, m_vecSubTypes()
	, m_vecInstances()
{
	ASSERT(m_iEntity != 0);

	SdaiModel iModel = engiGetEntityModel(iEntity);
	ASSERT(iModel != 0);

	m_szName = CEntity::GetName(m_iEntity);	

	// Attributes
	SdaiInteger iAttributesCount = engiGetEntityNoArguments(iEntity);
	for (SdaiInteger iIndex = 0; iIndex < iAttributesCount; iIndex++)
	{
		wchar_t* szArgumentName = 0;
		engiGetEntityArgumentName(m_iEntity, iIndex, sdaiUNICODE, (const char**)&szArgumentName);

		m_vecAttributes.push_back(szArgumentName);
	}

	// Instances
	SdaiAggr pAggr = sdaiGetEntityExtent(iModel, m_iEntity);
	SdaiInteger iInstancesCount = sdaiGetMemberCount(pAggr);

	SdaiInteger iIndex = 0;
	while (iIndex < iInstancesCount)
	{
		SdaiInstance iInstance = 0;
		sdaiGetAggrByIndex(pAggr, iIndex++, sdaiINSTANCE, &iInstance);

		m_vecInstances.push_back(iInstance);
	}
}

CEntity::~CEntity()
{}

/*static*/ wchar_t* CEntity::GetName(SdaiEntity iEntity)
{
	wchar_t* szName = nullptr;
	engiGetEntityName(iEntity, sdaiUNICODE, (const char**)&szName);

	return szName;
}

bool CEntity::IsAttributeInherited(const wstring& strAttribute) const
{
	if (m_pParent != nullptr)
	{
		const vector<wstring>& vecParentAttributes = m_pParent->GetAttributes();

		return find(vecParentAttributes.begin(), vecParentAttributes.end(), strAttribute) != vecParentAttributes.end();
	}

	return false;
}

void CEntity::PostProcessing()
{
	if (!m_vecSubTypes.empty())
	{
		std::sort(m_vecSubTypes.begin(), m_vecSubTypes.end(), SORT_ENTITIES());
	}
}

// ************************************************************************************************
CEntityProvider::CEntityProvider(SdaiModel iModel)
	: m_iModel(iModel)
	, m_mapEntities()
{
	ASSERT(m_iModel != 0);

	Load();
}

/*virtual*/ CEntityProvider::~CEntityProvider()
{
	for (auto itEntity : m_mapEntities)
	{
		delete itEntity.second;
	}
}

void CEntityProvider::Load()
{
	SdaiInteger iEntitiesCount = engiGetEntityCount(m_iModel);

	// Retrieve the Entities
	SdaiInteger iIndex = 0;
	while (iIndex < iEntitiesCount)
	{
		SdaiEntity iEntity = engiGetEntityElement(m_iModel, iIndex);

		auto pEntity = new CEntity(iEntity);

		ASSERT(m_mapEntities.find(iEntity) == m_mapEntities.end());
		m_mapEntities[iEntity] = pEntity;

		iIndex++;
	}

	// Connect the Entities
	for (auto itEntity : m_mapEntities)
	{
		SdaiEntity iParentEntity = engiGetEntityParent(itEntity.first);
		if (iParentEntity == 0)
		{
			continue;
		}

		auto itParentEntity = m_mapEntities.find(iParentEntity);
		ASSERT(itParentEntity != m_mapEntities.end());

		itEntity.second->SetParent(itParentEntity->second);

		itParentEntity->second->AddSubType(itEntity.second);
	} // for (; itEntities != ...	

	// Post-processing
	for (auto itEntity : m_mapEntities)
	{
		itEntity.second->PostProcessing();
	} // for (; itEntities != ...	
}
