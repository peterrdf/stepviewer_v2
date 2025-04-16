// IDSCheckerDialog.cpp : implementation file
//

#include "stdafx.h"
#include "STEPViewer.h"
#include "afxdialogex.h"
#include "IDSCheckerDialog.h"

// ************************************************************************************************
class IDSConsole : public RDF::IDS::Console
{

public:

	IDSConsole(CString& text) :m_text(text) {}

	virtual void out(const char* sz) override
	{
		m_text.Append(CString(sz));
	}

	CString& m_text;
};

// ************************************************************************************************
IMPLEMENT_DYNAMIC(CIDSCheckerDialog, CDialogEx)

CIDSCheckerDialog::CIDSCheckerDialog(_model* pModel, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_IDS_CHECKER, pParent)
	, m_pModel(pModel)
	, m_strIDSFile(L"")
{
	ASSERT(m_pModel != nullptr);
}

CIDSCheckerDialog::~CIDSCheckerDialog()
{
}

void CIDSCheckerDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_IDS_CHECKER_RESULTS, m_edResults);
	DDX_Control(pDX, IDC_CHECK_ERRORS_ONLY, m_chbShowOnlyErrors);
}


BEGIN_MESSAGE_MAP(CIDSCheckerDialog, CDialogEx)
	ON_BN_CLICKED(IDC_CHECK_ERRORS_ONLY, &CIDSCheckerDialog::OnBnClickedCheckErrorsOnly)
END_MESSAGE_MAP()


// CIDSCheckerDialog message handlers

BOOL CIDSCheckerDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_chbShowOnlyErrors.SetCheck(1);

	TCHAR IDS_FILTER[] = _T("IDS Models (*.ids)|*.ids|All Files (*.*)|*.*||");
	CFileDialog dlgFile(TRUE, nullptr, _T(""), OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT, IDS_FILTER);
	if (dlgFile.DoModal() != IDOK) {
		EndDialog(IDCANCEL);
		
	}
	m_strIDSFile = dlgFile.GetPathName();

	Do();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CIDSCheckerDialog::Do()
{
	CString strLog;
	RDF::IDS::File ids;
	if (ids.Read(m_strIDSFile)) {

		IDSConsole output(strLog);
		bool ok = ids.Check(_ptr<_ap_model>(m_pModel)->getSdaiModel(), false,
			m_chbShowOnlyErrors.GetCheck() ? RDF::IDS::MsgLevel::Error : RDF::IDS::MsgLevel::All,
			&output);

		CString strRes;
		strRes.Format(L"Result: %s\r\n\r\n", ok ? L"OK" : L"FAIL");

		strLog.Insert(0, strRes);
	} else {
		strLog.Format(L"Failed to read IDS file : %s", m_strIDSFile.GetString());
	}

	m_edResults.SetWindowTextW(strLog);
}

void CIDSCheckerDialog::OnBnClickedCheckErrorsOnly()
{
	Do();
}
