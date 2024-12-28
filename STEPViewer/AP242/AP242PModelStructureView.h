#pragma once

#include "_ptr.h"

#include "TreeViewBase.h"
#include "AP242ItemData.h"
#include "AP242Model.h"
#include "SearchTreeCtrlDialog.h"

// ************************************************************************************************
class _ap242_product_definition;
class _ap242_instance;
class _ap242_assembly;

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
		ProductInstances = 3,
		DraughtingModel = 4,
		AnnotationPlane = 5,
		DraughtingCallout = 6
	};

private: // Members
	
	CTreeCtrlEx* m_pTreeCtrl;
	CImageList* m_pImageList;

	// Cache	
	map<_ap242_product_definition*, _instance_iterator*> m_mapInstanceIterators;
	map<_instance*, HTREEITEM> m_mapInstance2Item;
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
	void LoadProduct(CAP242Model* pModel, _ap242_product_definition* pProduct, HTREEITEM hParent);
	void LoadAssembly(CAP242Model* pModel, _ap242_assembly* pAssembly, HTREEITEM hParent);
	void LoadInstance(CAP242Model* pModel, _ap242_instance* pInstance, HTREEITEM hParent);

	bool HasDescendantsWithGeometry(CAP242Model* pModel, _ap242_product_definition* pProduct);
	void HasDescendantsWithGeometryRecursively(CAP242Model* pModel, _ap242_product_definition* pProduct, bool& bHasDescendantWithGeometry);
	bool HasDescendantsWithGeometry(CAP242Model* pModel, _ap242_assembly* pAssembly);
	bool HasDescendantsWithGeometry(_ap242_draughting_model* pDraughtingModel);

	void LoadDraughtingModel(_ap242_draughting_model* pDraugthingModel, HTREEITEM hParent);
	void LoadAnnotationPlane(_ap242_annotation_plane* pAnnotationPlane, HTREEITEM hParent);
	void LoadDraughtingCallout(_ap242_draughting_callout* pDraugthingCallout, HTREEITEM hParent);

	void ResetTree(bool bEnable);
	void ResetTree(HTREEITEM hParent, bool bEnable);
	void UpdateChildrenItemData(CAP242ItemData* pParent, bool bEnable);
	void UpdateChildrenUI(HTREEITEM hParent);
	void UpdateParentsItemDataAndUI(HTREEITEM hParent);
	
	void ResetView();
};

