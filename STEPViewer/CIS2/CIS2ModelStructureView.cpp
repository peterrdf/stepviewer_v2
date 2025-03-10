#include "stdafx.h"

#include "_ptr.h"

#include "mainfrm.h"
#include "CIS2ModelStructureView.h"
#include "Resource.h"
#include "STEPViewer.h"

#include <algorithm>
#include <chrono>
using namespace std;

// ------------------------------------------------------------------------------------------------
CCIS2ModelStructureView::CCIS2ModelStructureView(CTreeCtrlEx* pTreeCtrl)
	: CModelStructureViewBase(pTreeCtrl)
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
	m_pImageList->DeleteImageList();
	delete m_pImageList;	

	m_pTreeCtrl->SetItemStateProvider(nullptr);

	delete m_pSearchDialog;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CCIS2ModelStructureView::onInstanceSelected(_view* pSender) /*override*/
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

	// Single instance selection
	UnselectAllItems();

	auto pSelectedInstance = getController()->getSelectedInstance() != nullptr ? 
		dynamic_cast<CCIS2Instance*>(getController()->getSelectedInstance()) : 
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

	auto pController = getController();
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
					pInstance->setEnable(false);
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
					pInstance->setEnable(true);
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

		pController->onInstancesEnabledStateChanged(this);

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

		pController->selectInstance(this, pSelectedInstance);

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
	auto pController = getController();
	if (pController == nullptr)
	{
		return false;
	}

	auto pInstance = (CCIS2Instance*)m_pTreeCtrl->GetItemData(hItem);
	if (pInstance == nullptr)
	{
		return false;
	}	

	return pInstance == pController->getSelectedInstance();
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

	auto pController = getAPController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	ASSERT(pController->getModels().size() == 1);

	auto pModel = _ptr<CCIS2Model>(pController->getModels()[0]);
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	// Select clicked item
	CPoint ptTree = point;
	m_pTreeCtrl->ScreenToClient(&ptTree);

	CCIS2Instance* pTargetInstance = nullptr;

	UINT flags = 0;
	HTREEITEM hItem = m_pTreeCtrl->HitTest(ptTree, &flags);
	if (hItem != nullptr)
	{
		m_pTreeCtrl->SelectItem(hItem);
		m_pTreeCtrl->SetFocus();

		pTargetInstance = (CCIS2Instance*)m_pTreeCtrl->GetItemData(hItem);
		if (pTargetInstance == nullptr)
		{
			// Check the first child
			HTREEITEM hChild = NULL;
			if (((hChild = m_pTreeCtrl->GetNextItem(hItem, TVGN_CHILD)) != NULL) &&
				(m_pTreeCtrl->GetItemText(hChild) == ITEM_GEOMETRY) &&
				(m_pTreeCtrl->GetItemData(hChild) != NULL))
			{
				hItem = hChild;
				pTargetInstance = (CCIS2Instance*)m_pTreeCtrl->GetItemData(hItem);
			}
		} // if (pInstance == nullptr)
	} // if (hItem != nullptr)	

	// ENTITY : VISIBLE COUNT
	map<wstring, long> mapEntity2VisibleCount;
	for (auto pGeometry : pModel->getGeometries())
	{
		if (!pGeometry->hasGeometry())
		{
			continue;
		}

		_ptr<_ap_geometry> apGeometry(pGeometry);
		ASSERT(apGeometry);

		const wchar_t* szEntityName = apGeometry->getEntityName();

		auto itEntity2VisibleCount = mapEntity2VisibleCount.find(szEntityName);
		if (itEntity2VisibleCount == mapEntity2VisibleCount.end())
		{
			mapEntity2VisibleCount[szEntityName] = pGeometry->getEnabledInstancesCount() ? 1 : 0;
		}
		else
		{
			itEntity2VisibleCount->second += pGeometry->getEnabledInstancesCount() ? 1 : 0;
		}
	} // for (; itInstance != ...

	ASSERT(!mapEntity2VisibleCount.empty());

	// Build menu
	CMenu menuMain;
	CMenu* pMenu = nullptr;

	if (pTargetInstance != nullptr)
	{
		if (pTargetInstance->hasGeometry())
		{
			VERIFY(menuMain.LoadMenuW(IDR_POPUP_INSTANCES));
			pMenu = menuMain.GetSubMenu(0);

			// Zoom to
			if (!pTargetInstance->getEnable())
			{
				pMenu->EnableMenuItem(ID_INSTANCES_ZOOM_TO, MF_BYCOMMAND | MF_DISABLED);
			}

			// Save
			if (!pTargetInstance->getEnable())
			{
				pMenu->EnableMenuItem(ID_INSTANCES_SAVE, MF_BYCOMMAND | MF_DISABLED);
			}

			// Enable
			if (pTargetInstance->getEnable())
			{
				pMenu->CheckMenuItem(ID_INSTANCES_ENABLE, MF_BYCOMMAND | MF_CHECKED);
			}
		} // if (pTargetInstance->HasGeometry())
		else 
		{
			VERIFY(menuMain.LoadMenuW(IDR_POPUP_INSTANCES_NO_GEOMETRY));
			pMenu = menuMain.GetSubMenu(0);
		} // else if (pTargetInstance->HasGeometry())
	} // if (pTargetInstance != nullptr)
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
	if (pTargetInstance != nullptr)
	{
		if (pTargetInstance->hasGeometry())
		{
			switch (uiCommand)
			{
				case ID_INSTANCES_ZOOM_TO:
				{
					pController->zoomToInstance(pTargetInstance);
				}
				break;

				case ID_VIEW_ZOOM_OUT:
				{
					pController->zoomOut();
				}
				break;

				case ID_INSTANCES_SAVE:
				{
					pController->saveInstance(pTargetInstance);
				}
				break;

				case ID_INSTANCES_ENABLE:
				{
					pTargetInstance->setEnable(!pTargetInstance->getEnable());

					int iImage = pTargetInstance->getEnable() ? IMAGE_SELECTED : IMAGE_NOT_SELECTED;
					m_pTreeCtrl->SetItemImage(hItem, iImage, iImage);

					ClickItem_UpdateChildren(hItem);
					ClickItem_UpdateParent(m_pTreeCtrl->GetParentItem(hItem));

					pController->onInstancesEnabledStateChanged(this);
				}
				break;

				case ID_INSTANCES_DISABLE_ALL_BUT_THIS:
				{
					for (auto pInstance : pModel->getInstances())
					{
						pInstance->setEnable(pTargetInstance == pInstance);
					}

					ResetView();
					onInstanceSelected(nullptr);

					pController->onInstancesEnabledStateChanged(this);
				}
				break;

				case ID_INSTANCES_ENABLE_ALL:
				{
					for (auto pInstance : pModel->getInstances())
					{
						pInstance->setEnable(true);
					}

					ResetView();
					onInstanceSelected(nullptr);

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
					pController->zoomOut();
				}
				break;

				case IDS_VIEW_IFC_RELATIONS:
				{
					pController->onViewRelations(this, pTargetInstance->getSdaiInstance());
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
				pController->zoomOut();
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
		for (auto pInstance : pModel->getInstances())
		{
			//#todo#mappeditems		
			pTargetInstance = _ptr<CCIS2Instance>(pInstance);

			if (pTargetInstance->getEntityName() == itCommand2Entity->second)
			{
				pTargetInstance->setEnable(itEntity2VisibleCount->second > 0 ? false : true);

				auto itInstance2GeometryItem = m_mapInstance2GeometryItem.find(pTargetInstance);
				ASSERT(itInstance2GeometryItem != m_mapInstance2GeometryItem.end());

				HTREEITEM hGeometryItem = itInstance2GeometryItem->second;

				int iImage = pTargetInstance->getEnable() ? IMAGE_SELECTED : IMAGE_NOT_SELECTED;
				m_pTreeCtrl->SetItemImage(hGeometryItem, iImage, iImage);

				ASSERT(!m_pTreeCtrl->ItemHasChildren(hGeometryItem));

				HTREEITEM hInstanceItem = m_pTreeCtrl->GetParentItem(hGeometryItem);
				ASSERT(hInstanceItem != NULL);

				ClickItem_UpdateParent(hInstanceItem, FALSE);

				m_setParents.insert(m_pTreeCtrl->GetParentItem(hInstanceItem));
			}
		} // for (auto pInstance = ...

		// Update the Parents
		for (auto hParent : m_setParents)
		{
			ClickItem_UpdateParent(hParent);
		}

		pController->onInstancesEnabledStateChanged(this);
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

	for (auto& pGeometry : pModel->getGeometries())
	{
		_ptr<_ap_geometry> apGeometry(pGeometry);
		LoadInstance(pModel, apGeometry->getSdaiInstance(), hModel);
	}

	//LoadTree_UpdateItems(hModel); //#todo?

	m_pTreeCtrl->Expand(hModel, TVE_EXPAND);
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

	SdaiEntity iIFCRelAggregatesEntity = sdaiGetEntity(pModel->getSdaiModel(), "IFCRELAGGREGATES");

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

	SdaiEntity iIFCRelNestsEntity = sdaiGetEntity(pModel->getSdaiModel(), "IFCRELNESTS");

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

	SdaiEntity iIFCRelContainedInSpatialStructureEntity = sdaiGetEntity(pModel->getSdaiModel(), "IFCRELCONTAINEDINSPATIALSTRUCTURE");

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
void CCIS2ModelStructureView::LoadInstance(CCIS2Model* pModel, SdaiInstance sdaiInstance, HTREEITEM hParent)
{
	ASSERT(pModel != nullptr);

	auto pGeometry = pModel->getGeometryByInstance(sdaiInstance);
	if (pGeometry != nullptr)
	{
		//#todo#mappeditems
		ASSERT(pGeometry->getInstances().size() == 1);
		_ptr<CCIS2Instance> cis2Instance(pGeometry->getInstances()[0]);

		wstring strItem = _ap_geometry::getName(sdaiInstance);

		/*
		* Object
		*/
		TV_INSERTSTRUCT tvInsertStruct;
		tvInsertStruct.hParent = hParent;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = 
			pGeometry->hasGeometry() ?
			(pGeometry->getEnabledInstancesCount() > 0 ? IMAGE_SELECTED : IMAGE_NOT_SELECTED) :
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
			pGeometry->hasGeometry() ?
			(pGeometry->getEnabledInstancesCount() > 0 ? IMAGE_SELECTED : IMAGE_NOT_SELECTED) :
			IMAGE_NO_GEOMETRY;
		tvInsertStruct.item.lParam = (LPARAM)cis2Instance.p();

		HTREEITEM hInstance = m_pTreeCtrl->InsertItem(&tvInsertStruct);
		m_mapInstance2GeometryItem[cis2Instance] = hInstance;

		/*
		* decomposition/nest/contains
		*/
		LoadIsDecomposedBy(pModel, sdaiInstance, hObject);
		LoadIsNestedBy(pModel, sdaiInstance, hObject);
		LoadContainsElements(pModel, sdaiInstance, hObject);
	} // if (itInstance != ...
	else
	{
		ASSERT(FALSE);
	}
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

	int iChildrenCount = 0;
	int iSelectedChildrenCount = 0;
	int iSemiSelectedChildrenCount = 0;

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

		iChildrenCount++;

		switch (iImage)
		{
		case IMAGE_SELECTED:
		{
			iSelectedChildrenCount++;
		}
		break;

		case IMAGE_SEMI_SELECTED:
		{
			iSemiSelectedChildrenCount++;
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

	if (iChildrenCount == 0)
	{
		// keep the state as it is
		return;
	}

	if (iSemiSelectedChildrenCount > 0)
	{
		m_pTreeCtrl->SetItemImage(hParent, IMAGE_SEMI_SELECTED, IMAGE_SEMI_SELECTED);

		return;
	}

	if (iSelectedChildrenCount == 0)
	{
		m_pTreeCtrl->SetItemImage(hParent, IMAGE_NOT_SELECTED, IMAGE_NOT_SELECTED);

		return;
	}

	if (iSelectedChildrenCount == iChildrenCount)
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
			pInstance->setEnable((iParentImage == IMAGE_SELECTED) || (iParentImage == IMAGE_SEMI_SELECTED) ? true : false);
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

	int iChildrenCount = 0;
	int iSelectedChildrenCount = 0;
	int iSemiSelectedChildrenCount = 0;

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

		iChildrenCount++;

		switch (iImage)
		{
			case IMAGE_SELECTED:
			{
				iSelectedChildrenCount++;
			}
			break;

			case IMAGE_SEMI_SELECTED:
			{
				iSemiSelectedChildrenCount++;
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

	if (iSemiSelectedChildrenCount > 0)
	{
		m_pTreeCtrl->SetItemImage(hParent, IMAGE_SEMI_SELECTED, IMAGE_SEMI_SELECTED);

		CCIS2Instance* pInstance = (CCIS2Instance*)m_pTreeCtrl->GetItemData(hParent);
		if (pInstance != nullptr)
		{
			pInstance->setEnable(true);
		}

		if (bRecursive)
		{
			ClickItem_UpdateParent(m_pTreeCtrl->GetParentItem(hParent));
		}		

		return;
	}

	if (iSelectedChildrenCount == 0)
	{
		m_pTreeCtrl->SetItemImage(hParent, IMAGE_NOT_SELECTED, IMAGE_NOT_SELECTED);

		CCIS2Instance* pInstance = (CCIS2Instance*)m_pTreeCtrl->GetItemData(hParent);
		if (pInstance != nullptr)
		{
			pInstance->setEnable(false);
		}

		if (bRecursive)
		{
			ClickItem_UpdateParent(m_pTreeCtrl->GetParentItem(hParent));
		}

		return;
	}

	if (iSelectedChildrenCount == iChildrenCount)
	{
		m_pTreeCtrl->SetItemImage(hParent, IMAGE_SELECTED, IMAGE_SELECTED);

		CCIS2Instance* pInstance = (CCIS2Instance*)m_pTreeCtrl->GetItemData(hParent);
		if (pInstance != nullptr)
		{
			pInstance->setEnable(true);
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
		pInstance->setEnable(true);
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

	auto pController = getAPController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	ASSERT(pController->getModels().size() == 1);

	auto pModel = _ptr<CCIS2Model>(pController->getModels()[0]);
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	LoadModel(pModel);
}