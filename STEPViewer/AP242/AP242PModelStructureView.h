#pragma once

#include "_ptr.h"

#include "ModelStructureViewBase.h"
#include "SearchTreeCtrlDialog.h"
#include "_ap242_model_structure.h"

// ************************************************************************************************
class _ap242_model;
class _ap242_geometry;
class _ap242_product_definition;
class _ap242_product_shape;
class _ap242_product_shape_representation;
class _ap242_instance;
class _ap242_assembly;
class _ap242_draughting_model;
class _ap242_annotation_plane;
class _ap242_draughting_callout;

// ************************************************************************************************
class CAP242PModelStructureView
	: public CModelStructureViewBase
	, public CItemStateProvider
	, public CSearchTreeCtrlDialogSite {

private: // Classes

	// ********************************************************************************************
	enum class enumSearchFilter : int {
		All = 0,
		ProductDefitions = 1,
		Assemblies = 2,
		ProductInstances = 3,
		DraughtingModel = 4,
		AnnotationPlane = 5,
		DraughtingCallout = 6
	};

	// ********************************************************************************************
	typedef map<_instance*, vector<HTREEITEM>> ITEMS;
	typedef map<_ap242_node*, HTREEITEM> NODES;

private: // Members

	// Model
	_ap242_model* m_pModel;
	_ap242_model_structure* m_pModelStructure;

	CImageList* m_pImageList;

	// Cache	
	NODES m_mapNodes;
	map<_ap242_geometry*, _instance_iterator*> m_mapInstanceIterators;
	ITEMS m_mapItems;
	HTREEITEM m_hSelectedItem;

	bool m_bInitInProgress; // don't send notifications while updating the view

	// Search
	CSearchTreeCtrlDialog* m_pSearchDialog;

public: // Methods

	CAP242PModelStructureView(CTreeCtrlEx* pTreeCtrl);
	virtual ~CAP242PModelStructureView();

	// _view
	virtual void onInstanceEnabledStateChanged(_view* pSender, _instance* pInstance, int iFlag) override;
	virtual void onInstanceSelected(_view* pSender) override;
	virtual void onApplicationPropertyChanged(_view* pSender, enumApplicationProperty enApplicationProperty);

	// CModelStructureViewBase
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

	_ap242_model* GetModel() const { return m_pModel; }
	_ap242_model_structure* GetModelStructure() const { return m_pModelStructure; }

	HTREEITEM GetModelItem() const;

	void Tree_Update(HTREEITEM hItem, bool bRecursive = true);
	void Tree_UpdateChildren(HTREEITEM hItem);
	void InMemoryTree_EnableChildren(_ap242_node* pNode, bool bEnable);
	void Tree_UpdateParents(HTREEITEM hItem);

	int Tree_GetItemState(HTREEITEM hItem);
	int InMemoryTree_GetItemState(_ap242_node* pNode);

	void ResetView();
};
