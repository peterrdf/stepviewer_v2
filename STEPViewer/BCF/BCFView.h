#pragma once

#include "bcfAPI.h"

class CMySTEPViewerDoc;
class CMySTEPViewerView;

// CBCFView dialog

class CBCFView : public CDialogEx
{
	DECLARE_DYNAMIC(CBCFView)

public:
	CBCFView(CMySTEPViewerDoc& doc);
	virtual ~CBCFView();

	void DeleteContent();
	bool ReadBCFFile(LPCTSTR bcfFilePath);
	bool CreateNewProject(); //
	bool HasContent() { return m_bcfProject != NULL; }
	bool Show();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BCF };
#endif

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnClose();
	afx_msg void OnSelchangeTopic();
	afx_msg void OnSelchangeCommentsList();
	afx_msg void OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClickedButtonAddMulti();
	afx_msg void OnClickedButtonRemoveMulti();
	afx_msg void OnSelchangeMultiList();

private:
	CMySTEPViewerView* GetView();
	void LoadProjectToView();
	void InsertTopicToList(int item, BCFTopic* topic);
	void FillFromExtension(CComboBox& wnd, BCFEnumeration enumeraion);
	void ShowLog(bool knownError); //false: show log if any, not neccessary error
	void SetActiveTopic(BCFTopic* topic);
	void SetActiveModels(BCFTopic* topic);
	void SetModelsExternallyManaged(std::vector<_model*>& models);
	void SetActiveViewPoint(BCFViewPoint* vp);
	void FillMultiList();
	void FillLabels(BCFTopic* topic);
	void FillRelated(BCFTopic* topic);
	void FillLinks(BCFTopic* topic);
	void FillDocuments(BCFTopic* topic);

private:
	CMySTEPViewerDoc&				m_doc;
	BCFProject*						m_bcfProject;
	CString							m_bcfFilePath;
	std::vector<_model*>			m_preloadedModels;
	std::map<BCFFile*, _model*>		m_loadedModels;

private:
	CComboBox m_wndTopics;
	CComboBox m_wndTopicType;
	CString m_strTopicType;
	CComboBox m_wndTopicStage;
	CString m_strTopicStage;
	CComboBox m_wndTopicStatus;
	CString m_strTopicStatus;
	CComboBox m_wndAssigned;
	CString m_strAssigned;
	CComboBox m_wndPriority;
	CString m_strPriority;
	CComboBox m_wndSnippetType;
	CString m_strSnippetType;
	CTabCtrl m_wndTab;
	CStatic m_wndAuthor;
	CString m_strAuthor;
	CEdit m_wndDue;
	CString m_strDue;
	CEdit m_wndDescription;
	CString m_strDescription;
	CEdit m_wndTitle;
	CString m_strTitle;
	CEdit m_wndSnippetReference;
	CString m_strSnippetReference;
	CEdit m_wndSnippetSchema;
	CString m_strSnippetSchema;
	CEdit m_wndIndex;
	CString m_strIndex;
	CEdit m_wndServerIndex;
	CString m_strServerId;
	CListBox m_wndCommentsList;
	CEdit m_wndCommentText;
	CString m_strCommentText;
	CListBox m_wndMultiList;
	CButton m_wndAddMulti;
	CButton m_wndRemoveMulti;
public:
	afx_msg void OnClickedProjectInfo();
};

