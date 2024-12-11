#pragma once

#include "TreeViewBase.h"
#include "AP242ItemData.h"
#include "AP242Model.h"
#include "SearchTreeCtrlDialog.h"

// ------------------------------------------------------------------------------------------------
class CAP242PModelStructureView 
	: public CTreeViewBase
	, public CItemStateProvider
	, public CSearchTreeCtrlDialogSite
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
	vector<CAP242ItemData*> m_vecItemData;
	map<_instance*, HTREEITEM> m_mapInstance2Item;
	HTREEITEM m_hSelectedItem;

	// --------------------------------------------------------------------------------------------
	// Don't send notifications while updating the view
	bool m_bInitInProgress;

	// Search
	CSearchTreeCtrlDialog* m_pSearchDialog;

public: // Methods

	CAP242PModelStructureView(CTreeCtrlEx* pTreeView);
	virtual ~CAP242PModelStructureView();	

	// CViewBase
	virtual void OnInstanceSelected(CViewBase* pSender) override;

	// CTreeViewBase
	virtual void Load();
	virtual CImageList* GetImageList() const override;
	virtual void OnShowWindow(BOOL bShow, UINT nStatus) override;
	virtual void OnTreeItemClick(NMHDR* pNMHDR, LRESULT* pResult) override;
	virtual void OnTreeItemExpanding(NMHDR* pNMHDR, LRESULT* pResult) override;
	virtual void OnContextMenu(CWnd* pWnd, CPoint point) override;
	virtual void OnSearch() override;

	// CItemStateProvider
	virtual bool IsSelected(HTREEITEM hItem);

	// CSearchTreeCtrlDialogSite
	virtual CTreeCtrlEx* GetTreeView() override;
	virtual vector<CString> GetSearchFilters() override;
	virtual void LoadChildrenIfNeeded(HTREEITEM hItem) override;
	virtual BOOL ContainsText(int iFilter, HTREEITEM hItem, const CString& strText) override;

private: // Methods	

	void LoadHeaderDescription(HTREEITEM hParent);
	void LoadModel();

	void WalkAssemblyTreeRecursively(CAP242Model* pModel, CAP242ProductDefinition* pDefinition, HTREEITEM hParent);
	void LoadProductDefinitionsInMemory(CAP242Model* pModel, CAP242ProductDefinition* pDefinition, CAP242ItemData* pParent);
	void WalkAssemblyTreeRecursivelyInMemory(CAP242Model* pModel, CAP242ProductDefinition* pDefinition, CAP242ItemData* pParent);
	void SearchForDescendantWithGeometry();
	void SearchForDescendantWithGeometryRecursively(CAP242ItemData* pItemData, bool& bHasDescendantWithGeometry);
	CAP242ItemData* FindItemData(CAP242ProductInstance* pInstance);
	void LoadItemChildren(CAP242ItemData* pItemData);
	void LoadInstanceAncestors(CAP242ProductInstance* pInstance);
	void ResetTree(bool bEnable);
	void ResetTree(HTREEITEM hParent, bool bEnable);
	void UpdateChildren(HTREEITEM hParent);
	void UpdateChildrenInMemory(CAP242ItemData* pParent, bool bEnable);
	void UpdateParent(HTREEITEM hParent);
	void ResetView();
};

