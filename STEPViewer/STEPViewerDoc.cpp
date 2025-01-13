
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
#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ************************************************************************************************
TCHAR IFC_FILES[] = _T("IFC Files (*.ifc; *.ifczip)|*.ifc; *.ifczip|All Files (*.*)|*.*||");
TCHAR SUPPORTED_FILES[] = _T("STEP Files (*.stp; *.step; *.ifc; *.ifczip)|*.stp;*.step; *.ifc; *.ifczip|All Files (*.*)|*.*||");


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

	auto pProductInstance = dynamic_cast<_ap_instance*>(getSelectedInstance());
	if (pProductInstance != nullptr)
	{
		OwlModel owlModel = getModel()->getOwlModel();
		ASSERT(owlModel != 0);

		OwlInstance owlInstance = getSelectedInstance()->getOwlInstance();
		if (owlInstance == 0)
		{
			owlInstance = _ap_geometry::buildOwlInstance(pProductInstance->getSdaiInstance());
			ASSERT(owlInstance != 0);
		}

		OwlInstance	owlMatrixInstance = CreateInstance(GetClassByName(owlModel, "Matrix"));
		ASSERT(owlMatrixInstance != 0);

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
			owlMatrixInstance,
			GetPropertyByName(owlModel, "coordinates"),
			vecMatrix.data(),
			vecMatrix.size());

		OwlInstance owlTransformationInstance = CreateInstance(GetClassByName(owlModel, "Transformation"));
		ASSERT(owlTransformationInstance != 0);

		SetObjectProperty(
			owlTransformationInstance,
			GetPropertyByName(owlModel, "object"),
			owlInstance);

		SetObjectProperty(
			owlTransformationInstance,
			GetPropertyByName(owlModel, "matrix"),
			owlMatrixInstance);

		SaveInstanceTreeW(owlTransformationInstance, dlgFile.GetPathName());
	}
	else
	{
		ASSERT(FALSE);
	}
}

void CMySTEPViewerDoc::OpenModels(vector<CString>& vecModels)
{
	bool bFirstFile = true;
	for (auto model : vecModels)
	{
		if (bFirstFile)
		{
			setModel(nullptr);
			setModel(CModelFactory::Load(this, model, true));

			bFirstFile = false;
		}
		else
		{
			addModel(CModelFactory::Load(this, model, true));
		}
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
	ON_COMMAND(ID_FILE_OPEN_MULTIPLE_IFC, &CMySTEPViewerDoc::OnFileOpenMultipleIFC)
	ON_UPDATE_COMMAND_UI(ID_FILE_OPEN_MULTIPLE_IFC, &CMySTEPViewerDoc::OnUpdateFileOpenMultipleIFC)
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

	setModel(CModelFactory::Load(this, lpszPathName, false));

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

void CMySTEPViewerDoc::OnFileOpenMultipleIFC()
{
	CFileDialog dlgFile(TRUE, nullptr, _T(""), OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT, IFC_FILES);
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
	}	

	OpenModels(vecModels);

	// Title
	CString strTitle = AfxGetAppName();
	strTitle += L" - ";
	strTitle += vecModels[0];
	strTitle += L", ...";
	AfxGetMainWnd()->SetWindowTextW(strTitle);

	// MRU
	AfxGetApp()->AddToRecentFileList(vecModels[0]);
}

void CMySTEPViewerDoc::OnUpdateFileOpenMultipleIFC(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}
