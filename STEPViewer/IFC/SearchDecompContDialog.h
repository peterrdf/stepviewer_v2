#pragma once

#include "ViewTree.h"

// CSearchDecompContDialog dialog

class CSearchDecompContDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CSearchDecompContDialog)

private: // Declarations

	enum class enumSearchFilter {
		All = 0,
		ExpressID = 1,
	};

private: // Members
	
	CViewTree* m_pIFCTreeCtrl;
	enumSearchFilter m_enSearchFilter;
	HTREEITEM m_hSearchResult;
	BOOL m_bEndOfSearch;

private: // Methods
	
	// Search/Select
	BOOL ContainsText(HTREEITEM hItem, const CString& strText);
	void SelectItem(HTREEITEM hItem);
	void UnselectItem(HTREEITEM hItem);
	HTREEITEM SearchChildren(HTREEITEM hParent);
	HTREEITEM SearchSiblings(HTREEITEM hItem);
	HTREEITEM SearchParents(HTREEITEM hItem);

public: // Methods
	
	// Resets internal state
	void Reset();

public:
	CSearchDecompContDialog(CViewTree* pIFCTreeCtrl);   // standard constructor
	virtual ~CSearchDecompContDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SEARCH_DECOMP_CONT };
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
	afx_msg void OnCbnSelchangeComboSearchFilter();
};
