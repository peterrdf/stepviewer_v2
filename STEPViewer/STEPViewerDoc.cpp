
// STEPViewerDoc.cpp : implementation of the CMySTEPViewerDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "STEPViewer.h"
#endif

#include "STEPViewerDoc.h"
#include "ModelFactory.h"
#include "AP242ProductInstance.h"
#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/*virtual*/ void CMySTEPViewerDoc::saveInstance() /*override*/
{
	ASSERT(getModel() != nullptr);

	if (getSelectedInstance() == nullptr)
	{
		return;
	} 

	CString strValidPath = dynamic_cast<_ap_instance*>(getSelectedInstance())->getName().c_str();
	strValidPath.Replace(_T("\\"), _T("-"));
	strValidPath.Replace(_T("/"), _T("-"));
	strValidPath.Replace(_T(":"), _T("-"));
	strValidPath.Replace(_T("*"), _T("-"));
	strValidPath.Replace(_T("?"), _T("-"));
	strValidPath.Replace(_T("\""), _T("-"));
	strValidPath.Replace(_T("\""), _T("-"));
	strValidPath.Replace(_T("\""), _T("-"));
	strValidPath.Replace(_T("<"), _T("-"));
	strValidPath.Replace(_T(">"), _T("-"));
	strValidPath.Replace(_T("|"), _T("-"));

	TCHAR szFilters[] = _T("BIN Files (*.bin)|*.bin|All Files (*.*)|*.*||");
	CFileDialog dlgFile(FALSE, _T("bin"), strValidPath,
		OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, szFilters);

	if (dlgFile.DoModal() != IDOK)
	{
		return;
	}

	auto pProductInstance = dynamic_cast<CAP242ProductInstance*>(getSelectedInstance());
	if (pProductInstance != nullptr)
	{
		SdaiModel iSdaiModel = sdaiGetInstanceModel(dynamic_cast<_ap_instance*>(getSelectedInstance())->getSdaiInstance());
		ASSERT(iSdaiModel != 0);

		OwlModel iOwlModel = 0;
		owlGetModel(iSdaiModel, &iOwlModel);
		ASSERT(iOwlModel != 0);

		OwlInstance	iMatrixInstance = CreateInstance(GetClassByName(iOwlModel, "Matrix"));
		ASSERT(iMatrixInstance != 0);

		vector<double> vecMatrix
		{
			pProductInstance->getTransformationMatrix()->_11,
			pProductInstance->getTransformationMatrix()->_12,
			pProductInstance->getTransformationMatrix()->_13,
			pProductInstance->getTransformationMatrix()->_21,
			pProductInstance->getTransformationMatrix()->_22,
			pProductInstance->getTransformationMatrix()->_23,
			pProductInstance->getTransformationMatrix()->_31,
			pProductInstance->getTransformationMatrix()->_32,
			pProductInstance->getTransformationMatrix()->_33,
			pProductInstance->getTransformationMatrix()->_41,
			pProductInstance->getTransformationMatrix()->_42,
			pProductInstance->getTransformationMatrix()->_43,
		};

		SetDatatypeProperty(
			iMatrixInstance,
			GetPropertyByName(iOwlModel, "coordinates"),
			vecMatrix.data(),
			vecMatrix.size());

		OwlInstance iTransformationInstance = CreateInstance(GetClassByName(iOwlModel, "Transformation"));
		ASSERT(iTransformationInstance != 0);

		SetObjectProperty(
			iTransformationInstance,
			GetPropertyByName(iOwlModel, "object"),
			getSelectedInstance()->getOwlInstance());

		SetObjectProperty(
			iTransformationInstance,
			GetPropertyByName(iOwlModel, "matrix"),
			iMatrixInstance);

		SaveInstanceTreeW(iTransformationInstance, dlgFile.GetPathName());
	}
	else
	{
		SaveInstanceTreeW(getSelectedInstance()->getOwlInstance(), dlgFile.GetPathName());
	}
}

// ************************************************************************************************
TCHAR SUPPORTED_FILES[] = _T("STEP Files (*.stp; *.step; *.ifc)|*.stp;*.step; *.ifc|All Files (*.*)|*.*||");

// ************************************************************************************************
// CMySTEPViewerDoc

IMPLEMENT_DYNCREATE(CMySTEPViewerDoc, CDocument)

BEGIN_MESSAGE_MAP(CMySTEPViewerDoc, CDocument)
	ON_COMMAND(ID_FILE_OPEN, &CMySTEPViewerDoc::OnFileOpen)
	ON_COMMAND(ID_VIEW_ZOOM_OUT, &CMySTEPViewerDoc::OnViewZoomOut)
	ON_COMMAND(ID_VIEW_MODEL_CHECKER, &CMySTEPViewerDoc::OnViewModelChecker)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MODEL_CHECKER, &CMySTEPViewerDoc::OnUpdateViewModelChecker)
	ON_COMMAND(ID_FILE_IMPORT, &CMySTEPViewerDoc::OnFileImport)
	ON_UPDATE_COMMAND_UI(ID_FILE_IMPORT, &CMySTEPViewerDoc::OnUpdateFileImport)
END_MESSAGE_MAP()


// CMySTEPViewerDoc construction/destruction

CMySTEPViewerDoc::CMySTEPViewerDoc()
{
}

CMySTEPViewerDoc::~CMySTEPViewerDoc()
{
	
}

BOOL CMySTEPViewerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

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

	setModel(CModelFactory::Load(this, lpszPathName));

	// Title
	CString strTitle = AfxGetAppName();
	strTitle += L" - ";
	strTitle += lpszPathName;

	AfxGetMainWnd()->SetWindowTextW(strTitle);

	// MRU
	AfxGetApp()->AddToRecentFileList(lpszPathName);

	return TRUE;
}

BOOL CMySTEPViewerDoc::OnSaveDocument(LPCTSTR /*lpszPathName*/)
{
	ASSERT(FALSE); // TODO

	return TRUE;
}

void CMySTEPViewerDoc::OnFileOpen()
{
	CFileDialog dlgFile(TRUE, nullptr, _T(""), 	OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, SUPPORTED_FILES);
	if (dlgFile.DoModal() != IDOK)
	{
		return;
	}

	OnOpenDocument(dlgFile.GetPathName());
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


void CMySTEPViewerDoc::OnFileImport()
{
	CFileDialog dlgFile(TRUE, nullptr, _T(""), OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, SUPPORTED_FILES);
	if (dlgFile.DoModal() != IDOK)
	{
		return;
	}

	addModel(CModelFactory::Load(this, dlgFile.GetPathName().GetString()));

	// Title
	CString strTitle = AfxGetAppName();
	strTitle += L" - ";
	strTitle += dlgFile.GetPathName().GetString();

	AfxGetMainWnd()->SetWindowTextW(strTitle);

	// MRU
	AfxGetApp()->AddToRecentFileList(dlgFile.GetPathName().GetString());
}

void CMySTEPViewerDoc::OnUpdateFileImport(CCmdUI* pCmdUI)
{
	BOOL bEnable = FALSE;
	if (!getModels().empty())
	{
		bEnable = _ptr<_ap_model>(getModels()[0])->getAP() == enumAP::IFC;
	}

	pCmdUI->Enable(bEnable);
}
