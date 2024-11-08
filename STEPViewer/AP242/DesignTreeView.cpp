
#include "stdafx.h"
#include "MainFrm.h"
#include "STEPViewer.h"
#include "DesignTreeView.h"
#include "Resource.h"
#include "IFCModel.h"
#include "STEPModel.h"
#include "ProductDefinition.h"

#include <algorithm>
using namespace std;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// ************************************************************************************************
#define IMAGE_MODEL    2
#define IMAGE_INSTANCE 1
#define IMAGE_PROPERTY 3
#define IMAGE_VALUE    5

// ************************************************************************************************
class CDesignTreeViewMenuButton : public CMFCToolBarMenuButton
{

	DECLARE_SERIAL(CDesignTreeViewMenuButton)

public:
	CDesignTreeViewMenuButton(HMENU hMenu = nullptr) : CMFCToolBarMenuButton((UINT)-1, hMenu, -1)
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

IMPLEMENT_SERIAL(CDesignTreeViewMenuButton, CMFCToolBarMenuButton, 1)

// ************************************************************************************************
/*virtual*/ void CDesignTreeView::OnModelChanged() /*override*/
{
	ResetView();

	delete m_pPropertyProvider;
	m_pPropertyProvider = nullptr;

	auto pModel = GetModel<CModel>();
	if ((pModel == nullptr) || (pModel->GetInstance() == 0))
	{
		return;
	}
	
	m_pPropertyProvider = new COWLPropertyProvider();

	m_treeCtrl.InsertItem(pModel->getPath(), IMAGE_MODEL, IMAGE_MODEL);
}

/*virtual*/ void CDesignTreeView::OnInstanceSelected(CViewBase* pSender) /*override*/
{
	if (pSender == this)
	{
		return;
	}

	ResetView();
		
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
		
	HTREEITEM hModel = m_treeCtrl.InsertItem(pModel->getPath(), IMAGE_MODEL, IMAGE_MODEL);

	auto pSelectedInstance = pController->GetSelectedInstance();
	if (pSelectedInstance == nullptr)
	{	
		m_treeCtrl.SelectItem(hModel);
		
		return;
	}

	OwlInstance iInstance = 0;
	owlBuildInstance(pModel->GetInstance(), pSelectedInstance->GetInstance(), &iInstance);

	if (iInstance == 0)
	{
		ExpressID iExpressID = internalGetP21Line(pSelectedInstance->GetInstance());
		if (iExpressID != 0)
		{
			iInstance = internalGetInstanceFromP21Line(pModel->GetInstance(), iExpressID);
		}
	}

	if (iInstance == 0)
	{
		ASSERT(FALSE);

		return;
	}

	AddInstance(hModel, iInstance);

	m_treeCtrl.Expand(hModel, TVE_EXPAND);
}

/*virtual*/ CTreeCtrlEx* CDesignTreeView::GetTreeView() /*override*/
{
	return &m_treeCtrl;
}

/*virtual*/ vector<CString> CDesignTreeView::GetSearchFilters() /*override*/
{
	return vector<CString>
	{
		_T("(All)"),
		_T("Instances"),
		_T("Properties"),
		_T("Values"),
	};
}

/*virtual*/ void CDesignTreeView::LoadChildrenIfNeeded(HTREEITEM hItem) /*override*/
{
	if (hItem == NULL)
	{
		ASSERT(FALSE);

		return;
	}

	TVITEMW tvItem = {};
	tvItem.hItem = hItem;
	tvItem.mask = TVIF_HANDLE | TVIF_CHILDREN;

	if (!GetTreeView()->GetItem(&tvItem))
	{
		ASSERT(FALSE);

		return;
	}

	if (tvItem.cChildren != 1)
	{
		return;
	}

	if (m_treeCtrl.GetChildItem(hItem) == nullptr)
	{
		GetTreeView()->Expand(hItem, TVE_EXPAND);
	}
}

/*virtual*/ BOOL CDesignTreeView::ContainsText(int iFilter, HTREEITEM hItem, const CString& strText) /*override*/
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

	// Instances
	if (iFilter == (int)enumSearchFilter::Instances)
	{
		int iImage, iSelectedImage = -1;
		GetTreeView()->GetItemImage(hItem, iImage, iSelectedImage);

		ASSERT(iImage == iSelectedImage);

		if (iImage == IMAGE_INSTANCE)
		{
			return strItemText.Find(strTextLower, 0) != -1;
		}
		else
		{
			return FALSE;
		}
	}

	// Properties
	if (iFilter == (int)enumSearchFilter::Properties)
	{
		int iImage, iSelectedImage = -1;
		GetTreeView()->GetItemImage(hItem, iImage, iSelectedImage);

		ASSERT(iImage == iSelectedImage);

		if (iImage == IMAGE_PROPERTY)
		{
			return strItemText.Find(strTextLower, 0) != -1;
		}
		else
		{
			return FALSE;
		}
	}

	// Values
	if (iFilter == (int)enumSearchFilter::Values)
	{
		int iImage, iSelectedImage = -1;
		GetTreeView()->GetItemImage(hItem, iImage, iSelectedImage);

		ASSERT(iImage == iSelectedImage);

		if (iImage == IMAGE_VALUE)
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

void CDesignTreeView::ResetView()
{
	// UI
	m_treeCtrl.DeleteAllItems();
	m_pSearchDialog->Reset();

	// Data
	Clean();
}

void CDesignTreeView::AddInstance(HTREEITEM hParent, OwlInstance iInstance)
{
	/*
	* The instances will be loaded on demand
	*/
	wstring strItem = CInstanceBase::GetName((SdaiInstance)iInstance);

	TV_INSERTSTRUCT tvInsertStruct;
 	tvInsertStruct.hParent = hParent;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
	tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_INSTANCE;
	tvInsertStruct.item.lParam = NULL;
	tvInsertStruct.item.cChildren = 1;

	HTREEITEM hInstance = m_treeCtrl.InsertItem(&tvInsertStruct);

	auto itInstance2Data = m_mapInstance2Data.find(iInstance);
	if (itInstance2Data == m_mapInstance2Data.end())
	{
		auto pInstanceData = new CInstanceData(iInstance);
		pInstanceData->Items().push_back(hInstance);

		m_mapInstance2Data[iInstance] = pInstanceData;

		m_treeCtrl.SetItemData(hInstance, (DWORD_PTR)pInstanceData);
	}
	else
	{
		itInstance2Data->second->Items().push_back(hInstance);

		m_treeCtrl.SetItemData(hInstance, (DWORD_PTR)itInstance2Data->second);
	}
}

void CDesignTreeView::AddProperties(HTREEITEM hParent, OwlInstance iInstance)
{
	if (iInstance == 0)
	{
		ASSERT(FALSE);

		return;
	}

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

	auto pPropertyCollection = m_pPropertyProvider->GetPropertyCollection(iInstance);
	if (pPropertyCollection != nullptr)
	{
		wchar_t szBuffer[100];

		for (auto pProperty : pPropertyCollection->Properties())
		{
			wstring strProperty = pProperty->GetName();
			strProperty += L" : ";
			strProperty += pProperty->GetTypeName();

			HTREEITEM hProperty = m_treeCtrl.InsertItem(strProperty.c_str(), IMAGE_PROPERTY, IMAGE_PROPERTY, hParent);

			/*
			* rdfs:range
			*/
			vector<int64_t> vecRestrictionClasses;

			wstring strRange = L"rdfs:range : ";
			strRange += pProperty->GetRange(vecRestrictionClasses);

			HTREEITEM hRange = m_treeCtrl.InsertItem(strRange.c_str(), IMAGE_PROPERTY, IMAGE_PROPERTY, hProperty);

			for (auto iRestrictionClass : vecRestrictionClasses)
			{
				wchar_t* szClassName = nullptr;
				GetNameOfClassW(iRestrictionClass, &szClassName);

				m_treeCtrl.InsertItem(szClassName, IMAGE_VALUE, IMAGE_VALUE, hRange);
			}
			
			/*
			* owl:cardinality
			*/
			wstring strCardinality = L"owl:cardinality : ";
			strCardinality += pProperty->GetCardinality(iInstance);

			m_treeCtrl.InsertItem(strCardinality.c_str(), IMAGE_VALUE, IMAGE_VALUE, hProperty);

			/*
			* value(s)
			*/
			switch (pProperty->GetType())
			{
				case OBJECTTYPEPROPERTY_TYPE:
				{
					OwlInstance* piInstances = nullptr;
					int64_t iValuesCount = 0;
					GetObjectProperty(iInstance, pProperty->GetInstance(), &piInstances, &iValuesCount);

					for (int64_t iValue = 0; iValue < iValuesCount; iValue++)
					{
						if (piInstances[iValue] != 0)
						{
							AddInstance(hProperty, piInstances[iValue]);
						}
						else
						{
							m_treeCtrl.InsertItem(EMPTY_INSTANCE, IMAGE_INSTANCE, IMAGE_INSTANCE, hProperty);
  						}
					} // for (int64_t iInstance = ...
				}
				break;

				case DATATYPEPROPERTY_TYPE_BOOLEAN:
				{
					bool* pbValue = nullptr;
					int64_t iValuesCount = 0;
					GetDatatypeProperty(iInstance, pProperty->GetInstance(), (void**)&pbValue, &iValuesCount);

					for (int64_t iValue = 0; iValue < iValuesCount; iValue++)
					{
						swprintf(szBuffer, 100, L"value = %s", pbValue[iValue] ? L"TRUE" : L"FALSE");
						m_treeCtrl.InsertItem(szBuffer, IMAGE_VALUE, IMAGE_VALUE, hProperty);
					}
				}
				break;

				case DATATYPEPROPERTY_TYPE_STRING:
				{
					char** szValue = nullptr;
					int64_t iValuesCount = 0;
					GetDatatypeProperty(iInstance, pProperty->GetInstance(), (void**)&szValue, &iValuesCount);

					for (int64_t iValue = 0; iValue < iValuesCount; iValue++)
					{
						wstring strValue = CA2W(szValue[iValue]);
						swprintf(szBuffer, 100, L"value = '%s'", strValue.c_str());

						m_treeCtrl.InsertItem(szBuffer, IMAGE_VALUE, IMAGE_VALUE, hProperty);
					} // for (int64_t iValue = ...
				}
				break;				

				case DATATYPEPROPERTY_TYPE_INTEGER:
				{
					int64_t* piValue = nullptr;
					int64_t iValuesCount = 0;
					GetDatatypeProperty(iInstance, pProperty->GetInstance(), (void**)&piValue, &iValuesCount);

					for (int64_t iValue = 0; iValue < iValuesCount; iValue++)
					{
						swprintf(szBuffer, 100, L"value = %lld", piValue[iValue]);

						m_treeCtrl.InsertItem(szBuffer, IMAGE_VALUE, IMAGE_VALUE, hProperty);
					}
				}
				break;

				case DATATYPEPROPERTY_TYPE_DOUBLE:
				{
					double* pdValue = nullptr;
					int64_t iValuesCount = 0;
					GetDatatypeProperty(iInstance, pProperty->GetInstance(), (void**)&pdValue, &iValuesCount);

					for (int64_t iValue = 0; iValue < iValuesCount; iValue++)
					{
						swprintf(szBuffer, 100, L"value = %.6f", pdValue[iValue]);

						m_treeCtrl.InsertItem(szBuffer, IMAGE_VALUE, IMAGE_VALUE, hProperty);
					}
				}
				break;

				case DATATYPEPROPERTY_TYPE_BYTE:
				{
					BYTE* piValue = nullptr;
					int64_t iValuesCount = 0;
					GetDatatypeProperty(iInstance, pProperty->GetInstance(), (void**)&piValue, &iValuesCount);

					for (int64_t iValue = 0; iValue < iValuesCount; iValue++)
					{
						swprintf(szBuffer, 100, L"value = %d", piValue[iValue]);

						m_treeCtrl.InsertItem(szBuffer, IMAGE_VALUE, IMAGE_VALUE, hProperty);
					}
				}
				break;

				default:
				{
					ASSERT(false); // unknown property
				}
				break;
			} // switch (pProperty->GetType())
		} // for (auto pProperty ...
	} // if (pPropertyCollection != nullptr)
	else
	{
		ASSERT(FALSE); // Internal error!
	}	
}

void CDesignTreeView::Clean()
{
	auto itInstance2Data = m_mapInstance2Data.begin();
	for (; itInstance2Data != m_mapInstance2Data.end(); itInstance2Data++)
	{
		delete itInstance2Data->second;
	}
	m_mapInstance2Data.clear();
}

// ************************************************************************************************
CDesignTreeView::CDesignTreeView()
	: m_pPropertyProvider(nullptr)
	, m_mapInstance2Data()
	, m_bInitInProgress(false)
	, m_pSearchDialog(nullptr)
{}

CDesignTreeView::~CDesignTreeView()
{	
	Clean();

	delete m_pPropertyProvider;
	m_pPropertyProvider = nullptr;
}

BEGIN_MESSAGE_MAP(CDesignTreeView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_PROPERTIES, OnProperties)
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_TREE_INSTANCE_VIEW, OnItemExpanding)
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

int CDesignTreeView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	ASSERT(GetController() != nullptr);
	GetController()->RegisterView(this);

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create view:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS;

	if (!m_treeCtrl.Create(dwViewStyle, rectDummy, this, IDC_TREE_INSTANCE_VIEW))
	{
		TRACE0("Failed to create file view\n");
		return -1;      // fail to create
	}

	// Load view images:
	m_images.Create(IDB_CLASS_VIEW, 16, 0, RGB(255, 0, 0));
	m_treeCtrl.SetImageList(&m_images, TVSIL_NORMAL);

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

void CDesignTreeView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CDesignTreeView::OnProperties()
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

void CDesignTreeView::AdjustLayout()
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

void CDesignTreeView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rectTree;
	m_treeCtrl.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CDesignTreeView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_treeCtrl.SetFocus();
}

void CDesignTreeView::OnChangeVisualStyle()
{
	m_images.DeleteImageList();

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

	m_images.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
	m_images.Add(&bmp, RGB(255, 0, 0));

	m_treeCtrl.SetImageList(&m_images, TVSIL_NORMAL);

	m_toolBar.CleanUpLockedImages();
	m_toolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_EXPLORER_24 : IDR_EXPLORER, 0, 0, TRUE /* Locked */);
}

void CDesignTreeView::OnDestroy()
{
	ASSERT(GetController() != nullptr);
	GetController()->UnRegisterView(this);

	__super::OnDestroy();

	delete m_pSearchDialog;
}

void CDesignTreeView::OnItemExpanding(NMHDR * pNMHDR, LRESULT * pResult)
{
	*pResult = 0;

	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	
	if (m_treeCtrl.GetChildItem(pNMTreeView->itemNew.hItem) != nullptr)
	{
		// it is loaded already
		return;
	}

	auto pItem = (CItemData*)m_treeCtrl.GetItemData(pNMTreeView->itemNew.hItem);
	if (pItem != nullptr)
	{
		ASSERT(pItem->GetType() == enumItemType::Instance);

		auto pInstanceData = dynamic_cast<CInstanceData*>(pItem);
		AddProperties(pNMTreeView->itemNew.hItem, pInstanceData->GetInstance());
	}
}

void CDesignTreeView::OnShowWindow(BOOL bShow, UINT nStatus)
{
	__super::OnShowWindow(bShow, nStatus);

	if (!bShow)
	{
		m_pSearchDialog->ShowWindow(SW_HIDE);
	}
}
