
#pragma once

#include "IFCView.h"
#include "IFCController.h"
#include "ViewTree.h"
#include "IFCUnit.h"
#include "IFCEntity.h"
#include "SearchSchemaDialog.h"

#include <map>
using namespace std;

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
	, public CIFCView
{

private: // Members

	// --------------------------------------------------------------------------------------------
	// Search
	CSearchSchemaDialog* m_pSearchDialog;

private: // Methods

	// --------------------------------------------------------------------------------------------
	// CIFCView
	virtual void OnModelLoadedEvent(CIFCModel* pModel);

	// --------------------------------------------------------------------------------------------
	// CIFCView
	virtual void OnActiveModelChangedEvent(const CIFCView * pSender);	

	// --------------------------------------------------------------------------------------------
	// CIFCView
	virtual void OnAllModelsDeleted();

	// --------------------------------------------------------------------------------------------
	// Load an IFC model
	void LoadModel(CIFCModel * pModel);

	// --------------------------------------------------------------------------------------------
	// Attributes
	void LoadAttributes(CIFCEntity * pIFCEntity, HTREEITEM hParent);

	// --------------------------------------------------------------------------------------------
	// Loads an IFC Entity
	void LoadEntity(CIFCEntity * pIFCEntity, HTREEITEM hParent);

	// --------------------------------------------------------------------------------------------
	// Count the Instances for this Entity and all Sub-types
	pair<int, int> GetInstancesCount(CIFCEntity * pEntity) const;

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

