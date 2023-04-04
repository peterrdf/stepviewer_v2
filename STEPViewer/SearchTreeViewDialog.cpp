// SearchTreeViewDialog.cpp : implementation file
//

#include "stdafx.h"
#include "SearchTreeViewDialog.h"
#include "afxdialogex.h"
#include "resource.h"


// CSearchTreeViewDialog dialog

// ------------------------------------------------------------------------------------------------
void CSearchTreeViewDialog::SelectItem(HTREEITEM hItem)
{
	ASSERT(hItem != nullptr);

	// Unselect
	if (m_hSearchResult != nullptr)
	{
		UnselectItem(m_hSearchResult);
	}

	// Select
	GetTreeView()->EnsureVisible(hItem);
	GetTreeView()->SetItemState(hItem, TVIS_SELECTED, TVIS_SELECTED);
	GetTreeView()->SetFocus();
}

// ------------------------------------------------------------------------------------------------
void CSearchTreeViewDialog::UnselectItem(HTREEITEM hItem)
{
	ASSERT(hItem != nullptr);

	GetTreeView()->SetItemState(hItem, 0, TVIS_SELECTED);
}

// ------------------------------------------------------------------------------------------------
HTREEITEM CSearchTreeViewDialog::SearchChildren(HTREEITEM hParent)
{
	ASSERT(hParent != nullptr);	

	m_pSite->LoadChildrenIfNeeded(hParent);

	HTREEITEM hChild = GetTreeView()->GetNextItem(hParent, TVGN_CHILD);
	while (hChild != nullptr)
	{
		if (m_pSite->ContainsText(m_cmbSearchFilter.GetCurSel(), hChild, m_strSearchText))
		{
			return hChild;
		}

		HTREEITEM hGrandchild = SearchChildren(hChild);
		if (hGrandchild != nullptr)
		{
			return hGrandchild;
		}

		hChild = GetTreeView()->GetNextSiblingItem(hChild);
	} // while (hChild != nullptr)

	return nullptr;
}

// ------------------------------------------------------------------------------------------------
HTREEITEM CSearchTreeViewDialog::SearchSiblings(HTREEITEM hItem)
{
	ASSERT(hItem != nullptr);

	HTREEITEM hSibling = GetTreeView()->GetNextSiblingItem(hItem);
	while (hSibling != nullptr)
	{
		if (m_pSite->ContainsText(m_cmbSearchFilter.GetCurSel(), hSibling, m_strSearchText))
		{
			return hSibling;
		}

		HTREEITEM hGrandchild = SearchChildren(hSibling);
		if (hGrandchild != nullptr)
		{
			return hGrandchild;
		}

		hSibling = GetTreeView()->GetNextSiblingItem(hSibling);
	} // while (hSibling != nullptr)

	return nullptr;
}

// ------------------------------------------------------------------------------------------------
HTREEITEM CSearchTreeViewDialog::SearchParents(HTREEITEM hItem)
{
	ASSERT(hItem != nullptr);

	HTREEITEM hParent = GetTreeView()->GetParentItem(hItem);
	if (hParent == nullptr)
	{
		return nullptr;
	}

	HTREEITEM hSibling = GetTreeView()->GetNextSiblingItem(hParent);
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
void CSearchTreeViewDialog::Reset()
{
	m_hSearchResult = nullptr;
	m_bEndOfSearch = FALSE;
}

IMPLEMENT_DYNAMIC(CSearchTreeViewDialog, CDialogEx)

CSearchTreeViewDialog::CSearchTreeViewDialog(CSearchTreeViewDialogSite* pSite)
	: CDialogEx(IDD_DIALOG_SEARCH, nullptr)
	, m_pSite(pSite)
	, m_hSearchResult(nullptr)
	, m_bEndOfSearch(FALSE)
	, m_strSearchText(_T(""))
{
	ASSERT(m_pSite != nullptr);
}

CSearchTreeViewDialog::~CSearchTreeViewDialog()
{
}

void CSearchTreeViewDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_SEARCH_TEXT, m_strSearchText);
	DDX_Control(pDX, IDC_BUTTON_SEARCH, m_btnSearch);
	DDX_Control(pDX, IDC_COMBO_SEARCH_FILTER, m_cmbSearchFilter);
}


BEGIN_MESSAGE_MAP(CSearchTreeViewDialog, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT_SEARCH_TEXT, &CSearchTreeViewDialog::OnEnChangeEditSearchText)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, &CSearchTreeViewDialog::OnBnClickedButtonSearch)
END_MESSAGE_MAP()


// CSearchTreeViewDialog message handlers

// ------------------------------------------------------------------------------------------------
void CSearchTreeViewDialog::OnEnChangeEditSearchText()
{
	UpdateData();

	m_btnSearch.EnableWindow(!m_strSearchText.IsEmpty());
}

// ------------------------------------------------------------------------------------------------
void CSearchTreeViewDialog::OnBnClickedButtonSearch()
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
		m_hSearchResult = GetTreeView()->GetRootItem();
		if (m_hSearchResult == nullptr)
		{
			// No items
			return;
		}

		if (m_pSite->ContainsText(m_cmbSearchFilter.GetCurSel(), m_hSearchResult, m_strSearchText))
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
BOOL CSearchTreeViewDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	auto vecSearchFilters = m_pSite->GetSearchFilters();
	ASSERT(!vecSearchFilters.empty());

	for (auto searchFilter : vecSearchFilters)
	{
		m_cmbSearchFilter.AddString(searchFilter.c_str());
	}

	m_cmbSearchFilter.SetCurSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}
