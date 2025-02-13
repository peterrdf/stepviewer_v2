
// STEPViewerDoc.h : interface of the CMySTEPViewerDoc class
//


#pragma once

#include "Controller.h"
#include "ModelCheckDlg.h"
#include "BCF\BCFView.h"

// ------------------------------------------------------------------------------------------------
class CMySTEPViewerDoc
	: public CDocument
	, public CController
{

public: // Methods

	// _controller
	virtual void saveInstance(_instance* pInstance) override;

	void OpenModels(vector<CString>& vecPaths);

protected: // create from serialization only
	CMySTEPViewerDoc();
	DECLARE_DYNCREATE(CMySTEPViewerDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual void DeleteContents();
	virtual void SetTitle(LPCTSTR lpszTitle);

#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CMySTEPViewerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);

protected:
	CModelCheckDlg		m_wndModelChecker;
	CBCFView            m_wndBCFView;

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnFileOpen();
	afx_msg void OnViewZoomOut();
	afx_msg void OnViewModelChecker();
	afx_msg void OnUpdateViewModelChecker(CCmdUI* pCmdUI);
	afx_msg void OnFileNew();

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
};
