// SearchAttributeDialog.cpp : implementation file
//

#include "stdafx.h"
#include "SearchAttributeDialog.h"
#include "afxdialogex.h"
#include "resource.h"
#include "IFCSchemaViewConsts.h"
#include "IFCModel.h"


// CSearchAttributeDialog dialog

// ------------------------------------------------------------------------------------------------
BOOL CSearchAttributeDialog::ContainsText(HTREEITEM hItem, const CString& strText)
{
	ASSERT(hItem != NULL);

	CString strItemText = m_pIFCTreeCtrl->GetItemText(hItem);
	strItemText.MakeLower();

	CString strTextLower = strText;
	strTextLower.MakeLower();

	// All
	return strItemText.Find(strTextLower, 0) != -1;
}

// ------------------------------------------------------------------------------------------------
void CSearchAttributeDialog::SelectItem(HTREEITEM hItem)
{
	ASSERT(hItem != NULL);

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
void CSearchAttributeDialog::UnselectItem(HTREEITEM hItem)
{
	ASSERT(hItem != NULL);

	m_pIFCTreeCtrl->SetItemState(hItem, 0, TVIS_SELECTED);
}

// ------------------------------------------------------------------------------------------------
HTREEITEM CSearchAttributeDialog::SearchChildren(HTREEITEM hParent)
{
	ASSERT(hParent != NULL);

	HTREEITEM hChild = m_pIFCTreeCtrl->GetNextItem(hParent, TVGN_CHILD);
	while (hChild != NULL)
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
	} // while (hChild != NULL)

	return nullptr;
}

// ------------------------------------------------------------------------------------------------
HTREEITEM CSearchAttributeDialog::SearchSiblings(HTREEITEM hItem)
{
	ASSERT(hItem != NULL);

	HTREEITEM hSibling = m_pIFCTreeCtrl->GetNextSiblingItem(hItem);
	while (hSibling != NULL)
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
	} // while (hSibling != NULL)

	return nullptr;
}

// ------------------------------------------------------------------------------------------------
HTREEITEM CSearchAttributeDialog::SearchParents(HTREEITEM hItem)
{
	ASSERT(hItem != NULL);

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
void CSearchAttributeDialog::Reset()
{
	m_hSearchResult = NULL;
	m_bEndOfSearch = FALSE;
}

IMPLEMENT_DYNAMIC(CSearchAttributeDialog, CDialogEx)

CSearchAttributeDialog::CSearchAttributeDialog(CSTEPController* pController, CViewTree* pIFCTreeCtrl)
	: CDialogEx(IDD_DIALOG_SEARCH, nullptr)
	, m_pController(pController)
	, m_pIFCTreeCtrl(pIFCTreeCtrl)
	, m_enSearchFilter(enumSearchFilter::All)
	, m_hSearchResult(NULL)
	, m_bEndOfSearch(FALSE)
	, m_strSearchText(_T(""))
{
	ASSERT(m_pIFCTreeCtrl != nullptr);
}

CSearchAttributeDialog::~CSearchAttributeDialog()
{
}

void CSearchAttributeDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_SEARCH_TEXT, m_strSearchText);
	DDX_Control(pDX, IDC_BUTTON_SEARCH, m_btnSearch);
	DDX_Control(pDX, IDC_COMBO_SEARCH_FILTER, m_cmbSearchFilter);
}


BEGIN_MESSAGE_MAP(CSearchAttributeDialog, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT_SEARCH_TEXT, &CSearchAttributeDialog::OnEnChangeEditSearchText)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, &CSearchAttributeDialog::OnBnClickedButtonSearch)
	ON_CBN_SELCHANGE(IDC_COMBO_SEARCH_FILTER, &CSearchAttributeDialog::OnCbnSelchangeComboSearchFilter)
END_MESSAGE_MAP()


// CSearchAttributeDialog message handlers

// ------------------------------------------------------------------------------------------------
void CSearchAttributeDialog::OnEnChangeEditSearchText()
{
	UpdateData();

	m_btnSearch.EnableWindow(!m_strSearchText.IsEmpty());
}

// ------------------------------------------------------------------------------------------------
void CSearchAttributeDialog::OnBnClickedButtonSearch()
{
	UpdateData();

	// ExpressID
	if (m_enSearchFilter == enumSearchFilter::ExpressID)
	{
		auto pModel = m_pController->GetModel();
		if (pModel == nullptr)
		{
			ASSERT(FALSE);

			return;
		}

		switch (pModel->GetType())
		{
			case enumSTEPModelType::STEP:
			{
				ASSERT(FALSE); // TODO
			}
			break;

			case enumSTEPModelType::IFC:
			{
				 auto pIFCmodel = m_pController->GetModel()->As<CIFCModel>();
				 int64_t iExpressID = _wtoi64((LPCTSTR)m_strSearchText);

				 auto pInstance = pIFCmodel->GetInstanceByExpressID(iExpressID);
				 if (pInstance != nullptr)
				 {
					 m_pController->SelectInstance(nullptr, pInstance);
				 }
				 else
				 {
					 ::MessageBox(::AfxGetMainWnd()->GetSafeHwnd(), L"Invalid Express ID.", L"Search", MB_ICONERROR | MB_OK);
				 }
			}
			break;

			default:
			{
				ASSERT(FALSE); // Unknown
			}
			break;
		} // switch (pModel ->GetType())

		return;
	} // if (m_enSearchFilter == enumSearchFilter::ExpressID)

	// Reset
	if (m_bEndOfSearch)
	{
		ASSERT(m_hSearchResult != NULL);

		UnselectItem(m_hSearchResult);

		m_hSearchResult = NULL;
		m_bEndOfSearch = FALSE;
	}

	// Initialize - take the first root
	if (m_hSearchResult == NULL)
	{
		m_hSearchResult = m_pIFCTreeCtrl->GetRootItem();
		if (m_hSearchResult == NULL)
		{
			// No items
			return;
		}

		if (ContainsText(m_hSearchResult, m_strSearchText))
		{
			SelectItem(m_hSearchResult);

			return;
		}
	} // if (m_hSearchResult == NULL)

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
BOOL CSearchAttributeDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_cmbSearchFilter.AddString(_T("(All)"));
	m_cmbSearchFilter.AddString(_T("Express ID"));

	m_cmbSearchFilter.SetCurSel((int)m_enSearchFilter);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

// ------------------------------------------------------------------------------------------------
void CSearchAttributeDialog::OnCbnSelchangeComboSearchFilter()
{
	m_enSearchFilter = (enumSearchFilter)m_cmbSearchFilter.GetCurSel();
}
