
// MainFrm.h : interface of the CMainFrame class
//

#pragma once

#include "Controller.h"
#include "FileView.h"
#include "PropertiesWnd.h"
#include "SchemaView.h"
#include "RelationsView.h"

class CMainFrame : public CFrameWndEx
{
	
private: // Methods

	// --------------------------------------------------------------------------------------------
	// Controller - MVC
	CController* GetController() const;


protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = nullptr, CCreateContext* pContext = nullptr);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CMFCMenuBar         m_wndMenuBar;
	CMFCToolBar         m_wndToolBar;
	CMFCStatusBar       m_wndStatusBar;
	CMFCToolBarImages   m_UserImages;
	CFileView           m_modelStructureView;
	CSchemaView			m_schemaView;
	CRelationsView		m_relationsView;
	CPropertiesWnd      m_propertiesView;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnViewCustomize();
	afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	BOOL CreateDockingWindows();
	void SetDockingWindowIcons(BOOL bHiColorIcons);
};


