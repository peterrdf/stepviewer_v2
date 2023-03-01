
#pragma once

// ------------------------------------------------------------------------------------------------
#include "ViewTree.h"
#include "STEPView.h"
#include "ViewTree.h"
#include "IFCUnit.h"
#include "IFCEntity.h"
#include "SearchSchemaDialog.h"

#include <map>
using namespace std;

// ------------------------------------------------------------------------------------------------
class CIFCModel;

// ------------------------------------------------------------------------------------------------
class CIFCSchemaViewToolBar : public CMFCToolBar
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
class CIFCSchemaView 
	: public CDockablePane
	, public CSTEPView
{

private: // Members

	CSearchSchemaDialog* m_pSearchDialog;

protected:

	// CSTEPView
	virtual void OnModelChanged() override;

private: // Methods	

	void LoadModel(CIFCModel* pModel);
	void LoadAttributes(CIFCEntity* pIFCEntity, HTREEITEM hParent);
	void LoadEntity(CIFCModel* pModel, CIFCEntity* pIFCEntity, HTREEITEM hParent);
	pair<int, int> GetInstancesCount(CIFCEntity* pEntity) const;

// Construction
public:
	CIFCSchemaView();

	void AdjustLayout();
	void OnChangeVisualStyle();

// Attributes
protected:

	CViewTree m_ifcTreeCtrl;
	CImageList m_imageList;
	CIFCSchemaViewToolBar m_wndToolBar;

protected:
	void ResetView();

// Implementation
public:
	virtual ~CIFCSchemaView();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnProperties();
	afx_msg void OnFileOpen();
	afx_msg void OnFileOpenWith();
	afx_msg void OnDummyCompile();
	afx_msg void OnEditCut();
	afx_msg void OnEditCopy();
	afx_msg void OnEditClear();
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnNMClickTreeIFC(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickTreeIFC(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnItemexpandingTreeIFC(NMHDR *pNMHDR, LRESULT *pResult);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};

