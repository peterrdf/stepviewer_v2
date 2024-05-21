
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
CController* CMySTEPViewerView::GetController()
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

	wchar_t szAppPath[_MAX_PATH];
	::GetModuleFileName(::GetModuleHandle(nullptr), szAppPath, sizeof(szAppPath));

	fs::path pthExe = szAppPath;
	auto pthRootFolder = pthExe.parent_path();

	wstring strSettingsFile = pthRootFolder.wstring();

	switch (pModel->GetType())
	{
		case enumModelType::STEP:
		{
			strSettingsFile += L"\\STEPViewer_STEP.settings";
			pController->getSettingsStorage()->loadSettings(strSettingsFile);

			m_pOpenGLView = new COpenGLSTEPView(this);
			m_pOpenGLView->SetController(pController);
			m_pOpenGLView->_load();
		}
		break;

		case enumModelType::IFC:
		{
			strSettingsFile += L"\\STEPViewer_IFC.settings";
			pController->getSettingsStorage()->loadSettings(strSettingsFile);

			m_pOpenGLView = new COpenGLIFCView(this);
			m_pOpenGLView->SetController(pController);
			m_pOpenGLView->_load();
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
	ON_COMMAND(ID_PROJECTION_ORTHOGRAPHIC, &CMySTEPViewerView::OnProjectionOrthographic)
	ON_UPDATE_COMMAND_UI(ID_PROJECTION_ORTHOGRAPHIC, &CMySTEPViewerView::OnUpdateProjectionOrthographic)
	ON_COMMAND(ID_PROJECTION_PERSPECTIVE, &CMySTEPViewerView::OnProjectionPerspective)
	ON_UPDATE_COMMAND_UI(ID_PROJECTION_PERSPECTIVE, &CMySTEPViewerView::OnUpdateProjectionPerspective)
	ON_COMMAND(ID_VIEW_FRONT, &CMySTEPViewerView::OnViewFront)
	ON_COMMAND(ID_VIEW_TOP, &CMySTEPViewerView::OnViewTop)
	ON_COMMAND(ID_VIEW_RIGHT, &CMySTEPViewerView::OnViewRight)
	ON_COMMAND(ID_VIEW_BACK, &CMySTEPViewerView::OnViewBack)
	ON_COMMAND(ID_VIEW_LEFT, &CMySTEPViewerView::OnViewLeft)
	ON_COMMAND(ID_VIEW_BOTTOM, &CMySTEPViewerView::OnViewBottom)
	ON_COMMAND(ID_VIEW_ISOMETRIC, &CMySTEPViewerView::OnViewIsometric)
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
{}

CMySTEPViewerView::~CMySTEPViewerView()
{}

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
		m_pOpenGLView->_draw(pDC);
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
	{
		return -1;
	}

	DragAcceptFiles(TRUE);

	auto pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (!pDoc)
	{
		return -1;
	}

	pDoc->RegisterView(this);

	return 0;
}

void CMySTEPViewerView::OnDestroy()
{
	auto pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (!pDoc)
	{
		return;
	}

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
	auto pDoc = GetDocument();
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
		m_pOpenGLView->_onMouseWheel(nFlags, zDelta, pt);
	}

	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void CMySTEPViewerView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (m_pOpenGLView != nullptr)
	{
		m_pOpenGLView->_onKeyUp(nChar, nRepCnt, nFlags);
	}
}

void CMySTEPViewerView::OnProjectionOrthographic()
{
	if (m_pOpenGLView != nullptr)
	{
		dynamic_cast<_oglRenderer*>(m_pOpenGLView)->_setProjection(enumProjection::Orthographic);

		GetController()->OnApplicationPropertyChanged(nullptr, enumApplicationProperty::Projection);
	}
}

void CMySTEPViewerView::OnUpdateProjectionOrthographic(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck((m_pOpenGLView != nullptr) && (dynamic_cast<_oglRenderer*>(m_pOpenGLView)->_getProjection() == enumProjection::Orthographic));
}

void CMySTEPViewerView::OnProjectionPerspective()
{
	if (m_pOpenGLView != nullptr)
	{
		dynamic_cast<_oglRenderer*>(m_pOpenGLView)->_setProjection(enumProjection::Perspective);

		GetController()->OnApplicationPropertyChanged(nullptr, enumApplicationProperty::Projection);
	}
}

void CMySTEPViewerView::OnUpdateProjectionPerspective(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck((m_pOpenGLView != nullptr) && (dynamic_cast<_oglRenderer*>(m_pOpenGLView)->_getProjection() == enumProjection::Perspective));
}

void CMySTEPViewerView::OnViewFront()
{
	if (m_pOpenGLView != nullptr)
	{
		dynamic_cast<_oglRenderer*>(m_pOpenGLView)->_setView(enumView::Front);

		GetController()->OnApplicationPropertyChanged(nullptr, enumApplicationProperty::View);
	}
}

void CMySTEPViewerView::OnViewTop()
{
	if (m_pOpenGLView != nullptr)
	{
		dynamic_cast<_oglRenderer*>(m_pOpenGLView)->_setView(enumView::Top);

		GetController()->OnApplicationPropertyChanged(nullptr, enumApplicationProperty::View);
	}
}

void CMySTEPViewerView::OnViewRight()
{
	if (m_pOpenGLView != nullptr)
	{
		dynamic_cast<_oglRenderer*>(m_pOpenGLView)->_setView(enumView::Right);

		GetController()->OnApplicationPropertyChanged(nullptr, enumApplicationProperty::View);
	}
}

void CMySTEPViewerView::OnViewBack()
{
	if (m_pOpenGLView != nullptr)
	{
		dynamic_cast<_oglRenderer*>(m_pOpenGLView)->_setView(enumView::Back);

		GetController()->OnApplicationPropertyChanged(nullptr, enumApplicationProperty::View);
	}
}

void CMySTEPViewerView::OnViewLeft()
{
	if (m_pOpenGLView != nullptr)
	{
		dynamic_cast<_oglRenderer*>(m_pOpenGLView)->_setView(enumView::Left);

		GetController()->OnApplicationPropertyChanged(nullptr, enumApplicationProperty::View);
	}
}

void CMySTEPViewerView::OnViewBottom()
{
	if (m_pOpenGLView != nullptr)
	{
		dynamic_cast<_oglRenderer*>(m_pOpenGLView)->_setView(enumView::Bottom);

		GetController()->OnApplicationPropertyChanged(nullptr, enumApplicationProperty::View);
	}
}

void CMySTEPViewerView::OnViewIsometric()
{
	if (m_pOpenGLView != nullptr)
	{
		dynamic_cast<_oglRenderer*>(m_pOpenGLView)->_setView(enumView::Isometric);

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
		pDocument->GetSelectedInstance()->HasGeometry() &&
		pDocument->GetSelectedInstance()->IsEnabled())
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
		pDocument->GetSelectedInstance()->HasGeometry() &&
		pDocument->GetSelectedInstance()->IsEnabled())
	{
		bEnable = TRUE;
	}

	pCmdUI->Enable(bEnable);
}

void CMySTEPViewerView::OnShowFaces()
{
	auto pRendererSettings = m_pOpenGLView != nullptr ? dynamic_cast<_oglRendererSettings*>(m_pOpenGLView) : nullptr;
	if (pRendererSettings != nullptr)
	{
		pRendererSettings->setShowFaces(!pRendererSettings->getShowFaces(nullptr));

		GetController()->OnApplicationPropertyChanged(this, enumApplicationProperty::ShowFaces);
	}
}

void CMySTEPViewerView::OnUpdateShowFaces(CCmdUI* pCmdUI)
{
	auto pRendererSettings = m_pOpenGLView != nullptr ? dynamic_cast<_oglRendererSettings*>(m_pOpenGLView) : nullptr;
	if (pRendererSettings != nullptr)
	{
		pCmdUI->SetCheck(pRendererSettings->getShowFaces(nullptr));
	}

	pCmdUI->Enable(pRendererSettings != nullptr);	
}

void CMySTEPViewerView::OnShowConcFacesWireframes()
{
	auto pRendererSettings = m_pOpenGLView != nullptr ? dynamic_cast<_oglRendererSettings*>(m_pOpenGLView) : nullptr;
	if (pRendererSettings != nullptr)
	{
		pRendererSettings->setShowConceptualFacesPolygons(!pRendererSettings->getShowConceptualFacesPolygons(nullptr));

		GetController()->OnApplicationPropertyChanged(this, enumApplicationProperty::ShowConceptualFacesWireframes);
	}
}

void CMySTEPViewerView::OnUpdateShowConcFacesWireframes(CCmdUI* pCmdUI)
{
	auto pRendererSettings = m_pOpenGLView != nullptr ? dynamic_cast<_oglRendererSettings*>(m_pOpenGLView) : nullptr;
	if (pRendererSettings != nullptr)
	{
		pCmdUI->SetCheck(pRendererSettings->getShowConceptualFacesPolygons(nullptr));
	}

	pCmdUI->Enable(pRendererSettings != nullptr);
}

void CMySTEPViewerView::OnShowLines()
{
	auto pRendererSettings = m_pOpenGLView != nullptr ? dynamic_cast<_oglRendererSettings*>(m_pOpenGLView) : nullptr;
	if (pRendererSettings != nullptr)
	{
		pRendererSettings->setShowLines(!pRendererSettings->getShowLines(nullptr));

		GetController()->OnApplicationPropertyChanged(this, enumApplicationProperty::ShowLines);
	}
}

void CMySTEPViewerView::OnUpdateShowLines(CCmdUI* pCmdUI)
{
	auto pRendererSettings = m_pOpenGLView != nullptr ? dynamic_cast<_oglRendererSettings*>(m_pOpenGLView) : nullptr;
	if (pRendererSettings != nullptr)
	{
		pCmdUI->SetCheck(pRendererSettings->getShowLines(nullptr));
	}

	pCmdUI->Enable(pRendererSettings != nullptr);
}

void CMySTEPViewerView::OnShowPoints()
{
	auto pRendererSettings = m_pOpenGLView != nullptr ? dynamic_cast<_oglRendererSettings*>(m_pOpenGLView) : nullptr;
	if (pRendererSettings != nullptr)
	{
		pRendererSettings->setShowPoints(!pRendererSettings->getShowPoints(nullptr));

		GetController()->OnApplicationPropertyChanged(this, enumApplicationProperty::ShowPoints);
	}
}

void CMySTEPViewerView::OnUpdateShowPoints(CCmdUI* pCmdUI)
{
	auto pRendererSettings = m_pOpenGLView != nullptr ? dynamic_cast<_oglRendererSettings*>(m_pOpenGLView) : nullptr;
	if (pRendererSettings != nullptr)
	{
		pCmdUI->SetCheck(pRendererSettings->getShowPoints(nullptr));
	}

	pCmdUI->Enable(pRendererSettings != nullptr);
}
