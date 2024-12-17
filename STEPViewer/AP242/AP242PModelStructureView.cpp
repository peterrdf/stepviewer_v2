#include "stdafx.h"

#include "_ptr.h"

#include "mainfrm.h"
#include "AP242PModelStructureView.h"
#include "AP242ProductDefinition.h"
#include "Resource.h"
#include "STEPViewer.h"
#include "AP242Model.h"
#include "StructureViewConsts.h"

#include <algorithm>
#include <chrono>
using namespace std;

// ************************************************************************************************
CAP242PModelStructureView::CAP242PModelStructureView(CTreeCtrlEx* pTreeView)
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

/*virtual*/ CAP242PModelStructureView::~CAP242PModelStructureView()
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

/*virtual*/ void CAP242PModelStructureView::onInstanceSelected(_view* pSender) /*override*/
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

	auto pController = getController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pSelectedInstance = pController->getSelectedInstance() != nullptr ? dynamic_cast<CAP242ProductInstance*>(getController()->getSelectedInstance()) : nullptr;
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
		ASSERT(FALSE);

		return;
	}

	/*
	* Disable the drawing
	*/
	m_pTreeCtrl->SendMessage(WM_SETREDRAW, 0, 0);

	ASSERT(!itInstance2Item->second.empty());
	m_hSelectedItem = itInstance2Item->second[0];

	m_pTreeCtrl->SetItemState(m_hSelectedItem, TVIS_BOLD, TVIS_BOLD);
	m_pTreeCtrl->EnsureVisible(m_hSelectedItem);
	m_pTreeCtrl->SelectItem(m_hSelectedItem);

	/*
	* Enable the drawing
	*/
	m_pTreeCtrl->SendMessage(WM_SETREDRAW, 1, 0);
}

/*virtual*/ void CAP242PModelStructureView::Load() /*override*/
{
	ResetView();
}

/*virtual*/ CImageList* CAP242PModelStructureView::GetImageList() const /*override*/
{
	return m_pImageList;
}

/*virtual*/ void CAP242PModelStructureView::OnShowWindow(BOOL bShow, UINT /*nStatus*/) /*override*/
{
	if (!bShow)
	{
		m_pSearchDialog->ShowWindow(SW_HIDE);
	}
}

/*virtual*/ void CAP242PModelStructureView::OnTreeItemClick(NMHDR* /*pNMHDR*/, LRESULT* pResult) /*override*/
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

	ASSERT(getController() != nullptr);

	/*
	* TVHT_ONITEMICON
	*/
	if ((hItem != nullptr) && ((uFlags & TVHT_ONITEMICON) == TVHT_ONITEMICON))
	{
		auto pController = getController();
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
				auto pItemData = (CAP242ItemData*)m_pTreeCtrl->GetItemData(hItem);
				if ((pItemData != nullptr) && (pItemData->getType() == enumSTEPItemDataType::ProductInstance))
				{
					pItemData->GetInstance<CAP242ProductInstance>()->setEnable(false);
				}
				
				UpdateChildrenInMemory(pItemData, false);
				
				m_pTreeCtrl->SetItemImage(hItem, IMAGE_NOT_SELECTED, IMAGE_NOT_SELECTED);

				UpdateChildren(hItem);
				UpdateParent(m_pTreeCtrl->GetParentItem(hItem));

				pController->onInstancesEnabledStateChanged(this);
			}
			break;

			case IMAGE_NOT_SELECTED:
			{
				auto pItemData = (CAP242ItemData*)m_pTreeCtrl->GetItemData(hItem);
				if ((pItemData != nullptr) && (pItemData->getType() == enumSTEPItemDataType::ProductInstance))
				{
					pItemData->GetInstance<CAP242ProductInstance>()->setEnable(true);
				}

				UpdateChildrenInMemory(pItemData, true);

				m_pTreeCtrl->SetItemImage(hItem, IMAGE_SELECTED, IMAGE_SELECTED);

				UpdateChildren(hItem);
				UpdateParent(m_pTreeCtrl->GetParentItem(hItem));

				pController->onInstancesEnabledStateChanged(this);
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

		auto pItemData = (CAP242ItemData*)m_pTreeCtrl->GetItemData(hItem);
		if ((pItemData == nullptr) || (pItemData->getType() != enumSTEPItemDataType::ProductInstance))
		{
			getController()->selectInstance(this, nullptr);

			return;
		}

		auto pInstance = pItemData->GetInstance<CAP242ProductInstance>();
		ASSERT(pInstance != nullptr);

		getController()->selectInstance(this, pInstance);
	} // if ((hItem != nullptr) && ...
}

/*virtual*/ void CAP242PModelStructureView::OnTreeItemExpanding(NMHDR* pNMHDR, LRESULT* pResult) /*override*/
{
	*pResult = 0;
}

/*virtual*/ bool CAP242PModelStructureView::IsSelected(HTREEITEM /*hItem*/)
{
	ASSERT(FALSE); // TODO

	return false;
}

/*virtual*/ CTreeCtrlEx* CAP242PModelStructureView::GetTreeView() /*override*/
{
	return m_pTreeCtrl;
}

/*virtual*/ vector<CString> CAP242PModelStructureView::GetSearchFilters() /*override*/
{
	return vector<CString>
		{
			_T("(All)"),
			_T("Product Definitions"),
			_T("Assemblies"),
			_T("Product Instances")
		};
}

/*virtual*/ void CAP242PModelStructureView::LoadChildrenIfNeeded(HTREEITEM hItem) /*override*/
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

/*virtual*/ BOOL CAP242PModelStructureView::ContainsText(int iFilter, HTREEITEM hItem, const CString& strText) /*override*/
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

	CAP242ItemData* pItemData = (CAP242ItemData*)GetTreeView()->GetItemData(hItem);

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

/*virtual*/ void CAP242PModelStructureView::OnContextMenu(CWnd* /*pWnd*/, CPoint point) /*override*/
{
	if (point == CPoint(-1, -1))
	{
		return;
	}

	auto pController = getController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pModel = getModelAs<CAP242Model>();
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
	auto pItemData = (CAP242ItemData*)m_pTreeCtrl->GetItemData(hItem);
	if ((pItemData != nullptr) && (pItemData->getType() == enumSTEPItemDataType::ProductInstance))
	{
		auto pTargetInstance = pItemData->GetInstance<CAP242ProductInstance>();

		CMenu menu;
		VERIFY(menu.LoadMenuW(IDR_POPUP_INSTANCES));

		auto pPopup = menu.GetSubMenu(0);

		// Zoom to
		if (!pTargetInstance->getEnable())
		{
			pPopup->EnableMenuItem(ID_INSTANCES_ZOOM_TO, MF_BYCOMMAND | MF_DISABLED);
		}

		// Save
		if (!pTargetInstance->getEnable())
		{
			pPopup->EnableMenuItem(ID_INSTANCES_SAVE, MF_BYCOMMAND | MF_DISABLED);
		}

		// Enable
		if (pTargetInstance->getEnable())
		{
			pPopup->CheckMenuItem(ID_INSTANCES_ENABLE, MF_BYCOMMAND | MF_CHECKED);
		}

		UINT uiCommand = pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, point.x, point.y, m_pTreeCtrl);
		if (uiCommand == 0)
		{
			return;
		}

		auto& vecInstances = pModel->getInstances();

		switch (uiCommand)
		{
			case ID_INSTANCES_ZOOM_TO:
			{
				pController->zoomToInstance();
			}
			break;

			case ID_VIEW_ZOOM_OUT:
			{
				pController->zoomOut();
			}
			break;

			case ID_INSTANCES_SAVE:
			{
				pController->saveInstance();
			}
			break;

			case ID_INSTANCES_ENABLE:
			{				
				pTargetInstance->setEnable(!pTargetInstance->getEnable());

				int iImage = pTargetInstance->getEnable() ? IMAGE_SELECTED : IMAGE_NOT_SELECTED;
				m_pTreeCtrl->SetItemImage(hItem, iImage, iImage);
				
				UpdateChildrenInMemory(pItemData, pTargetInstance->getEnable());
				
				UpdateChildren(hItem);
				UpdateParent(m_pTreeCtrl->GetParentItem(hItem));
								
				pController->onInstancesEnabledStateChanged(this);
			}
			break;

			case ID_INSTANCES_DISABLE_ALL_BUT_THIS:
			{
				for (auto pInstance : vecInstances)
				{
					pInstance->setEnable(pTargetInstance == pInstance);
				}

				ASSERT(pTargetInstance->getEnable());
				
				ResetTree(false);

				auto itInstance2Item = m_mapInstance2Item.find(pTargetInstance);
				if (itInstance2Item == m_mapInstance2Item.end())
				{
					ASSERT(FALSE);

					return;
				}

				ASSERT(!itInstance2Item->second.empty());
				for (auto hTreeitem : itInstance2Item->second)
				{
					m_pTreeCtrl->SetItemImage(hTreeitem, IMAGE_SELECTED, IMAGE_SELECTED);

					UpdateChildren(hTreeitem);
					UpdateParent(m_pTreeCtrl->GetParentItem(hTreeitem));
				}

				pController->onInstancesEnabledStateChanged(this);
			}
			break;

			case ID_INSTANCES_ENABLE_ALL:
			{
				for (auto pInstance : vecInstances)
				{
					pInstance->setEnable(true);
				}

				ResetTree(true);

				pController->onInstancesEnabledStateChanged(this);
			}
			break;

			case IDS_VIEW_IFC_RELATIONS:
			{
				pController->onViewRelations(this, pTargetInstance->getSdaiInstance());
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
				pController->zoomOut();
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

/*virtual*/ void CAP242PModelStructureView::OnSearch() /*override*/
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

void CAP242PModelStructureView::LoadModel()
{
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	auto pModel = getModelAs<CAP242Model>();
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	if (pModel->getGeometries().empty())
	{
		return;
	}

	m_bInitInProgress = true;

	/*
	* Model
	*/
	auto pModelItemData = new CAP242ItemData(nullptr, (int64_t*)pModel, enumSTEPItemDataType::Model);
	m_vecItemData.push_back(pModelItemData);

	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = nullptr;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
	tvInsertStruct.item.pszText = (LPWSTR)pModel->getPath();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_SELECTED;
	tvInsertStruct.item.cChildren = !pModel->getGeometries().empty() ? 1 : 0;
	tvInsertStruct.item.lParam = (LPARAM)pModelItemData;

	HTREEITEM hModel = m_pTreeCtrl->InsertItem(&tvInsertStruct);
	pModelItemData->treeItem() = hModel;

	/*
	* Header
	*/
	HTREEITEM hHeader = m_pTreeCtrl->InsertItem(_T("Header"), IMAGE_PROPERTY_SET, IMAGE_PROPERTY_SET, hModel);
	LoadHeader(hHeader);

	/*
	* Roots
	*/
	for (auto pGeometry : pModel->getGeometries())
	{
		_ptr<CAP242ProductDefinition> apProductDefinition(pGeometry);
		if (apProductDefinition->GetRelatedProducts() == 0)
		{
			LoadProduct(pModel, apProductDefinition, hModel);
		}
	}

	m_pTreeCtrl->Expand(hModel, TVE_EXPAND);

	m_bInitInProgress = false;

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	TRACE(L"\n*** CAP242PModelStructureView::LoadModel() : %lld [µs]", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());
}

void CAP242PModelStructureView::LoadHeader(HTREEITEM hParent)
{
	auto pModel = getModelAs<CAP242Model>();
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
		GetSPFFHeaderItem(pModel->getSdaiInstance(), 2, 0, sdaiUNICODE, (char**)&szText);

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
		GetSPFFHeaderItem(pModel->getSdaiInstance(), 3, 0, sdaiUNICODE, (char**)&szText);

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
		tvInsertStruct.hParent = hParent;
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
		GetSPFFHeaderItem(pModel->getSdaiInstance(), 7, 0, sdaiUNICODE, (char**)&szText);

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
		GetSPFFHeaderItem(pModel->getSdaiInstance(), 8, 0, sdaiUNICODE, (char**)&szText);

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

void CAP242PModelStructureView::LoadProduct(CAP242Model* pModel, CAP242ProductDefinition* pProduct, HTREEITEM hParent)
{
	if((pModel == nullptr) || (pProduct == nullptr))
	{
		ASSERT(FALSE);

		return;
	}

	// 
	// Product
	//

	CString strName;
	strName.Format(L"#%lld %s %s", pProduct->getExpressID(), pProduct->GetProductName(), ITEM_PRODUCT_DEFINION);

	HTREEITEM hProduct = m_pTreeCtrl->InsertItem(strName, IMAGE_SELECTED, IMAGE_SELECTED, hParent);

	int iGeometryImage = HasDescendantsWithGeometry(pModel, pProduct) ? IMAGE_SELECTED : IMAGE_NO_GEOMETRY;
	m_pTreeCtrl->InsertItem(ITEM_GEOMETRY, iGeometryImage, iGeometryImage, hProduct);

	auto pItemData = new CAP242ItemData(nullptr, (int64_t*)pProduct, enumSTEPItemDataType::ProductDefinition);
	m_vecItemData.push_back(pItemData);

	m_pTreeCtrl->SetItemData(hProduct, (DWORD_PTR)pItemData);

	// Assemblies
	for (auto itAssembly : pModel->GetAssemblies())
	{
		if (itAssembly.second->GetRelatingProductDefinition() == pProduct)
		{
			LoadAssembly(pModel, itAssembly.second, hProduct);			
		}
	}

	// Instances
	for (auto pInstance : pProduct->getInstances())
	{
		LoadInstance(pModel, _ptr<CAP242ProductInstance>(pInstance), hProduct);
	}
}

void CAP242PModelStructureView::LoadAssembly(CAP242Model* pModel, CAP242Assembly* pAssembly, HTREEITEM hParent)
{
	if ((pModel == nullptr) || (pAssembly == nullptr))
	{
		ASSERT(FALSE);

		return;
	}

	CString strName;
	strName.Format(L"#%lld %s %s", pAssembly->GetExpressID(), pAssembly->GetName(), ITEM_ASSEMBLY);

	HTREEITEM hAssembly = m_pTreeCtrl->InsertItem(strName, IMAGE_SELECTED, IMAGE_SELECTED, hParent);

	int iGeometryImage = HasDescendantsWithGeometry(pModel, pAssembly) ? IMAGE_SELECTED : IMAGE_NO_GEOMETRY;
	m_pTreeCtrl->InsertItem(ITEM_GEOMETRY, iGeometryImage, iGeometryImage, hAssembly);

	auto pItemData = new CAP242ItemData(nullptr, (int64_t*)pAssembly, enumSTEPItemDataType::Assembly);
	m_vecItemData.push_back(pItemData);

	m_pTreeCtrl->SetItemData(hAssembly, (DWORD_PTR)pItemData);

	LoadProduct(pModel, pAssembly->GetRelatedProductDefinition(), hAssembly);
}

void CAP242PModelStructureView::LoadInstance(CAP242Model* pModel, CAP242ProductInstance* pInstance, HTREEITEM hParent)
{
	if ((pModel == nullptr) || (pInstance == nullptr))
	{
		ASSERT(FALSE);

		return;
	}

	CString strName;
	strName.Format(L"%s %s (%lld)", pInstance->getName().c_str(), ITEM_PRODUCT_INSTANCE, pInstance->getID());

	HTREEITEM hInstance = m_pTreeCtrl->InsertItem(strName, IMAGE_SELECTED, IMAGE_SELECTED, hParent);

	int iGeometryImage = pInstance->hasGeometry() ? IMAGE_SELECTED : IMAGE_NO_GEOMETRY;
	m_pTreeCtrl->InsertItem(ITEM_GEOMETRY, iGeometryImage, iGeometryImage, hInstance);

	auto pItemData = new CAP242ItemData(nullptr, (int64_t*)pInstance, enumSTEPItemDataType::ProductInstance);
	m_vecItemData.push_back(pItemData);

	m_pTreeCtrl->SetItemData(hInstance, (DWORD_PTR)pItemData);

	auto itInstance2Item = m_mapInstance2Item.find(pInstance);
	if (itInstance2Item != m_mapInstance2Item.end())
	{
		itInstance2Item->second.push_back(hInstance);
	}
	else
	{
		m_mapInstance2Item[pInstance] = vector<HTREEITEM>{ hInstance };
	}
}

bool CAP242PModelStructureView::HasDescendantsWithGeometry(CAP242Model* pModel, CAP242ProductDefinition* pProduct)
{
	if((pModel == nullptr) || (pProduct == nullptr))
	{
		ASSERT(FALSE);

		return false;
	}

	if (pProduct->hasGeometry())
	{
		return true;
	}

	bool bHasDescendantWithGeometry = false;
	HasDescendantsWithGeometryRecursively(pModel, pProduct, bHasDescendantWithGeometry);

	return bHasDescendantWithGeometry;
}

void CAP242PModelStructureView::HasDescendantsWithGeometryRecursively(CAP242Model* pModel, CAP242ProductDefinition* pProduct, bool& bHasDescendantWithGeometry)
{
	if ((pModel == nullptr) || (pProduct == nullptr))
	{
		ASSERT(FALSE);

		return;
	}

	for (auto itAssembly : pModel->GetAssemblies())
	{
		if (itAssembly.second->GetRelatingProductDefinition() == pProduct)
		{
			bHasDescendantWithGeometry = HasDescendantsWithGeometry(pModel, itAssembly.second->GetRelatedProductDefinition());
			if (bHasDescendantWithGeometry)
			{
				break;
			}
		}
	}
}

bool CAP242PModelStructureView::HasDescendantsWithGeometry(CAP242Model* pModel, CAP242Assembly* pAssembly)
{
	if ((pModel == nullptr) || (pAssembly == nullptr))
	{
		ASSERT(FALSE);

		return false;
	}

	return HasDescendantsWithGeometry(pModel, pAssembly->GetRelatedProductDefinition());
}

void CAP242PModelStructureView::ResetTree(bool bEnable)
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

void CAP242PModelStructureView::ResetTree(HTREEITEM hParent, bool bEnable)
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

void CAP242PModelStructureView::UpdateChildren(HTREEITEM hParent)
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

		UpdateChildren(hChild);

		hChild = m_pTreeCtrl->GetNextSiblingItem(hChild);
	} // while (hChild != nullptr)
}

void CAP242PModelStructureView::UpdateChildrenInMemory(CAP242ItemData* pParent, bool bEnable)
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
			pChild->GetInstance<CAP242ProductInstance>()->setEnable(bEnable);
		}

		UpdateChildrenInMemory(pChild, bEnable);
	} // for (size_t iChild = ...
}

void CAP242PModelStructureView::UpdateParent(HTREEITEM hParent)
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

		CAP242ItemData* pItemData = (CAP242ItemData*)m_pTreeCtrl->GetItemData(hParent);
		if ((pItemData != nullptr) && (pItemData->getType() == enumSTEPItemDataType::ProductInstance))
		{
			pItemData->GetInstance<CAP242ProductInstance>()->setEnable(true);
		}

		UpdateParent(m_pTreeCtrl->GetParentItem(hParent));

		return;
	}

	if (iSelectedChidlrenCount == 0)
	{
		m_pTreeCtrl->SetItemImage(hParent, IMAGE_NOT_SELECTED, IMAGE_NOT_SELECTED);

		CAP242ItemData* pItemData = (CAP242ItemData*)m_pTreeCtrl->GetItemData(hParent);
		if ((pItemData != nullptr) && (pItemData->getType() == enumSTEPItemDataType::ProductInstance))
		{
			pItemData->GetInstance<CAP242ProductInstance>()->setEnable(false);
		}

		UpdateParent(m_pTreeCtrl->GetParentItem(hParent));

		return;
	}

	if (iSelectedChidlrenCount == iChidlrenCount)
	{
		m_pTreeCtrl->SetItemImage(hParent, IMAGE_SELECTED, IMAGE_SELECTED);

		CAP242ItemData* pItemData = (CAP242ItemData*)m_pTreeCtrl->GetItemData(hParent);
		if ((pItemData != nullptr) && (pItemData->getType() == enumSTEPItemDataType::ProductInstance))
		{
			pItemData->GetInstance<CAP242ProductInstance>()->setEnable(true);
		}

		UpdateParent(m_pTreeCtrl->GetParentItem(hParent));

		return;
	}

	m_pTreeCtrl->SetItemImage(hParent, IMAGE_SEMI_SELECTED, IMAGE_SEMI_SELECTED);

	CAP242ItemData* pItemData = (CAP242ItemData*)m_pTreeCtrl->GetItemData(hParent);
	if ((pItemData != nullptr) && (pItemData->getType() == enumSTEPItemDataType::ProductInstance))
	{
		pItemData->GetInstance<CAP242ProductInstance>()->setEnable(true);
	}

	UpdateParent(m_pTreeCtrl->GetParentItem(hParent));
}

void CAP242PModelStructureView::ResetView()
{
	m_pTreeCtrl->DeleteAllItems();

	for (size_t iItemData = 0; iItemData < m_vecItemData.size(); iItemData++)
	{
		delete m_vecItemData[iItemData];
	}
	m_vecItemData.clear();
	m_mapInstance2Item.clear();
	m_hSelectedItem = nullptr;

	m_pSearchDialog->Reset();

	LoadModel();
}