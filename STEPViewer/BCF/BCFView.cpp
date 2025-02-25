// BCF\BCFView.cpp : implementation file
//

#include "stdafx.h"

#include "STEPViewer.h"
#include "STEPViewerDoc.h"
#include "STEPViewerView.h"
#include "_ap_model_factory.h"
#include "BCF\BCFProjInfo.h"
#include "BCF\BCFView.h"
#include "BCF\BCFAddLabel.h"
#include "BCF\BCFAddRelatedTopic.h"
#include "BCF\BCFAddReferenceLink.h"
#include "BCF\BCFAddDocumentReference.h"
#include "BCF\BCFBimFiles.h"
 
#define TAB_Labels			3
#define TAB_Related			2
#define TAB_Links			1
#define TAB_Documents		0

// CBCFView dialog

IMPLEMENT_DYNAMIC(CBCFView, CDialogEx)

BEGIN_MESSAGE_MAP(CBCFView, CDialogEx)
	ON_WM_CLOSE()
	ON_CBN_SELCHANGE(IDC_TOPICS, &CBCFView::OnSelchangeTopic)
	ON_WM_SHOWWINDOW()
	ON_LBN_SELCHANGE(IDC_COMMENTS_LIST, &CBCFView::OnSelchangeCommentsList)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &CBCFView::OnSelchangeTab)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &CBCFView::OnClickedButtonAddMulti)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, &CBCFView::OnClickedButtonRemoveMulti)
	ON_LBN_SELCHANGE(IDC_MULTI_LIST, &CBCFView::OnSelchangeMultiList)
	ON_BN_CLICKED(IDC_PROJECT_INFO, &CBCFView::OnClickedProjectInfo)
	ON_EN_KILLFOCUS(IDC_TOPIC_TITLE, &CBCFView::OnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_TOPIC_DESCRIPTION, &CBCFView::OnKillfocusEdit)
	ON_CBN_KILLFOCUS(IDC_TOPIC_TYPE, &CBCFView::OnKillfocusEdit)
	ON_CBN_KILLFOCUS(IDC_TOPIC_STAGE, &CBCFView::OnKillfocusEdit)
	ON_CBN_KILLFOCUS(IDC_TOPIC_STATUS, &CBCFView::OnKillfocusEdit)
	ON_CBN_KILLFOCUS(IDC_TOPIC_ASSIGNED, &CBCFView::OnKillfocusEdit)
	ON_CBN_KILLFOCUS(IDC_TOPIC_PRIORITY, &CBCFView::OnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_TOPIC_DUE, &CBCFView::OnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_TOPIC_INDEX, &CBCFView::OnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_TOPIC_SERVER_ID, &CBCFView::OnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_TOPIC_COMMENT_TEXT, &CBCFView::OnKillfocusTopicCommentText)
	ON_BN_CLICKED(IDC_SAVE, &CBCFView::OnClickedSave)
	ON_BN_CLICKED(IDC_UPDATE_VIEWPOINT, &CBCFView::OnClickedUpdateViewpoint)
	ON_BN_CLICKED(IDC_BUTTON_BIMS, &CBCFView::OnClickedButtonBims)
END_MESSAGE_MAP()


CBCFView::CBCFView(CMySTEPViewerDoc& doc)
	: CDialogEx(IDD_BCF_VIEW, AfxGetMainWnd())
	, m_doc (doc)
	, m_viewPointMgr(*this)
	, m_bcfProject(NULL)
{
}

CBCFView::~CBCFView()
{
	Close();
}

bool CBCFView::IsBCF(LPCTSTR filePath)
{
	auto len = wcslen(filePath);

	if (len >= 4 && 0 == wcscmp(filePath + len - 4, L".bcf"))
		return true;

	if (len >= 4 && 0 == wcscmp(filePath + len - 7, L".bcfzip"))
		return true;

	return false;
}

void CBCFView::Close()
{
	//close project
	if (m_bcfProject) {
		ShowLog(false);
		if (!m_bcfProject->Delete()) {
			ShowLog(true);
		}
		m_bcfProject = NULL;
	}

	//free models
	m_doc.setOwnsModelsOn();
	m_mapBimFiles.clear();
	for (auto item : m_loadedModels) {
		delete item;
	}
	m_loadedModels.clear();

	//hide window
	if (GetSafeHwnd()) {
		ShowWindow(SW_HIDE);
	}
}

bool CBCFView::SaveModified()
{
	if (m_bcfProject && m_bcfProject->IsModified()) {
		auto answer = AfxMessageBox(L"BCF data are modified. Do you want to save before continue?", MB_YESNOCANCEL);
		if (IDNO != answer) {
			if (IDYES == answer) {
				PostMessage(WM_COMMAND, IDC_SAVE);
			}
			return false;
		}
	}
	return true;
}


void CBCFView::Open(LPCTSTR filePath)
{
	//clean old data
	Close();

	//
	m_doc.setOwnsModelsOff(m_loadedModels);

	m_bcfFilePath = filePath ? filePath : L"";

	//
	//open BCF project
	//
	m_bcfProject = BCFProject::Create();
	if (!m_bcfProject) {
		AfxMessageBox(L"Failed to initialize BCF Project");
		Close();
		return;
	}

	if (!m_bcfFilePath.IsEmpty()) {
		CWaitCursor wait;
		if (!m_bcfProject->ReadFile(ToUTF8(m_bcfFilePath).c_str(), true))
		{
			ShowLog(true);
			Close();
			return;
		}
	}

	//
	CBCFProjInfo projInfo(*m_bcfProject, AfxGetMainWnd());
	if (IDOK != projInfo.DoModal()) {
		Close();
		return;
	}

	//show view
	if (!IsWindow(GetSafeHwnd())) {
		Create(IDD_BCF_VIEW, AfxGetMainWnd());
	}
	ShowWindow(SW_SHOW);

	LoadProjectToView();
}


void CBCFView::OnOpenModels(vector<_model*>& vecModels)
{
	if (!m_bcfProject || vecModels.empty()) {
		return;
	}

	auto topic = GetActiveTopic();
	ASSERT(topic);

	bool ok = true;

	for (auto model : vecModels) {
		if (model) {
			m_loadedModels.push_back(model);

			auto path = ToUTF8(model->getPath());
			
			if (topic) {
				auto file = FindBimFileByPath(topic, path.c_str());
				if (!file) {
					file = topic->AddBimFile(path.c_str(), false);
				}

				if (file) {
					m_mapBimFiles[file] = model;
				}
				else {
					ok = false;
				}
			}
		}
	}
	
	if (topic)
		ViewTopicModels(topic);
	
	vecModels.clear();
	ShowLog(!ok);
}

BCFBimFile* CBCFView::FindBimFileByPath(BCFTopic* topic, const char* searchPath)
{
	if (topic) {
		int i = 0;
		while (auto file = topic->GetBimFile(i++)) {
			auto refPath = file->GetReference();
			if (0 == strcmp(searchPath, refPath)) {
				return file;
			}
		}
	}
	return NULL;
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
	DDX_Control(pDX, IDC_COMMENTS_LIST, m_wndCommentsList);
	DDX_Control(pDX, IDC_MULTI_LIST, m_wndMultiList);
	DDX_Control(pDX, IDC_BUTTON_ADD, m_wndAddMulti);
	DDX_Control(pDX, IDC_BUTTON_REMOVE, m_wndRemoveMulti);
	DDX_Control(pDX, IDC_UPDATE_VIEWPOINT, m_wndUpdateViewPoint);
}

void CBCFView::OnClose()
{
	m_wndCommentsList.SetFocus(); //to last upate from edit field

	if (SaveModified()) {
		CDialogEx::OnClose();
		Close();
	}
}


void CBCFView::OnCancel()
{
	//CDialogEx::OnCancel();
}


void CBCFView::OnOK()
{
	//CDialogEx::OnOK();
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

	//
	CString title;
	title.Format(L"BCF-XML %s", m_bcfFilePath.IsEmpty() ? L"<New>" : m_bcfFilePath);
	SetWindowText(title);

	//load extensions
	//
	LoadExtensions();

	// load topics
	//
	m_wndTopics.ResetContent();

	BCFTopic* topic = NULL;
	for (uint16_t i = 0; topic = m_bcfProject->GetTopic(i); i++) {
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

void CBCFView::LoadExtensions()
{
	LoadExtension(m_wndTopicType, BCFTopicTypes);
	LoadExtension(m_wndTopicStatus, BCFTopicStatuses);
	LoadExtension(m_wndPriority, BCFPriorities);
	LoadExtension(m_wndAssigned, BCFUsers);
	LoadExtension(m_wndSnippetType, BCFSnippetTypes);
	LoadExtension(m_wndTopicStage, BCFStages);
}

void CBCFView::LoadExtension(CComboBox& wnd, BCFEnumeration enumeraion)
{
	CString txt;
	wnd.GetWindowText(txt);

	wnd.ResetContent();

	if (m_bcfProject) {
		auto& extensions = m_bcfProject->GetExtensions();

		uint16_t ind = 0;
		while (auto elem = extensions.GetElement(enumeraion, ind++)) {
			wnd.AddString(FromUTF8(elem));
		}
	}

	wnd.SetWindowText(txt);
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
	if (!m_bcfProject) {
		return;
	}
	auto index = m_wndTopics.GetCurSel();
	auto topic = (BCFTopic*)m_wndTopics.GetItemData(index);

	if (!topic) {
		if (!m_bcfProject->GetTopic(0) //at least one topic must exist
			|| IDYES == AfxMessageBox(L"Do you want to create new topic?", MB_ICONQUESTION | MB_YESNO)) {
			topic = CreateNewTopic();
			if (topic) {
				InsertTopicToList(index, topic);
				PostMessage(WM_COMMAND, IDC_BUTTON_BIMS);
			}
		}
		else {
			index = 0;
		}
		m_wndTopics.SetCurSel(index);
		topic = (BCFTopic*)m_wndTopics.GetItemData(index);
	}

	LoadActiveTopic();

	m_wndTitle.SetFocus();
}

BCFTopic* CBCFView::GetActiveTopic()
{
	if (m_bcfProject) {
		auto index = m_wndTopics.GetCurSel();
		auto topic = (BCFTopic*)m_wndTopics.GetItemData(index);
		return topic;
	}
	return NULL;
}

void CBCFView::FillTopicAuthor(BCFTopic* topic)
{
	CString strAuthor;

	strAuthor.Format(L"Created by %s at %s", FromUTF8(topic->GetCreationAuthor()).GetString(), FromUTF8(topic->GetCreationDate()).GetString());
	if (*topic->GetModifiedAuthor()) {
		CString modifier;
		modifier.Format(L", modified by %s at %s", FromUTF8(topic->GetModifiedAuthor()).GetString(), FromUTF8(topic->GetModifiedDate()).GetString());
		strAuthor.Append(modifier);
	}

	m_wndAuthor.SetWindowText(strAuthor);
}

BCFTopic* CBCFView::CreateNewTopic()
{
	if (!m_bcfProject) {
		return NULL;
	}

	auto topic = m_bcfProject->AddTopic(NULL, NULL, NULL);

	ShowLog(!topic);

	return topic;
}

void CBCFView::LoadActiveTopic()
{
	auto topic = GetActiveTopic();
	if (!topic) {
		return;
	}

	ViewTopicModels(topic);

	FillTopicAuthor(topic);

	m_strTitle = FromUTF8(topic->GetTitle());
	m_strDescription = FromUTF8(topic->GetDescription());

	m_strTopicType = FromUTF8(topic->GetTopicType());
	m_strTopicStage = FromUTF8(topic->GetStage());
	m_strTopicStatus = FromUTF8(topic->GetTopicStatus());
	m_strAssigned = FromUTF8(topic->GetAssignedTo());
	m_strPriority = FromUTF8(topic->GetPriority());
	m_strDue = FromUTF8(topic->GetDueDate());

	m_strIndex = FromUTF8(topic->GetIndexStr());
	m_strServerId = FromUTF8(topic->GetServerAssignedId());

	auto snippet = topic->GetBimSnippet(false);
	m_wndSnippetType.EnableWindow(snippet != NULL);
	m_wndSnippetReference.EnableWindow(snippet!=NULL);
	m_wndSnippetSchema.EnableWindow(snippet != NULL);
	if (snippet) {
		m_strSnippetType = FromUTF8(snippet->GetSnippetType());
		fs::path path = snippet->GetReference();
		m_strSnippetReference = FromUTF8(path.filename().string().c_str());
		path = snippet->GetReferenceSchema();
		m_strSnippetSchema = FromUTF8(path.filename().string().c_str());
	}
	else {
		m_strSnippetType.Empty();
		m_strSnippetReference.Empty();
		m_strSnippetSchema.Empty();
	}

	UpdateData(FALSE);

	LoadComments(topic);
	FillMultiList();
}


void CBCFView::UpdateActiveTopic()
{
	auto topic = GetActiveTopic();
	if (!topic) {
		return;
	}

	UpdateData();

	bool ok = topic->SetTitle(ToUTF8(m_strTitle).c_str());
	ok = topic->SetDescription(ToUTF8(m_strDescription).c_str());
	ok = topic->SetTopicType(ToUTF8(m_strTopicType).c_str()) && ok;
	ok = topic->SetStage(ToUTF8(m_strTopicStage).c_str()) && ok;
	ok = topic->SetTopicStatus(ToUTF8(m_strTopicStatus).c_str()) && ok;
	ok = topic->SetAssignedTo(ToUTF8(m_strAssigned).c_str()) && ok;
	ok = topic->SetPriority(ToUTF8(m_strPriority).c_str()) && ok;
	ok = topic->SetDueDate(ToUTF8(m_strDue).c_str()) && ok;
	if (m_strIndex.IsEmpty()) {
		ok = topic->SetIndexStr(ToUTF8(m_strIndex).c_str()) && ok;
	}
	else {
		ok = topic->SetIndex(_wtoi(m_strIndex)) && ok;
	}
	ok = topic->SetServerAssignedId(ToUTF8(m_strServerId).c_str()) && ok;

	ShowLog(!ok);
	if (!ok) {
		LoadActiveTopic(); //restore data was not set
	}

	FillTopicAuthor(topic);
	LoadExtensions();
}


void CBCFView::LoadComments(BCFTopic* topic, int select)
{
	m_wndCommentsList.ResetContent();

	uint16_t i = 0;
	while (auto comment = topic->GetComment(i++)) {
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

	m_wndCommentsList.SetCurSel(select);
	OnSelchangeCommentsList();
}

void CBCFView::UpateActiveComment()
{
	auto topic = GetActiveTopic();
	if (!topic) {
		return;
	}

	auto indComment = m_wndCommentsList.GetCurSel();
	auto comment = (BCFComment*)m_wndCommentsList.GetItemDataPtr(indComment);

	CString newText;
	m_wndCommentText.GetWindowText(newText);
	newText.Trim();

	bool ok = true;

	if (!comment && !newText.IsEmpty()) {
		comment = topic->AddComment();
		if (comment)
			ok = m_viewPointMgr.SaveCurrentViewToComent(*comment) && ok;
		else
			ok = false;
	}

	if (comment) {
		ok = comment->SetText(ToUTF8(newText).c_str()) && ok;
	}

	ShowLog(!ok);

	LoadComments(topic, indComment);
}


void CBCFView::OnSelchangeCommentsList()
{
	BCFComment* comment = NULL;
	if (m_bcfProject) {

		auto item = m_wndCommentsList.GetCurSel();
		comment = (BCFComment*)m_wndCommentsList.GetItemDataPtr(item);

		if (comment) {
			CString strCommentText = FromUTF8(comment->GetText());
			m_wndCommentText.SetWindowText(strCommentText);

			m_viewPointMgr.SetViewFromComment(*comment);
		}
		else {
			m_wndCommentText.SetWindowText(L""); //new comment
		}
	}

	m_wndUpdateViewPoint.EnableWindow(comment != NULL);
	
	ShowLog(false);
}

_model* CBCFView::GetBimModel(BCFBimFile& file)
{
	_model* model = NULL;

	auto it = m_mapBimFiles.find(&file);
	if (it != m_mapBimFiles.end()) {

		model = it->second;
	}
	else {

		auto path = FromUTF8(file.GetReference());
		
		for (auto m : m_loadedModels) {
			if (m && m->getPath() == path) {
				model = m;
				break;
			}
		}

		if (!model) {
			model = _ap_model_factory::load(path, true, nullptr, false);
			//model may be NULL, assume message was shown while load
			if (model) {
				m_loadedModels.push_back(model);
			}
		}

		m_mapBimFiles[&file] = model;
	}

	if (model) {
		auto title = file.GetFilename();
		model->setTitle(FromUTF8(title));
	}

	return model;
}

void CBCFView::ViewTopicModels(BCFTopic* topic)
{
	std::vector<_model*> activeModels;

	if (topic) {
		uint16_t i = 0;
		while (BCFBimFile* file = topic->GetBimFile(i++)) {

			_model* model = GetBimModel(*file);

			if (model) {
				activeModels.push_back(model);
			}
		}
	}

	m_doc.deleteAllModels();
	m_doc.addModels(activeModels);
}


CMySTEPViewerView* CBCFView::GetViewerView()
{
	auto pos = m_doc.GetFirstViewPosition();
	while (auto view = m_doc.GetNextView(pos)) {
		auto stview = dynamic_cast<CMySTEPViewerView*>(view);
		if (stview) {
			return stview;
		}
	}
	return NULL;
}

void CBCFView::OnSelchangeTab(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	FillMultiList();
	*pResult = 0;
}

void CBCFView::FillMultiList()
{
	auto sel = m_wndMultiList.GetCurSel();
	m_wndMultiList.ResetContent();

	auto topic = GetActiveTopic();
	
	if (topic) {
		switch (m_wndTab.GetCurSel()) {
		case TAB_Labels:
			FillLabels(topic);
			break;
		case TAB_Related:
			FillRelated(topic);
			break;
		case TAB_Links:
			FillLinks(topic);
			break;
		case TAB_Documents:
			FillDocuments(topic);
			break;
		default:
			ASSERT(FALSE);
		}
	}

	m_wndAddMulti.EnableWindow(topic != NULL);
	
	if (sel != LB_ERR) {
		m_wndMultiList.SetCurSel(sel);
	}
	OnSelchangeMultiList();
}

void CBCFView::FillLabels(BCFTopic* topic)
{
	m_wndMultiList.ResetContent();
	int i = 0;
	while (auto label = topic->GetLabel(i++)) {
		m_wndMultiList.AddString(FromUTF8(label));
	}
}

void CBCFView::FillRelated(BCFTopic* topic)
{
	int i = 0;
	while (auto related = topic->GetRelatedTopic(i++)) {
		auto text = GetTopicDisplayName(*related);
		auto item = m_wndMultiList.AddString(text);
		m_wndMultiList.SetItemDataPtr(item, related);
	}
}

void CBCFView::FillLinks(BCFTopic* topic)
{
	int i = 0;
	while (auto link = topic->GetReferenceLink(i++)) {
		m_wndMultiList.AddString(FromUTF8(link));
	}
}

static CString GetDocumentText(BCFDocumentReference* doc)
{
	CString text = FromUTF8(doc->GetDescription());
	if (!text.IsEmpty()) {
		text.Append(L": ");
	}
	fs::path path = doc->GetFilePath();
	text += FromUTF8(path.filename().string().c_str());

	return text;
}

void CBCFView::FillDocuments(BCFTopic* topic)
{
	int i = 0;
	while (auto doc = topic->GetDocumentReference(i++)) {
		auto item = m_wndMultiList.AddString(GetDocumentText(doc));
		m_wndMultiList.SetItemDataPtr(item, doc);
	}
}


void CBCFView::OnClickedButtonAddMulti()
{
	auto item = m_wndTopics.GetCurSel();
	auto topic = (BCFTopic*)m_wndTopics.GetItemData(item);

	if (topic) {
		switch (m_wndTab.GetCurSel()) {
		case TAB_Labels:
			AddLabel(topic);
			break;
		case TAB_Related:
			AddRelated(topic);
			break;
		case TAB_Links:
			AddLink(topic);
			break;
		case TAB_Documents:
			AddDocument(topic);
			break;
		default:
			ASSERT(FALSE);
		}

		FillMultiList();
	}
}

void CBCFView::AddLabel(BCFTopic* topic)
{
	CBCFAddLabel dlg(*this);
	dlg.DoModal();
}

void CBCFView::AddRelated(BCFTopic* topic)
{
	CBCFAddRelatedTopic dlg(*this);
	dlg.DoModal();
}

void CBCFView::AddLink(BCFTopic* topic)
{
	CBCFAddReferenceLink dlg(*this);
	dlg.DoModal();
}

void CBCFView::AddDocument(BCFTopic* topic)
{
	CBCFAddDocumentReference dlg(*this);
	dlg.DoModal();
}

void CBCFView::OnClickedButtonRemoveMulti()
{
	auto item = m_wndTopics.GetCurSel();
	auto topic = (BCFTopic*)m_wndTopics.GetItemData(item);

	if (topic) {
		switch (m_wndTab.GetCurSel()) {
		case TAB_Labels:
			RemoveLabel(topic);
			break;
		case TAB_Related:
			RemoveRelated(topic);
			break;
		case TAB_Links:
			RemoveLink(topic);
			break;
		case TAB_Documents:
			RemoveDocument(topic);
			break;
		default:
			ASSERT(FALSE);
		}

		FillMultiList();
	}
}

void CBCFView::RemoveLabel(BCFTopic* topic)
{
	auto sel = m_wndMultiList.GetCurSel();
	if (sel != LB_ERR) {
		CString label;
		m_wndMultiList.GetText(sel, label);
		if (IDYES == AfxMessageBox(L"Do you want to delete label " + label + L"?", MB_YESNO)) {
			if (!topic->RemoveLabel(ToUTF8(label).c_str())) {
				ShowLog(true);
			}
		}
	}
}

void CBCFView::RemoveRelated(BCFTopic* topic)
{
	auto sel = m_wndMultiList.GetCurSel();
	if (sel != LB_ERR) {
		auto t = (BCFTopic*)m_wndMultiList.GetItemDataPtr(sel);
		if (t) {
			if (!topic->RemoveRelatedTopic(t)) {
				ShowLog(true);
			}
		}
	}
}

void CBCFView::RemoveLink(BCFTopic* topic)
{
	auto sel = m_wndMultiList.GetCurSel();
	if (sel != LB_ERR) {
		CString link;
		m_wndMultiList.GetText(sel, link);
		if (IDYES == AfxMessageBox(L"Do you want to delete reference link '" + link + L"'?", MB_YESNO)) {
			if (!topic->RemoveReferenceLink(ToUTF8(link).c_str())) {
				ShowLog(true);
			}
		}
	}
}

void CBCFView::RemoveDocument(BCFTopic* topic)
{
	auto sel = m_wndMultiList.GetCurSel();
	if (sel != LB_ERR) {
		auto doc = (BCFDocumentReference*)m_wndMultiList.GetItemDataPtr(sel);
		if (doc) {
			CString quest;
			quest.Format(L"Do you want to remove reference to document '%s'?", GetDocumentText(doc).GetString());
			if (IDYES == AfxMessageBox(quest, MB_YESNO)) {
				if (!doc->Remove()) {
					ShowLog(true);
				}
			}
		}
	}
}



void CBCFView::OnSelchangeMultiList()
{
	m_wndRemoveMulti.EnableWindow(m_wndMultiList.GetCurSel() != LB_ERR);
}


void CBCFView::OnClickedProjectInfo()
{
	if (m_bcfProject) {
		CBCFProjInfo projInfo(*m_bcfProject, this);
		projInfo.DoModal();
	}
}

void CBCFView::OnKillfocusEdit()
{
	UpdateActiveTopic();
}

void CBCFView::OnKillfocusTopicCommentText()
{
	UpateActiveComment();
}


void CBCFView::OnClickedSave()
{
	SaveBCFFile();
}

void CBCFView::SaveBCFFile()
{
	if (!m_bcfProject) {
		return;
	}

	LPCTSTR fileTypes = _T("BCF files (*.bcf)|*.bcf|All Files (*.*)|*.*||");

	CFileDialog dlgFile(FALSE, L"bcf", _T(""), OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, fileTypes);
	if (dlgFile.DoModal() != IDOK)
	{
		return;
	}

	m_bcfFilePath = dlgFile.GetPathName();

	bool ok = m_bcfProject->WriteFile(ToUTF8(m_bcfFilePath).c_str(), BCFVer_3_0);

	ShowLog(!ok);

	LoadProjectToView();
}

void CBCFView::OnClickedUpdateViewpoint()
{
	auto indComment = m_wndCommentsList.GetCurSel();
	if (auto comment = (BCFComment*)m_wndCommentsList.GetItemDataPtr(indComment)) {
		bool ok = m_viewPointMgr.SaveCurrentViewToComent(*comment);
		ShowLog(!ok);
	}
}

CString CBCFView::GetTopicDisplayName(BCFTopic& topic)
{
	int i = 0;
	auto& bcfProject = topic.GetProject();
	while (auto t = bcfProject.GetTopic(i++)) {
		if (t == &topic) {
			break;
		}
	}

	auto guid = FromUTF8(topic.GetGuid());
	auto title = FromUTF8(topic.GetTitle());

	CString text;
	text.Format(L"#%d: %s - %s", i, guid.GetString(), title.GetString());

	return text;
}


void CBCFView::OnClickedButtonBims()
{
	CBCFBimFiles dlg(*this);
	dlg.DoModal();
}
