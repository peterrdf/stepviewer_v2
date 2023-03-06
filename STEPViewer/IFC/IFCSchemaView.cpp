
#include "stdafx.h"
#include "mainfrm.h"
#include "IFCSchemaView.h"
#include "Resource.h"
#include "STEPViewer.h"
#include "IFCModel.h"
#include "IFCSchemaViewConsts.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CIFCSchemaView::OnModelChanged() /*override*/
{
	ResetView();
}

// ------------------------------------------------------------------------------------------------
void CIFCSchemaView::LoadModel(CModel* pModel)
{
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	/**********************************************************************************************
	* Model
	*/		
	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = nullptr;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = (LPWSTR)pModel->GetModelName();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_MODEL;
	tvInsertStruct.item.lParam = NULL;

	HTREEITEM hModel = m_ifcTreeCtrl.InsertItem(&tvInsertStruct);
	m_ifcTreeCtrl.SetItemState(hModel, TVIS_BOLD, TVIS_BOLD);
	//*********************************************************************************************

	// Roots **************************************************************************************
	auto pEntityProvider = pModel->GetEntityProvider();
	if ((pEntityProvider == nullptr) || pEntityProvider->GetEntities().empty())
	{
		return;
	}

	auto& mapEntities = pEntityProvider->GetEntities();

	map<wstring, CEntity *> mapRoots;
	for (auto itEntity : mapEntities)
	{
		if (itEntity.second->hasParent())
		{
			continue;
		}

		mapRoots[itEntity.second->getName()] = itEntity.second;
	}	

	for (auto itRoot : mapRoots)
	{
		LoadEntity(itRoot.second, hModel);
	}
	// ********************************************************************************************	

	m_ifcTreeCtrl.Expand(hModel, TVE_EXPAND);
}

// ------------------------------------------------------------------------------------------------
void CIFCSchemaView::LoadAttributes(CEntity* pEntity, HTREEITEM hParent)
{
	if (pEntity->getAttributesCount() == 0)
	{
		return;
	}

	/*
	* Attributes
	*/
	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = hParent;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = ITEM_ATTRIBUTES;
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_ATTRIBUTES;
	tvInsertStruct.item.lParam = NULL;

	HTREEITEM hAttributes = m_ifcTreeCtrl.InsertItem(&tvInsertStruct);

	for (size_t iAttribute = 0; iAttribute < pEntity->getAttributes().size(); iAttribute++)
	{
		const wstring& strAttribute = pEntity->getAttributes()[iAttribute];
		if (pEntity->isAttributeInherited(strAttribute))
		{
			continue;
		}

		tvInsertStruct.hParent = hAttributes;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = (LPWSTR)pEntity->getAttributes()[iAttribute].c_str();
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = 
			pEntity->isAttributeIgnored(pEntity->getAttributes()[iAttribute]) ? IMAGE_IGNORED_ATTRIBUTE : IMAGE_ATTRIBUTE;
		tvInsertStruct.item.lParam = (LPARAM)pEntity;

		HTREEITEM hAttribute = m_ifcTreeCtrl.InsertItem(&tvInsertStruct);
		VERIFY(hAttribute != nullptr);
	} // for (size_t iAttribute = ...
}

// ------------------------------------------------------------------------------------------------
void CIFCSchemaView::LoadEntity(CEntity* pEntity, HTREEITEM hParent)
{
	/*
	* Entity
	*/
	pair<int, int>  prInstancesCount = GetInstancesCount(pEntity);

	CString strEntity;
	strEntity.Format(_T("%s (%d/%d)"), pEntity->getName(), prInstancesCount.first, prInstancesCount.second);

	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = hParent;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = (LPTSTR)strEntity.GetBuffer();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_ENTITY;
	tvInsertStruct.item.lParam = (LPARAM)pEntity;

	HTREEITEM hEntity = m_ifcTreeCtrl.InsertItem(&tvInsertStruct);

	LoadAttributes(pEntity, hEntity);

	/*
	* Sub-types
	*/
	if (pEntity->getSubTypes().size() > 0)
	{
		tvInsertStruct.hParent = hEntity;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = ITEM_SUB_TYPES;
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_SUB_TYPES;
		tvInsertStruct.item.lParam = NULL;

		HTREEITEM hSubType = m_ifcTreeCtrl.InsertItem(&tvInsertStruct);

		for (size_t iSubType = 0; iSubType < pEntity->getSubTypes().size(); iSubType++)
		{
			LoadEntity(pEntity->getSubTypes()[iSubType], hSubType);
		}
	} // if (pEntity->getSubTypes().size() > 0)
}

// ------------------------------------------------------------------------------------------------
pair<int, int> CIFCSchemaView::GetInstancesCount(CEntity* pEntity) const
{
	int iInstancesCount = (int)pEntity->getInstancesCount();

	int iSubInstancesCount = 0;
	for (size_t iSubType = 0; iSubType < pEntity->getSubTypes().size(); iSubType++)
	{
		iSubInstancesCount += (int)pEntity->getSubTypes()[iSubType]->getInstancesCount();

		iSubInstancesCount += GetInstancesCount(pEntity->getSubTypes()[iSubType]).second;
	}

	return pair<int, int>(iInstancesCount, iSubInstancesCount);
}

// ----------------------------------------------------------------------------
void CIFCSchemaView::OnNMClickTreeIFC(NMHDR* /*pNMHDR*/, LRESULT * pResult)
{
	*pResult = 0;
}

// ----------------------------------------------------------------------------
void CIFCSchemaView::OnNMRClickTreeIFC(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	*pResult = 1;

	DWORD dwPosition = GetMessagePos();
	CPoint point(LOWORD(dwPosition), HIWORD(dwPosition));
	m_ifcTreeCtrl.ScreenToClient(&point);

	UINT uFlags = 0;
	HTREEITEM hItem = m_ifcTreeCtrl.HitTest(point, &uFlags);

	if ((hItem == nullptr) || (m_ifcTreeCtrl.GetItemData(hItem) == NULL))
	{
		return;
	}

	m_ifcTreeCtrl.SelectItem(hItem);

	auto pEntity = (CEntity *)m_ifcTreeCtrl.GetItemData(hItem);

	CMenu menu;
	VERIFY(menu.CreatePopupMenu());

	int iImage = -1;
	int iSelectedImage = -1;
	m_ifcTreeCtrl.GetItemImage(hItem, iImage, iSelectedImage);

	ASSERT(iImage == iSelectedImage);

	// ********************************************************************************************
	// View IFC Relations/Attributes
	CString strViewIFCRelations;
	VERIFY(strViewIFCRelations.LoadStringW(IDS_VIEW_IFC_RELATIONS));

	menu.AppendMenu(MF_STRING, IDS_VIEW_IFC_RELATIONS, strViewIFCRelations);
	// ********************************************************************************************

	CPoint pointScreen(LOWORD(dwPosition), HIWORD(dwPosition));
	int iResult = menu.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN, pointScreen.x, pointScreen.y, &m_ifcTreeCtrl);
	switch (iResult)
	{
		case IDS_VIEW_IFC_RELATIONS:
		{			
			GetController()->OnViewRelations(this, pEntity);
		}
		break;
	} // switch (iResult)

	VERIFY(menu.DestroyMenu());
}

// ----------------------------------------------------------------------------
void CIFCSchemaView::OnTvnItemexpandingTreeIFC(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	*pResult = 0;
}

CIFCSchemaView::CIFCSchemaView()
	: m_pSearchDialog(nullptr)
{
}

CIFCSchemaView::~CIFCSchemaView()
{
}

BEGIN_MESSAGE_MAP(CIFCSchemaView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_PROPERTIES, OnProperties)
	ON_WM_CONTEXTMENU()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_NOTIFY(NM_CLICK, IDC_TREE_IFC, &CIFCSchemaView::OnNMClickTreeIFC)
	ON_NOTIFY(NM_RCLICK, IDC_TREE_IFC, &CIFCSchemaView::OnNMRClickTreeIFC)
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_TREE_IFC, &CIFCSchemaView::OnTvnItemexpandingTreeIFC)
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorkspaceBar message handlers

int CIFCSchemaView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	ASSERT(GetController() != nullptr);
	GetController()->RegisterView(this);

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create view:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;

	if (!m_ifcTreeCtrl.Create(dwViewStyle, rectDummy, this, IDC_TREE_IFC))
	{
		ASSERT(FALSE);

		return -1;      // fail to create
	}

	// Load view images:
	m_imageList.Create(IDB_CLASS_VIEW, 16, 0, RGB(255, 0, 0));
	m_ifcTreeCtrl.SetImageList(&m_imageList, TVSIL_NORMAL);

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

	//  Search
	m_pSearchDialog = new CSearchSchemaDialog(&m_ifcTreeCtrl);
	m_pSearchDialog->Create(IDD_DIALOG_SEARCH, this);

	return 0;
}

void CIFCSchemaView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CIFCSchemaView::ResetView()
{
	m_ifcTreeCtrl.DeleteAllItems();	

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

	LoadModel(pModel);
}

void CIFCSchemaView::AdjustLayout()
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

	m_ifcTreeCtrl.SetWindowPos(
		nullptr,
		rectClient.left + 1, 
		rectClient.top + cyTlb + 1, 
		rectClient.Width() - 2, 
		rectClient.Height() - cyTlb - 2, 
		SWP_NOACTIVATE | SWP_NOZORDER);
}

void CIFCSchemaView::OnProperties()
{
	if (!m_pSearchDialog->IsWindowVisible())
	{
		m_pSearchDialog->ShowWindow(SW_SHOW);
	}
	else
	{
		m_pSearchDialog->ShowWindow(SW_HIDE);
	}
}

void CIFCSchemaView::OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/)
{
}

void CIFCSchemaView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rectTree;
	m_ifcTreeCtrl.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CIFCSchemaView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_ifcTreeCtrl.SetFocus();
}

void CIFCSchemaView::OnChangeVisualStyle()
{
	m_imageList.DeleteImageList();

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

	m_imageList.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
	m_imageList.Add(&bmp, RGB(255, 0, 0));

	m_ifcTreeCtrl.SetImageList(&m_imageList, TVSIL_NORMAL);

	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_EXPLORER_24 : IDR_EXPLORER, 0, 0, TRUE /* Locked */);
}

void CIFCSchemaView::OnDestroy()
{
	__super::OnDestroy();

	delete m_pSearchDialog;
}

void CIFCSchemaView::OnShowWindow(BOOL bShow, UINT nStatus)
{
	__super::OnShowWindow(bShow, nStatus);

	if (!bShow)
	{
		m_pSearchDialog->ShowWindow(SW_HIDE);
	}
}
