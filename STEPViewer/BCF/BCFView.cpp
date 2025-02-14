// BCF\BCFView.cpp : implementation file
//

#include "stdafx.h"

#include "STEPViewer.h"
#include "STEPViewerDoc.h"
#include "STEPViewerView.h"
#include "_ap_model_factory.h"
#include "BCF\BCFProjInfo.h"
#include "BCF\BCFView.h"

 
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
END_MESSAGE_MAP()


CBCFView::CBCFView(CMySTEPViewerDoc& doc)
	: CDialogEx(IDD_BCF, AfxGetMainWnd())
	, m_doc (doc)
	, m_bcfProject(NULL)
{
}

CBCFView::~CBCFView()
{
	ASSERT(!m_bcfProject);
	DeleteContent();
}

void CBCFView::DeleteContent()
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

		if (GetSafeHwnd()) {
			ShowWindow(SW_HIDE);
		}
	}
}

void CBCFView::SetModelsExternallyManaged(std::vector <_model*>& models)
{
	//do not delete models, just set new list
	m_doc.editModelList().clear();
	m_doc.addModels(models);
}

bool CBCFView::CreateNewProject()
{
	if (!m_bcfProject) {
		m_bcfProject = BCFProject::Create();
		if (!m_bcfProject) {
			AfxMessageBox(L"Failed to initialize BCF Project");
		}
		return m_bcfProject != NULL;
	}
	else {
		return false;
	}
}

bool CBCFView::ReadBCFFile(LPCTSTR bcfFilePath)
{
	fs::path path = bcfFilePath;
	auto ext = path.extension();
	if (ext != ".bcf" && ext != ".bcfzip") {
		return false;
	}

	bool isnew = CreateNewProject();

	if (!m_bcfProject->ReadFile(ToUTF8(bcfFilePath).c_str(), true))
	{
		ShowLog(true);
		if (isnew) {
			DeleteContent();
		}
		return false;
	}

	ShowLog(false);
	m_bcfFilePath = bcfFilePath;
	return true;
}

bool CBCFView::Show()
{
	if (!m_bcfProject) {
		return false;
	}

	CBCFProjInfo projInfo(*m_bcfProject, GetView());
	if (IDOK != projInfo.DoModal()) {
		DeleteContent();
		return false;
	}

	//create new, at least one topic is required
	if (!m_bcfProject->TopicGetAt(0)) {
		CreateNewTopic();
	}

	// prepare model views
	//
	std::swap(m_preloadedModels, m_doc.editModelList());
	m_doc.setModel(NULL);

	//show view
	if (!IsWindow(GetSafeHwnd())) {
		Create(IDD_BCF, AfxGetMainWnd());
	}
	ShowWindow(SW_SHOW);

	//
	CString title;
	title.Format(L"BCF-XML %s", m_bcfFilePath.IsEmpty() ? L"<New>" : m_bcfFilePath);
	SetWindowText(title);

	LoadProjectToView();

	return true;
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
}

void CBCFView::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);
	if (!bShow) {
		DeleteContent();
	}
}

void CBCFView::OnClose()
{
	CDialogEx::OnClose();
	DeleteContent();
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


	//load extensions
	//
	LoadExtensions();

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
	auto index = m_wndTopics.GetCurSel();
	auto topic = (BCFTopic*)m_wndTopics.GetItemData(index);

	if (!topic) {
		if (IDYES == AfxMessageBox(L"Do you want to create new topic?", MB_ICONQUESTION | MB_YESNO)) {
			topic = CreateNewTopic();
			if (topic) {
				InsertTopicToList(index, topic);
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
		modifier.Format(L", modified by % s at % s", FromUTF8(topic->GetModifiedAuthor()).GetString(), FromUTF8(topic->GetModifiedDate()).GetString());
		strAuthor.Append(modifier);
	}

	m_wndAuthor.SetWindowText(strAuthor);
}

BCFTopic* CBCFView::CreateNewTopic()
{
	if (!m_bcfProject) {
		return NULL;
	}

	auto topic = m_bcfProject->TopicAdd(NULL, NULL, NULL);

	for (auto model : m_doc.getModels()) {
		auto path = model->getPath();
		topic->FileAdd(ToUTF8(path).c_str(), false);
	}

	ShowLog(false);

	return topic;
}

void CBCFView::LoadActiveTopic()
{
	auto topic = GetActiveTopic();
	if (!topic) {
		return;
	}

	SetActiveModels(topic);

	FillTopicAuthor(topic);

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
	ok = topic->SetIndex(_wtoi(m_strIndex)) && ok;
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
		ok = CreateNewComment(topic, newText);
	}
	else if (comment) {
		ok = comment->SetText(ToUTF8(newText).c_str());
	}

	ShowLog(!ok);

	LoadComments(topic, indComment);
}

bool CBCFView::CreateNewComment(BCFTopic* topic, CString& text)
{
	auto comment = topic->CommentAdd();

	if (comment) {
		
		//TODO - save viewpoint

		return comment->SetText(ToUTF8(text).c_str());
	}

	return false;
}

void CBCFView::OnSelchangeCommentsList()
{
	if (!m_bcfProject) {
		return;
	}
	auto item = m_wndCommentsList.GetCurSel();
	auto comment = (BCFComment*)m_wndCommentsList.GetItemDataPtr(item);

	if (comment) {
		CString strCommentText = FromUTF8(comment->GetText());
		m_wndCommentText.SetWindowText(strCommentText);

		if (auto vp = comment->GetViewPoint()) {
			SetActiveViewPoint(vp);
		}
	}
	else {
		m_wndCommentText.SetWindowText(L""); //new comment
	}
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

void CBCFView::SetActiveViewPoint(BCFViewPoint* vp)
{
	if (vp) {
		if (auto view = GetView()) {
			BCFCamera camera = vp->GetCameraType();
			BCFPoint viewPoint;
			BCFPoint direction;
			BCFPoint upVector;
			vp->GetCameraViewPoint(viewPoint);
			vp->GetCameraDirection(direction);
			vp->GetCameraUpVector(upVector);
			double viewToWorldScale = vp->GetViewToWorldScale();
			double fieldOfView = vp->GetFieldOfView();
			double aspectRatio = vp->GetAspectRatio();

			view->SetBCFView(camera, viewPoint, direction, upVector, viewToWorldScale, fieldOfView, aspectRatio);
		}
	}
}

CMySTEPViewerView* CBCFView::GetView()
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
	
	OnSelchangeMultiList();
}

void CBCFView::FillLabels(BCFTopic* topic)
{
	m_wndMultiList.ResetContent();
	int i = 0;
	while (auto label = topic->LabelGetAt(i++)) {
		m_wndMultiList.AddString(FromUTF8(label));
	}
}

void CBCFView::FillRelated(BCFTopic* topic)
{
	int i = 0;
	while (auto related = topic->RelatedTopicGetAt(i++)) {
		auto item = m_wndMultiList.AddString(FromUTF8(related->GetTitle()));
		m_wndMultiList.SetItemDataPtr(item, related);
	}
}

void CBCFView::FillLinks(BCFTopic* topic)
{
	int i = 0;
	while (auto link = topic->ReferenceLinkGetAt(i++)) {
		m_wndMultiList.AddString(FromUTF8(link));
	}
}

void CBCFView::FillDocuments(BCFTopic* topic)
{
	int i = 0;
	while (auto doc = topic->DocumentReferenceGetAt(i++)) {
		CString text = FromUTF8(doc->GetDescription());
		if (!text.IsEmpty()) {
			text.Append(L": ");
		}
		fs::path path = doc->GetFilePath();
		text += FromUTF8(path.filename().string().c_str());
		auto item = m_wndMultiList.AddString(text);
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
			break;
		case TAB_Related:
			break;
		case TAB_Links:
			break;
		case TAB_Documents:
			break;
		default:
			ASSERT(FALSE);
		}
	}
}


void CBCFView::OnClickedButtonRemoveMulti()
{
	auto item = m_wndTopics.GetCurSel();
	auto topic = (BCFTopic*)m_wndTopics.GetItemData(item);

	if (topic) {
		switch (m_wndTab.GetCurSel()) {
		case TAB_Labels:
			break;
		case TAB_Related:
			break;
		case TAB_Links:
			break;
		case TAB_Documents:
			break;
		default:
			ASSERT(FALSE);
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
