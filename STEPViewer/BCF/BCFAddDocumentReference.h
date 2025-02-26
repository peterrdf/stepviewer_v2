#pragma once

class CBCFView;

// CBCFAddDocumentReference dialog

class CBCFAddDocumentReference : public CDialogEx
{
	DECLARE_DYNAMIC(CBCFAddDocumentReference)

public:
	CBCFAddDocumentReference(CBCFView& view);   // standard constructor
	virtual ~CBCFAddDocumentReference();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BCF_ADDOCUMENT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnClickedButtonBrowse();

public:
	CBCFView& m_view;
	CString m_strPath;
	CString m_strDescription;
	BOOL m_isExternal;
};
