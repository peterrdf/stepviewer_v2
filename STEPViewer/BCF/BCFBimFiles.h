#pragma once

#include "bcfAPI.h"

class CBCFView;

class CBCFBimFiles : public CDialogEx
{
	DECLARE_DYNAMIC(CBCFBimFiles)

public:
	CBCFBimFiles(CBCFView& bcfView);   // standard constructor
	virtual ~CBCFBimFiles();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BCF_TOPICFILES };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnCheckFileList();

private:
	typedef std::map<_model*, BCFBimFile*> UsedModels;

private:
	void FillFileList(BCFTopic& topic);
	void AddBimFile(BCFTopic& topic, _model& model);

private:
	CBCFView&     m_view;
	UsedModels    m_usedModels;
	CCheckListBox m_fileList;
};
