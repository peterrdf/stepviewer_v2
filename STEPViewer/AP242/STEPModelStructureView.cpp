#include "stdafx.h"
#include "mainfrm.h"
#include "STEPModelStructureView.h"
#include "ProductDefinition.h"
#include "Resource.h"
#include "STEPViewer.h"
#include "STEPModel.h"
#include "StructureViewConsts.h"

#include <algorithm>
#include <chrono>

using namespace std;

// ************************************************************************************************
// STEP Model Structure
// ************************************************************************************************

// ------------------------------------------------------------------------------------------------
CAP242ModelStructureView::CAP242ModelStructureView(CTreeCtrlEx* pTreeView)
	: CTreeViewBase()
	, m_pTreeCtrl(pTreeView)
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

	m_pTreeCtrl->SetImageList(m_pImageList, TVSIL_NORMAL);

	//  Search
	m_pSearchDialog = new CSearchTreeCtrlDialog(this);
	m_pSearchDialog->Create(IDD_DIALOG_SEARCH, m_pTreeCtrl);
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ CAP242ModelStructureView::~CAP242ModelStructureView()
{
	m_pTreeCtrl->SetImageList(nullptr, TVSIL_NORMAL);

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
/*virtual*/ void CAP242ModelStructureView::OnInstanceSelected(CViewBase* pSender) /*override*/
{
	if (pSender == this)
	{
		return;
	}

	if (m_hSelectedItem != nullptr)
	{
		m_pTreeCtrl->SetItemState(m_hSelectedItem, 0, TVIS_BOLD);
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
		HTREEITEM hModel = m_pTreeCtrl->GetChildItem(nullptr);
		ASSERT(hModel != nullptr);

		m_pTreeCtrl->SelectItem(hModel);

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
	m_pTreeCtrl->SendMessage(WM_SETREDRAW, 0, 0);

	m_hSelectedItem = itInstance2Item->second;

	m_pTreeCtrl->SetItemState(m_hSelectedItem, TVIS_BOLD, TVIS_BOLD);
	m_pTreeCtrl->EnsureVisible(m_hSelectedItem);
	m_pTreeCtrl->SelectItem(m_hSelectedItem);

	/*
	* Enable the drawing
	*/
	m_pTreeCtrl->SendMessage(WM_SETREDRAW, 1, 0);
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CAP242ModelStructureView::Load() /*override*/
{
	ResetView();
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ CImageList* CAP242ModelStructureView::GetImageList() const /*override*/
{
	return m_pImageList;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CAP242ModelStructureView::OnShowWindow(BOOL bShow, UINT /*nStatus*/) /*override*/
{
	if (!bShow)
	{
		m_pSearchDialog->ShowWindow(SW_HIDE);
	}
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CAP242ModelStructureView::OnTreeItemClick(NMHDR* /*pNMHDR*/, LRESULT* pResult) /*override*/
{
	*pResult = 0;

	if (m_bInitInProgress)
	{
		return;
	}

	DWORD dwPosition = GetMessagePos();
	CPoint point(LOWORD(dwPosition), HIWORD(dwPosition));
	m_pTreeCtrl->ScreenToClient(&point);

	UINT uFlags = 0;
	HTREEITEM hItem = m_pTreeCtrl->HitTest(point, &uFlags);

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

		int iImage, iSelectedImage = -1;
		m_pTreeCtrl->GetItemImage(hItem, iImage, iSelectedImage);

		ASSERT(iImage == iSelectedImage);

		switch (iImage)
		{
			case IMAGE_SELECTED:
			case IMAGE_SEMI_SELECTED:
			{
				auto pItemData = (CSTEPItemData*)m_pTreeCtrl->GetItemData(hItem);
				if ((pItemData != nullptr) && (pItemData->getType() == enumSTEPItemDataType::ProductInstance))
				{
					pItemData->GetProductInstance()->SetEnable(false);
				}
				
				UpdateChildrenInMemory(pItemData, false);
				
				m_pTreeCtrl->SetItemImage(hItem, IMAGE_NOT_SELECTED, IMAGE_NOT_SELECTED);

				UpdateChildren(hItem);
				UpdateParent(m_pTreeCtrl->GetParentItem(hItem));

				pController->OnInstancesEnabledStateChanged(this);
			}
			break;

			case IMAGE_NOT_SELECTED:
			{
				auto pItemData = (CSTEPItemData*)m_pTreeCtrl->GetItemData(hItem);
				if ((pItemData != nullptr) && (pItemData->getType() == enumSTEPItemDataType::ProductInstance))
				{
					pItemData->GetProductInstance()->SetEnable(true);
				}

				UpdateChildrenInMemory(pItemData, true);

				m_pTreeCtrl->SetItemImage(hItem, IMAGE_SELECTED, IMAGE_SELECTED);

				UpdateChildren(hItem);
				UpdateParent(m_pTreeCtrl->GetParentItem(hItem));

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
		if (m_pTreeCtrl->GetParentItem(m_hSelectedItem) != nullptr)
		{
			// keep the roots always bold
			m_pTreeCtrl->SetItemState(m_hSelectedItem, 0, TVIS_BOLD);
		}

		m_pTreeCtrl->SetItemState(hItem, TVIS_BOLD, TVIS_BOLD);
		m_hSelectedItem = hItem;

		auto pItemData = (CSTEPItemData*)m_pTreeCtrl->GetItemData(hItem);
		if ((pItemData == nullptr) || (pItemData->getType() != enumSTEPItemDataType::ProductInstance))
		{
			GetController()->SelectInstance(this, nullptr);

			return;
		}

		auto pInstance = pItemData->GetProductInstance();
		ASSERT(pInstance != nullptr);

		GetController()->SelectInstance(this, pInstance);
	} // if ((hItem != nullptr) && ...
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CAP242ModelStructureView::OnTreeItemExpanding(NMHDR* pNMHDR, LRESULT* pResult) /*override*/
{
	*pResult = 0;

	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	int iImage, iSelectedImage = -1;
	m_pTreeCtrl->GetItemImage(pNMTreeView->itemNew.hItem, iImage, iSelectedImage);

	ASSERT(iImage == iSelectedImage);

	if (pNMTreeView->itemNew.cChildren == 1)
	{
		HTREEITEM hChild = m_pTreeCtrl->GetChildItem(pNMTreeView->itemNew.hItem);
		if (hChild == nullptr)
		{
			ASSERT(FALSE); // Internal error

			return;
		}

		if (m_pTreeCtrl->GetItemText(hChild) != ITEM_PENDING_LOAD)
		{
			return;
		}

		m_pTreeCtrl->DeleteItem(hChild);

		auto pItemData = (CSTEPItemData*)m_pTreeCtrl->GetItemData(pNMTreeView->itemNew.hItem);
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
/*virtual*/ bool CAP242ModelStructureView::IsSelected(HTREEITEM /*hItem*/)
{
	ASSERT(FALSE); // TODO

	return false;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ CTreeCtrlEx* CAP242ModelStructureView::GetTreeView() /*override*/
{
	return m_pTreeCtrl;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ vector<CString> CAP242ModelStructureView::GetSearchFilters() /*override*/
{
	return vector<CString>
		{
			_T("(All)"),
			_T("Product Definitions"),
			_T("Assemblies"),
			_T("Product Instances")
		};
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CAP242ModelStructureView::LoadChildrenIfNeeded(HTREEITEM hItem) /*override*/
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
	
	HTREEITEM hChild = GetTreeView()->GetChildItem(hItem);
	if (hChild == NULL)
	{
		ASSERT(FALSE);

		return;
	}

	if (GetTreeView()->GetItemText(hChild) == ITEM_PENDING_LOAD)
	{
		GetTreeView()->Expand(hItem, TVE_EXPAND);
	}
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ BOOL CAP242ModelStructureView::ContainsText(int iFilter, HTREEITEM hItem, const CString& strText) /*override*/
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

	CSTEPItemData* pItemData = (CSTEPItemData*)GetTreeView()->GetItemData(hItem);

	// Product Definition
	if (iFilter == (int)enumSearchFilter::ProductDefitions)
	{
		if ((pItemData != nullptr) && (pItemData->getType() == enumSTEPItemDataType::ProductDefinition))
		{
			return strItemText.Find(strText, 0) != -1;
		}

		return FALSE;
	}

	// Assemblies
	if (iFilter == (int)enumSearchFilter::Assemblies)
	{
		if ((pItemData != nullptr) && (pItemData->getType() == enumSTEPItemDataType::Assembly))
		{
			return strItemText.Find(strText, 0) != -1;
		}

		return FALSE;
	}

	// Product Instance
	if (iFilter == (int)enumSearchFilter::ProductInstances)
	{
		if ((pItemData != nullptr) && (pItemData->getType() == enumSTEPItemDataType::ProductInstance))
		{
			return strItemText.Find(strText, 0) != -1;
		}

		return FALSE;
	}

	// All
	return strItemText.Find(strTextLower, 0) != -1;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CAP242ModelStructureView::OnContextMenu(CWnd* /*pWnd*/, CPoint point) /*override*/
{
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

	auto pModel = GetModel<CAP242Model>();
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	// Select clicked item:
	CPoint ptTree = point;
	m_pTreeCtrl->ScreenToClient(&ptTree);

	UINT flags = 0;
	HTREEITEM hItem = m_pTreeCtrl->HitTest(ptTree, &flags);
	if (hItem == nullptr)
	{
		return;
	}

	m_pTreeCtrl->SelectItem(hItem);
	m_pTreeCtrl->SetFocus();

	/*
	* Instances
	*/
	auto pItemData = (CSTEPItemData*)m_pTreeCtrl->GetItemData(hItem);
	if ((pItemData != nullptr) && (pItemData->getType() == enumSTEPItemDataType::ProductInstance))
	{
		CProductInstance * pInstance = pItemData->GetProductInstance();

		CMenu menu;
		VERIFY(menu.LoadMenuW(IDR_POPUP_INSTANCES));

		auto pPopup = menu.GetSubMenu(0);

		// Zoom to
		if (!pInstance->GetEnable())
		{
			pPopup->EnableMenuItem(ID_INSTANCES_ZOOM_TO, MF_BYCOMMAND | MF_DISABLED);
		}

		// Save
		if (!pInstance->GetEnable())
		{
			pPopup->EnableMenuItem(ID_INSTANCES_SAVE, MF_BYCOMMAND | MF_DISABLED);
		}

		if (pInstance->GetEnable())
		{
			pPopup->CheckMenuItem(ID_INSTANCES_ENABLE, MF_BYCOMMAND | MF_CHECKED);
		}

		UINT uiCommand = pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, point.x, point.y, m_pTreeCtrl);
		if (uiCommand == 0)
		{
			return;
		}

		auto& mapInstances = pModel->GetInstances();

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
				pController->SaveOwlInstance();
			}
			break;

			case ID_INSTANCES_ENABLE:
			{				
				pInstance->SetEnable(!pInstance->GetEnable());

				int iImage = pInstance->GetEnable() ? IMAGE_SELECTED : IMAGE_NOT_SELECTED;
				m_pTreeCtrl->SetItemImage(hItem, iImage, iImage);
				
				UpdateChildrenInMemory(pItemData, pInstance->GetEnable());
				
				UpdateChildren(hItem);
				UpdateParent(m_pTreeCtrl->GetParentItem(hItem));
								
				pController->OnInstancesEnabledStateChanged(this);
			}
			break;

			case ID_INSTANCES_DISABLE_ALL_BUT_THIS:
			{
				auto itInstance = mapInstances.begin();
				for (; itInstance != mapInstances.end(); itInstance++)
				{
					itInstance->second->SetEnable(itInstance->second == pInstance);
				}

				ASSERT(pInstance->GetEnable());
				
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

				m_pTreeCtrl->SetItemImage(itInstance2Item->second, IMAGE_SELECTED, IMAGE_SELECTED);
				
				UpdateChildren(itInstance2Item->second);
				UpdateParent(m_pTreeCtrl->GetParentItem(itInstance2Item->second));

				pController->OnInstancesEnabledStateChanged(this);
			}
			break;

			case ID_INSTANCES_ENABLE_ALL:
			{
				auto itInstance = mapInstances.begin();
				for (; itInstance != mapInstances.end(); itInstance++)
				{
					itInstance->second->SetEnable(true);
				}

				ResetTree(true);

				pController->OnInstancesEnabledStateChanged(this);
			}
			break;

			case IDS_VIEW_IFC_RELATIONS:
			{
				pController->OnViewRelations(this, pInstance->GetSdaiInstance());
			}
			break;

			default:
			{
				ASSERT(FALSE);
			}
			break;
		} // switch (uiCommand)
	} // if ((pItemData != nullptr) && ...
	else
	{
		CMenu menu;
		VERIFY(menu.LoadMenuW(IDR_POPUP_META_DATA));

		auto pPopup = menu.GetSubMenu(0);

		UINT uiCommand = pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, point.x, point.y, m_pTreeCtrl);
		if (uiCommand == 0)
		{
			return;
		}

		switch (uiCommand)
		{
			case ID_VIEW_ZOOM_OUT:
			{
				pController->ZoomOut();
			}
			break;

			default:
			{
				ASSERT(FALSE);
			}
			break;
		}
	} // else if ((pItemData != nullptr) && ...
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CAP242ModelStructureView::OnSearch() /*override*/
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
void CAP242ModelStructureView::LoadHeaderDescription(HTREEITEM hParent)
{
	auto pModel = GetModel<CAP242Model>();
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

		HTREEITEM hDescriptions = m_pTreeCtrl->InsertItem(&tvInsertStruct);

		int_t iItem = 0;
		if (!GetSPFFHeaderItem(pModel->GetSdaiModel(), 0, iItem, sdaiUNICODE, (char**)&szText))
		{
			while (!GetSPFFHeaderItem(pModel->GetSdaiModel(), 0, iItem++, sdaiUNICODE, (char**)&szText))
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
		GetSPFFHeaderItem(pModel->GetSdaiModel(), 1, 0, sdaiUNICODE, (char**)&szText);

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

		m_pTreeCtrl->InsertItem(&tvInsertStruct);
	}

	/*
	* Name
	*/
	{
		GetSPFFHeaderItem(pModel->GetSdaiModel(), 2, 0, sdaiUNICODE, (char**)&szText);

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

		m_pTreeCtrl->InsertItem(&tvInsertStruct);
	}

	/*
	* TimeStamp
	*/
	{
		GetSPFFHeaderItem(pModel->GetSdaiModel(), 3, 0, sdaiUNICODE, (char**)&szText);

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

		m_pTreeCtrl->InsertItem(&tvInsertStruct);
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

		HTREEITEM hDescriptions = m_pTreeCtrl->InsertItem(&tvInsertStruct);

		int_t iItem = 0;
		if (!GetSPFFHeaderItem(pModel->GetSdaiModel(), 4, iItem, sdaiUNICODE, (char**)&szText))
		{
			while (!GetSPFFHeaderItem(pModel->GetSdaiModel(), 4, iItem++, sdaiUNICODE, (char**)&szText))
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
		tvInsertStruct.hParent = hParent;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = L"Organizations";
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY_SET;
		tvInsertStruct.item.lParam = NULL;

		HTREEITEM hDescriptions = m_pTreeCtrl->InsertItem(&tvInsertStruct);

		int_t iItem = 0;
		if (!GetSPFFHeaderItem(pModel->GetSdaiModel(), 5, iItem, sdaiUNICODE, (char**)&szText))
		{
			while (!GetSPFFHeaderItem(pModel->GetSdaiModel(), 5, iItem++, sdaiUNICODE, (char**)&szText))
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
		GetSPFFHeaderItem(pModel->GetSdaiModel(), 6, 0, sdaiUNICODE, (char**)&szText);

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

		m_pTreeCtrl->InsertItem(&tvInsertStruct);
	}

	/*
	* OriginatingSystem
	*/
	{
		GetSPFFHeaderItem(pModel->GetSdaiModel(), 7, 0, sdaiUNICODE, (char**)&szText);

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

		m_pTreeCtrl->InsertItem(&tvInsertStruct);
	}

	/*
	* Authorization
	*/
	{
		GetSPFFHeaderItem(pModel->GetSdaiModel(), 8, 0, sdaiUNICODE, (char**)&szText);

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

		m_pTreeCtrl->InsertItem(&tvInsertStruct);
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

		HTREEITEM hDescriptions = m_pTreeCtrl->InsertItem(&tvInsertStruct);

		int_t iItem = 0;
		if (!GetSPFFHeaderItem(pModel->GetSdaiModel(), 9, iItem, sdaiUNICODE, (char**)&szText))
		{
			while (!GetSPFFHeaderItem(pModel->GetSdaiModel(), 9, iItem++, sdaiUNICODE, (char**)&szText))
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
void CAP242ModelStructureView::LoadModel()
{
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	m_bInitInProgress = true;

	auto pModel = GetModel<CAP242Model>();
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto& mapDefinitions = pModel->GetDefinitions();
	if (mapDefinitions.empty())
	{
		return;
	}

	/*
	* Header
	*/
	HTREEITEM hHeader = m_pTreeCtrl->InsertItem(_T("Header"), IMAGE_PROPERTY_SET, IMAGE_PROPERTY_SET);
	LoadHeaderDescription(hHeader);

	/*
	* Model
	*/
	auto pModelItemData = new CSTEPItemData(nullptr, pModel, enumSTEPItemDataType::Model);
	m_vecItemData.push_back(pModelItemData);

	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = nullptr;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
	tvInsertStruct.item.pszText = _T("Model");
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_SELECTED;
	tvInsertStruct.item.cChildren = !mapDefinitions.empty() ? 1 : 0;
	tvInsertStruct.item.lParam = (LPARAM)pModelItemData;

	HTREEITEM hModel = m_pTreeCtrl->InsertItem(&tvInsertStruct);
	pModelItemData->treeItem() = hModel;

	/*
	* Roots
	*/
	auto itDefinition = mapDefinitions.begin();
	for (; itDefinition != mapDefinitions.end(); itDefinition++)
	{
		auto pDefinition = itDefinition->second;

		if (pDefinition->GetRelatedProducts() == 0)
		{
			LoadProductDefinitionsInMemory(pModel, pDefinition, pModelItemData);
		}
	}

	// Check for descendants with Geometry
	SearchForDescendantWithGeometry();

	/* Update Model */

	bool bHasDescendantWithGeometry = false;
	for (auto pItemData : m_vecItemData)
	{
		if (pItemData->hasDescendantWithGeometry())
		{
			bHasDescendantWithGeometry = true;

			break;
		}
	}

	if (!bHasDescendantWithGeometry)
	{
		m_pTreeCtrl->SetItemImage(hModel, IMAGE_NO_GEOMETRY, IMAGE_NO_GEOMETRY);
	}

	m_pTreeCtrl->InsertItem(ITEM_PENDING_LOAD, IMAGE_SELECTED, IMAGE_SELECTED, hModel);

	m_bInitInProgress = false;

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	TRACE(L"\n*** CAP242ModelStructureView::LoadModel() : %lld [µs]", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());
}

// ------------------------------------------------------------------------------------------------
void CAP242ModelStructureView::WalkAssemblyTreeRecursively(CAP242Model* pModel, CProductDefinition* pDefinition, HTREEITEM hParent)
{
	const auto& mapAssemblies = pModel->GetAssemblies();

	auto itAssembly = mapAssemblies.begin();
	for (; itAssembly != mapAssemblies.end(); itAssembly++)
	{
		auto pAssembly = itAssembly->second;

		if (pAssembly->GetRelatingProductDefinition() == pDefinition)
		{
			/*
			* Assembly
			*/
			CString strName = pAssembly->GetId();
			strName += ITEM_ASSEMBLY;

			HTREEITEM hAssembly = m_pTreeCtrl->InsertItem(strName, IMAGE_SELECTED, IMAGE_SELECTED, hParent);
			m_pTreeCtrl->InsertItem(ITEM_GEOMETRY, IMAGE_NO_GEOMETRY, IMAGE_NO_GEOMETRY, hAssembly);

			auto pItemData = new CSTEPItemData(nullptr, pAssembly, enumSTEPItemDataType::Assembly);
			m_vecItemData.push_back(pItemData);

			m_pTreeCtrl->SetItemData(hAssembly, (DWORD_PTR)pItemData);

			/*
			* Instance
			*/
			if (pAssembly->GetRelatedProductDefinition()->GetRelatingProducts() == 0)
			{
				auto& vecInstances = pAssembly->GetRelatedProductDefinition()->GetInstances();
				int32_t iInstance = pAssembly->GetRelatedProductDefinition()->GetNextInstance();

				strName = pAssembly->GetRelatedProductDefinition()->GetId();
				strName += ITEM_PRODUCT_INSTANCE;

				HTREEITEM hInstance = m_pTreeCtrl->InsertItem(strName, IMAGE_SELECTED, IMAGE_SELECTED, hAssembly);

				pItemData = new CSTEPItemData(nullptr, vecInstances[iInstance], enumSTEPItemDataType::ProductInstance);
				m_vecItemData.push_back(pItemData);

				m_pTreeCtrl->SetItemData(hInstance, (DWORD_PTR)pItemData);

				int iImage = vecInstances[iInstance]->GetEnable() ? IMAGE_SELECTED : IMAGE_NOT_SELECTED;
				HTREEITEM hGeometry = m_pTreeCtrl->InsertItem(ITEM_GEOMETRY, iImage, iImage, hInstance);
				m_pTreeCtrl->SetItemData(hGeometry, (DWORD_PTR)pItemData);

				ASSERT(m_mapInstance2Item.find(vecInstances[iInstance]) == m_mapInstance2Item.end());
				m_mapInstance2Item[vecInstances[iInstance]] = hInstance;
			} // if (pAssembly->GetRelatedProductDefinition()->GetRelatingProductRefs() == 0)
			else
			{
				/*
				* Product
				*/
				strName = pDefinition->GetId();
				strName += ITEM_PRODUCT_DEFINION;

				HTREEITEM hProductDefinition = m_pTreeCtrl->InsertItem(strName, IMAGE_SELECTED, IMAGE_SELECTED, hAssembly);
				m_pTreeCtrl->InsertItem(ITEM_GEOMETRY, IMAGE_NO_GEOMETRY, IMAGE_NO_GEOMETRY, hProductDefinition);

				pItemData = new CSTEPItemData(nullptr, pDefinition, enumSTEPItemDataType::ProductDefinition);
				m_vecItemData.push_back(pItemData);

				m_pTreeCtrl->SetItemData(hProductDefinition, (DWORD_PTR)pItemData);

				WalkAssemblyTreeRecursively(pModel, pAssembly->GetRelatedProductDefinition(), hProductDefinition);
			} // else if (pAssembly->GetRelatedProductDefinition()->GetRelatingProductRefs() == 0)			
		} // if (pAssembly->m_pRelatingProductDefinition == ...
	} // for (; itAssembly != ...	
}

// ------------------------------------------------------------------------------------------------
void CAP242ModelStructureView::LoadProductDefinitionsInMemory(CAP242Model* pModel, CProductDefinition* pDefinition, CSTEPItemData* pParent)
{
	/*
	* Instance
	*/
	if (pDefinition->GetRelatingProducts() > 0)
	{
		auto pProductItemData = new CSTEPItemData(pParent, pDefinition, enumSTEPItemDataType::ProductDefinition);
		pParent->children().push_back(pProductItemData);

		m_vecItemData.push_back(pProductItemData);

		WalkAssemblyTreeRecursivelyInMemory(pModel, pDefinition, pProductItemData);
	} // if (pDefinition->GetRelatingProductRefs() > 0)	
	else
	{
		auto& vecInstances = pDefinition->GetInstances();
		int32_t iInstance = pDefinition->GetNextInstance();

		auto pProductInstanceData = new CSTEPItemData(pParent, vecInstances[iInstance], enumSTEPItemDataType::ProductInstance);
		pParent->children().push_back(pProductInstanceData);

		m_vecItemData.push_back(pProductInstanceData);
	} // else if (pDefinition->GetRelatingProductRefs() > 0)
}

// ------------------------------------------------------------------------------------------------
void CAP242ModelStructureView::WalkAssemblyTreeRecursivelyInMemory(CAP242Model* pModel, CProductDefinition* pDefinition, CSTEPItemData* pParent)
{
	const auto& mapAssemblies = pModel->GetAssemblies();

	auto itAssembly = mapAssemblies.begin();
	for (; itAssembly != mapAssemblies.end(); itAssembly++)
	{
		auto pAssembly = itAssembly->second;

		if (pAssembly->GetRelatingProductDefinition() == pDefinition)
		{
			/*
			* Assembly
			*/
			auto pAssemblyItemData = new CSTEPItemData(pParent, pAssembly, enumSTEPItemDataType::Assembly);
			pParent->children().push_back(pAssemblyItemData);

			m_vecItemData.push_back(pAssemblyItemData);

			/*
			* Instance
			*/
			if (pAssembly->GetRelatedProductDefinition()->GetRelatingProducts() == 0)
			{
				auto& vecInstances = pAssembly->GetRelatedProductDefinition()->GetInstances();
				int32_t iInstance = pAssembly->GetRelatedProductDefinition()->GetNextInstance();

				auto pInstanceItemData = new CSTEPItemData(pAssemblyItemData, vecInstances[iInstance], enumSTEPItemDataType::ProductInstance);
				pAssemblyItemData->children().push_back(pInstanceItemData);

				m_vecItemData.push_back(pInstanceItemData);
			} // if (pAssembly->GetRelatedProductDefinition()->GetRelatingProductRefs() == 0)
			else
			{
				/*
				* Product
				*/
				auto pProductItemData = new CSTEPItemData(pAssemblyItemData, pDefinition, enumSTEPItemDataType::ProductDefinition);
				pAssemblyItemData->children().push_back(pProductItemData);

				m_vecItemData.push_back(pProductItemData);

				WalkAssemblyTreeRecursivelyInMemory(pModel, pAssembly->GetRelatedProductDefinition(), pProductItemData);
			} // else if (pAssembly->GetRelatedProductDefinition()->GetRelatingProductRefs() == 0)			
		} // if (pAssembly->m_pRelatingProductDefinition == ...
	} // for (; itAssembly != ...	

	auto& vecInstances = pDefinition->GetInstances();
	int32_t iInstance = pDefinition->GetNextInstance();

	auto pInstanceItemData = new CSTEPItemData(pParent, vecInstances[iInstance], enumSTEPItemDataType::ProductInstance);
	pParent->children().push_back(pInstanceItemData);

	m_vecItemData.push_back(pInstanceItemData);
}

void CAP242ModelStructureView::SearchForDescendantWithGeometry()
{
	for (auto pItemData : m_vecItemData)
	{
		bool bHasDescendantWithGeometry = false;

		if (pItemData->getType() == enumSTEPItemDataType::ProductInstance)
		{
			auto pProductInstance = pItemData->GetProductInstance();

			bHasDescendantWithGeometry |= pProductInstance->HasGeometry();
		}
		
		if (!bHasDescendantWithGeometry)
		{
			for (auto pChildItemData : pItemData->children())
			{
				if (pChildItemData->getType() == enumSTEPItemDataType::ProductInstance)
				{
					CProductInstance* pProductInstance = pChildItemData->GetProductInstance();

					bHasDescendantWithGeometry |= pProductInstance->HasGeometry();
					if (bHasDescendantWithGeometry)
					{
						break;
					}
				}

				SearchForDescendantWithGeometryRecursively(pChildItemData, bHasDescendantWithGeometry);
				if (bHasDescendantWithGeometry)
				{
					break;
				}
			} // for (auto pChildItemData : ...
		} // if (!bHasDescendantWithGeometry)		

		pItemData->hasDescendantWithGeometry() = bHasDescendantWithGeometry;
	} // for (auto pItemData : ...
}

void CAP242ModelStructureView::SearchForDescendantWithGeometryRecursively(CSTEPItemData* pItemData, bool& bHasDescendantWithGeometry)
{
	for (auto pChildItemData : pItemData->children())
	{
		if (pChildItemData->getType() == enumSTEPItemDataType::ProductInstance)
		{
			CProductInstance* pProductInstance = pChildItemData->GetProductInstance();

			bHasDescendantWithGeometry |= pProductInstance->HasGeometry();
			if (bHasDescendantWithGeometry)
			{
				break;
			}
		}

		SearchForDescendantWithGeometryRecursively(pChildItemData, bHasDescendantWithGeometry);
		if (bHasDescendantWithGeometry)
		{
			break;
		}
	} // for (auto pChildItemData : ...
}

// ------------------------------------------------------------------------------------------------
CSTEPItemData* CAP242ModelStructureView::FindItemData(CProductInstance* pInstance)
{
	for (size_t iItemData = 0; iItemData < m_vecItemData.size(); iItemData++)
	{
		if (m_vecItemData[iItemData]->GetProductInstance() == pInstance)
		{
			return m_vecItemData[iItemData];
		}
	}

	return nullptr;
}

// ------------------------------------------------------------------------------------------------
void CAP242ModelStructureView::LoadItemChildren(CSTEPItemData* pItemData)
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
			case enumSTEPItemDataType::ProductDefinition:
			{
				CProductDefinition* pDefinition = pChild->GetProductDefinition();
				ASSERT(pDefinition != nullptr);
				ASSERT(pDefinition->GetId() != nullptr);
				ASSERT(pDefinition->GetProductName() != nullptr);

				strName.Format(L"#%lld %s %s", pDefinition->GetExpressID(), pDefinition->GetProductName(), ITEM_PRODUCT_DEFINION);
			}
			break;

			case enumSTEPItemDataType::Assembly:
			{
				auto pAssembly = pChild->GetAssembly();
				ASSERT(pAssembly != nullptr);
				ASSERT(pAssembly->GetId() != nullptr);
				ASSERT(pAssembly->GetName() != nullptr);

				strName.Format(L"#%lld %s %s", pAssembly->GetExpressID(), pAssembly->GetName(), ITEM_ASSEMBLY);
			}
			break;

			case enumSTEPItemDataType::ProductInstance:
			{
				CProductInstance* pInstance = pChild->GetProductInstance();
				ASSERT(pInstance != nullptr);
				ASSERT(pInstance->GetName() != L"");

				strName.Format(L"%s %s", pInstance->GetName().c_str(), ITEM_PRODUCT_INSTANCE);

				iGeometryImage = pInstance->GetProductDefinition()->hasGeometry() ? IMAGE_SELECTED : IMAGE_NO_GEOMETRY;
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
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = 
			pItemData->hasDescendantWithGeometry() ? IMAGE_SELECTED : IMAGE_NO_GEOMETRY;
		tvInsertStruct.item.cChildren = 1;
		tvInsertStruct.item.lParam = (LPARAM)pChild;

		HTREEITEM hChild = m_pTreeCtrl->InsertItem(&tvInsertStruct);
		pChild->treeItem() = hChild;

		if (!pChild->children().empty())
		{
			m_pTreeCtrl->InsertItem(ITEM_PENDING_LOAD, IMAGE_SELECTED, IMAGE_SELECTED, hChild);
		}

		HTREEITEM hGeometry = m_pTreeCtrl->InsertItem(ITEM_GEOMETRY, iGeometryImage, iGeometryImage, hChild);

		if (pChild->getType() == enumSTEPItemDataType::ProductInstance)
		{
			m_pTreeCtrl->SetItemData(hGeometry, (DWORD_PTR)pChild);

			ASSERT(m_mapInstance2Item.find(pChild->GetProductInstance()) == m_mapInstance2Item.end());
			m_mapInstance2Item[pChild->GetProductInstance()] = hChild;
		}
	} // for (size_t iChild = ...
}

// ------------------------------------------------------------------------------------------------
void CAP242ModelStructureView::LoadInstanceAncestors(CProductInstance* pInstance)
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
		m_pTreeCtrl->Expand(vecAncestors[iAncestor]->treeItem(), TVE_EXPAND);
	}
}

// ------------------------------------------------------------------------------------------------
void CAP242ModelStructureView::ResetTree(bool bEnable)
{
	HTREEITEM hRoot = m_pTreeCtrl->GetRootItem();
	while (hRoot != nullptr)
	{
		int iImage, iSelectedImage = -1;
		m_pTreeCtrl->GetItemImage(hRoot, iImage, iSelectedImage);

		ASSERT(iImage == iSelectedImage);

		if ((iImage != IMAGE_SELECTED) && (iImage != IMAGE_SEMI_SELECTED) && (iImage != IMAGE_NOT_SELECTED))
		{
			// skip the Header and unreferenced items
			hRoot = m_pTreeCtrl->GetNextSiblingItem(hRoot);

			continue;
		}

		ResetTree(hRoot, bEnable);

		hRoot = m_pTreeCtrl->GetNextSiblingItem(hRoot);
	}
}

// ------------------------------------------------------------------------------------------------
void CAP242ModelStructureView::ResetTree(HTREEITEM hParent, bool bEnable)
{
	if (hParent == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	HTREEITEM hChild = m_pTreeCtrl->GetNextItem(hParent, TVGN_CHILD);
	while (hChild != nullptr)
	{
		ResetTree(hChild, bEnable);

		hChild = m_pTreeCtrl->GetNextSiblingItem(hChild);
	} // while (hChild != nullptr)

	int iParentImage = -1;
	int iParentSelectedImage = -1;
	m_pTreeCtrl->GetItemImage(hParent, iParentImage, iParentSelectedImage);

	ASSERT(iParentImage == iParentSelectedImage);

	if ((iParentImage == IMAGE_SELECTED) || (iParentImage == IMAGE_SEMI_SELECTED) || (iParentImage == IMAGE_NOT_SELECTED))
	{
		int iImage = bEnable ? IMAGE_SELECTED : IMAGE_NOT_SELECTED;
		m_pTreeCtrl->SetItemImage(hParent, iImage, iImage);
	}
}

// ------------------------------------------------------------------------------------------------
void CAP242ModelStructureView::UpdateChildren(HTREEITEM hParent)
{
	if (hParent == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

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

#ifdef _DEBUG
		auto pItemData = (CSTEPItemData*)m_pTreeCtrl->GetItemData(hChild);
		if ((pItemData != nullptr) && (pItemData->getType() == enumSTEPItemDataType::ProductInstance))
		{
			if ((iParentImage == IMAGE_SELECTED) || (iParentImage == IMAGE_SEMI_SELECTED))
			{
				ASSERT(pItemData->GetProductInstance()->GetEnable());
			}
			else
			{
				ASSERT(!pItemData->GetProductInstance()->GetEnable());
			}
		}
#endif

		UpdateChildren(hChild);

		hChild = m_pTreeCtrl->GetNextSiblingItem(hChild);
	} // while (hChild != nullptr)
}

// ------------------------------------------------------------------------------------------------
void CAP242ModelStructureView::UpdateChildrenInMemory(CSTEPItemData* pParent, bool bEnable)
{
	if (pParent == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	for (size_t iChild = 0; iChild < pParent->children().size(); iChild++)
	{
		auto pChild = pParent->children()[iChild];
		if (pChild->getType() == enumSTEPItemDataType::ProductInstance)
		{
			pChild->GetProductInstance()->SetEnable(bEnable);
		}

		UpdateChildrenInMemory(pChild, bEnable);
	} // for (size_t iChild = ...
}

// ------------------------------------------------------------------------------------------------
void CAP242ModelStructureView::UpdateParent(HTREEITEM hParent)
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

		CSTEPItemData* pItemData = (CSTEPItemData*)m_pTreeCtrl->GetItemData(hParent);
		if ((pItemData != nullptr) && (pItemData->getType() == enumSTEPItemDataType::ProductInstance))
		{
			pItemData->GetProductInstance()->SetEnable(true);
		}

		UpdateParent(m_pTreeCtrl->GetParentItem(hParent));

		return;
	}

	if (iSelectedChidlrenCount == 0)
	{
		m_pTreeCtrl->SetItemImage(hParent, IMAGE_NOT_SELECTED, IMAGE_NOT_SELECTED);

		CSTEPItemData* pItemData = (CSTEPItemData*)m_pTreeCtrl->GetItemData(hParent);
		if ((pItemData != nullptr) && (pItemData->getType() == enumSTEPItemDataType::ProductInstance))
		{
			pItemData->GetProductInstance()->SetEnable(false);
		}

		UpdateParent(m_pTreeCtrl->GetParentItem(hParent));

		return;
	}

	if (iSelectedChidlrenCount == iChidlrenCount)
	{
		m_pTreeCtrl->SetItemImage(hParent, IMAGE_SELECTED, IMAGE_SELECTED);

		CSTEPItemData* pItemData = (CSTEPItemData*)m_pTreeCtrl->GetItemData(hParent);
		if ((pItemData != nullptr) && (pItemData->getType() == enumSTEPItemDataType::ProductInstance))
		{
			pItemData->GetProductInstance()->SetEnable(true);
		}

		UpdateParent(m_pTreeCtrl->GetParentItem(hParent));

		return;
	}

	m_pTreeCtrl->SetItemImage(hParent, IMAGE_SEMI_SELECTED, IMAGE_SEMI_SELECTED);

	CSTEPItemData* pItemData = (CSTEPItemData*)m_pTreeCtrl->GetItemData(hParent);
	if ((pItemData != nullptr) && (pItemData->getType() == enumSTEPItemDataType::ProductInstance))
	{
		pItemData->GetProductInstance()->SetEnable(true);
	}

	UpdateParent(m_pTreeCtrl->GetParentItem(hParent));
}

// ------------------------------------------------------------------------------------------------
void CAP242ModelStructureView::ResetView()
{
	m_pTreeCtrl->DeleteAllItems();

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