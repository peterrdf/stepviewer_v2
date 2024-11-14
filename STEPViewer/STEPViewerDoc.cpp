
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

/*virtual*/ void CMySTEPViewerDoc::SaveInstance() /*override*/
{
	ASSERT(m_pModel != nullptr);

	if (GetSelectedInstance() == nullptr)
	{
		return;
	} 

	CString strValidPath = GetSelectedInstance()->GetName().c_str();
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

	auto pProductInstance = dynamic_cast<CAP242*>(GetSelectedInstance());
	if (pProductInstance != nullptr)
	{
		SdaiModel iSdaiModel = sdaiGetInstanceModel(GetSelectedInstance()->GetInstance());
		ASSERT(iSdaiModel != 0);

		OwlModel iOwlModel = 0;
		owlGetModel(iSdaiModel, &iOwlModel);
		ASSERT(iOwlModel != 0);

		OwlInstance	iMatrixInstance = CreateInstance(GetClassByName(iOwlModel, "Matrix"));
		ASSERT(iMatrixInstance != 0);

		vector<double> vecMatrix
		{
			pProductInstance->GetTransformationMatrix()->_11,
			pProductInstance->GetTransformationMatrix()->_12,
			pProductInstance->GetTransformationMatrix()->_13,
			pProductInstance->GetTransformationMatrix()->_21,
			pProductInstance->GetTransformationMatrix()->_22,
			pProductInstance->GetTransformationMatrix()->_23,
			pProductInstance->GetTransformationMatrix()->_31,
			pProductInstance->GetTransformationMatrix()->_32,
			pProductInstance->GetTransformationMatrix()->_33,
			pProductInstance->GetTransformationMatrix()->_41,
			pProductInstance->GetTransformationMatrix()->_42,
			pProductInstance->GetTransformationMatrix()->_43,
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
			GetSelectedInstance()->GetInstance());

		SetObjectProperty(
			iTransformationInstance,
			GetPropertyByName(iOwlModel, "matrix"),
			iMatrixInstance);

		SaveInstanceTreeW(iTransformationInstance, dlgFile.GetPathName());
	}
	else
	{
		SaveInstanceTreeW(GetSelectedInstance()->GetInstance(), dlgFile.GetPathName());
	}
}

// CMySTEPViewerDoc

IMPLEMENT_DYNCREATE(CMySTEPViewerDoc, CDocument)

BEGIN_MESSAGE_MAP(CMySTEPViewerDoc, CDocument)
	ON_COMMAND(ID_FILE_OPEN, &CMySTEPViewerDoc::OnFileOpen)
	ON_COMMAND(ID_VIEW_ZOOM_OUT, &CMySTEPViewerDoc::OnViewZoomOut)
	ON_COMMAND(ID_VIEW_MODEL_CHECKER, &CMySTEPViewerDoc::OnViewModelChecker)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MODEL_CHECKER, &CMySTEPViewerDoc::OnUpdateViewModelChecker)
END_MESSAGE_MAP()


// CMySTEPViewerDoc construction/destruction

CMySTEPViewerDoc::CMySTEPViewerDoc()
{
}

CMySTEPViewerDoc::~CMySTEPViewerDoc()
{
	delete m_pModel;
}

BOOL CMySTEPViewerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	if (m_pModel != nullptr)
	{
		delete m_pModel;
		m_pModel = nullptr;
	}

	SetModel(new CSTEPModel());

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

	if (m_pModel != nullptr)
	{
		delete m_pModel;
		m_pModel = nullptr;
	}

	SetModel(CModelFactory::Load(lpszPathName));

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
	TCHAR szFilters[] = _T("STEP Files (*.stp; *.step; *.ifc)|*.stp;*.step; *.ifc|All Files (*.*)|*.*||");
	CFileDialog dlgFile(TRUE, nullptr, _T(""), 	OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, szFilters);

	if (dlgFile.DoModal() != IDOK)
	{
		return;
	}

	OnOpenDocument(dlgFile.GetPathName());
}

void CMySTEPViewerDoc::OnViewZoomOut()
{
	ZoomOut();
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
