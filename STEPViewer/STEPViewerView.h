
// STEPViewerView.h : interface of the CMySTEPViewerView class
//

#include "OpenGLView.h"

#pragma once

class CMySTEPViewerView 
	: public CView
	, public CSTEPView
{

private: // Members

	// --------------------------------------------------------------------------------------------
	// OpenGL View
	COpenGLView* m_pOpenGLView;

public: // Methods

	// --------------------------------------------------------------------------------------------
	// CSTEPView
	virtual void OnModelChanged();

protected: // create from serialization only
	CMySTEPViewerView();
	DECLARE_DYNCREATE(CMySTEPViewerView)

// Attributes
public:
	CMySTEPViewerDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CMySTEPViewerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
};

#ifndef _DEBUG  // debug version in STEPViewerView.cpp
inline CMySTEPViewerDoc* CMySTEPViewerView::GetDocument() const
   { return reinterpret_cast<CMySTEPViewerDoc*>(m_pDocument); }
#endif

