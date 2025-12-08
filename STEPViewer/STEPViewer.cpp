
// STEPViewer.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "STEPViewer.h"
#include "MainFrm.h"
#include "STEPViewerDoc.h"
#include "STEPViewerView.h"

#include "version.h"

#include <iostream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMySTEPViewerApp

BEGIN_MESSAGE_MAP(CMySTEPViewerApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CMySTEPViewerApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)
END_MESSAGE_MAP()


// CMySTEPViewerApp construction

CMySTEPViewerApp::CMySTEPViewerApp()
{
	m_bHiColorIcons = TRUE;

	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
	// If the application is built using Common Language Runtime support (/clr):
	//     1) This additional setting is needed for Restart Manager support to work properly.
	//     2) In your project, you must add a reference to System.Windows.Forms in order to build.
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("STEPViewer.AppID.NoVersion"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance

	std::wcout.imbue(std::locale::global(std::locale("")));
}

// The one and only CMySTEPViewerApp object

CMySTEPViewerApp theApp;


// CMySTEPViewerApp initialization

BOOL CMySTEPViewerApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();


	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction(FALSE);

	// AfxInitRichEdit2() is required to use RichEdit control	
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("STEPViewr, RDF LTD"));
	LoadStdProfileSettings(10);  // Load standard INI file options (including MRU)

	InitContextMenuManager();
	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CMySTEPViewerDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CMySTEPViewerView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	return TRUE;
}

int CMySTEPViewerApp::ExitInstance()
{
	//TODO: handle additional resources you may have added
	AfxOleTerm(FALSE);

	return CWinAppEx::ExitInstance();
}

// CMySTEPViewerApp message handlers


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	CString GetVersionInfo(LPCTSTR lpszKey);
	void UpdateVersionInfo();

	// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	UpdateVersionInfo();

	return TRUE;
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}
CString CAboutDlg::GetVersionInfo(LPCTSTR lpszKey)
{
	CString strValue;

	// Get the module handle
	HMODULE hModule = GetModuleHandle(NULL);

	// Get the filename of the executable
	TCHAR szFilename[MAX_PATH];
	GetModuleFileName(hModule, szFilename, MAX_PATH);

	// Get version information size
	DWORD dwHandle = 0;
	DWORD dwSize = GetFileVersionInfoSize(szFilename, &dwHandle);

	if (dwSize == 0)
		return strValue;

	// Allocate buffer for version information
	BYTE* pVersionInfo = new BYTE[dwSize];

	// Get the version information
	if (GetFileVersionInfo(szFilename, dwHandle, dwSize, pVersionInfo)) {
		// Query language and code page
		struct LANGANDCODEPAGE
		{
			WORD wLanguage;
			WORD wCodePage;
		} *lpTranslate;

		UINT cbTranslate;
		if (VerQueryValue(pVersionInfo, TEXT("\\VarFileInfo\\Translation"), (LPVOID*)&lpTranslate, &cbTranslate)) {
			// Format the query string
			CString strQuery;
			strQuery.Format(TEXT("\\StringFileInfo\\%04x%04x\\%s"),
				lpTranslate[0].wLanguage, lpTranslate[0].wCodePage, lpszKey);

			// Query the specific value
			LPCTSTR lpszValue;
			UINT cchValue;
			if (VerQueryValue(pVersionInfo, strQuery, (LPVOID*)&lpszValue, &cchValue)) {
				strValue = lpszValue;
			}
		}
	}

	delete[] pVersionInfo;
	return strValue;
}

void CAboutDlg::UpdateVersionInfo()
{
	wchar_t* szRevision = nullptr;
	auto iRevision = GetRevisionW(&szRevision);
	CString strProductVersion;
	strProductVersion.Format(L"%d.%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, iRevision);
	//CString strProductVersion = GetVersionInfo(_T("ProductVersion"));
	CString strProductName = GetVersionInfo(_T("ProductName"));
	if (!strProductName.IsEmpty() && !strProductVersion.IsEmpty()) {
		CString strVersionText;
		strVersionText.Format(L"%s, Version %s", (LPCWSTR)strProductName, (LPCWSTR)strProductVersion);
		SetDlgItemText(IDC_PRODUCT_NAME, strVersionText);
	}

	CString strLegalCopyright = GetVersionInfo(_T("LegalCopyright"));
	if (!strLegalCopyright.IsEmpty()) {
		SetDlgItemText(IDC_COPYRIGHT, strLegalCopyright);
	}

	CString strCompanyName = GetVersionInfo(_T("CompanyName"));
	if (!strCompanyName.IsEmpty()) {
		SetDlgItemText(IDC_COMPANY, strCompanyName);
	}
}


BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// App command to run the dialog
void CMySTEPViewerApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CMySTEPViewerApp customization load/save methods

void CMySTEPViewerApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
	bNameValid = strName.LoadString(IDS_EXPLORER);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EXPLORER);

	GetContextMenuManager()->AddMenu(L"Instances", IDR_POPUP_INSTANCES);	
}

void CMySTEPViewerApp::LoadCustomState()
{
}

void CMySTEPViewerApp::SaveCustomState()
{
}

// CMySTEPViewerApp message handlers



