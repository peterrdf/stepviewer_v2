
#pragma once

// ------------------------------------------------------------------------------------------------
#include "TreeCtrlEx.h"
#include "ViewBase.h"
#include "TreeCtrlEx.h"
#include "SearchTreeCtrlDialog.h"

#include <map>
using namespace std;

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class CSchemaViewToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

/**************************************************************************************************
* Schema
*/

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class CSchemaView 
	: public CDockablePane
	, public CViewBase
	, public CSearchTreeCtrlDialogSite
{

private: // Classes

	enum class enumSearchFilter {
		All = 0,
		Entities = 1,
		Attributes = 2
	};

private: // Members

	// Search
	CSearchTreeCtrlDialog* m_pSearchDialog;

protected:

	// CViewBase
	virtual void OnModelChanged() override;

	// CSearchTreeCtrlDialogSite
	virtual CTreeCtrlEx* GetTreeView() override;
	virtual vector<CString> GetSearchFilters() override;
	virtual void LoadChildrenIfNeeded(HTREEITEM hItem) override;
	virtual BOOL ContainsText(int iFilter, HTREEITEM hItem, const CString& strText) override;

private: // Methods	

	void LoadModel(CModel* pModel);
	void LoadAttributes(CEntity* pEntity, HTREEITEM hParent);
	void LoadEntity(CEntity* pEntity, HTREEITEM hParent);
	pair<int, int> GetInstancesCount(CEntity* pEntity) const;

// Construction
public:
	CSchemaView();

	void AdjustLayout();
	void OnChangeVisualStyle();

// Attributes
protected:

	CTreeCtrlEx m_treeCtrl;
	CImageList m_imageList;
	CSchemaViewToolBar m_toolBar;

protected:
	void ResetView();

// Implementation
public:
	virtual ~CSchemaView();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnProperties();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnNMClickTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTVNItemexpandingTree(NMHDR *pNMHDR, LRESULT *pResult);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};

