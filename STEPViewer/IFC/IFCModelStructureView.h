#pragma once

#include "STEPTreeViewBase.h"
#include "STEPItemData.h"
#include "STEPModel.h"
#include "SearchInstancesDialog.h"
#include "IFCInstance.h"
#include "SearchModelStructureDialog.h"
#include <map>

using namespace std;

// ************************************************************************************************
// IFC Model Structure
// ************************************************************************************************

// ------------------------------------------------------------------------------------------------
class CIFCModelStructureView 
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
	CSearchModelStructureDialog* m_pSearchDialog;

public: // Methods

	CIFCModelStructureView(CViewTree* pTreeView);
	virtual ~CIFCModelStructureView();

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

