#pragma once
#ifdef _ENABLE_BCF

#include "afxdialogex.h"
#include "bcfAPI.h"

class CMySTEPViewerDoc;

// CBCFView dialog

class CBCFView : public CDialogEx
{
	DECLARE_DYNAMIC(CBCFView)

public:
	CBCFView(CMySTEPViewerDoc& doc, LPCTSTR bcfFilePath);   // standard constructor
	virtual ~CBCFView();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BCF };
#endif

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnClose();

private:
	bool LoadBCFProject();
	void ShowLog(bool knownError); //false: show log if any, not neccessary error

private:
	CMySTEPViewerDoc& m_doc;
	CString           m_bcfFilePath;
	BCFProject*		  m_bcfProject;

private:
	CComboBox m_wndTopics;
};

#endif //_ENABLE_BCF