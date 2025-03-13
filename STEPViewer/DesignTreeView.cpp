
#include "stdafx.h"

#include "_rdf_instance.h"

#include "MainFrm.h"
#include "STEPViewer.h"
#include "DesignTreeView.h"
#include "Resource.h"

#include <algorithm>
using namespace std;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// ************************************************************************************************
#define IMAGE_MODEL_DESIGN_TREE_VIEW    2
#define IMAGE_INSTANCE_DESIGN_TREE_VIEW 1
#define IMAGE_PROPERTY_DESIGN_TREE_VIEW 3
#define IMAGE_VALUE_DESIGN_TREE_VIEW    5

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
/*virtual*/ void CDesignTreeView::onModelLoaded() /*override*/
{
	ResetView();

	delete m_pPropertyProvider;	
	m_pPropertyProvider = new _rdf_property_provider();
}

/*virtual*/ void CDesignTreeView::onInstanceSelected(_view* pSender) /*override*/
{
	if (pSender == this)
	{
		return;
	}	
		
	auto pController = getController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	ResetView();

	if (pController->getModels().empty())
	{
		return;
	}

	if (pController->getSelectedInstances().empty())
	{
		return;
	}

	map<_ap_model*, vector<SdaiInstance>> mapInstances;
	for (auto pInstance : getController()->getSelectedInstances())
	{
		_ptr<_ap_instance> apInstance(pInstance);

		auto pModel = dynamic_cast<_ap_model*>(pController->getModelByInstance(apInstance->getOwlModel()));
		if (pModel == nullptr)
		{
			ASSERT(FALSE);

			continue;
		}

		auto itInstances = mapInstances.find(pModel);
		if (itInstances != mapInstances.end())
		{
			itInstances->second.push_back(apInstance->getSdaiInstance());
		}
		else
		{
			mapInstances[pModel] = vector<SdaiInstance>{ apInstance->getSdaiInstance() };
		}
	}

	for (auto itInstances : mapInstances)
	{
		HTREEITEM hModel = m_treeCtrl.InsertItem(itInstances.first->getPath(), IMAGE_MODEL_DESIGN_TREE_VIEW, IMAGE_MODEL_DESIGN_TREE_VIEW);

		for (auto sdaiInstance : itInstances.second)
		{
			OwlInstance owlInstance = 0;
			owlBuildInstance(itInstances.first->getSdaiModel(), sdaiInstance, &owlInstance);

			if (owlInstance == 0)
			{
				continue;
			}

			AddInstance(hModel, owlInstance);
		}

		m_treeCtrl.Expand(hModel, TVE_EXPAND);
	} // for (auto itInstances : ...
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

		if (iImage == IMAGE_INSTANCE_DESIGN_TREE_VIEW)
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

		if (iImage == IMAGE_PROPERTY_DESIGN_TREE_VIEW)
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

		if (iImage == IMAGE_VALUE_DESIGN_TREE_VIEW)
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

void CDesignTreeView::AddInstance(HTREEITEM hParent, OwlInstance owlInstance)
{
	/*
	* The instances will be loaded on demand
	*/
	wstring strItem = _model::getInstanceName(owlInstance);

	TV_INSERTSTRUCT tvInsertStruct;
 	tvInsertStruct.hParent = hParent;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
	tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_INSTANCE_DESIGN_TREE_VIEW;
	tvInsertStruct.item.lParam = NULL;
	tvInsertStruct.item.cChildren = 1;

	HTREEITEM hInstance = m_treeCtrl.InsertItem(&tvInsertStruct);

	auto itInstance2Data = m_mapInstance2Data.find(owlInstance);
	if (itInstance2Data == m_mapInstance2Data.end())
	{
		auto pInstanceData = new CInstanceData(owlInstance);
		pInstanceData->Items().push_back(hInstance);

		m_mapInstance2Data[owlInstance] = pInstanceData;

		m_treeCtrl.SetItemData(hInstance, (DWORD_PTR)pInstanceData);
	}
	else
	{
		itInstance2Data->second->Items().push_back(hInstance);

		m_treeCtrl.SetItemData(hInstance, (DWORD_PTR)itInstance2Data->second);
	}
}

void CDesignTreeView::AddProperties(HTREEITEM hParent, OwlInstance owlInstance)
{
	if (owlInstance == 0)
	{
		ASSERT(FALSE);

		return;
	}

	auto pController = getController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pModel = dynamic_cast<_ap_model*>(pController->getModelByInstance(GetModel(owlInstance)));
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pPropertyCollection = m_pPropertyProvider->getPropertyCollection(owlInstance);
	if (pPropertyCollection != nullptr)
	{
		wchar_t szBuffer[100];

		for (auto pProperty : pPropertyCollection->properties())
		{
			wstring strProperty = pProperty->getName();
			strProperty += L" : ";
			strProperty += pProperty->getTypeAsString();

			HTREEITEM hProperty = m_treeCtrl.InsertItem(strProperty.c_str(), IMAGE_PROPERTY_DESIGN_TREE_VIEW, IMAGE_PROPERTY_DESIGN_TREE_VIEW, hParent);

			/*
			* rdfs:range
			*/
			wstring strRange = L"rdfs:range : ";
			strRange += pProperty->getRangeAsString();

			HTREEITEM hRange = m_treeCtrl.InsertItem(strRange.c_str(), IMAGE_PROPERTY_DESIGN_TREE_VIEW, IMAGE_PROPERTY_DESIGN_TREE_VIEW, hProperty);

			vector<OwlClass> vecRestrictionClasses;
			pProperty->getRangeRestrictions(vecRestrictionClasses);
			for (auto iRestrictionClass : vecRestrictionClasses)
			{
				wchar_t* szClassName = nullptr;
				GetNameOfClassW(iRestrictionClass, &szClassName);

				m_treeCtrl.InsertItem(szClassName, IMAGE_VALUE_DESIGN_TREE_VIEW, IMAGE_VALUE_DESIGN_TREE_VIEW, hRange);
			}
			
			/*
			* owl:cardinality
			*/
			wstring strCardinality = L"owl:cardinality : ";
			strCardinality += pProperty->getCardinality(owlInstance);

			m_treeCtrl.InsertItem(strCardinality.c_str(), IMAGE_VALUE_DESIGN_TREE_VIEW, IMAGE_VALUE_DESIGN_TREE_VIEW, hProperty);

			/*
			* value(s)
			*/
			switch (pProperty->getType())
			{
				case OBJECTTYPEPROPERTY_TYPE:
				{
					OwlInstance* powlInstances = nullptr;
					int64_t iValuesCount = 0;
					GetObjectProperty(owlInstance, pProperty->getRdfProperty(), &powlInstances, &iValuesCount);

					for (int64_t iValue = 0; iValue < iValuesCount; iValue++)
					{
						if (powlInstances[iValue] != 0)
						{
							AddInstance(hProperty, powlInstances[iValue]);
						}
						else
						{
							m_treeCtrl.InsertItem(EMPTY_INSTANCE, IMAGE_INSTANCE_DESIGN_TREE_VIEW, IMAGE_INSTANCE_DESIGN_TREE_VIEW, hProperty);
  						}
					} // for (int64_t iInstance = ...
				}
				break;

				case DATATYPEPROPERTY_TYPE_BOOLEAN:
				{
					bool* pbValue = nullptr;
					int64_t iValuesCount = 0;
					GetDatatypeProperty(owlInstance, pProperty->getRdfProperty(), (void**)&pbValue, &iValuesCount);

					for (int64_t iValue = 0; iValue < iValuesCount; iValue++)
					{
						swprintf(szBuffer, 100, L"value = %s", pbValue[iValue] ? L"TRUE" : L"FALSE");
						m_treeCtrl.InsertItem(szBuffer, IMAGE_VALUE_DESIGN_TREE_VIEW, IMAGE_VALUE_DESIGN_TREE_VIEW, hProperty);
					}
				}
				break;

				case DATATYPEPROPERTY_TYPE_STRING:
				{
					char** szValue = nullptr;
					int64_t iValuesCount = 0;
					GetDatatypeProperty(owlInstance, pProperty->getRdfProperty(), (void**)&szValue, &iValuesCount);

					for (int64_t iValue = 0; iValue < iValuesCount; iValue++)
					{
						wstring strValue = CA2W(szValue[iValue]);
						swprintf(szBuffer, 100, L"value = '%s'", strValue.c_str());

						m_treeCtrl.InsertItem(szBuffer, IMAGE_VALUE_DESIGN_TREE_VIEW, IMAGE_VALUE_DESIGN_TREE_VIEW, hProperty);
					} // for (int64_t iValue = ...
				}
				break;				

				case DATATYPEPROPERTY_TYPE_INTEGER:
				{
					int64_t* piValue = nullptr;
					int64_t iValuesCount = 0;
					GetDatatypeProperty(owlInstance, pProperty->getRdfProperty(), (void**)&piValue, &iValuesCount);

					for (int64_t iValue = 0; iValue < iValuesCount; iValue++)
					{
						swprintf(szBuffer, 100, L"value = %lld", piValue[iValue]);

						m_treeCtrl.InsertItem(szBuffer, IMAGE_VALUE_DESIGN_TREE_VIEW, IMAGE_VALUE_DESIGN_TREE_VIEW, hProperty);
					}
				}
				break;

				case DATATYPEPROPERTY_TYPE_DOUBLE:
				{
					double* pdValue = nullptr;
					int64_t iValuesCount = 0;
					GetDatatypeProperty(owlInstance, pProperty->getRdfProperty(), (void**)&pdValue, &iValuesCount);

					for (int64_t iValue = 0; iValue < iValuesCount; iValue++)
					{
						swprintf(szBuffer, 100, L"value = %.6f", pdValue[iValue]);

						m_treeCtrl.InsertItem(szBuffer, IMAGE_VALUE_DESIGN_TREE_VIEW, IMAGE_VALUE_DESIGN_TREE_VIEW, hProperty);
					}
				}
				break;

				case DATATYPEPROPERTY_TYPE_BYTE:
				{
					BYTE* piValue = nullptr;
					int64_t iValuesCount = 0;
					GetDatatypeProperty(owlInstance, pProperty->getRdfProperty(), (void**)&piValue, &iValuesCount);

					for (int64_t iValue = 0; iValue < iValuesCount; iValue++)
					{
						swprintf(szBuffer, 100, L"value = %d", piValue[iValue]);

						m_treeCtrl.InsertItem(szBuffer, IMAGE_VALUE_DESIGN_TREE_VIEW, IMAGE_VALUE_DESIGN_TREE_VIEW, hProperty);
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

	ASSERT(getController() != nullptr);
	getController()->registerView(this);

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
	ASSERT(getController() != nullptr);
	getController()->unRegisterView(this);

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
		AddProperties(pNMTreeView->itemNew.hItem, pInstanceData->GetOwlInstance());
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
