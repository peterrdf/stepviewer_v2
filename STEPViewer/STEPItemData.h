#pragma once

#include <vector>

using namespace std;

// ------------------------------------------------------------------------------------------------
enum class enumSTEPItemDataType : int
{
	Unknown = -1,
	Model = 0,
	ProductDefinition = 1,
	Assembly = 2,
	ProductInstance = 3
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

	// --------------------------------------------------------------------------------------------
	// Geometry
	bool m_bHasDescendantWithGeometry;

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
	T* GetInstance() const
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

	// --------------------------------------------------------------------------------------------
	// Accessor
	bool& hasDescendantWithGeometry() { return m_bHasDescendantWithGeometry; }
};

