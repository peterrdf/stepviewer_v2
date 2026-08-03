#include "stdafx.h"
#include "AP242PModelStructureView.h"

#include "_ap242_model.h"
#include "_ap242_product_definition.h"
#include "_ap242_instance.h"
#include "_ap242_draughting_model.h"
#include "_ptr.h"

#include "mainfrm.h"
#include "Resource.h"
#include "STEPViewer.h"

#include <algorithm>
#include <chrono>
using namespace std;

// ************************************************************************************************
CAP242PModelStructureView::CAP242PModelStructureView(CTreeCtrlEx* pTreeCtrl)
	: CModelStructureViewBase(pTreeCtrl)
	, m_pModel(nullptr)
	, m_pModelStructure(nullptr)
	, m_pImageList(nullptr)
	, m_mapNodes()
	, m_mapInstanceIterators()
	, m_mapItems()
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

	// State provider
	m_pTreeCtrl->SetItemStateProvider(this);

	//  Search
	m_pSearchDialog = new CSearchTreeCtrlDialog(this);
	m_pSearchDialog->Create(IDD_DIALOG_SEARCH, m_pTreeCtrl);
}

/*virtual*/ CAP242PModelStructureView::~CAP242PModelStructureView()
{
	m_pImageList->DeleteImageList();
	delete m_pImageList;

	delete m_pModelStructure;

	for (auto itInstanceIterator : m_mapInstanceIterators) {
		delete itInstanceIterator.second;
	}

	m_pTreeCtrl->SetItemStateProvider(nullptr);

	delete m_pSearchDialog;
}

/*virtual*/ void CAP242PModelStructureView::onInstanceEnabledStateChanged(_view* pSender, _instance* pInstance, int /*iFlag*/) /*override*/
{
	if (pSender == this) {
		return;
	}

	//
	// Model
	//

	_ptr<_ap242_instance> apProductInstance(pInstance);

	auto itItems = m_mapItems.find(pInstance);
	if (itItems == m_mapItems.end()) {
		ASSERT(FALSE);
		return;
	}

	if (itItems->second.empty()) {
		ASSERT(FALSE);
		return;
	}

	HTREEITEM hItem = itItems->second.front();
	ASSERT(hItem != NULL);

	//auto pItemData = (CAP242ItemData*)m_pTreeCtrl->GetItemData(hItem);
	//if (pItemData == nullptr) {
	//	ASSERT(FALSE);
	//	return;
	//}

	////
	//// UI
	////

	//int iImage = apProductInstance->getEnable() ? IMAGE_SELECTED : IMAGE_NOT_SELECTED;
	//m_pTreeCtrl->SetItemImage(hItem, iImage, iImage);

	////Model_EnableChildren(pItemData, apProductInstance->getEnable()); #todo: Enable/Disable children recursively

	//Tree_UpdateChildren(hItem);
	//Tree_UpdateParents(m_pTreeCtrl->GetParentItem(hItem));
}

/*virtual*/ void CAP242PModelStructureView::onInstanceSelected(_view* pSender) /*override*/
{
	if (pSender == this) {
		return;
	}

	if (m_hSelectedItem != nullptr) {
		m_pTreeCtrl->SetItemState(m_hSelectedItem, 0, TVIS_BOLD);
		m_hSelectedItem = nullptr;
	}

	auto pController = getController();
	if (pController == nullptr) {
		ASSERT(FALSE);
		return;
	}

	auto pSelectedInstance = pController->getSelectedInstance() != nullptr ? dynamic_cast<_ap242_instance*>(getController()->getSelectedInstance()) : nullptr;

	//
	// Select the Model by default
	//
	if (pSelectedInstance == nullptr) {
		HTREEITEM hModel = m_pTreeCtrl->GetRootItem();
		ASSERT(hModel != nullptr);

		m_pTreeCtrl->SelectItem(hModel);
		return;
	}

	auto itItems = m_mapItems.find(pSelectedInstance);

	// Load branch
	if (itItems == m_mapItems.end()) {
		vector<_ap242_node*> vecPath;
		m_pModelStructure->getInstancePath(pSelectedInstance, vecPath);

		for (auto pNode : vecPath) {
			auto itNode = m_mapNodes.find(pNode);
			ASSERT(itNode != m_mapNodes.end());

			m_pTreeCtrl->Expand(itNode->second, TVE_EXPAND);
		}
	}

	itItems = m_mapItems.find(pSelectedInstance);
	if (itItems == m_mapItems.end()) {
		ASSERT(FALSE);
		return;
	}

	/*
	* Disable the drawing
	*/
	m_pTreeCtrl->SendMessage(WM_SETREDRAW, 0, 0);

	m_hSelectedItem = itItems->second.front();

	m_pTreeCtrl->SetItemState(m_hSelectedItem, TVIS_BOLD, TVIS_BOLD);
	m_pTreeCtrl->EnsureVisible(m_hSelectedItem);
	m_pTreeCtrl->SelectItem(m_hSelectedItem);

	/*
	* Enable the drawing
	*/
	m_pTreeCtrl->SendMessage(WM_SETREDRAW, 1, 0);
}

/*virtual*/ void CAP242PModelStructureView::onApplicationPropertyChanged(_view* pSender, enumApplicationProperty enApplicationProperty) /*override*/
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
	if (!bShow) {
		m_pSearchDialog->ShowWindow(SW_HIDE);
	}
}

/*virtual*/ void CAP242PModelStructureView::OnTreeItemClick(NMHDR* /*pNMHDR*/, LRESULT* pResult) /*override*/
{
	*pResult = 0;

	if (m_bInitInProgress) {
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
	if ((hItem != nullptr) && ((uFlags & TVHT_ONITEMICON) == TVHT_ONITEMICON)) {
		auto pController = getController();
		if (pController == nullptr) {
			ASSERT(FALSE);

			return;
		}

		int iImage, iSelectedImage = -1;
		m_pTreeCtrl->GetItemImage(hItem, iImage, iSelectedImage);

		ASSERT(iImage == iSelectedImage);

		switch (iImage) {
			case IMAGE_SELECTED:
			case IMAGE_SEMI_SELECTED:
			case IMAGE_NOT_SELECTED:
				{
					bool bGeometryItem = false;
					auto pNode = (_ap242_node*)m_pTreeCtrl->GetItemData(hItem);
					_ap242_instance* pInstance = pNode ? pNode->getInstance() : nullptr;
					if ((pInstance == nullptr) &&
						((iImage == IMAGE_SELECTED) || (iImage == IMAGE_NOT_SELECTED)) &&
						!m_pTreeCtrl->ItemHasChildren(hItem) &&
						(m_pTreeCtrl->GetItemText(hItem) == ITEM_GEOMETRY)) {
						HTREEITEM hParent = m_pTreeCtrl->GetParentItem(hItem);
						ASSERT(hParent != NULL);

						pNode = (_ap242_node*)m_pTreeCtrl->GetItemData(hParent);
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
							ASSERT(hItem == GetModelItem());

							for (auto pRootProduct : m_pModelStructure->getRootProducts()) {
								InMemoryTree_EnableChildren(pRootProduct, iImage == IMAGE_NOT_SELECTED);
							}
						}
					}

					//
					// UI
					//

					Tree_Update(GetModelItem());

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
	} // if ((hItem != nullptr) && ...

	/*
	* TVHT_ONITEMLABEL
	*/
	if ((hItem != nullptr) && ((uFlags & TVHT_ONITEMLABEL) == TVHT_ONITEMLABEL)) {
		if (m_pTreeCtrl->GetParentItem(m_hSelectedItem) != nullptr) {
			// keep the roots always bold
			m_pTreeCtrl->SetItemState(m_hSelectedItem, 0, TVIS_BOLD);
		}

		m_pTreeCtrl->SetItemState(hItem, TVIS_BOLD, TVIS_BOLD);
		m_hSelectedItem = hItem;

		auto pNode = (_ap242_node*)m_pTreeCtrl->GetItemData(hItem);
		if ((pNode == nullptr) || (pNode->getInstance() == nullptr)) {
			getController()->selectInstance(this, nullptr);
		}
		else {
			getController()->selectInstance(this, pNode->getInstance());
		}
	} // if ((hItem != nullptr) && ...
}

/*virtual*/ void CAP242PModelStructureView::OnTreeItemExpanding(NMHDR* pNMHDR, LRESULT* pResult) /*override*/
{
	*pResult = 0;

	auto pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	if (m_pTreeCtrl->GetChildItem(pNMTreeView->itemNew.hItem) != NULL) {
		// it is loaded already
		return;
	}

	auto pNode = (_ap242_node*)m_pTreeCtrl->GetItemData(pNMTreeView->itemNew.hItem);
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

	for (auto pChildNode : pNode->children()) {
		auto pChildInstance = pChildNode->getInstance();

		wstring strItemName;
		if (pChildNode->getSdaiInstance() != 0) {
			strItemName = _ap_geometry::getDisplayString(pChildNode->getSdaiInstance(), apController->getFullDisplayName());
		}
		else {
			strItemName = (LPCWSTR)CA2W(pChildNode->getId().c_str());
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

		auto itItems = m_mapItems.find(pChildInstance);
		if (itItems != m_mapItems.end()) {
			itItems->second.push_back(hChildItem);
		}
		else {
			m_mapItems[pChildInstance] = vector<HTREEITEM>{ hChildItem };
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

/*virtual*/ bool CAP242PModelStructureView::IsSelected(HTREEITEM hItem)
{
	auto pController = getController();
	if (pController == nullptr) {
		return false;
	}

	if (pController->getSelectedInstance() == nullptr) {
		return false;
	}

	auto pNode = (_ap242_node*)m_pTreeCtrl->GetItemData(hItem);
	if ((pNode == nullptr) || (pNode->getInstance() == nullptr)) {
		return false;
	}

	return pNode->getInstance() == dynamic_cast<_ap242_instance*>(pController->getSelectedInstance());
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
			_T("Product Instances"),
			_T("Draughting Model"),
			_T("Annotation Plane"),
			_T("Draughting Callout")
	};
}

/*virtual*/ void CAP242PModelStructureView::LoadChildrenIfNeeded(HTREEITEM hItem) /*override*/
{
	if (hItem == NULL) {
		ASSERT(FALSE);

		return;
	}

	TVITEMW tvItem = {};
	tvItem.hItem = hItem;
	tvItem.mask = TVIF_HANDLE | TVIF_CHILDREN;

	if (!GetTreeView()->GetItem(&tvItem)) {
		ASSERT(FALSE);

		return;
	}

	if (tvItem.cChildren != 1) {
		return;
	}

	HTREEITEM hChild = GetTreeView()->GetChildItem(hItem);
	if (hChild == NULL) {
		ASSERT(FALSE);

		return;
	}

	if (GetTreeView()->GetItemText(hChild) == ITEM_PENDING_LOAD) {
		GetTreeView()->Expand(hItem, TVE_EXPAND);
	}
}

/*virtual*/ BOOL CAP242PModelStructureView::ContainsText(int iFilter, HTREEITEM hItem, const CString& strText) /*override*/
{
	if (hItem == NULL) {
		ASSERT(FALSE);

		return FALSE;
	}

	//CString strItemText = GetTreeView()->GetItemText(hItem);
	//strItemText.MakeLower();

	//CString strTextLower = strText;
	//strTextLower.MakeLower();

	//auto pItemData = (CAP242ItemData*)GetTreeView()->GetItemData(hItem);

	//// Product Definitions
	//if (iFilter == (int)enumSearchFilter::ProductDefitions) {
	//	if ((pItemData != nullptr) && (pItemData->GetType() == enumAP242ItemDataType::ProductDefinition)) {
	//		return strItemText.Find(strText, 0) != -1;
	//	}

	//	return FALSE;
	//}

	//// Assemblies
	//if (iFilter == (int)enumSearchFilter::Assemblies) {
	//	if ((pItemData != nullptr) && (pItemData->GetType() == enumAP242ItemDataType::Assembly)) {
	//		return strItemText.Find(strText, 0) != -1;
	//	}

	//	return FALSE;
	//}

	//// Product Instance
	//if (iFilter == (int)enumSearchFilter::ProductInstances) {
	//	if ((pItemData != nullptr) && (pItemData->GetType() == enumAP242ItemDataType::ProductInstance)) {
	//		return strItemText.Find(strText, 0) != -1;
	//	}

	//	return FALSE;
	//}

	//// Draughting Model
	//if (iFilter == (int)enumSearchFilter::DraughtingModel) {
	//	if ((pItemData != nullptr) && (pItemData->GetType() == enumAP242ItemDataType::DraughtingModel)) {
	//		return strItemText.Find(strText, 0) != -1;
	//	}

	//	return FALSE;
	//}

	//// Annotation Plane
	//if (iFilter == (int)enumSearchFilter::AnnotationPlane) {
	//	if ((pItemData != nullptr) && (pItemData->GetType() == enumAP242ItemDataType::AnnotationPlane)) {
	//		return strItemText.Find(strText, 0) != -1;
	//	}

	//	return FALSE;
	//}

	//// Draughting Callout
	//if (iFilter == (int)enumSearchFilter::DraughtingCallout) {
	//	if ((pItemData != nullptr) && (pItemData->GetType() == enumAP242ItemDataType::DraughtingCallout)) {
	//		return strItemText.Find(strText, 0) != -1;
	//	}

	//	return FALSE;
	//}

	//// All
	//return strItemText.Find(strTextLower, 0) != -1;

	return FALSE;
}

/*virtual*/ void CAP242PModelStructureView::OnContextMenu(CWnd* /*pWnd*/, CPoint point) /*override*/
{
	if (point == CPoint(-1, -1)) {
		return;
	}

	auto pController = getAPController();
	if (pController == nullptr) {
		ASSERT(FALSE);

		return;
	}

	if (pController->getModels().empty()) {
		return;
	}

	ASSERT(pController->getModels().size() == 1);

	auto pModel = _ptr<_ap242_model>(pController->getModels()[0]);
	if (pModel == nullptr) {
		return;
	}

	// Select clicked item:
	CPoint ptTree = point;
	m_pTreeCtrl->ScreenToClient(&ptTree);

	UINT flags = 0;
	HTREEITEM hItem = m_pTreeCtrl->HitTest(ptTree, &flags);
	if (hItem == nullptr) {
		return;
	}

	m_pTreeCtrl->SelectItem(hItem);
	m_pTreeCtrl->SetFocus();

	/*
	* Instances
	*/
	auto pTargetNode = (_ap242_node*)m_pTreeCtrl->GetItemData(hItem);
	auto pTargetInstance = pTargetNode ? pTargetNode->getInstance() : nullptr;	
	if ((pTargetInstance != nullptr) && (pTargetInstance != nullptr) &&
		((pTargetNode->getType() == _ap242_node_type::ProductInstance) ||
			(pTargetNode->getType() == _ap242_node_type::ProductShape) ||
			(pTargetNode->getType() == _ap242_node_type::ProductShapeRepresentation) ||
			(pTargetNode->getType() == _ap242_node_type::ProductShapeRepresentationItem) ||
			(pTargetNode->getType() == _ap242_node_type::AnnotationPlane) ||
			(pTargetNode->getType() == _ap242_node_type::DraughtingCallout))) {
		CMenu menu;
		VERIFY(menu.LoadMenuW(IDR_POPUP_INSTANCES));

		auto pMenu = menu.GetSubMenu(0);

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

		UINT uiCommand = pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, point.x, point.y, m_pTreeCtrl);
		if (uiCommand == 0) {
			return;
		}

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

					Tree_Update(GetModelItem());

					pController->onInstanceEnabledStateChanged(this, pTargetInstance, 0);
				}
				break;

			case ID_INSTANCES_DISABLE_ALL_BUT_THIS:
				{
					//
						// Model
						//

					for (auto pInstance : GetModel()->getInstances()) {
						pInstance->setEnable(pTargetInstance == pInstance);
					}

					//
					// In Memory Tree
					//

					InMemoryTree_EnableChildren(pTargetNode, pTargetInstance->getEnable());

					//
					// UI
					//

					Tree_Update(GetModelItem());

					pController->onInstancesEnabledStateChanged(this);
				}
				break;

			case ID_INSTANCES_ENABLE_ALL:
				{
					for (auto pInstance : GetModel()->getInstances()) {
						pInstance->setEnable(true);
					}

					Tree_Update(GetModelItem());

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
	else {
		CMenu menu;
		VERIFY(menu.LoadMenuW(IDR_POPUP_META_DATA));

		auto pMenu = menu.GetSubMenu(0);

		pMenu->EnableMenuItem(ID_INSTANCES_ZOOM_TO_CHILDREN, MF_BYCOMMAND | MF_DISABLED);

		UINT uiCommand = pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, point.x, point.y, m_pTreeCtrl);
		if (uiCommand == 0) {
			return;
		}

		switch (uiCommand) {
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
	if (!m_pSearchDialog->IsWindowVisible()) {
		m_pSearchDialog->ShowWindow(SW_SHOW);
	}
	else {
		m_pSearchDialog->ShowWindow(SW_HIDE);
	}
}

void CAP242PModelStructureView::LoadModel()
{
	auto pController = getAPController();
	if (pController == nullptr) {
		ASSERT(FALSE);

		return;
	}

	if (pController->getModels().empty()) {
		return;
	}

	ASSERT(pController->getModels().size() == 1);

	auto pModel = _ptr<_ap242_model>(pController->getModels()[0]);
	if (pModel == nullptr) {
		return;
	}

	if (pModel->getGeometries().empty()) {
		return;
	}

	m_pModel = pModel;
	m_pModelStructure = pModel->getModelStructure();

	_ptr<_ap_controller> apController(pController);

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	m_bInitInProgress = true;

	//
	// Model
	//
	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = nullptr;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = (LPWSTR)pModel->getPath();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_SELECTED;
	tvInsertStruct.item.lParam = (LPARAM)m_pModelStructure->getModelNode();
	HTREEITEM hModel = m_pTreeCtrl->InsertItem(&tvInsertStruct);

	ASSERT(m_mapNodes.find(m_pModelStructure->getModelNode()) == m_mapNodes.end());
	m_mapNodes[m_pModelStructure->getModelNode()] = hModel;

	//
	// Header
	//
	LoadHeader(pModel, hModel);

	//
	// Roots
	//
	for (auto pRootProduct : m_pModelStructure->getRootProducts()) {
		wstring strItem = _ap_geometry::getDisplayString(pRootProduct->getSdaiInstance(), apController->getFullDisplayName());

		TV_INSERTSTRUCT tvInsertStruct;
		tvInsertStruct.hParent = hModel;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
		tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = InMemoryTree_GetItemState(pRootProduct);
		tvInsertStruct.item.lParam = (LPARAM)pRootProduct;
		tvInsertStruct.item.cChildren = pRootProduct->children().size() > 0 ? 1 : 0;
		HTREEITEM hProduct = m_pTreeCtrl->InsertItem(&tvInsertStruct);

		ASSERT(m_mapNodes.find(pRootProduct) == m_mapNodes.end());
		m_mapNodes[pRootProduct] = hProduct;
	}

	m_pTreeCtrl->Expand(hModel, TVE_EXPAND);

	m_bInitInProgress = false;

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	TRACE(L"\n*** CAP242PModelStructureView::LoadModel() : %lld [µs]", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());
}

HTREEITEM CAP242PModelStructureView::GetModelItem() const
{
	ASSERT(m_pTreeCtrl != nullptr);

	return m_pTreeCtrl->GetRootItem();
}

void CAP242PModelStructureView::Tree_UpdateChildren(HTREEITEM hItem)
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
		auto pNode = (_ap242_node*)m_pTreeCtrl->GetItemData(hItem);
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

void CAP242PModelStructureView::Tree_Update(HTREEITEM hItem, bool bRecursive/* = true*/)
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

void CAP242PModelStructureView::InMemoryTree_EnableChildren(_ap242_node* pNode, bool bEnable)
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

void CAP242PModelStructureView::Tree_UpdateParents(HTREEITEM hItem)
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

int CAP242PModelStructureView::Tree_GetItemState(HTREEITEM hItem)
{
	auto pNode = (_ap242_node*)m_pTreeCtrl->GetItemData(hItem);
	if (pNode == nullptr) {
		if (m_pTreeCtrl->GetItemText(hItem) == ITEM_GEOMETRY) {
			// The Geometry Item of an Instance
			HTREEITEM hParent = m_pTreeCtrl->GetParentItem(hItem);
			ASSERT(hParent != NULL);

			pNode = (_ap242_node*)m_pTreeCtrl->GetItemData(hParent);
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

int CAP242PModelStructureView::InMemoryTree_GetItemState(_ap242_node* pNode)
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

void CAP242PModelStructureView::ResetView()
{
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	m_pTreeCtrl->DeleteAllItems();

	m_pModel = nullptr;
	m_pModelStructure = nullptr;

	m_mapNodes.clear();
	m_mapInstanceIterators.clear();
	m_mapItems.clear();
	m_hSelectedItem = nullptr;

	m_pSearchDialog->Reset();

	LoadModel();

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	TRACE(L"\n*** CAP242PModelStructureView::ResetView(): %lld [ms]", std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count());
}
