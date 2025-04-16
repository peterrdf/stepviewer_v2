#pragma once
#include "afxdialogex.h"

#include "IDS.h"
#include "_ap_mvc.h"

// ************************************************************************************************
class CIDSCheckerDialog : public CDialogEx
{
private: // Fields

	_model* m_pModel;
	CString m_strIDSFile;

	DECLARE_DYNAMIC(CIDSCheckerDialog)

public:
	CIDSCheckerDialog(_model* pModel, CWnd* pParent = nullptr);   // standard constructor
	virtual ~CIDSCheckerDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_IDS_CHECKER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support	

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_edResults;
	CButton m_chbShowOnlyErrors;
	virtual BOOL OnInitDialog();
private: 
	void Do();
public:
	afx_msg void OnBnClickedCheckErrorsOnly();
};
