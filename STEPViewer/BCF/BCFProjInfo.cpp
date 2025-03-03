// W:\DevArea\RDF\stepviewer_v2\STEPViewer\BCF\BCFProjInfo.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "BCF\BCFProjInfo.h"


// CBCFProjInfo dialog

IMPLEMENT_DYNAMIC(CBCFProjInfo, CDialogEx)

CBCFProjInfo::CBCFProjInfo(BCFProject& bcfProject, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_BCF_PROJINFO, pParent)
	, m_bcfProject(bcfProject)
	, m_strUserName(_T(""))
	, m_strProjectId(_T(""))
	, m_strProjectName(_T(""))
{

}

CBCFProjInfo::~CBCFProjInfo()
{
}

void CBCFProjInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_USERNAME, m_strUserName);
	DDX_Text(pDX, IDC_PROJECTID, m_strProjectId);
	DDX_Text(pDX, IDC_PROJECTNAME, m_strProjectName);
}


BEGIN_MESSAGE_MAP(CBCFProjInfo, CDialogEx)
END_MESSAGE_MAP()


// CBCFProjInfo message handlers


BOOL CBCFProjInfo::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_strUserName = AfxGetApp()->GetProfileString(L"BCF", L"User");

	m_strProjectId = FromUTF8(m_bcfProject.GetProjectId());
	m_strProjectName = FromUTF8(m_bcfProject.GetName());

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CBCFProjInfo::OnOK()
{
	UpdateData();
	m_strUserName.Trim();
	m_strProjectName.Trim();

	if (m_strUserName.IsEmpty()) {
		AfxMessageBox(L"Enter your user name", MB_ICONEXCLAMATION | MB_OK);
		return;
	}
	
	AfxGetApp()->WriteProfileString(L"BCF", L"User", m_strUserName);

	m_bcfProject.SetOptions(ToUTF8(m_strUserName).c_str(), true, true);
	m_bcfProject.SetName(ToUTF8(m_strProjectName).c_str());

	CDialogEx::OnOK();
}
