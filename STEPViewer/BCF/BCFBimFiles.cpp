// W:\DevArea\RDF\stepviewer_v2\STEPViewer\BCF\BCFBimFiles.cpp : implementation file
//

#include "stdafx.h"
#include "STEPViewer.h"
#include "STEPViewerDoc.h"
#include "_mvc.h"
#include "_ap_model_factory.h"
#include "BCFBimFiles.h"
#include "BCFView.h"

// CBCFBimFiles dialog

IMPLEMENT_DYNAMIC(CBCFBimFiles, CDialogEx)

CBCFBimFiles::CBCFBimFiles(CBCFView& bcfView)
	: CDialogEx(IDD_BCF_TOPICFILES, &bcfView)
	, m_view(bcfView)
{

}

CBCFBimFiles::~CBCFBimFiles()
{
}

void CBCFBimFiles::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FILE_LIST, m_fileList);
}


BEGIN_MESSAGE_MAP(CBCFBimFiles, CDialogEx)
	ON_CLBN_CHKCHANGE(IDC_FILE_LIST, &CBCFBimFiles::OnCheckFileList)
END_MESSAGE_MAP()



BOOL CBCFBimFiles::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	auto topic = m_view.GetActiveTopic();
	ASSERT(topic);
	if (!topic) {
		EndDialog(IDCANCEL);
		return false;
	}

	FillFileList(*topic);

	CString title;
	title.Format(L"BIM files for topic %s", m_view.GetTopicDisplayName(*topic).GetString());
	SetWindowText(title);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CBCFBimFiles::FillFileList(BCFTopic& topic)
{
	//get used models
	m_usedModels.clear();
	int i = 0;
	while (BCFBimFile* file = topic.GetBimFile(i++)) {
		if (auto model = m_view.GetBimModel(*file)) {
			auto& f = m_usedModels[model];
			ASSERT(!f);
			f = file;
		}
	}

	//refill list
	m_fileList.SetRedraw(FALSE);

	auto sel = m_fileList.GetCurSel();
	auto top = m_fileList.GetTopIndex();

	m_fileList.ResetContent();

	for (auto model : m_view.GetViewerDoc().getModels()) {
		if (model) {
			auto item = m_fileList.AddString(model->getPath());
			m_fileList.SetItemDataPtr(item, model);
			m_fileList.SetCheck(item, m_usedModels.find(model) != m_usedModels.end());
		}
	}

	if (sel != LB_ERR)
		m_fileList.SetCurSel(sel);

	if (top != LB_ERR)
		m_fileList.SetTopIndex(top);

	m_fileList.SetRedraw(TRUE);
}

void CBCFBimFiles::OnOK()
{
	auto topic = m_view.GetActiveTopic();
	ASSERT(topic);
	if (!topic)
		return;

	TCHAR szFilters[] = _T("BIM Files (*.ifc;*.ifczip)|*.ifc;*.ifczip|All Files (*.*)|*.*||");
	CFileDialog dlgFile(TRUE, NULL, NULL, OFN_FILEMUSTEXIST, szFilters);
	if (dlgFile.DoModal() != IDOK)
		return;

	auto path = dlgFile.GetPathName();

	auto model = _ap_model_factory::load(path, false, !m_view.GetViewerDoc().getModels().empty() ? m_view.GetViewerDoc().getModels()[0] : nullptr, false);
	//model may be NULL, assume message was shown while load
	if (!model)
		return;
		
	m_view.GetViewerDoc().addModel(model);

	AddBimFile(*topic, *model);

	FillFileList(*topic);

	m_view.ViewTopicModels(topic);

	m_view.ShowLog(false);
		
	//CDialogEx::OnOK();
}

void CBCFBimFiles::AddBimFile(BCFTopic& topic, _model& model)
{
	auto file = topic.AddBimFile(ToUTF8(model.getPath()).c_str(), false);
	if (!file) {
		m_view.ShowLog(true);
		return;
	}
}

void CBCFBimFiles::OnCheckFileList()
{
	auto topic = m_view.GetActiveTopic();
	ASSERT(topic);
	if (!topic)
		return;

	for (int i = 0; i < m_fileList.GetCount(); i++) {
		auto model = (_model*)m_fileList.GetItemDataPtr(i);
		if (model) {
			auto found = m_usedModels.find(model);
			if (m_fileList.GetCheck(i)) {
				if (found == m_usedModels.end()) {

					AddBimFile(*topic, *model);
					FillFileList(*topic);
					m_view.ViewTopicModels(topic);
				}
			}
			else {
				if (found != m_usedModels.end()) {

					found->second->Remove();
					FillFileList(*topic);
					m_view.ViewTopicModels(topic);
				}
			}
		}
	}
}
