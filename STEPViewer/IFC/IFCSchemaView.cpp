
#include "stdafx.h"
#include "mainfrm.h"
#include "IFCSchemaView.h"
#include "Resource.h"
#include "IFCVisualExplorer.h"
#include "IFCModel.h"
#include "IFCSchemaViewConsts.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// CIFCSchemaView

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CIFCSchemaView::OnModelLoadedEvent(CIFCModel* pModel)
{
	ResetView();
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CIFCSchemaView::OnActiveModelChangedEvent(const CIFCView * pSender)
{
	ResetView();
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CIFCSchemaView::OnAllModelsDeleted()
{
	ResetView();
}

// ------------------------------------------------------------------------------------------------
void CIFCSchemaView::LoadModel(CIFCModel * pModel)
{
	ASSERT(pModel != NULL);

	CIFCController * pController = GetController();
	ASSERT(pController != NULL);

	/**********************************************************************************************
	* Model
	*/		
	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = NULL;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = (LPWSTR)pModel->getModelName();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_MODEL;
	tvInsertStruct.item.lParam = NULL;

	HTREEITEM hModel = m_ifcTreeCtrl.InsertItem(&tvInsertStruct);

	if (pController->GetActiveModel() == pModel)
	{
		m_ifcTreeCtrl.SetItemState(hModel, TVIS_BOLD, TVIS_BOLD);
	}
	//*********************************************************************************************

	// Roots **************************************************************************************
	const map<int_t, CIFCEntity *> & mapEntities = pModel->getEntities();
	if (mapEntities.empty())
	{
		return;
	}

	map<wstring, CIFCEntity *> mapRoots;

	map<int_t, CIFCEntity *>::const_iterator itEntity = mapEntities.begin();
	for (; itEntity != mapEntities.end(); itEntity++)
	{
		if (itEntity->second->hasParent())
		{
			continue;
		}

		mapRoots[itEntity->second->getName()] = itEntity->second;
	}	

	map<wstring, CIFCEntity *>::iterator itRoot = mapRoots.begin();
	for (; itRoot != mapRoots.end(); itRoot++)
	{
		LoadEntity(itRoot->second, hModel);
	}
	// ********************************************************************************************	

	m_ifcTreeCtrl.Expand(hModel, TVE_EXPAND);
}

// ------------------------------------------------------------------------------------------------
void CIFCSchemaView::LoadAttributes(CIFCEntity * pIFCEntity, HTREEITEM hParent)
{
	ASSERT(pIFCEntity != NULL);

	if (pIFCEntity->getAttributesCount() == 0)
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

	for (size_t iAttribute = 0; iAttribute < pIFCEntity->getAttributes().size(); iAttribute++)
	{
		const wstring& strAttribute = pIFCEntity->getAttributes()[iAttribute];
		if (pIFCEntity->isAttributeInherited(strAttribute))
		{
			continue;
		}

		tvInsertStruct.hParent = hAttributes;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = (LPWSTR)pIFCEntity->getAttributes()[iAttribute].c_str();
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = 
			pIFCEntity->isAttributeIgnored(pIFCEntity->getAttributes()[iAttribute]) ? IMAGE_IGNORED_ATTRIBUTE : IMAGE_ATTRIBUTE;
		tvInsertStruct.item.lParam = (LPARAM)pIFCEntity;

		HTREEITEM hAttribute = m_ifcTreeCtrl.InsertItem(&tvInsertStruct);
	} // for (size_t iAttribute = ...
}

// ------------------------------------------------------------------------------------------------
void CIFCSchemaView::LoadEntity(CIFCEntity * pIFCEntity, HTREEITEM hParent)
{
	CIFCController * pController = GetController();
	ASSERT(pController != NULL);

	CIFCModel * pModel = pController->GetActiveModel();

	/*
	* Entity
	*/
	pair<int, int>  prInstancesCount = GetInstancesCount(pIFCEntity);

	CString strEntity;
	strEntity.Format(_T("%s (%d/%d)"), pIFCEntity->getName(), prInstancesCount.first, prInstancesCount.second);

	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = hParent;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = (LPTSTR)strEntity.GetBuffer();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_ENTITY;
	tvInsertStruct.item.lParam = (LPARAM)pIFCEntity;

	HTREEITEM hEntity = m_ifcTreeCtrl.InsertItem(&tvInsertStruct);

	LoadAttributes(pIFCEntity, hEntity);

	/*
	* Sub-types
	*/
	if (pIFCEntity->getSubTypes().size() > 0)
	{
		tvInsertStruct.hParent = hEntity;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = ITEM_SUB_TYPES;
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_SUB_TYPES;
		tvInsertStruct.item.lParam = NULL;	

		HTREEITEM hSubType = m_ifcTreeCtrl.InsertItem(&tvInsertStruct);

		for (size_t iSubType = 0; iSubType < pIFCEntity->getSubTypes().size(); iSubType++)
		{
			LoadEntity(pIFCEntity->getSubTypes()[iSubType], hSubType);
		}
	} // if (pIFCEntity->getSubTypes().size() > 0)
}

// ------------------------------------------------------------------------------------------------
pair<int, int> CIFCSchemaView::GetInstancesCount(CIFCEntity * pEntity) const
{
	ASSERT(pEntity != NULL);

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
void CIFCSchemaView::OnNMClickTreeIFC(NMHDR * pNMHDR, LRESULT * pResult)
{
	*pResult = 0;
}

// ----------------------------------------------------------------------------
void CIFCSchemaView::OnNMRClickTreeIFC(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 1;

	CIFCController * pController = GetController();
	ASSERT(pController != NULL);

	DWORD dwPosition = GetMessagePos();
	CPoint point(LOWORD(dwPosition), HIWORD(dwPosition));
	m_ifcTreeCtrl.ScreenToClient(&point);

	UINT uFlags = 0;
	HTREEITEM hItem = m_ifcTreeCtrl.HitTest(point, &uFlags);

	if ((hItem == NULL) || (m_ifcTreeCtrl.GetItemData(hItem) == NULL))
	{
		return;
	}

	m_ifcTreeCtrl.SelectItem(hItem);

	CIFCEntity * pIFCEntity = (CIFCEntity *)m_ifcTreeCtrl.GetItemData(hItem);

	CMenu menu;
	VERIFY(menu.CreatePopupMenu());

	int iImage = -1;
	int iSelectedImage = -1;
	m_ifcTreeCtrl.GetItemImage(hItem, iImage, iSelectedImage);

	ASSERT(iImage == iSelectedImage);

	if ((iImage == IMAGE_ATTRIBUTE) || (iImage == IMAGE_IGNORED_ATTRIBUTE))
	{
		// ********************************************************************************************
		// Ignore Attribute
		CString strIgnoreAttribute;
		VERIFY(strIgnoreAttribute.LoadStringW(IDS_IGNORE_ATTRIBUTE));

		menu.AppendMenu(MF_STRING | (iImage == IMAGE_IGNORED_ATTRIBUTE ? MF_CHECKED : MF_UNCHECKED), IDS_IGNORE_ATTRIBUTE, strIgnoreAttribute);
		// ********************************************************************************************
	} // if ((iImage == IMAGE_ATTRIBUTE) || ...
	else
	{
		// ********************************************************************************************
		// View IFC Relations & Properties
		CString strViewIFCRelations;
		VERIFY(strViewIFCRelations.LoadStringW(IDS_VIEW_IFC_RELATIONS));

		CString strViewInstanceProperties;
		VERIFY(strViewInstanceProperties.LoadStringW(IDS_VIEW_INSTANCE_PROPERTIES));

		menu.AppendMenu(MF_STRING, IDS_VIEW_IFC_RELATIONS, strViewIFCRelations);
		menu.AppendMenu(MF_STRING, IDS_VIEW_INSTANCE_PROPERTIES, strViewInstanceProperties);
		// ********************************************************************************************
	} // else if ((iImage == IMAGE_ATTRIBUTE) || ...

	// ********************************************************************************************
	menu.AppendMenu(MF_SEPARATOR, 0, _T(""));

	// Save/Load Schema
	CString strSaveSchema;
	VERIFY(strSaveSchema.LoadStringW(IDS_SAVE_SCHEMA));

	CString strLoadSchema;
	VERIFY(strLoadSchema.LoadStringW(IDS_LOAD_SCHEMA));

	menu.AppendMenu(MF_STRING, IDS_SAVE_SCHEMA, strSaveSchema);
	menu.AppendMenu(MF_STRING, IDS_LOAD_SCHEMA, strLoadSchema);
	// ********************************************************************************************

	CPoint pointScreen(LOWORD(dwPosition), HIWORD(dwPosition));
	int iResult = menu.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN, pointScreen.x, pointScreen.y, &m_ifcTreeCtrl);
	switch (iResult)
	{
		case IDS_IGNORE_ATTRIBUTE:
		{
			CString strAttribute = m_ifcTreeCtrl.GetItemText(hItem);

			if (iImage == IMAGE_ATTRIBUTE)
			{
				pIFCEntity->ignoreAttribute((LPCTSTR)strAttribute, true);

				m_ifcTreeCtrl.SetItemImage(hItem, IMAGE_IGNORED_ATTRIBUTE, IMAGE_IGNORED_ATTRIBUTE);
			}
			else
			{
				pIFCEntity->ignoreAttribute((LPCTSTR)strAttribute, false);

				m_ifcTreeCtrl.SetItemImage(hItem, IMAGE_ATTRIBUTE, IMAGE_ATTRIBUTE);
			}
		}
		break;

		case IDS_VIEW_IFC_RELATIONS:
		{
			pController->FireOnViewRelations(this, pIFCEntity);
		}
		break;

		case IDS_VIEW_INSTANCE_PROPERTIES:
		{
			pController->FireOnViewInstanceProperties(this, pIFCEntity);
		}
		break;

		case IDS_SAVE_SCHEMA:
		{
			CIFCModel* pModel = pController->GetActiveModel();
			ASSERT(pModel != NULL);

			pModel->SaveSchema();
		}
		break;

		case IDS_LOAD_SCHEMA:
		{
			CIFCModel* pModel = pController->GetActiveModel();
			ASSERT(pModel != NULL);

			pModel->LoadSchema();

			ResetView();
		}
		break;
	} // switch (iResult)

	VERIFY(menu.DestroyMenu());
}

// ----------------------------------------------------------------------------
void CIFCSchemaView::OnTvnItemexpandingTreeIFC(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	*pResult = 0;
}

CIFCSchemaView::CIFCSchemaView()
	: m_pSearchDialog(NULL)
{
}

CIFCSchemaView::~CIFCSchemaView()
{
}

BEGIN_MESSAGE_MAP(CIFCSchemaView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	//ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_PROPERTIES, OnProperties)
	ON_COMMAND(ID_OPEN, OnFileOpen)
	ON_COMMAND(ID_OPEN_WITH, OnFileOpenWith)
	ON_COMMAND(ID_DUMMY_COMPILE, OnDummyCompile)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
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

	ASSERT(GetController() != NULL);
	GetController()->RegisterView(this);

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create view:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;

	if (!m_ifcTreeCtrl.Create(dwViewStyle, rectDummy, this, IDC_TREE_IFC))
	{
		TRACE0("Failed to create IFC Instances View\n");
		return -1;      // fail to create
	}

	// Load view images:
	m_imageList.Create(IDB_CLASS_VIEW, 16, 0, RGB(255, 0, 0));
	m_ifcTreeCtrl.SetImageList(&m_imageList, TVSIL_NORMAL);

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_EXPLORER);
	m_wndToolBar.LoadToolBar(IDR_EXPLORER, 0, 0, TRUE /* Is locked */);

	OnChangeVisualStyle();

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	m_wndToolBar.SetOwner(this);

	// All commands will be routed via this control , not via the parent frame:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	ResetView();
	AdjustLayout();

	//  Search
	m_pSearchDialog = new CSearchSchemaDialog(&m_ifcTreeCtrl);
	m_pSearchDialog->Create(IDD_DIALOG_SEARCH_SCHEMA, this);

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

	CIFCController * pController = GetController();
	ASSERT(pController != NULL);

	CIFCModel * pModel = pController->GetActiveModel();

	if (pModel != NULL)
	{
		LoadModel(pModel);
	}	
}

void CIFCSchemaView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CTreeCtrl* pWndTree = (CTreeCtrl*) &m_ifcTreeCtrl;
	ASSERT_VALID(pWndTree);

	if (pWnd != pWndTree)
	{
		CDockablePane::OnContextMenu(pWnd, point);
		return;
	}

	if (point != CPoint(-1, -1))
	{
		// Select clicked item:
		CPoint ptTree = point;
		pWndTree->ScreenToClient(&ptTree);

		UINT flags = 0;
		HTREEITEM hTreeItem = pWndTree->HitTest(ptTree, &flags);
		if (hTreeItem != NULL)
		{
			pWndTree->SelectItem(hTreeItem);
		}
	}

	pWndTree->SetFocus();
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EXPLORER, point.x, point.y, this, TRUE);
}

void CIFCSchemaView::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_ifcTreeCtrl.SetWindowPos(NULL, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
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

void CIFCSchemaView::OnFileOpen()
{
	// TODO: Add your command handler code here
}

void CIFCSchemaView::OnFileOpenWith()
{
	// TODO: Add your command handler code here
}

void CIFCSchemaView::OnDummyCompile()
{
	// TODO: Add your command handler code here
}

void CIFCSchemaView::OnEditCut()
{
	// TODO: Add your command handler code here
}

void CIFCSchemaView::OnEditCopy()
{
	// TODO: Add your command handler code here
}

void CIFCSchemaView::OnEditClear()
{
	// TODO: Add your command handler code here
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
