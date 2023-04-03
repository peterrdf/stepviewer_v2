#pragma once

#include "STEPTreeViewBase.h"
#include "STEPItemData.h"
#include "STEPModel.h"
#include "STEPSearchModelStructureDialog.h"

// ------------------------------------------------------------------------------------------------
class CSTEPModelStructureView 
	: public CSTEPTreeViewBase
	, public CItemStateProvider
{

private: // Members
	
	CViewTree* m_pTreeView;
	CImageList* m_pImageList;

	// Cache
	vector<CSTEPItemData*> m_vecItemData;
	map<CProductInstance*, HTREEITEM> m_mapInstance2Item;
	HTREEITEM m_hSelectedItem;

	// --------------------------------------------------------------------------------------------
	// Don't send notifications while updating the view
	bool m_bInitInProgress;

	// Search
	CSTEPSearchModelStructureDialog* m_pSearchDialog;

public: // Methods

	CSTEPModelStructureView(CViewTree* pTreeView);
	virtual ~CSTEPModelStructureView();	

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

	// --------------------------------------------------------------------------------------------
	// Cleanup
	void ResetView();
};

