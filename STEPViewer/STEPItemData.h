#pragma once

#include <vector>

using namespace std;

// ------------------------------------------------------------------------------------------------
enum class enumSTEPItemDataType
{
	dtUnknown = -1,
	dtModel = 0,
	dtProductDefinition = 1,
	dtAssembly = 2,
	dtProductInstance = 3
};

// ------------------------------------------------------------------------------------------------
class CSTEPItemData
{

private: // Members	

	// --------------------------------------------------------------------------------------------
	// Parent
	CSTEPItemData* m_pParent;

	// --------------------------------------------------------------------------------------------
	// Instance - C++ wrapper class
	int64_t* m_pInstance;

	// --------------------------------------------------------------------------------------------
	// Type
	enumSTEPItemDataType m_enSTEPItemDataType;

	// --------------------------------------------------------------------------------------------
	// Tree Item
	HTREEITEM m_hItem;

	// --------------------------------------------------------------------------------------------
	// Children
	vector<CSTEPItemData*> m_vecChildren;

public: // Members

	// --------------------------------------------------------------------------------------------
	// ctor
	CSTEPItemData(CSTEPItemData* pParent, int64_t* pInstance, enumSTEPItemDataType enItemDataType);

	// --------------------------------------------------------------------------------------------
	// dtor
	virtual ~CSTEPItemData();

	// --------------------------------------------------------------------------------------------
	// Getter
	CSTEPItemData* getParent();

	// --------------------------------------------------------------------------------------------
	// Getter
	template<typename T>
	T* getInstance() const
	{
		return (T*)m_pInstance;
	}

	// --------------------------------------------------------------------------------------------
	// Getter
	enumSTEPItemDataType getType() const;

	// --------------------------------------------------------------------------------------------
	// Accessor
	HTREEITEM& treeItem();

	// --------------------------------------------------------------------------------------------
	// Accessor
	vector<CSTEPItemData*>& children();
};

