#pragma once

#include "STEPTreeViewBase.h"
#include "STEPItemData.h"
#include "STEPModel.h"
#include "SearchInstancesDialog.h"
#include "IFCInstance.h"
#include "SearchDecompContDialog.h"
#include <map>

using namespace std;

// ************************************************************************************************
// IFC Model Structure
// ************************************************************************************************

// ------------------------------------------------------------------------------------------------
class CIFCDecompContTreeView 
	: public CSTEPTreeViewBase
	, public CItemStateProvider
{

private: // Members	
	
	CViewTree* m_pTreeView;
	CImageList* m_pImageList;

	// Cache
	map<HTREEITEM, CIFCModel*> m_mapModelHTREEITEM;
	map<CIFCInstance*, HTREEITEM> m_mapInstance2Item;	
	map<CIFCInstance*, HTREEITEM> m_mapSelectedInstances;
	
	// Search
	CSearchDecompContDialog* m_pSearchDialog;

public: // Methods

	CIFCDecompContTreeView(CViewTree* pTreeView);
	virtual ~CIFCDecompContTreeView();

	// CSTEPView
	virtual void OnInstanceSelected(CSTEPView* pSender);

	// CSTEPTreeViewBase
	virtual void Load();
	virtual CImageList* GetImageList() const;
	virtual void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual void OnTreeItemClick(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnTreeItemExpanding(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnContextMenu(CWnd* pWnd, CPoint point);
	virtual void OnSearch();

	// CItemStateProvider
	virtual bool IsSelected(HTREEITEM hItem);

private: // Methods		

	// --------------------------------------------------------------------------------------------
	CIFCModel* GetModel(HTREEITEM hItem);

	// --------------------------------------------------------------------------------------------
	HTREEITEM GetModelHTREEITEM(HTREEITEM hItem);

	// --------------------------------------------------------------------------------------------
	HTREEITEM GetActiveModelHTREEITEM();

	// --------------------------------------------------------------------------------------------
	// Load an IFC model
	void LoadModel(CIFCModel* pModel);

	// --------------------------------------------------------------------------------------------
	// Load IFC Header
	void LoadHeader(CIFCModel* pModel, HTREEITEM hModel);

	// --------------------------------------------------------------------------------------------
	// IFCProject
	void LoadProject(CIFCModel* pModel, HTREEITEM hModel, int64_t iIFCProjectInstance);

	// --------------------------------------------------------------------------------------------
	// IsDecomposedBy
	void LoadIsDecomposedBy(CIFCModel* pModel, int64_t iInstance, HTREEITEM hParent);

	// --------------------------------------------------------------------------------------------
	// ContainsElements
	void LoadContainsElements(CIFCModel* pModel, int64_t iInstance, HTREEITEM hParent);

	// --------------------------------------------------------------------------------------------
	// Loads an IFC object
	void LoadObject(CIFCModel* pModel, int64_t iInstance, HTREEITEM hParent);

	// --------------------------------------------------------------------------------------------
	// Check for properties
	bool HasProperties(CIFCModel* pModel, int64_t iInstance);

	// --------------------------------------------------------------------------------------------
	// Support for properties
	void LoadProperties(CIFCModel* pModel, int64_t iInstance, HTREEITEM hParent);

	// --------------------------------------------------------------------------------------------
	// Support for properties
	void LoadRelDefinesByProperties(CIFCModel* pModel, int64_t iIFCIsDefinedByInstance, HTREEITEM hParent);

	// --------------------------------------------------------------------------------------------
	// Support for properties
	void LoadRelDefinesByType(CIFCModel* pModel, int64_t iIFCIsDefinedByInstance, HTREEITEM hParent);

	// --------------------------------------------------------------------------------------------
	// Support for properties
	void LoadPropertySet(CIFCModel* pModel, int64_t iIFCPropertySetInstance, HTREEITEM hParent);

	// --------------------------------------------------------------------------------------------
	// Support for properties
	void LoadQuantites(CIFCModel* pModel, int64_t iIFCPropertySetInstance, HTREEITEM hParent);

	// --------------------------------------------------------------------------------------------
	// Support for properties
	void LoadIFCQuantityLength(CIFCModel* pModel, int_t iIFCQuantity, HTREEITEM hParent);

	// --------------------------------------------------------------------------------------------
	// Support for properties
	void LoadIFCQuantityArea(CIFCModel* pModel, int_t iIFCQuantity, HTREEITEM hParent);

	// --------------------------------------------------------------------------------------------
	// Support for properties
	void LoadIFCQuantityVolume(CIFCModel* pModel, int_t iIFCQuantity, HTREEITEM hParent);

	// --------------------------------------------------------------------------------------------
	// Support for properties
	void LoadIFCQuantityCount(CIFCModel* pModel, int_t iIFCQuantity, HTREEITEM hParent);

	// --------------------------------------------------------------------------------------------
	// Support for properties
	void LoadIFCQuantityWeight(CIFCModel* pModel, int_t iIFCQuantity, HTREEITEM hParent);

	// --------------------------------------------------------------------------------------------
	// Support for properties
	void LoadIFCQuantityTime(CIFCModel* pModel, int_t iIFCQuantity, HTREEITEM hParent);

	// --------------------------------------------------------------------------------------------
	// Unreferenced
	void LoadUnreferencedItems(CIFCModel* pModel, HTREEITEM hModel);

	// --------------------------------------------------------------------------------------------
	// Updates the initial state of tree items
	void LoadTree_UpdateItems(HTREEITEM hModel);

	// --------------------------------------------------------------------------------------------
	// Updates the initial state of a tree item
	void LoadTree_UpdateItem(HTREEITEM hParent);

	// --------------------------------------------------------------------------------------------
	// Updates the state of the tree items
	void ClickItem_UpdateChildren(HTREEITEM hParent);

	// --------------------------------------------------------------------------------------------
	// Updates the state of the tree items
	void ClickItem_UpdateParent(HTREEITEM hParent);

	// --------------------------------------------------------------------------------------------
	// All instances have been deselected
	void UnselectAllItems();

	// --------------------------------------------------------------------------------------------
	// Cleanup
	void ResetView();
};

