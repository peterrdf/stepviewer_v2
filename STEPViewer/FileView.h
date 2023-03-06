
#pragma once

#include "ViewTree.h"
#include "STEPView.h"
#include "SearchInstancesDialog.h"
#include "Assembly.h"
#include "STEPItemData.h"
#include "STEPProductsTreeView.h"

#include <map>

using namespace std;


// ------------------------------------------------------------------------------------------------
class CFileViewToolBar : public CMFCToolBar
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
class CFileView
	: public CDockablePane
	, public CSTEPView
{

private: // Members

	CSTEPTreeViewBase* m_pSTEPTreeView;

public: // Methods

	// CSTEPView
	virtual void OnModelChanged();	

// Construction
public:
	CFileView();

	void AdjustLayout();
	void OnChangeVisualStyle();

// Attributes
protected:

	CViewTree m_modelStructureView;
	CFileViewToolBar m_wndToolBar;

protected:

// Implementation
public:
	virtual ~CFileView();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnProperties();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);	
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnNMClickTree(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnItemExpanding(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnNewInstance();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);	
};

