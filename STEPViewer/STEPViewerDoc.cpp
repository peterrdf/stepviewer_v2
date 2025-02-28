
// STEPViewerDoc.cpp : implementation of the CMySTEPViewerDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "STEPViewer.h"
#endif

#include "STEPViewerDoc.h"
#include "_ap_model_factory.h"
#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ************************************************************************************************
TCHAR OPEN_BCF_FILTER[] = _T("BCF Packages (*.bcf; *.bcfzip)|*.bcf; *.bcfzip|All Files (*.*)|*.*||");
TCHAR OPEN_FILES_FILTER[] = _T("Supported files (*.stp; *.step; *.stpz; *.ifc; *.ifczip; *.bcf; *.bcfzip)|*.stp; *.step; *.stpz; *.ifc; *.ifczip; *.bcf; *.bcfzip|All Files (*.*)|*.*||");
TCHAR OPEN_MODEL_FILTER[] = _T("Design model files (*.stp; *.step; *.stpz; *.ifc; *.ifczip)|*.stp; *.step; *.stpz; *.ifc; *.ifczip|All Files (*.*)|*.*||");
TCHAR SAVE_IFC_FILTER[] = _T("IFC Files (*.ifc)|*.ifc|All Files (*.*)|*.*||");
TCHAR SAVE_STEP_FILTER[] = _T("STEP Files (*.step)|*.step|All Files (*.*)|*.*||");
TCHAR SAVE_CIS2_FILTER[] = _T("CIS2 Files (*.stp)|*.stp|All Files (*.*)|*.*||");

// ************************************************************************************************
/*virtual*/ void CMySTEPViewerDoc::saveInstance(_instance* pInstance) /*override*/
{
	if (pInstance == nullptr)
	{
		return;
	} 

	CString strValidFileName = validateFileName(dynamic_cast<_ap_instance*>(pInstance)->getName().c_str()).c_str();

	TCHAR szFilters[] = _T("BIN Files (*.bin)|*.bin|All Files (*.*)|*.*||");
	CFileDialog dlgFile(FALSE, _T("bin"), strValidFileName,
		OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, szFilters);

	if (dlgFile.DoModal() != IDOK)
	{
		return;
	}

	auto pAPInstance = dynamic_cast<_ap_instance*>(pInstance);
	if (pAPInstance == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	pAPInstance->saveInstance((LPCWSTR)dlgFile.GetPathName());
}

void CMySTEPViewerDoc::OpenModels(const vector<CString>& vecPaths)
{
	m_wndBCFView.Close();

	setModel(nullptr);

	vector<_model*> vecModels;
	for (auto strPath : vecPaths)
	{
		auto pModel = _ap_model_factory::load(strPath, vecPaths.size() > 1, !vecModels.empty() ? vecModels.front() : nullptr, false);
		if ((vecPaths.size() > 1) && (dynamic_cast<_ifc_model*>(pModel) == nullptr))
		{
			delete pModel;

			continue;
		}

		vecModels.push_back(pModel);
	}

	if (!vecModels.empty())
	{
		setModels(vecModels);
	}
}


// ************************************************************************************************
// CMySTEPViewerDoc

IMPLEMENT_DYNCREATE(CMySTEPViewerDoc, CDocument)

BEGIN_MESSAGE_MAP(CMySTEPViewerDoc, CDocument)
	ON_COMMAND(ID_FILE_OPEN, &CMySTEPViewerDoc::OnFileOpen)
	ON_COMMAND(ID_VIEW_ZOOM_OUT, &CMySTEPViewerDoc::OnViewZoomOut)
	ON_COMMAND(ID_VIEW_MODEL_CHECKER, &CMySTEPViewerDoc::OnViewModelChecker)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MODEL_CHECKER, &CMySTEPViewerDoc::OnUpdateViewModelChecker)
	ON_COMMAND(ID_FILE_SAVE, &CMySTEPViewerDoc::OnFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, &CMySTEPViewerDoc::OnUpdateFileSave)
	ON_COMMAND(ID_FILE_SAVE_AS, &CMySTEPViewerDoc::OnFileSaveAs)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, &CMySTEPViewerDoc::OnUpdateFileSaveAs)
	ON_COMMAND(ID_BCF_ADDBIM, &CMySTEPViewerDoc::OnBcfAddbim)
	ON_UPDATE_COMMAND_UI(ID_BCF_ADDBIM, &CMySTEPViewerDoc::OnUpdateBcfAddbim)
	ON_COMMAND(ID_BCF_NEW, &CMySTEPViewerDoc::OnBcfNew)
	ON_UPDATE_COMMAND_UI(ID_BCF_NEW, &CMySTEPViewerDoc::OnUpdateBcfNew)
	ON_COMMAND(ID_BCF_OPEN, &CMySTEPViewerDoc::OnBcfOpen)
	ON_UPDATE_COMMAND_UI(ID_BCF_OPEN, &CMySTEPViewerDoc::OnUpdateBcfOpen)
END_MESSAGE_MAP()


// CMySTEPViewerDoc construction/destruction

CMySTEPViewerDoc::CMySTEPViewerDoc()
	: m_wndBCFView(*this)
{
}

CMySTEPViewerDoc::~CMySTEPViewerDoc()
{	
	TRACE(L"CMySTEPViewerDoc::~CMySTEPViewerDoc()");
}

BOOL CMySTEPViewerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;	

	m_wndBCFView.Close();

	setModel(nullptr);

	return TRUE;
}

// CMySTEPViewerDoc serialization

void CMySTEPViewerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CMySTEPViewerDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CMySTEPViewerDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CMySTEPViewerDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = nullptr;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != nullptr)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CMySTEPViewerDoc diagnostics

#ifdef _DEBUG
void CMySTEPViewerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CMySTEPViewerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CMySTEPViewerDoc commands


BOOL CMySTEPViewerDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	m_wndBCFView.Close();

	setModel(_ap_model_factory::load(lpszPathName, false, nullptr, false));

	// Title
	CString strTitle = AfxGetAppName();
	strTitle += L" - ";
	strTitle += lpszPathName;
	AfxGetMainWnd()->SetWindowTextW(strTitle);

	// MRU
	AfxGetApp()->AddToRecentFileList(lpszPathName);

	return TRUE;
}

void CMySTEPViewerDoc::OnFileOpen()
{
	CFileDialog dlgFile(TRUE, nullptr, _T(""), OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT, OPEN_MODEL_FILTER);
	if (dlgFile.DoModal() != IDOK)
	{
		return;
	}

	vector<CString> vecModels;
	POSITION pos(dlgFile.GetStartPosition());
	while (pos != nullptr)
	{
		CString strFileName = dlgFile.GetNextPathName(pos);
		vecModels.push_back(strFileName);

		// MRU
		AfxGetApp()->AddToRecentFileList(vecModels[0]);
	}

	OpenModels(vecModels);

	// Title
	CString strTitle = AfxGetAppName();
	strTitle += L" - ";
	strTitle += vecModels[0];
	strTitle += vecModels.size() > 1 ? L", ..." : L"";
	AfxGetMainWnd()->SetWindowTextW(strTitle);	
}

void CMySTEPViewerDoc::OnViewZoomOut()
{
	zoomOut();
}

void CMySTEPViewerDoc::DeleteContents()
{
	m_wndModelChecker.Hide(true);
	__super::DeleteContents();
}

void CMySTEPViewerDoc::OnCloseDocument()
{
	m_wndBCFView.Close();
	__super::OnCloseDocument();
}

BOOL CMySTEPViewerDoc::SaveModified()
{
	if (!m_wndBCFView.SaveModified()) {
		return FALSE;
	}

	return __super::SaveModified();
}

void CMySTEPViewerDoc::OnViewModelChecker()
{
	if (m_wndModelChecker.IsVisible()) {
		m_wndModelChecker.Hide(false);
	}
	else {
		m_wndModelChecker.Show();
	}
}

void CMySTEPViewerDoc::OnUpdateViewModelChecker(CCmdUI* pCmdUI)
{
	auto visible = m_wndModelChecker.IsVisible();
	pCmdUI->SetCheck(visible);
}

void CMySTEPViewerDoc::OnFileSave()
{
	_ptr<_ap_model> apModel(getModels()[0]);

	CString strFiler;
	CString strExtension;
	if (apModel->getAP() == enumAP::STEP)
	{
		strFiler = SAVE_STEP_FILTER;
		strExtension = L"ifc";
	}
	else if (apModel->getAP() == enumAP::IFC)
	{
		strFiler = SAVE_IFC_FILTER;
		strExtension = L"step";
	}
	else  if (apModel->getAP() == enumAP::CIS2)
	{
		strFiler = SAVE_CIS2_FILTER;
		strExtension = L"stp";
	}
	else
	{
		ASSERT(FALSE);

		return;
	}

	CFileDialog dlgFile(FALSE, strExtension, apModel->getPath(), OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, strFiler);
	if (dlgFile.DoModal() != IDOK)
	{
		return;
	}

	sdaiSaveModelBNUnicode(apModel->getSdaiModel(), (LPCWSTR)dlgFile.GetPathName());
}

void CMySTEPViewerDoc::OnUpdateFileSave(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(getModels().size() == 1);
}

void CMySTEPViewerDoc::OnFileSaveAs()
{
	OnFileSave();
}

void CMySTEPViewerDoc::OnUpdateFileSaveAs(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(getModels().size() == 1);
}

void CMySTEPViewerDoc::OnBcfAddbim()
{
	CFileDialog dlgFile(TRUE, nullptr, _T(""), OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT, SAVE_IFC_FILTER);
	if (dlgFile.DoModal() != IDOK)
	{
		return;
	}

	POSITION pos(dlgFile.GetStartPosition());
	while (pos != nullptr)
	{
		CString strPath = dlgFile.GetNextPathName(pos);

		auto pModel = _ap_model_factory::load(strPath, false, !getModels().empty() ? getModels()[0] : nullptr, false);
		if (pModel->getAP() != enumAP::IFC)
		{
			delete pModel;

			continue;
		}

		m_wndBCFView.GetActiveTopic()->AddBimFile(ToUTF8(pModel->getPath()).c_str(), false);

		// MRU
		AfxGetApp()->AddToRecentFileList(strPath);
	}

	m_wndBCFView.ViewTopicModels(m_wndBCFView.GetActiveTopic());
}

void CMySTEPViewerDoc::OnUpdateBcfAddbim(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_wndBCFView.GetActiveTopic() != NULL);
}

void CMySTEPViewerDoc::OnBcfNew()
{
	m_wndBCFView.Open(NULL);

	setModel(nullptr);
}

void CMySTEPViewerDoc::OnUpdateBcfNew(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_wndBCFView.GetSafeHwnd() || !m_wndBCFView.IsWindowVisible());
}

void CMySTEPViewerDoc::OnBcfOpen()
{
	CFileDialog dlgFile(TRUE, nullptr, _T(""), OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, OPEN_BCF_FILTER);
	if (dlgFile.DoModal() != IDOK)
	{
		return;
	}

	CString strPath = dlgFile.GetPathName();
	if (m_wndBCFView.IsBCF(strPath))
	{
		m_wndBCFView.Open(strPath);
	}
}

void CMySTEPViewerDoc::OnUpdateBcfOpen(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}
