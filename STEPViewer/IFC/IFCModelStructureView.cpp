#include "stdafx.h"

#include "_ifc_model_structure.h"
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
	, m_pModelStructure(pModel->getModelStructure())
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
	if (hItem == NULL) {
		return false;
	}
	else if (hItem == m_hProject) {
		return true;
	}

	return IsProjectItem(m_pTreeCtrl->GetParentItem(hItem));
}

bool CIFCModelStructureView::CModelData::IsGroupsItem(HTREEITEM hItem)
{
	if (hItem == NULL) {
		return false;
	}
	else if (hItem == m_hGroups) {
		return true;
	}

	return IsGroupsItem(m_pTreeCtrl->GetParentItem(hItem));
}

bool CIFCModelStructureView::CModelData::IsSpaceBoundariesItem(HTREEITEM hItem)
{
	if (hItem == NULL) {
		return false;
	}
	else if (hItem == m_hSpaceBoundaries) {
		return true;
	}

	return IsSpaceBoundariesItem(m_pTreeCtrl->GetParentItem(hItem));
}

bool CIFCModelStructureView::CModelData::IsUnreferencedItem(HTREEITEM hItem)
{
	if (hItem == NULL) {
		return false;
	}
	else if (hItem == m_hUnreferenced) {
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
	if (IsProjectItem(hItem)) {
		return m_mapProject;
	}
	else if (IsGroupsItem(hItem)) {
		return m_mapGroups;
	}
	else if (IsSpaceBoundariesItem(hItem)) {
		return m_mapSpaceBoundaries;
	}
	else if (IsUnreferencedItem(hItem)) {
		return m_mapUnreferenced;
	}

	HTREEITEM hParent = m_pTreeCtrl->GetParentItem(hItem);
	while (hParent != NULL) {
		hItem = hParent;

		hParent = m_pTreeCtrl->GetParentItem(hItem);
	}

	ASSERT((m_pTreeCtrl->GetItemText(hItem) == L"Header") ||
		(hItem == m_hModel));

	return m_mapModel;
}

// ************************************************************************************************
CIFCModelStructureView::CIFCModelStructureView(CTreeCtrlEx* pTreeCtrl)
	: CModelStructureViewBase(pTreeCtrl)
	, m_pImageList(nullptr)
	, m_vecModelData()
	, m_mapNodes()
	, m_vecSelectedInstances()
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
	m_pImageList->DeleteImageList();
	delete m_pImageList;

	m_pTreeCtrl->SetItemStateProvider(nullptr);

	delete m_pSearchDialog;

	for (auto pModelData : m_vecModelData) {
		delete pModelData;
	}
	m_vecModelData.clear();
}

/*virtual*/ void CIFCModelStructureView::onInstanceEnabledStateChanged(_view* pSender, _instance* pInstance, int /*iFlag*/) /*override*/
{
	if (pSender == this) {
		return;
	}

	//
	// Model
	//

	_ptr<_ifc_instance> ifcInstance(pInstance);

	auto pModel = getController()->getModelByInstance(ifcInstance->getOwlModel());
	ASSERT(pModel != nullptr);

	auto pModelData = Model_GetData(pModel);
	ASSERT(pModelData != nullptr);

	//
	// UI
	//

	Tree_Update(pModelData->GetModelItem());
}

/*virtual*/ void CIFCModelStructureView::onInstanceSelected(_view* pSender) /*override*/
{
	auto pController = getController();
	if (pController == nullptr) {
		ASSERT(FALSE);
		return;
	}

	Tree_Select(false);

	m_vecSelectedInstances = pController->getSelectedInstances();

	if (!m_vecSelectedInstances.empty()) {
		_ptr<_ifc_instance> ifcInstance(m_vecSelectedInstances.back());

		auto pModel = pController->getModelByInstance(ifcInstance->getOwlModel());
		ASSERT(pModel != nullptr);

		auto pModelData = Model_GetData(pModel);
		ASSERT(pModelData != nullptr);

		if (pSender != this) {
			Tree_EnsureVisible(pModelData, ifcInstance);
		}
	}
	else {
		if (pSender != this) {
			for (auto pModel : m_vecModelData) {
				HTREEITEM hChild = m_pTreeCtrl->GetNextItem(pModel->GetProjectItem(), TVGN_CHILD);
				while (hChild != NULL) {
					m_pTreeCtrl->Expand(hChild, TVE_COLLAPSE);

					hChild = m_pTreeCtrl->GetNextSiblingItem(hChild);
				}
			}
		}
	}

	Tree_Select(true);
}

/*virtual*/ void CIFCModelStructureView::onApplicationPropertyChanged(_view* pSender, enumApplicationProperty enApplicationProperty) /*override*/
{
	if (pSender == this) {
		return;
	}

	if (enApplicationProperty == enumApplicationProperty::FullDisplayName) {
		// Reload
		ResetView();

		// Restore Selection
		onInstanceSelected(nullptr);
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
	if (!bShow) {
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
	if (pController == nullptr) {
		ASSERT(FALSE);
		return;
	}

	/*
	* TVHT_ONITEMICON
	*/
	if ((hItem != NULL) && ((uFlags & TVHT_ONITEMICON) == TVHT_ONITEMICON)) {
		int iImage, iSelectedImage = -1;
		m_pTreeCtrl->GetItemImage(hItem, iImage, iSelectedImage);

		ASSERT(iImage == iSelectedImage);

		auto pModelData = Model_GetData(hItem);
		ASSERT(pModelData != nullptr);

		switch (iImage) {
			case IMAGE_SELECTED:
			case IMAGE_SEMI_SELECTED:
			case IMAGE_NOT_SELECTED:
				{
					bool bGeometryItem = false;
					auto pNode = (_ifc_node*)m_pTreeCtrl->GetItemData(hItem);
					_ifc_instance* pInstance = pNode ? pNode->getInstance() : nullptr;
					if ((pInstance == nullptr) &&
						((iImage == IMAGE_SELECTED) || (iImage == IMAGE_NOT_SELECTED)) &&
						!m_pTreeCtrl->ItemHasChildren(hItem) &&
						(m_pTreeCtrl->GetItemText(hItem) == ITEM_GEOMETRY)) {
						HTREEITEM hParent = m_pTreeCtrl->GetParentItem(hItem);
						ASSERT(hParent != NULL);

						pNode = (_ifc_node*)m_pTreeCtrl->GetItemData(hParent);
						ASSERT(pNode != nullptr);
						pInstance = pNode ? pNode->getInstance() : nullptr;
						ASSERT(pInstance != nullptr);

						bGeometryItem = true;
					}

					//
					// Instance
					//

					if (pInstance != nullptr) {
						pInstance->setEnable(iImage == IMAGE_NOT_SELECTED);
					}

					//
					// In Memory Tree
					//

					if (!bGeometryItem) {
						if (pNode != nullptr) {
							InMemoryTree_EnableChildren(pNode, iImage == IMAGE_NOT_SELECTED);
						}
						else {
							ASSERT(hItem == pModelData->GetModelItem());

							if (pModelData->GetModelStructure()->getProjectNode() != nullptr) {
								InMemoryTree_EnableChildren(pModelData->GetModelStructure()->getProjectNode(), iImage == IMAGE_NOT_SELECTED);
							}
							if (pModelData->GetModelStructure()->getGroupsNode() != nullptr) {
								InMemoryTree_EnableChildren(pModelData->GetModelStructure()->getGroupsNode(), iImage == IMAGE_NOT_SELECTED);
							}
							if (pModelData->GetModelStructure()->getUnreferencedNode() != nullptr) {
								InMemoryTree_EnableChildren(pModelData->GetModelStructure()->getUnreferencedNode(), iImage == IMAGE_NOT_SELECTED);
							}
						}
					}

					//
					// UI
					//

					if (pModelData->IsProjectItem(hItem)) {
						Tree_Update(pModelData->GetProjectItem());
						if (pModelData->GetGroupsItem() != NULL) {
							Tree_Update(pModelData->GetGroupsItem());
						}

						Tree_Update(pModelData->GetModelItem(), false);
					}
					else if (pModelData->IsGroupsItem(hItem)) {
						Tree_Update(pModelData->GetGroupsItem());
						if (pModelData->GetProjectItem()) {
							Tree_Update(pModelData->GetProjectItem());
						}

						Tree_Update(pModelData->GetModelItem(), false);
					}
					else if (pModelData->IsUnreferencedItem(hItem)) {
						Tree_Update(pModelData->GetUnreferencedItem());

						Tree_Update(pModelData->GetModelItem(), false);
					}
					else {
						ASSERT(hItem == pModelData->GetModelItem());

						Tree_Update(pModelData->GetModelItem());
					}

					pController->onInstancesEnabledStateChanged(this);
				}
				break;

			default:
				{
					// skip Items without a Geometry, Header, etc.
					return;
				}
		} // switch (iImage)		

		return;
	} // if ((hItem != NULL) && ...

	/*
	* TVHT_ONITEMLABEL
	*/
	if ((hItem != NULL) && ((uFlags & TVHT_ONITEMLABEL) == TVHT_ONITEMLABEL)) {
		auto pSelectedNode = m_pTreeCtrl->GetItemData(hItem) != NULL ?
			(_ifc_node*)m_pTreeCtrl->GetItemData(hItem) :
			nullptr;

		pController->selectInstance(
			this,
			pSelectedNode != nullptr ? pSelectedNode->getInstance() : nullptr,
			GetKeyState(VK_CONTROL) & 0x8000);
	}
}

/*virtual*/ void CIFCModelStructureView::OnTreeItemExpanding(NMHDR* pNMHDR, LRESULT* pResult) /*override*/
{
	*pResult = 0;

	auto pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	if (m_pTreeCtrl->GetChildItem(pNMTreeView->itemNew.hItem) != NULL) {
		// it is loaded already
		return;
	}

	auto pNode = (_ifc_node*)m_pTreeCtrl->GetItemData(pNMTreeView->itemNew.hItem);
	ASSERT(pNode != nullptr);

	auto pInstance = pNode ? pNode->getInstance() : nullptr;

	// Geometry
	if (pInstance != nullptr) {
		TV_INSERTSTRUCT tvInsertStruct;
		tvInsertStruct.hParent = pNMTreeView->itemNew.hItem;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = ITEM_GEOMETRY;
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage =
			pInstance->hasGeometry() ?
			(pInstance->getEnable() ? IMAGE_SELECTED : IMAGE_NOT_SELECTED) :
			IMAGE_NO_GEOMETRY;
		tvInsertStruct.item.lParam = NULL;
		m_pTreeCtrl->InsertItem(&tvInsertStruct);
	}

	_ptr<_ap_controller> apController(getController());

	auto pModelData = Model_GetData(pNMTreeView->itemNew.hItem);
	ASSERT(pModelData != nullptr);

	ITEMS& mapItems = pModelData->GetItems(pNMTreeView->itemNew.hItem);

	for (auto pChildNode : pNode->children()) {
		auto pChildInstance = pChildNode->getInstance();

		wstring strItemName;
		if (pChildInstance != nullptr) {
			strItemName = _ap_geometry::getDisplayString(pChildInstance->getSdaiInstance(), apController->getFullDisplayName());
		}
		else {
			strItemName = pChildNode->getGlobalId();
		}

		// Instance
		TV_INSERTSTRUCT tvInsertStruct;
		tvInsertStruct.hParent = pNMTreeView->itemNew.hItem;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
		tvInsertStruct.item.pszText = (LPWSTR)strItemName.c_str();
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = InMemoryTree_GetItemState(pChildNode);
		tvInsertStruct.item.lParam = (LPARAM)pChildNode;
		tvInsertStruct.item.cChildren = (pChildNode->children().size() > 0 || pChildInstance != nullptr) ? 1 : 0;
		HTREEITEM hChildItem = m_pTreeCtrl->InsertItem(&tvInsertStruct);

		ASSERT(m_mapNodes.find(pChildNode) == m_mapNodes.end());
		m_mapNodes[pChildNode] = hChildItem;

		auto itItems = mapItems.find(pChildInstance);
		if (itItems != mapItems.end()) {
			itItems->second.push_back(hChildItem);
		}
		else {
			mapItems[pChildInstance] = vector<HTREEITEM>{ hChildItem };
		}

		// Show
		if (pChildInstance != nullptr) {
			m_pTreeCtrl->SetItemState(
				hChildItem,
				pChildInstance->getGeometry()->getShow() ? 0 : TVIS_CUT, TVIS_CUT);
		}

		// Geometry
		if ((pChildInstance != nullptr) && pChildNode->children().empty()) {
			tvInsertStruct.hParent = hChildItem;
			tvInsertStruct.hInsertAfter = TVI_LAST;
			tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
			tvInsertStruct.item.pszText = ITEM_GEOMETRY;
			tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage =
				pChildNode->getInstance()->hasGeometry() ?
				(pChildNode->getInstance()->getEnable() ? IMAGE_SELECTED : IMAGE_NOT_SELECTED) :
				IMAGE_NO_GEOMETRY;
			tvInsertStruct.item.lParam = NULL;
			m_pTreeCtrl->InsertItem(&tvInsertStruct);
		}
	}
}

/*virtual*/ bool CIFCModelStructureView::IsSelected(HTREEITEM hItem) /*override*/
{
	auto pController = getController();
	if (pController == nullptr) {
		return false;
	}

	auto pNode = (_ifc_node*)m_pTreeCtrl->GetItemData(hItem);
	if (pNode == nullptr) {
		return false;
	}

	auto vecSelectedInstances = pController->getSelectedInstances();
	if (vecSelectedInstances.empty()) {
		return false;
	}

	return find(vecSelectedInstances.begin(), vecSelectedInstances.end(), pNode->getInstance()) != vecSelectedInstances.end();
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

/*virtual*/ void CIFCModelStructureView::LoadChildrenIfNeeded(HTREEITEM hItem) /*override*/
{
	ASSERT(hItem != NULL);

	if (m_pTreeCtrl->GetChildItem(hItem) != NULL) {
		// it is loaded already
		return;
	}

	m_pTreeCtrl->Expand(hItem, TVE_EXPAND);
}

/*virtual*/ BOOL CIFCModelStructureView::ContainsText(int iFilter, HTREEITEM hItem, const CString& strText) /*override*/
{
	if (hItem == NULL) {
		ASSERT(FALSE);

		return FALSE;
	}

	CString strItemText = GetTreeView()->GetItemText(hItem);
	strItemText.MakeLower();

	CString strTextLower = strText;
	strTextLower.MakeLower();

	// Express line number
	if (iFilter == (int)enumSearchFilter::ExpressID) {
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
	if (pWnd != m_pTreeCtrl) {
		return;
	}

	if (point == CPoint(-1, -1)) {
		return;
	}

	auto pController = getAPController();
	if (pController == nullptr) {
		ASSERT(FALSE);

		return;
	}

	// Select clicked item
	CPoint ptTree = point;
	m_pTreeCtrl->ScreenToClient(&ptTree);

	UINT flags = 0;
	HTREEITEM hItem = m_pTreeCtrl->HitTest(ptTree, &flags);
	if (hItem == NULL) {
		return;
	}

	m_pTreeCtrl->SelectItem(hItem);
	m_pTreeCtrl->SetFocus();

	auto pTargetNode = (_ifc_node*)m_pTreeCtrl->GetItemData(hItem);
	auto pTargetInstance = pTargetNode ? pTargetNode->getInstance() : nullptr;

	auto pModelData = Model_GetData(hItem);
	ASSERT(pModelData != nullptr);

	// Zoom to
	set<_instance*> setZoomToInstances;

	// ENTITY : ENBABLE COUNT
	map<wstring, long> mapEntity2EnableCount;
	map<wstring, long> mapEntity2ShowCount;
	for (auto pGeometry : pModelData->GetModel()->getGeometries()) {
		if (!pGeometry->hasGeometry()) {
			continue;
		}

		_ptr<_ifc_geometry> ifcGeometry(pGeometry);
		if (ifcGeometry->getIsMappedItem()) {
			continue;
		}

		const wchar_t* szEntityName = ifcGeometry->getEntityName();

		// Enable
		auto itEntity2EnableCount = mapEntity2EnableCount.find(szEntityName);
		if (itEntity2EnableCount == mapEntity2EnableCount.end()) {
			mapEntity2EnableCount[szEntityName] = pGeometry->getEnabledInstancesCount();
		}
		else {
			itEntity2EnableCount->second += pGeometry->getEnabledInstancesCount();
		}

		// Show
		auto itEntity2ShowCount = mapEntity2ShowCount.find(szEntityName);
		if (itEntity2ShowCount == mapEntity2ShowCount.end()) {
			mapEntity2ShowCount[szEntityName] = pGeometry->getShow() ? 1 : 0;
		}
		else {
			itEntity2ShowCount->second += pGeometry->getShow() ? 1 : 0;
		}
	} // for (; itInstance != ...

	ASSERT(!mapEntity2EnableCount.empty());

	// Build menu
	CMenu menuMain;
	CMenu* pMenu = nullptr;

	if (pTargetInstance != nullptr) {
		if (pTargetInstance->hasGeometry()) {
			VERIFY(menuMain.LoadMenuW(IDR_POPUP_INSTANCES));
			pMenu = menuMain.GetSubMenu(0);

			// Zoom to
			if (!pTargetInstance->getEnable()) {
				pMenu->EnableMenuItem(ID_INSTANCES_ZOOM_TO, MF_BYCOMMAND | MF_DISABLED);
			}

			// Save
			if (!pTargetInstance->getEnable()) {
				pMenu->EnableMenuItem(ID_INSTANCES_SAVE, MF_BYCOMMAND | MF_DISABLED);
			}

			// Enable
			if (pTargetInstance->getEnable()) {
				pMenu->CheckMenuItem(ID_INSTANCES_ENABLE, MF_BYCOMMAND | MF_CHECKED);
			}
		} // if (pTargetInstance->hasGeometry())
		else {
			Model_GetChildren(hItem, true, setZoomToInstances);

			VERIFY(menuMain.LoadMenuW(IDR_POPUP_INSTANCES_NO_GEOMETRY));
			pMenu = menuMain.GetSubMenu(0);

			// Zoom to
			if (setZoomToInstances.empty()) {
				pMenu->EnableMenuItem(ID_INSTANCES_ZOOM_TO_CHILDREN, MF_BYCOMMAND | MF_DISABLED);
			}
		}
	} // if (pTargetInstance != nullptr)
	else {
		Model_GetChildren(hItem, true, setZoomToInstances);

		VERIFY(menuMain.LoadMenuW(IDR_POPUP_META_DATA));
		pMenu = menuMain.GetSubMenu(0);

		// Zoom to
		if (setZoomToInstances.empty()) {
			pMenu->EnableMenuItem(ID_INSTANCES_ZOOM_TO_CHILDREN, MF_BYCOMMAND | MF_DISABLED);
		}
	}

	// Enable Entities
	CMenu menuEnableEntities;
	VERIFY(menuEnableEntities.CreatePopupMenu());

	UINT uiID = 1;
	map<UINT, wstring> mapCommand2EnableEntity;
	for (auto itEntity2EnableCount = mapEntity2EnableCount.begin();
		itEntity2EnableCount != mapEntity2EnableCount.end();
		itEntity2EnableCount++) {
		mapCommand2EnableEntity[uiID] = itEntity2EnableCount->first;

		auto itEntity2ShowCount = mapEntity2ShowCount.find(itEntity2EnableCount->first);
		ASSERT(itEntity2ShowCount != mapEntity2ShowCount.end());

		wstring strMenuItem = itEntity2EnableCount->first;
		if (itEntity2ShowCount->second == 0) {
			strMenuItem += L" (hidden)";
		}

		menuEnableEntities.AppendMenu(
			MF_STRING | (itEntity2EnableCount->second > 0 ? MF_CHECKED : MF_UNCHECKED),
			uiID++,
			strMenuItem.c_str());
	}

	// Show Entities
	CMenu menuShowEntities;
	VERIFY(menuShowEntities.CreatePopupMenu());

	map<UINT, wstring> mapCommand2ShowEntity;
	for (auto itEntity2ShowCount = mapEntity2ShowCount.begin();
		itEntity2ShowCount != mapEntity2ShowCount.end();
		itEntity2ShowCount++) {
		mapCommand2ShowEntity[uiID] = itEntity2ShowCount->first;

		auto itEntity2EnableCount = mapEntity2EnableCount.find(itEntity2ShowCount->first);
		ASSERT(itEntity2EnableCount != mapEntity2EnableCount.end());

		wstring strMenuItem = itEntity2EnableCount->first;
		if (itEntity2EnableCount->second == 0) {
			strMenuItem += L" (disabled)";
		}

		menuShowEntities.AppendMenu(
			MF_STRING | (itEntity2ShowCount->second > 0 ? MF_CHECKED : MF_UNCHECKED),
			uiID++,
			strMenuItem.c_str());
	}

	if (pMenu != nullptr) {
		pMenu->AppendMenu(MF_SEPARATOR, 0, L"");
		pMenu->AppendMenu(MF_STRING | MF_POPUP, (UINT_PTR)menuShowEntities.GetSafeHmenu(), L"Show Entities");
		pMenu->AppendMenu(MF_STRING | MF_POPUP, (UINT_PTR)menuEnableEntities.GetSafeHmenu(), L"Enable Entities");
	}
	else {
		pMenu = &menuShowEntities;
		pMenu->AppendMenu(MF_STRING | MF_POPUP, (UINT_PTR)menuEnableEntities.GetSafeHmenu(), L"Enable Entities");
	}

	// Show
	UINT uiCommand = pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, point.x, point.y, m_pTreeCtrl);
	if (uiCommand == 0) {
		return;
	}

	// Execute the command
	bool bExecuted = true;
	if (pTargetInstance != nullptr) {
		if (pTargetInstance->hasGeometry()) {
			switch (uiCommand) {
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
						//
						// Instance
						//

						pTargetInstance->setEnable(!pTargetInstance->getEnable());

						//
						// In Memory Tree
						//

						InMemoryTree_EnableChildren(pTargetNode, !pTargetInstance->getEnable());

						//
						// UI
						//

						if (pModelData->IsProjectItem(hItem)) {
							Tree_Update(pModelData->GetProjectItem());
							if (pModelData->GetGroupsItem() != NULL) {
								Tree_Update(pModelData->GetGroupsItem());
							}

							Tree_Update(pModelData->GetModelItem(), false);
						}
						else if (pModelData->IsGroupsItem(hItem)) {
							Tree_Update(pModelData->GetGroupsItem());
							if (pModelData->GetProjectItem()) {
								Tree_Update(pModelData->GetProjectItem());
							}

							Tree_Update(pModelData->GetModelItem(), false);
						}
						else if (pModelData->IsUnreferencedItem(hItem)) {
							Tree_Update(pModelData->GetUnreferencedItem());

							Tree_Update(pModelData->GetModelItem(), false);
						}
						else {
							ASSERT(hItem == pModelData->GetModelItem());

							Tree_Update(pModelData->GetModelItem());
						}

						pController->onInstanceEnabledStateChanged(this, pTargetInstance, 0);
					}
					break;

				case ID_INSTANCES_DISABLE_ALL_BUT_THIS:
					{
						//
						// Model
						//

						for (auto pInstance : pModelData->GetModel()->getInstances()) {
							pInstance->setEnable(pTargetInstance == pInstance);
						}

						//
						// In Memory Tree
						//

						InMemoryTree_EnableChildren(pTargetNode, pTargetInstance->getEnable());

						//
						// UI
						//

						if (pModelData->IsProjectItem(hItem)) {
							Tree_Update(pModelData->GetProjectItem());
							if (pModelData->GetGroupsItem() != NULL) {
								Tree_Update(pModelData->GetGroupsItem());
							}

							Tree_Update(pModelData->GetModelItem(), false);
						}
						else if (pModelData->IsGroupsItem(hItem)) {
							Tree_Update(pModelData->GetGroupsItem());
							if (pModelData->GetProjectItem()) {
								Tree_Update(pModelData->GetProjectItem());
							}

							Tree_Update(pModelData->GetModelItem(), false);
						}
						else if (pModelData->IsUnreferencedItem(hItem)) {
							Tree_Update(pModelData->GetUnreferencedItem());

							Tree_Update(pModelData->GetModelItem(), false);
						}
						else {
							ASSERT(hItem == pModelData->GetModelItem());

							Tree_Update(pModelData->GetModelItem());
						}

						pController->onInstancesEnabledStateChanged(this);
					}
					break;

				case ID_INSTANCES_ENABLE_ALL:
					{
						for (auto pInstance : pModelData->GetModel()->getInstances()) {
							pInstance->setEnable(true);
						}

						Tree_Update(pModelData->GetModelItem());

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
		else {
			switch (uiCommand) {
				case ID_INSTANCES_ZOOM_TO_CHILDREN:
					{
						pController->zoomToInstances(setZoomToInstances);
					}
					break;

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
	else {
		switch (uiCommand) {
			case ID_INSTANCES_ZOOM_TO_CHILDREN:
				{
					pController->zoomToInstances(setZoomToInstances);
				}
				break;

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
	if (!bExecuted) {
		// Enable
		auto itCommand2EnableEntity = mapCommand2EnableEntity.find(uiCommand);
		if (itCommand2EnableEntity != mapCommand2EnableEntity.end()) {
			auto itEntity2EnableCount = mapEntity2EnableCount.find(itCommand2EnableEntity->second);
			if (itEntity2EnableCount == mapEntity2EnableCount.end()) {
				ASSERT(FALSE); // Internal error!

				return;
			}

			//
			// Model
			//

			for (auto pInstance : pModelData->GetModel()->getInstances()) {
				_ptr<_ifc_instance> ifcInstance(pInstance);
				if (ifcInstance->getOwner() != nullptr) {
					continue;
				}

				if (ifcInstance->getEntityName() == itCommand2EnableEntity->second) {
					ifcInstance->setEnable(itEntity2EnableCount->second > 0 ? false : true);
				}
			}

			//
			// UI
			//

			Tree_Update(pModelData->GetModelItem());

			pController->onInstancesEnabledStateChanged(this);
		} // Enable
		else {
			auto itCommand2ShowEntity = mapCommand2ShowEntity.find(uiCommand);
			if (itCommand2ShowEntity != mapCommand2ShowEntity.end()) {
				auto itEntity2ShowCount = mapEntity2ShowCount.find(itCommand2ShowEntity->second);
				if (itEntity2ShowCount == mapEntity2ShowCount.end()) {
					ASSERT(FALSE); // Internal error!

					return;
				}

				//
				// Model
				//

				set<_ifc_instance*> setInstances;
				for (auto pGeometry : pModelData->GetModel()->getGeometries()) {
					_ptr<_ifc_geometry> ifcGeometry(pGeometry);
					if (ifcGeometry->getIsMappedItem()) {
						continue;
					}

					const wchar_t* szEntityName = ifcGeometry->getEntityName();

					for (auto pInstance : pGeometry->getInstances()) {
						_ptr<_ifc_instance> ifcInstance(pInstance);
						if (szEntityName == itCommand2ShowEntity->second) {
							ifcGeometry->setShow(itEntity2ShowCount->second > 0 ? false : true);

							setInstances.insert(ifcInstance);
						}
					}
				}

				Tree_Show(setInstances);

				pController->onInstancesShowStateChanged(this);
			} // Show
		} // else Enable
	} // if (!bProcessed)
}

/*virtual*/ void CIFCModelStructureView::OnSearch() /*override*/
{
	if (!m_pSearchDialog->IsWindowVisible()) {
		m_pSearchDialog->ShowWindow(SW_SHOW);
	}
	else {
		m_pSearchDialog->ShowWindow(SW_HIDE);
	}
}

void CIFCModelStructureView::LoadModel(_ifc_model* pModel)
{
	// Model
	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = NULL;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = (LPWSTR)pModel->getPath();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_SELECTED;
	tvInsertStruct.item.lParam = (LPARAM)pModel->getModelStructure()->getModelNode();
	HTREEITEM hModel = m_pTreeCtrl->InsertItem(&tvInsertStruct);

	ASSERT(m_mapNodes.find(pModel->getModelStructure()->getModelNode()) == m_mapNodes.end());
	m_mapNodes[pModel->getModelStructure()->getModelNode()] = hModel;

	auto pModelData = new CModelData(pModel, m_pTreeCtrl, hModel);
	m_vecModelData.push_back(pModelData);

	// Header
	LoadHeader(pModel, hModel);

	// Project/Units/Unreferenced
	SdaiAggr sdaiProjectAggr = sdaiGetEntityExtentBN(pModel->getSdaiModel(), "IFCPROJECT");

	SdaiInteger iProjectInstancesCount = sdaiGetMemberCount(sdaiProjectAggr);
	if (iProjectInstancesCount > 0) {
		SdaiInstance sdaiProjectInstance = 0;
		sdaiGetAggrByIndex(sdaiProjectAggr, 0, sdaiINSTANCE, &sdaiProjectInstance);

		// Load
		LoadProject(pModelData, hModel, sdaiProjectInstance, pModelData->GetProjectItems());
		LoadGroups(pModelData, hModel);
		LoadUnreferencedItems(pModelData, hModel);

		// Update UI
		Tree_Update(hModel);
	} // if (iProjectInstancesCount > 0)

	m_pTreeCtrl->Expand(hModel, TVE_EXPAND);
}

void CIFCModelStructureView::LoadProject(CModelData* pModelData, HTREEITEM hModel, SdaiInstance sdaiProjectInstance, ITEMS& mapItems)
{
	auto pController = getController();
	if (pController == nullptr) {
		ASSERT(FALSE);
		return;
	}

	_ptr<_ap_controller> apController(pController);

	ASSERT(pModelData != nullptr);
	ASSERT(pModelData->GetModelStructure() != nullptr);

	const auto& mapInstance2Node = pModelData->GetModelStructure()->getInstance2Node();

	auto pGeometry = pModelData->GetModel()->getGeometryByInstance(sdaiProjectInstance);
	if (pGeometry != nullptr) {
		wstring strItem = _ap_geometry::getDisplayString(sdaiProjectInstance, apController->getFullDisplayName());

		const auto& itInstance2Node = mapInstance2Node.find(sdaiProjectInstance);
		ASSERT(itInstance2Node != mapInstance2Node.end());

		// Project
		TV_INSERTSTRUCT tvInsertStruct;
		tvInsertStruct.hParent = hModel;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
		tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = InMemoryTree_GetItemState(itInstance2Node->second);
		tvInsertStruct.item.lParam = (LPARAM)itInstance2Node->second;
		tvInsertStruct.item.cChildren = 1;
		HTREEITEM hProject = m_pTreeCtrl->InsertItem(&tvInsertStruct);
		pModelData->SetProjectItem(hProject);

		ASSERT(mapItems.find(itInstance2Node->second->getInstance()) == mapItems.end());
		mapItems[itInstance2Node->second->getInstance()] = vector<HTREEITEM>{ hProject };

		ASSERT(m_mapNodes.find(itInstance2Node->second) == m_mapNodes.end());
		m_mapNodes[itInstance2Node->second] = hProject;

		m_pTreeCtrl->Expand(hProject, TVE_EXPAND);
	} // if (itInstance != ...
}

void CIFCModelStructureView::LoadGroups(CModelData* pModelData, HTREEITEM hModel)
{
	ASSERT(pModelData != nullptr);
	ASSERT(pModelData->GetModelStructure() != nullptr);

	auto pGroupsNode = pModelData->GetModelStructure()->getGroupsNode();
	if (pGroupsNode == nullptr) {
		return;
	}

	// Groups
	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = hModel;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
	tvInsertStruct.item.pszText = (LPWSTR)ITEM_GROUPS;
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = InMemoryTree_GetItemState(pGroupsNode);
	tvInsertStruct.item.lParam = (LPARAM)pGroupsNode;
	tvInsertStruct.item.cChildren = 1;
	HTREEITEM hGroups = m_pTreeCtrl->InsertItem(&tvInsertStruct);
	pModelData->SetGroupsItem(hGroups);

	ASSERT(m_mapNodes.find(pGroupsNode) == m_mapNodes.end());
	m_mapNodes[pGroupsNode] = hGroups;
}

void CIFCModelStructureView::LoadUnreferencedItems(CModelData* pModelData, HTREEITEM hModel)
{
	ASSERT(pModelData != nullptr);
	ASSERT(pModelData->GetModelStructure() != nullptr);

	auto pUnreferencedNode = pModelData->GetModelStructure()->getUnreferencedNode();
	if (pUnreferencedNode == nullptr) {
		return;
	}

	// Unreferenced Items
	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = hModel;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
	tvInsertStruct.item.pszText = (LPWSTR)ITEM_UNREFERENCED;
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = InMemoryTree_GetItemState(pUnreferencedNode);
	tvInsertStruct.item.lParam = (LPARAM)pUnreferencedNode;
	tvInsertStruct.item.cChildren = 1;
	HTREEITEM hUnreferenced = m_pTreeCtrl->InsertItem(&tvInsertStruct);
	pModelData->SetUnreferencedItem(hUnreferenced);

	ASSERT(m_mapNodes.find(pUnreferencedNode) == m_mapNodes.end());
	m_mapNodes[pUnreferencedNode] = hUnreferenced;
}

CIFCModelStructureView::CModelData* CIFCModelStructureView::Model_GetData(HTREEITEM hItem)
{
	HTREEITEM hModel = Tree_GetModelItem(hItem);
	if (hModel == NULL) {
		ASSERT(FALSE);

		return nullptr;
	}

	auto itModelData = find_if(m_vecModelData.begin(), m_vecModelData.end(), [&](CModelData* pModelData) {
		return pModelData->GetModelItem() == hModel;
		});

	if (itModelData != m_vecModelData.end()) {
		return *itModelData;
	}

	return nullptr;
}

CIFCModelStructureView::CModelData* CIFCModelStructureView::Model_GetData(_model* pModel)
{
	ASSERT(pModel != nullptr);

	auto itModelData = find_if(m_vecModelData.begin(), m_vecModelData.end(), [&](CModelData* pModelData) {
		return pModelData->GetModel() == pModel;
		});

	if (itModelData != m_vecModelData.end()) {
		return *itModelData;
	}

	return nullptr;
}

void CIFCModelStructureView::Model_GetChildren(HTREEITEM hItem, bool bEnabledOnly, set<_instance*>& setChildren)
{
	if (hItem == NULL) {
		ASSERT(FALSE);

		return;
	}

	if (!m_pTreeCtrl->ItemHasChildren(hItem)) {
		return;
	}

	HTREEITEM hChild = m_pTreeCtrl->GetNextItem(hItem, TVGN_CHILD);
	while (hChild != NULL) {
		auto pNode = (_ifc_node*)m_pTreeCtrl->GetItemData(hChild);
		_ifc_instance* pInstance = pNode ? pNode->getInstance() : nullptr;
		if (pInstance != nullptr) {
			if (bEnabledOnly) {
				if (pInstance->getEnable()) {
					setChildren.insert(pInstance);
				}
			}
			else {
				setChildren.insert(pInstance);
			}
		}

		Model_GetChildren(hChild, bEnabledOnly, setChildren);

		hChild = m_pTreeCtrl->GetNextSiblingItem(hChild);
	} // while (hChild != NULL)
}

HTREEITEM CIFCModelStructureView::Tree_GetModelItem(HTREEITEM hItem) const
{
	ASSERT(hItem != NULL);

	HTREEITEM hParent = m_pTreeCtrl->GetParentItem(hItem);
	if (hParent == NULL) {
		return hItem;
	}

	return Tree_GetModelItem(hParent);
}

void CIFCModelStructureView::Tree_Update(HTREEITEM hItem, bool bRecursive/* = true*/)
{
	ASSERT(hItem != nullptr);

	int iItemState = Tree_GetItemState(hItem);
	m_pTreeCtrl->SetItemImage(hItem, iItemState, iItemState);

	if (!bRecursive) {
		return;
	}

	HTREEITEM hChild = m_pTreeCtrl->GetNextItem(hItem, TVGN_CHILD);
	while (hChild != NULL) {
		int iImage, iSelectedImage = -1;
		m_pTreeCtrl->GetItemImage(hChild, iImage, iSelectedImage);
		ASSERT(iImage == iSelectedImage);

		if ((iImage != IMAGE_SELECTED) && (iImage != IMAGE_SEMI_SELECTED) && (iImage != IMAGE_NOT_SELECTED)) {
			// skip Items without a Geometry, Header, etc.
			hChild = m_pTreeCtrl->GetNextSiblingItem(hChild);
			continue;
		}

		iItemState = Tree_GetItemState(hChild);
		m_pTreeCtrl->SetItemImage(hChild, iItemState, iItemState);

		Tree_Update(hChild);

		hChild = m_pTreeCtrl->GetNextSiblingItem(hChild);
	}
}

void CIFCModelStructureView::Tree_UpdateChildren(HTREEITEM hItem)
{
	if (hItem == NULL) {
		ASSERT(FALSE);
		return;
	}

	if (!m_pTreeCtrl->ItemHasChildren(hItem)) {
		return;
	}

	int iParentImage = -1;
	int iParentSelectedImage = -1;
	m_pTreeCtrl->GetItemImage(hItem, iParentImage, iParentSelectedImage);
	ASSERT(iParentImage == iParentSelectedImage);
	ASSERT(iParentImage == IMAGE_SELECTED || iParentImage == IMAGE_NOT_SELECTED);

	HTREEITEM hChild = m_pTreeCtrl->GetNextItem(hItem, TVGN_CHILD);
	if (hChild == NULL) {
		auto pNode = (_ifc_node*)m_pTreeCtrl->GetItemData(hItem);
		InMemoryTree_EnableChildren(pNode, iParentImage == IMAGE_SELECTED);
		return;
	}

	while (hChild != NULL) {
		int iImage, iSelectedImage = -1;
		m_pTreeCtrl->GetItemImage(hChild, iImage, iSelectedImage);
		ASSERT(iImage == iSelectedImage);

		if ((iImage != IMAGE_SELECTED) && (iImage != IMAGE_SEMI_SELECTED) && (iImage != IMAGE_NOT_SELECTED)) {
			// skip Items without a Geometry, Header, etc.
			hChild = m_pTreeCtrl->GetNextSiblingItem(hChild);
			continue;
		}

		m_pTreeCtrl->SetItemImage(hChild, iParentImage, iParentImage);

		Tree_UpdateChildren(hChild);

		hChild = m_pTreeCtrl->GetNextSiblingItem(hChild);
	} // while (hChild != NULL)
}

void CIFCModelStructureView::InMemoryTree_EnableChildren(_ifc_node* pNode, bool bEnable)
{
	if (pNode == nullptr) {
		ASSERT(FALSE);
		return;
	}

	for (auto pChildNode : pNode->children()) {
		if (pChildNode->getInstance() != nullptr) {
			pChildNode->getInstance()->setEnable(bEnable);
		}

		InMemoryTree_EnableChildren(pChildNode, bEnable);
	}
}

void CIFCModelStructureView::Tree_UpdateParents(HTREEITEM hItem)
{
	if (hItem == NULL) {
		return;
	}

	ASSERT(m_pTreeCtrl->ItemHasChildren(hItem));

	int iChildrenCount = 0;
	int iSelectedChildrenCount = 0;
	int iSemiSelectedChildrenCount = 0;

	HTREEITEM hChild = m_pTreeCtrl->GetNextItem(hItem, TVGN_CHILD);
	while (hChild != nullptr) {
		int iImage, iSelectedImage = -1;
		m_pTreeCtrl->GetItemImage(hChild, iImage, iSelectedImage);
		ASSERT(iImage == iSelectedImage);

		if ((iImage != IMAGE_SELECTED) && (iImage != IMAGE_SEMI_SELECTED) && (iImage != IMAGE_NOT_SELECTED)) {
			// skip Items without a Geometry, Header, etc.
			hChild = m_pTreeCtrl->GetNextSiblingItem(hChild);
			continue;
		}

		iChildrenCount++;

		switch (iImage) {
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

	if (iSemiSelectedChildrenCount > 0) {
		m_pTreeCtrl->SetItemImage(hItem, IMAGE_SEMI_SELECTED, IMAGE_SEMI_SELECTED);

		Tree_UpdateParents(m_pTreeCtrl->GetParentItem(hItem));
	}
	else if (iSelectedChildrenCount == 0) {
		m_pTreeCtrl->SetItemImage(hItem, IMAGE_NOT_SELECTED, IMAGE_NOT_SELECTED);

		Tree_UpdateParents(m_pTreeCtrl->GetParentItem(hItem));
	}
	else if (iSelectedChildrenCount == iChildrenCount) {
		m_pTreeCtrl->SetItemImage(hItem, IMAGE_SELECTED, IMAGE_SELECTED);

		Tree_UpdateParents(m_pTreeCtrl->GetParentItem(hItem));
	}
	else {
		ASSERT(iSelectedChildrenCount < iChildrenCount);
		m_pTreeCtrl->SetItemImage(hItem, IMAGE_SEMI_SELECTED, IMAGE_SEMI_SELECTED);

		Tree_UpdateParents(m_pTreeCtrl->GetParentItem(hItem));
	}
}

void CIFCModelStructureView::Tree_Select(bool bEnable)
{
	for (auto pInstance : m_vecSelectedInstances) {
		_ptr<_ifc_instance> ifcInstance(pInstance);

		auto pModel = getController()->getModelByInstance(ifcInstance->getOwlModel());
		ASSERT(pModel != nullptr);

		auto pModelData = Model_GetData(pModel);
		ASSERT(pModelData != nullptr);

		Tree_Select(ifcInstance, pModelData->GetProjectItems(), bEnable);
		Tree_Select(ifcInstance, pModelData->GetGroupsItems(), bEnable);
		Tree_Select(ifcInstance, pModelData->GetSpaceBoundariesItems(), bEnable);
		Tree_Select(ifcInstance, pModelData->GetUnreferencedItems(), bEnable);
	}
}

void CIFCModelStructureView::Tree_Select(_ifc_instance* pInstance, ITEMS& mapItems, bool bEnable)
{
	ASSERT(pInstance != nullptr);

	auto itItems = mapItems.find(pInstance);
	if (itItems == mapItems.end()) {
		return;
	}

	for (auto hInstance : itItems->second) {
		m_pTreeCtrl->SetItemState(hInstance, bEnable ? TVIS_BOLD : 0, TVIS_BOLD);
	}
}

void CIFCModelStructureView::Tree_Show(const set<_ifc_instance*>& setInstances)
{
	for (auto pInstance : setInstances) {
		auto pModel = getController()->getModelByInstance(pInstance->getOwlModel());
		ASSERT(pModel != nullptr);

		auto pModelData = Model_GetData(pModel);
		ASSERT(pModelData != nullptr);

		Tree_Show(pInstance, pModelData->GetProjectItems());
		Tree_Show(pInstance, pModelData->GetGroupsItems());
		Tree_Show(pInstance, pModelData->GetSpaceBoundariesItems());
		Tree_Show(pInstance, pModelData->GetUnreferencedItems());
	}
}

void CIFCModelStructureView::Tree_Show(_ifc_instance* pInstance, ITEMS& mapItems)
{
	ASSERT(pInstance != nullptr);

	auto itItems = mapItems.find(pInstance);
	if (itItems == mapItems.end()) {
		return;
	}

	for (auto hInstance : itItems->second) {
		m_pTreeCtrl->SetItemState(
			hInstance,
			pInstance->getGeometry()->getShow() ? 0 : TVIS_CUT, TVIS_CUT);
	}
}

bool CIFCModelStructureView::Tree_EnsureVisible(CModelData* pModelData, _ifc_instance* pInstance)
{
	ASSERT(pModelData != nullptr);

	return
		Tree_EnsureVisible(pModelData, pModelData->GetProjectItems(), pInstance) ||
		Tree_EnsureVisible(pModelData, pModelData->GetGroupsItems(), pInstance) ||
		Tree_EnsureVisible(pModelData, pModelData->GetSpaceBoundariesItems(), pInstance) ||
		Tree_EnsureVisible(pModelData, pModelData->GetUnreferencedItems(), pInstance);
}

bool CIFCModelStructureView::Tree_EnsureVisible(CModelData* pModelData, ITEMS& mapItems, _ifc_instance* pInstance)
{
	ASSERT(pModelData != nullptr);

	if (pInstance == nullptr) {
		return false;
	}

	auto itItems = mapItems.find(pInstance);

	// Load branch
	if (itItems == mapItems.end()) {
		auto pModelStructure = pModelData->GetModelStructure();

		vector<_ifc_node*> vecPath;
		pModelStructure->getInstancePath(pInstance->getSdaiInstance(), vecPath);

		for (auto pNode : vecPath) {
			auto itNode = m_mapNodes.find(pNode);
			ASSERT(itNode != m_mapNodes.end());

			m_pTreeCtrl->Expand(itNode->second, TVE_EXPAND);
		}
	}

	itItems = mapItems.find(pInstance);
	if (itItems != mapItems.end()) {
		m_pTreeCtrl->EnsureVisible(itItems->second.front());
		return true;
	}

	return false;
}

int CIFCModelStructureView::Tree_GetItemState(HTREEITEM hItem)
{
	auto pNode = (_ifc_node*)m_pTreeCtrl->GetItemData(hItem);
	if (pNode == nullptr) {
		if (m_pTreeCtrl->GetItemText(hItem) == ITEM_GEOMETRY) {
			// The Geometry Item of an Instance
			HTREEITEM hParent = m_pTreeCtrl->GetParentItem(hItem);
			ASSERT(hParent != NULL);

			pNode = (_ifc_node*)m_pTreeCtrl->GetItemData(hParent);
			auto pInstance = pNode ? pNode->getInstance() : nullptr;
			ASSERT(pInstance != nullptr);

			return pInstance->hasGeometry() ?
				(pInstance->getEnable() ? IMAGE_SELECTED : IMAGE_NOT_SELECTED) :
				IMAGE_NO_GEOMETRY;
		}
		else {
			ASSERT(FALSE); // Unexpected
			return IMAGE_NO_GEOMETRY;
		}
	}

	if (pNode->children().empty()) {
		auto pInstance = pNode->getInstance();
		ASSERT(pInstance != nullptr);

		return pInstance->hasGeometry() ?
			(pInstance->getEnable() ? IMAGE_SELECTED : IMAGE_NOT_SELECTED) :
			IMAGE_NO_GEOMETRY;
	}

	auto pInstance = pNode->getInstance();

	int iChildrenCount = (int)pNode->children().size() + ((pInstance != nullptr) && pInstance->hasGeometry() ? 1 : 0);
	int iSelectedChildrenCount = (pInstance != nullptr) && pInstance->hasGeometry() && pInstance->getEnable() ? 1 : 0;
	int iSemiSelectedChildrenCount = 0;
	int iNoGeometryChildrenCount = 0;

	for (auto pChildNode : pNode->children()) {
		int iChildState = InMemoryTree_GetItemState(pChildNode);
		switch (iChildState) {
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
		} // switch (iChildState)
	} // for (auto pChildNode : ...

	if (iChildrenCount == iNoGeometryChildrenCount) {
		return IMAGE_NO_GEOMETRY;
	}

	if (iSemiSelectedChildrenCount > 0) {
		return IMAGE_SEMI_SELECTED;
	}

	if (iSelectedChildrenCount == 0) {
		return IMAGE_NOT_SELECTED;
	}

	if (iChildrenCount == iSelectedChildrenCount) {
		return IMAGE_SELECTED;
	}

	if ((iChildrenCount - iNoGeometryChildrenCount) == iSelectedChildrenCount) {
		return IMAGE_SELECTED;
	}

	ASSERT(iChildrenCount > iSelectedChildrenCount);
	return IMAGE_SEMI_SELECTED;
}

int CIFCModelStructureView::InMemoryTree_GetItemState(_ifc_node* pNode)
{
	ASSERT(pNode != nullptr);

	auto pInstance = pNode->getInstance();	

	// Leaf
	if (pNode->children().empty()) {
		ASSERT(pInstance != nullptr);
		return pInstance->hasGeometry() ?
			(pInstance->getEnable() ? IMAGE_SELECTED : IMAGE_NOT_SELECTED) :
			IMAGE_NO_GEOMETRY;
	}

	// Instance
	int iChildrenCount = (int)pNode->children().size() + ((pInstance != nullptr) && pInstance->hasGeometry() ? 1 : 0);
	int iSelectedChildrenCount = (pInstance != nullptr) && pInstance->hasGeometry() && pInstance->getEnable() ? 1 : 0;
	int iSemiSelectedChildrenCount = 0;
	int iNoGeometryChildrenCount = 0;

	for (auto pChild : pNode->children()) {
		int iChildState = InMemoryTree_GetItemState(pChild);
		switch (iChildState) {
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
		} // switch (iChildState)
	} // for (auto pChild : ...

	if (iChildrenCount == iNoGeometryChildrenCount) {
		return IMAGE_NO_GEOMETRY;
	}

	if (iSemiSelectedChildrenCount > 0) {
		return IMAGE_SEMI_SELECTED;
	}

	if (iSelectedChildrenCount == 0) {
		return IMAGE_NOT_SELECTED;
	}

	if (iChildrenCount == iSelectedChildrenCount) {
		return IMAGE_SELECTED;
	}

	if ((iChildrenCount - iNoGeometryChildrenCount) == iSelectedChildrenCount) {
		return IMAGE_SELECTED;
	}

	ASSERT(iChildrenCount > iSelectedChildrenCount);
	return IMAGE_SEMI_SELECTED;
}

void CIFCModelStructureView::ResetView()
{
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	m_pTreeCtrl->DeleteAllItems();

	for (auto pModelData : m_vecModelData) {
		delete pModelData;
	}
	m_vecModelData.clear();

	m_mapNodes.clear();

	m_vecSelectedInstances.clear();

	for (auto pModel : getController()->getModels()) {
		if (!pModel->getEnable()) {
			continue;
		}

		LoadModel(_ptr<_ifc_model>(pModel));
	}

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	TRACE(L"\n*** CIFCModelStructureView::ResetView(): %lld [ms]", std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count());
}