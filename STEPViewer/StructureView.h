
#pragma once

#include "TreeCtrlEx.h"
#include "ViewBase.h"
#include "Assembly.h"
#include "STEPItemData.h"
#include "STEPModelStructureView.h"

#include <map>

using namespace std;


// ------------------------------------------------------------------------------------------------
class CStructureViewToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

// ************************************************************************************************
// Model Structure
// ************************************************************************************************

// ------------------------------------------------------------------------------------------------
class CStructureView
	: public CDockablePane
	, public CViewBase
{

private: // Members

	CTreeViewBase* m_pSTEPTreeView;

public: // Methods

	// CViewBase
	virtual void OnModelChanged();	

// Construction
public:
	CStructureView();

	void AdjustLayout();
	void OnChangeVisualStyle();

// Attributes
protected:

	CTreeCtrlEx m_treeCtrl;
	CStructureViewToolBar m_toolBar;

protected:

// Implementation
public:
	virtual ~CStructureView();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnProperties();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);	
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnNMClickTree(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnItemExpanding(NMHDR * pNMHDR, LRESULT * pResult);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);	
};

