
#include "stdafx.h"
#include "mainfrm.h"
#include "FileView.h"
#include "Resource.h"
#include "STEPViewer.h"
#include "STEPModel.h"
#include "Generic.h"
#include "IFCDecompContTreeView.h"

#include <algorithm>
#include <chrono>

using namespace std;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

class CFileViewMenuButton : public CMFCToolBarMenuButton
{

	DECLARE_SERIAL(CFileViewMenuButton)

public:
	CFileViewMenuButton(HMENU hMenu = nullptr) : CMFCToolBarMenuButton((UINT)-1, hMenu, -1)
	{
	}

	virtual void OnDraw(CDC* pDC, const CRect& rect, CMFCToolBarImages* pImages, BOOL bHorz = TRUE,
		BOOL bCustomizeMode = FALSE, BOOL bHighlight = FALSE, BOOL bDrawBorder = TRUE, BOOL bGrayDisabledButtons = TRUE)
	{
		pImages = CMFCToolBar::GetImages();

		// Patch: wrong background when the app starts
		pImages->SetTransparentColor(::GetSysColor(COLOR_BTNFACE));

		CAfxDrawState ds;
		pImages->PrepareDrawImage(ds);

		CMFCToolBarMenuButton::OnDraw(pDC, rect, pImages, bHorz, bCustomizeMode, bHighlight, bDrawBorder, bGrayDisabledButtons);

		pImages->EndDrawImage(ds);
	}
};

IMPLEMENT_SERIAL(CFileViewMenuButton, CMFCToolBarMenuButton, 1)

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CFileView::OnModelChanged()
{
	auto pController = GetController();
	if (pController == nullptr)
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

	delete m_pSTEPTreeView;
	m_pSTEPTreeView = nullptr;

	switch (pModel->GetType())
	{
		case enumModelType::STEP:
		{
			m_pSTEPTreeView = new CSTEPProductsTreeView(&m_modelStructureView);
			m_pSTEPTreeView->SetController(pController);
			m_pSTEPTreeView->Load();
		}
		break;

		case enumModelType::IFC:
		{
			m_pSTEPTreeView = new CIFCDecompContTreeView(&m_modelStructureView);
			m_pSTEPTreeView->SetController(pController);
			m_pSTEPTreeView->Load();
		}
		break;

		default:
		{
			ASSERT(FALSE); // Unknown
		}
		break;
	} // switch (pModel ->GetType())
}

/////////////////////////////////////////////////////////////////////////////
// CFileView

CFileView::CFileView()
	: m_pSTEPTreeView(nullptr)
{
}

CFileView::~CFileView()
{	
	delete m_pSTEPTreeView;
}

BEGIN_MESSAGE_MAP(CFileView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_PROPERTIES, OnProperties)
	ON_WM_CONTEXTMENU()	
	ON_NOTIFY(NM_CLICK, IDC_TREE_INSTANCE_VIEW, OnNMClickTree)
	ON_NOTIFY(NM_RCLICK, IDC_TREE_INSTANCE_VIEW, OnNMClickTree)
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_TREE_INSTANCE_VIEW, OnItemExpanding)
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_WM_DESTROY()
	ON_COMMAND(ID_NEW_INSTANCE, OnNewInstance)
	ON_WM_SHOWWINDOW()	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorkspaceBar message handlers

int CFileView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	auto pController = GetController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return -1;
	}

	pController->RegisterView(this);

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create view:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS;

	if (!m_modelStructureView.Create(dwViewStyle, rectDummy, this, IDC_TREE_INSTANCE_VIEW))
	{
		ASSERT(FALSE);

		return -1;      // fail to create
	}	

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_EXPLORER);
	m_wndToolBar.LoadToolBar(IDR_EXPLORER, 0, 0, TRUE /* Is locked */);

	OnChangeVisualStyle();

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() 
		| CBRS_TOOLTIPS | CBRS_FLYBY);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & 
		~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	m_wndToolBar.SetOwner(this);

	// All commands will be routed via this control , not via the parent frame:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);	

	AdjustLayout();

	return 0;
}

void CFileView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CFileView::OnProperties()
{
	if (m_pSTEPTreeView != nullptr)
	{
		m_pSTEPTreeView->OnSearch();
	}	
}

void CFileView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if (m_pSTEPTreeView != nullptr)
	{
		m_pSTEPTreeView->OnContextMenu(pWnd, point);
	}
}

void CFileView::AdjustLayout()
{
	if (GetSafeHwnd() == nullptr)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(
		nullptr,
		rectClient.left,
		rectClient.top,
		rectClient.Width(),
		cyTlb,
		SWP_NOACTIVATE | SWP_NOZORDER);

	m_modelStructureView.SetWindowPos(
		nullptr, rectClient.left + 1,
		rectClient.top + cyTlb + 1,
		rectClient.Width() - 2,
		rectClient.Height() - cyTlb - 2,
		SWP_NOACTIVATE | SWP_NOZORDER);
}

void CFileView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rectTree;
	m_modelStructureView.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CFileView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_modelStructureView.SetFocus();
}

void CFileView::OnChangeVisualStyle()
{
	UINT uiBmpId = theApp.m_bHiColorIcons ? IDB_CLASS_VIEW_24 : IDB_CLASS_VIEW;

	CBitmap bmp;
	if (!bmp.LoadBitmap(uiBmpId))
	{
		TRACE(_T("Can't load bitmap: %x\n"), uiBmpId);
		ASSERT(FALSE);
		return;
	}

	BITMAP bmpObj;
	bmp.GetBitmap(&bmpObj);

	UINT nFlags = ILC_MASK;
	nFlags |= (theApp.m_bHiColorIcons) ? ILC_COLOR24 : ILC_COLOR4;

	if (m_pSTEPTreeView != nullptr)
	{
		m_modelStructureView.SetImageList(m_pSTEPTreeView->GetImageList(), TVSIL_NORMAL);
	}

	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_EXPLORER_24 : IDR_EXPLORER, 0, 0, TRUE /* Locked */);
}

void CFileView::OnDestroy()
{
	auto pController = GetController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	pController->UnRegisterView(this);

	__super::OnDestroy();
}

void CFileView::OnNMClickTree(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;

	if (m_pSTEPTreeView != nullptr)
	{
		m_pSTEPTreeView->OnTreeItemClick(pNMHDR, pResult);
	}
}

void CFileView::OnItemExpanding(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;	

	if (m_pSTEPTreeView != nullptr)
	{
		m_pSTEPTreeView->OnTreeItemExpanding(pNMHDR, pResult);
	}
}

void CFileView::OnNewInstance()
{
	ASSERT(0); // todo
}

void CFileView::OnShowWindow(BOOL bShow, UINT nStatus)
{
	__super::OnShowWindow(bShow, nStatus);

	if (m_pSTEPTreeView != nullptr)
	{
		m_pSTEPTreeView->OnShowWindow(bShow, nStatus);
	}
}



