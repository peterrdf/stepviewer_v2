#pragma once

#include "_ifc_instance.h"

#include "ModelStructureViewBase.h"
#include "SearchTreeCtrlDialog.h"

#include <map>
#include <set>
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

	// ********************************************************************************************
	typedef map<_ifc_instance*, vector<HTREEITEM>> ITEMS;

	// ********************************************************************************************
	class CModelData
	{

	private: // Members

		_ifc_model* m_pModel;

		CTreeCtrlEx* m_pTreeCtrl;
		HTREEITEM m_hModel;
		HTREEITEM m_hProject;
		HTREEITEM m_hGroups;
		HTREEITEM m_hSpaceBoundaries;
		HTREEITEM m_hUnreferenced;

		ITEMS m_mapModel;
		ITEMS m_mapProject;
		ITEMS m_mapGroups;
		ITEMS m_mapSpaceBoundaries;
		ITEMS m_mapUnreferenced;

	public: // Methods

		CModelData(_ifc_model* pModel, CTreeCtrlEx* pTreeCtrl, HTREEITEM hModel);
		virtual ~CModelData();

		bool IsProjectItem(HTREEITEM hItem);
		bool IsGroupsItem(HTREEITEM hItem);
		bool IsSpaceBoundariesItem(HTREEITEM hItem);
		bool IsUnreferencedItem(HTREEITEM hItem);

	public: // Properties

		_ifc_model* GetModel() const { return m_pModel; }

		HTREEITEM GetModelItem() const;
		void SetProjectItem(HTREEITEM hItem);
		HTREEITEM GetProjectItem() const;
		void SetGroupsItem(HTREEITEM hItem);
		HTREEITEM GetGroupsItem() const;
		void SetSpaceBoundariesItem(HTREEITEM hItem);
		HTREEITEM GetSpaceBoundariesItem() const;
		void SetUnreferencedItem(HTREEITEM hItem);
		HTREEITEM GetUnreferencedItem() const;

		ITEMS& GetProjectItems() { return m_mapProject; }
		ITEMS& GetGroupsItems() { return m_mapGroups; }
		ITEMS& GetSpaceBoundariesItems() { return m_mapSpaceBoundaries; }
		ITEMS& GetUnreferencedItems() { return m_mapUnreferenced; }

		ITEMS& GetItems(HTREEITEM hItem);
	};

	// ********************************************************************************************
	enum class enumSearchFilter : int {
		All = 0,
		ExpressID = 1,
	};

private: // Members	
	
	CImageList* m_pImageList;

	// Cache
	vector<CModelData*> m_vecModelData;
	_ifc_instance* m_pSelectedInstance;
	
	// Search
	CSearchTreeCtrlDialog* m_pSearchDialog;

public: // Methods

	CIFCModelStructureView(CTreeCtrlEx* pTreeCtrl);
	virtual ~CIFCModelStructureView();

	// _view
	virtual void onInstanceEnabledStateChanged(_view* pSender, _instance* pInstance, int iFlag) override;
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
	void LoadProject(CModelData* pModelData, HTREEITEM hModel, SdaiInstance sdaiProjectInstance, ITEMS& mapItems);
	void LoadIsDecomposedBy(_ifc_model* pModel, SdaiInstance sdaiInstance, HTREEITEM hParent, ITEMS& mapItems);
	void LoadIsNestedBy(_ifc_model* pModel, SdaiInstance sdaiInstance, HTREEITEM hParent, ITEMS& mapItems);
	void LoadContainsElements(_ifc_model* pModel, SdaiInstance sdaiInstance, HTREEITEM hParent, ITEMS& mapItems);
	void LoadBoundedBy(_ifc_model* pModel, SdaiInstance sdaiInstance, HTREEITEM hParent, ITEMS& mapItems);
	HTREEITEM LoadInstance(_ifc_model* pModel, SdaiInstance sdaiInstance, HTREEITEM hParent, ITEMS& mapItems, bool bLoadChildren = true);
	void LoadGroups(CModelData* pModelData, HTREEITEM hModel, ITEMS& mapItems);
	void LoadSpaceBoundaries(CModelData* pModelData, HTREEITEM hModel, ITEMS& mapItems);
	void LoadBuildingStoreyChildren(_ifc_model* pModel, SdaiInstance sdaiInstance, HTREEITEM hBuildingStorey, ITEMS& mapItems);
	void LoadUnreferencedItems(CModelData* pModelData, HTREEITEM hModel, ITEMS& mapItems);

	CModelData* Model_GetData(HTREEITEM hItem);
	CModelData* Model_GetData(_model* pModel);
	void Model_EnableChildren(HTREEITEM hItem, bool bEnable, set<_ifc_instance*>& setChildren);	

	HTREEITEM Tree_GetModelItem(HTREEITEM hItem) const;

	void Tree_Update(HTREEITEM hItem, bool bRecursive = true);
	void Tree_Update(HTREEITEM hModel, HTREEITEM hItem, ITEMS& mapItems, const set<_ifc_instance*>& setInstances);
	void Tree_UpdateChildren(HTREEITEM hItem);
	void Tree_UpdateParents(HTREEITEM hItem);
	void Tree_Reset(HTREEITEM hItem, bool bEnable);
	void Tree_Select(bool bEnable);
	void Tree_Select(_ifc_instance* pInstance, ITEMS& mapItems, bool bEnable);
	bool Tree_EnsureVisible(CModelData* pModelData, _ifc_instance* pInstance);
	bool Tree_EnsureVisible(_ifc_instance* pInstance, ITEMS& mapItems);

	void ResetView();
};

