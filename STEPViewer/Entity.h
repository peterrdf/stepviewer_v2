#pragma once

#include "InstanceBase.h"

#include "engdef.h"

#include <vector>
#include <string>
#include <set>
#include <map>

using namespace std;

// ------------------------------------------------------------------------------------------------
class CEntity
{

private: // Members
	
	int_t m_iEntity;
	wstring m_strName;
	CEntity* m_pParent;

	int_t m_iAttributesCount;
	vector<wstring> m_vecAttributes;
	int_t m_iInstancesCount;
	vector<CEntity*> m_vecSubTypes;
	vector<int_t> m_vecInstances;

public: // Methods
	
	CEntity(int_t iModel, int_t iEntity, int_t iAttributesCount, int_t iInstancesCount);
	virtual ~CEntity();

	int_t GetEntity() const;
	const wchar_t* GetName() const;
	static wchar_t* GetName(int_t iEntity);
	CEntity* GetParent() const;
	void SetParent(CEntity* pParent);
	bool HasParent() const;

	int_t GetAttributesCount() const;
	const vector<wstring>& GetAttributes() const;
	bool IsAttributeInherited(const wstring& strAttribute) const;
	int_t GetInstancesCount() const;
	void AddSubType(CEntity* pEntity);
	const vector<CEntity*>& GetSubTypes() const;
	const vector<int_t>& GetInstances() const;

	void PostProcessing();
};

// ------------------------------------------------------------------------------------------------
struct SORT_ENTITIES
{
	bool operator()(const CEntity * a, const CEntity * b) const
	{
		return wcscmp(a->GetName(), b->GetName()) < 0;
	}
};

// ------------------------------------------------------------------------------------------------
class CEntityProvider
{

private: // Members

	int64_t m_iModel;
	map<int_t, CEntity*> m_mapEntities;

public: // Methods

	CEntityProvider(int64_t iModel);
	virtual ~CEntityProvider();

	const map<int_t, CEntity*>& GetEntities() const;

protected: // Methods

	void Load();
};