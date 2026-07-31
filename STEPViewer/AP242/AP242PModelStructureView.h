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
class CAP242ItemData;

// ************************************************************************************************
class CAP242PModelStructureView 
	: public CModelStructureViewBase
	, public CItemStateProvider
	, public CSearchTreeCtrlDialogSite
{

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

	CImageList* m_pImageList;

	// Cache
	_ap242_model_structure* m_pModelStructure;
	NODES m_mapNodes;
	map<_ap242_geometry*, _instance_iterator*> m_mapInstanceIterators;
	ITEMS m_mapItems;
	vector<CAP242ItemData*> m_vecItemData;
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
	void LoadProduct(_ap242_model* pModel, _ap242_product_definition* pProduct, HTREEITEM hParent);
	void LoadAssembly(_ap242_model* pModel, _ap242_assembly* pAssembly, HTREEITEM hParent);
	void LoadInstance(_ap242_model* pModel, _ap242_instance* pInstance, HTREEITEM hParent);

	bool HasDescendantsWithGeometry(_ap242_model* pModel, _ap242_product_definition* pProduct);
	void HasDescendantsWithGeometryRecursively(_ap242_model* pModel, _ap242_product_definition* pProduct, bool& bHasDescendantWithGeometry);
	bool HasDescendantsWithGeometry(_ap242_model* pModel, _ap242_product_shape* pProductShape);
	bool HasDescendantsWithGeometry(_ap242_model* pModel, _ap242_product_shape_representation* pProductShapeRepresentation);
	bool HasDescendantsWithGeometry(_ap242_model* pModel, _ap242_assembly* pAssembly);
	bool HasDescendantsWithGeometry(_ap242_draughting_model* pDraughtingModel);

	void LoadDraughtingModel(_ap242_draughting_model* pDraugthingModel, HTREEITEM hParent);
	void LoadAnnotationPlane(_ap242_annotation_plane* pAnnotationPlane, HTREEITEM hParent);
	void LoadDraughtingCallout(_ap242_draughting_callout* pDraugthingCallout, HTREEITEM hParent);

	void Tree_Reset(bool bEnable);
	void Tree_Reset(HTREEITEM hItem, bool bEnable);
	void Model_EnableChildren(CAP242ItemData* pParent, bool bEnable);
	void Tree_UpdateChildren(HTREEITEM hItem);
	void Tree_UpdateParents(HTREEITEM hItem);

	int Tree_GetItemState(HTREEITEM hItem);
	int InMemoryTree_GetItemState(_ap242_node* pNode);
	
	void ResetView();
};

// ************************************************************************************************
enum class enumAP242ItemDataType : int
{
	Unknown = -1,
	Model,
	ProductDefinition,
	ProductShape,
	ProductShapeRepresentation,
	ProductShapeRepresentationItem,
	Assembly,
	ProductInstance,
	DraughtingModel,
	AnnotationPlane,
	DraughtingCallout,
};

// ************************************************************************************************
class CAP242ItemData
{

private: // Members	

	CAP242ItemData* m_pParent;	
	int64_t* m_pInstance; // Instance - C++ wrapper class
	enumAP242ItemDataType m_enAP242ItemDataType;
	HTREEITEM m_hItem;
	vector<CAP242ItemData*> m_vecChildren;

public: // Methods

	CAP242ItemData(CAP242ItemData* pParent, int64_t* pInstance, enumAP242ItemDataType enItemDataType);
	virtual ~CAP242ItemData();

public: // Properties

	CAP242ItemData* GetParent() const { return m_pParent; }
	vector<CAP242ItemData*>& Children() { return m_vecChildren; }
	enumAP242ItemDataType GetType() const { return m_enAP242ItemDataType; }
	HTREEITEM& TreeItem() { return m_hItem; }
	 
	template<typename T>
	T* GetInstance() const
	{
		return (T*)m_pInstance;
	}
};