#pragma once

#include "ViewTree.h"

// CSearchSchemaDialog dialog

class CSearchSchemaDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CSearchSchemaDialog)

private: // Declarations

	enum enumSearchWhere {
		swAll = 0,
		swEntities = 1,
		swAttributes = 2
	};

private: // Members

	// --------------------------------------------------------------------------------------------
	// IFC Tree
	CViewTree * m_pIFCTreeCtrl;

	// --------------------------------------------------------------------------------------------
	// Filter
	enumSearchWhere m_enSearchWhere;

	// --------------------------------------------------------------------------------------------
	// Last found HTREEITEM
	HTREEITEM m_hSearchResult;

	// --------------------------------------------------------------------------------------------
	// No more results
	BOOL m_bEndOfSearch;

private: // Methods

	// --------------------------------------------------------------------------------------------
	// Search
	BOOL ContainsText(HTREEITEM hItem, const CString& strText);

	// --------------------------------------------------------------------------------------------
	// Select
	void SelectItem(HTREEITEM hItem);

	// --------------------------------------------------------------------------------------------
	// Select
	void UnselectItem(HTREEITEM hItem);

	// --------------------------------------------------------------------------------------------
	// Search the item's children
	HTREEITEM SearchChildren(HTREEITEM hParent);

	// --------------------------------------------------------------------------------------------
	// Search the item's siblings
	HTREEITEM SearchSiblings(HTREEITEM hItem);

	// --------------------------------------------------------------------------------------------
	// Search the item's parents
	HTREEITEM SearchParents(HTREEITEM hItem);

public: // Methods

	// --------------------------------------------------------------------------------------------
	// Resets internal state
	void Reset();

public:
	CSearchSchemaDialog(CViewTree* pIFCTreeCtrl);   // standard constructor
	virtual ~CSearchSchemaDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SEARCH_SCHEMA };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_strSearchText;
	afx_msg void OnEnChangeEditSearchText();
	CButton m_btnSearch;
	afx_msg void OnBnClickedButtonSearch();
	CComboBox m_cmbSearchWhere;
	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelchangeComboSearchFilter();
};
