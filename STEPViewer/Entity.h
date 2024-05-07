#pragma once

#include "InstanceBase.h"

#include "engdef.h"

#include <vector>
#include <string>
#include <set>
#include <map>
using namespace std;

// ************************************************************************************************
class CEntity
{

private: // Members
	
	SdaiEntity m_iEntity;
	wchar_t* m_szName;
	CEntity* m_pParent;

	vector<wstring> m_vecAttributes;
	vector<CEntity*> m_vecSubTypes;
	vector<SdaiInstance> m_vecInstances;

public: // Methods
	
	CEntity(SdaiEntity iEntity);
	virtual ~CEntity();

	SdaiEntity GetEntity() const { return m_iEntity; }
	const wchar_t* GetName() const { return m_szName; }
	static wchar_t* GetName(SdaiEntity iEntity);
	CEntity* GetParent() const { return m_pParent; }
	void SetParent(CEntity* pParent) { m_pParent = pParent; }
	bool HasParent() const { return m_pParent != nullptr; }
	const vector<wstring>& GetAttributes() const { return m_vecAttributes; }
	bool IsAttributeInherited(const wstring& strAttribute) const;
	void AddSubType(CEntity* pEntity) { m_vecSubTypes.push_back(pEntity); }
	const vector<CEntity*>& GetSubTypes() const { return m_vecSubTypes; }
	const vector<SdaiInstance>& GetInstances() const { return m_vecInstances; }

	void PostProcessing();
};

// ************************************************************************************************
struct SORT_ENTITIES
{
	bool operator()(const CEntity* pLeft, const CEntity* pRight) const
	{
		return wcscmp(pLeft->GetName(), pRight->GetName()) < 0;
	}
};

// ************************************************************************************************
class CEntityProvider
{

private: // Members

	SdaiModel m_iModel;
	map<SdaiEntity, CEntity*> m_mapEntities;

public: // Methods

	CEntityProvider(SdaiModel iModel);
	virtual ~CEntityProvider();

	const map<SdaiEntity, CEntity*>& GetEntities() const { return m_mapEntities; }

protected: // Methods

	void Load();
};