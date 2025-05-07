#pragma once

#include "_rdf_property.h"

#include "TreeCtrlEx.h"
#include "SearchTreeCtrlDialog.h"

#include <map>
using namespace std;

// ************************************************************************************************
class CDesignTreeViewToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*)GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

// ************************************************************************************************
// Design Tree
// ************************************************************************************************
class CDesignTreeView
	: public CDockablePane
	, public _ap_view
	, public CSearchTreeCtrlDialogSite
{

private: // Classes

	// ********************************************************************************************
	enum class enumItemType : int
	{
		Unknown = 0,
		Instance = 1,
	};

	// ********************************************************************************************
	class CItemData
	{

	private: // Members

		OwlInstance m_owlInstance;
		enumItemType m_enItemType;
		vector<HTREEITEM> m_vecItems;

	public: // Methods

		CItemData(OwlInstance owlInstance, enumItemType enItemType)
			: m_owlInstance(owlInstance)
			, m_enItemType(enItemType)
			, m_vecItems()
		{
		}

		virtual ~CItemData() {}

	public: // Properties

		OwlInstance GetOwlInstance() const { return m_owlInstance; }
		enumItemType GetType() const { return m_enItemType; }
		vector<HTREEITEM>& Items() { return m_vecItems; }
	};

	// ********************************************************************************************
	class CInstanceData
		: public CItemData
	{

	public: // Methods

		CInstanceData(OwlInstance owlInstance)
			: CItemData(owlInstance, enumItemType::Instance)
		{
		}

		virtual ~CInstanceData() {}
	};

	// ********************************************************************************************
	enum class enumSearchFilter : int {
		All = 0,
		Instances,
		Properties,
		Values,
	};

private: // Members

	map<_ap_model*, vector<SdaiInstance>> m_mapSelectedInstances;
	_rdf_property_provider* m_pPropertyProvider;	
	map<OwlInstance, CInstanceData*> m_mapInstance2Data; // C INSTANCE : C++ INSTANCE
	bool m_bInitInProgress;
	CSearchTreeCtrlDialog* m_pSearchDialog;

public: // Methods

	// _view
	virtual void onModelLoaded() override;
	virtual void onInstanceSelected(_view* pSender) override;

	// CSearchTreeCtrlDialogSite
	virtual CTreeCtrlEx* GetTreeView() override;
	virtual vector<CString> GetSearchFilters() override;
	virtual void LoadChildrenIfNeeded(HTREEITEM hItem) override;
	virtual BOOL ContainsText(int iFilter, HTREEITEM hItem, const CString& strText) override;

private: // Methods

	void ResetView();
	void AddInstance(HTREEITEM hParent, OwlInstance owlInstance);
	void AddProperties(HTREEITEM hParent, OwlInstance owlInstance);

	void Clean();

	// Construction
public:
	CDesignTreeView();

	void AdjustLayout();
	void OnChangeVisualStyle();

	// Attributes
protected:

	CTreeCtrlEx m_treeCtrl;
	CImageList m_images;
	CDesignTreeViewToolBar m_toolBar;

protected:

	// Implementation
public:
	virtual ~CDesignTreeView();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnProperties();
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnItemExpanding(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};

