#pragma once

#include "STEPInstance.h"

#include "engdef.h"

#include <vector>
#include <string>
#include <set>

using namespace std;

// ------------------------------------------------------------------------------------------------
class CIFCEntity
	: public CSTEPEntity
{

private: // Members
	
	int_t m_iEntity;
	wstring m_strName;
	CIFCEntity * m_pParent;

	int_t m_iAttributesCount;
	vector<wstring> m_vecAttributes;
	set<wstring> m_setIgnoredAttributes;
	int_t m_iInstancesCount;
	vector<CIFCEntity *> m_vecSubTypes;
	vector<int_t> m_vecInstances;

public: // Methods
	
	CIFCEntity(int_t iModel, int_t iEntity, int_t iAttributesCount, int_t iInstancesCount);
	virtual ~CIFCEntity();

	int_t getEntity() const;
	const wchar_t* getName() const;
	CIFCEntity * getParent() const;
	void setParent(CIFCEntity * pParent);
	bool hasParent() const;

	int_t getAttributesCount() const;
	const vector<wstring> & getAttributes() const;
	const set<wstring>& getIgnoredAttributes() const;
	void ignoreAttribute(const wstring& strAttribute, bool bIgnore);
	bool isAttributeIgnored(const wstring& strAttribute) const;
	bool isAttributeInherited(const wstring & strAttribute) const;
	int_t getInstancesCount() const;
	void addSubType(CIFCEntity * pEntity);
	const vector<CIFCEntity *> & getSubTypes() const;
	const vector<int_t> & getInstances() const;

	void postProcessing();
};

// ------------------------------------------------------------------------------------------------
struct SORT_IFC_ENTITIES
{
	bool operator()(const CIFCEntity * a, const CIFCEntity * b) const
	{
		return wcscmp(a->getName(), b->getName()) < 0;
	}
};