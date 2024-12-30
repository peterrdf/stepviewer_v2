
#include "stdafx.h"

#include "_ptr.h"

#include "mainfrm.h"
#include "StructureView.h"
#include "Resource.h"
#include "STEPViewer.h"
#include "AP242PModelStructureView.h"
#include "IFCModelStructureView.h"
#include "CIS2ModelStructureView.h"

#include <algorithm>
#include <chrono>
using namespace std;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CStructureView::onModelLoaded()
{
	auto pController = getController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	enumAP enAP = enumAP::STEP;
	if (!pController->getModels().empty())
	{
		_ptr<_ap_model> apModel(pController->getModels().back());
		ASSERT(apModel);

		enAP = apModel->getAP();
	}

	delete m_pSTEPTreeView;
	m_pSTEPTreeView = nullptr;

	switch (enAP)
	{
		case enumAP::STEP:
		{
			m_pSTEPTreeView = new CAP242PModelStructureView(&m_treeCtrl);
			m_pSTEPTreeView->setController(pController);
			m_pSTEPTreeView->Load();
		}
		break;

		case enumAP::IFC:
		{
			m_pSTEPTreeView = new CIFCModelStructureView(&m_treeCtrl);
			m_pSTEPTreeView->setController(pController);
			m_pSTEPTreeView->Load();
		}
		break;

		case enumAP::CIS2:
		{
			m_pSTEPTreeView = new CCIS2ModelStructureView(&m_treeCtrl);
			m_pSTEPTreeView->setController(pController);
			m_pSTEPTreeView->Load();
		}
		break;

		default:
		{
			ASSERT(FALSE); // Unknown
		}
		break;
	} // switch (enAP)
}

/////////////////////////////////////////////////////////////////////////////
// CStructureView

CStructureView::CStructureView()
	: m_pSTEPTreeView(nullptr)
{}

CStructureView::~CStructureView()
{	
	delete m_pSTEPTreeView;
}

BEGIN_MESSAGE_MAP(CStructureView, CDockablePane)
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
	ON_WM_SHOWWINDOW()	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorkspaceBar message handlers

int CStructureView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	auto pController = getController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return -1;
	}

	pController->registerView(this);

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create view:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS;

	if (!m_treeCtrl.Create(dwViewStyle, rectDummy, this, IDC_TREE_INSTANCE_VIEW))
	{
		ASSERT(FALSE);

		return -1;
	}	

	m_toolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_EXPLORER);
	m_toolBar.LoadToolBar(IDR_EXPLORER, 0, 0, TRUE /* Is locked */);

	OnChangeVisualStyle();

	m_toolBar.SetPaneStyle(m_toolBar.GetPaneStyle() 
		| CBRS_TOOLTIPS | CBRS_FLYBY);

	m_toolBar.SetPaneStyle(m_toolBar.GetPaneStyle() & 
		~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	m_toolBar.SetOwner(this);

	// All commands will be routed via this control , not via the parent frame:
	m_toolBar.SetRouteCommandsViaFrame(FALSE);	

	AdjustLayout();

	return 0;
}

void CStructureView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CStructureView::OnProperties()
{
	if (m_pSTEPTreeView != nullptr)
	{
		m_pSTEPTreeView->OnSearch();
	}	
}

void CStructureView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if (m_pSTEPTreeView != nullptr)
	{
		m_pSTEPTreeView->OnContextMenu(pWnd, point);
	}
}

void CStructureView::AdjustLayout()
{
	if (GetSafeHwnd() == nullptr)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_toolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_toolBar.SetWindowPos(
		nullptr,
		rectClient.left,
		rectClient.top,
		rectClient.Width(),
		cyTlb,
		SWP_NOACTIVATE | SWP_NOZORDER);

	m_treeCtrl.SetWindowPos(
		nullptr, rectClient.left + 1,
		rectClient.top + cyTlb + 1,
		rectClient.Width() - 2,
		rectClient.Height() - cyTlb - 2,
		SWP_NOACTIVATE | SWP_NOZORDER);
}

void CStructureView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rectTree;
	m_treeCtrl.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CStructureView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_treeCtrl.SetFocus();
}

void CStructureView::OnChangeVisualStyle()
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
		m_treeCtrl.SetImageList(m_pSTEPTreeView->GetImageList(), TVSIL_NORMAL);
	}

	m_toolBar.CleanUpLockedImages();
	m_toolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_EXPLORER_24 : IDR_EXPLORER, 0, 0, TRUE /* Locked */);
}

void CStructureView::OnDestroy()
{
	ASSERT(getController() != nullptr);
	getController()->unRegisterView(this);

	__super::OnDestroy();
}

void CStructureView::OnNMClickTree(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;

	if (m_pSTEPTreeView != nullptr)
	{
		m_pSTEPTreeView->OnTreeItemClick(pNMHDR, pResult);
	}
}

void CStructureView::OnItemExpanding(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;	

	if (m_pSTEPTreeView != nullptr)
	{
		m_pSTEPTreeView->OnTreeItemExpanding(pNMHDR, pResult);
	}
}

void CStructureView::OnShowWindow(BOOL bShow, UINT nStatus)
{
	__super::OnShowWindow(bShow, nStatus);

	if (m_pSTEPTreeView != nullptr)
	{
		m_pSTEPTreeView->OnShowWindow(bShow, nStatus);
	}
}



