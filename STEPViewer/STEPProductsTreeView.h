#pragma once

#include "STEPTreeViewBase.h"
#include "STEPItemData.h"
#include "STEPModel.h"
#include "SearchInstancesDialog.h"

// ------------------------------------------------------------------------------------------------
class CSTEPProductsTreeView : public CSTEPTreeViewBase
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

	// --------------------------------------------------------------------------------------------
	// ctor
	CSTEPProductsTreeView(CViewTree* pTreeView);

	// --------------------------------------------------------------------------------------------
	// dtor
	virtual ~CSTEPProductsTreeView();	

	// --------------------------------------------------------------------------------------------
	// CSTEPView
	virtual void OnInstanceSelected(CSTEPView* pSender);

	// --------------------------------------------------------------------------------------------
	// CSTEPView
	virtual void OnInstanceEnabledStateChanged(CSTEPView* pSender, CProductInstance* pProductInstance);

	// --------------------------------------------------------------------------------------------
	// CSTEPView
	virtual void OnDisableAllButThis(CSTEPView* pSender, CProductInstance* pProductInstance);

	// --------------------------------------------------------------------------------------------
	// CSTEPView
	virtual void OnEnableAllInstances(CSTEPView* pSender);

	// --------------------------------------------------------------------------------------------
	// CSTEPTreeViewBase
	virtual void Load();

	// --------------------------------------------------------------------------------------------
	// CSTEPTreeViewBase
	virtual CImageList* GetImageList() const;

	// --------------------------------------------------------------------------------------------
	// CSTEPTreeViewBase
	virtual void OnShowWindow(BOOL bShow, UINT nStatus);

	// --------------------------------------------------------------------------------------------
	// CSTEPTreeViewBase
	virtual void OnTreeItemClick(NMHDR* pNMHDR, LRESULT* pResult);

	// --------------------------------------------------------------------------------------------
	// CSTEPTreeViewBase
	virtual void OnTreeItemExpanding(NMHDR* pNMHDR, LRESULT* pResult);

	// --------------------------------------------------------------------------------------------
	// CSTEPTreeViewBase
	virtual void OnContextMenu(CWnd* pWnd, CPoint point);

private: // Methods	

	// --------------------------------------------------------------------------------------------
	void LoadHeaderDescription(HTREEITEM hParent);

	// --------------------------------------------------------------------------------------------
	void LoadModel();

	// --------------------------------------------------------------------------------------------
	void LoadProductDefinition(CSTEPModel* pModel, CProductDefinition* pProductDefinition, HTREEITEM hParent);

	// --------------------------------------------------------------------------------------------
	void WalkAssemblyTreeRecursively(CSTEPModel* pModel, CProductDefinition* pProductDefinition, HTREEITEM hParent);

	// --------------------------------------------------------------------------------------------
	void LoadProductDefinitionInMemory(CSTEPModel* pModel, CProductDefinition* pProductDefinition, CSTEPItemData* pParent);

	// --------------------------------------------------------------------------------------------
	void WalkAssemblyTreeRecursivelyInMemory(CSTEPModel* pModel, CProductDefinition* pProductDefinition, CSTEPItemData* pParent);

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

