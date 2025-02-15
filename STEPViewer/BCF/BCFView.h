#pragma once

#include "bcfAPI.h"

class CMySTEPViewerDoc;
class CMySTEPViewerView;
class _model;


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

	BCFTopic* GetActiveTopic();
	void ShowLog(bool knownError); //false: show log if any, not neccessary error

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BCF_VIEW };
#endif

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnCancel();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnClose();
	afx_msg void OnSelchangeTopic();
	afx_msg void OnSelchangeCommentsList();
	afx_msg void OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClickedButtonAddMulti();
	afx_msg void OnClickedButtonRemoveMulti();
	afx_msg void OnSelchangeMultiList();
	afx_msg void OnClickedProjectInfo();
	afx_msg void OnKillfocusEdit();
	afx_msg void OnKillfocusTopicCommentText();
	afx_msg void OnClickedSave();

private:
	CMySTEPViewerView* GetView();
	void LoadProjectToView();
	void InsertTopicToList(int item, BCFTopic* topic);
	void LoadExtensions();
	void LoadExtension(CComboBox& wnd, BCFEnumeration enumeraion);
	void LoadActiveTopic();
	BCFTopic* CreateNewTopic();
	void UpdateActiveTopic();
	void LoadComments(BCFTopic* topic, int select = 0);
	void UpateActiveComment();
	bool CreateNewComment(BCFTopic* topic, CString& text);
	void SetActiveModels(BCFTopic* topic);
	void SetModelsExternallyManaged(std::vector<_model*>& models);
	void SetActiveViewPoint(BCFViewPoint* vp);
	void FillMultiList();
	void FillLabels(BCFTopic* topic);
	void FillRelated(BCFTopic* topic);
	void FillLinks(BCFTopic* topic);
	void FillDocuments(BCFTopic* topic);
	void AddLabel(BCFTopic* topic);
	void AddRelated(BCFTopic* topic);
	void AddLink(BCFTopic* topic);
	void AddDocument(BCFTopic* topic);
	void RemoveLabel(BCFTopic* topic);
	void RemoveRelated(BCFTopic* topic);
	void RemoveLink(BCFTopic* topic);
	void RemoveDocument(BCFTopic* topic);

	void FillTopicAuthor(BCFTopic* topic);
	void SaveBCFFile();

private:
	CMySTEPViewerDoc&				m_doc;
	BCFProject*						m_bcfProject;
	CString							m_bcfFilePath;
	std::vector<_model*>			m_preloadedModels;
	std::map<BCFBimFile*, _model*>	m_loadedModels;

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
	CListBox m_wndMultiList;
	CButton m_wndAddMulti;
	CButton m_wndRemoveMulti;
};

