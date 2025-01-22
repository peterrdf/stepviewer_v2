
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
TCHAR IFC_FILES[] = _T("IFC Files (*.ifc; *.ifczip)|*.ifc; *.ifczip|All Files (*.*)|*.*||");
TCHAR SUPPORTED_FILES[] = _T("STEP Files (*.stp; *.step; *.stpz; *.ifc; *.ifczip)|*.stp; *.step; *.stpz; *.ifc; *.ifczip|All Files (*.*)|*.*||");

// ************************************************************************************************
/*virtual*/ void CMySTEPViewerDoc::saveSelectedInstance() /*override*/
{
	ASSERT(getModel() != nullptr);

	if (getSelectedInstance() == nullptr)
	{
		return;
	} 

	CString strValidFileName = validateFileName(dynamic_cast<_ap_instance*>(getSelectedInstance())->getName().c_str()).c_str();

	TCHAR szFilters[] = _T("BIN Files (*.bin)|*.bin|All Files (*.*)|*.*||");
	CFileDialog dlgFile(FALSE, _T("bin"), strValidFileName,
		OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, szFilters);

	if (dlgFile.DoModal() != IDOK)
	{
		return;
	}

	auto pAPInstance = dynamic_cast<_ap_instance*>(getSelectedInstance());
	if (pAPInstance == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	OwlModel owlModel = getModel()->getOwlModel();
	ASSERT(owlModel != 0);

	OwlInstance owlInstance = getSelectedInstance()->getOwlInstance();
	if (owlInstance == 0)
	{
		owlInstance = _ap_geometry::buildOwlInstance(pAPInstance->getSdaiInstance());
		ASSERT(owlInstance != 0);
	}

	OwlInstance	owlMatrixInstance = CreateInstance(GetClassByName(owlModel, "Matrix"));
	ASSERT(owlMatrixInstance != 0);

	vector<double> vecMatrix
	{
		pAPInstance->getTransformationMatrix()->_11,
		pAPInstance->getTransformationMatrix()->_12,
		pAPInstance->getTransformationMatrix()->_13,
		pAPInstance->getTransformationMatrix()->_21,
		pAPInstance->getTransformationMatrix()->_22,
		pAPInstance->getTransformationMatrix()->_23,
		pAPInstance->getTransformationMatrix()->_31,
		pAPInstance->getTransformationMatrix()->_32,
		pAPInstance->getTransformationMatrix()->_33,
		pAPInstance->getTransformationMatrix()->_41,
		pAPInstance->getTransformationMatrix()->_42,
		pAPInstance->getTransformationMatrix()->_43,
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

void CMySTEPViewerDoc::OpenModels(vector<CString>& vecModels)
{
	setModel(nullptr);

	bool bFirstFile = true;
	for (auto model : vecModels)
	{
		if (bFirstFile)
		{
			auto pModel = _ap_model_factory::load(this, model, true, false);
			if (dynamic_cast<_ifc_model*>(pModel) == nullptr)
			{
				continue;
			}

			setModel(pModel);

			bFirstFile = false;
		}
		else
		{
			auto pModel = _ap_model_factory::load(this, model, true, false);
			if (dynamic_cast<_ifc_model*>(pModel) == nullptr)
			{
				continue;
			}

			addModel(pModel);
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

	setModel(_ap_model_factory::load(this, lpszPathName, false, false));

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
