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
	, m_pImageList(nullptr)
	, m_mapInstanceIterators()
	, m_mapInstance2Item()
	, m_vecItemData()
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
	m_pTreeCtrl->SetImageList(nullptr, TVSIL_NORMAL);

	m_pImageList->DeleteImageList();
	delete m_pImageList;

	for (auto itInstanceIterator : m_mapInstanceIterators)
	{
		delete itInstanceIterator.second;
	}

	for (size_t iItemData = 0; iItemData < m_vecItemData.size(); iItemData++)
	{
		delete m_vecItemData[iItemData];
	}

	m_pTreeCtrl->SetItemStateProvider(nullptr);

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

	auto pSelectedInstance = pController->getSelectedInstance() != nullptr ? dynamic_cast<_ap242_instance*>(getController()->getSelectedInstance()) : nullptr;
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

	ASSERT(itInstance2Item->second != NULL);
	m_hSelectedItem = itInstance2Item->second;

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
				if ((pItemData == nullptr) &&
					(iImage == IMAGE_SELECTED) &&
					!m_pTreeCtrl->ItemHasChildren(hItem) &&
					(m_pTreeCtrl->GetItemText(hItem) == ITEM_GEOMETRY))
				{
					HTREEITEM hParent = m_pTreeCtrl->GetParentItem(hItem);
					ASSERT(hParent != NULL);

					pItemData = (CAP242ItemData*)m_pTreeCtrl->GetItemData(hParent);
				}

				if ((pItemData != nullptr) && 
					((pItemData->GetType() == enumAP242ItemDataType::ProductInstance) ||
					(pItemData->GetType() == enumAP242ItemDataType::AnnotationPlane) ||
					(pItemData->GetType() == enumAP242ItemDataType::DraughtingCallout)))
				{
					pItemData->GetInstance<_instance>()->setEnable(false);
				}
				else if (pItemData->GetType() == enumAP242ItemDataType::DraughtingModel)
				{
					pItemData->GetInstance<_ap242_draughting_model>()->enableInstances(false);
				}
				
				Model_EnableChildren(pItemData, false);
				
				m_pTreeCtrl->SetItemImage(hItem, IMAGE_NOT_SELECTED, IMAGE_NOT_SELECTED);

				Tree_UpdateChildren(hItem);
				Tree_UpdateParents(m_pTreeCtrl->GetParentItem(hItem));

				pController->onInstancesEnabledStateChanged(this);
			}
			break;

			case IMAGE_NOT_SELECTED:
			{
				auto pItemData = (CAP242ItemData*)m_pTreeCtrl->GetItemData(hItem);
				if ((pItemData == nullptr) &&
					!m_pTreeCtrl->ItemHasChildren(hItem) && 
					(m_pTreeCtrl->GetItemText(hItem) == ITEM_GEOMETRY))
				{
					HTREEITEM hParent = m_pTreeCtrl->GetParentItem(hItem);
					ASSERT(hParent != NULL);

					pItemData = (CAP242ItemData*)m_pTreeCtrl->GetItemData(hParent);
				}

				if ((pItemData != nullptr) &&
					((pItemData->GetType() == enumAP242ItemDataType::ProductInstance) ||
					(pItemData->GetType() == enumAP242ItemDataType::AnnotationPlane) ||
					(pItemData->GetType() == enumAP242ItemDataType::DraughtingCallout)))
				{
					pItemData->GetInstance<_instance>()->setEnable(true);
				}
				else if (pItemData->GetType() == enumAP242ItemDataType::DraughtingModel)
				{
					pItemData->GetInstance<_ap242_draughting_model>()->enableInstances(true);
				}

				Model_EnableChildren(pItemData, true);

				m_pTreeCtrl->SetItemImage(hItem, IMAGE_SELECTED, IMAGE_SELECTED);

				Tree_UpdateChildren(hItem);
				Tree_UpdateParents(m_pTreeCtrl->GetParentItem(hItem));

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
		if ((pItemData == nullptr) ||
			((pItemData->GetType() != enumAP242ItemDataType::ProductInstance) &&
				(pItemData->GetType() != enumAP242ItemDataType::AnnotationPlane) &&
				(pItemData->GetType() != enumAP242ItemDataType::DraughtingCallout)))
		{
			getController()->selectInstance(this, nullptr);

			return;
		}

		auto pInstance = pItemData->GetInstance<_ap242_instance>();
		ASSERT(pInstance != nullptr);

		getController()->selectInstance(this, pInstance);
	} // if ((hItem != nullptr) && ...
}

/*virtual*/ void CAP242PModelStructureView::OnTreeItemExpanding(NMHDR* /*pNMHDR*/, LRESULT* pResult) /*override*/
{
	*pResult = 0;
}

/*virtual*/ bool CAP242PModelStructureView::IsSelected(HTREEITEM hItem)
{
	auto pController = getController();
	if (pController == nullptr)
	{
		return false;
	}

	auto pItemData = (CAP242ItemData*)m_pTreeCtrl->GetItemData(hItem);
	if ((pItemData != nullptr) &&
		((pItemData->GetType() == enumAP242ItemDataType::ProductInstance) ||
			(pItemData->GetType() == enumAP242ItemDataType::AnnotationPlane) ||
			(pItemData->GetType() == enumAP242ItemDataType::DraughtingCallout)))
	{
		return pItemData->GetInstance<_instance>() == pController->getSelectedInstance();
	}

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
			_T("Product Instances"),
			_T("Draughting Model"),
			_T("Annotation Plane"),
			_T("Draughting Callout")
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

	auto pItemData = (CAP242ItemData*)GetTreeView()->GetItemData(hItem);

	// Product Definitions
	if (iFilter == (int)enumSearchFilter::ProductDefitions)
	{
		if ((pItemData != nullptr) && (pItemData->GetType() == enumAP242ItemDataType::ProductDefinition))
		{
			return strItemText.Find(strText, 0) != -1;
		}

		return FALSE;
	}

	// Assemblies
	if (iFilter == (int)enumSearchFilter::Assemblies)
	{
		if ((pItemData != nullptr) && (pItemData->GetType() == enumAP242ItemDataType::Assembly))
		{
			return strItemText.Find(strText, 0) != -1;
		}

		return FALSE;
	}

	// Product Instance
	if (iFilter == (int)enumSearchFilter::ProductInstances)
	{
		if ((pItemData != nullptr) && (pItemData->GetType() == enumAP242ItemDataType::ProductInstance))
		{
			return strItemText.Find(strText, 0) != -1;
		}

		return FALSE;
	}

	// Draughting Model
	if (iFilter == (int)enumSearchFilter::DraughtingModel)
	{
		if ((pItemData != nullptr) && (pItemData->GetType() == enumAP242ItemDataType::DraughtingModel))
		{
			return strItemText.Find(strText, 0) != -1;
		}

		return FALSE;
	}

	// Annotation Plane
	if (iFilter == (int)enumSearchFilter::AnnotationPlane)
	{
		if ((pItemData != nullptr) && (pItemData->GetType() == enumAP242ItemDataType::AnnotationPlane))
		{
			return strItemText.Find(strText, 0) != -1;
		}

		return FALSE;
	}

	// Draughting Callout
	if (iFilter == (int)enumSearchFilter::DraughtingCallout)
	{
		if ((pItemData != nullptr) && (pItemData->GetType() == enumAP242ItemDataType::DraughtingCallout))
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

	auto pModel = getModelAs<_ap242_model>();
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
	if ((pItemData != nullptr) &&
		((pItemData->GetType() == enumAP242ItemDataType::ProductInstance) ||
			(pItemData->GetType() == enumAP242ItemDataType::AnnotationPlane) ||
			(pItemData->GetType() == enumAP242ItemDataType::DraughtingCallout)))
	{
		auto pTargetInstance = pItemData->GetInstance<_ap_instance>();

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
				pTargetInstance->setEnable(!pTargetInstance->getEnable());

				int iImage = pTargetInstance->getEnable() ? IMAGE_SELECTED : IMAGE_NOT_SELECTED;
				m_pTreeCtrl->SetItemImage(hItem, iImage, iImage);
				
				Model_EnableChildren(pItemData, pTargetInstance->getEnable());
				
				Tree_UpdateChildren(hItem);
				Tree_UpdateParents(m_pTreeCtrl->GetParentItem(hItem));
								
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
				ASSERT(itInstance2Item != m_mapInstance2Item.end());
				ASSERT(itInstance2Item->second != NULL);

				HTREEITEM hGeometry = m_pTreeCtrl->GetChildItem(itInstance2Item->second);
				ASSERT((hGeometry != nullptr) && !m_pTreeCtrl->ItemHasChildren(hGeometry) && (m_pTreeCtrl->GetItemText(hGeometry) == ITEM_GEOMETRY));

				m_pTreeCtrl->SetItemImage(hGeometry, IMAGE_SELECTED, IMAGE_SELECTED);

				Tree_UpdateChildren(hGeometry);
				Tree_UpdateParents(m_pTreeCtrl->GetParentItem(hGeometry));

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

	auto pModel = getModelAs<_ap242_model>();
	if (pModel == nullptr)
	{
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
	auto pModelItemData = new CAP242ItemData(nullptr, (int64_t*)pModel, enumAP242ItemDataType::Model);
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
	pModelItemData->TreeItem() = hModel;

	// Header
	LoadHeader(hModel);

	//
	// Roots
	//

	// Product definitions
	for (auto pGeometry : pModel->getGeometries())
	{
		auto pProduct = dynamic_cast<_ap242_product_definition*>(pGeometry);		
		if ((pProduct != nullptr) && (pProduct->getRelatedProducts() == 0))
		{
			LoadProduct(pModel, pProduct, hModel);
		}
	}

	// Draughitng models
	for (auto pDraughtingModel : pModel->getDraughtingModels())
	{
		LoadDraughtingModel(pDraughtingModel, hModel);
	}

	m_pTreeCtrl->Expand(hModel, TVE_EXPAND);

	m_bInitInProgress = false;

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	TRACE(L"\n*** CAP242PModelStructureView::LoadModel() : %lld [µs]", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());
}

void CAP242PModelStructureView::LoadProduct(_ap242_model* pModel, _ap242_product_definition* pProduct, HTREEITEM hParent)
{
	if ((pModel == nullptr) || (pProduct == nullptr))
	{
		ASSERT(FALSE);

		return;
	}

	// 
	// Product
	//

	CString strName;
	strName.Format(L"#%lld %s %s", pProduct->getExpressID(), pProduct->getProductName(), ITEM_PRODUCT_DEFINION);

	int iProductImage = pProduct->hasGeometry() || HasDescendantsWithGeometry(pModel, pProduct) ? IMAGE_SELECTED : IMAGE_NO_GEOMETRY;
	HTREEITEM hProduct = m_pTreeCtrl->InsertItem(strName, iProductImage, iProductImage, hParent);
	int iGeometryImage = pProduct->hasGeometry() ? IMAGE_SELECTED : IMAGE_NO_GEOMETRY;
	m_pTreeCtrl->InsertItem(ITEM_GEOMETRY, iGeometryImage, iGeometryImage, hProduct);

	auto* pParentItemData = (CAP242ItemData*)m_pTreeCtrl->GetItemData(hParent);
	ASSERT(pParentItemData != nullptr);

	//
	// I) Do not show Instances
	//

	//
	// Instance
	//

	// Iterator
	_instance_iterator* pInstanceIterator = nullptr;
	auto itInstanceIterator = m_mapInstanceIterators.find(pProduct);
	if (itInstanceIterator == m_mapInstanceIterators.end())
	{
		pInstanceIterator = new _instance_iterator(pProduct->getInstances());
		m_mapInstanceIterators[pProduct] = pInstanceIterator;
	}
	else
	{
		pInstanceIterator = itInstanceIterator->second;
	}

	// Load
	_ptr<_ap242_instance> apProductInstance(pInstanceIterator->getNextItem());
	if (apProductInstance)
	{
		auto pItemData = new CAP242ItemData(pParentItemData, (int64_t*)apProductInstance.p(), enumAP242ItemDataType::ProductInstance);
		pParentItemData->Children().push_back(pItemData);

		m_pTreeCtrl->SetItemData(hProduct, (DWORD_PTR)pItemData);

		ASSERT(m_mapInstance2Item.find(apProductInstance) == m_mapInstance2Item.end());
		m_mapInstance2Item[apProductInstance] = hProduct;
	}
	else
	{
		ASSERT(FALSE);
	}

	// Assemblies
	for (auto itExpressID2Assembly : pModel->getExpressID2Assembly())
	{
		if (itExpressID2Assembly.second->getRelatingProductDefinition() == pProduct)
		{
			LoadAssembly(pModel, itExpressID2Assembly.second, hProduct);
		}
	}

	//
	// II) Show Instances
	//

	/*auto pItemData = new CAP242ItemData(pParentItemData, (int64_t*)pProduct, enumAP242ItemDataType::ProductDefinition);
	pParentItemData->children().push_back(pItemData);

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

	//
	// Instances
	//

	// Iterator
	_instance_iterator* pInstanceIterator = nullptr;
	auto itInstanceIterator = m_mapInstanceIterators.find(pProduct);
	if (itInstanceIterator == m_mapInstanceIterators.end())
	{
		pInstanceIterator = new _instance_iterator(pProduct->getInstances());
		m_mapInstanceIterators[pProduct] = pInstanceIterator;
	}
	else
	{
		pInstanceIterator = itInstanceIterator->second;
	}

	// Load
	_ptr<_ap242_instance> apProductInstance(pInstanceIterator->getNextItem());
	if (apProductInstance)
	{
		LoadInstance(pModel, apProductInstance, hProduct);
	}
	else
	{
		ASSERT(FALSE);
	}*/
}

void CAP242PModelStructureView::LoadAssembly(_ap242_model* pModel, _ap242_assembly* pAssembly, HTREEITEM hParent)
{
	if ((pModel == nullptr) || (pAssembly == nullptr))
	{
		ASSERT(FALSE);

		return;
	}

	CString strName;
	strName.Format(L"#%lld %s %s", pAssembly->getExpressID(), pAssembly->getName(), ITEM_ASSEMBLY);

	int iAssemblyImage = HasDescendantsWithGeometry(pModel, pAssembly) ? IMAGE_SELECTED : IMAGE_NO_GEOMETRY;
	HTREEITEM hAssembly = m_pTreeCtrl->InsertItem(strName, iAssemblyImage, iAssemblyImage, hParent);
	m_pTreeCtrl->InsertItem(ITEM_GEOMETRY, IMAGE_NO_GEOMETRY, IMAGE_NO_GEOMETRY, hAssembly);

	auto* pParentItemData = (CAP242ItemData*)m_pTreeCtrl->GetItemData(hParent);
	ASSERT(pParentItemData != nullptr);

	auto pItemData = new CAP242ItemData(pParentItemData, (int64_t*)pAssembly, enumAP242ItemDataType::Assembly);
	pParentItemData->Children().push_back(pItemData);

	m_vecItemData.push_back(pItemData);

	m_pTreeCtrl->SetItemData(hAssembly, (DWORD_PTR)pItemData);

	LoadProduct(pModel, pAssembly->getRelatedProductDefinition(), hAssembly);
}

void CAP242PModelStructureView::LoadInstance(_ap242_model* pModel, _ap242_instance* pInstance, HTREEITEM hParent)
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

	auto* pParentItemData = (CAP242ItemData*)m_pTreeCtrl->GetItemData(hParent);
	ASSERT(pParentItemData != nullptr);

	auto pItemData = new CAP242ItemData(pParentItemData, (int64_t*)pInstance, enumAP242ItemDataType::ProductInstance);
	pParentItemData->Children().push_back(pItemData);

	m_vecItemData.push_back(pItemData);

	m_pTreeCtrl->SetItemData(hInstance, (DWORD_PTR)pItemData);

	ASSERT(m_mapInstance2Item.find(pInstance) == m_mapInstance2Item.end());
	m_mapInstance2Item[pInstance] = hInstance;
}

void CAP242PModelStructureView::LoadDraughtingModel(_ap242_draughting_model* pDraugthingModel, HTREEITEM hParent)
{
	if (pDraugthingModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	CString strName;
	strName.Format(L"#%lld %s %s", pDraugthingModel->getExpressID(), pDraugthingModel->getName(), ITEM_DRAUGHTING_MODEL);

	int iImage = HasDescendantsWithGeometry(pDraugthingModel) ? IMAGE_SELECTED : IMAGE_NO_GEOMETRY;
	HTREEITEM hDraugthingModel = m_pTreeCtrl->InsertItem(strName, iImage, iImage, hParent);
	m_pTreeCtrl->InsertItem(ITEM_GEOMETRY, IMAGE_NO_GEOMETRY, IMAGE_NO_GEOMETRY, hDraugthingModel);

	auto* pParentItemData = (CAP242ItemData*)m_pTreeCtrl->GetItemData(hParent);
	ASSERT(pParentItemData != nullptr);

	auto pItemData = new CAP242ItemData(pParentItemData, (int64_t*)pDraugthingModel, enumAP242ItemDataType::DraughtingModel);
	pParentItemData->Children().push_back(pItemData);
	m_pTreeCtrl->SetItemData(hDraugthingModel, (DWORD_PTR)pItemData);

	for (auto pAnnotationPlane : pDraugthingModel->getAnnotationPlanes())
	{
		LoadAnnotationPlane(pAnnotationPlane, hDraugthingModel);
	}

	for (auto pDraughtingCallout : pDraugthingModel->getDraughtingCallouts())
	{
		LoadDraughtingCallout(pDraughtingCallout, hDraugthingModel);
	}
}

void CAP242PModelStructureView::LoadAnnotationPlane(_ap242_annotation_plane* pAnnotationPlane, HTREEITEM hParent)
{
	if (pAnnotationPlane == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	CString strName;
	strName.Format(L"#%lld %s %s", pAnnotationPlane->getExpressID(), pAnnotationPlane->getName(), ITEM_ANNOTATION_PLANE);

	int iImage = pAnnotationPlane->hasGeometry() ? IMAGE_SELECTED : IMAGE_NO_GEOMETRY;
	HTREEITEM hAnnotationPlane = m_pTreeCtrl->InsertItem(strName, iImage, iImage, hParent);
	m_pTreeCtrl->InsertItem(ITEM_GEOMETRY, iImage, iImage, hAnnotationPlane);

	auto* pParentItemData = (CAP242ItemData*)m_pTreeCtrl->GetItemData(hParent);
	ASSERT(pParentItemData != nullptr);

	ASSERT(pAnnotationPlane->getInstances().size() == 1);
	auto pInstance = pAnnotationPlane->getInstances()[0];

	auto pItemData = new CAP242ItemData(pParentItemData, (int64_t*)pInstance, enumAP242ItemDataType::AnnotationPlane);
	pParentItemData->Children().push_back(pItemData);

	m_pTreeCtrl->SetItemData(hAnnotationPlane, (DWORD_PTR)pItemData);

	ASSERT(m_mapInstance2Item.find(pInstance) == m_mapInstance2Item.end());
	m_mapInstance2Item[pInstance] = hAnnotationPlane;
}

void CAP242PModelStructureView::LoadDraughtingCallout(_ap242_draughting_callout* pDraugthingCallout, HTREEITEM hParent)
{
	if (pDraugthingCallout == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	CString strName;
	strName.Format(L"#%lld %s %s", pDraugthingCallout->getExpressID(), pDraugthingCallout->getName(), ITEM_DRAUGHTING_CALLOUT);

	int iImage = pDraugthingCallout->hasGeometry() ? IMAGE_SELECTED : IMAGE_NO_GEOMETRY;
	HTREEITEM hDraugthingCallout = m_pTreeCtrl->InsertItem(strName, iImage, iImage, hParent);
	m_pTreeCtrl->InsertItem(ITEM_GEOMETRY, iImage, iImage, hDraugthingCallout);

	auto* pParentItemData = (CAP242ItemData*)m_pTreeCtrl->GetItemData(hParent);
	ASSERT(pParentItemData != nullptr);

	ASSERT(pDraugthingCallout->getInstances().size() == 1);
	auto pInstance = pDraugthingCallout->getInstances()[0];

	auto pItemData = new CAP242ItemData(pParentItemData, (int64_t*)pInstance, enumAP242ItemDataType::DraughtingCallout);
	pParentItemData->Children().push_back(pItemData);

	m_pTreeCtrl->SetItemData(hDraugthingCallout, (DWORD_PTR)pItemData);

	ASSERT(m_mapInstance2Item.find(pInstance) == m_mapInstance2Item.end());
	m_mapInstance2Item[pInstance] = hDraugthingCallout;
}

bool CAP242PModelStructureView::HasDescendantsWithGeometry(_ap242_model* pModel, _ap242_product_definition* pProduct)
{
	if ((pModel == nullptr) || (pProduct == nullptr))
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

void CAP242PModelStructureView::HasDescendantsWithGeometryRecursively(_ap242_model* pModel, _ap242_product_definition* pProduct, bool& bHasDescendantWithGeometry)
{
	if ((pModel == nullptr) || (pProduct == nullptr))
	{
		ASSERT(FALSE);

		return;
	}

	for (auto itExpressID2Assembly : pModel->getExpressID2Assembly())
	{
		if (itExpressID2Assembly.second->getRelatingProductDefinition() == pProduct)
		{
			bHasDescendantWithGeometry = HasDescendantsWithGeometry(pModel, itExpressID2Assembly.second->getRelatedProductDefinition());
			if (bHasDescendantWithGeometry)
			{
				break;
			}
		}
	}
}

bool CAP242PModelStructureView::HasDescendantsWithGeometry(_ap242_model* pModel, _ap242_assembly* pAssembly)
{
	if ((pModel == nullptr) || (pAssembly == nullptr))
	{
		ASSERT(FALSE);

		return false;
	}

	return HasDescendantsWithGeometry(pModel, pAssembly->getRelatedProductDefinition());
}

bool CAP242PModelStructureView::HasDescendantsWithGeometry(_ap242_draughting_model* pDraughtingModel)
{
	if (pDraughtingModel == nullptr)
	{
		ASSERT(FALSE);

		return false;
	}

	for (auto pAnnotationPlane : pDraughtingModel->getAnnotationPlanes())
	{
		if (pAnnotationPlane->hasGeometry())
		{
			return true;
		}
	}

	for (auto pDraughtingCallout : pDraughtingModel->getDraughtingCallouts())
	{
		if (pDraughtingCallout->hasGeometry())
		{
			return true;
		}
	}

	return false;
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

void CAP242PModelStructureView::ResetTree(HTREEITEM hItem, bool bEnable)
{
	if (hItem == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	HTREEITEM hChild = m_pTreeCtrl->GetNextItem(hItem, TVGN_CHILD);
	while (hChild != nullptr)
	{
		ResetTree(hChild, bEnable);

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

void CAP242PModelStructureView::Model_EnableChildren(CAP242ItemData* pParent, bool bEnable)
{
	if (pParent == nullptr)
	{
		return;
	}

	for (size_t iChild = 0; iChild < pParent->Children().size(); iChild++)
	{
		auto pChild = pParent->Children()[iChild];
		if ((pChild->GetType() == enumAP242ItemDataType::ProductInstance) ||
			(pChild->GetType() == enumAP242ItemDataType::AnnotationPlane) ||
			(pChild->GetType() == enumAP242ItemDataType::DraughtingCallout))
		{
			pChild->GetInstance<_instance>()->setEnable(bEnable);
		}
		else if (pChild->GetType() == enumAP242ItemDataType::DraughtingModel)
		{
			pChild->GetInstance<_ap242_draughting_model>()->enableInstances(bEnable);
		}

		Model_EnableChildren(pChild, bEnable);
	} // for (size_t iChild = ...
}

void CAP242PModelStructureView::Tree_UpdateChildren(HTREEITEM hItem)
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

void CAP242PModelStructureView::Tree_UpdateParents(HTREEITEM hItem)
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

// ************************************************************************************************
CAP242ItemData::CAP242ItemData(CAP242ItemData* pParent, int64_t* pInstance, enumAP242ItemDataType enItemDataType)
	: m_pParent(pParent)
	, m_pInstance(pInstance)
	, m_enAP242ItemDataType(enItemDataType)
	, m_hItem(nullptr)
	, m_vecChildren()
{
	ASSERT(m_pInstance != nullptr);
	ASSERT(m_enAP242ItemDataType != enumAP242ItemDataType::Unknown);
}

CAP242ItemData::~CAP242ItemData()
{
}
