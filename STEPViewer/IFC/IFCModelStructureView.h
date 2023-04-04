#pragma once

#include "STEPTreeViewBase.h"
#include "STEPItemData.h"
#include "STEPModel.h"
#include "IFCInstance.h"
#include "SearchTreeViewDialog.h"
#include <map>

using namespace std;

// ************************************************************************************************
// IFC Model Structure
// ************************************************************************************************

// ------------------------------------------------------------------------------------------------
class CIFCModelStructureView 
	: public CSTEPTreeViewBase
	, public CItemStateProvider
	, public CSearchTreeViewDialogSite
{

private: // Classes

	enum class enumSearchFilter : int {
		All = 0,
		ExpressID = 1,
	};

private: // Members	
	
	CViewTree* m_pTreeView;
	CImageList* m_pImageList;

	// Cache
	map<HTREEITEM, CIFCModel*> m_mapModelHTREEITEM;
	map<CIFCInstance*, HTREEITEM> m_mapInstance2Item;	
	map<CIFCInstance*, HTREEITEM> m_mapSelectedInstances;
	
	// Search
	CSearchTreeViewDialog* m_pSearchDialog;

public: // Methods

	CIFCModelStructureView(CViewTree* pTreeView);
	virtual ~CIFCModelStructureView();

	// CSTEPView
	virtual void OnInstanceSelected(CSTEPView* pSender) override;

	// CSTEPTreeViewBase
	virtual void Load() override;
	virtual CImageList* GetImageList() const override;
	virtual void OnShowWindow(BOOL bShow, UINT nStatus) override;
	virtual void OnTreeItemClick(NMHDR* pNMHDR, LRESULT* pResult) override;
	virtual void OnTreeItemExpanding(NMHDR* pNMHDR, LRESULT* pResult) override;
	virtual void OnContextMenu(CWnd* pWnd, CPoint point) override;
	virtual void OnSearch() override;

	// CItemStateProvider
	virtual bool IsSelected(HTREEITEM hItem);

	// CSearchTreeViewDialogSite
	virtual CViewTree* GetTreeView() override;
	virtual vector<wstring> GetSearchFilters() override;
	virtual void LoadChildrenIfNeeded(HTREEITEM hItem) override;
	virtual BOOL ContainsText(int iFilter, HTREEITEM hItem, const CString& strText) override;

private: // Methods		

	CIFCModel* GetModel(HTREEITEM hItem);	
	void LoadModel(CIFCModel* pModel);
	void LoadHeader(CIFCModel* pModel, HTREEITEM hModel);
	void LoadProject(CIFCModel* pModel, HTREEITEM hModel, int64_t iIFCProjectInstance);
	void LoadIsDecomposedBy(CIFCModel* pModel, int64_t iInstance, HTREEITEM hParent);
	void LoadContainsElements(CIFCModel* pModel, int64_t iInstance, HTREEITEM hParent);
	void LoadObject(CIFCModel* pModel, int64_t iInstance, HTREEITEM hParent);
	void LoadUnreferencedItems(CIFCModel* pModel, HTREEITEM hModel);
	void LoadTree_UpdateItems(HTREEITEM hModel);
	void LoadTree_UpdateItem(HTREEITEM hParent);
	void ClickItem_UpdateChildren(HTREEITEM hParent);
	void ClickItem_UpdateParent(HTREEITEM hParent);
	void UnselectAllItems();
	void ResetView();
};

