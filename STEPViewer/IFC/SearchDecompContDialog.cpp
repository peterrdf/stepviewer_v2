// SearchDecompContDialog.cpp : implementation file
//

#include "stdafx.h"
#include "SearchDecompContDialog.h"
#include "afxdialogex.h"
#include "resource.h"
#include "FileViewConsts.h"


// CSearchDecompContDialog dialog

// ------------------------------------------------------------------------------------------------
BOOL CSearchDecompContDialog::ContainsText(HTREEITEM hItem, const CString& strText)
{
	ASSERT(hItem != nullptr);

	CString strItemText = m_pIFCTreeCtrl->GetItemText(hItem);
	strItemText.MakeLower();

	CString strTextLower = strText;
	strTextLower.MakeLower();

	// Express line number
	if (m_enSearchFilter == enumSearchFilter::ExpressID)
	{
		CString strExpressionLine = L"#";
		strExpressionLine += strText;

		return strItemText.Find(strExpressionLine, 0) == 0;
	}	

	// All
	return strItemText.Find(strTextLower, 0) != -1;
}

// ------------------------------------------------------------------------------------------------
void CSearchDecompContDialog::SelectItem(HTREEITEM hItem)
{
	ASSERT(hItem != nullptr);

	// Unselect
	if (m_hSearchResult != nullptr)
	{
		UnselectItem(m_hSearchResult);
	}

	// Select
	m_pIFCTreeCtrl->EnsureVisible(hItem);
	m_pIFCTreeCtrl->SetItemState(hItem, TVIS_SELECTED, TVIS_SELECTED);
	m_pIFCTreeCtrl->SetFocus();
}

// ------------------------------------------------------------------------------------------------
void CSearchDecompContDialog::UnselectItem(HTREEITEM hItem)
{
	ASSERT(hItem != nullptr);

	m_pIFCTreeCtrl->SetItemState(hItem, 0, TVIS_SELECTED);
}

// ------------------------------------------------------------------------------------------------
HTREEITEM CSearchDecompContDialog::SearchChildren(HTREEITEM hParent)
{
	ASSERT(hParent != nullptr);	

	HTREEITEM hChild = m_pIFCTreeCtrl->GetNextItem(hParent, TVGN_CHILD);
	while (hChild != nullptr)
	{
		if (ContainsText(hChild, m_strSearchText))
		{
			return hChild;
		}

		HTREEITEM hGrandchild = SearchChildren(hChild);
		if (hGrandchild != nullptr)
		{
			return hGrandchild;
		}

		hChild = m_pIFCTreeCtrl->GetNextSiblingItem(hChild);
	} // while (hChild != nullptr)

	return nullptr;
}

// ------------------------------------------------------------------------------------------------
HTREEITEM CSearchDecompContDialog::SearchSiblings(HTREEITEM hItem)
{
	ASSERT(hItem != nullptr);

	HTREEITEM hSibling = m_pIFCTreeCtrl->GetNextSiblingItem(hItem);
	while (hSibling != nullptr)
	{
		if (ContainsText(hSibling, m_strSearchText))
		{
			return hSibling;
		}

		HTREEITEM hGrandchild = SearchChildren(hSibling);
		if (hGrandchild != nullptr)
		{
			return hGrandchild;
		}

		hSibling = m_pIFCTreeCtrl->GetNextSiblingItem(hSibling);
	} // while (hSibling != nullptr)

	return nullptr;
}

// ------------------------------------------------------------------------------------------------
HTREEITEM CSearchDecompContDialog::SearchParents(HTREEITEM hItem)
{
	ASSERT(hItem != nullptr);

	HTREEITEM hParent = m_pIFCTreeCtrl->GetParentItem(hItem);
	if (hParent == nullptr)
	{
		return nullptr;
	}

	HTREEITEM hSibling = m_pIFCTreeCtrl->GetNextSiblingItem(hParent);
	if (hSibling == nullptr)
	{
		return SearchParents(hParent);
	}

	// Children
	HTREEITEM hSearchResult = SearchChildren(hSibling);
	if (hSearchResult != nullptr)
	{
		return hSearchResult;
	}

	// Siblings and their children
	hSearchResult = SearchSiblings(hSibling);
	if (hSearchResult != nullptr)
	{
		return hSearchResult;
	}

	return SearchParents(hSibling);
}

// ------------------------------------------------------------------------------------------------
void CSearchDecompContDialog::Reset()
{
	m_hSearchResult = nullptr;
	m_bEndOfSearch = FALSE;
}

IMPLEMENT_DYNAMIC(CSearchDecompContDialog, CDialogEx)

CSearchDecompContDialog::CSearchDecompContDialog(CViewTree* pIFCTreeCtrl)
	: CDialogEx(IDD_DIALOG_SEARCH, nullptr)
	, m_pIFCTreeCtrl(pIFCTreeCtrl)
	, m_enSearchFilter(enumSearchFilter::All)
	, m_hSearchResult(nullptr)
	, m_bEndOfSearch(FALSE)
	, m_strSearchText(_T(""))
{
	ASSERT(m_pIFCTreeCtrl != nullptr);
}

CSearchDecompContDialog::~CSearchDecompContDialog()
{
}

void CSearchDecompContDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_SEARCH_TEXT, m_strSearchText);
	DDX_Control(pDX, IDC_BUTTON_SEARCH, m_btnSearch);
	DDX_Control(pDX, IDC_COMBO_SEARCH_FILTER, m_cmbSearchFilter);
}


BEGIN_MESSAGE_MAP(CSearchDecompContDialog, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT_SEARCH_TEXT, &CSearchDecompContDialog::OnEnChangeEditSearchText)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, &CSearchDecompContDialog::OnBnClickedButtonSearch)
	ON_CBN_SELCHANGE(IDC_COMBO_SEARCH_FILTER, &CSearchDecompContDialog::OnCbnSelchangeComboSearchFilter)
END_MESSAGE_MAP()


// CSearchDecompContDialog message handlers

// ------------------------------------------------------------------------------------------------
void CSearchDecompContDialog::OnEnChangeEditSearchText()
{
	UpdateData();

	m_btnSearch.EnableWindow(!m_strSearchText.IsEmpty());
}

// ------------------------------------------------------------------------------------------------
void CSearchDecompContDialog::OnBnClickedButtonSearch()
{
	UpdateData();

	// Reset
	if (m_bEndOfSearch)
	{
		ASSERT(m_hSearchResult != nullptr);

		UnselectItem(m_hSearchResult);

		m_hSearchResult = nullptr;
		m_bEndOfSearch = FALSE;
	}

	// Initialize - take the first root
	if (m_hSearchResult == nullptr)
	{
		m_hSearchResult = m_pIFCTreeCtrl->GetRootItem();
		if (m_hSearchResult == nullptr)
		{
			// No items
			return;
		}

		if (ContainsText(m_hSearchResult, m_strSearchText))
		{
			SelectItem(m_hSearchResult);

			return;
		}
	} // if (m_hSearchResult == nullptr)

	// Children
	HTREEITEM hSearchResult = SearchChildren(m_hSearchResult);
	if (hSearchResult != nullptr)
	{
		SelectItem(hSearchResult);

		m_hSearchResult = hSearchResult;

		return;
	}

	// Siblings and their children
	hSearchResult = SearchSiblings(m_hSearchResult);
	if (hSearchResult != nullptr)
	{
		SelectItem(hSearchResult);

		m_hSearchResult = hSearchResult;

		return;
	}

	// Parents, siblings and their children
	hSearchResult = SearchParents(m_hSearchResult);
	if (hSearchResult != nullptr)
	{
		SelectItem(hSearchResult);

		m_hSearchResult = hSearchResult;

		return;
	}

	m_bEndOfSearch = TRUE;

	::MessageBox(::AfxGetMainWnd()->GetSafeHwnd(), L"No more results found.", L"Search", MB_ICONINFORMATION | MB_OK);
}

// ------------------------------------------------------------------------------------------------
BOOL CSearchDecompContDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_cmbSearchFilter.AddString(_T("(All)"));
	m_cmbSearchFilter.AddString(_T("Express ID"));

	m_cmbSearchFilter.SetCurSel((int)m_enSearchFilter);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

// ------------------------------------------------------------------------------------------------
void CSearchDecompContDialog::OnCbnSelchangeComboSearchFilter()
{
	m_enSearchFilter = (enumSearchFilter)m_cmbSearchFilter.GetCurSel();
}
