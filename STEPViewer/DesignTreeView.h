
#pragma once

// ------------------------------------------------------------------------------------------------
#include "ViewTree.h"
#include "STEPView.h"
#include "ViewTree.h"
#include "SearchSchemaDialog.h"

#include <map>
using namespace std;

// ------------------------------------------------------------------------------------------------
class CDesignTreeViewToolBar : public CMFCToolBar
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
class CDesignTreeView 
	: public CDockablePane
	, public CSTEPView
{

private: // Members

	// Search
	CSearchSchemaDialog* m_pSearchDialog;

protected:

	// CSTEPView
	virtual void OnModelChanged() override;

private: // Methods	

	void LoadModel(CModel* pModel);
	void LoadAttributes(CEntity* pEntity, HTREEITEM hParent);
	void LoadEntity(CEntity* pEntity, HTREEITEM hParent);
	pair<int, int> GetInstancesCount(CEntity* pEntity) const;

// Construction
public:
	CDesignTreeView();

	void AdjustLayout();
	void OnChangeVisualStyle();

// Attributes
protected:

	CViewTree m_treeCtrl;
	CImageList m_imageList;
	CDesignTreeViewToolBar m_toolBar;

protected:
	void ResetView();

// Implementation
public:
	virtual ~CDesignTreeView();

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

