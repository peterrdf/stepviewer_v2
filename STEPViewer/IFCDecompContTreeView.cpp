#include "stdafx.h"
#include "mainfrm.h"
#include "IFCDecompContTreeView.h"
#include "ProductDefinition.h"
#include "Resource.h"
#include "STEPViewer.h"
#include "IFCModel.h"
#include "Generic.h"
#include "FileViewConsts.h"

#include <algorithm>
#include <chrono>

using namespace std;

// ------------------------------------------------------------------------------------------------
CIFCDecompContTreeView::CIFCDecompContTreeView(CViewTree* pTreeView)
	: CSTEPTreeViewBase()
	, m_pTreeView(pTreeView)
	, m_pImageList(nullptr)
	, m_mapModelHTREEITEM()
	, m_mapInstance2Item()
	, m_mapSelectedInstances()
	//, m_pSearchDialog(nullptr)
{
	m_pImageList = new CImageList();
	m_pImageList->Create(16, 16, ILC_COLOR4, 6, 6);

	CBitmap bitmap;

	bitmap.LoadBitmap(IDB_SELECTED_ALL);
	m_pImageList->Add(&bitmap, (COLORREF)0x000000);
	bitmap.DeleteObject();

	bitmap.LoadBitmap(IDB_SELECTED_PART);
	m_pImageList->Add(&bitmap, (COLORREF)0x000000);
	bitmap.DeleteObject();

	bitmap.LoadBitmap(IDB_SELECTED_NONE);
	m_pImageList->Add(&bitmap, (COLORREF)0x000000);
	bitmap.DeleteObject();

	bitmap.LoadBitmap(IDB_PROPERTY_SET);
	m_pImageList->Add(&bitmap, (COLORREF)0x000000);
	bitmap.DeleteObject();

	bitmap.LoadBitmap(IDB_PROPERTY);
	m_pImageList->Add(&bitmap, (COLORREF)0x000000);
	bitmap.DeleteObject();

	bitmap.LoadBitmap(IDB_NONE);
	m_pImageList->Add(&bitmap, (COLORREF)0x000000);
	bitmap.DeleteObject();

	(*m_pTreeView).SetImageList(m_pImageList, TVSIL_NORMAL);

	// State provider
	(*m_pTreeView).SetItemStateProvider(this);

	//  Search
	//m_pSearchDialog = new CSearchInstancesDialog(m_pTreeView);
	//m_pSearchDialog->Create(IDD_DIALOG_SEARCH_INSTANCES, m_pTreeView);
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ CIFCDecompContTreeView::~CIFCDecompContTreeView()
{
	(*m_pTreeView).SetImageList(nullptr, TVSIL_NORMAL);

	m_pImageList->DeleteImageList();
	delete m_pImageList;	

	(*m_pTreeView).SetItemStateProvider(nullptr);

	//delete m_pSearchDialog;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CIFCDecompContTreeView::OnInstanceSelected(CSTEPView* pSender)
{	
	if (pSender == this)
	{
		return;
	}

	auto pController = GetController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	// Single instance selection
	UnselectAllItems();

	auto pSelectedInstance = GetController()->GetSelectedInstance() != nullptr ? 
		dynamic_cast<CIFCInstance*>(GetController()->GetSelectedInstance()) : 
		nullptr;

	if (pSelectedInstance != nullptr)
	{
		auto itIInstance2Item = m_mapInstance2Item.find(pSelectedInstance);
		ASSERT(itIInstance2Item != m_mapInstance2Item.end());

		ASSERT(m_mapSelectedInstances.find(pSelectedInstance) == m_mapSelectedInstances.end());
		m_mapSelectedInstances[pSelectedInstance] = itIInstance2Item->second;

		(*m_pTreeView).SetItemState(itIInstance2Item->second, TVIS_BOLD, TVIS_BOLD);
		(*m_pTreeView).EnsureVisible(itIInstance2Item->second);
	}
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CIFCDecompContTreeView::OnInstanceEnabledStateChanged(CSTEPView* pSender, CProductInstance* /*pInstance*/)
{	
	if (pSender == this)
	{
		return;
	}

	ASSERT(FALSE); // TODO
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CIFCDecompContTreeView::OnDisableAllButThis(CSTEPView* pSender, CProductInstance* /*pInstance*/)
{
	if (pSender == this)
	{
		return;
	}

	ASSERT(FALSE); // TODO
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CIFCDecompContTreeView::Load()
{
	ResetView();
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ CImageList* CIFCDecompContTreeView::GetImageList() const
{
	return m_pImageList;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CIFCDecompContTreeView::OnShowWindow(BOOL bShow, UINT /*nStatus*/)
{
	if (!bShow)
	{
		//m_pSearchDialog->ShowWindow(SW_HIDE);
	}
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CIFCDecompContTreeView::OnEnableAllInstances(CSTEPView* pSender)
{
	if (pSender == this)
	{
		return;
	}

	ASSERT(FALSE); // TODO
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CIFCDecompContTreeView::OnTreeItemClick(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	*pResult = 0;

	DWORD dwPosition = GetMessagePos();
	CPoint point(LOWORD(dwPosition), HIWORD(dwPosition));
	(*m_pTreeView).ScreenToClient(&point);

	UINT uFlags = 0;
	HTREEITEM hItem = (*m_pTreeView).HitTest(point, &uFlags);

	auto pController = GetController();
	if(pController == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	/*
	* TVHT_ONITEMICON
	*/
	if ((hItem != nullptr) && ((uFlags & TVHT_ONITEMICON) == TVHT_ONITEMICON))
	{
		int iImage = -1;
		int iSelectedImage = -1;
		(*m_pTreeView).GetItemImage(hItem, iImage, iSelectedImage);

		ASSERT(iImage == iSelectedImage);

		CIFCInstance* pInstance = (CIFCInstance*)(*m_pTreeView).GetItemData(hItem);

		switch (iImage)
		{
			case IMAGE_SELECTED:
			case IMAGE_SEMI_SELECTED:
			{
				(*m_pTreeView).SetItemImage(hItem, IMAGE_NOT_SELECTED, IMAGE_NOT_SELECTED);

				if (pInstance != nullptr)
				{
					pInstance->setEnable(false);
				}

				ClickItem_UpdateChildren(hItem);
				ClickItem_UpdateParent((*m_pTreeView).GetParentItem(hItem));
			}
			break;

			case IMAGE_NOT_SELECTED:
			{
				(*m_pTreeView).SetItemImage(hItem, IMAGE_SELECTED, IMAGE_SELECTED);

				if (pInstance != nullptr)
				{
					pInstance->setEnable(true);
				}

				ClickItem_UpdateChildren(hItem);
				ClickItem_UpdateParent((*m_pTreeView).GetParentItem(hItem));
			}
			break;

			default:
			{
				// skip the properties, items without a geometry, etc.
				return;
			}
		} // switch (iImage)		

		pController->OnInstancesEnabledStateChanged(this);

		return;
	} // if ((hItem != nullptr) && ...

	/*
	* TVHT_ONITEMLABEL
	*/
	if ((hItem != nullptr) && ((uFlags & TVHT_ONITEMLABEL) == TVHT_ONITEMLABEL))
	{
		// Single instance selection
		UnselectAllItems();

		auto pSelectedInstance = (*m_pTreeView).GetItemData(hItem) != NULL ?
			(CIFCInstance*)(*m_pTreeView).GetItemData(hItem) :
			nullptr;

		pController->SelectInstance(this, pSelectedInstance);

		if (pSelectedInstance != nullptr)
		{
			auto itIInstance2Item = m_mapInstance2Item.find(pSelectedInstance);
			ASSERT(itIInstance2Item != m_mapInstance2Item.end());

			ASSERT(m_mapSelectedInstances.find(pSelectedInstance) == m_mapSelectedInstances.end());
			m_mapSelectedInstances[pSelectedInstance] = itIInstance2Item->second;

			(*m_pTreeView).SetItemState(itIInstance2Item->second, TVIS_BOLD, TVIS_BOLD);
			(*m_pTreeView).EnsureVisible(itIInstance2Item->second);
		}
	} // if ((hItem != nullptr) && ...
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CIFCDecompContTreeView::OnTreeItemExpanding(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;	

	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	int iImage = -1;
	int iSelectedImage = -1;
	(*m_pTreeView).GetItemImage(pNMTreeView->itemNew.hItem, iImage, iSelectedImage);

	ASSERT(iImage == iSelectedImage);

	if ((iImage == IMAGE_PROPERTY_SET) && (pNMTreeView->itemNew.cChildren == 1))
	{
		HTREEITEM hChild = (*m_pTreeView).GetChildItem(pNMTreeView->itemNew.hItem);
		ASSERT(hChild != nullptr);

		if ((*m_pTreeView).GetItemText(hChild) != ITEM_PROPERTIES_PENDING)
		{
			return;
		}

		(*m_pTreeView).DeleteItem(hChild);

		HTREEITEM hGeometry = (*m_pTreeView).GetPrevSiblingItem(pNMTreeView->itemNew.hItem);

		CIFCInstance* pInstance = (CIFCInstance*)(*m_pTreeView).GetItemData(hGeometry);
		ASSERT(pInstance != nullptr);

		CIFCModel* pModel = GetModel(hGeometry);

		LoadProperties(pModel, pInstance->getInstance(), pNMTreeView->itemNew.hItem);
	} // if ((iImage == IMAGE_PROPERTY_SET) && ...
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ bool CIFCDecompContTreeView::IsSelected(HTREEITEM hItem)
{
	auto pInstance = (CIFCInstance*)(*m_pTreeView).GetItemData(hItem);
	if (pInstance == nullptr)
	{
		return false;
	}

	auto pController = GetController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return false;
	}

	return pInstance == pController->GetSelectedInstance();
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CIFCDecompContTreeView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	ASSERT_VALID(m_pTreeView);
	if (pWnd != m_pTreeView)
	{
		return;
	}

	if (point == CPoint(-1, -1))
	{
		return;
	}

	// Select clicked item:
	CPoint ptTree = point;
	m_pTreeView->ScreenToClient(&ptTree);

	UINT flags = 0;
	HTREEITEM hItem = m_pTreeView->HitTest(ptTree, &flags);
	if (hItem == nullptr)
	{
		return;
	}

	m_pTreeView->SelectItem(hItem);
	m_pTreeView->SetFocus();

	auto pInstance = (CIFCInstance*)(*m_pTreeView).GetItemData(hItem);
	if (pInstance == nullptr)
	{
		return;
	}

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

	auto pModel = pController->GetModel()->As<CIFCModel>();
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto& mapInstances = pModel->GetInstances();	

	if (pInstance->hasGeometry())
	{
		CMenu menu;
		VERIFY(menu.LoadMenuW(IDR_POPUP_INSTANCES));

		auto pPopup = menu.GetSubMenu(0);		

		// Zoom to
		if (!pInstance->getEnable())
		{
			pPopup->EnableMenuItem(ID_INSTANCES_ZOOM_TO, MF_BYCOMMAND | MF_DISABLED);
		}

		// Save
		if (!pInstance->getEnable())
		{
			pPopup->EnableMenuItem(ID_INSTANCES_SAVE, MF_BYCOMMAND | MF_DISABLED);
		}

		// Enable
		if (pInstance->getEnable())
		{
			pPopup->CheckMenuItem(ID_INSTANCES_ENABLE, MF_BYCOMMAND | MF_CHECKED);
		}

		UINT uiCommand = pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, point.x, point.y, m_pTreeView);
		if (uiCommand == 0)
		{
			return;
		}

		switch (uiCommand)
		{
			case ID_INSTANCES_ZOOM_TO:
			{
				pController->ZoomToInstance();
			}
			break;

			case ID_VIEW_ZOOM_OUT:
			{
				pController->ZoomOut();
			}
			break;

			case ID_INSTANCES_SAVE:
			{
				pController->SaveInstance();
			}
			break;

			case ID_INSTANCES_ENABLE:
			{
				pInstance->setEnable(!pInstance->getEnable());

				int iImage = pInstance->getEnable() ? IMAGE_SELECTED : IMAGE_NOT_SELECTED;
				m_pTreeView->SetItemImage(hItem, iImage, iImage);

				ClickItem_UpdateChildren(hItem);
				ClickItem_UpdateParent((*m_pTreeView).GetParentItem(hItem));

				GetController()->OnInstancesEnabledStateChanged(this);
			}
			break;

			case ID_INSTANCES_DISABLE_ALL_BUT_THIS:
			{
				pInstance->setEnable(true);

				int iImage = pInstance->getEnable() ? IMAGE_SELECTED : IMAGE_NOT_SELECTED;
				m_pTreeView->SetItemImage(hItem, iImage, iImage);

				auto itInstance = mapInstances.begin();
				for (; itInstance != mapInstances.end(); itInstance++)
				{
					if (itInstance->second != pInstance)
					{
						itInstance->second->setEnable(false);
					}					
				}

				ResetView();
				OnInstanceSelected(nullptr);

				GetController()->OnInstancesEnabledStateChanged(this);
			}
			break;

			case ID_INSTANCES_ENABLE_ALL:
			{
				auto itInstance = mapInstances.begin();
				for (; itInstance != mapInstances.end(); itInstance++)
				{
					itInstance->second->setEnable(true);
				}

				ResetView();
				OnInstanceSelected(nullptr);

				GetController()->OnInstancesEnabledStateChanged(this);
			}
			break;
		} // switch (uiCommand)

		return;
	} // if (pInstance->hasGeometry())
}

// ------------------------------------------------------------------------------------------------
CIFCModel* CIFCDecompContTreeView::GetModel(HTREEITEM hItem)
{
	ASSERT(hItem != nullptr);

	HTREEITEM hModel = nullptr;
	HTREEITEM hParent = (*m_pTreeView).GetParentItem(hItem);
	if (hParent != nullptr)
	{
		// Item
		while (hParent != nullptr)
		{
			hModel = hParent;

			hParent = (*m_pTreeView).GetParentItem(hParent);
		}
	}
	else
	{
		// Model
		hModel = hItem;
	}

	ASSERT(m_mapModelHTREEITEM.find(hModel) != m_mapModelHTREEITEM.end());

	return m_mapModelHTREEITEM[hModel];
}

// ------------------------------------------------------------------------------------------------
HTREEITEM CIFCDecompContTreeView::GetModelHTREEITEM(HTREEITEM hItem)
{
	ASSERT(hItem != nullptr);

	HTREEITEM hModel = nullptr;
	HTREEITEM hParent = (*m_pTreeView).GetParentItem(hItem);
	if (hParent != nullptr)
	{
		// Item
		while (hParent != nullptr)
		{
			hModel = hParent;

			hParent = (*m_pTreeView).GetParentItem(hParent);
		}
	}
	else
	{
		// Model
		hModel = hItem;
	}

	ASSERT(hModel != nullptr);

	return hModel;
}

// ------------------------------------------------------------------------------------------------
HTREEITEM CIFCDecompContTreeView::GetActiveModelHTREEITEM()
{
	CSTEPController* pController = GetController();
	ASSERT(pController != nullptr);

	map<HTREEITEM, CIFCModel*>::iterator itModelHTREEITEM = m_mapModelHTREEITEM.begin();
	for (; itModelHTREEITEM != m_mapModelHTREEITEM.end(); itModelHTREEITEM++)
	{
		if (pController->GetModel() == itModelHTREEITEM->second)
		{
			return itModelHTREEITEM->first;
		}
	}

	return nullptr;
}

// ------------------------------------------------------------------------------------------------
void CIFCDecompContTreeView::LoadModel(CIFCModel* pModel)
{
	CSTEPController* pController = GetController();
	ASSERT(pController != nullptr);

	/**********************************************************************************************
	* Model
	*/
	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = nullptr;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = (LPWSTR)pModel->getModelName();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_SELECTED;
	tvInsertStruct.item.lParam = NULL;

	HTREEITEM hModel = (*m_pTreeView).InsertItem(&tvInsertStruct);

	if (pController->GetModel() == pModel)
	{
		(*m_pTreeView).SetItemState(hModel, TVIS_BOLD, TVIS_BOLD);
	}

	m_mapModelHTREEITEM[hModel] = pModel;
	//*********************************************************************************************

	/*
	* Header
	*/
	LoadHeader(pModel, hModel);

	/**********************************************************************************************
	* Project/Units/Unreferenced
	*/
	int64_t* iIFCProjectInstances = sdaiGetEntityExtentBN(pModel->getModel(), (char*)"IFCPROJECT");

	int64_t iIFCProjectInstancesCount = sdaiGetMemberCount(iIFCProjectInstances);
	if (iIFCProjectInstancesCount > 0)
	{
		int64_t	iIFCProjectInstance = 0;
		engiGetAggrElement(iIFCProjectInstances, 0, sdaiINSTANCE, &iIFCProjectInstance);

		/*
		* Project
		*/
		LoadProject(pModel, hModel, iIFCProjectInstance);

		/*
		* Unreferenced
		*/
		LoadUnreferencedItems(pModel, hModel);

		LoadTree_UpdateItems(hModel);
	} // if (iIFCProjectInstancesCount > 0)
	//*********************************************************************************************

	(*m_pTreeView).Expand(hModel, TVE_EXPAND);
}

// ------------------------------------------------------------------------------------------------
void CIFCDecompContTreeView::LoadHeader(CIFCModel* pModel, HTREEITEM hModel)
{
	/*********************************************************************************************
	* Header
	*/
	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = hModel;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = L"Header";
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY_SET;
	tvInsertStruct.item.lParam = NULL;

	HTREEITEM hHeader = (*m_pTreeView).InsertItem(&tvInsertStruct);
	//*********************************************************************************************	

	wchar_t* szText = nullptr;

	/*
	* Descriptions
	*/
	{
		tvInsertStruct.hParent = hHeader;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = L"Descriptions";
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY_SET;
		tvInsertStruct.item.lParam = NULL;

		HTREEITEM hDescriptions = (*m_pTreeView).InsertItem(&tvInsertStruct);

		int_t iItem = 0;
		if (!GetSPFFHeaderItem(pModel->getModel(), 0, iItem, sdaiUNICODE, (char**)&szText))
		{
			while (!GetSPFFHeaderItem(pModel->getModel(), 0, iItem++, sdaiUNICODE, (char**)&szText))
			{
				/*
				* Item
				*/
				tvInsertStruct.hParent = hDescriptions;
				tvInsertStruct.hInsertAfter = TVI_LAST;
				tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
				tvInsertStruct.item.pszText = szText;
				tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
				tvInsertStruct.item.lParam = NULL;

				(*m_pTreeView).InsertItem(&tvInsertStruct);
			}
		} // if (!GetSPFFHeaderItem(...
	}

	/*
	* ImplementationLevel
	*/
	{
		GetSPFFHeaderItem(pModel->getModel(), 1, 0, sdaiUNICODE, (char**)&szText);

		wstring strItem = L"ImplementationLevel";
		strItem += L" = '";
		strItem += szText != nullptr ? szText : L"";
		strItem += L"'";

		tvInsertStruct.hParent = hHeader;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
		tvInsertStruct.item.lParam = NULL;

		(*m_pTreeView).InsertItem(&tvInsertStruct);
	}

	/*
	* Name
	*/
	{
		GetSPFFHeaderItem(pModel->getModel(), 2, 0, sdaiUNICODE, (char**)&szText);

		wstring strItem = L"Name";
		strItem += L" = '";
		strItem += szText != nullptr ? szText : L"";
		strItem += L"'";

		tvInsertStruct.hParent = hHeader;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
		tvInsertStruct.item.lParam = NULL;

		(*m_pTreeView).InsertItem(&tvInsertStruct);
	}

	/*
	* TimeStamp
	*/
	{
		GetSPFFHeaderItem(pModel->getModel(), 3, 0, sdaiUNICODE, (char**)&szText);

		wstring strItem = L"TimeStamp";
		strItem += L" = '";
		strItem += szText != nullptr ? szText : L"";
		strItem += L"'";

		tvInsertStruct.hParent = hHeader;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
		tvInsertStruct.item.lParam = NULL;

		(*m_pTreeView).InsertItem(&tvInsertStruct);
	}

	/*
	* Authors
	*/
	{
		tvInsertStruct.hParent = hHeader;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = L"Authors";
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY_SET;
		tvInsertStruct.item.lParam = NULL;

		HTREEITEM hDescriptions = (*m_pTreeView).InsertItem(&tvInsertStruct);

		int_t iItem = 0;
		if (!GetSPFFHeaderItem(pModel->getModel(), 4, iItem, sdaiUNICODE, (char**)&szText))
		{
			while (!GetSPFFHeaderItem(pModel->getModel(), 4, iItem++, sdaiUNICODE, (char**)&szText))
			{
				/*
				* Item
				*/
				tvInsertStruct.hParent = hDescriptions;
				tvInsertStruct.hInsertAfter = TVI_LAST;
				tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
				tvInsertStruct.item.pszText = szText;
				tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
				tvInsertStruct.item.lParam = NULL;

				(*m_pTreeView).InsertItem(&tvInsertStruct);
			}
		} // if (!GetSPFFHeaderItem(...
	}

	/*
	* Organizations
	*/
	{
		tvInsertStruct.hParent = hHeader;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = L"Organizations";
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY_SET;
		tvInsertStruct.item.lParam = NULL;

		HTREEITEM hDescriptions = (*m_pTreeView).InsertItem(&tvInsertStruct);

		int_t iItem = 0;
		if (!GetSPFFHeaderItem(pModel->getModel(), 5, iItem, sdaiUNICODE, (char**)&szText))
		{
			while (!GetSPFFHeaderItem(pModel->getModel(), 5, iItem++, sdaiUNICODE, (char**)&szText))
			{
				/*
				* Item
				*/
				tvInsertStruct.hParent = hDescriptions;
				tvInsertStruct.hInsertAfter = TVI_LAST;
				tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
				tvInsertStruct.item.pszText = szText;
				tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
				tvInsertStruct.item.lParam = NULL;

				(*m_pTreeView).InsertItem(&tvInsertStruct);
			}
		} // if (!GetSPFFHeaderItem(...
	}

	/*
	* PreprocessorVersion
	*/
	{
		GetSPFFHeaderItem(pModel->getModel(), 6, 0, sdaiUNICODE, (char**)&szText);

		wstring strItem = L"PreprocessorVersion";
		strItem += L" = '";
		strItem += szText != nullptr ? szText : L"";
		strItem += L"'";

		tvInsertStruct.hParent = hHeader;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
		tvInsertStruct.item.lParam = NULL;

		(*m_pTreeView).InsertItem(&tvInsertStruct);
	}

	/*
	* OriginatingSystem
	*/
	{
		GetSPFFHeaderItem(pModel->getModel(), 7, 0, sdaiUNICODE, (char**)&szText);

		wstring strItem = L"OriginatingSystem";
		strItem += L" = '";
		strItem += szText != nullptr ? szText : L"";
		strItem += L"'";

		tvInsertStruct.hParent = hHeader;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
		tvInsertStruct.item.lParam = NULL;

		(*m_pTreeView).InsertItem(&tvInsertStruct);
	}

	/*
	* Authorization
	*/
	{
		GetSPFFHeaderItem(pModel->getModel(), 8, 0, sdaiUNICODE, (char**)&szText);

		wstring strItem = L"Authorization";
		strItem += L" = '";
		strItem += szText != nullptr ? szText : L"";
		strItem += L"'";

		tvInsertStruct.hParent = hHeader;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
		tvInsertStruct.item.lParam = NULL;

		(*m_pTreeView).InsertItem(&tvInsertStruct);
	}

	/*
	* FileSchemas
	*/
	{
		tvInsertStruct.hParent = hHeader;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = L"FileSchemas";
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY_SET;
		tvInsertStruct.item.lParam = NULL;

		HTREEITEM hDescriptions = (*m_pTreeView).InsertItem(&tvInsertStruct);

		int_t iItem = 0;
		if (!GetSPFFHeaderItem(pModel->getModel(), 9, iItem, sdaiUNICODE, (char**)&szText))
		{
			while (!GetSPFFHeaderItem(pModel->getModel(), 9, iItem++, sdaiUNICODE, (char**)&szText))
			{
				/*
				* Item
				*/
				tvInsertStruct.hParent = hDescriptions;
				tvInsertStruct.hInsertAfter = TVI_LAST;
				tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
				tvInsertStruct.item.pszText = szText;
				tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
				tvInsertStruct.item.lParam = NULL;

				(*m_pTreeView).InsertItem(&tvInsertStruct);
			}
		} // if (!GetSPFFHeaderItem(...
	}
}

// ------------------------------------------------------------------------------------------------
void CIFCDecompContTreeView::LoadProject(CIFCModel* pModel, HTREEITEM hModel, int64_t iIFCProjectInstance)
{
	CSTEPController* pController = GetController();
	VERIFY(pController != nullptr);

	wchar_t* szName = nullptr;
	sdaiGetAttrBN(iIFCProjectInstance, "Name", sdaiUNICODE, &szName);

	wchar_t* szDescription = nullptr;
	sdaiGetAttrBN(iIFCProjectInstance, "Description", sdaiUNICODE, &szDescription);

	auto& mapInstances = pModel->GetInstances();

	auto itInstance = mapInstances.find(iIFCProjectInstance);
	if (itInstance != mapInstances.end())
	{
		wstring strItem = itInstance->second->getEntityName();
		if ((szName != nullptr) && (wcslen(szName) > 0))
		{
			strItem += L" '";
			strItem += szName;
			strItem += L"'";
		}

		if ((szDescription != nullptr) && (wcslen(szDescription) > 0))
		{
			strItem += L" (";
			strItem += szDescription;
			strItem += L")";
		}

		/*
		* Project
		*/
		TV_INSERTSTRUCT tvInsertStruct;
		tvInsertStruct.hParent = hModel;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_SELECTED;
		tvInsertStruct.item.lParam = NULL;

		HTREEITEM hProject = (*m_pTreeView).InsertItem(&tvInsertStruct);

		/*
		* Geometry
		*/
		tvInsertStruct.hParent = hProject;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = ITEM_GEOMETRY;
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_NO_GEOMETRY;
		tvInsertStruct.item.lParam = NULL;

		(*m_pTreeView).InsertItem(&tvInsertStruct);

		/*
		* Properties
		*/
		tvInsertStruct.hParent = hProject;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = ITEM_PROPERTIES;
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY_SET;
		tvInsertStruct.item.lParam = NULL;

		(*m_pTreeView).InsertItem(&tvInsertStruct);

		/*
		* decomposition/contains
		*/
		LoadIsDecomposedBy(pModel, iIFCProjectInstance, hProject);
		LoadContainsElements(pModel, iIFCProjectInstance, hProject);

		(*m_pTreeView).Expand(hProject, TVE_EXPAND);
	} // if (itInstance != ...
	else
	{
		ASSERT(false);
	}
}

// ------------------------------------------------------------------------------------------------
void CIFCDecompContTreeView::LoadIsDecomposedBy(CIFCModel* pModel, int64_t iInstance, HTREEITEM hParent)
{
	ASSERT(pModel != nullptr);

	int64_t* piIsDecomposedByInstances = nullptr;
	sdaiGetAttrBN(iInstance, "IsDecomposedBy", sdaiAGGR, &piIsDecomposedByInstances);

	if (piIsDecomposedByInstances == nullptr)
	{
		return;
	}

	const int64_t iIFCRelAggregatesEntity = sdaiGetEntity(pModel->getModel(), "IFCRELAGGREGATES");

	int64_t iIFCIsDecomposedByInstancesCount = sdaiGetMemberCount(piIsDecomposedByInstances);
	for (int64_t i = 0; i < iIFCIsDecomposedByInstancesCount; ++i)
	{
		int64_t iIFCIsDecomposedByInstance = 0;
		engiGetAggrElement(piIsDecomposedByInstances, i, sdaiINSTANCE, &iIFCIsDecomposedByInstance);

		if (sdaiGetInstanceType(iIFCIsDecomposedByInstance) != iIFCRelAggregatesEntity)
		{
			continue;
		}

		/*
		* decomposition
		*/
		TV_INSERTSTRUCT tvInsertStruct;
		tvInsertStruct.hParent = hParent;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = ITEM_DECOMPOSITION;
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_SELECTED;
		tvInsertStruct.item.lParam = NULL;

		HTREEITEM hDecomposition = (*m_pTreeView).InsertItem(&tvInsertStruct);

		int64_t* piIFCRelatedObjectsInstances = 0;
		sdaiGetAttrBN(iIFCIsDecomposedByInstance, "RelatedObjects", sdaiAGGR, &piIFCRelatedObjectsInstances);

		int64_t iIFCRelatedObjectsInstancesCount = sdaiGetMemberCount(piIFCRelatedObjectsInstances);
		for (int64_t j = 0; j < iIFCRelatedObjectsInstancesCount; ++j)
		{
			int64_t iIFCRelatedObjectsInstance = 0;
			engiGetAggrElement(piIFCRelatedObjectsInstances, j, sdaiINSTANCE, &iIFCRelatedObjectsInstance);

			LoadObject(pModel, iIFCRelatedObjectsInstance, hDecomposition);
		} // for (int_t j = ...
	} // for (int64_t i = ...
}

// ------------------------------------------------------------------------------------------------
void CIFCDecompContTreeView::LoadContainsElements(CIFCModel* pModel, int64_t iInstance, HTREEITEM hParent)
{
	ASSERT(pModel != nullptr);

	int64_t* piContainsElementsInstances = nullptr;
	sdaiGetAttrBN(iInstance, "ContainsElements", sdaiAGGR, &piContainsElementsInstances);

	if (piContainsElementsInstances == nullptr)
	{
		return;
	}

	const int64_t iIFCRelContainedInSpatialStructureEntity = sdaiGetEntity(pModel->getModel(), "IFCRELCONTAINEDINSPATIALSTRUCTURE");

	int64_t iIFCContainsElementsInstancesCount = sdaiGetMemberCount(piContainsElementsInstances);
	for (int64_t i = 0; i < iIFCContainsElementsInstancesCount; ++i)
	{
		int64_t iIFCContainsElementsInstance = 0;
		engiGetAggrElement(piContainsElementsInstances, i, sdaiINSTANCE, &iIFCContainsElementsInstance);

		if (sdaiGetInstanceType(iIFCContainsElementsInstance) != iIFCRelContainedInSpatialStructureEntity)
		{
			continue;
		}

		/*
		* contains
		*/
		TV_INSERTSTRUCT tvInsertStruct;
		tvInsertStruct.hParent = hParent;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = ITEM_CONTAINS;
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_SELECTED;
		tvInsertStruct.item.lParam = NULL;

		HTREEITEM hContains = (*m_pTreeView).InsertItem(&tvInsertStruct);

		int64_t* piIFCRelatedElementsInstances = 0;
		sdaiGetAttrBN(iIFCContainsElementsInstance, "RelatedElements", sdaiAGGR, &piIFCRelatedElementsInstances);

		int64_t iIFCRelatedElementsInstancesCount = sdaiGetMemberCount(piIFCRelatedElementsInstances);
		for (int64_t j = 0; j < iIFCRelatedElementsInstancesCount; ++j)
		{
			int64_t iIFCRelatedElementsInstance = 0;
			engiGetAggrElement(piIFCRelatedElementsInstances, j, sdaiINSTANCE, &iIFCRelatedElementsInstance);

			LoadObject(pModel, iIFCRelatedElementsInstance, hContains);
		} // for (int_t j = ...
	} // for (int64_t i = ...
}

// ------------------------------------------------------------------------------------------------
void CIFCDecompContTreeView::LoadObject(CIFCModel* pModel, int64_t iInstance, HTREEITEM hParent)
{
	ASSERT(pModel != nullptr);

	const map<int64_t, CIFCInstance*>& mapInstances = pModel->GetInstances();

	wchar_t* szName = nullptr;
	sdaiGetAttrBN(iInstance, "Name", sdaiUNICODE, &szName);

	wchar_t* szDescription = nullptr;
	sdaiGetAttrBN(iInstance, "Description", sdaiUNICODE, &szDescription);

	map<int64_t, CIFCInstance*>::const_iterator itInstance = mapInstances.find(iInstance);
	if (itInstance != mapInstances.end())
	{
		itInstance->second->referenced() = true;

		CString strExpressID;
		strExpressID.Format(_T("#%lld"), itInstance->second->expressID());

		wstring strItem = strExpressID;
		strItem += L" ";
		strItem += itInstance->second->getEntityName();
		if ((szName != nullptr) && (wcslen(szName) > 0))
		{
			strItem += L" '";
			strItem += szName;
			strItem += L"'";
		}

		if ((szDescription != nullptr) && (wcslen(szDescription) > 0))
		{
			strItem += L" (";
			strItem += szDescription;
			strItem += L")";
		}

		/*
		* Object
		*/
		TV_INSERTSTRUCT tvInsertStruct;
		tvInsertStruct.hParent = hParent;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = itInstance->second->getEnable() ? IMAGE_SELECTED : IMAGE_NOT_SELECTED;
		tvInsertStruct.item.lParam = NULL;

		HTREEITEM hObject = (*m_pTreeView).InsertItem(&tvInsertStruct);

		/*
		* Geometry
		*/
		tvInsertStruct.hParent = hObject;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = ITEM_GEOMETRY;
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage =
			itInstance->second->hasGeometry() ? (itInstance->second->getEnable() ? IMAGE_SELECTED : IMAGE_NOT_SELECTED) : IMAGE_NO_GEOMETRY;
		tvInsertStruct.item.lParam = itInstance->second->hasGeometry() ? (LPARAM)itInstance->second : NULL;

		HTREEITEM hGeometry = (*m_pTreeView).InsertItem(&tvInsertStruct);
		if (itInstance->second->hasGeometry())
		{
			m_mapInstance2Item[itInstance->second] = hGeometry;
		}

		/*
		* Properties
		*/
		bool bHasProperties = HasProperties(pModel, itInstance->first);

		tvInsertStruct.hParent = hObject;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
		tvInsertStruct.item.pszText = ITEM_PROPERTIES;
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY_SET;
		tvInsertStruct.item.cChildren = bHasProperties ? 1 : 0;
		tvInsertStruct.item.lParam = NULL;

		HTREEITEM hProperties = (*m_pTreeView).InsertItem(&tvInsertStruct);

		/*
		* Add a fake item
		*/
		if (bHasProperties)
		{
			tvInsertStruct.hParent = hProperties;
			tvInsertStruct.hInsertAfter = TVI_LAST;
			tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
			tvInsertStruct.item.pszText = ITEM_PROPERTIES_PENDING;
			tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY_SET;
			tvInsertStruct.item.lParam = NULL;

			(*m_pTreeView).InsertItem(&tvInsertStruct);
		} // if (bHasProperties)

		/*
		* decomposition/contains
		*/
		LoadIsDecomposedBy(pModel, iInstance, hObject);
		LoadContainsElements(pModel, iInstance, hObject);
	} // if (itInstance != ...
	else
	{
		ASSERT(false);
	}
}

// ------------------------------------------------------------------------------------------------
bool CIFCDecompContTreeView::HasProperties(CIFCModel* pModel, int64_t iInstance)
{
	ASSERT(pModel != nullptr);

	int64_t* piIFCIsDefinedByInstances = 0;
	sdaiGetAttrBN(iInstance, "IsDefinedBy", sdaiAGGR, &piIFCIsDefinedByInstances);

	if (piIFCIsDefinedByInstances == nullptr)
	{
		return false;
	}

	const int64_t iIFCRelDefinesByTypeEntity = sdaiGetEntity(pModel->getModel(), "IFCRELDEFINESBYTYPE");
	const int64_t iIFCRelDefinesByPropertiesEntity = sdaiGetEntity(pModel->getModel(), "IFCRELDEFINESBYPROPERTIES");

	int64_t iIFCIsDefinedByInstancesCount = sdaiGetMemberCount(piIFCIsDefinedByInstances);

	for (int64_t i = 0; i < iIFCIsDefinedByInstancesCount; ++i)
	{
		int64_t	iIFCIsDefinedByInstance = 0;
		engiGetAggrElement(piIFCIsDefinedByInstances, i, sdaiINSTANCE, &iIFCIsDefinedByInstance);

		if ((sdaiGetInstanceType(iIFCIsDefinedByInstance) == iIFCRelDefinesByPropertiesEntity) ||
			(sdaiGetInstanceType(iIFCIsDefinedByInstance) == iIFCRelDefinesByTypeEntity))
		{
			return true;
		}
	} // for (int64_t i = ...

	return	false;
}

// ------------------------------------------------------------------------------------------------
void CIFCDecompContTreeView::LoadProperties(CIFCModel* pModel, int64_t iInstance, HTREEITEM hParent)
{
	ASSERT(pModel != nullptr);

	int64_t* piIFCIsDefinedByInstances = 0;
	sdaiGetAttrBN(iInstance, "IsDefinedBy", sdaiAGGR, &piIFCIsDefinedByInstances);

	if (piIFCIsDefinedByInstances == nullptr)
	{
		return;
	}

	const int64_t iIFCRelDefinesByTypeEntity = sdaiGetEntity(pModel->getModel(), "IFCRELDEFINESBYTYPE");
	const int64_t iIFCRelDefinesByPropertiesEntity = sdaiGetEntity(pModel->getModel(), "IFCRELDEFINESBYPROPERTIES");

	int64_t iIFCIsDefinedByInstancesCount = sdaiGetMemberCount(piIFCIsDefinedByInstances);
	for (int64_t i = 0; i < iIFCIsDefinedByInstancesCount; ++i)
	{
		int64_t	iIFCIsDefinedByInstance = 0;
		engiGetAggrElement(piIFCIsDefinedByInstances, i, sdaiINSTANCE, &iIFCIsDefinedByInstance);

		if (sdaiGetInstanceType(iIFCIsDefinedByInstance) == iIFCRelDefinesByPropertiesEntity)
		{
			LoadRelDefinesByProperties(pModel, iIFCIsDefinedByInstance, hParent);
		}
		else
		{
			if (sdaiGetInstanceType(iIFCIsDefinedByInstance) == iIFCRelDefinesByTypeEntity)
			{
				LoadRelDefinesByType(pModel, iIFCIsDefinedByInstance, hParent);
			}
		}
	} // for (int64_t i = ...
}

// ------------------------------------------------------------------------------------------------
void CIFCDecompContTreeView::LoadRelDefinesByProperties(CIFCModel* pModel, int64_t iIFCIsDefinedByInstance, HTREEITEM hParent)
{
	ASSERT(pModel != nullptr);
	ASSERT(iIFCIsDefinedByInstance != 0);

	const int64_t iIFCElementQuantityEntity = sdaiGetEntity(pModel->getModel(), "IFCELEMENTQUANTITY");
	const int64_t iIFCPropertySetEntity = sdaiGetEntity(pModel->getModel(), "IFCPROPERTYSET");

	int64_t iIFCPropertyDefinitionInstance = 0;
	sdaiGetAttrBN(iIFCIsDefinedByInstance, "RelatingPropertyDefinition", sdaiINSTANCE, &iIFCPropertyDefinitionInstance);

	if (sdaiGetInstanceType(iIFCPropertyDefinitionInstance) == iIFCElementQuantityEntity)
	{
		LoadQuantites(pModel, iIFCPropertyDefinitionInstance, hParent);
	}
	else
	{
		if (sdaiGetInstanceType(iIFCPropertyDefinitionInstance) == iIFCPropertySetEntity)
		{
			LoadPropertySet(pModel, iIFCPropertyDefinitionInstance, hParent);
		}
	}
}

// ------------------------------------------------------------------------------------------------
void CIFCDecompContTreeView::LoadRelDefinesByType(CIFCModel* pModel, int64_t iIFCIsDefinedByInstance, HTREEITEM hParent)
{
	ASSERT(pModel != nullptr);
	ASSERT(iIFCIsDefinedByInstance != 0);

	int64_t iIFCRelatingType = 0;
	sdaiGetAttrBN(iIFCIsDefinedByInstance, "RelatingType", sdaiINSTANCE, &iIFCRelatingType);

	if (iIFCRelatingType == 0)
	{
		return;
	}

	const int64_t iIFCElementQuantityEntity = sdaiGetEntity(pModel->getModel(), "IFCELEMENTQUANTITY");
	const int64_t iIFCPropertySetEntity = sdaiGetEntity(pModel->getModel(), "IFCPROPERTYSET");

	int64_t* piIFCHasPropertySets = nullptr;
	sdaiGetAttrBN(iIFCRelatingType, "HasPropertySets", sdaiAGGR, &piIFCHasPropertySets);

	int64_t iIFCHasPropertySetsCount = sdaiGetMemberCount(piIFCHasPropertySets);
	for (int64_t i = 0; i < iIFCHasPropertySetsCount; ++i)
	{
		int64_t iIFCHasPropertySetInstance = 0;
		engiGetAggrElement(piIFCHasPropertySets, i, sdaiINSTANCE, &iIFCHasPropertySetInstance);
		if (sdaiGetInstanceType(iIFCHasPropertySetInstance) == iIFCElementQuantityEntity)
		{
			LoadQuantites(pModel, iIFCHasPropertySetInstance, hParent);
		}
		else
			if (sdaiGetInstanceType(iIFCHasPropertySetInstance) == iIFCPropertySetEntity)
			{
				LoadPropertySet(pModel, iIFCHasPropertySetInstance, hParent);
			}
			else
			{
				ASSERT(false);
			}
	} // for (int64_t i = ...
}

// ------------------------------------------------------------------------------------------------
void CIFCDecompContTreeView::LoadPropertySet(CIFCModel* pModel, int64_t iIFCPropertySetInstance, HTREEITEM hParent)
{
	ASSERT(pModel != nullptr);
	ASSERT(iIFCPropertySetInstance != 0);

	wchar_t* szName = nullptr;
	sdaiGetAttrBN(iIFCPropertySetInstance, "Name", sdaiUNICODE, &szName);

	wchar_t* szDescription = nullptr;
	sdaiGetAttrBN(iIFCPropertySetInstance, "Description", sdaiUNICODE, &szDescription);

	wstring strItem;
	if ((szName != nullptr) && (wcslen(szName) > 0))
	{
		strItem = szName;
	}
	else
	{
		strItem = L"<empty>";
	}

	if ((szDescription != nullptr) && (wcslen(szDescription) > 0))
	{
		strItem += L" (";
		strItem += szDescription;
		strItem += L")";
	}

	/*
	* Property set
	*/
	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = hParent;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY_SET;
	tvInsertStruct.item.lParam = NULL;

	HTREEITEM hPropertySet = (*m_pTreeView).InsertItem(&tvInsertStruct);

	int64_t* piIFCHasPropertiesInstances = nullptr;
	sdaiGetAttrBN(iIFCPropertySetInstance, "HasProperties", sdaiAGGR, &piIFCHasPropertiesInstances);

	if (piIFCHasPropertiesInstances == nullptr)
	{
		return;
	}

	const int64_t iIFCPropertySingleValueEntity = sdaiGetEntity(pModel->getModel(), "IFCPROPERTYSINGLEVALUE");

	int64_t iIFCHasPropertiesInstancesCount = sdaiGetMemberCount(piIFCHasPropertiesInstances);
	for (int64_t i = 0; i < iIFCHasPropertiesInstancesCount; ++i)
	{
		int64_t iIFCHasPropertiesInstance = 0;
		engiGetAggrElement(piIFCHasPropertiesInstances, i, sdaiINSTANCE, &iIFCHasPropertiesInstance);

		szName = nullptr;
		sdaiGetAttrBN(iIFCHasPropertiesInstance, "Name", sdaiUNICODE, &szName);

		szDescription = nullptr;
		sdaiGetAttrBN(iIFCHasPropertiesInstance, "Description", sdaiUNICODE, &szDescription);

		strItem = L"";
		if ((szName != nullptr) && (wcslen(szName) > 0))
		{
			strItem = szName;
		}
		else
		{
			strItem = L"<empty>";
		}

		if ((szDescription != nullptr) && (wcslen(szDescription) > 0))
		{
			strItem += L" ('";
			strItem += szDescription;
			strItem += L"')";
		}

		if (sdaiGetInstanceType(iIFCHasPropertiesInstance) == iIFCPropertySingleValueEntity)
		{
			wstring strValue = CIFCUnit::GetPropertyValue(iIFCHasPropertiesInstance);
			if (strValue.empty())
			{
				strValue = L"<empty>";
			}

			strItem += L" = ";
			strItem += strValue;
		}

		tvInsertStruct.hParent = hPropertySet;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
		tvInsertStruct.item.lParam = NULL;

		(*m_pTreeView).InsertItem(&tvInsertStruct);
	} // for  (int64_t i = ...
}

// ------------------------------------------------------------------------------------------------
void CIFCDecompContTreeView::LoadQuantites(CIFCModel* pModel, int64_t iIFCPropertySetInstance, HTREEITEM hParent)
{
	ASSERT(pModel != nullptr);
	ASSERT(iIFCPropertySetInstance != 0);

	wchar_t* szName = nullptr;
	sdaiGetAttrBN(iIFCPropertySetInstance, "Name", sdaiUNICODE, &szName);

	wchar_t* szDescription = nullptr;
	sdaiGetAttrBN(iIFCPropertySetInstance, "Description", sdaiUNICODE, &szDescription);

	wstring strItem;
	if ((szName != nullptr) && (wcslen(szName) > 0))
	{
		strItem = szName;
	}
	else
	{
		strItem = L"<empty>";
	}

	if ((szDescription != nullptr) && (wcslen(szDescription) > 0))
	{
		strItem += L" (";
		strItem += szDescription;
		strItem += L")";
	}

	/*
	* Property set
	*/
	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = hParent;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY_SET;
	tvInsertStruct.item.lParam = NULL;

	HTREEITEM hPropertySet = (*m_pTreeView).InsertItem(&tvInsertStruct);

	int64_t iIFCQuantityLength_TYPE = sdaiGetEntity(pModel->getModel(), "IFCQUANTITYLENGTH");
	int64_t iIFCQuantityArea_TYPE = sdaiGetEntity(pModel->getModel(), "IFCQUANTITYAREA");
	int64_t iIFCQuantityVolume_TYPE = sdaiGetEntity(pModel->getModel(), "IFCQUANTITYVOLUME");
	int64_t iIFCQuantityCount_TYPE = sdaiGetEntity(pModel->getModel(), "IFCQUANTITYCOUNT");
	int64_t iIFCQuantityWeigth_TYPE = sdaiGetEntity(pModel->getModel(), "IFCQUANTITYWEIGHT");
	int64_t iIFCQuantityTime_TYPE = sdaiGetEntity(pModel->getModel(), "IFCQUANTITYTIME");

	int64_t* piIFCQuantities = nullptr;
	sdaiGetAttrBN(iIFCPropertySetInstance, "Quantities", sdaiAGGR, &piIFCQuantities);

	int64_t iIFCQuantitiesCount = sdaiGetMemberCount(piIFCQuantities);
	for (int64_t i = 0; i < iIFCQuantitiesCount; ++i)
	{
		int64_t iIFCQuantityInstance = 0;
		engiGetAggrElement(piIFCQuantities, i, sdaiINSTANCE, &iIFCQuantityInstance);

		if (sdaiGetInstanceType(iIFCQuantityInstance) == iIFCQuantityLength_TYPE)
		{
			LoadIFCQuantityLength(pModel, iIFCQuantityInstance, hPropertySet);
		}
		else
			if (sdaiGetInstanceType(iIFCQuantityInstance) == iIFCQuantityArea_TYPE)
			{
				LoadIFCQuantityArea(pModel, iIFCQuantityInstance, hPropertySet);
			}
			else
				if (sdaiGetInstanceType(iIFCQuantityInstance) == iIFCQuantityVolume_TYPE)
				{
					LoadIFCQuantityVolume(pModel, iIFCQuantityInstance, hPropertySet);
				}
				else
					if (sdaiGetInstanceType(iIFCQuantityInstance) == iIFCQuantityCount_TYPE)
					{
						LoadIFCQuantityCount(pModel, iIFCQuantityInstance, hPropertySet);
					}
					else
						if (sdaiGetInstanceType(iIFCQuantityInstance) == iIFCQuantityWeigth_TYPE)
						{
							LoadIFCQuantityWeight(pModel, iIFCQuantityInstance, hPropertySet);
						}
						else
							if (sdaiGetInstanceType(iIFCQuantityInstance) == iIFCQuantityTime_TYPE)
							{
								LoadIFCQuantityTime(pModel, iIFCQuantityInstance, hPropertySet);
							}
							else
							{
								ASSERT(FALSE); // TODO
							}
	} // for (int64_t i = ...
}

// ------------------------------------------------------------------------------------------------
void CIFCDecompContTreeView::LoadIFCQuantityLength(CIFCModel* pModel, int_t iIFCQuantity, HTREEITEM hParent)
{
	ASSERT(pModel != nullptr);

	wstring strQuantity;
	pModel->LoadIFCQuantityLength(iIFCQuantity, strQuantity);

	/*
	* Quantity
	*/
	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = hParent;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = (LPWSTR)strQuantity.c_str();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
	tvInsertStruct.item.lParam = NULL;

	(*m_pTreeView).InsertItem(&tvInsertStruct);
}

// ------------------------------------------------------------------------------------------------
void CIFCDecompContTreeView::LoadIFCQuantityArea(CIFCModel* pModel, int_t iIFCQuantity, HTREEITEM hParent)
{
	ASSERT(pModel != nullptr);

	wstring strQuantity;
	pModel->LoadIFCQuantityArea(iIFCQuantity, strQuantity);

	/*
	* Quantity
	*/
	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = hParent;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = (LPWSTR)strQuantity.c_str();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
	tvInsertStruct.item.lParam = NULL;

	(*m_pTreeView).InsertItem(&tvInsertStruct);
}

// ------------------------------------------------------------------------------------------------
void CIFCDecompContTreeView::LoadIFCQuantityVolume(CIFCModel* pModel, int_t iIFCQuantity, HTREEITEM hParent)
{
	ASSERT(pModel != nullptr);

	wstring strQuantity;
	pModel->LoadIFCQuantityVolume(iIFCQuantity, strQuantity);

	/*
	* Quantity
	*/
	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = hParent;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = (LPWSTR)strQuantity.c_str();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
	tvInsertStruct.item.lParam = NULL;

	(*m_pTreeView).InsertItem(&tvInsertStruct);
}

// ------------------------------------------------------------------------------------------------
void CIFCDecompContTreeView::LoadIFCQuantityCount(CIFCModel* pModel, int_t iIFCQuantity, HTREEITEM hParent)
{
	ASSERT(pModel != nullptr);

	wstring strQuantity;
	pModel->LoadIFCQuantityCount(iIFCQuantity, strQuantity);

	/*
	* Quantity
	*/
	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = hParent;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = (LPWSTR)strQuantity.c_str();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
	tvInsertStruct.item.lParam = NULL;

	(*m_pTreeView).InsertItem(&tvInsertStruct);
}

// ------------------------------------------------------------------------------------------------
void CIFCDecompContTreeView::LoadIFCQuantityWeight(CIFCModel* pModel, int_t iIFCQuantity, HTREEITEM hParent)
{
	ASSERT(pModel != nullptr);

	wstring strQuantity;
	pModel->LoadIFCQuantityWeight(iIFCQuantity, strQuantity);

	/*
	* Quantity
	*/
	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = hParent;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = (LPWSTR)strQuantity.c_str();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
	tvInsertStruct.item.lParam = NULL;

	(*m_pTreeView).InsertItem(&tvInsertStruct);
}

// ------------------------------------------------------------------------------------------------
void CIFCDecompContTreeView::LoadIFCQuantityTime(CIFCModel* pModel, int_t iIFCQuantity, HTREEITEM hParent)
{
	ASSERT(pModel != nullptr);

	wstring strQuantity;
	pModel->LoadIFCQuantityTime(iIFCQuantity, strQuantity);

	/*
	* Quantity
	*/
	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = hParent;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = (LPWSTR)strQuantity.c_str();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
	tvInsertStruct.item.lParam = NULL;

	(*m_pTreeView).InsertItem(&tvInsertStruct);
}

// ------------------------------------------------------------------------------------------------
void CIFCDecompContTreeView::LoadUnreferencedItems(CIFCModel* pModel, HTREEITEM hModel)
{
	ASSERT(pModel != nullptr);

	map<wstring, vector<CIFCInstance*>> mapUnreferencedItems;

	auto& mapInstances = pModel->GetInstances();

	auto itInstance = mapInstances.begin();
	for (; itInstance != mapInstances.end(); itInstance++)
	{
		if (!itInstance->second->hasGeometry())
		{
			continue;
		}

		if (!itInstance->second->referenced())
		{
			auto itUnreferencedItems = mapUnreferencedItems.find(itInstance->second->getEntityName());
			if (itUnreferencedItems == mapUnreferencedItems.end())
			{
				vector<CIFCInstance*> veCIFCInstances;
				veCIFCInstances.push_back(itInstance->second);

				mapUnreferencedItems[itInstance->second->getEntityName()] = veCIFCInstances;
			}
			else
			{
				itUnreferencedItems->second.push_back(itInstance->second);
			}
		} // for (; itInstance != ...
	} // for (; itInstance != ...

	if (mapUnreferencedItems.empty())
	{
		return;
	}

	/*
	* Unreferenced
	*/
	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = hModel;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = L"Unreferenced";
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_SELECTED;
	tvInsertStruct.item.lParam = NULL;

	HTREEITEM hUnreferenced = (*m_pTreeView).InsertItem(&tvInsertStruct);

	map<wstring, vector<CIFCInstance*>>::iterator itUnreferencedItems = mapUnreferencedItems.begin();
	for (; itUnreferencedItems != mapUnreferencedItems.end(); itUnreferencedItems++)
	{
		/*
		* Entity
		*/
		tvInsertStruct.hParent = hUnreferenced;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = (LPWSTR)itUnreferencedItems->first.c_str();
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_SELECTED;
		tvInsertStruct.item.lParam = NULL;

		HTREEITEM hEntity = (*m_pTreeView).InsertItem(&tvInsertStruct);

		for (size_t iInstance = 0; iInstance < itUnreferencedItems->second.size(); iInstance++)
		{
			auto pInstance = itUnreferencedItems->second[iInstance];

			wchar_t* szName = nullptr;
			sdaiGetAttrBN(pInstance->getInstance(), "Name", sdaiUNICODE, &szName);

			wchar_t* szDescription = nullptr;
			sdaiGetAttrBN(pInstance->getInstance(), "Description", sdaiUNICODE, &szDescription);

			CString strExpressID;
			strExpressID.Format(_T("#%lld"), pInstance->expressID());

			wstring strItem = strExpressID;
			strItem += L" ";
			strItem += pInstance->getEntityName();
			if ((szName != nullptr) && (wcslen(szName) > 0))
			{
				strItem += L" '";
				strItem += szName;
				strItem += L"'";
			}

			if ((szDescription != nullptr) && (wcslen(szDescription) > 0))
			{
				strItem += L" (";
				strItem += szDescription;
				strItem += L")";
			}

			/*
			* Object
			*/
			tvInsertStruct.hParent = hEntity;
			tvInsertStruct.hInsertAfter = TVI_LAST;
			tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
			tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
			tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_SELECTED;
			tvInsertStruct.item.lParam = NULL;

			HTREEITEM hObject = (*m_pTreeView).InsertItem(&tvInsertStruct);

			/*
			* Geometry
			*/
			tvInsertStruct.hParent = hObject;
			tvInsertStruct.hInsertAfter = TVI_LAST;
			tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
			tvInsertStruct.item.pszText = ITEM_GEOMETRY;
			tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage =
				pInstance->hasGeometry() ? (pInstance->getEnable() ? IMAGE_SELECTED : IMAGE_NOT_SELECTED) : IMAGE_NO_GEOMETRY;
			tvInsertStruct.item.lParam = pInstance->hasGeometry() ? (LPARAM)pInstance : NULL;

			HTREEITEM hGeometry = (*m_pTreeView).InsertItem(&tvInsertStruct);
			if (pInstance->hasGeometry())
			{
				m_mapInstance2Item[pInstance] = hGeometry;
			}

			/*
			* Properties
			*/
			bool bHasProperties = HasProperties(pModel, pInstance->getInstance());

			tvInsertStruct.hParent = hObject;
			tvInsertStruct.hInsertAfter = TVI_LAST;
			tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
			tvInsertStruct.item.pszText = ITEM_PROPERTIES;
			tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY_SET;
			tvInsertStruct.item.cChildren = bHasProperties ? 1 : 0;
			tvInsertStruct.item.lParam = NULL;

			HTREEITEM hProperties = (*m_pTreeView).InsertItem(&tvInsertStruct);

			/*
			* Add a fake item
			*/
			if (bHasProperties)
			{
				tvInsertStruct.hParent = hProperties;
				tvInsertStruct.hInsertAfter = TVI_LAST;
				tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
				tvInsertStruct.item.pszText = ITEM_PROPERTIES_PENDING;
				tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY_SET;
				tvInsertStruct.item.lParam = NULL;

				(*m_pTreeView).InsertItem(&tvInsertStruct);
			} // if (bHasProperties)
		} // for (size_t iInstance = ...
	} // for (; itUnreferencedItems != ...
}

// ------------------------------------------------------------------------------------------------
void CIFCDecompContTreeView::LoadTree_UpdateItems(HTREEITEM hModel)
{
	HTREEITEM hModelChild = (*m_pTreeView).GetNextItem(hModel, TVGN_CHILD);
	while (hModelChild != nullptr)
	{
		int iImage = -1;
		int iSelectedImage = -1;
		(*m_pTreeView).GetItemImage(hModelChild, iImage, iSelectedImage);

		ASSERT(iImage == iSelectedImage);

		if ((iImage != IMAGE_SELECTED) && (iImage != IMAGE_SEMI_SELECTED) && (iImage != IMAGE_NOT_SELECTED))
		{
			// skip the Header and unreferenced items
			hModelChild = (*m_pTreeView).GetNextSiblingItem(hModelChild);

			continue;
		}

		LoadTree_UpdateItem(hModelChild);

		hModelChild = (*m_pTreeView).GetNextSiblingItem(hModelChild);
	}
}

// ----------------------------------------------------------------------------
void CIFCDecompContTreeView::LoadTree_UpdateItem(HTREEITEM hParent)
{
	ASSERT(hParent != nullptr);

	if (!(*m_pTreeView).ItemHasChildren(hParent))
	{
		// keep the state as it is
		return;
	}

	int iChidlrenCount = 0;
	int iSelectedChidlrenCount = 0;
	int iSemiSelectedChidlrenCount = 0;

	HTREEITEM hChild = (*m_pTreeView).GetNextItem(hParent, TVGN_CHILD);
	while (hChild != nullptr)
	{
		int iImage = -1;
		int iSelectedImage = -1;
		(*m_pTreeView).GetItemImage(hChild, iImage, iSelectedImage);

		ASSERT(iImage == iSelectedImage);

		if ((iImage != IMAGE_SELECTED) && (iImage != IMAGE_SEMI_SELECTED) && (iImage != IMAGE_NOT_SELECTED))
		{
			// skip the properties, items without a geometry, etc.
			hChild = (*m_pTreeView).GetNextSiblingItem(hChild);

			continue;
		}

		LoadTree_UpdateItem(hChild);

		iImage = -1;
		iSelectedImage = -1;
		(*m_pTreeView).GetItemImage(hChild, iImage, iSelectedImage);

		ASSERT(iImage == iSelectedImage);

		iChidlrenCount++;

		switch (iImage)
		{
		case IMAGE_SELECTED:
		{
			iSelectedChidlrenCount++;
		}
		break;

		case IMAGE_SEMI_SELECTED:
		{
			iSemiSelectedChidlrenCount++;
		}
		break;

		case IMAGE_NOT_SELECTED:
		{
			// NA
		}
		break;

		default:
		{
			ASSERT(false); // unexpected
		}
		break;
		} // switch (iImage)

		hChild = (*m_pTreeView).GetNextSiblingItem(hChild);
	} // while (hChild != nullptr)

	if (iChidlrenCount == 0)
	{
		// keep the state as it is
		return;
	}

	if (iSemiSelectedChidlrenCount > 0)
	{
		(*m_pTreeView).SetItemImage(hParent, IMAGE_SEMI_SELECTED, IMAGE_SEMI_SELECTED);

		return;
	}

	if (iSelectedChidlrenCount == 0)
	{
		(*m_pTreeView).SetItemImage(hParent, IMAGE_NOT_SELECTED, IMAGE_NOT_SELECTED);

		return;
	}

	if (iSelectedChidlrenCount == iChidlrenCount)
	{
		(*m_pTreeView).SetItemImage(hParent, IMAGE_SELECTED, IMAGE_SELECTED);

		return;
	}

	(*m_pTreeView).SetItemImage(hParent, IMAGE_SEMI_SELECTED, IMAGE_SEMI_SELECTED);
}

// ----------------------------------------------------------------------------
void CIFCDecompContTreeView::ClickItem_UpdateChildren(HTREEITEM hParent)
{
	ASSERT(hParent != nullptr);

	if (!(*m_pTreeView).ItemHasChildren(hParent))
	{
		return;
	}

	int iParentImage = -1;
	int iParentSelectedImage = -1;
	(*m_pTreeView).GetItemImage(hParent, iParentImage, iParentSelectedImage);

	ASSERT(iParentImage == iParentSelectedImage);

	HTREEITEM hChild = (*m_pTreeView).GetNextItem(hParent, TVGN_CHILD);
	while (hChild != nullptr)
	{
		int iImage = -1;
		int iSelectedImage = -1;
		(*m_pTreeView).GetItemImage(hChild, iImage, iSelectedImage);

		ASSERT(iImage == iSelectedImage);

		if ((iImage != IMAGE_SELECTED) && (iImage != IMAGE_SEMI_SELECTED) && (iImage != IMAGE_NOT_SELECTED))
		{
			// skip the properties, items without a geometry, etc.
			hChild = (*m_pTreeView).GetNextSiblingItem(hChild);

			continue;
		}

		(*m_pTreeView).SetItemImage(hChild, iParentImage, iParentImage);

		CIFCInstance* pInstance = (CIFCInstance*)(*m_pTreeView).GetItemData(hChild);
		if (pInstance != nullptr)
		{
			pInstance->setEnable((iParentImage == IMAGE_SELECTED) || (iParentImage == IMAGE_SEMI_SELECTED) ? true : false);
		}

		ClickItem_UpdateChildren(hChild);

		hChild = (*m_pTreeView).GetNextSiblingItem(hChild);
	} // while (hChild != nullptr)
}

// ----------------------------------------------------------------------------
void CIFCDecompContTreeView::ClickItem_UpdateParent(HTREEITEM hParent)
{
	if (hParent == nullptr)
	{
		return;
	}

	ASSERT((*m_pTreeView).ItemHasChildren(hParent));

	int iChidlrenCount = 0;
	int iSelectedChidlrenCount = 0;
	int iSemiSelectedChidlrenCount = 0;

	HTREEITEM hChild = (*m_pTreeView).GetNextItem(hParent, TVGN_CHILD);
	while (hChild != nullptr)
	{
		int iImage = -1;
		int iSelectedImage = -1;
		(*m_pTreeView).GetItemImage(hChild, iImage, iSelectedImage);

		ASSERT(iImage == iSelectedImage);

		if ((iImage != IMAGE_SELECTED) && (iImage != IMAGE_SEMI_SELECTED) && (iImage != IMAGE_NOT_SELECTED))
		{
			// skip the properties, items without a geometry, etc.
			hChild = (*m_pTreeView).GetNextSiblingItem(hChild);

			continue;
		}

		iChidlrenCount++;

		switch (iImage)
		{
			case IMAGE_SELECTED:
			{
				iSelectedChidlrenCount++;
			}
			break;

			case IMAGE_SEMI_SELECTED:
			{
				iSemiSelectedChidlrenCount++;
			}
			break;

			case IMAGE_NOT_SELECTED:
			{
				// NA
			}
			break;

			default:
			{
				ASSERT(false); // unexpected
			}
			break;
		} // switch (iImage)

		hChild = (*m_pTreeView).GetNextSiblingItem(hChild);
	} // while (hChild != nullptr)

	if (iSemiSelectedChidlrenCount > 0)
	{
		(*m_pTreeView).SetItemImage(hParent, IMAGE_SEMI_SELECTED, IMAGE_SEMI_SELECTED);

		CIFCInstance* pInstance = (CIFCInstance*)(*m_pTreeView).GetItemData(hParent);
		if (pInstance != nullptr)
		{
			pInstance->setEnable(true);
		}

		ClickItem_UpdateParent((*m_pTreeView).GetParentItem(hParent));

		return;
	}

	if (iSelectedChidlrenCount == 0)
	{
		(*m_pTreeView).SetItemImage(hParent, IMAGE_NOT_SELECTED, IMAGE_NOT_SELECTED);

		CIFCInstance* pInstance = (CIFCInstance*)(*m_pTreeView).GetItemData(hParent);
		if (pInstance != nullptr)
		{
			pInstance->setEnable(false);
		}

		ClickItem_UpdateParent((*m_pTreeView).GetParentItem(hParent));

		return;
	}

	if (iSelectedChidlrenCount == iChidlrenCount)
	{
		(*m_pTreeView).SetItemImage(hParent, IMAGE_SELECTED, IMAGE_SELECTED);

		CIFCInstance* pInstance = (CIFCInstance*)(*m_pTreeView).GetItemData(hParent);
		if (pInstance != nullptr)
		{
			pInstance->setEnable(true);
		}

		ClickItem_UpdateParent((*m_pTreeView).GetParentItem(hParent));

		return;
	}

	(*m_pTreeView).SetItemImage(hParent, IMAGE_SEMI_SELECTED, IMAGE_SEMI_SELECTED);

	CIFCInstance* pInstance = (CIFCInstance*)(*m_pTreeView).GetItemData(hParent);
	if (pInstance != nullptr)
	{
		pInstance->setEnable(true);
	}

	ClickItem_UpdateParent((*m_pTreeView).GetParentItem(hParent));
}

// ----------------------------------------------------------------------------
void CIFCDecompContTreeView::UnselectAllItems()
{
	auto itSelectedIInstance = m_mapSelectedInstances.begin();
	for (; itSelectedIInstance != m_mapSelectedInstances.end(); itSelectedIInstance++)
	{
		(*m_pTreeView).SetItemState(itSelectedIInstance->second, 0, TVIS_BOLD);
	}

	m_mapSelectedInstances.clear();
}

// ----------------------------------------------------------------------------
void CIFCDecompContTreeView::ResetView()
{
	m_mapModelHTREEITEM.clear();
	m_mapInstance2Item.clear();
	m_mapSelectedInstances.clear();

	(*m_pTreeView).DeleteAllItems();

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

	auto pModel = pController->GetModel()->As<CIFCModel>();
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	LoadModel(pModel);
}