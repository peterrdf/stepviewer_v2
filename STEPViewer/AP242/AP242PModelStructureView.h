#pragma once

#include "_ptr.h"

#include "TreeViewBase.h"
#include "AP242ItemData.h"
#include "AP242Model.h"
#include "SearchTreeCtrlDialog.h"

// ************************************************************************************************
typedef _vector_sequential_iterator<_instance> _instance_iterator;

// ************************************************************************************************
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
	map<CAP242ProductDefinition*, _instance_iterator*> m_mapInstanceIterators;
	map<CAP242ProductInstance*, HTREEITEM> m_mapInstance2Item;
	vector<CAP242ItemData*> m_vecItemData;
	HTREEITEM m_hSelectedItem;
		
	bool m_bInitInProgress; // don't send notifications while updating the view

	// Search
	CSearchTreeCtrlDialog* m_pSearchDialog;

public: // Methods

	CAP242PModelStructureView(CTreeCtrlEx* pTreeView);
	virtual ~CAP242PModelStructureView();	

	// _view
	virtual void onInstanceSelected(_view* pSender) override;

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

	void LoadModel();
	void LoadHeader(HTREEITEM hParent);	
	void LoadProduct(CAP242Model* pModel, CAP242ProductDefinition* pProduct, HTREEITEM hParent);
	void LoadAssembly(CAP242Model* pModel, CAP242Assembly* pAssembly, HTREEITEM hParent);
	void LoadInstance(CAP242Model* pModel, CAP242ProductInstance* pInstance, HTREEITEM hParent);

	bool HasDescendantsWithGeometry(CAP242Model* pModel, CAP242ProductDefinition* pProduct);
	void HasDescendantsWithGeometryRecursively(CAP242Model* pModel, CAP242ProductDefinition* pProduct, bool& bHasDescendantWithGeometry);
	bool HasDescendantsWithGeometry(CAP242Model* pModel, CAP242Assembly* pAssembly);

	void LoadDraughtingModel(_ap242_draughting_model* pDraugthingModel, HTREEITEM hParent);
	void LoadAnnotationPlane(_ap242_annotation_plane* pAnnotationPlane, HTREEITEM hParent);
	void LoadDraughtingCallout(_ap242_draughting_callout* pDraugthingCallout, HTREEITEM hParent);

	void ResetTree(bool bEnable);
	void ResetTree(HTREEITEM hParent, bool bEnable);
	void UpdateChildren(HTREEITEM hParent);
	void UpdateChildrenInMemory(CAP242ItemData* pParent, bool bEnable);
	void UpdateParent(HTREEITEM hParent);
	void ResetView();
};

