
// STEPViewerView.cpp : implementation of the CMySTEPViewerView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "STEPViewer.h"
#endif

#include "STEPViewerDoc.h"
#include "STEPViewerView.h"
#include "OpenGLIFCView.h"
#include "OpenGLSTEPView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ------------------------------------------------------------------------------------------------
CSTEPController* CMySTEPViewerView::GetController()
{
	auto pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	return pDoc;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CMySTEPViewerView::OnModelChanged()
{
	delete m_pOpenGLView;
	m_pOpenGLView = nullptr;

	auto pController = GetController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	if (pController->GetModel() == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pModel = pController->GetModel();
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	switch (pModel->GetType())
	{
		case enumSTEPModelType::STEP:
		{
			m_pOpenGLView = new COpenGLSTEPView(this);
			m_pOpenGLView->SetController(pController);
			m_pOpenGLView->Load();
		}
		break;

		case enumSTEPModelType::IFC:
		{
			m_pOpenGLView = new COpenGLIFCView(this);
			m_pOpenGLView->SetController(pController);
			m_pOpenGLView->Load();
		}
		break;


		default:
		{
			ASSERT(FALSE); // Unknown
		}
		break;
	}
}


// CMySTEPViewerView

IMPLEMENT_DYNCREATE(CMySTEPViewerView, CView)

BEGIN_MESSAGE_MAP(CMySTEPViewerView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CMySTEPViewerView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_DROPFILES()
	ON_WM_MOUSEWHEEL()
	ON_WM_KEYUP()
	ON_COMMAND(ID_PROJECTION_ISOMETRIC, &CMySTEPViewerView::OnProjectionIsometric)
	ON_UPDATE_COMMAND_UI(ID_PROJECTION_ISOMETRIC, &CMySTEPViewerView::OnUpdateProjectionIsometric)
	ON_COMMAND(ID_PROJECTION_PERSPECTIVE, &CMySTEPViewerView::OnProjectionPerspective)
	ON_UPDATE_COMMAND_UI(ID_PROJECTION_PERSPECTIVE, &CMySTEPViewerView::OnUpdateProjectionPerspective)
	ON_COMMAND(ID_VIEW_FRONT, &CMySTEPViewerView::OnViewFront)
	ON_COMMAND(ID_VIEW_TOP, &CMySTEPViewerView::OnViewTop)
	ON_COMMAND(ID_VIEW_RIGHT, &CMySTEPViewerView::OnViewRight)
	ON_COMMAND(ID_VIEW_BACK, &CMySTEPViewerView::OnViewBack)
	ON_COMMAND(ID_VIEW_LEFT, &CMySTEPViewerView::OnViewLeft)
	ON_COMMAND(ID_VIEW_BOTTOM, &CMySTEPViewerView::OnViewBottom)
	ON_COMMAND(ID_INSTANCES_SAVE, &CMySTEPViewerView::OnInstancesSave)
	ON_UPDATE_COMMAND_UI(ID_INSTANCES_SAVE, &CMySTEPViewerView::OnUpdateInstancesSave)
	ON_COMMAND(ID_INSTANCES_ZOOM_TO, &CMySTEPViewerView::OnInstancesZoomTo)
	ON_UPDATE_COMMAND_UI(ID_INSTANCES_ZOOM_TO, &CMySTEPViewerView::OnUpdateInstancesZoomTo)
	ON_COMMAND(ID_SHOW_FACES, &CMySTEPViewerView::OnShowFaces)
	ON_UPDATE_COMMAND_UI(ID_SHOW_FACES, &CMySTEPViewerView::OnUpdateShowFaces)
	ON_COMMAND(ID_SHOW_CONC_FACES_WIREFRAMES, &CMySTEPViewerView::OnShowConcFacesWireframes)
	ON_UPDATE_COMMAND_UI(ID_SHOW_CONC_FACES_WIREFRAMES, &CMySTEPViewerView::OnUpdateShowConcFacesWireframes)
	ON_COMMAND(ID_SHOW_LINES, &CMySTEPViewerView::OnShowLines)
	ON_UPDATE_COMMAND_UI(ID_SHOW_LINES, &CMySTEPViewerView::OnUpdateShowLines)
	ON_COMMAND(ID_SHOW_POINTS, &CMySTEPViewerView::OnShowPoints)
	ON_UPDATE_COMMAND_UI(ID_SHOW_POINTS, &CMySTEPViewerView::OnUpdateShowPoints)
END_MESSAGE_MAP()

// CMySTEPViewerView construction/destruction

CMySTEPViewerView::CMySTEPViewerView()
	: m_pOpenGLView(nullptr)
{
}

CMySTEPViewerView::~CMySTEPViewerView()
{
}

BOOL CMySTEPViewerView::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style &= ~CS_PARENTDC;
	cs.style |= CS_OWNDC;
	cs.style |= (WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

	return CView::PreCreateWindow(cs);
}

// CMySTEPViewerView drawing

void CMySTEPViewerView::OnDraw(CDC* pDC)
{
	if (m_pOpenGLView != nullptr)
	{
		m_pOpenGLView->Draw(pDC);
	}
}


// CMySTEPViewerView printing


void CMySTEPViewerView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CMySTEPViewerView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CMySTEPViewerView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CMySTEPViewerView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CMySTEPViewerView::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	CMenu menu;
	VERIFY(menu.LoadMenuW(IDR_MENU_3D_VIEW));

	auto pPopup = menu.GetSubMenu(0);

#ifndef SHARED_HANDLERS
	CMFCPopupMenu* settingsMenu = theApp.GetContextMenuManager()->ShowPopupMenu(pPopup->GetSafeHmenu(), point.x, point.y, this, TRUE);
	settingsMenu->AlwaysShowEmptyToolsEntry(1);
#endif
}


// CMySTEPViewerView diagnostics

#ifdef _DEBUG
void CMySTEPViewerView::AssertValid() const
{
	CView::AssertValid();
}

void CMySTEPViewerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMySTEPViewerDoc* CMySTEPViewerView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMySTEPViewerDoc)));
	return (CMySTEPViewerDoc*)m_pDocument;
}
#endif //_DEBUG


// CMySTEPViewerView message handlers


int CMySTEPViewerView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	DragAcceptFiles(TRUE);

	CMySTEPViewerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return -1;

	pDoc->RegisterView(this);

	return 0;
}


void CMySTEPViewerView::OnDestroy()
{
	CMySTEPViewerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	pDoc->UnRegisterView(this);

	delete m_pOpenGLView;
	m_pOpenGLView = nullptr;

	CView::OnDestroy();
}


BOOL CMySTEPViewerView::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}


void CMySTEPViewerView::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_pOpenGLView != nullptr)
	{
		m_pOpenGLView->OnMouseEvent(enumMouseEvent::LBtnDown, nFlags, point);
	}

	CView::OnLButtonDown(nFlags, point);
}


void CMySTEPViewerView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_pOpenGLView != nullptr)
	{
		m_pOpenGLView->OnMouseEvent(enumMouseEvent::LBtnUp, nFlags, point);
	}

	CView::OnLButtonUp(nFlags, point);
}


void CMySTEPViewerView::OnMButtonDown(UINT nFlags, CPoint point)
{
	if (m_pOpenGLView != nullptr)
	{
		m_pOpenGLView->OnMouseEvent(enumMouseEvent::MBtnDown, nFlags, point);
	}

	CView::OnMButtonDown(nFlags, point);
}


void CMySTEPViewerView::OnMButtonUp(UINT nFlags, CPoint point)
{
	if (m_pOpenGLView != nullptr)
	{
		m_pOpenGLView->OnMouseEvent(enumMouseEvent::MBtnUp, nFlags, point);
	}

	CView::OnMButtonUp(nFlags, point);
}


void CMySTEPViewerView::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (m_pOpenGLView != nullptr)
	{
		m_pOpenGLView->OnMouseEvent(enumMouseEvent::RBtnDown, nFlags, point);
	}

	CView::OnRButtonDown(nFlags, point);
}

void CMySTEPViewerView::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (m_pOpenGLView != nullptr)
	{
		m_pOpenGLView->OnMouseEvent(enumMouseEvent::RBtnUp, nFlags, point);
	}

	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CMySTEPViewerView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_pOpenGLView != nullptr)
	{
		m_pOpenGLView->OnMouseEvent(enumMouseEvent::Move, nFlags, point);
	}

	CView::OnMouseMove(nFlags, point);
}

// https://code.msdn.microsoft.com/windowsdesktop/Drag-and-Drop-in-MFC-f3008aad
void CMySTEPViewerView::OnDropFiles(HDROP hDropInfo)
{
	// Get the number of files dropped 
	int iFilesDropped = DragQueryFile(hDropInfo, 0xFFFFFFFF, nullptr, 0);
	if (iFilesDropped != 1)
	{
		return;
	}

	// Get the buffer size of the file.
	DWORD dwBuffer = DragQueryFile(hDropInfo, 0, nullptr, 0);

	// Get path and name of the file 
	CString strFile;
	DragQueryFile(hDropInfo, 0, strFile.GetBuffer(dwBuffer + 1), dwBuffer + 1);

	// Open
	CMySTEPViewerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	pDoc->OnOpenDocument(strFile);

	strFile.ReleaseBuffer();

	// Free the memory block containing the dropped-file information 
	DragFinish(hDropInfo);
}

BOOL CMySTEPViewerView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (m_pOpenGLView != nullptr)
	{
		m_pOpenGLView->OnMouseWheel(nFlags, zDelta, pt);
	}

	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void CMySTEPViewerView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (m_pOpenGLView != nullptr)
	{
		m_pOpenGLView->OnKeyUp(nChar, nRepCnt, nFlags);
	}
}

void CMySTEPViewerView::OnProjectionIsometric()
{
	if (m_pOpenGLView != nullptr)
	{
		m_pOpenGLView->SetProjection(enumProjection::Isometric);

		GetController()->OnApplicationPropertyChanged(nullptr, enumApplicationProperty::Projection);
	}
}

void CMySTEPViewerView::OnUpdateProjectionIsometric(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck((m_pOpenGLView != nullptr) && (m_pOpenGLView->GetProjection() == enumProjection::Isometric));
}

void CMySTEPViewerView::OnProjectionPerspective()
{
	if (m_pOpenGLView != nullptr)
	{
		m_pOpenGLView->SetProjection(enumProjection::Perspective);

		GetController()->OnApplicationPropertyChanged(nullptr, enumApplicationProperty::Projection);
	}
}

void CMySTEPViewerView::OnUpdateProjectionPerspective(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck((m_pOpenGLView != nullptr) && (m_pOpenGLView->GetProjection() == enumProjection::Perspective));
}

void CMySTEPViewerView::OnViewFront()
{
	if (m_pOpenGLView != nullptr)
	{
		m_pOpenGLView->SetView(enumView::Front);

		GetController()->OnApplicationPropertyChanged(nullptr, enumApplicationProperty::View);
	}
}

void CMySTEPViewerView::OnViewTop()
{
	if (m_pOpenGLView != nullptr)
	{
		m_pOpenGLView->SetView(enumView::Top);

		GetController()->OnApplicationPropertyChanged(nullptr, enumApplicationProperty::View);
	}
}

void CMySTEPViewerView::OnViewRight()
{
	if (m_pOpenGLView != nullptr)
	{
		m_pOpenGLView->SetView(enumView::Right);

		GetController()->OnApplicationPropertyChanged(nullptr, enumApplicationProperty::View);
	}
}

void CMySTEPViewerView::OnViewBack()
{
	if (m_pOpenGLView != nullptr)
	{
		m_pOpenGLView->SetView(enumView::Back);

		GetController()->OnApplicationPropertyChanged(nullptr, enumApplicationProperty::View);
	}
}

void CMySTEPViewerView::OnViewLeft()
{
	if (m_pOpenGLView != nullptr)
	{
		m_pOpenGLView->SetView(enumView::Left);

		GetController()->OnApplicationPropertyChanged(nullptr, enumApplicationProperty::View);
	}
}

void CMySTEPViewerView::OnViewBottom()
{
	if (m_pOpenGLView != nullptr)
	{
		m_pOpenGLView->SetView(enumView::Bottom);

		GetController()->OnApplicationPropertyChanged(nullptr, enumApplicationProperty::View);
	}
}

void CMySTEPViewerView::OnInstancesSave()
{
	auto pDocument = GetDocument();
	ASSERT_VALID(pDocument); 
	
	pDocument->SaveInstance();
}

void CMySTEPViewerView::OnUpdateInstancesSave(CCmdUI* pCmdUI)
{
	BOOL bEnable = FALSE;

	auto pDocument = GetDocument();
	ASSERT_VALID(pDocument);

	if ((pDocument != nullptr) && 
		(pDocument->GetModel() != nullptr) &&
		(pDocument->GetSelectedInstance() != nullptr) &&
		pDocument->GetSelectedInstance()->_hasGeometry() &&
		pDocument->GetSelectedInstance()->_isEnabled())
	{
		bEnable = TRUE;
	}

	pCmdUI->Enable(bEnable);
}

void CMySTEPViewerView::OnInstancesZoomTo()
{
	auto pDocument = GetDocument();
	ASSERT_VALID(pDocument);

	pDocument->ZoomToInstance();
}

void CMySTEPViewerView::OnUpdateInstancesZoomTo(CCmdUI* pCmdUI)
{
	BOOL bEnable = FALSE;

	auto pDocument = GetDocument();
	ASSERT_VALID(pDocument);

	if ((pDocument != nullptr) &&
		(pDocument->GetModel() != nullptr) &&
		(pDocument->GetSelectedInstance() != nullptr) &&
		pDocument->GetSelectedInstance()->_hasGeometry() &&
		pDocument->GetSelectedInstance()->_isEnabled())
	{
		bEnable = TRUE;
	}

	pCmdUI->Enable(bEnable);
}


void CMySTEPViewerView::OnShowFaces()
{
	if (m_pOpenGLView != nullptr)
	{
		m_pOpenGLView->ShowFaces(!m_pOpenGLView->AreFacesShown());

		GetController()->OnApplicationPropertyChanged(this, enumApplicationProperty::ShowFaces);
	}
}

void CMySTEPViewerView::OnUpdateShowFaces(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_pOpenGLView != nullptr);
	pCmdUI->SetCheck((m_pOpenGLView != nullptr) && m_pOpenGLView->AreFacesShown());
}

void CMySTEPViewerView::OnShowConcFacesWireframes()
{
	if (m_pOpenGLView != nullptr)
	{
		m_pOpenGLView->ShowConceptualFacesPolygons(!m_pOpenGLView->AreConceptualFacesPolygonsShown());

		GetController()->OnApplicationPropertyChanged(this, enumApplicationProperty::ShowConceptualFacesWireframes);
	}
}

void CMySTEPViewerView::OnUpdateShowConcFacesWireframes(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_pOpenGLView != nullptr);
	pCmdUI->SetCheck((m_pOpenGLView != nullptr) && m_pOpenGLView->AreConceptualFacesPolygonsShown());
}

void CMySTEPViewerView::OnShowLines()
{
	if (m_pOpenGLView != nullptr)
	{
		m_pOpenGLView->ShowLines(!m_pOpenGLView->AreLinesShown());

		GetController()->OnApplicationPropertyChanged(this, enumApplicationProperty::ShowLines);
	}
}

void CMySTEPViewerView::OnUpdateShowLines(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_pOpenGLView != nullptr);
	pCmdUI->SetCheck((m_pOpenGLView != nullptr) && m_pOpenGLView->AreLinesShown());
}

void CMySTEPViewerView::OnShowPoints()
{
	if (m_pOpenGLView != nullptr)
	{
		m_pOpenGLView->ShowPoints(!m_pOpenGLView->ArePointsShown());

		GetController()->OnApplicationPropertyChanged(this, enumApplicationProperty::ShowPoints);
	}
}

void CMySTEPViewerView::OnUpdateShowPoints(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_pOpenGLView != nullptr);
	pCmdUI->SetCheck((m_pOpenGLView != nullptr) && m_pOpenGLView->ArePointsShown());
}
