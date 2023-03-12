// SearchSchemaDialog.cpp : implementation file
//

#include "stdafx.h"
#include "SearchSchemaDialog.h"
#include "afxdialogex.h"
#include "resource.h"
#include "SchemaViewConsts.h"


// CSearchSchemaDialog dialog

// ------------------------------------------------------------------------------------------------
BOOL CSearchSchemaDialog::ContainsText(HTREEITEM hItem, const CString& strText)
{
	ASSERT(hItem != nullptr);

	CString strItemText = m_pTreeCtrl->GetItemText(hItem);
	strItemText.MakeLower();

	CString strTextLower = strText;
	strTextLower.MakeLower();

	// Entities
	if (m_enSearchFilter == enumSearchFilter::Entities)
	{
		int iImage, iSelectedImage = -1;
		m_pTreeCtrl->GetItemImage(hItem, iImage, iSelectedImage);

		ASSERT(iImage == iSelectedImage);

		if (iImage == IMAGE_ENTITY)
		{
			return strItemText.Find(strTextLower, 0) != -1;
		}
		else
		{
			return false;
		}
	}

	// Attributes
	if (m_enSearchFilter == enumSearchFilter::Attributes)
	{
		int iImage, iSelectedImage = -1;
		m_pTreeCtrl->GetItemImage(hItem, iImage, iSelectedImage);

		ASSERT(iImage == iSelectedImage);

		if (iImage == IMAGE_ATTRIBUTE)
		{
			return strItemText.Find(strTextLower, 0) != -1;
		}
		else
		{
			return false;
		}
	}

	// All
	return strItemText.Find(strTextLower, 0) != -1;
}

// ------------------------------------------------------------------------------------------------
void CSearchSchemaDialog::SelectItem(HTREEITEM hItem)
{
	ASSERT(hItem != nullptr);

	// Unselect
	if (m_hSearchResult != nullptr)
	{
		UnselectItem(m_hSearchResult);
	}

	// Select
	m_pTreeCtrl->EnsureVisible(hItem);
	m_pTreeCtrl->SetItemState(hItem, TVIS_SELECTED, TVIS_SELECTED);
	m_pTreeCtrl->SetFocus();
}

// ------------------------------------------------------------------------------------------------
void CSearchSchemaDialog::UnselectItem(HTREEITEM hItem)
{
	ASSERT(hItem != nullptr);

	m_pTreeCtrl->SetItemState(hItem, 0, TVIS_SELECTED);
}

// ------------------------------------------------------------------------------------------------
HTREEITEM CSearchSchemaDialog::SearchChildren(HTREEITEM hParent)
{
	ASSERT(hParent != nullptr);

	HTREEITEM hChild = m_pTreeCtrl->GetNextItem(hParent, TVGN_CHILD);
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

		hChild = m_pTreeCtrl->GetNextSiblingItem(hChild);
	} // while (hChild != nullptr)

	return nullptr;
}

// ------------------------------------------------------------------------------------------------
HTREEITEM CSearchSchemaDialog::SearchSiblings(HTREEITEM hItem)
{
	ASSERT(hItem != nullptr);

	HTREEITEM hSibling = m_pTreeCtrl->GetNextSiblingItem(hItem);
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

		hSibling = m_pTreeCtrl->GetNextSiblingItem(hSibling);
	} // while (hSibling != nullptr)

	return nullptr;
}

// ------------------------------------------------------------------------------------------------
HTREEITEM CSearchSchemaDialog::SearchParents(HTREEITEM hItem)
{
	ASSERT(hItem != nullptr);

	HTREEITEM hParent = m_pTreeCtrl->GetParentItem(hItem);
	if (hParent == nullptr)
	{
		return nullptr;
	}

	HTREEITEM hSibling = m_pTreeCtrl->GetNextSiblingItem(hParent);
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
void CSearchSchemaDialog::Reset()
{
	m_hSearchResult = nullptr;
	m_bEndOfSearch = FALSE;
}

IMPLEMENT_DYNAMIC(CSearchSchemaDialog, CDialogEx)

CSearchSchemaDialog::CSearchSchemaDialog(CViewTree* pTreeCtrl)
	: CDialogEx(IDD_DIALOG_SEARCH, nullptr)
	, m_pTreeCtrl(pTreeCtrl)
	, m_enSearchFilter(enumSearchFilter::All)
	, m_hSearchResult(nullptr)
	, m_bEndOfSearch(FALSE)
	, m_strSearchText(_T(""))
{
	ASSERT(m_pTreeCtrl != nullptr);
}

CSearchSchemaDialog::~CSearchSchemaDialog()
{
}

void CSearchSchemaDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_SEARCH_TEXT, m_strSearchText);
	DDX_Control(pDX, IDC_BUTTON_SEARCH, m_btnSearch);
	DDX_Control(pDX, IDC_COMBO_SEARCH_FILTER, m_cmbSearchFilter);
}


BEGIN_MESSAGE_MAP(CSearchSchemaDialog, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT_SEARCH_TEXT, &CSearchSchemaDialog::OnEnChangeEditSearchText)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, &CSearchSchemaDialog::OnBnClickedButtonSearch)
	ON_CBN_SELCHANGE(IDC_COMBO_SEARCH_FILTER, &CSearchSchemaDialog::OnCbnSelchangeComboSearchFilter)
END_MESSAGE_MAP()


// CSearchSchemaDialog message handlers

// ------------------------------------------------------------------------------------------------
void CSearchSchemaDialog::OnEnChangeEditSearchText()
{
	UpdateData();

	m_btnSearch.EnableWindow(!m_strSearchText.IsEmpty());
}

// ------------------------------------------------------------------------------------------------
void CSearchSchemaDialog::OnBnClickedButtonSearch()
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
		m_hSearchResult = m_pTreeCtrl->GetRootItem();
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
BOOL CSearchSchemaDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_cmbSearchFilter.AddString(_T("(All)"));
	m_cmbSearchFilter.AddString(_T("Entities"));
	m_cmbSearchFilter.AddString(_T("Attributes"));

	m_cmbSearchFilter.SetCurSel((int)m_enSearchFilter);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

// ------------------------------------------------------------------------------------------------
void CSearchSchemaDialog::OnCbnSelchangeComboSearchFilter()
{
	m_enSearchFilter = (enumSearchFilter)m_cmbSearchFilter.GetCurSel();
}
