#include "stdafx.h"

#include "_ifc_model.h"
#include "_ifc_geometry.h"
#include "_ptr.h"

#include "mainfrm.h"
#include "IFCModelStructureView.h"
#include "Resource.h"
#include "STEPViewer.h"

#include <algorithm>
#include <chrono>
using namespace std;

// ************************************************************************************************
CIFCModelStructureView::CIFCModelStructureView(CTreeCtrlEx* pTreeCtrl)
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

/*virtual*/ CIFCModelStructureView::~CIFCModelStructureView()
{
	m_pTreeCtrl->SetImageList(nullptr, TVSIL_NORMAL);

	m_pImageList->DeleteImageList();
	delete m_pImageList;	

	m_pTreeCtrl->SetItemStateProvider(nullptr);

	delete m_pSearchDialog;
}

/*virtual*/ void CIFCModelStructureView::onInstanceSelected(_view* pSender) /*override*/
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
		dynamic_cast<_ifc_instance*>(getController()->getSelectedInstance()) : 
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

/*virtual*/ void CIFCModelStructureView::Load() /*override*/
{
	ResetView();
}

/*virtual*/ CImageList* CIFCModelStructureView::GetImageList() const /*override*/
{
	return m_pImageList;
}

/*virtual*/ void CIFCModelStructureView::OnShowWindow(BOOL bShow, UINT /*nStatus*/) /*override*/
{
	if (!bShow)
	{
		m_pSearchDialog->ShowWindow(SW_HIDE);
	}
}

/*virtual*/ void CIFCModelStructureView::OnTreeItemClick(NMHDR* /*pNMHDR*/, LRESULT* pResult) /*override*/
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

		_ifc_instance* pInstance = (_ifc_instance*)m_pTreeCtrl->GetItemData(hItem);

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
			(_ifc_instance*)m_pTreeCtrl->GetItemData(hItem) :
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

/*virtual*/ void CIFCModelStructureView::OnTreeItemExpanding(NMHDR* /*pNMHDR*/, LRESULT* pResult) /*override*/
{
	*pResult = 0;
}

/*virtual*/ bool CIFCModelStructureView::IsSelected(HTREEITEM hItem) /*override*/
{
	auto pController = getController();
	if (pController == nullptr)
	{
		return false;
	}

	auto pInstance = (_ifc_instance*)m_pTreeCtrl->GetItemData(hItem);
	if (pInstance == nullptr)
	{
		return false;
	}	

	return pInstance == pController->getSelectedInstance();
}

/*virtual*/ CTreeCtrlEx* CIFCModelStructureView::GetTreeView() /*override*/
{
	return m_pTreeCtrl;
}

/*virtual*/ vector<CString> CIFCModelStructureView::GetSearchFilters() /*override*/
{
	return vector<CString>
		{
			_T("(All)"),
			_T("Express ID"),
		};
}

/*virtual*/ void CIFCModelStructureView::LoadChildrenIfNeeded(HTREEITEM /*hItem*/) /*override*/
{
}

/*virtual*/ BOOL CIFCModelStructureView::ContainsText(int iFilter, HTREEITEM hItem, const CString& strText) /*override*/
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

/*virtual*/ void CIFCModelStructureView::OnContextMenu(CWnd* pWnd, CPoint point) /*override*/
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

	auto pController = getController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	// Select clicked item
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

	auto pTargetInstance = (_ifc_instance*)m_pTreeCtrl->GetItemData(hItem);
	if (pTargetInstance == nullptr)
	{
		// Check the first child
		HTREEITEM hChild = NULL;
		if (((hChild = m_pTreeCtrl->GetNextItem(hItem, TVGN_CHILD)) != NULL) &&
			(m_pTreeCtrl->GetItemText(hChild) == ITEM_GEOMETRY) &&
			(m_pTreeCtrl->GetItemData(hChild) != NULL))
		{
			hItem = hChild;
			pTargetInstance = (_ifc_instance*)m_pTreeCtrl->GetItemData(hItem);
		}
	} // if (pInstance == nullptr)

	_model* pModel = nullptr;
	if (pTargetInstance == nullptr)
	{
		HTREEITEM hParent = m_pTreeCtrl->GetParentItem(hItem);
		while (hParent != nullptr)
		{
			auto pParentInstance = (_ifc_instance*)m_pTreeCtrl->GetItemData(hParent);
			if (pParentInstance == nullptr)
			{
				// Check the first child
				HTREEITEM hChild = NULL;
				if (((hChild = m_pTreeCtrl->GetNextItem(hParent, TVGN_CHILD)) != NULL) &&
					(m_pTreeCtrl->GetItemText(hChild) == ITEM_GEOMETRY) &&
					(m_pTreeCtrl->GetItemData(hChild) != NULL))
				{
					pModel = pController->getModelByInstance(((_ifc_instance*)m_pTreeCtrl->GetItemData(hChild))->getOwlModel());

					break;
				}
			}

			hParent = m_pTreeCtrl->GetParentItem(hParent);
		} // while (hParent != nullptr)
	} // if (pTargetInstance == nullptr)
	else
	{
		pModel = pController->getModelByInstance(pTargetInstance->getOwlModel());
	}

	if (pModel == nullptr)
	{
		return;
	}

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

		const wchar_t* szEntityName = _ap_instance::getEntityName(apGeometry->getSdaiInstance());

		auto itEntity2VisibleCount = mapEntity2VisibleCount.find(szEntityName);
		if (itEntity2VisibleCount == mapEntity2VisibleCount.end())
		{
			mapEntity2VisibleCount[szEntityName] = pGeometry->getEnabledInstancesCount();
		}
		else
		{
			itEntity2VisibleCount->second += pGeometry->getEnabledInstancesCount();
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
		} // if (pTargetInstance->hasGeometry())
		else 
		{
			VERIFY(menuMain.LoadMenuW(IDR_POPUP_INSTANCES_NO_GEOMETRY));
			pMenu = menuMain.GetSubMenu(0);
		}
	} // if (pTargetInstance != nullptr)
	else
	{
		VERIFY(menuMain.LoadMenuW(IDR_POPUP_META_DATA));
		pMenu = menuMain.GetSubMenu(0);
	}

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
					pController->zoomToSelectedInstance();
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
		} // if (pTargetInstance->HasGeometry())
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
		} // else if (pTargetInstance->HasGeometry())
	} // if (pTargetInstance != nullptr)
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
			 pTargetInstance = _ptr<_ifc_instance>(pInstance);

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

/*virtual*/ void CIFCModelStructureView::OnSearch() /*override*/
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

void CIFCModelStructureView::LoadModel(_ifc_model* pModel)
{
	// Model
	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = nullptr;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = (LPWSTR)pModel->getPath();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_SELECTED;
	tvInsertStruct.item.lParam = NULL;
	HTREEITEM hModel = m_pTreeCtrl->InsertItem(&tvInsertStruct);

	// Header
	LoadHeader(hModel);

	// Project/Units/Unreferenced
	SdaiAggr sdaiProjectAggr = sdaiGetEntityExtentBN(pModel->getSdaiModel(), (char*)"IFCPROJECT");

	SdaiInteger iProjectInstancesCount = sdaiGetMemberCount(sdaiProjectAggr);
	if (iProjectInstancesCount > 0)
	{
		SdaiInstance sdaiProjectInstance = 0;
		engiGetAggrElement(sdaiProjectAggr, 0, sdaiINSTANCE, &sdaiProjectInstance);

		// Project
		LoadProject(pModel, hModel, sdaiProjectInstance);

		// Unreferenced
		LoadUnreferencedItems(pModel, hModel);

		// UI
		LoadTree_UpdateItem(hModel);
	} // if (iProjectInstancesCount > 0)

	m_pTreeCtrl->Expand(hModel, TVE_EXPAND);
}

void CIFCModelStructureView::LoadProject(_ifc_model* pModel, HTREEITEM hModel, SdaiInstance sdaiProjectInstance)
{
	auto pController = getController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return;
	}	

	auto pGeometry = pModel->getGeometryByInstance(sdaiProjectInstance);
	if (pGeometry != nullptr)
	{
		wstring strItem = _ap_instance::getName(sdaiProjectInstance);

		//#todo#mappeditems
		ASSERT(pGeometry->getInstances().size() == 1);
		_ptr<_ifc_instance> ifcInstance(pGeometry->getInstances()[0]);

		_ptr<_ap_geometry> apGeometry(pGeometry);
		ASSERT(apGeometry);

		// Project
		TV_INSERTSTRUCT tvInsertStruct;
		tvInsertStruct.hParent = hModel;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_SELECTED;
		tvInsertStruct.item.lParam = NULL;
		HTREEITEM hProject = m_pTreeCtrl->InsertItem(&tvInsertStruct);

		// Instance
		tvInsertStruct.hParent = hProject;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = ITEM_GEOMETRY;
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_NO_GEOMETRY;
		tvInsertStruct.item.lParam = (LPARAM)ifcInstance.p();
		HTREEITEM hInstance = m_pTreeCtrl->InsertItem(&tvInsertStruct);		

		m_mapInstance2GeometryItem[ifcInstance] = hInstance;

		// decomposition/contains
		LoadIsDecomposedBy(pModel, sdaiProjectInstance, hProject);
		LoadIsNestedBy(pModel, sdaiProjectInstance, hProject);
		LoadContainsElements(pModel, sdaiProjectInstance, hProject);

		m_pTreeCtrl->Expand(hProject, TVE_EXPAND);
	} // if (itInstance != ...
}

void CIFCModelStructureView::LoadIsDecomposedBy(_ifc_model* pModel, SdaiInstance sdaiInstance, HTREEITEM hParent)
{
	ASSERT(pModel != nullptr);

	SdaiAggr sdaiIsDecomposedByAggr = nullptr;
	sdaiGetAttrBN(sdaiInstance, "IsDecomposedBy", sdaiAGGR, &sdaiIsDecomposedByAggr);

	if (sdaiIsDecomposedByAggr == nullptr)
	{
		return;
	}

	SdaiEntity sdaiRelAggregatesEntity = sdaiGetEntity(pModel->getSdaiModel(), "IFCRELAGGREGATES");

	SdaiInteger iIsDecomposedByInstancesCount = sdaiGetMemberCount(sdaiIsDecomposedByAggr);
	for (SdaiInteger i = 0; i < iIsDecomposedByInstancesCount; ++i)
	{
		SdaiInstance sdaiIsDecomposedByInstance = 0;
		engiGetAggrElement(sdaiIsDecomposedByAggr, i, sdaiINSTANCE, &sdaiIsDecomposedByInstance);

		if (sdaiGetInstanceType(sdaiIsDecomposedByInstance) != sdaiRelAggregatesEntity)
		{
			continue;
		}

		TV_INSERTSTRUCT tvInsertStruct;
		tvInsertStruct.hParent = hParent;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = ITEM_DECOMPOSITION;
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_SELECTED;
		tvInsertStruct.item.lParam = NULL;
		HTREEITEM hDecomposition = m_pTreeCtrl->InsertItem(&tvInsertStruct);

		SdaiAggr sdaiRelatedObjectsAggr = 0;
		sdaiGetAttrBN(sdaiIsDecomposedByInstance, "RelatedObjects", sdaiAGGR, &sdaiRelatedObjectsAggr);

		SdaiInteger iRelatedObjectsInstancesCount = sdaiGetMemberCount(sdaiRelatedObjectsAggr);
		for (SdaiInteger j = 0; j < iRelatedObjectsInstancesCount; ++j)
		{
			SdaiInstance sdaiRelatedObjectsInstance = 0;
			engiGetAggrElement(sdaiRelatedObjectsAggr, j, sdaiINSTANCE, &sdaiRelatedObjectsInstance);

			LoadObject(pModel, sdaiRelatedObjectsInstance, hDecomposition);
		}
	} // for (SdaiInteger i = ...	
}

void CIFCModelStructureView::LoadIsNestedBy(_ifc_model* pModel, SdaiInstance sdaiInstance, HTREEITEM hParent)
{
	ASSERT(pModel != nullptr);

	SdaiAggr sdaiIsNestedByAggr = nullptr;
	sdaiGetAttrBN(sdaiInstance, "IsNestedBy", sdaiAGGR, &sdaiIsNestedByAggr);

	if (sdaiIsNestedByAggr == nullptr)
	{
		return;
	}

	SdaiEntity sdaiRelNestsEntity = sdaiGetEntity(pModel->getSdaiModel(), "IFCRELNESTS");

	SdaiInteger sdaiIsNestedByInstancesCount = sdaiGetMemberCount(sdaiIsNestedByAggr);
	for (SdaiInteger i = 0; i < sdaiIsNestedByInstancesCount; ++i)
	{
		SdaiInstance sdaiIsNestedByInstance = 0;
		engiGetAggrElement(sdaiIsNestedByAggr, i, sdaiINSTANCE, &sdaiIsNestedByInstance);

		if (sdaiGetInstanceType(sdaiIsNestedByInstance) != sdaiRelNestsEntity)
		{
			continue;
		}

		TV_INSERTSTRUCT tvInsertStruct;
		tvInsertStruct.hParent = hParent;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = ITEM_DECOMPOSITION;
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_SELECTED;
		tvInsertStruct.item.lParam = NULL;
		HTREEITEM hDecomposition = m_pTreeCtrl->InsertItem(&tvInsertStruct);

		SdaiAggr sdaiRelatedObjectsAggr = 0;
		sdaiGetAttrBN(sdaiIsNestedByInstance, "RelatedObjects", sdaiAGGR, &sdaiRelatedObjectsAggr);

		SdaiInteger sdaiRelatedObjectsInstancesCount = sdaiGetMemberCount(sdaiRelatedObjectsAggr);
		for (SdaiInteger j = 0; j < sdaiRelatedObjectsInstancesCount; ++j)
		{
			SdaiInstance sdaiRelatedObjectsInstance = 0;
			engiGetAggrElement(sdaiRelatedObjectsAggr, j, sdaiINSTANCE, &sdaiRelatedObjectsInstance);

			LoadObject(pModel, sdaiRelatedObjectsInstance, hDecomposition);
		}
	} // for (SdaiInteger i = ...
}

void CIFCModelStructureView::LoadContainsElements(_ifc_model* pModel, SdaiInstance sdaiInstance, HTREEITEM hParent)
{
	ASSERT(pModel != nullptr);

	SdaiAggr sdaiContainsElementsAggr = nullptr;
	sdaiGetAttrBN(sdaiInstance, "ContainsElements", sdaiAGGR, &sdaiContainsElementsAggr);

	if (sdaiContainsElementsAggr == nullptr)
	{
		return;
	}

	SdaiEntity sdaiRelContainedInSpatialStructureEntity = sdaiGetEntity(pModel->getSdaiModel(), "IFCRELCONTAINEDINSPATIALSTRUCTURE");

	SdaiInteger iContainsElementsInstancesCount = sdaiGetMemberCount(sdaiContainsElementsAggr);
	for (SdaiInteger i = 0; i < iContainsElementsInstancesCount; ++i)
	{
		SdaiInstance sdaiContainsElementsInstance = 0;
		engiGetAggrElement(sdaiContainsElementsAggr, i, sdaiINSTANCE, &sdaiContainsElementsInstance);

		if (sdaiGetInstanceType(sdaiContainsElementsInstance) != sdaiRelContainedInSpatialStructureEntity)
		{
			continue;
		}

		TV_INSERTSTRUCT tvInsertStruct;
		tvInsertStruct.hParent = hParent;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = ITEM_CONTAINS;
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_SELECTED;
		tvInsertStruct.item.lParam = NULL;
		HTREEITEM hContains = m_pTreeCtrl->InsertItem(&tvInsertStruct);

		SdaiAggr sdaiRelatedElementsInstances = 0;
		sdaiGetAttrBN(sdaiContainsElementsInstance, "RelatedElements", sdaiAGGR, &sdaiRelatedElementsInstances);

		SdaiInteger iIFCRelatedElementsInstancesCount = sdaiGetMemberCount(sdaiRelatedElementsInstances);
		for (SdaiInteger j = 0; j < iIFCRelatedElementsInstancesCount; ++j)
		{
			SdaiInstance sdaiRelatedElementsInstance = 0;
			engiGetAggrElement(sdaiRelatedElementsInstances, j, sdaiINSTANCE, &sdaiRelatedElementsInstance);

			LoadObject(pModel, sdaiRelatedElementsInstance, hContains);
		}
	} // for (SdaiInteger i = ...
}

void CIFCModelStructureView::LoadObject(_ifc_model* pModel, SdaiInstance sdaiInstance, HTREEITEM hParent)
{
	ASSERT(pModel != nullptr);

	auto pGeometry = pModel->getGeometryByInstance(sdaiInstance);
	if (pGeometry == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	//#todo#mappeditems
	ASSERT(pGeometry->getInstances().size() == 1);
	_ptr<_ifc_instance> ifcInstance(pGeometry->getInstances()[0]);
	if (ifcInstance)
	{
		ASSERT(_ptr<_ifc_geometry>(pGeometry)->getIsReferenced());

		wstring strItem = _ap_instance::getName(sdaiInstance);

		// Object
		TV_INSERTSTRUCT tvInsertStruct;
		tvInsertStruct.hParent = hParent;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = ifcInstance->getEnable() ? IMAGE_SELECTED : IMAGE_NOT_SELECTED;
		tvInsertStruct.item.lParam = NULL;
		HTREEITEM hObject = m_pTreeCtrl->InsertItem(&tvInsertStruct);

		// Geometry
		tvInsertStruct.hParent = hObject;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = ITEM_GEOMETRY;
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage =
			ifcInstance->hasGeometry() ?
			(ifcInstance->getEnable() ? IMAGE_SELECTED : IMAGE_NOT_SELECTED) :
			IMAGE_NO_GEOMETRY;
		tvInsertStruct.item.lParam = (LPARAM)ifcInstance.p();
		HTREEITEM hGeometry = m_pTreeCtrl->InsertItem(&tvInsertStruct);
		m_mapInstance2GeometryItem[ifcInstance.p()] = hGeometry;

		// decomposition/nested/contains
		LoadIsDecomposedBy(pModel, sdaiInstance, hObject);
		LoadIsNestedBy(pModel, sdaiInstance, hObject);
		LoadContainsElements(pModel, sdaiInstance, hObject);
	} // if (ifcInstance)
	else
	{
		ASSERT(FALSE);
	}
}

void CIFCModelStructureView::LoadUnreferencedItems(_ifc_model* pModel, HTREEITEM hModel)
{
	ASSERT(pModel != nullptr);

	map<wstring, vector<_ifc_instance*>> mapUnreferencedItems;
	for (auto pGeometry : pModel->getGeometries())
	{
		if (!pGeometry->hasGeometry())
		{
			continue;
		}

		_ptr<_ifc_geometry> ifcGeometry(pGeometry);

		//#todo#mappeditems
		ASSERT(pGeometry->getInstances().size() == 1);
		_ptr<_ifc_instance> ifcInstance(pGeometry->getInstances()[0]);
		if (!ifcGeometry->getIsReferenced())
		{
			const wchar_t* szEntity = _ap_instance::getEntityName(ifcGeometry->getSdaiInstance());

			auto itUnreferencedItems = mapUnreferencedItems.find(szEntity);
			if (itUnreferencedItems == mapUnreferencedItems.end())
			{
				vector<_ifc_instance*> veCIFCInstances;
				veCIFCInstances.push_back(ifcInstance.p());

				mapUnreferencedItems[szEntity] = veCIFCInstances;
			}
			else
			{
				itUnreferencedItems->second.push_back(ifcInstance.p());
			}
		}
	} // for (auto pGeometry : ...

	if (mapUnreferencedItems.empty())
	{
		return;
	}

	// Unreferenced
	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = hModel;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = L"Unreferenced";
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_SELECTED;
	tvInsertStruct.item.lParam = NULL;
	HTREEITEM hUnreferenced = m_pTreeCtrl->InsertItem(&tvInsertStruct);

	map<wstring, vector<_ifc_instance*>>::iterator itUnreferencedItems = mapUnreferencedItems.begin();
	for (; itUnreferencedItems != mapUnreferencedItems.end(); itUnreferencedItems++)
	{
		// Entity
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

			wstring strItem = _ap_instance::getName(pInstance->getSdaiInstance());

			// Object
			tvInsertStruct.hParent = hEntity;
			tvInsertStruct.hInsertAfter = TVI_LAST;
			tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
			tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
			tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_SELECTED;
			tvInsertStruct.item.lParam = NULL;
			HTREEITEM hObject = m_pTreeCtrl->InsertItem(&tvInsertStruct);

			// Geometry
			tvInsertStruct.hParent = hObject;
			tvInsertStruct.hInsertAfter = TVI_LAST;
			tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
			tvInsertStruct.item.pszText = ITEM_GEOMETRY;
			tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage =
				pInstance->hasGeometry() ? (pInstance->getEnable() ? IMAGE_SELECTED : IMAGE_NOT_SELECTED) : IMAGE_NO_GEOMETRY;
			tvInsertStruct.item.lParam = (LPARAM)pInstance;
			HTREEITEM hGeometry = m_pTreeCtrl->InsertItem(&tvInsertStruct);
			m_mapInstance2GeometryItem[pInstance] = hGeometry;
		} // for (size_t iInstance = ...
	} // for (; itUnreferencedItems != ...
}

void CIFCModelStructureView::LoadTree_UpdateItem(HTREEITEM hParent)
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

		if ((iImage != IMAGE_SELECTED) && 
			(iImage != IMAGE_SEMI_SELECTED) && 
			(iImage != IMAGE_NOT_SELECTED))
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
			case IMAGE_NO_GEOMETRY:
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
		m_pTreeCtrl->SetItemImage(hParent, IMAGE_NO_GEOMETRY, IMAGE_NO_GEOMETRY);

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

void CIFCModelStructureView::ClickItem_UpdateChildren(HTREEITEM hParent)
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

		_ifc_instance* pInstance = (_ifc_instance*)m_pTreeCtrl->GetItemData(hChild);
		if (pInstance != nullptr)
		{
			pInstance->setEnable((iParentImage == IMAGE_SELECTED) || (iParentImage == IMAGE_SEMI_SELECTED) ? true : false);
		}

		ClickItem_UpdateChildren(hChild);

		hChild = m_pTreeCtrl->GetNextSiblingItem(hChild);
	} // while (hChild != nullptr)
}

void CIFCModelStructureView::ClickItem_UpdateParent(HTREEITEM hParent, BOOL bRecursive/* = TRUE*/)
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

		_ifc_instance* pInstance = (_ifc_instance*)m_pTreeCtrl->GetItemData(hParent);
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

	if (iSelectedChidlrenCount == 0)
	{
		m_pTreeCtrl->SetItemImage(hParent, IMAGE_NOT_SELECTED, IMAGE_NOT_SELECTED);

		_ifc_instance* pInstance = (_ifc_instance*)m_pTreeCtrl->GetItemData(hParent);
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

	if (iSelectedChidlrenCount == iChidlrenCount)
	{
		m_pTreeCtrl->SetItemImage(hParent, IMAGE_SELECTED, IMAGE_SELECTED);

		_ifc_instance* pInstance = (_ifc_instance*)m_pTreeCtrl->GetItemData(hParent);
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

	_ifc_instance* pInstance = (_ifc_instance*)m_pTreeCtrl->GetItemData(hParent);
	if (pInstance != nullptr)
	{
		pInstance->setEnable(true);
	}

	if (bRecursive)
	{
		ClickItem_UpdateParent(m_pTreeCtrl->GetParentItem(hParent));
	}
}

void CIFCModelStructureView::UnselectAllItems()
{
	auto itSelectedIInstance = m_mapSelectedInstances.begin();
	for (; itSelectedIInstance != m_mapSelectedInstances.end(); itSelectedIInstance++)
	{
		m_pTreeCtrl->SetItemState(itSelectedIInstance->second, 0, TVIS_BOLD);
	}

	m_mapSelectedInstances.clear();
}

void CIFCModelStructureView::ResetView()
{
	m_mapInstance2GeometryItem.clear();
	m_mapSelectedInstances.clear();

	m_pTreeCtrl->DeleteAllItems();

	for (auto pModel : getController()->getModels())
	{
		LoadModel(_ptr<_ifc_model>(pModel));
	}	
}