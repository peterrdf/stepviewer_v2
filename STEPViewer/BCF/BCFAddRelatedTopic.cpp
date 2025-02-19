// W:\DevArea\RDF\stepviewer_v2\STEPViewer\BCF\BCFAddRelatedTopic.cpp : implementation file
//

#include "stdafx.h"
#include "STEPViewer.h"
#include "BCFAddRelatedTopic.h"
#include "BCFView.h"


IMPLEMENT_DYNAMIC(CBCFAddRelatedTopic, CDialogEx)

CBCFAddRelatedTopic::CBCFAddRelatedTopic(CBCFView& bcfView)
	: CDialogEx(IDD_ADDRELATEDTOPIC, &bcfView)
	, m_view(bcfView)
{

}

CBCFAddRelatedTopic::~CBCFAddRelatedTopic()
{
}

void CBCFAddRelatedTopic::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_TOPIC, m_wndListTopic);
	DDX_Control(pDX, IDOK, m_wndOK);
}


BEGIN_MESSAGE_MAP(CBCFAddRelatedTopic, CDialogEx)
	ON_LBN_SELCHANGE(IDC_LIST_TOPIC, &CBCFAddRelatedTopic::OnSelchangeListTopic)
END_MESSAGE_MAP()


// CBCFAddRelatedTopic message handlers


BOOL CBCFAddRelatedTopic::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	auto topic = m_view.GetActiveTopic();
	if (!topic) {
		ASSERT(0);
		EndDialog(IDCANCEL);
	}

	std::set<BCFTopic*> exist;
	int i = 0;
	while (auto t = topic->GetRelatedTopic(i++)) {
		exist.insert(t);
	}
	exist.insert(topic);

	auto& bcfProject = topic->GetProject();
	i = 0;
	while (auto t = bcfProject.GetTopic(i++)) {
		if (exist.find(t) == exist.end()) {
			CString text = FormatText(*t);
			auto item = m_wndListTopic.AddString(text);
			m_wndListTopic.SetItemDataPtr(item, t);
		}
	}

	OnSelchangeListTopic();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

CString CBCFAddRelatedTopic::FormatText(BCFTopic& topic)
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


void CBCFAddRelatedTopic::OnOK()
{
	auto item = m_wndListTopic.GetCurSel();
	if (item != LB_ERR) {
		auto t = (BCFTopic*)m_wndListTopic.GetItemData(item);
		if (t) {
			if (auto topic = m_view.GetActiveTopic()) {
				topic->AddRelatedTopic(t);
			}
		}
	}

	CDialogEx::OnOK();
}


void CBCFAddRelatedTopic::OnSelchangeListTopic()
{
	bool enable = false;
	auto item = m_wndListTopic.GetCurSel();
	if (item != LB_ERR) {
		auto t = (BCFTopic*)m_wndListTopic.GetItemData(item);
		if (t) {
			if (auto topic = m_view.GetActiveTopic()) {
				enable = true;
			}
		}
	}
	m_wndOK.EnableWindow(enable);
}
