#pragma once

#include "ModelStructureViewBase.h"
#include "CIS2Model.h"
#include "CIS2Instance.h"
#include "SearchTreeCtrlDialog.h"

#include <map>
using namespace std;

// ************************************************************************************************
// IFC Model Structure
// ************************************************************************************************

// ------------------------------------------------------------------------------------------------
class CCIS2ModelStructureView 
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
	map<CCIS2Instance*, HTREEITEM> m_mapInstance2GeometryItem;	
	map<CCIS2Instance*, HTREEITEM> m_mapSelectedInstances;
	
	// Search
	CSearchTreeCtrlDialog* m_pSearchDialog;

public: // Methods

	CCIS2ModelStructureView(CTreeCtrlEx* pTreeCtrl);
	virtual ~CCIS2ModelStructureView();

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

	void LoadModel(CCIS2Model* pModel);
	void LoadIsDecomposedBy(CCIS2Model* pModel, SdaiInstance sdaiInstance, HTREEITEM hParent);
	void LoadIsNestedBy(CCIS2Model* pModel, SdaiInstance sdaiInstance, HTREEITEM hParent);
	void LoadContainsElements(CCIS2Model* pModel, SdaiInstance sdaiInstance, HTREEITEM hParent);
	void LoadInstance(CCIS2Model* pModel, SdaiInstance sdaiInstance, HTREEITEM hParent);
	void LoadTree_UpdateItems(HTREEITEM hModel);
	void LoadTree_UpdateItem(HTREEITEM hParent);
	void ClickItem_UpdateChildren(HTREEITEM hParent);
	void ClickItem_UpdateParent(HTREEITEM hParent, BOOL bRecursive = TRUE);
	void UnselectAllItems();
	void ResetView();
};

