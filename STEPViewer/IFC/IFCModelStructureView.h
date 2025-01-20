#pragma once

#include "_ifc_instance.h"

#include "ModelStructureViewBase.h"
#include "SearchTreeCtrlDialog.h"

#include <map>
using namespace std;

// ************************************************************************************************
class _ifc_model;

// ************************************************************************************************
// IFC Model Structure
class CIFCModelStructureView 
	: public CModelStructureViewBase
	, public CItemStateProvider
	, public CSearchTreeCtrlDialogSite
{

private: // Classes

	enum class enumSearchFilter : int {
		All = 0,
		ExpressID = 1,
	};

private: // Members	
	
	CImageList* m_pImageList;

	// Cache
	map<_ifc_instance*, vector<HTREEITEM>> m_mapInstanceItems;	
	_ifc_instance* m_pSelectedInstance;

	
	// Search
	CSearchTreeCtrlDialog* m_pSearchDialog;

public: // Methods

	CIFCModelStructureView(CTreeCtrlEx* pTreeCtrl);
	virtual ~CIFCModelStructureView();

	// _view
	virtual void onInstanceSelected(_view* pSender) override;

	// CModelStructureViewBase
	virtual void Load() override;
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

	void LoadModel(_ifc_model* pModel);
	void LoadProject(_ifc_model* pModel, HTREEITEM hModel, SdaiInstance sdaiProjectInstance);
	void LoadIsDecomposedBy(_ifc_model* pModel, SdaiInstance sdaiInstance, HTREEITEM hParent);
	void LoadIsNestedBy(_ifc_model* pModel, SdaiInstance sdaiInstance, HTREEITEM hParent);
	void LoadContainsElements(_ifc_model* pModel, SdaiInstance sdaiInstance, HTREEITEM hParent);
	void LoadBoundedBy(_ifc_model* pModel, SdaiInstance sdaiInstance, HTREEITEM hParent);
	HTREEITEM LoadInstance(_ifc_model* pModel, SdaiInstance sdaiInstance, HTREEITEM hParent, bool bLoadChildren = true);
	void LoadGroups(_ifc_model* pModel, HTREEITEM hModel);
	void LoadSpaceBoundaries(_ifc_model* pModel, HTREEITEM hModel);
	void LoadBuildingStoreyChildren(_ifc_model* pModel, SdaiInstance sdaiInstance, HTREEITEM hBuildingStorey);
	void LoadUnreferencedItems(_ifc_model* pModel, HTREEITEM hModel);

	void Tree_UpdateAll(HTREEITEM hModel);
	void Model_EnableChildren(HTREEITEM hParent, bool bEnable);
	void Tree_UpdateChildren(HTREEITEM hItem);
	void Tree_UpdateParents(HTREEITEM hItem);
	void ClickItem_UpdateChildren(HTREEITEM hParent);
	void ClickItem_UpdateParent(HTREEITEM hParent, BOOL bRecursive = TRUE);
	void Tree_Unselect();
	void Tree_Reset(bool bEnable);
	void Tree_Reset(HTREEITEM hParent, bool bEnable);

	void ResetView();
};

