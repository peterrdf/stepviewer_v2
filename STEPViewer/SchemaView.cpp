
#include "stdafx.h"
#include "SchemaView.h"

#include "_ptr.h"

#include "mainfrm.h"
#include "Resource.h"
#include "STEPViewer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// ************************************************************************************************
#define IMAGE_ENTITY_SCHEMA_VIEW				1
#define IMAGE_MODEL_SCHEMA_VIEW					2
#define IMAGE_SUB_TYPES_SCHEMA_VIEW				2
#define IMAGE_ATTRIBUTES_SCHEMA_VIEW			2
#define IMAGE_ATTRIBUTE_SCHEMA_VIEW				5

#define ITEM_SUB_TYPES_SCHEMA_VIEW				L"Sub-types"
#define ITEM_ATTRIBUTES_SCHEMA_VIEW				L"Attributes"

// ************************************************************************************************
/*virtual*/ void CSchemaView::onModelLoaded() /*override*/
{
	ResetView();
}

/*virtual*/ void CSchemaView::onInstanceSelected(_view* /*pSender*/) /*override*/
{
	ResetView();
}

/*virtual*/ CTreeCtrlEx* CSchemaView::GetTreeView() /*override*/
{
	return &m_treeCtrl;
}

/*virtual*/ vector<CString> CSchemaView::GetSearchFilters() /*override*/
{
	return vector<CString>
		{
			_T("(All)"),
			_T("Entities"),
			_T("Attributes"),
		};
}

/*virtual*/ void CSchemaView::LoadChildrenIfNeeded(HTREEITEM /*hItem*/) /*override*/
{
}

/*virtual*/ BOOL CSchemaView::ContainsText(int iFilter, HTREEITEM hItem, const CString& strText) /*override*/
{
	if (hItem == NULL)
	{
		ASSERT(FALSE);

		return FALSE;
	}

	CString strItemText = GetTreeView()->GetItemText(hItem);
	strItemText.MakeLower();

	CString strTextLower = strText;
	strTextLower.MakeLower();

	// Entities
	if (iFilter == (int)enumSearchFilter::Entities)
	{
		int iImage, iSelectedImage = -1;
		GetTreeView()->GetItemImage(hItem, iImage, iSelectedImage);

		ASSERT(iImage == iSelectedImage);

		if (iImage == IMAGE_ENTITY_SCHEMA_VIEW)
		{
			return strItemText.Find(strTextLower, 0) != -1;
		}
		else
		{
			return FALSE;
		}
	}

	// Attributes
	if (iFilter == (int)enumSearchFilter::Attributes)
	{
		int iImage, iSelectedImage = -1;
		GetTreeView()->GetItemImage(hItem, iImage, iSelectedImage);

		ASSERT(iImage == iSelectedImage);

		if (iImage == IMAGE_ATTRIBUTE_SCHEMA_VIEW)
		{
			return strItemText.Find(strTextLower, 0) != -1;
		}
		else
		{
			return FALSE;
		}
	}

	// All
	return strItemText.Find(strTextLower, 0) != -1;
}

// ************************************************************************************************
HTREEITEM CSchemaView::LoadModel(_ap_model* pModel)
{
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return NULL;
	}

	// Model	
	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = nullptr;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
	tvInsertStruct.item.pszText = (LPWSTR)pModel->getPath();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_MODEL_SCHEMA_VIEW;
	tvInsertStruct.item.lParam = NULL;
	tvInsertStruct.item.cChildren = INT_MAX;

	return m_treeCtrl.InsertItem(&tvInsertStruct);
}

void CSchemaView::LoadAttributes(_entity* pEntity, HTREEITEM hParent)
{
	if (pEntity->getAttributes().size() == 0)
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
	tvInsertStruct.item.pszText = ITEM_ATTRIBUTES_SCHEMA_VIEW;
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_ATTRIBUTES_SCHEMA_VIEW;
	tvInsertStruct.item.lParam = NULL;

	HTREEITEM hAttributes = m_treeCtrl.InsertItem(&tvInsertStruct);

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
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_ATTRIBUTE_SCHEMA_VIEW;
		tvInsertStruct.item.lParam = (LPARAM)pEntity;

		HTREEITEM hAttribute = m_treeCtrl.InsertItem(&tvInsertStruct);
		VERIFY(hAttribute != nullptr);
	} // for (size_t iAttribute = ...
}

void CSchemaView::LoadEntity(_entity* pEntity, HTREEITEM hParent)
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
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_ENTITY_SCHEMA_VIEW;
	tvInsertStruct.item.lParam = (LPARAM)pEntity;

	HTREEITEM hEntity = m_treeCtrl.InsertItem(&tvInsertStruct);

	LoadAttributes(pEntity, hEntity);

	/*
	* Sub-types
	*/
	if (pEntity->getSubTypes().size() > 0)
	{
		tvInsertStruct.hParent = hEntity;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = ITEM_SUB_TYPES_SCHEMA_VIEW;
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_SUB_TYPES_SCHEMA_VIEW;
		tvInsertStruct.item.lParam = NULL;

		HTREEITEM hSubType = m_treeCtrl.InsertItem(&tvInsertStruct);

		for (size_t iSubType = 0; iSubType < pEntity->getSubTypes().size(); iSubType++)
		{
			LoadEntity(pEntity->getSubTypes()[iSubType], hSubType);
		}
	} // if (pEntity->getSubTypes().size() > 0)
}

pair<int, int> CSchemaView::GetInstancesCount(_entity* pEntity) const
{
	int iInstancesCount = (int)pEntity->getInstances().size();

	int iSubInstancesCount = 0;
	for (size_t iSubType = 0; iSubType < pEntity->getSubTypes().size(); iSubType++)
	{
		iSubInstancesCount += (int)pEntity->getSubTypes()[iSubType]->getInstances().size();

		iSubInstancesCount += GetInstancesCount(pEntity->getSubTypes()[iSubType]).second;
	}

	return pair<int, int>(iInstancesCount, iSubInstancesCount);
}

void CSchemaView::OnNMClickTree(NMHDR* /*pNMHDR*/, LRESULT * pResult)
{
	*pResult = 0;
}

void CSchemaView::OnNMRClickTree(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	*pResult = 1;

	DWORD dwPosition = GetMessagePos();
	CPoint point(LOWORD(dwPosition), HIWORD(dwPosition));
	m_treeCtrl.ScreenToClient(&point);

	UINT uFlags = 0;
	HTREEITEM hItem = m_treeCtrl.HitTest(point, &uFlags);

	if ((hItem == nullptr) || (m_treeCtrl.GetItemData(hItem) == NULL))
	{
		return;
	}

	m_treeCtrl.SelectItem(hItem);

	auto pEntity = (_entity *)m_treeCtrl.GetItemData(hItem);

	CMenu menu;
	VERIFY(menu.CreatePopupMenu());

	int iImage, iSelectedImage = -1;
	m_treeCtrl.GetItemImage(hItem, iImage, iSelectedImage);

	ASSERT(iImage == iSelectedImage);

	// ********************************************************************************************
	// View Relations/Attributes
	CString strViewRelations;
	VERIFY(strViewRelations.LoadStringW(IDS_VIEW_IFC_RELATIONS));

	menu.AppendMenu(MF_STRING, IDS_VIEW_IFC_RELATIONS, strViewRelations);
	// ********************************************************************************************

	CPoint pointScreen(LOWORD(dwPosition), HIWORD(dwPosition));
	int iResult = menu.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN, pointScreen.x, pointScreen.y, &m_treeCtrl);
	switch (iResult)
	{
		case IDS_VIEW_IFC_RELATIONS:
		{			
			getAPController()->onViewRelations(this, pEntity);
		}
		break;
	} // switch (iResult)

	VERIFY(menu.DestroyMenu());
}

void CSchemaView::OnTVNItemexpandingTree(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	*pResult = 0;

	int iImage, iSelectedImage = -1;
	m_treeCtrl.GetItemImage(pNMTreeView->itemNew.hItem, iImage, iSelectedImage);

	ASSERT(iImage == iSelectedImage);

	if ((iImage == IMAGE_MODEL_SCHEMA_VIEW) &&
		(m_treeCtrl.GetNextItem(pNMTreeView->itemNew.hItem, TVGN_CHILD) == NULL))
	{
		HTREEITEM hModel = pNMTreeView->itemNew.hItem;

		auto itModel = m_mapModels.find(hModel);
		ASSERT(itModel != m_mapModels.end());

		auto pEntityProvider = itModel->second->getEntityProvider();
		if ((pEntityProvider == nullptr) || pEntityProvider->getEntities().empty())
		{
			return;
		}

		auto& mapEntities = pEntityProvider->getEntities();

		map<wstring, _entity*> mapRoots;
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
	} // if ((iImage == IMAGE_MODEL_SCHEMA_VIEW) && ...
}

CSchemaView::CSchemaView()
	: m_mapModels()
	, m_pSearchDialog(nullptr)
{
}

CSchemaView::~CSchemaView()
{
}

BEGIN_MESSAGE_MAP(CSchemaView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_PROPERTIES, OnProperties)
	ON_WM_CONTEXTMENU()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_NOTIFY(NM_CLICK, IDC_TREE_IFC, &CSchemaView::OnNMClickTree)
	ON_NOTIFY(NM_RCLICK, IDC_TREE_IFC, &CSchemaView::OnNMRClickTree)
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_TREE_IFC, &CSchemaView::OnTVNItemexpandingTree)
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorkspaceBar message handlers

int CSchemaView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	ASSERT(getController() != nullptr);
	getController()->registerView(this);

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create view:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;

	if (!m_treeCtrl.Create(dwViewStyle, rectDummy, this, IDC_TREE_IFC))
	{
		ASSERT(FALSE);

		return -1;
	}

	// Load view images:
	m_imageList.Create(IDB_CLASS_VIEW, 16, 0, RGB(255, 0, 0));
	m_treeCtrl.SetImageList(&m_imageList, TVSIL_NORMAL);

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

	//  Search
	m_pSearchDialog = new CSearchTreeCtrlDialog(this);
	m_pSearchDialog->Create(IDD_DIALOG_SEARCH, this);

	return 0;
}

void CSchemaView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CSchemaView::ResetView()
{
	m_mapModels.clear();

	m_treeCtrl.DeleteAllItems();	

	auto pController = getController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	if (pController->getModels().empty())
	{
		return;
	}

	HTREEITEM hFirstModel = NULL;
	for (auto pModel : pController->getModels())
	{
		_ptr<_ap_model> apModel(pModel);

		HTREEITEM hModel = LoadModel(apModel);

		m_mapModels[hModel] = apModel;

		if (hFirstModel == NULL)
		{
			hFirstModel = hModel;
		}
	}

	if (hFirstModel != NULL)
	{
		m_treeCtrl.Expand(hFirstModel, TVE_EXPAND);
	}
}

void CSchemaView::AdjustLayout()
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
		nullptr,
		rectClient.left + 1, 
		rectClient.top + cyTlb + 1, 
		rectClient.Width() - 2, 
		rectClient.Height() - cyTlb - 2, 
		SWP_NOACTIVATE | SWP_NOZORDER);
}

void CSchemaView::OnProperties()
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

void CSchemaView::OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/)
{
}

void CSchemaView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rectTree;
	m_treeCtrl.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CSchemaView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_treeCtrl.SetFocus();
}

void CSchemaView::OnChangeVisualStyle()
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

	m_treeCtrl.SetImageList(&m_imageList, TVSIL_NORMAL);

	m_toolBar.CleanUpLockedImages();
	m_toolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_EXPLORER_24 : IDR_EXPLORER, 0, 0, TRUE /* Locked */);
}

void CSchemaView::OnDestroy()
{
	ASSERT(getController() != nullptr);
	getController()->unRegisterView(this);

	__super::OnDestroy();

	delete m_pSearchDialog;
}

void CSchemaView::OnShowWindow(BOOL bShow, UINT nStatus)
{
	__super::OnShowWindow(bShow, nStatus);

	if (!bShow)
	{
		m_pSearchDialog->ShowWindow(SW_HIDE);
	}
}
