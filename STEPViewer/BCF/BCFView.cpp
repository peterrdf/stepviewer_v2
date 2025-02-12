// BCF\BCFView.cpp : implementation file
//

#include "stdafx.h"

#include "STEPViewer.h"
#include "afxdialogex.h"
#include "STEPViewerDoc.h"
#include "_ap_model_factory.h"
#include "BCF\BCFView.h"


// CBCFView dialog

IMPLEMENT_DYNAMIC(CBCFView, CDialogEx)

BEGIN_MESSAGE_MAP(CBCFView, CDialogEx)
	ON_WM_CLOSE()
	ON_CBN_SELCHANGE(IDC_TOPICS, &CBCFView::OnSelchangeTopic)
	ON_WM_SHOWWINDOW()
	ON_LBN_SELCHANGE(IDC_COMMENTS_LIST, &CBCFView::OnSelchangeCommentsList)
END_MESSAGE_MAP()


CBCFView::CBCFView(CMySTEPViewerDoc& doc)
	: CDialogEx(IDD_BCF, AfxGetMainWnd())
	, m_doc (doc)
	, m_bcfProject(NULL)
	, m_strTopicType(_T(""))
	, m_strTopicStage(_T(""))
	, m_strTopicStatus(_T(""))
	, m_strAssigned(_T(""))
	, m_strPriority(_T(""))
	, m_strSnippetType(_T(""))
	, m_strAuthor(_T(""))
	, m_strDue(_T(""))
	, m_strDescription(_T(""))
	, m_strTitle(_T(""))
	, m_strSnippetReference(_T(""))
	, m_strSnippetSchema(_T(""))
	, m_strIndex(_T(""))
	, m_strServerId(_T(""))
	, m_strCommentText(_T(""))
{
}

CBCFView::~CBCFView()
{
	ASSERT(!m_bcfProject);
	CloseBCFProject();
}

void CBCFView::CloseBCFProject()
{
	if (m_bcfProject) {
		ShowLog(false);
		if (!m_bcfProject->Delete()) {
			ShowLog(true);
		}
		m_bcfProject = NULL;

		SetModelsExternallyManaged(m_preloadedModels);
		m_preloadedModels.clear(); //lave to manage by m_doc

		for (auto& item : m_loadedModels) {
			if (item.second) {
				delete item.second;
			}
		}
		m_loadedModels.clear();
	}
}

void CBCFView::SetModelsExternallyManaged(std::vector <_model*>& models)
{
	//do not delete models, just set new list
	m_doc.editModelList().clear();
	m_doc.setModels(models);
}

void CBCFView::OpenBCFProject(LPCTSTR bcfFilePath)
{
	ASSERT(!m_bcfProject);
	CloseBCFProject();

	//
	m_bcfProject = BCFProject::Create();
	if (!m_bcfProject) {
		AfxMessageBox(L"Failed to initialize BCF.");
		return;
	}

	std::swap(m_preloadedModels, m_doc.editModelList());
	m_doc.setModel(NULL);

	//auto user = AfxGetApp()->GetProfileString(L"BCF", L"User");
	//if (user.IsEmpty()) {
	//	AfxMessageBox(L"Enter your user name on Project Info dialog", MB_ICONEXCLAMATION | MB_OK);
	//}
	//m_bcfProject->SetOptions(user, )

	if (bcfFilePath) {
		//open existing
		if (!m_bcfProject->ReadFile(ToUTF8(bcfFilePath).c_str(), true))
		{
			ShowLog(true);
			CloseBCFProject();
			return;
		}
	}
	else {
		//create new, at least one topic is required
		m_bcfProject->TopicAdd(NULL, NULL, NULL);
	}
	ShowLog(false);

	if (!IsWindow(GetSafeHwnd())) {
		Create(IDD_BCF, AfxGetMainWnd());
	}
	ShowWindow(SW_SHOW);

	//
	CString title;
	title.Format(L"BCF-XML %s", bcfFilePath ? bcfFilePath : L"<New>");
	SetWindowText(title);

	LoadProjectToView();
}


void CBCFView::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TOPICS, m_wndTopics);
	DDX_Control(pDX, IDC_TOPIC_TYPE, m_wndTopicType);
	DDX_CBString(pDX, IDC_TOPIC_TYPE, m_strTopicType);
	DDX_Control(pDX, IDC_TOPIC_STAGE, m_wndTopicStage);
	DDX_CBString(pDX, IDC_TOPIC_STAGE, m_strTopicStage);
	DDX_Control(pDX, IDC_TOPIC_STATUS, m_wndTopicStatus);
	DDX_CBString(pDX, IDC_TOPIC_STATUS, m_strTopicStatus);
	DDX_Control(pDX, IDC_TOPIC_ASSIGNED, m_wndAssigned);
	DDX_CBString(pDX, IDC_TOPIC_ASSIGNED, m_strAssigned);
	DDX_Control(pDX, IDC_TOPIC_PRIORITY, m_wndPriority);
	DDX_CBString(pDX, IDC_TOPIC_PRIORITY, m_strPriority);
	DDX_Control(pDX, IDC_SNIPPET_TYPE, m_wndSnippetType);
	DDX_CBString(pDX, IDC_SNIPPET_TYPE, m_strSnippetType);
	DDX_Control(pDX, IDC_TAB, m_wndTab);
	DDX_Control(pDX, IDC_AUTHOR, m_wndAuthor);
	DDX_Text(pDX, IDC_AUTHOR, m_strAuthor);
	DDX_Control(pDX, IDC_TOPIC_DUE, m_wndDue);
	DDX_Text(pDX, IDC_TOPIC_DUE, m_strDue);
	DDX_Control(pDX, IDC_TOPIC_DESCRIPTION, m_wndDescription);
	DDX_Text(pDX, IDC_TOPIC_DESCRIPTION, m_strDescription);
	DDX_Control(pDX, IDC_TOPIC_TITLE, m_wndTitle);
	DDX_Text(pDX, IDC_TOPIC_TITLE, m_strTitle);
	DDX_Control(pDX, IDC_SNIPPET_REFERENCE, m_wndSnippetReference);
	DDX_Text(pDX, IDC_SNIPPET_REFERENCE, m_strSnippetReference);
	DDX_Control(pDX, IDC_SNIPPET_SCHEMA, m_wndSnippetSchema);
	DDX_Text(pDX, IDC_SNIPPET_SCHEMA, m_strSnippetSchema);
	DDX_Control(pDX, IDC_TOPIC_INDEX, m_wndIndex);
	DDX_Text(pDX, IDC_TOPIC_INDEX, m_strIndex);
	DDX_Control(pDX, IDC_TOPIC_SERVER_ID, m_wndServerIndex);
	DDX_Text(pDX, IDC_TOPIC_SERVER_ID, m_strServerId);
	DDX_Control(pDX, IDC_TOPIC_COMMENT_TEXT, m_wndCommentText);
	DDX_Text(pDX, IDC_TOPIC_COMMENT_TEXT, m_strCommentText);
	DDX_Control(pDX, IDC_COMMENTS_LIST, m_wndCommentsList);
}

void CBCFView::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);
	if (!bShow) {
		CloseBCFProject();
	}
}

void CBCFView::OnClose()
{
	CDialogEx::OnClose();
	CloseBCFProject();
}


BOOL CBCFView::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_wndTab.InsertItem(0, L"Labels");
	m_wndTab.InsertItem(0, L"Related topic");
	m_wndTab.InsertItem(0, L"Links");
	m_wndTab.InsertItem(0, L"Documents");

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CBCFView::LoadProjectToView()
{
	CWaitCursor wait;


	//load extensions
	//
	FillFromExtension(m_wndTopicType, BCFTopicTypes);
	FillFromExtension(m_wndTopicStatus, BCFTopicStatuses);
	FillFromExtension(m_wndPriority, BCFPriorities);
	FillFromExtension(m_wndAssigned, BCFUsers);
	FillFromExtension(m_wndSnippetType, BCFSnippetTypes);
	FillFromExtension(m_wndTopicStage, BCFStages);

	// load topics
	//
	m_wndTopics.ResetContent();

	BCFTopic* topic = NULL;
	for (uint16_t i = 0; topic = m_bcfProject->TopicGetAt(i); i++) {
		InsertTopicToList(i, topic);
	}
	m_wndTopics.AddString(L"<New>");
	m_wndTopics.SetCurSel(0);
	OnSelchangeTopic();
}

void CBCFView::InsertTopicToList(int item, BCFTopic* topic)
{
	CString guid = FromUTF8((topic->GetGuid()));
	CString text;
	text.Format(L"#%d: %s", item + 1, guid.GetString());
	m_wndTopics.InsertString(item, text);
	m_wndTopics.SetItemData(item, (DWORD_PTR)topic);
}

void CBCFView::FillFromExtension(CComboBox& wnd, BCFEnumeration enumeraion)
{
	wnd.ResetContent();

	if (m_bcfProject) {
		auto& extensions = m_bcfProject->GetExtensions();

		uint16_t ind = 0;
		while (auto elem = extensions.GetElement(enumeraion, ind++)) {
			wnd.AddString(FromUTF8(elem));
		}
	}
}

void CBCFView::ShowLog(bool knownError)
{
	const char* msg = NULL;
	if (m_bcfProject) {
		msg = m_bcfProject->GetErrors();
	}

	if (knownError) {
		if (!msg || !*msg) {
			msg = "Unknown BCF error";
		}
	}

	if (msg && *msg) {
		AfxMessageBox(FromUTF8(msg), knownError ? MB_ICONERROR : MB_ICONEXCLAMATION);
	}
}


void CBCFView::OnSelchangeTopic()
{
	auto item = m_wndTopics.GetCurSel();
	auto topic = (BCFTopic*)m_wndTopics.GetItemData(item);

	if (!topic) {
		auto sel = m_wndTopics.GetCurSel();
		if (IDYES == AfxMessageBox(L"Do you want to create new topic?", MB_ICONQUESTION | MB_YESNO)) {
			topic = m_bcfProject->TopicAdd(NULL, NULL, NULL);
			InsertTopicToList(sel, topic);
		}
		else {
			sel = 0;
		}
		m_wndTopics.SetCurSel(sel);
		topic = (BCFTopic*)m_wndTopics.GetItemData(sel);
	}

	if (topic) { 
		SetActiveTopic(topic);
	}
}

void CBCFView::SetActiveTopic(BCFTopic* topic)
{
	ASSERT(topic);
	if (!topic) {
		return;
	}

	SetActiveModels(topic);

	//
	m_strAuthor.Format(L"Created by %s at %s", FromUTF8(topic->GetCreationAuthor()).GetString(), FromUTF8(topic->GetCreationDate()).GetString());
	if (*topic->GetModifiedAuthor()) {
		CString modifier;
		modifier.Format(L", modified by % s at % s", FromUTF8(topic->GetModifiedAuthor()).GetString(), FromUTF8(topic->GetModifiedDate()).GetString());
		m_strAuthor.Append(modifier);
	}

	m_strTitle = FromUTF8(topic->GetTitle());
	m_strDescription = FromUTF8(topic->GetDescription());

	m_strTopicType = FromUTF8(topic->GetTopicType());
	m_strTopicStage = FromUTF8(topic->GetStage());
	m_strTopicStatus = FromUTF8(topic->GetTopicStatus());
	m_strAssigned = FromUTF8(topic->GetAssignedTo());
	m_strPriority = FromUTF8(topic->GetPriority());
	m_strDue = FromUTF8(topic->GetDueDate());

	m_strIndex.Format(L"%d", topic->GetIndex());
	m_strServerId = FromUTF8(topic->GetServerAssignedId());

	m_wndCommentsList.ResetContent();
	m_strCommentText.Empty();
	uint16_t i = 0;
	while (auto comment = topic->CommentGetAt(i++)) {
		CString text;
		text.Format(L"#%d created by %s at %s",
			(int)i,
			FromUTF8(comment->GetAuthor()).GetString(),
			FromUTF8(comment->GetDate())
		);
		if (*comment->GetModifiedAuthor()) {
			CString modifier;
			modifier.Format(L", modified by %s at %s",
				FromUTF8(comment->GetModifiedAuthor()).GetString(),
				FromUTF8(comment->GetModifiedDate()).GetString()
			);
			text.Append(modifier);
		}
		auto item = m_wndCommentsList.AddString(text);
		m_wndCommentsList.SetItemDataPtr(item, comment);
	}
	m_wndCommentsList.AddString(L"<My new comment>");
	m_wndCommentsList.SetCurSel(0);

	auto snippet = topic->GetBimSnippet(false);
	m_wndSnippetType.EnableWindow(snippet != NULL);
	m_wndSnippetReference.EnableWindow(snippet!=NULL);
	m_wndSnippetSchema.EnableWindow(snippet != NULL);
	if (snippet) {
		m_strSnippetType = FromUTF8(snippet->GetSnippetType());
		m_strSnippetReference = FromUTF8(snippet->GetReference());
		m_strSnippetSchema = FromUTF8(snippet->GetReferenceSchema());
	}
	else {
		m_strSnippetType.Empty();
		m_strSnippetReference.Empty();
		m_strSnippetSchema.Empty();
	}

	UpdateData(FALSE);
	OnSelchangeCommentsList();
}


void CBCFView::OnSelchangeCommentsList()
{
	auto item = m_wndCommentsList.GetCurSel();
	auto comment = (BCFComment*)m_wndCommentsList.GetItemDataPtr(item);

	if (!comment) {
		m_strCommentText.Empty();
		//TODO
	}
	else {
		m_strCommentText = FromUTF8(comment->GetText());
	}

	UpdateData(FALSE);
}

void CBCFView::SetActiveModels(BCFTopic* topic)
{
	std::vector<_model*> activeModels = m_preloadedModels; //copy is intentional

	uint16_t i = 0;
	while (BCFFile* file = topic->FileGetAt(i++)) {
		_model* model = NULL;
		
		auto it = m_loadedModels.find(file);
		if (it != m_loadedModels.end()) {
			model = it->second;
		}
		else {
			auto path = FromUTF8(file->GetReference());
			model = _ap_model_factory::load(path, true, nullptr, false);
			if (!model) {
				CString msg;
				msg.Format(L"Failed to read file: %s", path.GetString());
				AfxMessageBox(msg, MB_ICONEXCLAMATION);
			}
			m_loadedModels[file] = model;
		}

		if (model) {
			activeModels.push_back(model);
		}
	}

	SetModelsExternallyManaged(activeModels);
}
