#pragma once

#include "STEPTreeViewBase.h"
#include "STEPItemData.h"
#include "STEPModel.h"
#include "SearchTreeViewDialog.h"

// ------------------------------------------------------------------------------------------------
class CSTEPModelStructureView 
	: public CSTEPTreeViewBase
	, public CItemStateProvider
	, public CSearchTreeViewDialogSite
{

private: // Classes

	enum class enumSearchFilter : int {
		All = 0,
		ProductDefitions = 1,
		Assemblies = 2,
		ProductInstances = 3
	};

private: // Members
	
	CTreeCtrlEx* m_pTreeCtrl;
	CImageList* m_pImageList;

	// Cache
	vector<CSTEPItemData*> m_vecItemData;
	map<CProductInstance*, HTREEITEM> m_mapInstance2Item;
	HTREEITEM m_hSelectedItem;

	// --------------------------------------------------------------------------------------------
	// Don't send notifications while updating the view
	bool m_bInitInProgress;

	// Search
	CSearchTreeViewDialog* m_pSearchDialog;

public: // Methods

	CSTEPModelStructureView(CTreeCtrlEx* pTreeView);
	virtual ~CSTEPModelStructureView();	

	// CSTEPView
	virtual void OnInstanceSelected(CSTEPView* pSender) override;

	// CSTEPTreeViewBase
	virtual void Load();
	virtual CImageList* GetImageList() const override;
	virtual void OnShowWindow(BOOL bShow, UINT nStatus) override;
	virtual void OnTreeItemClick(NMHDR* pNMHDR, LRESULT* pResult) override;
	virtual void OnTreeItemExpanding(NMHDR* pNMHDR, LRESULT* pResult) override;
	virtual void OnContextMenu(CWnd* pWnd, CPoint point) override;
	virtual void OnSearch() override;

	// CItemStateProvider
	virtual bool IsSelected(HTREEITEM hItem);

	// CSearchTreeViewDialogSite
	virtual CTreeCtrlEx* GetTreeView() override;
	virtual vector<wstring> GetSearchFilters() override;
	virtual void LoadChildrenIfNeeded(HTREEITEM hItem) override;
	virtual BOOL ContainsText(int iFilter, HTREEITEM hItem, const CString& strText) override;

private: // Methods	

	void LoadHeaderDescription(HTREEITEM hParent);
	void LoadModel();

	void LoadProductDefinition(CSTEPModel* pModel, CProductDefinition* pDefinition, HTREEITEM hParent);
	void WalkAssemblyTreeRecursively(CSTEPModel* pModel, CProductDefinition* pDefinition, HTREEITEM hParent);
	void LoadProductDefinitionInMemory(CSTEPModel* pModel, CProductDefinition* pDefinition, CSTEPItemData* pParent);
	void WalkAssemblyTreeRecursivelyInMemory(CSTEPModel* pModel, CProductDefinition* pDefinition, CSTEPItemData* pParent);
	CSTEPItemData* FindItemData(CProductInstance* pInstance);
	void LoadItemChildren(CSTEPItemData* pItemData);
	void LoadInstanceAncestors(CProductInstance* pInstance);
	void ResetTree(bool bEnable);
	void ResetTree(HTREEITEM hParent, bool bEnable);
	void UpdateChildren(HTREEITEM hParent);
	void UpdateChildrenInMemory(CSTEPItemData* pParent, bool bEnable);
	void UpdateParent(HTREEITEM hParent);
	void ResetView();
};

