#pragma once

#include <vector>

using namespace std;

// ************************************************************************************************
class CAP242Assembly;
class CAP242Model;
class CAP242ProductDefinition;
class CAP242ProductInstance;

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
class CAP242ItemData
{

private: // Members	

	// --------------------------------------------------------------------------------------------
	// Parent
	CAP242ItemData* m_pParent;

	// --------------------------------------------------------------------------------------------
	// Instance - C++ wrapper class
	SdaiInstance m_pSdaiInstance;

	// --------------------------------------------------------------------------------------------
	CAP242Assembly* m_pAssembly;

	// --------------------------------------------------------------------------------------------
	CAP242Model* m_pAP242Model;

	// --------------------------------------------------------------------------------------------
	CAP242ProductDefinition* m_pProductDefinition;

	// --------------------------------------------------------------------------------------------
	CAP242ProductInstance* m_pProductInstance;

	// --------------------------------------------------------------------------------------------
	// Type
	enumSTEPItemDataType m_enSTEPItemDataType;

	// --------------------------------------------------------------------------------------------
	// Tree Item
	HTREEITEM m_hItem;

	// --------------------------------------------------------------------------------------------
	// Children
	vector<CAP242ItemData*> m_vecChildren;

	// --------------------------------------------------------------------------------------------
	// Geometry
	bool m_bHasDescendantWithGeometry;

public: // Members

	// --------------------------------------------------------------------------------------------
	// ctor
	CAP242ItemData(CAP242ItemData* pParent, SdaiInstance pSdaiInstance, enumSTEPItemDataType enItemDataType);

	// --------------------------------------------------------------------------------------------
	// ctor
	CAP242ItemData(CAP242ItemData* pParent, CAP242Assembly* pAssembly, enumSTEPItemDataType enItemDataType);

	// --------------------------------------------------------------------------------------------
	// ctor
	CAP242ItemData(CAP242ItemData* pParent, CAP242Model* pAP242Model, enumSTEPItemDataType enItemDataType);

	// --------------------------------------------------------------------------------------------
	// ctor
	CAP242ItemData(CAP242ItemData* pParent, CAP242ProductDefinition* pProductDefinition, enumSTEPItemDataType enItemDataType);

	// --------------------------------------------------------------------------------------------
	// ctor
	CAP242ItemData(CAP242ItemData* pParent, CAP242ProductInstance* pProductInstance, enumSTEPItemDataType enItemDataType);

	// --------------------------------------------------------------------------------------------
	// dtor
	virtual ~CAP242ItemData();

	// --------------------------------------------------------------------------------------------
	// Getter
	CAP242ItemData* getParent();

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

	CAP242ProductInstance* GetProductInstance() const
	{
		return m_pProductInstance;
	}

	CAP242ProductDefinition* GetProductDefinition() const
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
	vector<CAP242ItemData*>& children();

	// --------------------------------------------------------------------------------------------
	// Accessor
	bool& hasDescendantWithGeometry() { return m_bHasDescendantWithGeometry; }
};

