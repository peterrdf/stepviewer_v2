
#pragma once

#include "ViewTree.h"
#include "STEPView.h"
#include "Instance.h"
#include "OWLProperty.h"
#include "SearchInstancesDialog.h"

#include <map>

using namespace std;

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class CDesignTreeViewToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class CDesignTreeView
	: public CDockablePane
	, public CSTEPView
{

private: // Classes

	// ------------------------------------------------------------------------------------------------
	enum class enumItemType : int
	{
		Unknown = 0,
		Instance = 1,
	};

	// ------------------------------------------------------------------------------------------------
	class CItemData
	{

	private: // Methods
		
		int64_t m_iInstance;
		enumItemType m_enItemType;
		vector<HTREEITEM> m_vecItems;

	public: // Members
		
		CItemData(int64_t iInstance, enumItemType enItemType)
			: m_iInstance(iInstance)
			, m_enItemType(enItemType)
			, m_vecItems()
		{}

		virtual ~CItemData() {}

		int64_t GetInstance() const { return m_iInstance; }
		enumItemType GetType() const { return m_enItemType; }
		vector<HTREEITEM>& Items() { return m_vecItems; }
	};

	// ------------------------------------------------------------------------------------------------
	class CInstanceData
		: public CItemData
	{

	public: // Methods

		CInstanceData(int64_t iInstance)
			: CItemData(iInstance, enumItemType::Instance)
		{}

		virtual ~CInstanceData() {}
	};

private: // Members
	
	COWLPropertyProvider* m_pPropertyProvider;
	map<int64_t, CInstanceData*> m_mapInstance2Item; // C INSTANCE : C++ INSTANCE
	bool m_bInitInProgress;
	//CSearchInstancesDialog* m_pSearchDialog;

public: // Methods
	
	// CSTEPView
	virtual void OnModelChanged() override;
	virtual void OnInstanceSelected(CSTEPView* pSender) override;

private: // Methods

	void ResetView();	
	void AddInstance(HTREEITEM hParent, int64_t iInstance);
	void AddProperties(HTREEITEM hParent, int64_t iInstance);

	void Clean();

// Construction
public:
	CDesignTreeView();

	void AdjustLayout();
	void OnChangeVisualStyle();

// Attributes
protected:

	CViewTree m_treeCtrl;
	CImageList m_images;
	CDesignTreeViewToolBar m_toolBar;

protected:

// Implementation
public:
	virtual ~CDesignTreeView();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSelectedItemChanged(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnItemExpanding(NMHDR * pNMHDR, LRESULT * pResult);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};

