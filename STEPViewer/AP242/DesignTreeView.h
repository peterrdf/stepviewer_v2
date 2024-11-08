
#pragma once

#include "TreeCtrlEx.h"
#include "ViewBase.h"
#include "InstanceBase.h"
#include "OWLProperty.h"

#include <map>

using namespace std;

// ************************************************************************************************
class CDesignTreeViewToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

// ************************************************************************************************
class CDesignTreeView
	: public CDockablePane
	, public CViewBase
	, public CSearchTreeCtrlDialogSite
{

private: // Classes

	enum class enumItemType : int
	{
		Unknown = 0,
		Instance = 1,
	};

	class CItemData
	{

	private: // Methods
		
		OwlInstance m_iInstance;
		enumItemType m_enItemType;
		vector<HTREEITEM> m_vecItems;

	public: // Members
		
		CItemData(OwlInstance iInstance, enumItemType enItemType)
			: m_iInstance(iInstance)
			, m_enItemType(enItemType)
			, m_vecItems()
		{}

		virtual ~CItemData() {}

		OwlInstance GetInstance() const { return m_iInstance; }
		enumItemType GetType() const { return m_enItemType; }
		vector<HTREEITEM>& Items() { return m_vecItems; }
	};

	class CInstanceData
		: public CItemData
	{

	public: // Methods

		CInstanceData(OwlInstance iInstance)
			: CItemData(iInstance, enumItemType::Instance)
		{}

		virtual ~CInstanceData() {}
	};

	enum class enumSearchFilter : int {
		All = 0,
		Instances,
		Properties,
		Values,
	};

private: // Members
	
	COWLPropertyProvider* m_pPropertyProvider;
	map<OwlInstance, CInstanceData*> m_mapInstance2Data; // C INSTANCE : C++ INSTANCE
	bool m_bInitInProgress;
	CSearchTreeCtrlDialog* m_pSearchDialog;

public: // Methods
	
	// CViewBase
	virtual void OnModelChanged() override;
	virtual void OnInstanceSelected(CViewBase* pSender) override;

	// CSearchTreeCtrlDialogSite
	virtual CTreeCtrlEx* GetTreeView() override;
	virtual vector<CString> GetSearchFilters() override;
	virtual void LoadChildrenIfNeeded(HTREEITEM hItem) override;
	virtual BOOL ContainsText(int iFilter, HTREEITEM hItem, const CString& strText) override;

private: // Methods

	void ResetView();	
	void AddInstance(HTREEITEM hParent, OwlInstance iInstance);
	void AddProperties(HTREEITEM hParent, OwlInstance iInstance);

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
	afx_msg void OnItemExpanding(NMHDR * pNMHDR, LRESULT * pResult);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};

