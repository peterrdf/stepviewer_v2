#pragma once

#include "Instance.h"

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
	CEntity * m_pParent;

	int_t m_iAttributesCount;
	vector<wstring> m_vecAttributes;
	set<wstring> m_setIgnoredAttributes;
	int_t m_iInstancesCount;
	vector<CEntity *> m_vecSubTypes;
	vector<int_t> m_vecInstances;

public: // Methods
	
	CEntity(int_t iModel, int_t iEntity, int_t iAttributesCount, int_t iInstancesCount);
	virtual ~CEntity();

	int_t getEntity() const;
	const wchar_t* getName() const;
	CEntity* getParent() const;
	void setParent(CEntity* pParent);
	bool hasParent() const;

	int_t getAttributesCount() const;
	const vector<wstring>& getAttributes() const;
	const set<wstring>& getIgnoredAttributes() const;
	void ignoreAttribute(const wstring& strAttribute, bool bIgnore);
	bool isAttributeIgnored(const wstring& strAttribute) const;
	bool isAttributeInherited(const wstring& strAttribute) const;
	int_t getInstancesCount() const;
	void addSubType(CEntity* pEntity);
	const vector<CEntity*>& getSubTypes() const;
	const vector<int_t>& getInstances() const;

	void postProcessing();
};

// ------------------------------------------------------------------------------------------------
struct SORT_ENTITIES
{
	bool operator()(const CEntity * a, const CEntity * b) const
	{
		return wcscmp(a->getName(), b->getName()) < 0;
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