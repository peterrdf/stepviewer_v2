// SearchTreeCtrlDialog.cpp : implementation file
//

#include "stdafx.h"
#include "SearchTreeCtrlDialog.h"
#include "afxdialogex.h"
#include "resource.h"


// CSearchTreeCtrlDialog dialog

// ************************************************************************************************
const int MAX_SEARCH_DEPTH = 20;

// ************************************************************************************************
void CSearchTreeCtrlDialog::SelectItem(HTREEITEM hItem)
{
	ASSERT(hItem != nullptr);

	// Unselect
	if (m_hSearchResult != nullptr) {
		UnselectItem(m_hSearchResult);
	}

	// Select
	GetTreeView()->EnsureVisible(hItem);
	GetTreeView()->SetItemState(hItem, TVIS_SELECTED, TVIS_SELECTED);
	GetTreeView()->SetFocus();
}

void CSearchTreeCtrlDialog::UnselectItem(HTREEITEM hItem)
{
	ASSERT(hItem != nullptr);

	GetTreeView()->SetItemState(hItem, 0, TVIS_SELECTED);
}

HTREEITEM CSearchTreeCtrlDialog::SearchChildren(HTREEITEM hParent)
{
	ASSERT(hParent != nullptr);

	m_iSearchDepth++;
	ASSERT(m_iSearchDepth <= MAX_SEARCH_DEPTH);

	m_pSite->LoadChildrenIfNeeded(hParent);

	HTREEITEM hChild = GetTreeView()->GetNextItem(hParent, TVGN_CHILD);
	while (hChild != nullptr) {
		if (m_pSite->ContainsText(m_cmbSearchFilter.GetCurSel(), hChild, m_strSearchText)) {
			return hChild;
		}

		if (m_iSearchDepth + 1 < MAX_SEARCH_DEPTH) {
			HTREEITEM hSearchResult = SearchChildren(hChild);
			if (hSearchResult != nullptr) {
				return hSearchResult;
			}
		}

		hChild = GetTreeView()->GetNextSiblingItem(hChild);
	} // while (hChild != nullptr)

	m_iSearchDepth--;
	//ASSERT(m_iSearchDepth > 0);#todo

	return nullptr;
}

HTREEITEM CSearchTreeCtrlDialog::SearchSiblings(HTREEITEM hItem)
{
	ASSERT(hItem != nullptr);

	HTREEITEM hSibling = GetTreeView()->GetNextSiblingItem(hItem);
	while (hSibling != nullptr) {
		if (m_pSite->ContainsText(m_cmbSearchFilter.GetCurSel(), hSibling, m_strSearchText)) {
			return hSibling;
		}

		if (m_iSearchDepth + 1 < MAX_SEARCH_DEPTH) {
			HTREEITEM hSearchResult = SearchChildren(hSibling);
			if (hSearchResult != nullptr) {
				return hSearchResult;
			}
		}

		hSibling = GetTreeView()->GetNextSiblingItem(hSibling);
	} // while (hSibling != nullptr)

	return nullptr;
}

HTREEITEM CSearchTreeCtrlDialog::SearchParents(HTREEITEM hItem)
{
	ASSERT(hItem != nullptr);

	HTREEITEM hParent = GetTreeView()->GetParentItem(hItem);
	if (hParent == nullptr) {
		return nullptr;
	}

	m_iSearchDepth--;
	ASSERT(m_iSearchDepth >= 0);

	HTREEITEM hSearchResult = SearchSiblings(hParent);
	if (hSearchResult != nullptr) {
		return hSearchResult;
	}

	return SearchParents(hParent);
}

void CSearchTreeCtrlDialog::Reset()
{
	if (m_hSearchResult != nullptr) {
		UnselectItem(m_hSearchResult);
		m_hSearchResult = nullptr;
	}	
	m_bEndOfSearch = FALSE;
	m_iSearchDepth = 0;
	m_vecSearchResults.clear();
}

IMPLEMENT_DYNAMIC(CSearchTreeCtrlDialog, CDialogEx)

CSearchTreeCtrlDialog::CSearchTreeCtrlDialog(CSearchTreeCtrlDialogSite* pSite)
	: CDialogEx(IDD_DIALOG_SEARCH, nullptr)
	, m_pSite(pSite)
	, m_hSearchResult(nullptr)
	, m_bEndOfSearch(FALSE)
	, m_iSearchDepth(0)
	, m_vecSearchResults()
	, m_strSearchText(_T(""))
{
	ASSERT(m_pSite != nullptr);
}

CSearchTreeCtrlDialog::~CSearchTreeCtrlDialog()
{
}

void CSearchTreeCtrlDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_SEARCH_TEXT, m_strSearchText);
	DDX_Control(pDX, IDC_BUTTON_SEARCH, m_btnSearch);
	DDX_Control(pDX, IDC_COMBO_SEARCH_FILTER, m_cmbSearchFilter);
}


BEGIN_MESSAGE_MAP(CSearchTreeCtrlDialog, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT_SEARCH_TEXT, &CSearchTreeCtrlDialog::OnEnChangeEditSearchText)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, &CSearchTreeCtrlDialog::OnBnClickedButtonSearch)
	ON_CBN_SELCHANGE(IDC_COMBO_SEARCH_FILTER, &CSearchTreeCtrlDialog::OnSelchangeComboSearchFilter)
END_MESSAGE_MAP()


// CSearchTreeCtrlDialog message handlers

void CSearchTreeCtrlDialog::OnEnChangeEditSearchText()
{
	UpdateData();

	Reset();

	m_btnSearch.EnableWindow(!m_strSearchText.IsEmpty());
}

void CSearchTreeCtrlDialog::OnBnClickedButtonSearch()
{
	UpdateData();

	if (m_strSearchText.IsEmpty()) {
		return;
	}

	//
	// v2
	//

	if (m_vecSearchResults.empty()) {
		m_pSite->SearchFor(m_strSearchText, m_cmbSearchFilter.GetCurSel(), m_vecSearchResults, 1000);
	}

	if (!m_vecSearchResults.empty()) {
		void* pSearchResult = m_vecSearchResults.front();
		m_vecSearchResults.erase(m_vecSearchResults.begin());

		if (m_hSearchResult != nullptr) {
			UnselectItem(m_hSearchResult);
		}

		m_hSearchResult = m_pSite->GetSearchResult(pSearchResult);
		ASSERT(m_hSearchResult != nullptr);
		SelectItem(m_hSearchResult);
		
		/*if (m_vecSearchResults.empty()) {
			::MessageBox(::AfxGetMainWnd()->GetSafeHwnd(), L"No more results found.", L"Search", MB_ICONINFORMATION | MB_OK);
		}*/

		return;
	}

	//
	// v1
	//

	if (m_pSite->ProcessSearch(m_cmbSearchFilter.GetCurSel(), m_strSearchText)) {
		return;
	}

	// Reset
	if (m_bEndOfSearch) {
		if (m_hSearchResult != nullptr) {
			UnselectItem(m_hSearchResult);
		}

		Reset();
	}

	// Initialize - take the first root
	if (m_hSearchResult == nullptr) {
		m_hSearchResult = GetTreeView()->GetRootItem();
		if (m_hSearchResult == nullptr) {
			return;
		}

		if (m_pSite->ContainsText(m_cmbSearchFilter.GetCurSel(), m_hSearchResult, m_strSearchText)) {
			SelectItem(m_hSearchResult);
			return;
		}
	} // if (m_hSearchResult == nullptr)

	// Children
	HTREEITEM hSearchResult = SearchChildren(m_hSearchResult);
	if (hSearchResult != nullptr) {
		SelectItem(hSearchResult);
		m_hSearchResult = hSearchResult;
		return;
	}

	// Siblings and their children
	hSearchResult = SearchSiblings(m_hSearchResult);
	if (hSearchResult != nullptr) {
		SelectItem(hSearchResult);
		m_hSearchResult = hSearchResult;
		return;
	}

	// Parents, siblings and their children
	hSearchResult = SearchParents(m_hSearchResult);
	if (hSearchResult != nullptr) {
		SelectItem(hSearchResult);
		m_hSearchResult = hSearchResult;
		return;
	}

	m_bEndOfSearch = TRUE;

	::MessageBox(::AfxGetMainWnd()->GetSafeHwnd(), L"No more results found.", L"Search", MB_ICONINFORMATION | MB_OK);
}

BOOL CSearchTreeCtrlDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	auto vecSearchFilters = m_pSite->GetSearchFilters();
	ASSERT(!vecSearchFilters.empty());

	for (auto strSearchFilter : vecSearchFilters) {
		m_cmbSearchFilter.AddString(strSearchFilter);
	}

	m_cmbSearchFilter.SetCurSel(0);

	SetWindowText(m_pSite->GetSearchFilterType(vecSearchFilters[0]));

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// ------------------------------------------------------------------------------------------------
void CSearchTreeCtrlDialog::OnSelchangeComboSearchFilter()
{
	Reset();

	auto vecSearchFilters = m_pSite->GetSearchFilters();
	ASSERT(!vecSearchFilters.empty());

	SetWindowText(m_pSite->GetSearchFilterType(vecSearchFilters[m_cmbSearchFilter.GetCurSel()]));
}
