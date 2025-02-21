#pragma once

class CBCFView;

class CBCFAddRelatedTopic : public CDialogEx
{
public:
	DECLARE_DYNAMIC(CBCFAddRelatedTopic)

public:
	CBCFAddRelatedTopic(CBCFView& bcfView);   // standard constructor
	virtual ~CBCFAddRelatedTopic();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BCF_ADDRELATEDTOPIC };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSelchangeListTopic();

private:
	CBCFView&	m_view;
	CListBox	m_wndListTopic;
	CButton		m_wndOK;
};
