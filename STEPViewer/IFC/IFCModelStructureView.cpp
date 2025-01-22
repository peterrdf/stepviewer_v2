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

CIFCModelStructureView::CModelData::CModelData(_ifc_model* pModel, CTreeCtrlEx* pTreeCtrl, HTREEITEM hModel)
	: m_pModel(pModel)
	, m_pTreeCtrl(pTreeCtrl)
	, m_hModel(hModel)
	, m_hProject(NULL)
	, m_hGroups(NULL)
	, m_hSpaceBoundaries(NULL)
	, m_hUnreferenced(NULL)
	, m_mapProject()
	, m_mapModel()
	, m_mapGroups()
	, m_mapSpaceBoundaries()
	, m_mapUnreferenced()
{
	ASSERT(m_pModel != nullptr);
	ASSERT(m_pTreeCtrl != nullptr);
	ASSERT(m_hModel != NULL);
}

/*virtual*/ CIFCModelStructureView::CModelData::~CModelData()
{
}

bool CIFCModelStructureView::CModelData::IsProjectItem(HTREEITEM hItem)
{
	if (hItem == NULL)
	{
		return false;
	}
	else if (hItem == m_hProject)
	{
		return true;
	}

	return IsProjectItem(m_pTreeCtrl->GetParentItem(hItem));
}

bool CIFCModelStructureView::CModelData::IsGroupsItem(HTREEITEM hItem)
{
	if (hItem == NULL)
	{
		return false;
	}
	else if (hItem == m_hGroups)
	{
		return true;
	}

	return IsGroupsItem(m_pTreeCtrl->GetParentItem(hItem));
}

bool CIFCModelStructureView::CModelData::IsSpaceBoundariesItem(HTREEITEM hItem)
{
	if (hItem == NULL)
	{
		return false;
	}
	else if (hItem == m_hSpaceBoundaries)
	{
		return true;
	}

	return IsSpaceBoundariesItem(m_pTreeCtrl->GetParentItem(hItem));
}

bool CIFCModelStructureView::CModelData::IsUnreferencedItem(HTREEITEM hItem)
{
	if (hItem == NULL)
	{
		return false;
	}
	else if (hItem == m_hUnreferenced)
	{
		return true;
	}

	return IsUnreferencedItem(m_pTreeCtrl->GetParentItem(hItem));
}

HTREEITEM CIFCModelStructureView::CModelData::GetModelItem() const
{
	return m_hModel;
}

void CIFCModelStructureView::CModelData::SetProjectItem(HTREEITEM hItem)
{
	ASSERT(hItem != NULL);
	ASSERT(m_hProject == NULL);

	m_hProject = hItem;
}

HTREEITEM CIFCModelStructureView::CModelData::GetProjectItem() const
{
	return m_hProject;
}

void CIFCModelStructureView::CModelData::SetGroupsItem(HTREEITEM hItem)
{
	ASSERT(hItem != NULL);
	ASSERT(m_hGroups == NULL);

	m_hGroups = hItem;
}

HTREEITEM CIFCModelStructureView::CModelData::GetGroupsItem() const
{
	return m_hGroups;
}

void CIFCModelStructureView::CModelData::SetSpaceBoundariesItem(HTREEITEM hItem)
{
	ASSERT(hItem != NULL);
	ASSERT(m_hSpaceBoundaries == NULL);

	m_hSpaceBoundaries = hItem;
}

HTREEITEM CIFCModelStructureView::CModelData::GetSpaceBoundariesItem() const
{
	return m_hSpaceBoundaries;
}

void CIFCModelStructureView::CModelData::SetUnreferencedItem(HTREEITEM hItem)
{
	ASSERT(hItem != NULL);
	ASSERT(m_hUnreferenced == NULL);

	m_hUnreferenced = hItem;
}

HTREEITEM CIFCModelStructureView::CModelData::GetUnreferencedItem() const
{
	return m_hUnreferenced;
}

CIFCModelStructureView::ITEMS& CIFCModelStructureView::CModelData::GetItems(HTREEITEM hItem)
{
	if (IsProjectItem(hItem))
	{
		return m_mapProject;
	}
	else if (IsGroupsItem(hItem))
	{
		return m_mapGroups;
	}
	else if (IsSpaceBoundariesItem(hItem))
	{
		return m_mapSpaceBoundaries;
	}
	else if (IsUnreferencedItem(hItem)) 
	{
		return m_mapUnreferenced;
	}

	ASSERT(hItem == m_hModel);

	return m_mapModel;
}

// ************************************************************************************************
CIFCModelStructureView::CIFCModelStructureView(CTreeCtrlEx* pTreeCtrl)
	: CModelStructureViewBase(pTreeCtrl)
	, m_pImageList(nullptr)
	, m_vecModelData()
	, m_hModel(NULL)
	, m_pSelectedInstance(nullptr)
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

	for (auto pModelData : m_vecModelData)
	{
		delete pModelData;
	}
	m_vecModelData.clear();
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

	m_pSelectedInstance = getController()->getSelectedInstance() != nullptr ?
		dynamic_cast<_ifc_instance*>(getController()->getSelectedInstance()) : 
		nullptr;

	if (m_pSelectedInstance != nullptr)
	{
		auto pModel = pController->getModelByInstance(m_pSelectedInstance->getOwlModel());
		ASSERT(pModel != nullptr);

		auto pModelData = Model_GetData(pModel);
		ASSERT(pModelData != nullptr);

		Tree_EnsureVisible(pModelData, m_pSelectedInstance);
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

		auto pModelData = Model_GetData(hItem);
		ASSERT(pModelData != nullptr);

		ITEMS& mapItems = pModelData->GetItems(hItem);

		switch (iImage)
		{
			case IMAGE_SELECTED:
			case IMAGE_SEMI_SELECTED:
			{
				bool bGeometryItem = false;
				_ifc_instance* pInstance = (_ifc_instance*)m_pTreeCtrl->GetItemData(hItem);
				if ((pInstance == nullptr) &&
					(iImage == IMAGE_SELECTED) &&
					!m_pTreeCtrl->ItemHasChildren(hItem) &&
					(m_pTreeCtrl->GetItemText(hItem) == ITEM_GEOMETRY))
				{
					HTREEITEM hParent = m_pTreeCtrl->GetParentItem(hItem);
					ASSERT(hParent != NULL);

					pInstance = (_ifc_instance*)m_pTreeCtrl->GetItemData(hParent);

					bGeometryItem = true;
				}

				//
				// Model
				//

				set<_ifc_instance*> setInstances;
				if (pInstance != nullptr)
				{
					pInstance->setEnable(false);

					setInstances.insert(pInstance);
				}				
				
				if (!bGeometryItem)
				{
					Model_EnableChildren(hItem, false, setInstances);
				}				

				//
				// UI
				//
				
				if (pInstance != nullptr)
				{
					//
					// Instance
					//
					
					auto itItems = mapItems.find(pInstance);
					ASSERT(itItems != mapItems.end());

					for (auto hInstance : itItems->second)
					{
						if (bGeometryItem)
						{
							HTREEITEM hGeometry = m_pTreeCtrl->GetChildItem(hInstance);
							ASSERT((hGeometry != nullptr) && !m_pTreeCtrl->ItemHasChildren(hGeometry) && (m_pTreeCtrl->GetItemText(hGeometry) == ITEM_GEOMETRY));

							m_pTreeCtrl->SetItemImage(hGeometry, IMAGE_NOT_SELECTED, IMAGE_NOT_SELECTED);

							Tree_UpdateChildren(hGeometry);
							Tree_UpdateParents(m_pTreeCtrl->GetParentItem(hGeometry));
						}
						else
						{
							m_pTreeCtrl->SetItemImage(hInstance, IMAGE_NOT_SELECTED, IMAGE_NOT_SELECTED);

							Tree_UpdateChildren(hInstance);
							Tree_UpdateParents(m_pTreeCtrl->GetParentItem(hInstance));
						}						
					}
				}
				else
				{
					//
					// Item
					//

					m_pTreeCtrl->SetItemImage(hItem, IMAGE_NOT_SELECTED, IMAGE_NOT_SELECTED);

					Tree_UpdateChildren(hItem);
					Tree_UpdateParents(m_pTreeCtrl->GetParentItem(hItem));
				}

				if (pModelData->IsProjectItem(hItem))
				{
					Tree_Update(pModelData->GetGroupsItem(), pModelData->GetGroupsItems(), setInstances);
				}
				else if (pModelData->IsGroupsItem(hItem))
				{
					Tree_Update(pModelData->GetProjectItem(), pModelData->GetProjectItems(), setInstances);
				}

				pController->onInstancesEnabledStateChanged(this);
			}
			break;

			case IMAGE_NOT_SELECTED:
			{
				bool bGeometryItem = false;
				_ifc_instance* pInstance = (_ifc_instance*)m_pTreeCtrl->GetItemData(hItem);
				if ((pInstance == nullptr) &&
					!m_pTreeCtrl->ItemHasChildren(hItem) &&
					(m_pTreeCtrl->GetItemText(hItem) == ITEM_GEOMETRY))
				{
					HTREEITEM hParent = m_pTreeCtrl->GetParentItem(hItem);
					ASSERT(hParent != NULL);

					pInstance = (_ifc_instance*)m_pTreeCtrl->GetItemData(hParent);

					bGeometryItem = true;
				}

				//
				// Model
				//

				set<_ifc_instance*> setInstances;
				if (pInstance != nullptr)
				{
					pInstance->setEnable(true);

					setInstances.insert(pInstance);
				}

				if (!bGeometryItem)
				{					
					Model_EnableChildren(hItem, true, setInstances);
				}				

				//
				// UI
				//

				if (pInstance != nullptr)
				{
					//
					// Instance/Geometry
					//

					auto itItems = mapItems.find(pInstance);
					ASSERT(itItems != mapItems.end());

					for (auto hInstance : itItems->second)
					{
						if (bGeometryItem)
						{
							HTREEITEM hGeometry = m_pTreeCtrl->GetChildItem(hInstance);
							ASSERT((hGeometry != nullptr) && !m_pTreeCtrl->ItemHasChildren(hGeometry) && (m_pTreeCtrl->GetItemText(hGeometry) == ITEM_GEOMETRY));

							m_pTreeCtrl->SetItemImage(hGeometry, IMAGE_SELECTED, IMAGE_SELECTED);

							Tree_UpdateChildren(hGeometry);
							Tree_UpdateParents(m_pTreeCtrl->GetParentItem(hGeometry));
						}
						else
						{
							m_pTreeCtrl->SetItemImage(hInstance, IMAGE_SELECTED, IMAGE_SELECTED);

							Tree_UpdateChildren(hInstance);
							Tree_UpdateParents(m_pTreeCtrl->GetParentItem(hInstance));
						}						
					} // for (auto hInstance : ...
				}
				else
				{
					//
					// Item
					//

					m_pTreeCtrl->SetItemImage(hItem, IMAGE_SELECTED, IMAGE_SELECTED);

					Tree_UpdateChildren(hItem);
					Tree_UpdateParents(m_pTreeCtrl->GetParentItem(hItem));
				}

				if (pModelData->IsProjectItem(hItem))
				{
					Tree_Update(pModelData->GetGroupsItem(), pModelData->GetGroupsItems(), setInstances);
				}
				else if (pModelData->IsGroupsItem(hItem))
				{
					Tree_Update(pModelData->GetProjectItem(), pModelData->GetProjectItems(), setInstances);
				}

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
		m_pSelectedInstance = m_pTreeCtrl->GetItemData(hItem) != NULL ?
			(_ifc_instance*)m_pTreeCtrl->GetItemData(hItem) :
			nullptr;

		pController->selectInstance(this, m_pSelectedInstance);
	}
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
		return;
	}

	auto pModelData = Model_GetData(hItem);
	ASSERT(pModelData != nullptr);

	ITEMS& mapItems = pModelData->GetItems(hItem);
		
	// ENTITY : VISIBLE COUNT
	map<wstring, long> mapEntity2VisibleCount;
	for (auto pGeometry : pModelData->GetModel()->getGeometries())
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
					pController->saveSelectedInstance();
				}
				break;

				case ID_INSTANCES_ENABLE:
				{
					//
					// Model
					//

					pTargetInstance->setEnable(!pTargetInstance->getEnable());

					set<_ifc_instance*> setInstances{ pTargetInstance };

					//
					// UI
					//

					HTREEITEM hGeometry = m_pTreeCtrl->GetChildItem(hItem);
					ASSERT((hGeometry != nullptr) && !m_pTreeCtrl->ItemHasChildren(hGeometry) && (m_pTreeCtrl->GetItemText(hGeometry) == ITEM_GEOMETRY));

					int iImage = pTargetInstance->getEnable() ? IMAGE_SELECTED : IMAGE_NOT_SELECTED;

					m_pTreeCtrl->SetItemImage(hGeometry, iImage, iImage);

					Tree_UpdateChildren(hGeometry);
					Tree_UpdateParents(m_pTreeCtrl->GetParentItem(hGeometry));

					auto itItems = mapItems.find(pTargetInstance);
					ASSERT(itItems != mapItems.end());

					for (auto hInstance : itItems->second)
					{
						m_pTreeCtrl->SetItemImage(hInstance, iImage, iImage);

						Tree_UpdateChildren(hInstance);
						Tree_UpdateParents(m_pTreeCtrl->GetParentItem(hInstance));
					}

					if (pModelData->IsProjectItem(hItem))
					{
						Tree_Update(pModelData->GetGroupsItem(), pModelData->GetGroupsItems(), setInstances);
					}
					else if (pModelData->IsGroupsItem(hItem))
					{
						Tree_Update(pModelData->GetProjectItem(), pModelData->GetProjectItems(), setInstances);
					}

					pController->onInstancesEnabledStateChanged(this);
				}
				break;

				case ID_INSTANCES_DISABLE_ALL_BUT_THIS:
				{
					//
					// Model
					//

					for (auto pInstance : pModelData->GetModel()->getInstances())
					{
						pInstance->setEnable(pTargetInstance == pInstance);
					}

					set<_ifc_instance*> setInstances{ pTargetInstance };

					//
					// UI
					//

					Tree_Reset(false);

					HTREEITEM hGeometry = m_pTreeCtrl->GetChildItem(hItem);
					ASSERT((hGeometry != nullptr) && !m_pTreeCtrl->ItemHasChildren(hGeometry) && (m_pTreeCtrl->GetItemText(hGeometry) == ITEM_GEOMETRY));

					m_pTreeCtrl->SetItemImage(hGeometry, IMAGE_SELECTED, IMAGE_SELECTED);

					Tree_UpdateChildren(hGeometry);
					Tree_UpdateParents(m_pTreeCtrl->GetParentItem(hGeometry));

					if (pModelData->IsProjectItem(hItem))
					{
						Tree_Update(pModelData->GetGroupsItem(), pModelData->GetGroupsItems(), setInstances);
					}
					else if (pModelData->IsGroupsItem(hItem))
					{
						Tree_Update(pModelData->GetProjectItem(), pModelData->GetProjectItems(), setInstances);
					}

					pController->onInstancesEnabledStateChanged(this);
				}
				break;

				case ID_INSTANCES_ENABLE_ALL:
				{
					for (auto pInstance : pModelData->GetModel()->getInstances())
					{
						pInstance->setEnable(true);
					}

					Tree_Reset(true);

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

		//
		// Model
		//

		set<_ifc_instance*> setInstances;
		for (auto pInstance : pModelData->GetModel()->getInstances())
		{
			//#todo#mappeditems		
			_ptr<_ifc_instance> ifcInstance(pInstance);

			if (ifcInstance->getEntityName() == itCommand2Entity->second)
			{
				ifcInstance->setEnable(itEntity2VisibleCount->second > 0 ? false : true);

				setInstances.insert(ifcInstance);
			}
		} // for (auto pInstance = ...

		//
		// UI
		//

		Tree_Update(pModelData->GetProjectItem(), pModelData->GetProjectItems(), setInstances);
		Tree_Update(pModelData->GetGroupsItem(), pModelData->GetGroupsItems(), setInstances);
		Tree_Update(pModelData->GetSpaceBoundariesItem(), pModelData->GetSpaceBoundariesItems(), setInstances);
		Tree_Update(pModelData->GetUnreferencedItem(), pModelData->GetUnreferencedItems(), setInstances);

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
	m_hModel = m_pTreeCtrl->InsertItem(&tvInsertStruct);

	auto pModelData = new CModelData(pModel, m_pTreeCtrl, m_hModel);
	m_vecModelData.push_back(pModelData);

	// Header
	LoadHeader(m_hModel);

	// Project/Units/Unreferenced
	SdaiAggr sdaiProjectAggr = sdaiGetEntityExtentBN(pModel->getSdaiModel(), "IFCPROJECT");

	SdaiInteger iProjectInstancesCount = sdaiGetMemberCount(sdaiProjectAggr);
	if (iProjectInstancesCount > 0)
	{
		SdaiInstance sdaiProjectInstance = 0;
		engiGetAggrElement(sdaiProjectAggr, 0, sdaiINSTANCE, &sdaiProjectInstance);

		// Load
		LoadProject(pModelData, m_hModel, sdaiProjectInstance, pModelData->GetProjectItems());
		LoadGroups(pModelData, m_hModel, pModelData->GetGroupsItems());
		//LoadSpaceBoundaries(pModelData, m_hModel, pModelData->GetSpaceBoundariesItems()); // TEST
		LoadUnreferencedItems(pModelData, m_hModel, pModelData->GetUnreferencedItems());

		// Update UI
		Tree_Update(m_hModel);
	} // if (iProjectInstancesCount > 0)

	m_pTreeCtrl->Expand(m_hModel, TVE_EXPAND);
}

void CIFCModelStructureView::LoadProject(CModelData* pModelData, HTREEITEM hModel, SdaiInstance sdaiProjectInstance, ITEMS& mapItems)
{
	auto pController = getController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return;
	}	

	ASSERT(pModelData != nullptr);

	auto pGeometry = pModelData->GetModel()->getGeometryByInstance(sdaiProjectInstance);
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
		tvInsertStruct.item.lParam = (LPARAM)ifcInstance.p();
		HTREEITEM hProject = m_pTreeCtrl->InsertItem(&tvInsertStruct);
		pModelData->SetProjectItem(hProject);

		// Geometry
		tvInsertStruct.hParent = hProject;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = ITEM_GEOMETRY;
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_NO_GEOMETRY;
		tvInsertStruct.item.lParam = NULL;
		m_pTreeCtrl->InsertItem(&tvInsertStruct);

		ASSERT(mapItems.find(ifcInstance) == mapItems.end());
		mapItems[ifcInstance] = vector<HTREEITEM>{ hProject };

		// decomposition/contains
		LoadIsDecomposedBy(pModelData->GetModel(), sdaiProjectInstance, hProject, mapItems);
		LoadIsNestedBy(pModelData->GetModel(), sdaiProjectInstance, hProject, mapItems);
		LoadContainsElements(pModelData->GetModel(), sdaiProjectInstance, hProject, mapItems);

		m_pTreeCtrl->Expand(hProject, TVE_EXPAND);
	} // if (itInstance != ...
}

void CIFCModelStructureView::LoadIsDecomposedBy(_ifc_model* pModel, SdaiInstance sdaiInstance, HTREEITEM hParent, ITEMS& mapItems)
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

			LoadInstance(pModel, sdaiRelatedObjectsInstance, hDecomposition, mapItems);
		}
	} // for (SdaiInteger i = ...	
}

void CIFCModelStructureView::LoadIsNestedBy(_ifc_model* pModel, SdaiInstance sdaiInstance, HTREEITEM hParent, ITEMS& mapItems)
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

			LoadInstance(pModel, sdaiRelatedObjectsInstance, hDecomposition, mapItems);
		}
	} // for (SdaiInteger i = ...
}

void CIFCModelStructureView::LoadContainsElements(_ifc_model* pModel, SdaiInstance sdaiInstance, HTREEITEM hParent, ITEMS& mapItems)
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

			LoadInstance(pModel, sdaiRelatedElementsInstance, hContains, mapItems);
		}
	} // for (SdaiInteger i = ...
}

void CIFCModelStructureView::LoadBoundedBy(_ifc_model* pModel, SdaiInstance sdaiInstance, HTREEITEM hParent, ITEMS& mapItems)
{
	ASSERT(pModel != nullptr);

	SdaiAggr sdaiContainsElementsAggr = nullptr;
	sdaiGetAttrBN(sdaiInstance, "BoundedBy", sdaiAGGR, &sdaiContainsElementsAggr);

	if (sdaiContainsElementsAggr == nullptr)
	{
		return;
	}

	SdaiEntity sdaiRelSpaceBoundaryEntity = sdaiGetEntity(pModel->getSdaiModel(), "IFCRELSPACEBOUNDARY");

	SdaiInteger iBoundedByInstancesCount = sdaiGetMemberCount(sdaiContainsElementsAggr);
	for (SdaiInteger i = 0; i < iBoundedByInstancesCount; ++i)
	{
		SdaiInstance sdaiBoundedByInstance = 0;
		engiGetAggrElement(sdaiContainsElementsAggr, i, sdaiINSTANCE, &sdaiBoundedByInstance);

		if (sdaiGetInstanceType(sdaiBoundedByInstance) != sdaiRelSpaceBoundaryEntity)
		{
			continue;
		}

		LoadInstance(pModel, sdaiBoundedByInstance, hParent, mapItems);
	}
}

HTREEITEM CIFCModelStructureView::LoadInstance(_ifc_model* pModel, SdaiInstance sdaiInstance, HTREEITEM hParent, ITEMS& mapItems, bool bLoadChildren/* = true*/)
{
	ASSERT(pModel != nullptr);

	auto pGeometry = pModel->getGeometryByInstance(sdaiInstance);
	if (pGeometry == nullptr)
	{
		ASSERT(FALSE);

		return NULL;
	}

	//#todo#mappeditems
	ASSERT(pGeometry->getInstances().size() == 1);

	_ptr<_ifc_instance> ifcInstance(pGeometry->getInstances()[0]);
	if (!ifcInstance)
	{
		ASSERT(FALSE);

		return NULL;
	}
		
	ASSERT(_ptr<_ifc_geometry>(pGeometry)->getIsReferenced());

	wstring strItem = _ap_instance::getName(sdaiInstance);

	// Instance
	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = hParent;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = ifcInstance->getEnable() ? IMAGE_SELECTED : IMAGE_NOT_SELECTED;
	tvInsertStruct.item.lParam = (LPARAM)ifcInstance.p();
	HTREEITEM hInstance = m_pTreeCtrl->InsertItem(&tvInsertStruct);

	// Geometry
	tvInsertStruct.hParent = hInstance;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = ITEM_GEOMETRY;
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage =
		ifcInstance->hasGeometry() ?
		(ifcInstance->getEnable() ? IMAGE_SELECTED : IMAGE_NOT_SELECTED) :
		IMAGE_NO_GEOMETRY;
	tvInsertStruct.item.lParam = NULL;
	m_pTreeCtrl->InsertItem(&tvInsertStruct);

	auto itItems = mapItems.find(ifcInstance);
	if (itItems != mapItems.end())
	{
		itItems->second.push_back(hInstance);
	}
	else
	{
		mapItems[ifcInstance] = vector<HTREEITEM>{ hInstance };
	}

	if (bLoadChildren)
	{
		LoadIsDecomposedBy(pModel, sdaiInstance, hInstance, mapItems);
		LoadIsNestedBy(pModel, sdaiInstance, hInstance, mapItems);
		LoadContainsElements(pModel, sdaiInstance, hInstance, mapItems);
	}		

	return hInstance;	
}

void CIFCModelStructureView::LoadGroups(CModelData* pModelData, HTREEITEM hModel, ITEMS& mapItems)
{
	ASSERT(pModelData != nullptr);

	vector<_ap_geometry*> vecGeometries;
	pModelData->GetModel()->getGeometriesByType("IFCGROUP", vecGeometries);

	if (vecGeometries.empty())
	{
		return;
	}

	// Groups
	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = hModel;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = L"Groups";
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_SELECTED;
	tvInsertStruct.item.lParam = NULL;
	HTREEITEM hGroups = m_pTreeCtrl->InsertItem(&tvInsertStruct);
	pModelData->SetGroupsItem(hGroups);

	for (auto pGeometry : vecGeometries)
	{
		ASSERT(pGeometry != nullptr);

		//#todo#mappeditems
		_ptr<_ifc_geometry> ifcGeometry(pGeometry);		
		ASSERT(pGeometry->getInstances().size() == 1);

		_ptr<_ifc_instance> ifcInstance(pGeometry->getInstances()[0]);
		ASSERT(ifcInstance);

		wstring strItem = _ap_instance::getName(ifcInstance->getSdaiInstance());

		// Instance
		tvInsertStruct.hParent = hGroups;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_SELECTED;
		tvInsertStruct.item.lParam = (LPARAM)ifcInstance.p();
		HTREEITEM hInstance = m_pTreeCtrl->InsertItem(&tvInsertStruct);

		// Geometry
		tvInsertStruct.hParent = hInstance;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = ITEM_GEOMETRY;
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage =
			ifcInstance->hasGeometry() ? (ifcInstance->getEnable() ? IMAGE_SELECTED : IMAGE_NOT_SELECTED) : IMAGE_NO_GEOMETRY;
		tvInsertStruct.item.lParam = NULL;
		m_pTreeCtrl->InsertItem(&tvInsertStruct);

		assert(mapItems.find(ifcInstance) == mapItems.end());
		mapItems[ifcInstance] = vector<HTREEITEM>{ hInstance };
		
		SdaiInstance sdaiIsGroupedByInstance = 0;
		sdaiGetAttrBN(ifcInstance->getSdaiInstance(), "IsGroupedBy", sdaiINSTANCE, &sdaiIsGroupedByInstance);
		if (sdaiIsGroupedByInstance != 0)
		{
			SdaiAggr sdaiRelatedObjectsAggr = nullptr;
			sdaiGetAttrBN(sdaiIsGroupedByInstance, "RelatedObjects", sdaiAGGR, &sdaiRelatedObjectsAggr);

			SdaiInteger iRelatedObjectsCount = sdaiGetMemberCount(sdaiRelatedObjectsAggr);
			for (SdaiInteger i = 0; i < iRelatedObjectsCount; i++)
			{
				SdaiInstance sdaiRelatedObject = 0;
				sdaiGetAggrByIndex(sdaiRelatedObjectsAggr, i, sdaiINSTANCE, &sdaiRelatedObject);

				LoadInstance(pModelData->GetModel(), sdaiRelatedObject, hInstance, mapItems);
			} // for (SdaiInteger i = ...
		} // if (sdaiIsGroupedByInstance != 0)
	} // for (SdaiInteger iGroupInstance = ...
}

void CIFCModelStructureView::LoadSpaceBoundaries(CModelData* pModelData, HTREEITEM hModel, ITEMS& mapItems)
{
	ASSERT(pModelData != nullptr);

	if (sdaiGetMemberCount(sdaiGetEntityExtentBN(pModelData->GetModel()->getSdaiModel(), "IFCRELSPACEBOUNDARY")) == 0)
	{
		return;
	}

	vector<_ap_geometry*> vecGeometries;
	pModelData->GetModel()->getGeometriesByType("IFCBUILDINGSTOREY", vecGeometries);

	if (vecGeometries.empty())
	{
		return;
	}	
	
	// Space Boundaries
	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = hModel;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = L"Space Boundaries";
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_SELECTED;
	tvInsertStruct.item.lParam = NULL;
	HTREEITEM hSpaceBoundaries = m_pTreeCtrl->InsertItem(&tvInsertStruct);
	pModelData->SetSpaceBoundariesItem(hSpaceBoundaries);

	for (auto pGeometry : vecGeometries)
	{
		ASSERT(pGeometry != nullptr);

		//#todo#mappeditems
		_ptr<_ifc_geometry> ifcGeometry(pGeometry);
		ASSERT(pGeometry->getInstances().size() == 1);

		_ptr<_ifc_instance> ifcInstance(pGeometry->getInstances()[0]);
		ASSERT(ifcInstance);

		wstring strItem = _ap_instance::getName(ifcInstance->getSdaiInstance());

		// Instance
		tvInsertStruct.hParent = hSpaceBoundaries;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_SELECTED;
		tvInsertStruct.item.lParam = NULL;
		HTREEITEM hInstance = m_pTreeCtrl->InsertItem(&tvInsertStruct);

		auto itItems = mapItems.find(ifcInstance);
		if (itItems != mapItems.end())
		{
			itItems->second.push_back(hInstance);
		}
		else
		{
			mapItems[ifcInstance] = vector<HTREEITEM>{ hInstance };
		}

		LoadBuildingStoreyChildren(pModelData->GetModel(), ifcInstance->getSdaiInstance(), hInstance, mapItems);
	} // for (SdaiInteger iBuildingStoreyInstance = ...
}

void CIFCModelStructureView::LoadBuildingStoreyChildren(_ifc_model* pModel, SdaiInstance sdaiInstance, HTREEITEM hBuildingStorey, ITEMS& mapItems)
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

		SdaiAggr sdaiRelatedObjectsAggr = 0;
		sdaiGetAttrBN(sdaiIsDecomposedByInstance, "RelatedObjects", sdaiAGGR, &sdaiRelatedObjectsAggr);

		SdaiInteger iRelatedObjectsInstancesCount = sdaiGetMemberCount(sdaiRelatedObjectsAggr);
		for (SdaiInteger j = 0; j < iRelatedObjectsInstancesCount; ++j)
		{
			SdaiInstance sdaiRelatedObjectsInstance = 0;
			engiGetAggrElement(sdaiRelatedObjectsAggr, j, sdaiINSTANCE, &sdaiRelatedObjectsInstance);

			HTREEITEM hSpace = LoadInstance(pModel, sdaiRelatedObjectsInstance, hBuildingStorey, mapItems, false);
			ASSERT(hSpace != NULL);

			LoadBoundedBy(pModel, sdaiRelatedObjectsInstance, hSpace, mapItems);
		}
	} // for (SdaiInteger i = ...	
}

void CIFCModelStructureView::LoadUnreferencedItems(CModelData* pModelData, HTREEITEM hModel, ITEMS& mapItems)
{
	ASSERT(pModelData != nullptr);

	map<wstring, vector<_ifc_instance*>> mapUnreferencedItems;
	for (auto pGeometry : pModelData->GetModel()->getGeometries())
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
	pModelData->SetUnreferencedItem(hUnreferenced);

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

			// Instance
			tvInsertStruct.hParent = hEntity;
			tvInsertStruct.hInsertAfter = TVI_LAST;
			tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
			tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
			tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_SELECTED;
			tvInsertStruct.item.lParam = (LPARAM)pInstance;
			HTREEITEM hInstance = m_pTreeCtrl->InsertItem(&tvInsertStruct);

			// Geometry
			tvInsertStruct.hParent = hInstance;
			tvInsertStruct.hInsertAfter = TVI_LAST;
			tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
			tvInsertStruct.item.pszText = ITEM_GEOMETRY;
			tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage =
				pInstance->hasGeometry() ? (pInstance->getEnable() ? IMAGE_SELECTED : IMAGE_NOT_SELECTED) : IMAGE_NO_GEOMETRY;
			tvInsertStruct.item.lParam = NULL;
			m_pTreeCtrl->InsertItem(&tvInsertStruct);

			assert(mapItems.find(pInstance) == mapItems.end());
			mapItems[pInstance] = vector<HTREEITEM>{ hInstance };
		} // for (size_t iInstance = ...
	} // for (; itUnreferencedItems != ...
}

CIFCModelStructureView::CModelData* CIFCModelStructureView::Model_GetData(HTREEITEM hItem)
{
	HTREEITEM hModel = Tree_GetModelItem(hItem);
	if (hModel == NULL)
	{
		ASSERT(FALSE);

		return nullptr;
	}

	auto itModelData = find_if(m_vecModelData.begin(), m_vecModelData.end(), [&](CModelData* pModelData)
		{
			return pModelData->GetModelItem() == hModel;
		});

	if (itModelData != m_vecModelData.end())
	{
		return *itModelData;
	}

	return nullptr;
}

CIFCModelStructureView::CModelData* CIFCModelStructureView::Model_GetData(_model* pModel)
{
	ASSERT(pModel != nullptr);

	auto itModelData = find_if(m_vecModelData.begin(), m_vecModelData.end(), [&](CModelData* pModelData)
		{
			return pModelData->GetModel() == pModel;
		});

	if (itModelData != m_vecModelData.end())
	{
		return *itModelData;
	}

	return nullptr;
}

void CIFCModelStructureView::Model_EnableChildren(HTREEITEM hItem, bool bEnable, set<_ifc_instance*>& setChildren)
{
	if (hItem == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	if (!m_pTreeCtrl->ItemHasChildren(hItem))
	{
		return;
	}

	HTREEITEM hChild = m_pTreeCtrl->GetNextItem(hItem, TVGN_CHILD);
	while (hChild != nullptr)
	{
		int iImage, iSelectedImage = -1;
		m_pTreeCtrl->GetItemImage(hChild, iImage, iSelectedImage);

		ASSERT(iImage == iSelectedImage);

		_ifc_instance* pInstance = (_ifc_instance*)m_pTreeCtrl->GetItemData(hChild);
		if (pInstance != nullptr)
		{
			pInstance->setEnable(bEnable);

			setChildren.insert(pInstance);
		}

		Model_EnableChildren(hChild, bEnable, setChildren);

		hChild = m_pTreeCtrl->GetNextSiblingItem(hChild);
	} // while (hChild != nullptr)
}

HTREEITEM CIFCModelStructureView::Tree_GetModelItem(HTREEITEM hItem) const
{
	ASSERT(hItem != NULL);

	HTREEITEM hParent = m_pTreeCtrl->GetParentItem(hItem);
	if (hParent == NULL)
	{
		return hItem;
	}

	return Tree_GetModelItem(hParent);
}

void CIFCModelStructureView::Tree_Update(HTREEITEM hItem, bool bRecursive/* = true*/)
{
	ASSERT(hItem != nullptr);

	if (!m_pTreeCtrl->ItemHasChildren(hItem))
	{
		// keep the state as it is
		return;
	}

	int iChildrenCount = 0;
	int iSelectedChildrenCount = 0;
	int iSemiSelectedChildrenCount = 0;
	int iNoGeometryChildrenCount = 0;

	HTREEITEM hChild = m_pTreeCtrl->GetNextItem(hItem, TVGN_CHILD);
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

		if (bRecursive)
		{
			Tree_Update(hChild);
		}

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

			case IMAGE_NO_GEOMETRY:
			{
				iNoGeometryChildrenCount++;
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

	if (iChildrenCount == 0) /*Instance*/
	{
		m_pTreeCtrl->SetItemImage(hItem, IMAGE_NO_GEOMETRY, IMAGE_NO_GEOMETRY);

		return;
	}

	if (iChildrenCount == iNoGeometryChildrenCount) /*contains/decomposition*/
	{
		m_pTreeCtrl->SetItemImage(hItem, IMAGE_NO_GEOMETRY, IMAGE_NO_GEOMETRY);

		return;
	}

	if (iSemiSelectedChildrenCount > 0)
	{
		m_pTreeCtrl->SetItemImage(hItem, IMAGE_SEMI_SELECTED, IMAGE_SEMI_SELECTED);

		return;
	}

	if (iSelectedChildrenCount == 0)
	{
		m_pTreeCtrl->SetItemImage(hItem, IMAGE_NOT_SELECTED, IMAGE_NOT_SELECTED);

		return;
	}

	if (iChildrenCount == iSelectedChildrenCount)
	{
		m_pTreeCtrl->SetItemImage(hItem, IMAGE_SELECTED, IMAGE_SELECTED);

		return;
	}

	if ((iChildrenCount - iNoGeometryChildrenCount) == iSelectedChildrenCount)
	{
		m_pTreeCtrl->SetItemImage(hItem, IMAGE_SELECTED, IMAGE_SELECTED);

		return;
	}

	ASSERT(iChildrenCount > iSelectedChildrenCount);

	m_pTreeCtrl->SetItemImage(hItem, IMAGE_SEMI_SELECTED, IMAGE_SEMI_SELECTED);
}

void CIFCModelStructureView::Tree_Update(HTREEITEM hItem, ITEMS& mapItems, const set<_ifc_instance*>& setInstances)
{
	if (hItem == NULL)
	{
		return;
	}

	vector<HTREEITEM> vecItems;
	for (auto pInstance : setInstances)
	{
		auto itItems = mapItems.find(pInstance);
		if (itItems == mapItems.end())
		{
			continue;
		}

		for (auto hInstance : itItems->second)
		{
			HTREEITEM hGeometry = m_pTreeCtrl->GetChildItem(hInstance);
			ASSERT((hGeometry != nullptr) && !m_pTreeCtrl->ItemHasChildren(hGeometry) && (m_pTreeCtrl->GetItemText(hGeometry) == ITEM_GEOMETRY));

			int iImage = pInstance->getEnable() ? IMAGE_SELECTED : IMAGE_NOT_SELECTED;

			m_pTreeCtrl->SetItemImage(hGeometry, iImage, iImage);

			vecItems.push_back(hGeometry);
		}
	} // for (auto pInstance : ...	

	if (vecItems.empty())
	{
		return;
	}

	if (vecItems.size() == 1)
	{
		Tree_UpdateChildren(vecItems.front());
		Tree_UpdateParents(m_pTreeCtrl->GetParentItem(vecItems.front()));
	}
	else
	{
		Tree_Update(hItem);
	}

	Tree_Update(m_hModel, false);
}

void CIFCModelStructureView::Tree_UpdateChildren(HTREEITEM hItem)
{
	if (hItem == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	if (!m_pTreeCtrl->ItemHasChildren(hItem))
	{
		return;
	}

	int iParentImage = -1;
	int iParentSelectedImage = -1;
	m_pTreeCtrl->GetItemImage(hItem, iParentImage, iParentSelectedImage);

	ASSERT(iParentImage == iParentSelectedImage);

	HTREEITEM hChild = m_pTreeCtrl->GetNextItem(hItem, TVGN_CHILD);
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

		Tree_UpdateChildren(hChild);

		hChild = m_pTreeCtrl->GetNextSiblingItem(hChild);
	} // while (hChild != nullptr)
}

void CIFCModelStructureView::Tree_UpdateParents(HTREEITEM hItem)
{
	if (hItem == nullptr)
	{
		return;
	}

	ASSERT(m_pTreeCtrl->ItemHasChildren(hItem));

	int iChildrenCount = 0;
	int iSelectedChildrenCount = 0;
	int iSemiSelectedChildrenCount = 0;

	HTREEITEM hChild = m_pTreeCtrl->GetNextItem(hItem, TVGN_CHILD);
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
		m_pTreeCtrl->SetItemImage(hItem, IMAGE_SEMI_SELECTED, IMAGE_SEMI_SELECTED);

		Tree_UpdateParents(m_pTreeCtrl->GetParentItem(hItem));
	}
	else if (iSelectedChildrenCount == 0)
	{
		m_pTreeCtrl->SetItemImage(hItem, IMAGE_NOT_SELECTED, IMAGE_NOT_SELECTED);

		Tree_UpdateParents(m_pTreeCtrl->GetParentItem(hItem));
	}
	else if (iSelectedChildrenCount == iChildrenCount)
	{
		m_pTreeCtrl->SetItemImage(hItem, IMAGE_SELECTED, IMAGE_SELECTED);

		Tree_UpdateParents(m_pTreeCtrl->GetParentItem(hItem));
	}
	else
	{
		ASSERT(iSelectedChildrenCount < iChildrenCount);

		m_pTreeCtrl->SetItemImage(hItem, IMAGE_SEMI_SELECTED, IMAGE_SEMI_SELECTED);

		Tree_UpdateParents(m_pTreeCtrl->GetParentItem(hItem));
	}
}

void CIFCModelStructureView::Tree_Reset(bool bEnable)
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

		Tree_Reset(hRoot, bEnable);

		hRoot = m_pTreeCtrl->GetNextSiblingItem(hRoot);
	}
}

bool CIFCModelStructureView::Tree_EnsureVisible(CModelData* pModelData, _ifc_instance* pInstance)
{
	ASSERT(pModelData != nullptr);

	return 
		Tree_EnsureVisible(pInstance, pModelData->GetProjectItems()) ||
		Tree_EnsureVisible(pInstance, pModelData->GetGroupsItems()) ||
		Tree_EnsureVisible(pInstance, pModelData->GetSpaceBoundariesItems()) ||
		Tree_EnsureVisible(pInstance, pModelData->GetUnreferencedItems());
}

bool CIFCModelStructureView::Tree_EnsureVisible(_ifc_instance* pInstance, ITEMS& mapItems)
{
	if (pInstance == nullptr)
	{
		return false;
	}

	auto itItems = mapItems.find(pInstance);
	if (itItems != mapItems.end())
	{ 
		m_pTreeCtrl->EnsureVisible(itItems->second.front());

		return true;
	}

	return false;
}

void CIFCModelStructureView::Tree_Reset(HTREEITEM hItem, bool bEnable)
{
	if (hItem == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	HTREEITEM hChild = m_pTreeCtrl->GetNextItem(hItem, TVGN_CHILD);
	while (hChild != nullptr)
	{
		Tree_Reset(hChild, bEnable);

		hChild = m_pTreeCtrl->GetNextSiblingItem(hChild);
	} // while (hChild != nullptr)

	int iParentImage = -1;
	int iParentSelectedImage = -1;
	m_pTreeCtrl->GetItemImage(hItem, iParentImage, iParentSelectedImage);

	ASSERT(iParentImage == iParentSelectedImage);

	if ((iParentImage == IMAGE_SELECTED) || (iParentImage == IMAGE_SEMI_SELECTED) || (iParentImage == IMAGE_NOT_SELECTED))
	{
		int iImage = bEnable ? IMAGE_SELECTED : IMAGE_NOT_SELECTED;
		m_pTreeCtrl->SetItemImage(hItem, iImage, iImage);
	}
}

void CIFCModelStructureView::ResetView()
{
	for (auto pModelData : m_vecModelData)
	{
		delete pModelData;
	}
	m_vecModelData.clear();

	m_hModel = NULL;

	m_pSelectedInstance = nullptr;

	m_pTreeCtrl->DeleteAllItems();

	for (auto pModel : getController()->getModels())
	{
		LoadModel(_ptr<_ifc_model>(pModel));
	}	
}