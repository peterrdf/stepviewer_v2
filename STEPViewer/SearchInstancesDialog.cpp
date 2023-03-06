// SearchInstancesDialog.cpp : implementation file
//

#include "stdafx.h"
#include "SearchInstancesDialog.h"
#include "afxdialogex.h"
#include "resource.h"
#include "StructureViewConsts.h"
#include "STEPItemData.h"


// CSearchInstancesDialog dialog

// ------------------------------------------------------------------------------------------------
BOOL CSearchInstancesDialog::ContainsText(HTREEITEM hItem, const CString& strText)
{
	ASSERT(hItem != nullptr);

	CString strItemText = m_pTreeCtrl->GetItemText(hItem);
	strItemText.MakeLower();

	CString strTextLower = strText;
	strTextLower.MakeLower();

	CSTEPItemData* pItemData = (CSTEPItemData*)m_pTreeCtrl->GetItemData(hItem);	

	// Product Definition
	if (m_enSearchWhere == swProductDefitions)
	{
		if ((pItemData != nullptr) && (pItemData->getType() == enumSTEPItemDataType::dtProductDefinition))
		{
			return strItemText.Find(strText, 0) != -1;
		}

		return FALSE;
	}

	// Assemblies
	if (m_enSearchWhere == swAssemblies)
	{
		if ((pItemData != nullptr) && (pItemData->getType() == enumSTEPItemDataType::dtAssembly))
		{
			return strItemText.Find(strText, 0) != -1;
		}

		return FALSE;
	}

	// Product Instance
	if (m_enSearchWhere == swProductInstances)
	{
		if ((pItemData != nullptr) && (pItemData->getType() == enumSTEPItemDataType::dtProductInstance))
		{
			return strItemText.Find(strText, 0) != -1;
		}

		return FALSE;
	}

	// All
	return strItemText.Find(strTextLower, 0) != -1;
}

// ------------------------------------------------------------------------------------------------
void CSearchInstancesDialog::SelectItem(HTREEITEM hItem)
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
void CSearchInstancesDialog::UnselectItem(HTREEITEM hItem)
{
	ASSERT(hItem != nullptr);

	m_pTreeCtrl->SetItemState(hItem, 0, TVIS_SELECTED);
}

// ------------------------------------------------------------------------------------------------
HTREEITEM CSearchInstancesDialog::SearchChildren(HTREEITEM hParent)
{
	ASSERT(hParent != nullptr);	

	CSTEPItemData* pItemData = (CSTEPItemData*)m_pTreeCtrl->GetItemData(hParent);

	if ((pItemData != nullptr) && 
		((pItemData->getType() == enumSTEPItemDataType::dtProductDefinition) || 
			(pItemData->getType() == enumSTEPItemDataType::dtAssembly) || 
			(pItemData->getType() == enumSTEPItemDataType::dtProductInstance)))
	{
		if (m_pTreeCtrl->ItemHasChildren(hParent) && (m_pTreeCtrl->GetChildItem(hParent) == nullptr))
		{
			m_pTreeCtrl->Expand(hParent, TVE_EXPAND);
		}
	}

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
HTREEITEM CSearchInstancesDialog::SearchSiblings(HTREEITEM hItem)
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
HTREEITEM CSearchInstancesDialog::SearchParents(HTREEITEM hItem)
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
void CSearchInstancesDialog::Reset()
{
	m_hSearchResult = nullptr;
	m_bEndOfSearch = FALSE;
}

IMPLEMENT_DYNAMIC(CSearchInstancesDialog, CDialogEx)

CSearchInstancesDialog::CSearchInstancesDialog(CViewTree* pTreeCtrl)
	: CDialogEx(IDD_DIALOG_SEARCH, nullptr)
	, m_pTreeCtrl(pTreeCtrl)
	, m_enSearchWhere(swAll)
	, m_hSearchResult(nullptr)
	, m_bEndOfSearch(FALSE)
	, m_strSearchText(_T(""))
{
	ASSERT(m_pTreeCtrl != nullptr);
}

CSearchInstancesDialog::~CSearchInstancesDialog()
{
}

void CSearchInstancesDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_SEARCH_TEXT, m_strSearchText);
	DDX_Control(pDX, IDC_BUTTON_SEARCH, m_btnSearch);
	DDX_Control(pDX, IDC_COMBO_SEARCH_FILTER, m_cmbSearchWhere);
}


BEGIN_MESSAGE_MAP(CSearchInstancesDialog, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT_SEARCH_TEXT, &CSearchInstancesDialog::OnEnChangeEditSearchText)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, &CSearchInstancesDialog::OnBnClickedButtonSearch)
	ON_CBN_SELCHANGE(IDC_COMBO_SEARCH_FILTER, &CSearchInstancesDialog::OnCbnSelchangeComboSearchFilter)
END_MESSAGE_MAP()


// CSearchInstancesDialog message handlers

// ------------------------------------------------------------------------------------------------
void CSearchInstancesDialog::OnEnChangeEditSearchText()
{
	UpdateData();

	m_btnSearch.EnableWindow(!m_strSearchText.IsEmpty());
}

// ------------------------------------------------------------------------------------------------
void CSearchInstancesDialog::OnBnClickedButtonSearch()
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
BOOL CSearchInstancesDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_cmbSearchWhere.AddString(_T("(All)"));
	m_cmbSearchWhere.AddString(_T("Product Definitions"));
	m_cmbSearchWhere.AddString(_T("Assemblies"));
	m_cmbSearchWhere.AddString(_T("Product Instances"));

	m_cmbSearchWhere.SetCurSel(m_enSearchWhere);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

// ------------------------------------------------------------------------------------------------
void CSearchInstancesDialog::OnCbnSelchangeComboSearchFilter()
{
	m_enSearchWhere = (enumSearchWhere)m_cmbSearchWhere.GetCurSel();
}
