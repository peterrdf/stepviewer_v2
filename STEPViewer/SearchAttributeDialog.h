#pragma once

#include "ViewTree.h"
#include "Controller.h"

// CSearchAttributeDialog dialog

class CSearchAttributeDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CSearchAttributeDialog)

private: // Declarations

	enum class enumSearchFilter : int {
		All = 0,
		ExpressID,
	};

private: // Members
	
	CController* m_pController;
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
	CSearchAttributeDialog(CController* pController, CViewTree* pTreeCtrl);   // standard constructor
	virtual ~CSearchAttributeDialog();

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
