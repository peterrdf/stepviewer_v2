#pragma once

#include <vector>

using namespace std;

// ************************************************************************************************
class CAP242Assembly;
class CAP242Model;
class CProductDefinition;
class CProductInstance;

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
	SdaiInstance m_pSdaiInstance;

	// --------------------------------------------------------------------------------------------
	CAP242Assembly* m_pAssembly;

	// --------------------------------------------------------------------------------------------
	CAP242Model* m_pAP242Model;

	// --------------------------------------------------------------------------------------------
	CProductDefinition* m_pProductDefinition;

	// --------------------------------------------------------------------------------------------
	CProductInstance* m_pProductInstance;

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
	CSTEPItemData(CSTEPItemData* pParent, SdaiInstance pSdaiInstance, enumSTEPItemDataType enItemDataType);

	// --------------------------------------------------------------------------------------------
	// ctor
	CSTEPItemData(CSTEPItemData* pParent, CAP242Assembly* pAssembly, enumSTEPItemDataType enItemDataType);

	// --------------------------------------------------------------------------------------------
	// ctor
	CSTEPItemData(CSTEPItemData* pParent, CAP242Model* pAP242Model, enumSTEPItemDataType enItemDataType);

	// --------------------------------------------------------------------------------------------
	// ctor
	CSTEPItemData(CSTEPItemData* pParent, CProductDefinition* pProductDefinition, enumSTEPItemDataType enItemDataType);

	// --------------------------------------------------------------------------------------------
	// ctor
	CSTEPItemData(CSTEPItemData* pParent, CProductInstance* pProductInstance, enumSTEPItemDataType enItemDataType);

	// --------------------------------------------------------------------------------------------
	// dtor
	virtual ~CSTEPItemData();

	// --------------------------------------------------------------------------------------------
	// Getter
	CSTEPItemData* getParent();

	// --------------------------------------------------------------------------------------------
	// Getter
	///template<typename T>
	///T* GetSdaiInstance() const
	SdaiInstance GetSdaiInstance__() const
	{
		return m_pSdaiInstance;
	}

	CAP242Assembly* GetAssembly() const
	{
		return m_pAssembly;
	}

	CProductInstance* GetProductInstance() const
	{
		return m_pProductInstance;
	}

	CProductDefinition* GetProductDefinition() const
	{
		return m_pProductDefinition;
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

