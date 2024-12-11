#include "stdafx.h"
#include "mainfrm.h"
#include "CIS2ModelStructureView.h"
#include "Resource.h"
#include "STEPViewer.h"
#include "IFCModel.h"
#include "StructureViewConsts.h"

#include <algorithm>
#include <chrono>
using namespace std;

// ------------------------------------------------------------------------------------------------
CCIS2ModelStructureView::CCIS2ModelStructureView(CTreeCtrlEx* pTreeView)
	: CTreeViewBase()
	, m_pTreeCtrl(pTreeView)
	, m_pImageList(nullptr)
	, m_mapInstance2GeometryItem()
	, m_mapSelectedInstances()
	, m_pSearchDialog(nullptr)
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

	m_pTreeCtrl->SetImageList(m_pImageList, TVSIL_NORMAL);

	// State provider
	m_pTreeCtrl->SetItemStateProvider(this);

	//  Search
	m_pSearchDialog = new CSearchTreeCtrlDialog(this);
	m_pSearchDialog->Create(IDD_DIALOG_SEARCH, m_pTreeCtrl);
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ CCIS2ModelStructureView::~CCIS2ModelStructureView()
{
	m_pTreeCtrl->SetImageList(nullptr, TVSIL_NORMAL);

	m_pImageList->DeleteImageList();
	delete m_pImageList;	

	m_pTreeCtrl->SetItemStateProvider(nullptr);

	delete m_pSearchDialog;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CCIS2ModelStructureView::OnInstanceSelected(CViewBase* pSender) /*override*/
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
		dynamic_cast<CCIS2Instance*>(GetController()->GetSelectedInstance()) : 
		nullptr;

	if (pSelectedInstance != nullptr)
	{
		auto itIInstance2GeometryItem = m_mapInstance2GeometryItem.find(pSelectedInstance);
	
		if (itIInstance2GeometryItem != m_mapInstance2GeometryItem.end())
		{
			ASSERT(m_mapSelectedInstances.find(pSelectedInstance) == m_mapSelectedInstances.end());
			m_mapSelectedInstances[pSelectedInstance] = itIInstance2GeometryItem->second;

			m_pTreeCtrl->SetItemState(itIInstance2GeometryItem->second, TVIS_BOLD, TVIS_BOLD);
			m_pTreeCtrl->EnsureVisible(itIInstance2GeometryItem->second);
		}
	}
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CCIS2ModelStructureView::Load() /*override*/
{
	ResetView();
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ CImageList* CCIS2ModelStructureView::GetImageList() const /*override*/
{
	return m_pImageList;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CCIS2ModelStructureView::OnShowWindow(BOOL bShow, UINT /*nStatus*/) /*override*/
{
	if (!bShow)
	{
		m_pSearchDialog->ShowWindow(SW_HIDE);
	}
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CCIS2ModelStructureView::OnTreeItemClick(NMHDR* /*pNMHDR*/, LRESULT* pResult) /*override*/
{
	*pResult = 0;

	DWORD dwPosition = GetMessagePos();
	CPoint point(LOWORD(dwPosition), HIWORD(dwPosition));
	m_pTreeCtrl->ScreenToClient(&point);

	UINT uFlags = 0;
	HTREEITEM hItem = m_pTreeCtrl->HitTest(point, &uFlags);

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
		int iImage, iSelectedImage = -1;
		m_pTreeCtrl->GetItemImage(hItem, iImage, iSelectedImage);

		ASSERT(iImage == iSelectedImage);

		CCIS2Instance* pInstance = (CCIS2Instance*)m_pTreeCtrl->GetItemData(hItem);

		switch (iImage)
		{
			case IMAGE_SELECTED:
			case IMAGE_SEMI_SELECTED:
			{
				m_pTreeCtrl->SetItemImage(hItem, IMAGE_NOT_SELECTED, IMAGE_NOT_SELECTED);

				if (pInstance != nullptr)
				{
					pInstance->_instance::setEnable(false);
				}

				ClickItem_UpdateChildren(hItem);
				ClickItem_UpdateParent(m_pTreeCtrl->GetParentItem(hItem));
			}
			break;

			case IMAGE_NOT_SELECTED:
			{
				m_pTreeCtrl->SetItemImage(hItem, IMAGE_SELECTED, IMAGE_SELECTED);

				if (pInstance != nullptr)
				{
					pInstance->_instance::setEnable(true);
				}

				ClickItem_UpdateChildren(hItem);
				ClickItem_UpdateParent(m_pTreeCtrl->GetParentItem(hItem));
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

		auto pSelectedInstance = m_pTreeCtrl->GetItemData(hItem) != NULL ?
			(CCIS2Instance*)m_pTreeCtrl->GetItemData(hItem) :
			nullptr;

		pController->SelectInstance(this, pSelectedInstance);

		if (pSelectedInstance != nullptr)
		{
			auto itIInstance2GeometryItem = m_mapInstance2GeometryItem.find(pSelectedInstance);
			ASSERT(itIInstance2GeometryItem != m_mapInstance2GeometryItem.end());

			ASSERT(m_mapSelectedInstances.find(pSelectedInstance) == m_mapSelectedInstances.end());
			m_mapSelectedInstances[pSelectedInstance] = itIInstance2GeometryItem->second;

			m_pTreeCtrl->SetItemState(itIInstance2GeometryItem->second, TVIS_BOLD, TVIS_BOLD);
			m_pTreeCtrl->EnsureVisible(itIInstance2GeometryItem->second);
		}
	} // if ((hItem != nullptr) && ...
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CCIS2ModelStructureView::OnTreeItemExpanding(NMHDR* /*pNMHDR*/, LRESULT* pResult) /*override*/
{
	*pResult = 0;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ bool CCIS2ModelStructureView::IsSelected(HTREEITEM hItem) /*override*/
{
	auto pController = GetController();
	if (pController == nullptr)
	{
		return false;
	}

	auto pInstance = (CCIS2Instance*)m_pTreeCtrl->GetItemData(hItem);
	if (pInstance == nullptr)
	{
		return false;
	}	

	return pInstance == pController->GetSelectedInstance();
}


// ------------------------------------------------------------------------------------------------
/*virtual*/ CTreeCtrlEx* CCIS2ModelStructureView::GetTreeView() /*override*/
{
	return m_pTreeCtrl;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ vector<CString> CCIS2ModelStructureView::GetSearchFilters() /*override*/
{
	return vector<CString>
		{
			_T("(All)"),
			_T("Express ID"),
		};
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CCIS2ModelStructureView::LoadChildrenIfNeeded(HTREEITEM /*hItem*/) /*override*/
{
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ BOOL CCIS2ModelStructureView::ContainsText(int iFilter, HTREEITEM hItem, const CString& strText) /*override*/
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

	// Express line number
	if (iFilter == (int)enumSearchFilter::ExpressID)
	{
		CString strExpressionLine = L"#";
		strExpressionLine += strText;

		return strItemText.Find(strExpressionLine, 0) == 0;
	}

	// All
	return strItemText.Find(strTextLower, 0) != -1;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CCIS2ModelStructureView::OnContextMenu(CWnd* pWnd, CPoint point) /*override*/
{
	ASSERT_VALID(m_pTreeCtrl);
	if (pWnd != m_pTreeCtrl)
	{
		return;
	}

	if (point == CPoint(-1, -1))
	{
		return;
	}

	auto pController = GetController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pModel = GetModel<CCIS2Model>();
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	// Select clicked item
	CPoint ptTree = point;
	m_pTreeCtrl->ScreenToClient(&ptTree);

	CCIS2Instance* pInstance = nullptr;

	UINT flags = 0;
	HTREEITEM hItem = m_pTreeCtrl->HitTest(ptTree, &flags);
	if (hItem != nullptr)
	{
		m_pTreeCtrl->SelectItem(hItem);
		m_pTreeCtrl->SetFocus();

		pInstance = (CCIS2Instance*)m_pTreeCtrl->GetItemData(hItem);
		if (pInstance == nullptr)
		{
			// Check the first child
			HTREEITEM hChild = NULL;
			if (((hChild = m_pTreeCtrl->GetNextItem(hItem, TVGN_CHILD)) != NULL) &&
				(m_pTreeCtrl->GetItemText(hChild) == ITEM_GEOMETRY) &&
				(m_pTreeCtrl->GetItemData(hChild) != NULL))
			{
				hItem = hChild;
				pInstance = (CCIS2Instance*)m_pTreeCtrl->GetItemData(hItem);
			}
		} // if (pInstance == nullptr)
	} // if (hItem != nullptr)	

	auto& mapInstances = pModel->GetInstances();	

	// ENTITY : VISIBLE COUNT
	map<wstring, long> mapEntity2VisibleCount;
	for (auto itInstance = mapInstances.begin(); 
		itInstance != mapInstances.end(); 
		itInstance++)
	{
		if (!itInstance->second->_instance::hasGeometry())
		{
			continue;
		}

		auto itEntity2VisibleCount = mapEntity2VisibleCount.find(itInstance->second->GetEntityName());
		if (itEntity2VisibleCount == mapEntity2VisibleCount.end())
		{
			mapEntity2VisibleCount[itInstance->second->GetEntityName()] = itInstance->second->_instance::getEnable() ? 1 : 0;
		}
		else
		{
			itEntity2VisibleCount->second += itInstance->second->_instance::getEnable() ? 1 : 0;
		}
	} // for (; itInstance != ...

	ASSERT(!mapEntity2VisibleCount.empty());

	// Build menu
	CMenu menuMain;
	CMenu* pMenu = nullptr;

	if (pInstance != nullptr)
	{
		if (pInstance->_instance::hasGeometry())
		{
			VERIFY(menuMain.LoadMenuW(IDR_POPUP_INSTANCES));
			pMenu = menuMain.GetSubMenu(0);

			// Zoom to
			if (!pInstance->_instance::getEnable())
			{
				pMenu->EnableMenuItem(ID_INSTANCES_ZOOM_TO, MF_BYCOMMAND | MF_DISABLED);
			}

			// Save
			if (!pInstance->_instance::getEnable())
			{
				pMenu->EnableMenuItem(ID_INSTANCES_SAVE, MF_BYCOMMAND | MF_DISABLED);
			}

			// Enable
			if (pInstance->_instance::getEnable())
			{
				pMenu->CheckMenuItem(ID_INSTANCES_ENABLE, MF_BYCOMMAND | MF_CHECKED);
			}
		} // if (pInstance->HasGeometry())
		else 
		{
			VERIFY(menuMain.LoadMenuW(IDR_POPUP_INSTANCES_NO_GEOMETRY));
			pMenu = menuMain.GetSubMenu(0);
		} // else if (pInstance->HasGeometry())
	} // if (pInstance != nullptr)
	else
	{
		VERIFY(menuMain.LoadMenuW(IDR_POPUP_META_DATA));
		pMenu = menuMain.GetSubMenu(0);
	} // else if (pInstance != nullptr)

	// Entities
	CMenu menuEntities;
	VERIFY(menuEntities.CreatePopupMenu());

	UINT uiID = 1;
	map<UINT, wstring> mapCommand2Entity;
	for (auto itEntity2VisibleCount = mapEntity2VisibleCount.begin();
		itEntity2VisibleCount != mapEntity2VisibleCount.end();
		itEntity2VisibleCount++)
	{
		mapCommand2Entity[uiID] = itEntity2VisibleCount->first;

		menuEntities.AppendMenu(
			MF_STRING | (itEntity2VisibleCount->second > 0 ? MF_CHECKED : MF_UNCHECKED),
			uiID++,
			itEntity2VisibleCount->first.c_str());
	}

	if (pMenu != nullptr)
	{
		pMenu->AppendMenu(MF_SEPARATOR, 0, L"");
		pMenu->AppendMenu(MF_STRING | MF_POPUP, (UINT_PTR)menuEntities.GetSafeHmenu(), L"Entities");
	}
	else
	{
		pMenu = &menuEntities;
	}

	// Show
	UINT uiCommand = pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, point.x, point.y, m_pTreeCtrl);
	if (uiCommand == 0)
	{
		return;
	}

	// Execute the command
	bool bExecuted = true;
	if (pInstance != nullptr)
	{
		if (pInstance->_instance::hasGeometry())
		{
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
					pInstance->_instance::setEnable(!pInstance->_instance::getEnable());

					int iImage = pInstance->_instance::getEnable() ? IMAGE_SELECTED : IMAGE_NOT_SELECTED;
					m_pTreeCtrl->SetItemImage(hItem, iImage, iImage);

					ClickItem_UpdateChildren(hItem);
					ClickItem_UpdateParent(m_pTreeCtrl->GetParentItem(hItem));

					pController->OnInstancesEnabledStateChanged(this);
				}
				break;

				case ID_INSTANCES_DISABLE_ALL_BUT_THIS:
				{
					for (auto itInstance = mapInstances.begin(); 
						itInstance != mapInstances.end(); 
						itInstance++)
					{
						itInstance->second->_instance::setEnable(itInstance->second == pInstance);
					}

					ResetView();
					OnInstanceSelected(nullptr);

					pController->OnInstancesEnabledStateChanged(this);
				}
				break;

				case ID_INSTANCES_ENABLE_ALL:
				{
					for (auto itInstance = mapInstances.begin(); 
						itInstance != mapInstances.end(); 
						itInstance++)
					{
						itInstance->second->_instance::setEnable(true);
					}

					ResetView();
					OnInstanceSelected(nullptr);

					pController->OnInstancesEnabledStateChanged(this);
				}
				break;

				case IDS_VIEW_IFC_RELATIONS:
				{
					pController->OnViewRelations(this, pInstance->_ap_geometry::getSdaiInstance());
				}
				break;

				default:
				{
					bExecuted = false;
				}
				break;
			} // switch (uiCommand)
		} // if (pInstance->HasGeometry())
		else
		{
			switch (uiCommand)
			{
				case ID_VIEW_ZOOM_OUT:
				{
					pController->ZoomOut();
				}
				break;

				case IDS_VIEW_IFC_RELATIONS:
				{
					pController->OnViewRelations(this, pInstance->_ap_geometry::getSdaiInstance());
				}
				break;

				default:
				{
					bExecuted = false;
				}
				break;
			} // switch (uiCommand) 
		} // else if (pInstance->HasGeometry())
	} // if (pInstance != nullptr)
	else
	{
		switch (uiCommand)
		{
			case ID_VIEW_ZOOM_OUT:
			{
				pController->ZoomOut();
			}
			break;

			default:
			{
				bExecuted = false;
			}
			break;
		}
	}  // else if (pInstance != nullptr)

	// Enable Entity command
	if (!bExecuted)
	{
		auto itCommand2Entity = mapCommand2Entity.find(uiCommand);
		if (itCommand2Entity == mapCommand2Entity.end())
		{
			ASSERT(FALSE); // Internal error!

			return;
		}

		auto itEntity2VisibleCount = mapEntity2VisibleCount.find(itCommand2Entity->second);
		if (itEntity2VisibleCount == mapEntity2VisibleCount.end())
		{
			ASSERT(FALSE); // Internal error!

			return;
		}

		// Update the Parents just ones
		set<HTREEITEM> m_setParents;

		for (auto itInstance = mapInstances.begin();
			itInstance != mapInstances.end();
			itInstance++)
		{
			pInstance = itInstance->second;

			if (pInstance->GetEntityName() == itCommand2Entity->second)
			{
				pInstance->_instance::setEnable(itEntity2VisibleCount->second > 0 ? false : true);

				auto itInstance2GeometryItem = m_mapInstance2GeometryItem.find(pInstance);
				ASSERT(itInstance2GeometryItem != m_mapInstance2GeometryItem.end());

				HTREEITEM hGeometryItem = itInstance2GeometryItem->second;

				int iImage = pInstance->_instance::getEnable() ? IMAGE_SELECTED : IMAGE_NOT_SELECTED;
				m_pTreeCtrl->SetItemImage(hGeometryItem, iImage, iImage);

				ASSERT(!m_pTreeCtrl->ItemHasChildren(hGeometryItem));

				HTREEITEM hInstanceItem = m_pTreeCtrl->GetParentItem(hGeometryItem);
				ASSERT(hInstanceItem != NULL);

				ClickItem_UpdateParent(hInstanceItem, FALSE);

				m_setParents.insert(m_pTreeCtrl->GetParentItem(hInstanceItem));
			}
		} // for (auto itInstance = ...

		// Update the Parents
		for (auto hParent : m_setParents)
		{
			ClickItem_UpdateParent(hParent);
		}

		pController->OnInstancesEnabledStateChanged(this);
	} // if (!bProcessed)
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CCIS2ModelStructureView::OnSearch() /*override*/
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

// ------------------------------------------------------------------------------------------------
void CCIS2ModelStructureView::LoadModel(CCIS2Model* pModel)
{
	/**********************************************************************************************
	* Model
	*/
	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = nullptr;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = (LPWSTR)pModel->getPath();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_SELECTED;
	tvInsertStruct.item.lParam = NULL;

	HTREEITEM hModel = m_pTreeCtrl->InsertItem(&tvInsertStruct);
	//*********************************************************************************************

	/*
	* Header
	*/
	LoadHeader(pModel, hModel);

	//
	// Design Parts & Representations
	// 

	auto& mapInstances = pModel->GetInstances();
	for (auto& itInstance : mapInstances)
	{
		LoadInstance(pModel, itInstance.first, hModel);
	}

	//LoadTree_UpdateItems(hModel); //#todo?

	m_pTreeCtrl->Expand(hModel, TVE_EXPAND);
}

// ------------------------------------------------------------------------------------------------
void CCIS2ModelStructureView::LoadHeader(CCIS2Model* pModel, HTREEITEM hModel)
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

	HTREEITEM hHeader = m_pTreeCtrl->InsertItem(&tvInsertStruct);
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

		HTREEITEM hDescriptions = m_pTreeCtrl->InsertItem(&tvInsertStruct);

		int_t iItem = 0;
		if (!GetSPFFHeaderItem(pModel->getSdaiInstance(), 0, iItem, sdaiUNICODE, (char**)&szText))
		{
			while (!GetSPFFHeaderItem(pModel->getSdaiInstance(), 0, iItem++, sdaiUNICODE, (char**)&szText))
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

				m_pTreeCtrl->InsertItem(&tvInsertStruct);
			}
		} // if (!GetSPFFHeaderItem(...
	}

	/*
	* ImplementationLevel
	*/
	{
		GetSPFFHeaderItem(pModel->getSdaiInstance(), 1, 0, sdaiUNICODE, (char**)&szText);

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

		m_pTreeCtrl->InsertItem(&tvInsertStruct);
	}

	/*
	* Name
	*/
	{
		GetSPFFHeaderItem(pModel->getSdaiInstance(), 2, 0, sdaiUNICODE, (char**)&szText);

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

		m_pTreeCtrl->InsertItem(&tvInsertStruct);
	}

	/*
	* TimeStamp
	*/
	{
		GetSPFFHeaderItem(pModel->getSdaiInstance(), 3, 0, sdaiUNICODE, (char**)&szText);

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

		m_pTreeCtrl->InsertItem(&tvInsertStruct);
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

		HTREEITEM hDescriptions = m_pTreeCtrl->InsertItem(&tvInsertStruct);

		int_t iItem = 0;
		if (!GetSPFFHeaderItem(pModel->getSdaiInstance(), 4, iItem, sdaiUNICODE, (char**)&szText))
		{
			while (!GetSPFFHeaderItem(pModel->getSdaiInstance(), 4, iItem++, sdaiUNICODE, (char**)&szText))
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

				m_pTreeCtrl->InsertItem(&tvInsertStruct);
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

		HTREEITEM hDescriptions = m_pTreeCtrl->InsertItem(&tvInsertStruct);

		int_t iItem = 0;
		if (!GetSPFFHeaderItem(pModel->getSdaiInstance(), 5, iItem, sdaiUNICODE, (char**)&szText))
		{
			while (!GetSPFFHeaderItem(pModel->getSdaiInstance(), 5, iItem++, sdaiUNICODE, (char**)&szText))
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

				m_pTreeCtrl->InsertItem(&tvInsertStruct);
			}
		} // if (!GetSPFFHeaderItem(...
	}

	/*
	* PreprocessorVersion
	*/
	{
		GetSPFFHeaderItem(pModel->getSdaiInstance(), 6, 0, sdaiUNICODE, (char**)&szText);

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

		m_pTreeCtrl->InsertItem(&tvInsertStruct);
	}

	/*
	* OriginatingSystem
	*/
	{
		GetSPFFHeaderItem(pModel->getSdaiInstance(), 7, 0, sdaiUNICODE, (char**)&szText);

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

		m_pTreeCtrl->InsertItem(&tvInsertStruct);
	}

	/*
	* Authorization
	*/
	{
		GetSPFFHeaderItem(pModel->getSdaiInstance(), 8, 0, sdaiUNICODE, (char**)&szText);

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

		m_pTreeCtrl->InsertItem(&tvInsertStruct);
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

		HTREEITEM hDescriptions = m_pTreeCtrl->InsertItem(&tvInsertStruct);

		int_t iItem = 0;
		if (!GetSPFFHeaderItem(pModel->getSdaiInstance(), 9, iItem, sdaiUNICODE, (char**)&szText))
		{
			while (!GetSPFFHeaderItem(pModel->getSdaiInstance(), 9, iItem++, sdaiUNICODE, (char**)&szText))
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

				m_pTreeCtrl->InsertItem(&tvInsertStruct);
			}
		} // if (!GetSPFFHeaderItem(...
	}
}

// ------------------------------------------------------------------------------------------------
void CCIS2ModelStructureView::LoadProject(CCIS2Model* pModel, HTREEITEM hModel, SdaiInstance iIFCProjectInstance)
{
	auto pController = GetController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	wchar_t* szName = nullptr;
	sdaiGetAttrBN(iIFCProjectInstance, "Name", sdaiUNICODE, &szName);

	wchar_t* szDescription = nullptr;
	sdaiGetAttrBN(iIFCProjectInstance, "Description", sdaiUNICODE, &szDescription);

	auto& mapInstances = pModel->GetInstances();

	auto itInstance = mapInstances.find(iIFCProjectInstance);
	if (itInstance != mapInstances.end())
	{
		wstring strItem = itInstance->second->GetEntityName();
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

		HTREEITEM hProject = m_pTreeCtrl->InsertItem(&tvInsertStruct);

		/*
		* Geometry
		*/
		tvInsertStruct.hParent = hProject;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = ITEM_GEOMETRY;
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_NO_GEOMETRY;
		tvInsertStruct.item.lParam = (LPARAM)itInstance->second;

		HTREEITEM hGeometry = m_pTreeCtrl->InsertItem(&tvInsertStruct);
		m_mapInstance2GeometryItem[itInstance->second] = hGeometry;

		/*
		* decomposition/contains
		*/
		LoadIsDecomposedBy(pModel, iIFCProjectInstance, hProject);
		LoadIsNestedBy(pModel, iIFCProjectInstance, hProject);
		LoadContainsElements(pModel, iIFCProjectInstance, hProject);

		m_pTreeCtrl->Expand(hProject, TVE_EXPAND);
	} // if (itInstance != ...
}

void CCIS2ModelStructureView::LoadIsDecomposedBy(CCIS2Model* pModel, SdaiInstance iInstance, HTREEITEM hParent)
{
	ASSERT(pModel != nullptr);

	SdaiAggr piIsDecomposedByInstances = nullptr;
	sdaiGetAttrBN(iInstance, "IsDecomposedBy", sdaiAGGR, &piIsDecomposedByInstances);

	if (piIsDecomposedByInstances == nullptr)
	{
		return;
	}

	SdaiEntity iIFCRelAggregatesEntity = sdaiGetEntity(pModel->getSdaiInstance(), "IFCRELAGGREGATES");

	SdaiInteger iIFCIsDecomposedByInstancesCount = sdaiGetMemberCount(piIsDecomposedByInstances);
	for (SdaiInteger i = 0; i < iIFCIsDecomposedByInstancesCount; ++i)
	{
		SdaiInstance iIFCIsDecomposedByInstance = 0;
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

		HTREEITEM hDecomposition = m_pTreeCtrl->InsertItem(&tvInsertStruct);

		SdaiAggr piIFCRelatedObjectsInstances = 0;
		sdaiGetAttrBN(iIFCIsDecomposedByInstance, "RelatedObjects", sdaiAGGR, &piIFCRelatedObjectsInstances);

		SdaiInteger iIFCRelatedObjectsInstancesCount = sdaiGetMemberCount(piIFCRelatedObjectsInstances);
		for (SdaiInteger j = 0; j < iIFCRelatedObjectsInstancesCount; ++j)
		{
			SdaiInstance iIFCRelatedObjectsInstance = 0;
			engiGetAggrElement(piIFCRelatedObjectsInstances, j, sdaiINSTANCE, &iIFCRelatedObjectsInstance);

			LoadInstance(pModel, iIFCRelatedObjectsInstance, hDecomposition);
		} // for (int_t j = ...
	} // for (int64_t i = ...	
}

void CCIS2ModelStructureView::LoadIsNestedBy(CCIS2Model* pModel, SdaiInstance iInstance, HTREEITEM hParent)
{
	ASSERT(pModel != nullptr);

	SdaiAggr piIsDecomposedByInstances = nullptr;
	sdaiGetAttrBN(iInstance, "IsNestedBy", sdaiAGGR, &piIsDecomposedByInstances);

	if (piIsDecomposedByInstances == nullptr)
	{
		return;
	}

	SdaiEntity iIFCRelNestsEntity = sdaiGetEntity(pModel->getSdaiInstance(), "IFCRELNESTS");

	SdaiInteger iIFCIsDecomposedByInstancesCount = sdaiGetMemberCount(piIsDecomposedByInstances);
	for (SdaiInteger i = 0; i < iIFCIsDecomposedByInstancesCount; ++i)
	{
		SdaiInstance iIFCIsDecomposedByInstance = 0;
		engiGetAggrElement(piIsDecomposedByInstances, i, sdaiINSTANCE, &iIFCIsDecomposedByInstance);

		if (sdaiGetInstanceType(iIFCIsDecomposedByInstance) != iIFCRelNestsEntity)
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

		HTREEITEM hDecomposition = m_pTreeCtrl->InsertItem(&tvInsertStruct);

		SdaiAggr piIFCRelatedObjectsInstances = 0;
		sdaiGetAttrBN(iIFCIsDecomposedByInstance, "RelatedObjects", sdaiAGGR, &piIFCRelatedObjectsInstances);

		SdaiInteger iIFCRelatedObjectsInstancesCount = sdaiGetMemberCount(piIFCRelatedObjectsInstances);
		for (SdaiInteger j = 0; j < iIFCRelatedObjectsInstancesCount; ++j)
		{
			SdaiInstance iIFCRelatedObjectsInstance = 0;
			engiGetAggrElement(piIFCRelatedObjectsInstances, j, sdaiINSTANCE, &iIFCRelatedObjectsInstance);

			LoadInstance(pModel, iIFCRelatedObjectsInstance, hDecomposition);
		} // for (int_t j = ...
	} // for (int64_t i = ...
}

// ------------------------------------------------------------------------------------------------
void CCIS2ModelStructureView::LoadContainsElements(CCIS2Model* pModel, SdaiInstance iInstance, HTREEITEM hParent)
{
	ASSERT(pModel != nullptr);

	SdaiAggr piContainsElementsInstances = nullptr;
	sdaiGetAttrBN(iInstance, "ContainsElements", sdaiAGGR, &piContainsElementsInstances);

	if (piContainsElementsInstances == nullptr)
	{
		return;
	}

	SdaiEntity iIFCRelContainedInSpatialStructureEntity = sdaiGetEntity(pModel->getSdaiInstance(), "IFCRELCONTAINEDINSPATIALSTRUCTURE");

	SdaiInteger iIFCContainsElementsInstancesCount = sdaiGetMemberCount(piContainsElementsInstances);
	for (SdaiInteger i = 0; i < iIFCContainsElementsInstancesCount; ++i)
	{
		SdaiInstance iIFCContainsElementsInstance = 0;
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

		HTREEITEM hContains = m_pTreeCtrl->InsertItem(&tvInsertStruct);

		SdaiAggr piIFCRelatedElementsInstances = 0;
		sdaiGetAttrBN(iIFCContainsElementsInstance, "RelatedElements", sdaiAGGR, &piIFCRelatedElementsInstances);

		SdaiInteger iIFCRelatedElementsInstancesCount = sdaiGetMemberCount(piIFCRelatedElementsInstances);
		for (SdaiInteger j = 0; j < iIFCRelatedElementsInstancesCount; ++j)
		{
			SdaiInstance iIFCRelatedElementsInstance = 0;
			engiGetAggrElement(piIFCRelatedElementsInstances, j, sdaiINSTANCE, &iIFCRelatedElementsInstance);

			LoadInstance(pModel, iIFCRelatedElementsInstance, hContains);
		} // for (int_t j = ...
	} // for (int64_t i = ...
}

// ------------------------------------------------------------------------------------------------
void CCIS2ModelStructureView::LoadInstance(CCIS2Model* pModel, SdaiInstance iInstance, HTREEITEM hParent)
{
	ASSERT(pModel != nullptr);

	auto& mapInstances = pModel->GetInstances();

	auto itInstance = mapInstances.find(iInstance);
	if (itInstance != mapInstances.end())
	{
		wstring strItem = _ap_instance::GetName(iInstance);

		/*
		* Object
		*/
		TV_INSERTSTRUCT tvInsertStruct;
		tvInsertStruct.hParent = hParent;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = 
			itInstance->second->_instance::hasGeometry() ?
			(itInstance->second->_instance::getEnable() ? IMAGE_SELECTED : IMAGE_NOT_SELECTED) :
			IMAGE_NO_GEOMETRY;
		tvInsertStruct.item.lParam = NULL;

		HTREEITEM hObject = m_pTreeCtrl->InsertItem(&tvInsertStruct);

		/*
		* Geometry
		*/
		tvInsertStruct.hParent = hObject;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = ITEM_GEOMETRY;
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage =
			itInstance->second->_instance::hasGeometry() ?
			(itInstance->second->_instance::getEnable() ? IMAGE_SELECTED : IMAGE_NOT_SELECTED) :
			IMAGE_NO_GEOMETRY;
		tvInsertStruct.item.lParam = (LPARAM)itInstance->second;

		HTREEITEM hGeometry = m_pTreeCtrl->InsertItem(&tvInsertStruct);
		m_mapInstance2GeometryItem[itInstance->second] = hGeometry;

		/*
		* decomposition/nest/contains
		*/
		LoadIsDecomposedBy(pModel, iInstance, hObject);
		LoadIsNestedBy(pModel, iInstance, hObject);
		LoadContainsElements(pModel, iInstance, hObject);
	} // if (itInstance != ...
	else
	{
		ASSERT(FALSE);
	}
}

// ------------------------------------------------------------------------------------------------
void CCIS2ModelStructureView::LoadUnreferencedItems(CCIS2Model* pModel, HTREEITEM hModel)
{
	ASSERT(pModel != nullptr);

	map<wstring, vector<CCIS2Instance*>> mapUnreferencedItems;

	auto& mapInstances = pModel->GetInstances();

	auto itInstance = mapInstances.begin();
	for (; itInstance != mapInstances.end(); itInstance++)
	{
		if (!itInstance->second->_instance::hasGeometry())
		{
			continue;
		}

		if (!itInstance->second->Referenced())
		{
			const wchar_t* szEntity = itInstance->second->GetEntityName();

			auto itUnreferencedItems = mapUnreferencedItems.find(szEntity);
			if (itUnreferencedItems == mapUnreferencedItems.end())
			{
				vector<CCIS2Instance*> veCIFCInstances;
				veCIFCInstances.push_back(itInstance->second);

				mapUnreferencedItems[szEntity] = veCIFCInstances;
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

	HTREEITEM hUnreferenced = m_pTreeCtrl->InsertItem(&tvInsertStruct);

	map<wstring, vector<CCIS2Instance*>>::iterator itUnreferencedItems = mapUnreferencedItems.begin();
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

		HTREEITEM hEntity = m_pTreeCtrl->InsertItem(&tvInsertStruct);

		for (size_t iInstance = 0; iInstance < itUnreferencedItems->second.size(); iInstance++)
		{
			auto pInstance = itUnreferencedItems->second[iInstance];

			wstring strItem = _ap_instance::GetName(pInstance->_ap_geometry::getSdaiInstance());

			/*
			* Object
			*/
			tvInsertStruct.hParent = hEntity;
			tvInsertStruct.hInsertAfter = TVI_LAST;
			tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
			tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
			tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_SELECTED;
			tvInsertStruct.item.lParam = NULL;

			HTREEITEM hObject = m_pTreeCtrl->InsertItem(&tvInsertStruct);

			/*
			* Geometry
			*/
			tvInsertStruct.hParent = hObject;
			tvInsertStruct.hInsertAfter = TVI_LAST;
			tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
			tvInsertStruct.item.pszText = ITEM_GEOMETRY;
			tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage =
				pInstance->_instance::hasGeometry() ? (pInstance->_instance::getEnable() ? IMAGE_SELECTED : IMAGE_NOT_SELECTED) : IMAGE_NO_GEOMETRY;
			tvInsertStruct.item.lParam = (LPARAM)pInstance;

			HTREEITEM hGeometry = m_pTreeCtrl->InsertItem(&tvInsertStruct);
			m_mapInstance2GeometryItem[pInstance] = hGeometry;
		} // for (size_t iInstance = ...
	} // for (; itUnreferencedItems != ...
}

// ------------------------------------------------------------------------------------------------
void CCIS2ModelStructureView::LoadTree_UpdateItems(HTREEITEM hModel)
{
	HTREEITEM hModelChild = m_pTreeCtrl->GetNextItem(hModel, TVGN_CHILD);
	while (hModelChild != nullptr)
	{
		int iImage, iSelectedImage = -1;
		m_pTreeCtrl->GetItemImage(hModelChild, iImage, iSelectedImage);

		ASSERT(iImage == iSelectedImage);

		if ((iImage != IMAGE_SELECTED) && (iImage != IMAGE_SEMI_SELECTED) && (iImage != IMAGE_NOT_SELECTED))
		{
			// skip the Header and unreferenced items
			hModelChild = m_pTreeCtrl->GetNextSiblingItem(hModelChild);

			continue;
		}

		LoadTree_UpdateItem(hModelChild);

		hModelChild = m_pTreeCtrl->GetNextSiblingItem(hModelChild);
	}
}

// ----------------------------------------------------------------------------
void CCIS2ModelStructureView::LoadTree_UpdateItem(HTREEITEM hParent)
{
	ASSERT(hParent != nullptr);

	if (!m_pTreeCtrl->ItemHasChildren(hParent))
	{
		// keep the state as it is
		return;
	}

	int iChidlrenCount = 0;
	int iSelectedChidlrenCount = 0;
	int iSemiSelectedChidlrenCount = 0;

	HTREEITEM hChild = m_pTreeCtrl->GetNextItem(hParent, TVGN_CHILD);
	while (hChild != nullptr)
	{
		int iImage, iSelectedImage = -1;
		m_pTreeCtrl->GetItemImage(hChild, iImage, iSelectedImage);

		ASSERT(iImage == iSelectedImage);

		if ((iImage != IMAGE_SELECTED) && (iImage != IMAGE_SEMI_SELECTED) && (iImage != IMAGE_NOT_SELECTED))
		{
			// skip the properties, items without a geometry, etc.
			hChild = m_pTreeCtrl->GetNextSiblingItem(hChild);

			continue;
		}

		LoadTree_UpdateItem(hChild);

		iImage = iSelectedImage = -1;
		m_pTreeCtrl->GetItemImage(hChild, iImage, iSelectedImage);

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
			ASSERT(FALSE); // unexpected
		}
		break;
		} // switch (iImage)

		hChild = m_pTreeCtrl->GetNextSiblingItem(hChild);
	} // while (hChild != nullptr)

	if (iChidlrenCount == 0)
	{
		// keep the state as it is
		return;
	}

	if (iSemiSelectedChidlrenCount > 0)
	{
		m_pTreeCtrl->SetItemImage(hParent, IMAGE_SEMI_SELECTED, IMAGE_SEMI_SELECTED);

		return;
	}

	if (iSelectedChidlrenCount == 0)
	{
		m_pTreeCtrl->SetItemImage(hParent, IMAGE_NOT_SELECTED, IMAGE_NOT_SELECTED);

		return;
	}

	if (iSelectedChidlrenCount == iChidlrenCount)
	{
		m_pTreeCtrl->SetItemImage(hParent, IMAGE_SELECTED, IMAGE_SELECTED);

		return;
	}

	m_pTreeCtrl->SetItemImage(hParent, IMAGE_SEMI_SELECTED, IMAGE_SEMI_SELECTED);
}

// ----------------------------------------------------------------------------
void CCIS2ModelStructureView::ClickItem_UpdateChildren(HTREEITEM hParent)
{
	ASSERT(hParent != nullptr);

	if (!m_pTreeCtrl->ItemHasChildren(hParent))
	{
		return;
	}

	int iParentImage = -1;
	int iParentSelectedImage = -1;
	m_pTreeCtrl->GetItemImage(hParent, iParentImage, iParentSelectedImage);

	ASSERT(iParentImage == iParentSelectedImage);

	HTREEITEM hChild = m_pTreeCtrl->GetNextItem(hParent, TVGN_CHILD);
	while (hChild != nullptr)
	{
		int iImage, iSelectedImage = -1;
		m_pTreeCtrl->GetItemImage(hChild, iImage, iSelectedImage);

		ASSERT(iImage == iSelectedImage);

		if ((iImage != IMAGE_SELECTED) && (iImage != IMAGE_SEMI_SELECTED) && (iImage != IMAGE_NOT_SELECTED))
		{
			// skip the properties, items without a geometry, etc.
			hChild = m_pTreeCtrl->GetNextSiblingItem(hChild);

			continue;
		}

		m_pTreeCtrl->SetItemImage(hChild, iParentImage, iParentImage);

		CCIS2Instance* pInstance = (CCIS2Instance*)m_pTreeCtrl->GetItemData(hChild);
		if (pInstance != nullptr)
		{
			pInstance->_instance::setEnable((iParentImage == IMAGE_SELECTED) || (iParentImage == IMAGE_SEMI_SELECTED) ? true : false);
		}

		ClickItem_UpdateChildren(hChild);

		hChild = m_pTreeCtrl->GetNextSiblingItem(hChild);
	} // while (hChild != nullptr)
}

// ----------------------------------------------------------------------------
void CCIS2ModelStructureView::ClickItem_UpdateParent(HTREEITEM hParent, BOOL bRecursive/* = TRUE*/)
{
	if (hParent == nullptr)
	{
		return;
	}

	ASSERT(m_pTreeCtrl->ItemHasChildren(hParent));

	int iChidlrenCount = 0;
	int iSelectedChidlrenCount = 0;
	int iSemiSelectedChidlrenCount = 0;

	HTREEITEM hChild = m_pTreeCtrl->GetNextItem(hParent, TVGN_CHILD);
	while (hChild != nullptr)
	{
		int iImage, iSelectedImage = -1;
		m_pTreeCtrl->GetItemImage(hChild, iImage, iSelectedImage);

		ASSERT(iImage == iSelectedImage);

		if ((iImage != IMAGE_SELECTED) && (iImage != IMAGE_SEMI_SELECTED) && (iImage != IMAGE_NOT_SELECTED))
		{
			// skip the properties, items without a geometry, etc.
			hChild = m_pTreeCtrl->GetNextSiblingItem(hChild);

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
				ASSERT(FALSE); // unexpected
			}
			break;
		} // switch (iImage)

		hChild = m_pTreeCtrl->GetNextSiblingItem(hChild);
	} // while (hChild != nullptr)

	if (iSemiSelectedChidlrenCount > 0)
	{
		m_pTreeCtrl->SetItemImage(hParent, IMAGE_SEMI_SELECTED, IMAGE_SEMI_SELECTED);

		CCIS2Instance* pInstance = (CCIS2Instance*)m_pTreeCtrl->GetItemData(hParent);
		if (pInstance != nullptr)
		{
			pInstance->_instance::setEnable(true);
		}

		if (bRecursive)
		{
			ClickItem_UpdateParent(m_pTreeCtrl->GetParentItem(hParent));
		}		

		return;
	}

	if (iSelectedChidlrenCount == 0)
	{
		m_pTreeCtrl->SetItemImage(hParent, IMAGE_NOT_SELECTED, IMAGE_NOT_SELECTED);

		CCIS2Instance* pInstance = (CCIS2Instance*)m_pTreeCtrl->GetItemData(hParent);
		if (pInstance != nullptr)
		{
			pInstance->_instance::setEnable(false);
		}

		if (bRecursive)
		{
			ClickItem_UpdateParent(m_pTreeCtrl->GetParentItem(hParent));
		}

		return;
	}

	if (iSelectedChidlrenCount == iChidlrenCount)
	{
		m_pTreeCtrl->SetItemImage(hParent, IMAGE_SELECTED, IMAGE_SELECTED);

		CCIS2Instance* pInstance = (CCIS2Instance*)m_pTreeCtrl->GetItemData(hParent);
		if (pInstance != nullptr)
		{
			pInstance->_instance::setEnable(true);
		}

		if (bRecursive)
		{
			ClickItem_UpdateParent(m_pTreeCtrl->GetParentItem(hParent));
		}

		return;
	}

	m_pTreeCtrl->SetItemImage(hParent, IMAGE_SEMI_SELECTED, IMAGE_SEMI_SELECTED);

	CCIS2Instance* pInstance = (CCIS2Instance*)m_pTreeCtrl->GetItemData(hParent);
	if (pInstance != nullptr)
	{
		pInstance->_instance::setEnable(true);
	}

	if (bRecursive)
	{
		ClickItem_UpdateParent(m_pTreeCtrl->GetParentItem(hParent));
	}
}

// ----------------------------------------------------------------------------
void CCIS2ModelStructureView::UnselectAllItems()
{
	auto itSelectedIInstance = m_mapSelectedInstances.begin();
	for (; itSelectedIInstance != m_mapSelectedInstances.end(); itSelectedIInstance++)
	{
		m_pTreeCtrl->SetItemState(itSelectedIInstance->second, 0, TVIS_BOLD);
	}

	m_mapSelectedInstances.clear();
}

// ----------------------------------------------------------------------------
void CCIS2ModelStructureView::ResetView()
{
	m_mapInstance2GeometryItem.clear();
	m_mapSelectedInstances.clear();

	m_pTreeCtrl->DeleteAllItems();

	auto pModel = GetModel<CCIS2Model>();
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	LoadModel(pModel);
}