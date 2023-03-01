#include "stdafx.h"
#include "mainfrm.h"
#include "STEPProductsTreeView.h"
#include "ProductDefinition.h"
#include "Resource.h"
#include "STEPViewer.h"
#include "STEPModel.h"
#include "Generic.h"
#include "FileViewConsts.h"

#include <algorithm>
#include <chrono>

using namespace std;

// ------------------------------------------------------------------------------------------------
CSTEPProductsTreeView::CSTEPProductsTreeView(CViewTree* pTreeView)
	: CSTEPTreeViewBase()
	, m_pTreeView(pTreeView)
	, m_pImageList(nullptr)
	, m_vecItemData()
	, m_mapInstance2Item()
	, m_hSelectedItem(nullptr)
	, m_bInitInProgress(false)
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

	m_pTreeView->SetImageList(m_pImageList, TVSIL_NORMAL);

	//  Search
	m_pSearchDialog = new CSearchInstancesDialog(m_pTreeView);
	m_pSearchDialog->Create(IDD_DIALOG_SEARCH, m_pTreeView);
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ CSTEPProductsTreeView::~CSTEPProductsTreeView()
{
	m_pTreeView->SetImageList(nullptr, TVSIL_NORMAL);

	m_pImageList->DeleteImageList();
	delete m_pImageList;

	for (size_t iItemData = 0; iItemData < m_vecItemData.size(); iItemData++)
	{
		delete m_vecItemData[iItemData];
	}
	m_vecItemData.clear();

	delete m_pSearchDialog;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CSTEPProductsTreeView::OnInstanceSelected(CSTEPView* pSender)
{
	if (pSender == this)
	{
		return;
	}

	if (m_hSelectedItem != nullptr)
	{
		m_pTreeView->SetItemState(m_hSelectedItem, 0, TVIS_BOLD);
		m_hSelectedItem = nullptr;
	}

	auto pController = GetController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pSelectedInstance = pController->GetSelectedInstance() != nullptr ? dynamic_cast<CProductInstance*>(GetController()->GetSelectedInstance()) : nullptr;
	if (pSelectedInstance == nullptr)
	{
		/*
		* Select the Model by default
		*/
		HTREEITEM hModel = m_pTreeView->GetChildItem(nullptr);
		ASSERT(hModel != nullptr);

		m_pTreeView->SelectItem(hModel);

		return;
	}

	auto itInstance2Item = m_mapInstance2Item.find(pSelectedInstance);
	if (itInstance2Item == m_mapInstance2Item.end())
	{
		LoadInstanceAncestors(pSelectedInstance);
	}

	itInstance2Item = m_mapInstance2Item.find(pSelectedInstance);
	if (itInstance2Item == m_mapInstance2Item.end())
	{
		ASSERT(FALSE);

		return;
	}

	/*
	* Disable the drawing
	*/
	m_pTreeView->SendMessage(WM_SETREDRAW, 0, 0);

	m_hSelectedItem = itInstance2Item->second;

	m_pTreeView->SetItemState(m_hSelectedItem, TVIS_BOLD, TVIS_BOLD);
	m_pTreeView->EnsureVisible(m_hSelectedItem);
	m_pTreeView->SelectItem(m_hSelectedItem);

	/*
	* Enable the drawing
	*/
	m_pTreeView->SendMessage(WM_SETREDRAW, 1, 0);
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CSTEPProductsTreeView::Load()
{
	ResetView();
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ CImageList* CSTEPProductsTreeView::GetImageList() const
{
	return m_pImageList;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CSTEPProductsTreeView::OnShowWindow(BOOL bShow, UINT /*nStatus*/)
{
	if (!bShow)
	{
		m_pSearchDialog->ShowWindow(SW_HIDE);
	}
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CSTEPProductsTreeView::OnTreeItemClick(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	*pResult = 0;

	if (m_bInitInProgress)
	{
		return;
	}

	DWORD dwPosition = GetMessagePos();
	CPoint point(LOWORD(dwPosition), HIWORD(dwPosition));
	m_pTreeView->ScreenToClient(&point);

	UINT uFlags = 0;
	HTREEITEM hItem = m_pTreeView->HitTest(point, &uFlags);

	ASSERT(GetController() != nullptr);

	/*
	* TVHT_ONITEMICON
	*/
	if ((hItem != nullptr) && ((uFlags & TVHT_ONITEMICON) == TVHT_ONITEMICON))
	{
		auto pController = GetController();
		if (pController == nullptr)
		{
			ASSERT(FALSE);

			return;
		}

		int iImage = -1;
		int iSelectedImage = -1;
		m_pTreeView->GetItemImage(hItem, iImage, iSelectedImage);

		ASSERT(iImage == iSelectedImage);

		switch (iImage)
		{
			case IMAGE_SELECTED:
			case IMAGE_SEMI_SELECTED:
			{
				auto pItemData = (CSTEPItemData*)m_pTreeView->GetItemData(hItem);
				if ((pItemData != nullptr) && (pItemData->getType() == enumSTEPItemDataType::dtProductInstance))
				{
					pItemData->getInstance<CProductInstance>()->setEnable(false);
				}
				
				UpdateChildrenInMemory(pItemData, false);
				
				m_pTreeView->SetItemImage(hItem, IMAGE_NOT_SELECTED, IMAGE_NOT_SELECTED);

				UpdateChildren(hItem);
				UpdateParent(m_pTreeView->GetParentItem(hItem));

				pController->OnInstancesEnabledStateChanged(this);
			}
			break;

			case IMAGE_NOT_SELECTED:
			{
				auto pItemData = (CSTEPItemData*)m_pTreeView->GetItemData(hItem);
				if ((pItemData != nullptr) && (pItemData->getType() == enumSTEPItemDataType::dtProductInstance))
				{
					pItemData->getInstance<CProductInstance>()->setEnable(true);
				}

				UpdateChildrenInMemory(pItemData, true);

				m_pTreeView->SetItemImage(hItem, IMAGE_SELECTED, IMAGE_SELECTED);

				UpdateChildren(hItem);
				UpdateParent(m_pTreeView->GetParentItem(hItem));

				pController->OnInstancesEnabledStateChanged(this);
			}
			break;

			default:
			{
				// skip the properties, items without a geometry, etc.
				return;
			}
		} // switch (iImage)

		return;
	} // if ((hItem != nullptr) && ...

	/*
	* TVHT_ONITEMLABEL
	*/
	if ((hItem != nullptr) && ((uFlags & TVHT_ONITEMLABEL) == TVHT_ONITEMLABEL))
	{
		if (m_pTreeView->GetParentItem(m_hSelectedItem) != nullptr)
		{
			// keep the roots always bold
			m_pTreeView->SetItemState(m_hSelectedItem, 0, TVIS_BOLD);
		}

		m_pTreeView->SetItemState(hItem, TVIS_BOLD, TVIS_BOLD);
		m_hSelectedItem = hItem;

		auto pItemData = (CSTEPItemData*)m_pTreeView->GetItemData(hItem);
		if ((pItemData == nullptr) || (pItemData->getType() != enumSTEPItemDataType::dtProductInstance))
		{
			GetController()->SelectInstance(this, nullptr);

			return;
		}

		auto pInstance = pItemData->getInstance<CProductInstance>();
		ASSERT(pInstance != nullptr);

		GetController()->SelectInstance(this, pInstance);
	} // if ((hItem != nullptr) && ...
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CSTEPProductsTreeView::OnTreeItemExpanding(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;

	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	int iImage = -1;
	int iSelectedImage = -1;
	m_pTreeView->GetItemImage(pNMTreeView->itemNew.hItem, iImage, iSelectedImage);

	ASSERT(iImage == iSelectedImage);

	if (pNMTreeView->itemNew.cChildren == 1)
	{
		HTREEITEM hChild = m_pTreeView->GetChildItem(pNMTreeView->itemNew.hItem);
		if (hChild == nullptr)
		{
			ASSERT(FALSE); // Internal error

			return;
		}

		if (m_pTreeView->GetItemText(hChild) != ITEM_PENDING_LOAD)
		{
			return;
		}

		m_pTreeView->DeleteItem(hChild);

		CSTEPItemData* pItemData = (CSTEPItemData*)m_pTreeView->GetItemData(pNMTreeView->itemNew.hItem);
		if (pItemData == nullptr)
		{
			ASSERT(FALSE); // Internal error

			return;
		}

		LoadItemChildren(pItemData);

		/*
		* UI
		*/
		UpdateChildren(pItemData->treeItem());
	} // if (pNMTreeView->itemNew.cChildren == 1)
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ bool CSTEPProductsTreeView::IsSelected(HTREEITEM /*hItem*/)
{
	ASSERT(FALSE); // TODO

	return false;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CSTEPProductsTreeView::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
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

	/*
	* Instances
	*/
	auto pItemData = (CSTEPItemData*)m_pTreeView->GetItemData(hItem);
	if ((pItemData != nullptr) && (pItemData->getType() == enumSTEPItemDataType::dtProductInstance))
	{
		auto pInstance = pItemData->getInstance<CProductInstance>();

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

		if (pInstance->getEnable())
		{
			pPopup->CheckMenuItem(ID_INSTANCES_ENABLE, MF_BYCOMMAND | MF_CHECKED);
		}

		UINT uiCommand = pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, point.x, point.y, m_pTreeView);
		if (uiCommand == 0)
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

		auto pModel = pController->GetModel()->As<CSTEPModel>();
		if (pModel == nullptr)
		{
			ASSERT(FALSE);

			return;
		}

		auto& mapInstances = pModel->getProductInstances();

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
				
				UpdateChildrenInMemory(pItemData, pInstance->getEnable());
				
				UpdateChildren(hItem);
				UpdateParent(m_pTreeView->GetParentItem(hItem));
								
				pController->OnInstancesEnabledStateChanged(this);
			}
			break;

			case ID_INSTANCES_DISABLE_ALL_BUT_THIS:
			{
				auto itInstance = mapInstances.begin();
				for (; itInstance != mapInstances.end(); itInstance++)
				{
					itInstance->second->setEnable(itInstance->second == pInstance);
				}

				ASSERT(pInstance->getEnable());
				
				ResetTree(false);				

				auto itInstance2Item = m_mapInstance2Item.find(pInstance);
				if (itInstance2Item == m_mapInstance2Item.end())
				{
					LoadInstanceAncestors(pInstance);
				}

				itInstance2Item = m_mapInstance2Item.find(pInstance);
				if (itInstance2Item == m_mapInstance2Item.end())
				{
					ASSERT(FALSE);

					return;
				}

				m_pTreeView->SetItemImage(itInstance2Item->second, IMAGE_SELECTED, IMAGE_SELECTED);
				
				UpdateChildren(itInstance2Item->second);
				UpdateParent(m_pTreeView->GetParentItem(itInstance2Item->second));

				pController->OnInstancesEnabledStateChanged(this);
			}
			break;

			case ID_INSTANCES_ENABLE_ALL:
			{
				auto itInstance = mapInstances.begin();
				for (; itInstance != mapInstances.end(); itInstance++)
				{
					itInstance->second->setEnable(true);
				}

				ResetTree(true);

				pController->OnInstancesEnabledStateChanged(this);
			}
			break;

			case ID_INSTANCES_SEARCH:
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
			break;

			default:
			{
				ASSERT(false);
			}
			break;
		} // switch (uiCommand)	

		return;
	} // if ((pItemData != nullptr) && ...	

	/*
	* Product Definitions and Assemblies
	*/
	CMenu menu;
	VERIFY(menu.LoadMenuW(IDR_POPUP_INSTANCES_NO_GEOMETRY));

	auto pPopup = menu.GetSubMenu(0);

	UINT uiCommand = pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, point.x, point.y, m_pTreeView);
	if (uiCommand == 0)
	{
		return;
	}

	switch (uiCommand)
	{
		case ID_INSTANCES_SEARCH:
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
		break;

		default:
		{
			ASSERT(false);
		}
		break;
	} // switch (uiCommand)	
}

// ------------------------------------------------------------------------------------------------
void CSTEPProductsTreeView::LoadHeaderDescription(HTREEITEM hParent)
{
	auto pController = GetController();
	ASSERT(pController != nullptr);

	if (pController->GetModel() == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pModel = pController->GetModel()->As<CSTEPModel>();
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	wchar_t* szText = nullptr;
	TV_INSERTSTRUCT tvInsertStruct;

	/*
	* Descriptions
	*/
	{
		tvInsertStruct.hParent = hParent;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = L"Descriptions";
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY_SET;
		tvInsertStruct.item.lParam = NULL;

		HTREEITEM hDescriptions = m_pTreeView->InsertItem(&tvInsertStruct);

		int_t iItem = 0;
		if (!GetSPFFHeaderItem(pModel->GetInstance(), 0, iItem, sdaiUNICODE, (char**)&szText))
		{
			while (!GetSPFFHeaderItem(pModel->GetInstance(), 0, iItem++, sdaiUNICODE, (char**)&szText))
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

				m_pTreeView->InsertItem(&tvInsertStruct);
			}
		} // if (!GetSPFFHeaderItem(...
	}

	/*
	* ImplementationLevel
	*/
	{
		GetSPFFHeaderItem(pModel->GetInstance(), 1, 0, sdaiUNICODE, (char**)&szText);

		wstring strItem = L"ImplementationLevel";
		strItem += L" = '";
		strItem += szText != nullptr ? szText : L"";
		strItem += L"'";

		tvInsertStruct.hParent = hParent;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
		tvInsertStruct.item.lParam = NULL;

		m_pTreeView->InsertItem(&tvInsertStruct);
	}

	/*
	* Name
	*/
	{
		GetSPFFHeaderItem(pModel->GetInstance(), 2, 0, sdaiUNICODE, (char**)&szText);

		wstring strItem = L"Name";
		strItem += L" = '";
		strItem += szText != nullptr ? szText : L"";
		strItem += L"'";

		tvInsertStruct.hParent = hParent;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
		tvInsertStruct.item.lParam = NULL;

		m_pTreeView->InsertItem(&tvInsertStruct);
	}

	/*
	* TimeStamp
	*/
	{
		GetSPFFHeaderItem(pModel->GetInstance(), 3, 0, sdaiUNICODE, (char**)&szText);

		wstring strItem = L"TimeStamp";
		strItem += L" = '";
		strItem += szText != nullptr ? szText : L"";
		strItem += L"'";

		tvInsertStruct.hParent = hParent;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
		tvInsertStruct.item.lParam = NULL;

		m_pTreeView->InsertItem(&tvInsertStruct);
	}

	/*
	* Authors
	*/
	{
		tvInsertStruct.hParent = hParent;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = L"Authors";
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY_SET;
		tvInsertStruct.item.lParam = NULL;

		HTREEITEM hDescriptions = m_pTreeView->InsertItem(&tvInsertStruct);

		int_t iItem = 0;
		if (!GetSPFFHeaderItem(pModel->GetInstance(), 4, iItem, sdaiUNICODE, (char**)&szText))
		{
			while (!GetSPFFHeaderItem(pModel->GetInstance(), 4, iItem++, sdaiUNICODE, (char**)&szText))
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

				m_pTreeView->InsertItem(&tvInsertStruct);
			}
		} // if (!GetSPFFHeaderItem(...
	}

	/*
	* Organizations
	*/
	{
		tvInsertStruct.hParent = hParent;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = L"Organizations";
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY_SET;
		tvInsertStruct.item.lParam = NULL;

		HTREEITEM hDescriptions = m_pTreeView->InsertItem(&tvInsertStruct);

		int_t iItem = 0;
		if (!GetSPFFHeaderItem(pModel->GetInstance(), 5, iItem, sdaiUNICODE, (char**)&szText))
		{
			while (!GetSPFFHeaderItem(pModel->GetInstance(), 5, iItem++, sdaiUNICODE, (char**)&szText))
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

				m_pTreeView->InsertItem(&tvInsertStruct);
			}
		} // if (!GetSPFFHeaderItem(...
	}

	/*
	* PreprocessorVersion
	*/
	{
		GetSPFFHeaderItem(pModel->GetInstance(), 6, 0, sdaiUNICODE, (char**)&szText);

		wstring strItem = L"PreprocessorVersion";
		strItem += L" = '";
		strItem += szText != nullptr ? szText : L"";
		strItem += L"'";

		tvInsertStruct.hParent = hParent;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
		tvInsertStruct.item.lParam = NULL;

		m_pTreeView->InsertItem(&tvInsertStruct);
	}

	/*
	* OriginatingSystem
	*/
	{
		GetSPFFHeaderItem(pModel->GetInstance(), 7, 0, sdaiUNICODE, (char**)&szText);

		wstring strItem = L"OriginatingSystem";
		strItem += L" = '";
		strItem += szText != nullptr ? szText : L"";
		strItem += L"'";

		tvInsertStruct.hParent = hParent;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
		tvInsertStruct.item.lParam = NULL;

		m_pTreeView->InsertItem(&tvInsertStruct);
	}

	/*
	* Authorization
	*/
	{
		GetSPFFHeaderItem(pModel->GetInstance(), 8, 0, sdaiUNICODE, (char**)&szText);

		wstring strItem = L"Authorization";
		strItem += L" = '";
		strItem += szText != nullptr ? szText : L"";
		strItem += L"'";

		tvInsertStruct.hParent = hParent;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
		tvInsertStruct.item.lParam = NULL;

		m_pTreeView->InsertItem(&tvInsertStruct);
	}

	/*
	* FileSchemas
	*/
	{
		tvInsertStruct.hParent = hParent;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = L"FileSchemas";
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY_SET;
		tvInsertStruct.item.lParam = NULL;

		HTREEITEM hDescriptions = m_pTreeView->InsertItem(&tvInsertStruct);

		int_t iItem = 0;
		if (!GetSPFFHeaderItem(pModel->GetInstance(), 9, iItem, sdaiUNICODE, (char**)&szText))
		{
			while (!GetSPFFHeaderItem(pModel->GetInstance(), 9, iItem++, sdaiUNICODE, (char**)&szText))
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

				m_pTreeView->InsertItem(&tvInsertStruct);
			}
		} // if (!GetSPFFHeaderItem(...
	}
}

// ------------------------------------------------------------------------------------------------
void CSTEPProductsTreeView::LoadModel()
{
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	m_bInitInProgress = true;

	auto pController = GetController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	if (GetController()->GetModel() == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pModel = GetController()->GetModel()->As<CSTEPModel>();
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}	

	auto& mapDefinitions = pModel->getProductDefinitions();
	if (mapDefinitions.empty())
	{
		return;
	}

	/*
	* Header
	*/
	HTREEITEM hHeader = m_pTreeView->InsertItem(_T("Header"), IMAGE_PROPERTY_SET, IMAGE_PROPERTY_SET);
	LoadHeaderDescription(hHeader);

	/*
	* Model
	*/
	auto pModelItemData = new CSTEPItemData(nullptr, (int64_t*)pModel, enumSTEPItemDataType::dtModel);
	m_vecItemData.push_back(pModelItemData);

	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = nullptr;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
	tvInsertStruct.item.pszText = _T("Model");
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_SELECTED;
	tvInsertStruct.item.cChildren = !mapDefinitions.empty() ? 1 : 0;
	tvInsertStruct.item.lParam = (LPARAM)pModelItemData;

	HTREEITEM hModel = m_pTreeView->InsertItem(&tvInsertStruct);
	pModelItemData->treeItem() = hModel;

	/*
	* Roots
	*/
	auto itDefinition = mapDefinitions.begin();
	for (; itDefinition != mapDefinitions.end(); itDefinition++)
	{
		CProductDefinition* pDefinition = itDefinition->second;

		if (pDefinition->getRelatedProductRefs() == 0)
		{
			// Load all items in the memory
			LoadProductDefinitionInMemory(pModel, pDefinition, pModelItemData);

			// Load all items and populate the tree
			//LoadProductDefinition(pModel, pDefinition, hModel);
		}
	}

	m_pTreeView->InsertItem(ITEM_PENDING_LOAD, IMAGE_SELECTED, IMAGE_SELECTED, hModel);

	m_bInitInProgress = false;

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	TRACE(L"\n*** CSTEPProductsTreeView::LoadModel() : %lld [µs]", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());
}

// ------------------------------------------------------------------------------------------------
void CSTEPProductsTreeView::LoadProductDefinition(CSTEPModel* pModel, CProductDefinition* pDefinition, HTREEITEM hParent)
{
	CString strName = pDefinition->getId();
	strName += ITEM_PRODUCT_DEFINION;

	/*
	* Instance
	*/
	if (pDefinition->getRelatingProductRefs() > 0)
	{
		HTREEITEM hProductDefinition = m_pTreeView->InsertItem(strName, IMAGE_SELECTED, IMAGE_SELECTED, hParent);
		m_pTreeView->InsertItem(ITEM_GEOMETRY, IMAGE_NO_GEOMETRY, IMAGE_NO_GEOMETRY, hProductDefinition);

		auto pItemData = new CSTEPItemData(nullptr, (int64_t*)pDefinition, enumSTEPItemDataType::dtProductDefinition);
		m_vecItemData.push_back(pItemData);

		m_pTreeView->SetItemData(hProductDefinition, (DWORD_PTR)pItemData);

		WalkAssemblyTreeRecursively(pModel, pDefinition, hProductDefinition);
	} // if (pDefinition->getRelatingProductRefs() > 0)
	else
	{
		auto& vecInstances = pDefinition->getProductInstances();
		int iInstance = pDefinition->getNextProductInstance();

		strName = pDefinition->getId();
		strName += ITEM_PRODUCT_INSTANCE;

		HTREEITEM hInstance = m_pTreeView->InsertItem(strName, IMAGE_SELECTED, IMAGE_SELECTED, hParent);

		auto pItemData = new CSTEPItemData(nullptr, (int64_t*)vecInstances[iInstance], enumSTEPItemDataType::dtProductInstance);
		m_vecItemData.push_back(pItemData);

		m_pTreeView->SetItemData(hInstance, (DWORD_PTR)pItemData);

		int iImage = vecInstances[iInstance]->getEnable() ? IMAGE_SELECTED : IMAGE_NOT_SELECTED;
		HTREEITEM hGeometry = m_pTreeView->InsertItem(ITEM_GEOMETRY, iImage, iImage, hInstance);
		m_pTreeView->SetItemData(hGeometry, (DWORD_PTR)pItemData);

		ASSERT(m_mapInstance2Item.find(vecInstances[iInstance]) == m_mapInstance2Item.end());
		m_mapInstance2Item[vecInstances[iInstance]] = hInstance;
	} // else if (pDefinition->getRelatingProductRefs() > 0)
}

// ------------------------------------------------------------------------------------------------
void CSTEPProductsTreeView::WalkAssemblyTreeRecursively(CSTEPModel* pModel, CProductDefinition* pDefinition, HTREEITEM hParent)
{
	const map<int_t, CAssembly*>& mapAssemblies = pModel->getAssemblies();

	map<int_t, CAssembly*>::const_iterator itAssembly = mapAssemblies.begin();
	for (; itAssembly != mapAssemblies.end(); itAssembly++)
	{
		CAssembly* pAssembly = itAssembly->second;

		if (pAssembly->getRelatingProductDefinition() == pDefinition)
		{
			/*
			* Assembly
			*/
			CString strName = pAssembly->getId();
			strName += ITEM_ASSEMBLY;

			HTREEITEM hAssembly = m_pTreeView->InsertItem(strName, IMAGE_SELECTED, IMAGE_SELECTED, hParent);
			m_pTreeView->InsertItem(ITEM_GEOMETRY, IMAGE_NO_GEOMETRY, IMAGE_NO_GEOMETRY, hAssembly);

			auto pItemData = new CSTEPItemData(nullptr, (int64_t*)pAssembly, enumSTEPItemDataType::dtAssembly);
			m_vecItemData.push_back(pItemData);

			m_pTreeView->SetItemData(hAssembly, (DWORD_PTR)pItemData);

			/*
			* Instance
			*/
			if (pAssembly->getRelatedProductDefinition()->getRelatingProductRefs() == 0)
			{
				auto& vecInstances = pAssembly->getRelatedProductDefinition()->getProductInstances();
				int iInstance = pAssembly->getRelatedProductDefinition()->getNextProductInstance();

				strName = pAssembly->getRelatedProductDefinition()->getId();
				strName += ITEM_PRODUCT_INSTANCE;

				HTREEITEM hInstance = m_pTreeView->InsertItem(strName, IMAGE_SELECTED, IMAGE_SELECTED, hAssembly);

				pItemData = new CSTEPItemData(nullptr, (int64_t*)vecInstances[iInstance], enumSTEPItemDataType::dtProductInstance);
				m_vecItemData.push_back(pItemData);

				m_pTreeView->SetItemData(hInstance, (DWORD_PTR)pItemData);

				int iImage = vecInstances[iInstance]->getEnable() ? IMAGE_SELECTED : IMAGE_NOT_SELECTED;
				HTREEITEM hGeometry = m_pTreeView->InsertItem(ITEM_GEOMETRY, iImage, iImage, hInstance);
				m_pTreeView->SetItemData(hGeometry, (DWORD_PTR)pItemData);

				ASSERT(m_mapInstance2Item.find(vecInstances[iInstance]) == m_mapInstance2Item.end());
				m_mapInstance2Item[vecInstances[iInstance]] = hInstance;
			} // if (pAssembly->getRelatedProductDefinition()->getRelatingProductRefs() == 0)
			else
			{
				/*
				* Product
				*/
				strName = pDefinition->getId();
				strName += ITEM_PRODUCT_DEFINION;

				HTREEITEM hProductDefinition = m_pTreeView->InsertItem(strName, IMAGE_SELECTED, IMAGE_SELECTED, hAssembly);
				m_pTreeView->InsertItem(ITEM_GEOMETRY, IMAGE_NO_GEOMETRY, IMAGE_NO_GEOMETRY, hProductDefinition);

				pItemData = new CSTEPItemData(nullptr, (int64_t*)pDefinition, enumSTEPItemDataType::dtProductDefinition);
				m_vecItemData.push_back(pItemData);

				m_pTreeView->SetItemData(hProductDefinition, (DWORD_PTR)pItemData);

				WalkAssemblyTreeRecursively(pModel, pAssembly->getRelatedProductDefinition(), hProductDefinition);
			} // else if (pAssembly->getRelatedProductDefinition()->getRelatingProductRefs() == 0)			
		} // if (pAssembly->m_pRelatingProductDefinition == ...
	} // for (; itAssembly != ...	
}

// ------------------------------------------------------------------------------------------------
void CSTEPProductsTreeView::LoadProductDefinitionInMemory(CSTEPModel* pModel, CProductDefinition* pDefinition, CSTEPItemData* pParent)
{
	/*
	* Instance
	*/
	if (pDefinition->getRelatingProductRefs() > 0)
	{
		auto pProductItemData = new CSTEPItemData(pParent, (int64_t*)pDefinition, enumSTEPItemDataType::dtProductDefinition);
		pParent->children().push_back(pProductItemData);

		m_vecItemData.push_back(pProductItemData);

		WalkAssemblyTreeRecursivelyInMemory(pModel, pDefinition, pProductItemData);
	} // if (pDefinition->getRelatingProductRefs() > 0)	
	else
	{
		auto& vecInstances = pDefinition->getProductInstances();
		int iInstance = pDefinition->getNextProductInstance();

		auto pProductInstanceData = new CSTEPItemData(pParent, (int64_t*)vecInstances[iInstance], enumSTEPItemDataType::dtProductInstance);
		pParent->children().push_back(pProductInstanceData);

		m_vecItemData.push_back(pProductInstanceData);
	} // else if (pDefinition->getRelatingProductRefs() > 0)
}

// ------------------------------------------------------------------------------------------------
void CSTEPProductsTreeView::WalkAssemblyTreeRecursivelyInMemory(CSTEPModel* pModel, CProductDefinition* pDefinition, CSTEPItemData* pParent)
{
	const map<int_t, CAssembly*>& mapAssemblies = pModel->getAssemblies();

	map<int_t, CAssembly*>::const_iterator itAssembly = mapAssemblies.begin();
	for (; itAssembly != mapAssemblies.end(); itAssembly++)
	{
		CAssembly* pAssembly = itAssembly->second;

		if (pAssembly->getRelatingProductDefinition() == pDefinition)
		{
			/*
			* Assembly
			*/
			auto pAssemblyItemData = new CSTEPItemData(pParent, (int64_t*)pAssembly, enumSTEPItemDataType::dtAssembly);
			pParent->children().push_back(pAssemblyItemData);

			m_vecItemData.push_back(pAssemblyItemData);

			/*
			* Instance
			*/
			if (pAssembly->getRelatedProductDefinition()->getRelatingProductRefs() == 0)
			{
				auto& vecInstances = pAssembly->getRelatedProductDefinition()->getProductInstances();
				int iInstance = pAssembly->getRelatedProductDefinition()->getNextProductInstance();

				auto pInstanceItemData = new CSTEPItemData(pAssemblyItemData, (int64_t*)vecInstances[iInstance], enumSTEPItemDataType::dtProductInstance);
				pAssemblyItemData->children().push_back(pInstanceItemData);

				m_vecItemData.push_back(pInstanceItemData);
			} // if (pAssembly->getRelatedProductDefinition()->getRelatingProductRefs() == 0)
			else
			{
				/*
				* Product
				*/
				auto pProductItemData = new CSTEPItemData(pAssemblyItemData, (int64_t*)pDefinition, enumSTEPItemDataType::dtProductDefinition);
				pAssemblyItemData->children().push_back(pProductItemData);

				m_vecItemData.push_back(pProductItemData);

				WalkAssemblyTreeRecursivelyInMemory(pModel, pAssembly->getRelatedProductDefinition(), pProductItemData);
			} // else if (pAssembly->getRelatedProductDefinition()->getRelatingProductRefs() == 0)			
		} // if (pAssembly->m_pRelatingProductDefinition == ...
	} // for (; itAssembly != ...	

	auto& vecInstances = pDefinition->getProductInstances();
	int iInstance = pDefinition->getNextProductInstance();

	auto pInstanceItemData = new CSTEPItemData(pParent, (int64_t*)vecInstances[iInstance], enumSTEPItemDataType::dtProductInstance);
	pParent->children().push_back(pInstanceItemData);

	m_vecItemData.push_back(pInstanceItemData);
}

// ------------------------------------------------------------------------------------------------
CSTEPItemData* CSTEPProductsTreeView::FindItemData(CProductInstance* pInstance)
{
	for (size_t iItemData = 0; iItemData < m_vecItemData.size(); iItemData++)
	{
		if (m_vecItemData[iItemData]->getInstance<CProductInstance>() == pInstance)
		{
			return m_vecItemData[iItemData];
		}
	}

	return nullptr;
}

// ------------------------------------------------------------------------------------------------
void CSTEPProductsTreeView::LoadItemChildren(CSTEPItemData* pItemData)
{
	if ((pItemData == nullptr) || (pItemData->treeItem() == nullptr) || pItemData->children().empty())
	{
		ASSERT(FALSE); // Internal error

		return;
	}

	for (size_t iChild = 0; iChild < pItemData->children().size(); iChild++)
	{
		auto pChild = pItemData->children()[iChild];

		ASSERT(pChild->treeItem() == nullptr);

		CString strName;
		int iGeometryImage = IMAGE_NO_GEOMETRY;
		switch (pChild->getType())
		{
		case enumSTEPItemDataType::dtProductDefinition:
		{
			auto pDefinition = pChild->getInstance<CProductDefinition>();

			strName = pDefinition->getId();
			strName += ITEM_PRODUCT_DEFINION;
		}
		break;

		case enumSTEPItemDataType::dtAssembly:
		{
			auto pAssembly = pChild->getInstance<CAssembly>();

			strName = pAssembly->getId();
			strName += ITEM_ASSEMBLY;
		}
		break;

		case enumSTEPItemDataType::dtProductInstance:
		{
			auto pInstance = pChild->getInstance<CProductInstance>();

			strName = pInstance->getProductDefinition()->getId();
			strName += ITEM_PRODUCT_INSTANCE;

			iGeometryImage = IMAGE_SELECTED;
		}
		break;

		default:
		{
			ASSERT(FALSE); // Unexpected
		}
		break;
		} // switch (pChild->getType())

		TV_INSERTSTRUCT tvInsertStruct;
		tvInsertStruct.hParent = pItemData->treeItem();
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
		tvInsertStruct.item.pszText = strName.GetBuffer();
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_SELECTED;
		tvInsertStruct.item.cChildren = 1;
		tvInsertStruct.item.lParam = (LPARAM)pChild;

		HTREEITEM hChild = m_pTreeView->InsertItem(&tvInsertStruct);
		pChild->treeItem() = hChild;

		if (!pChild->children().empty())
		{
			m_pTreeView->InsertItem(ITEM_PENDING_LOAD, IMAGE_SELECTED, IMAGE_SELECTED, hChild);
		}

		HTREEITEM hGeometry = m_pTreeView->InsertItem(ITEM_GEOMETRY, iGeometryImage, iGeometryImage, hChild);

		if (pChild->getType() == enumSTEPItemDataType::dtProductInstance)
		{
			m_pTreeView->SetItemData(hGeometry, (DWORD_PTR)pChild);

			ASSERT(m_mapInstance2Item.find(pChild->getInstance<CProductInstance>()) == m_mapInstance2Item.end());
			m_mapInstance2Item[pChild->getInstance<CProductInstance>()] = hChild;
		}
	} // for (size_t iChild = ...
}

// ------------------------------------------------------------------------------------------------
void CSTEPProductsTreeView::LoadInstanceAncestors(CProductInstance* pInstance)
{
	if (pInstance == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pItemData = FindItemData(pInstance);
	if (pItemData == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	pItemData = pItemData->getParent();

	vector<CSTEPItemData*> vecAncestors;
	vecAncestors.push_back(pItemData);

	while (pItemData->treeItem() == nullptr)
	{
		pItemData = pItemData->getParent();

		vecAncestors.insert(vecAncestors.begin(), pItemData);
	}

	for (size_t iAncestor = 0; iAncestor < vecAncestors.size(); iAncestor++)
	{
		m_pTreeView->Expand(vecAncestors[iAncestor]->treeItem(), TVE_EXPAND);
	}
}

// ------------------------------------------------------------------------------------------------
void CSTEPProductsTreeView::ResetTree(bool bEnable)
{
	HTREEITEM hRoot = m_pTreeView->GetRootItem();
	while (hRoot != nullptr)
	{
		int iImage = -1;
		int iSelectedImage = -1;
		m_pTreeView->GetItemImage(hRoot, iImage, iSelectedImage);

		ASSERT(iImage == iSelectedImage);

		if ((iImage != IMAGE_SELECTED) && (iImage != IMAGE_SEMI_SELECTED) && (iImage != IMAGE_NOT_SELECTED))
		{
			// skip the Header and unreferenced items
			hRoot = m_pTreeView->GetNextSiblingItem(hRoot);

			continue;
		}

		ResetTree(hRoot, bEnable);

		hRoot = m_pTreeView->GetNextSiblingItem(hRoot);
	}
}

// ------------------------------------------------------------------------------------------------
void CSTEPProductsTreeView::ResetTree(HTREEITEM hParent, bool bEnable)
{
	if (hParent == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	HTREEITEM hChild = m_pTreeView->GetNextItem(hParent, TVGN_CHILD);
	while (hChild != nullptr)
	{
		ResetTree(hChild, bEnable);

		hChild = m_pTreeView->GetNextSiblingItem(hChild);
	} // while (hChild != nullptr)

	int iParentImage = -1;
	int iParentSelectedImage = -1;
	m_pTreeView->GetItemImage(hParent, iParentImage, iParentSelectedImage);

	ASSERT(iParentImage == iParentSelectedImage);

	if ((iParentImage == IMAGE_SELECTED) || (iParentImage == IMAGE_SEMI_SELECTED) || (iParentImage == IMAGE_NOT_SELECTED))
	{
		int iImage = bEnable ? IMAGE_SELECTED : IMAGE_NOT_SELECTED;
		m_pTreeView->SetItemImage(hParent, iImage, iImage);
	}
}

// ------------------------------------------------------------------------------------------------
void CSTEPProductsTreeView::UpdateChildren(HTREEITEM hParent)
{
	if (hParent == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	if (!m_pTreeView->ItemHasChildren(hParent))
	{
		return;
	}

	int iParentImage = -1;
	int iParentSelectedImage = -1;
	m_pTreeView->GetItemImage(hParent, iParentImage, iParentSelectedImage);

	ASSERT(iParentImage == iParentSelectedImage);

	HTREEITEM hChild = m_pTreeView->GetNextItem(hParent, TVGN_CHILD);
	while (hChild != nullptr)
	{
		int iImage = -1;
		int iSelectedImage = -1;
		m_pTreeView->GetItemImage(hChild, iImage, iSelectedImage);

		ASSERT(iImage == iSelectedImage);

		if ((iImage != IMAGE_SELECTED) && (iImage != IMAGE_SEMI_SELECTED) && (iImage != IMAGE_NOT_SELECTED))
		{
			// skip the properties, items without a geometry, etc.
			hChild = m_pTreeView->GetNextSiblingItem(hChild);

			continue;
		}

		m_pTreeView->SetItemImage(hChild, iParentImage, iParentImage);

#ifdef _DEBUG
		auto pItemData = (CSTEPItemData*)m_pTreeView->GetItemData(hChild);
		if ((pItemData != nullptr) && (pItemData->getType() == enumSTEPItemDataType::dtProductInstance))
		{
			if ((iParentImage == IMAGE_SELECTED) || (iParentImage == IMAGE_SEMI_SELECTED))
			{
				ASSERT(pItemData->getInstance<CProductInstance>()->getEnable());
			}
			else
			{
				ASSERT(!pItemData->getInstance<CProductInstance>()->getEnable());
			}
		}
#endif

		UpdateChildren(hChild);

		hChild = m_pTreeView->GetNextSiblingItem(hChild);
	} // while (hChild != nullptr)
}

// ------------------------------------------------------------------------------------------------
void CSTEPProductsTreeView::UpdateChildrenInMemory(CSTEPItemData* pParent, bool bEnable)
{
	if (pParent == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	for (size_t iChild = 0; iChild < pParent->children().size(); iChild++)
	{
		auto pChild = pParent->children()[iChild];
		if (pChild->getType() == enumSTEPItemDataType::dtProductInstance)
		{
			pChild->getInstance<CProductInstance>()->setEnable(bEnable);
		}

		UpdateChildrenInMemory(pChild, bEnable);
	} // for (size_t iChild = ...
}

// ------------------------------------------------------------------------------------------------
void CSTEPProductsTreeView::UpdateParent(HTREEITEM hParent)
{
	if (hParent == nullptr)
	{
		return;
	}

	ASSERT(m_pTreeView->ItemHasChildren(hParent));

	int iChidlrenCount = 0;
	int iSelectedChidlrenCount = 0;
	int iSemiSelectedChidlrenCount = 0;

	HTREEITEM hChild = m_pTreeView->GetNextItem(hParent, TVGN_CHILD);
	while (hChild != nullptr)
	{
		int iImage = -1;
		int iSelectedImage = -1;
		m_pTreeView->GetItemImage(hChild, iImage, iSelectedImage);

		ASSERT(iImage == iSelectedImage);

		if ((iImage != IMAGE_SELECTED) && (iImage != IMAGE_SEMI_SELECTED) && (iImage != IMAGE_NOT_SELECTED))
		{
			// skip the properties, items without a geometry, etc.
			hChild = m_pTreeView->GetNextSiblingItem(hChild);

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

		hChild = m_pTreeView->GetNextSiblingItem(hChild);
	} // while (hChild != nullptr)

	if (iSemiSelectedChidlrenCount > 0)
	{
		m_pTreeView->SetItemImage(hParent, IMAGE_SEMI_SELECTED, IMAGE_SEMI_SELECTED);

		CSTEPItemData* pItemData = (CSTEPItemData*)m_pTreeView->GetItemData(hParent);
		if ((pItemData != nullptr) && (pItemData->getType() == enumSTEPItemDataType::dtProductInstance))
		{
			pItemData->getInstance<CProductInstance>()->setEnable(true);
		}

		UpdateParent(m_pTreeView->GetParentItem(hParent));

		return;
	}

	if (iSelectedChidlrenCount == 0)
	{
		m_pTreeView->SetItemImage(hParent, IMAGE_NOT_SELECTED, IMAGE_NOT_SELECTED);

		CSTEPItemData* pItemData = (CSTEPItemData*)m_pTreeView->GetItemData(hParent);
		if ((pItemData != nullptr) && (pItemData->getType() == enumSTEPItemDataType::dtProductInstance))
		{
			pItemData->getInstance<CProductInstance>()->setEnable(false);
		}

		UpdateParent(m_pTreeView->GetParentItem(hParent));

		return;
	}

	if (iSelectedChidlrenCount == iChidlrenCount)
	{
		m_pTreeView->SetItemImage(hParent, IMAGE_SELECTED, IMAGE_SELECTED);

		CSTEPItemData* pItemData = (CSTEPItemData*)m_pTreeView->GetItemData(hParent);
		if ((pItemData != nullptr) && (pItemData->getType() == enumSTEPItemDataType::dtProductInstance))
		{
			pItemData->getInstance<CProductInstance>()->setEnable(true);
		}

		UpdateParent(m_pTreeView->GetParentItem(hParent));

		return;
	}

	m_pTreeView->SetItemImage(hParent, IMAGE_SEMI_SELECTED, IMAGE_SEMI_SELECTED);

	CSTEPItemData* pItemData = (CSTEPItemData*)m_pTreeView->GetItemData(hParent);
	if ((pItemData != nullptr) && (pItemData->getType() == enumSTEPItemDataType::dtProductInstance))
	{
		pItemData->getInstance<CProductInstance>()->setEnable(true);
	}

	UpdateParent(m_pTreeView->GetParentItem(hParent));
}

// ------------------------------------------------------------------------------------------------
void CSTEPProductsTreeView::ResetView()
{
	m_pTreeView->DeleteAllItems();

	for (size_t iItemData = 0; iItemData < m_vecItemData.size(); iItemData++)
	{
		delete m_vecItemData[iItemData];
	}
	m_vecItemData.clear();

	m_hSelectedItem = nullptr;

	m_mapInstance2Item.clear();

	m_pSearchDialog->Reset();

	LoadModel();
}