#pragma once

#include "STEPTreeViewBase.h"
#include "STEPItemData.h"
#include "STEPModel.h"
#include "SearchInstancesDialog.h"

// ------------------------------------------------------------------------------------------------
class CSTEPProductsTreeView 
	: public CSTEPTreeViewBase
	, public CItemStateProvider
{

private: // Members

	// --------------------------------------------------------------------------------------------
	// Tree View
	CViewTree* m_pTreeView;

	// --------------------------------------------------------------------------------------------
	// Images
	CImageList* m_pImageList;

	// --------------------------------------------------------------------------------------------
	vector<CSTEPItemData*> m_vecItemData;

	// --------------------------------------------------------------------------------------------
	// CProductInstance* : HTREEITEM
	map<CProductInstance*, HTREEITEM> m_mapInstance2Item;

	// --------------------------------------------------------------------------------------------
	// Selected HTREEITEM
	HTREEITEM m_hSelectedItem;

	// --------------------------------------------------------------------------------------------
	// Don't send notifications while updating the view
	bool m_bInitInProgress;

	// --------------------------------------------------------------------------------------------
	// Search
	CSearchInstancesDialog* m_pSearchDialog;

public: // Methods

	CSTEPProductsTreeView(CViewTree* pTreeView);
	virtual ~CSTEPProductsTreeView();	

	// CSTEPView
	virtual void OnInstanceSelected(CSTEPView* pSender);
	virtual void OnInstanceEnabledStateChanged(CSTEPView* pSender, CProductInstance* pInstance);
	virtual void OnDisableAllButThis(CSTEPView* pSender, CProductInstance* pInstance);
	virtual void OnEnableAllInstances(CSTEPView* pSender);

	// CSTEPTreeViewBase
	virtual void Load();
	virtual CImageList* GetImageList() const;
	virtual void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual void OnTreeItemClick(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnTreeItemExpanding(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnContextMenu(CWnd* pWnd, CPoint point);

	// CItemStateProvider
	virtual bool IsSelected(HTREEITEM hItem);

private: // Methods	

	// --------------------------------------------------------------------------------------------
	void LoadHeaderDescription(HTREEITEM hParent);

	// --------------------------------------------------------------------------------------------
	void LoadModel();

	// --------------------------------------------------------------------------------------------
	void LoadProductDefinition(CSTEPModel* pModel, CProductDefinition* pDefinition, HTREEITEM hParent);

	// --------------------------------------------------------------------------------------------
	void WalkAssemblyTreeRecursively(CSTEPModel* pModel, CProductDefinition* pDefinition, HTREEITEM hParent);

	// --------------------------------------------------------------------------------------------
	void LoadProductDefinitionInMemory(CSTEPModel* pModel, CProductDefinition* pDefinition, CSTEPItemData* pParent);

	// --------------------------------------------------------------------------------------------
	void WalkAssemblyTreeRecursivelyInMemory(CSTEPModel* pModel, CProductDefinition* pDefinition, CSTEPItemData* pParent);

	// --------------------------------------------------------------------------------------------
	CSTEPItemData* FindItemData(CProductInstance* pInstance);

	// --------------------------------------------------------------------------------------------
	void LoadItemChildren(CSTEPItemData* pItemData);

	// --------------------------------------------------------------------------------------------
	void LoadInstanceAncestors(CProductInstance* pInstance);

	// --------------------------------------------------------------------------------------------
	void ResetTree(bool bEnable);

	// --------------------------------------------------------------------------------------------
	void ResetTree(HTREEITEM hParent, bool bEnable);

	// --------------------------------------------------------------------------------------------
	void UpdateChildren(HTREEITEM hParent);

	// --------------------------------------------------------------------------------------------
	void UpdateChildrenInMemory(CSTEPItemData* pParent, bool bEnable);

	// --------------------------------------------------------------------------------------------
	void UpdateParent(HTREEITEM hParent);
};

