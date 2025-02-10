// BCF\BCFView.cpp : implementation file
//

#include "stdafx.h"

#ifdef _ENABLE_BCF

#include "STEPViewer.h"
#include "afxdialogex.h"
#include "BCF\BCFView.h"


// CBCFView dialog

IMPLEMENT_DYNAMIC(CBCFView, CDialogEx)

BEGIN_MESSAGE_MAP(CBCFView, CDialogEx)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


CBCFView::CBCFView(CMySTEPViewerDoc& doc, LPCTSTR bcfFilePath)
	: CDialogEx(IDD_BCF, AfxGetMainWnd())
	, m_doc (doc)
	, m_bcfFilePath(bcfFilePath)
	, m_bcfProject(NULL)
{
	Create(IDD_BCF, AfxGetMainWnd());
}

CBCFView::~CBCFView()
{
	if (m_bcfProject) {
		ShowLog(false);
		if (!m_bcfProject->Delete()) {
			ShowLog(true);
		}
		m_bcfProject = NULL;
	}
}

void CBCFView::PostNcDestroy()
{
	CDialogEx::PostNcDestroy();

	delete this;
}

void CBCFView::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TOPIC_NUMBER, m_wndTopics);
}

void CBCFView::OnClose()
{
	CDialogEx::OnClose();
	DestroyWindow();
}


BOOL CBCFView::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//
	//
	if (!LoadBCFProject())
	{
		EndDialog(IDCANCEL);
		return TRUE;
	}
	
	//
	//
	ShowWindow(SW_SHOW);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


bool CBCFView::LoadBCFProject()
{
	CWaitCursor wait;

	m_bcfProject = BCFProject::Create();
	if (!m_bcfProject) {
		AfxMessageBox(L"Failed to initialize BCF.");
	}

	if (!m_bcfProject->ReadFile(ToUTF8(m_bcfFilePath).c_str(), true))
	{
		ShowLog(true);
		EndDialog(IDCANCEL);
		return TRUE;
	}
	ShowLog(false);

	m_wndTopics.ResetContent();

	BCFTopic* topic = NULL;
	for (uint16_t i = 0; topic = m_bcfProject->TopicGetAt(i); i++) {
		CString guid = FromUTF8((topic->GetGuid()));
		CString text;
		text.Format(L"#%d: %s", (int)i+1, guid.GetString());
		int item = m_wndTopics.AddString(text);
		m_wndTopics.SetItemData(item, (DWORD_PTR)topic);
	}
	m_wndTopics.AddString(L"<New>");

	m_wndTopics.SetCurSel(0);

	return true;
}


void CBCFView::ShowLog(bool knownError)
{
	const char* msg = NULL;
	if (m_bcfProject) {
		msg = m_bcfProject->GetErrors();
	}

	if (knownError) {
		if (!msg || !*msg) {
			msg = "Unknown BCF error";
		}
	}

	if (msg && *msg) {
		AfxMessageBox(FromUTF8(msg), knownError ? MB_ICONERROR : MB_ICONEXCLAMATION);
	}
}


#endif //_ENABLE_BCF


