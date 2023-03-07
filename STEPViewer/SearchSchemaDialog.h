#pragma once

#include "ViewTree.h"

// CSearchSchemaDialog dialog

class CSearchSchemaDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CSearchSchemaDialog)

private: // Declarations

	enum class enumSearchFilter {
		All = 0,
		Entities = 1,
		Attributes = 2
	};

private: // Members
	
	CViewTree* m_pTreeCtrl;	
	enumSearchFilter m_enSearchFilter;
	HTREEITEM m_hSearchResult;
	BOOL m_bEndOfSearch;

private: // Methods
	
	// Search
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
	CSearchSchemaDialog(CViewTree* pTreeCtrl);   // standard constructor
	virtual ~CSearchSchemaDialog();

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
	afx_msg void OnCbnSelchangeComboSearchFilter();
};
