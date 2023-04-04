#pragma once

#include <vector>
#include <string>

using namespace std;

#include "ViewTree.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class CSearchTreeViewDialogSite
{

public: // Methods

	virtual CViewTree* GetTreeView() PURE;
	virtual vector<wstring> GetSearchFilters() PURE;
	virtual void LoadChildrenIfNeeded(HTREEITEM hItem) PURE;
	virtual BOOL ProcessSearch(int /*iFilter*/, const CString& /*strSearchText*/) { return FALSE; }
	virtual BOOL ContainsText(int iFilter, HTREEITEM hItem, const CString& strText) PURE;
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class CSearchTreeViewDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CSearchTreeViewDialog)

private: // Members

	CSearchTreeViewDialogSite* m_pSite;

	HTREEITEM m_hSearchResult;
	BOOL m_bEndOfSearch;

private: // Methods

	CViewTree* GetTreeView() { return m_pSite->GetTreeView(); }

	void SelectItem(HTREEITEM hItem);
	void UnselectItem(HTREEITEM hItem);

	HTREEITEM SearchChildren(HTREEITEM hParent);
	HTREEITEM SearchSiblings(HTREEITEM hItem);
	HTREEITEM SearchParents(HTREEITEM hItem);

public: // Methods
	
	void Reset();

public:
	CSearchTreeViewDialog(CSearchTreeViewDialogSite* pSite);   // standard constructor
	virtual ~CSearchTreeViewDialog();

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
	CButton m_btnSearch;
	afx_msg void OnBnClickedButtonSearch();
	CComboBox m_cmbSearchFilter;
	virtual BOOL OnInitDialog();
};
