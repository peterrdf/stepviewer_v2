#pragma once


// CBCFProjInfo dialog

class CBCFProjInfo : public CDialogEx
{
	DECLARE_DYNAMIC(CBCFProjInfo)

public:
	CBCFProjInfo(BCFProject& bcfProject, CWnd* pParent = nullptr);   // standard constructor
	virtual ~CBCFProjInfo();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BCF_PROJINFO };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()

private:
	BCFProject& m_bcfProject;

	CString m_strUserName;
	CString m_strProjectId;
	CString m_strProjectName;
};
