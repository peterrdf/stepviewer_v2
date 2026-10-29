#pragma once

#include <vector>
#include <string>
#include <set>
using namespace std;

#include "TreeCtrlEx.h"

// ************************************************************************************************
class CSearchTreeCtrlDialogSite
{

public: // Methods

	// v1
	virtual CTreeCtrlEx* GetTreeView() PURE;
	virtual vector<CString> GetSearchFilters() PURE;
	virtual CString GetSearchFilterType(const CString& /*strFilter*/) { return L"Search"; } // Title
	virtual void LoadChildrenIfNeeded(HTREEITEM hItem) {};
	virtual BOOL ProcessSearch(int /*iFilter*/, const CString& /*strSearchText*/) { return FALSE; }
	virtual BOOL ContainsText(int /*iFilter*/, HTREEITEM /*hItem*/, const CString& /*strText*/) { return FALSE; }

	// v2
	virtual void SearchFor(const CString& /*strSearchText*/, int /*iFilter*/, vector<void*>& /*vecSearchResults*/, int /*iLimit*/) {}
	virtual HTREEITEM GetSearchResult(void* /*pSearchResult*/) { return nullptr; }
};

// ************************************************************************************************
class CSearchTreeCtrlDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CSearchTreeCtrlDialog)

private: // Members

	CSearchTreeCtrlDialogSite* m_pSite;

	HTREEITEM m_hSearchResult;
	BOOL m_bEndOfSearch;
	int m_iSearchDepth;

	vector<void*> m_vecSearchResults;

private: // Methods

	CTreeCtrlEx* GetTreeView() { return m_pSite->GetTreeView(); }

	void SelectItem(HTREEITEM hItem);
	void UnselectItem(HTREEITEM hItem);

	HTREEITEM SearchChildren(HTREEITEM hParent);
	HTREEITEM SearchSiblings(HTREEITEM hItem);
	HTREEITEM SearchParents(HTREEITEM hItem);

public: // Methods
	
	void Reset();

public:
	CSearchTreeCtrlDialog(CSearchTreeCtrlDialogSite* pSite);   // standard constructor
	virtual ~CSearchTreeCtrlDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SEARCH
	};
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_strSearchText;
	afx_msg void OnEnChangeEditSearchText();
	CButton m_btnCloseSearch;
	afx_msg void OnBnClickedButtonCloseSearch();
	CComboBox m_cmbSearchFilter;
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeComboSearchFilter();
	afx_msg void OnBnClickedOk();
};
