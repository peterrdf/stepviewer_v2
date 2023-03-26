
#include "stdafx.h"
#include "MainFrm.h"
#include "STEPViewer.h"
#include "DesignTreeView.h"
#include "Resource.h"
#include "Generic.h"
#include "DesignTreeViewConsts.h"
#include "IFCModel.h"

#include <algorithm>

using namespace std;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// ------------------------------------------------------------------------------------------------
class CDesignTreeViewMenuButton : public CMFCToolBarMenuButton
{

	DECLARE_SERIAL(CDesignTreeViewMenuButton)

public:
	CDesignTreeViewMenuButton(HMENU hMenu = nullptr) : CMFCToolBarMenuButton((UINT)-1, hMenu, -1)
	{
	}

	virtual void OnDraw(CDC* pDC, const CRect& rect, CMFCToolBarImages* pImages, BOOL bHorz = TRUE,
		BOOL bCustomizeMode = FALSE, BOOL bHighlight = FALSE, BOOL bDrawBorder = TRUE, BOOL bGrayDisabledButtons = TRUE)
	{
		pImages = CMFCToolBar::GetImages();

		// Patch: wrong background when the app starts
		pImages->SetTransparentColor(::GetSysColor(COLOR_BTNFACE));

		CAfxDrawState ds;
		pImages->PrepareDrawImage(ds);

		CMFCToolBarMenuButton::OnDraw(pDC, rect, pImages, bHorz, bCustomizeMode, bHighlight, bDrawBorder, bGrayDisabledButtons);

		pImages->EndDrawImage(ds);
	}
};

IMPLEMENT_SERIAL(CDesignTreeViewMenuButton, CMFCToolBarMenuButton, 1)

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CDesignTreeView::OnModelChanged()
{
	m_hSelectedItem = nullptr;

	UpdateView();
}

// ------------------------------------------------------------------------------------------------
///*virtual*/ void CDesignTreeView::OnInstanceSelected(CRDFView * pSender)
//{
//	if (pSender == this)
//	{
//		return;
//	}
//
//	if (m_hSelectedItem != nullptr)
//	{
//		m_treeCtrl.SetItemState(m_hSelectedItem, 0, TVIS_BOLD);
//		m_hSelectedItem = nullptr;
//	}
//
//	ASSERT(GetController() != nullptr);
//
//	CInstance * pSelectedInstance = GetController()->GetSelectedInstance();
//	if (pSelectedInstance == nullptr)
//	{
//		/*
//		* Select the Model by default
//		*/
//		HTREEITEM hModel = m_treeCtrl.GetChildItem(nullptr);
//		ASSERT(hModel != nullptr);
//
//		m_treeCtrl.SelectItem(hModel);
//
//		return;
//	}
//
//	/*
//	* Disable the drawing
//	*/
//	m_treeCtrl.SendMessage(WM_SETREDRAW, 0, 0);
//
//	map<int64_t, CInstanceData *>::iterator itInstance2Data = m_mapInstance2Item.find(pSelectedInstance->GetInstance());
//	if (itInstance2Data == m_mapInstance2Item.end())
//	{
//		/*
//		* Load all ancestors
//		*/
//		vector<int64_t> vecAncestors;
//
//		int64_t iInstance = GetInstanceInverseReferencesByIterator(pSelectedInstance->GetInstance(), 0);
//		while (iInstance != 0)
//		{
//			vecAncestors.push_back(iInstance);
//
//			iInstance = GetInstanceInverseReferencesByIterator(iInstance, 0);			
//		}		
//
//		/*
//		* Load the ancestors
//		*/
//		for (int64_t iAncestor = vecAncestors.size() - 1; iAncestor >= 0; iAncestor--)
//		{
//			itInstance2Data = m_mapInstance2Item.find(vecAncestors[iAncestor]);
//			if (itInstance2Data != m_mapInstance2Item.end())
//			{
//				// The item is visible	
//				ASSERT(!itInstance2Data->second->Items().empty());
//				m_treeCtrl.Expand(itInstance2Data->second->Items()[0], TVE_EXPAND);
//			} 
//			else
//			{
//				// The item is not visible - it is in "..." group
//				break;
//			}
//		} // for (size_t iAncestor = ...
//
//		itInstance2Data = m_mapInstance2Item.find(pSelectedInstance->GetInstance());
//	} // if (itInstance2Data == m_mapInstance2Item.end())
//
//	if (itInstance2Data != m_mapInstance2Item.end())
//	{
//		// The item is visible
//		ASSERT(!itInstance2Data->second->Items().empty());
//
//		m_hSelectedItem = itInstance2Data->second->Items()[0];
//
//		m_treeCtrl.SetItemState(m_hSelectedItem, TVIS_BOLD, TVIS_BOLD);
//		m_treeCtrl.EnsureVisible(m_hSelectedItem);
//		m_treeCtrl.SelectItem(m_hSelectedItem);
//	}
//	else
//	{
//		// The item is not visible - it is in "..." group
//		if (m_hSelectedItem != nullptr)
//		{
//			m_treeCtrl.SetItemState(m_hSelectedItem, 0, TVIS_BOLD);
//			m_hSelectedItem = nullptr;
//		}
//
//		MessageBox(L"The selected item is not visible in Instance View.\nPlease, increase 'Visible values count limit' property.", L"Information", MB_ICONINFORMATION | MB_OK);
//	}
//
//	/*
//	* Enable the drawing
//	*/
//	m_treeCtrl.SendMessage(WM_SETREDRAW, 1, 0);
//}

// ------------------------------------------------------------------------------------------------
/*virtual*/ bool CDesignTreeView::IsSelected(HTREEITEM /*hItem*/)
{
	/*ASSERT(0); // todo

	auto pItem = (CItemData*)m_treeCtrl.GetItemData(hItem);
	if ((pItem != nullptr) && (pItem->GetType() == enumItemType::Instance) &&
		(GetController()->GetSelectedInstance() == pItem->GetInstance()))
	{
		return pItem->GetInstance()->IsEnabled();
	}*/

	return false;
}

// ------------------------------------------------------------------------------------------------
//void CDesignTreeView::GetItemPath(HTREEITEM hItem, vector<pair<CInstance*, CRDFProperty*>>& vecPath)
//{
//	if (hItem == nullptr)
//	{
//		return;
//	}
//
//	auto pItem = (CItemData*)m_treeCtrl.GetItemData(hItem);
//	if (pItem != nullptr)
//	{
//		switch (pItem->GetType())
//		{
//			case enumItemType::Instance:
//			{
//				auto pInstanceItem = dynamic_cast<CInstanceData*>(pItem);
//				ASSERT(pInstanceItem != nullptr);
//
//				vecPath.insert(vecPath.begin(), pair<CInstance*, CRDFProperty *>(pInstanceItem->GetInstance(), nullptr));
//			}
//			break;
//
//			case enumItemType::Property:
//			{
//				auto pPropertyItem = dynamic_cast<CRDFPropertyItem*>(pItem);
//				ASSERT(pPropertyItem != nullptr);
//
//				vecPath.insert(vecPath.begin(), pair<CInstance*, CRDFProperty*>(pPropertyItem->GetInstance(), pPropertyItem->getProperty()));
//			}
//			break;
//
//			default:
//			{
//				ASSERT(false); // unexpected
//			}
//			break;
//		}
//	} // if (pItem != nullptr)
//
//	GetItemPath(m_treeCtrl.GetParentItem(hItem), vecPath);
//}

// ------------------------------------------------------------------------------------------------
void CDesignTreeView::GetDescendants(HTREEITEM hItem, vector<HTREEITEM> & vecDescendants)
{
	ASSERT(hItem != nullptr);

	HTREEITEM hChild = m_treeCtrl.GetChildItem(hItem);
	while (hChild != nullptr)
	{
		if (m_treeCtrl.GetItemData(hChild) != NULL)
		{
			vecDescendants.push_back(hChild);

			GetDescendants(hChild, vecDescendants);
		}		

		hChild = m_treeCtrl.GetNextSiblingItem(hChild);
	}
}

// ------------------------------------------------------------------------------------------------
void CDesignTreeView::RemoveInstanceItemData(CInstance * pInstance, HTREEITEM hInstance)
{
	ASSERT(pInstance != nullptr);
	ASSERT(hInstance != nullptr);

	auto itInstance2Data = m_mapInstance2Item.find(pInstance->GetInstance());
	ASSERT(itInstance2Data != m_mapInstance2Item.end());

	vector<HTREEITEM>::const_iterator itInstance = find(itInstance2Data->second->Items().begin(), itInstance2Data->second->Items().end(), hInstance);
	ASSERT(itInstance != itInstance2Data->second->Items().end());

	itInstance2Data->second->Items().erase(itInstance);
}

// ------------------------------------------------------------------------------------------------
//void CDesignTreeView::RemovePropertyItemData(CInstance * pInstance, CRDFProperty * pProperty, HTREEITEM hProperty)
//{
//	ASSERT(pInstance != nullptr);
//	ASSERT(pProperty != nullptr);
//	ASSERT(hProperty != nullptr);
//
//	map<int64_t, map<int64_t, CRDFPropertyItem *> >::iterator itInstance2Properties = m_mapInstance2Properties.find(pInstance->GetInstance());
//	ASSERT(itInstance2Properties != m_mapInstance2Properties.end());
//
//	map<int64_t, CRDFPropertyItem *>::iterator itPropertyItem = itInstance2Properties->second.find(pProperty->GetInstance());
//	ASSERT(itPropertyItem != itInstance2Properties->second.end());
//
//	vector<HTREEITEM>::const_iterator itInstance = find(itPropertyItem->second->Items().begin(), itPropertyItem->second->Items().end(), hProperty);
//	ASSERT(itInstance != itPropertyItem->second->Items().end());
//
//	itPropertyItem->second->Items().erase(itInstance);
//}

// ------------------------------------------------------------------------------------------------
void CDesignTreeView::RemoveItemData(HTREEITEM hItem)
{
	ASSERT(hItem != nullptr);

	auto pItem = (CItemData *)m_treeCtrl.GetItemData(hItem);
	if (pItem != nullptr)
	{
		switch (pItem->GetType())
		{
			case enumItemType::Instance:
			{
				ASSERT(false);

				// TODO
				/*auto pInstanceItem = dynamic_cast<CInstanceData *>(pItem);
				ASSERT(pInstanceItem != nullptr);

				RemoveInstanceItemData(pInstanceItem->GetInstance(), hItem);*/
			}
			break;

			/*case enumItemType::Property:
			{
				auto pPropertyItem = dynamic_cast<CRDFPropertyItem *>(pItem);
				ASSERT(pPropertyItem != nullptr);

				RemovePropertyItemData(pPropertyItem->GetInstance(), pPropertyItem->getProperty(), hItem);
			}
			break;*/

		default:
			{
				ASSERT(false); // unexpected
			}
			break;
		} // switch (pItem->GetType())
	} // if (pItem != nullptr)
}

// ------------------------------------------------------------------------------------------------
void CDesignTreeView::UpdateView()
{
	auto pController = GetController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pModel = pController->GetModel();
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}	

	//m_pSearchDialog->Reset();

	/*
	* Disable the drawing
	*/
	m_bInitInProgress = true;
	m_treeCtrl.SendMessage(WM_SETREDRAW, 0, 0);

	/*
	* Load
	*/
	switch (pModel->GetType())
	{
		case enumModelType::STEP:
		{			
			/*int64_t* iIFCSiteInstances = sdaiGetEntityExtentBN(pModel->GetInstance(), (char*)"IFCSITE");
			ASSERT(iIFCSiteInstances != NULL);
			int64_t iIFCSiteInstancesCount = sdaiGetMemberCount(iIFCSiteInstances);
			ASSERT(iIFCSiteInstancesCount > 0);*/

			ASSERT(FALSE); // TODO

			//LoadSTEPDeisgnTree(dynamic_cast<CSTEPModel*>(pModel));

			/*m_pSTEPTreeView = new CSTEPModelStructureView(&m_treeCtrl);
			m_pSTEPTreeView->SetController(pController);
			m_pSTEPTreeView->Load();*/
		}
		break;

		case enumModelType::IFC:
		{
			LoadIFCDeisgnTree(dynamic_cast<CIFCModel*>(pModel));
		}
		break;

		default:
		{
			ASSERT(FALSE); // Unknown
		}
		break;
	} // switch (pModel ->GetType())

	/*
	* Restore the selected instance
	*/	
	if (pController->GetSelectedInstance() != nullptr)
	{
		//OnInstanceSelected(nullptr);
	}

	/*
	* Restore the selected property
	*/
	//auto prSelectedInstanceProperty = pController->GetSelectedInstanceProperty();
	//if ((prSelectedInstanceProperty.first != nullptr) && (prSelectedInstanceProperty.second != nullptr))
	//{
	//	ASSERT(m_hSelectedItem != nullptr);
	//	m_treeCtrl.SetItemState(m_hSelectedItem, 0, TVIS_BOLD);
	//	m_treeCtrl.Expand(m_hSelectedItem, TVE_EXPAND);

	//	auto itInstance2Properties = m_mapInstance2Properties.find(prSelectedInstanceProperty.first->GetInstance());
	//	ASSERT(itInstance2Properties != m_mapInstance2Properties.end());

	//	auto itPropertyItem = itInstance2Properties->second.find(prSelectedInstanceProperty.second->GetInstance());
	//	ASSERT(itPropertyItem != itInstance2Properties->second.end());
	//	ASSERT(!itPropertyItem->second->Items().empty());

	//	m_hSelectedItem = itPropertyItem->second->Items()[0];

	//	m_treeCtrl.SetItemState(m_hSelectedItem, TVIS_BOLD, TVIS_BOLD);
	//	m_treeCtrl.EnsureVisible(m_hSelectedItem);
	//	m_treeCtrl.SelectItem(m_hSelectedItem);
	//} // if ((prSelectedInstanceProperty.first != nullptr) && ...

	/*
	* Enable the drawing
	*/
	m_bInitInProgress = false;
	m_treeCtrl.SendMessage(WM_SETREDRAW, 1, 0);
}

// ------------------------------------------------------------------------------------------------
void CDesignTreeView::LoadIFCDeisgnTree(CIFCModel* pModel)
{
	delete m_pPropertyProvider;
	m_pPropertyProvider = nullptr;

	m_treeCtrl.DeleteAllItems();

	map<int64_t, CInstanceData *>::iterator itInstance2Data = m_mapInstance2Item.begin();
	for (; itInstance2Data != m_mapInstance2Item.end(); itInstance2Data++)
	{
		delete itInstance2Data->second;
	}

	m_mapInstance2Item.clear();

	/*map<int64_t, map<int64_t, CRDFPropertyItem *> >::iterator itInstance2Properties = m_mapInstance2Properties.begin();
	for (; itInstance2Properties != m_mapInstance2Properties.end(); itInstance2Properties++)
	{
		map<int64_t, CRDFPropertyItem *>::iterator itPropertyItem = itInstance2Properties->second.begin();
		for (; itPropertyItem != itInstance2Properties->second.end(); itPropertyItem++)
		{
			delete itPropertyItem->second;
		}
	}

	m_mapInstance2Properties.clear();*/
	
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	m_pPropertyProvider = new COWLPropertyProvider(pModel->GetInstance());

	// TEST !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	int64_t* iIFCSiteInstances = sdaiGetEntityExtentBN(pModel->GetInstance(), (char*)"IFCSITE");

	int64_t iIFCSiteInstancesCount = sdaiGetMemberCount(iIFCSiteInstances);
	if (iIFCSiteInstancesCount > 0)
	{
		int64_t	iIFCSiteInstance = 0;
		engiGetAggrElement(iIFCSiteInstances, 0, sdaiINSTANCE, &iIFCSiteInstance);

		int64_t owlInstance = 0;
		owlBuildInstance(pModel->GetInstance(), iIFCSiteInstance, &owlInstance);

		// ???
		HTREEITEM hModel = m_treeCtrl.InsertItem(
			pModel->GetModelName(), IMAGE_MODEL, IMAGE_MODEL);

		AddInstance(hModel, owlInstance);

		TRACE(L"\n");
	} // if (iIFCProjectInstancesCount > 0)

	// TEST !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	//auto& mapInstances = pModel->GetInstances();

	//vector<CInstance*> vecInstances;
	//for (auto itInstance : mapInstances)
	//{
	//	auto pInstance = itInstance.second;

	//	if (pInstance->isReferenced())
	//	{
	//		continue;
	//	}

	//	if (pInstance->GetModel() == pModel->GetModel())
	//	{
	//		vecModel.push_back(pInstance);
	//	}
	//	else
	//	{
	//		ASSERT(FALSE);
	//	}
	//} // for (; itRFDInstances != ...

	///*
	//* Model
	//*/
	//sort(vecModel.begin(), vecModel.end(), SORT_RDFINSTANCES());

	//HTREEITEM hModel = m_treeCtrl.InsertItem(_T("Model"), IMAGE_MODEL, IMAGE_MODEL);
	//m_treeCtrl.SetItemState(hModel, TVIS_BOLD, TVIS_BOLD);

	//for (size_t iInstance = 0; iInstance < vecModel.size(); iInstance++)
	//{
	//	AddInstance(hModel, vecModel[iInstance]);
	//}

	//m_treeCtrl.Expand(hModel, TVE_EXPAND);
}

// ------------------------------------------------------------------------------------------------
void CDesignTreeView::AddInstance(HTREEITEM hParent, int64_t iInstance)
{
	/*
	* The instances will be loaded on demand
	*/
	wstring strItem = CInstance::GetName(iInstance);

	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = hParent;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
	tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_INSTANCE;
	tvInsertStruct.item.lParam = NULL;
	tvInsertStruct.item.cChildren = 1;

	HTREEITEM hInstance = m_treeCtrl.InsertItem(&tvInsertStruct);

	auto itInstance2Data = m_mapInstance2Item.find(iInstance);
	if (itInstance2Data == m_mapInstance2Item.end())
	{
		auto pInstanceData = new CInstanceData(iInstance);
		pInstanceData->Items().push_back(hInstance);

		m_mapInstance2Item[iInstance] = pInstanceData;

		m_treeCtrl.SetItemData(hInstance, (DWORD_PTR)pInstanceData);
	}
	else
	{
		itInstance2Data->second->Items().push_back(hInstance);

		m_treeCtrl.SetItemData(hInstance, (DWORD_PTR)itInstance2Data->second);
	}
}

// ------------------------------------------------------------------------------------------------
void CDesignTreeView::AddProperties(HTREEITEM hParent, int64_t iInstance)
{
	if (iInstance == 0)
	{
		ASSERT(FALSE);

		return;
	}

	auto pController = GetController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pModel = pController->GetModel();
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pPropertyCollection = m_pPropertyProvider->GetPropertyCollection(iInstance);
	if (pPropertyCollection != nullptr)
	{
		wchar_t szBuffer[100];

		for (auto pProperty : pPropertyCollection->Properties())
		{
			wstring strProperty = pProperty->GetName();
			strProperty += L" : ";
			strProperty += pProperty->GetTypeName();

			HTREEITEM hProperty = m_treeCtrl.InsertItem(strProperty.c_str(), IMAGE_PROPERTY, IMAGE_PROPERTY, hParent);

			/*
			* rdfs:range
			*/
			vector<int64_t> vecRestrictionClasses;

			wstring strRange = L"rdfs:range : ";
			strRange += pProperty->GetRange(vecRestrictionClasses);

			HTREEITEM hRange = m_treeCtrl.InsertItem(strRange.c_str(), IMAGE_PROPERTY, IMAGE_PROPERTY, hProperty);

			for (auto iRestrictionClass : vecRestrictionClasses)
			{
				wchar_t* szClassName = nullptr;
				GetNameOfClassW(iRestrictionClass, &szClassName);

				m_treeCtrl.InsertItem(szClassName, IMAGE_VALUE, IMAGE_VALUE, hRange);
			}
			
			/*
			* owl:cardinality
			*/
			wstring strCardinality = L"owl:cardinality : ";
			strCardinality += pProperty->GetCardinality(iInstance);

			m_treeCtrl.InsertItem(strCardinality.c_str(), IMAGE_VALUE, IMAGE_VALUE, hProperty);

			/*
			* value(s)
			*/
			switch (pProperty->GetType())
			{
				case OBJECTTYPEPROPERTY_TYPE:
				{
					int64_t* piInstances = nullptr;
					int64_t iValuesCount = 0;
					GetObjectProperty(iInstance, pProperty->GetInstance(), &piInstances, &iValuesCount);

					for (int64_t iValue = 0; iValue < iValuesCount; iValue++)
					{
						if (piInstances[iValue] != 0)
						{
							AddInstance(hProperty, piInstances[iValue]);
						}
						else
						{
							m_treeCtrl.InsertItem(EMPTY_INSTANCE, IMAGE_INSTANCE, IMAGE_INSTANCE, hProperty);
						}
					} // for (int64_t iInstance = ...
				}
				break;

				case DATATYPEPROPERTY_TYPE_BOOLEAN:
				{
					bool* pbValue = nullptr;
					int64_t iValuesCount = 0;
					GetDatatypeProperty(iInstance, pProperty->GetInstance(), (void**)&pbValue, &iValuesCount);

					for (int64_t iValue = 0; iValue < iValuesCount; iValue++)
					{
						swprintf(szBuffer, 100, L"value = %s", pbValue[iValue] ? L"TRUE" : L"FALSE");
						m_treeCtrl.InsertItem(szBuffer, IMAGE_VALUE, IMAGE_VALUE, hProperty);
					}
				}
				break;

				case DATATYPEPROPERTY_TYPE_CHAR:
				{
					char** szValue = nullptr;
					int64_t iValuesCount = 0;
					GetDatatypeProperty(iInstance, pProperty->GetInstance(), (void**)&szValue, &iValuesCount);

					for (int64_t iValue = 0; iValue < iValuesCount; iValue++)
					{
						wstring strValue = CA2W(szValue[iValue]);
						swprintf(szBuffer, 100, L"value = '%s'", strValue.c_str());

						m_treeCtrl.InsertItem(szBuffer, IMAGE_VALUE, IMAGE_VALUE, hProperty);
					} // for (int64_t iValue = ...
				}
				break;				

				case DATATYPEPROPERTY_TYPE_INTEGER:
				{
					int64_t* piValue = nullptr;
					int64_t iValuesCount = 0;
					GetDatatypeProperty(iInstance, pProperty->GetInstance(), (void**)&piValue, &iValuesCount);

					for (int64_t iValue = 0; iValue < iValuesCount; iValue++)
					{
						swprintf(szBuffer, 100, L"value = %lld", piValue[iValue]);

						m_treeCtrl.InsertItem(szBuffer, IMAGE_VALUE, IMAGE_VALUE, hProperty);
					}
				}
				break;

				case DATATYPEPROPERTY_TYPE_DOUBLE:
				{
					double* pdValue = nullptr;
					int64_t iValuesCount = 0;
					GetDatatypeProperty(iInstance, pProperty->GetInstance(), (void**)&pdValue, &iValuesCount);

					for (int64_t iValue = 0; iValue < iValuesCount; iValue++)
					{
						swprintf(szBuffer, 100, L"value = %.6f", pdValue[iValue]);

						m_treeCtrl.InsertItem(szBuffer, IMAGE_VALUE, IMAGE_VALUE, hProperty);
					}
				}
				break;

				case DATATYPEPROPERTY_TYPE_BYTE:
				{
					BYTE* piValue = nullptr;
					int64_t iValuesCount = 0;
					GetDatatypeProperty(iInstance, pProperty->GetInstance(), (void**)&piValue, &iValuesCount);

					for (int64_t iValue = 0; iValue < iValuesCount; iValue++)
					{
						swprintf(szBuffer, 100, L"value = %d", piValue[iValue]);

						m_treeCtrl.InsertItem(szBuffer, IMAGE_VALUE, IMAGE_VALUE, hProperty);
					}
				}
				break;

				default:
				{
					ASSERT(false); // unknown property
				}
				break;
			} // switch (pProperty->GetType())
		} // for (auto pProperty ...
	} // if (pPropertyCollection != nullptr)
	else
	{
		ASSERT(FALSE); // Internal error!
	}	
}

// ------------------------------------------------------------------------------------------------
//void CDesignTreeView::UpdateRootItemsUnreferencedItemsView(int64_t iModel, HTREEITEM hModel)
//{
	//ASSERT(iModel != 0);
	//ASSERT(hModel != nullptr);

	//CRDFModel * pModel = GetController()->GetModel();
	//ASSERT(pModel != nullptr);

	//auto& mapInstances = pModel->GetInstances();

	//vector<CInstance *> vecInstances;
	//vector<HTREEITEM> vecObsoleteItems;

	//HTREEITEM hItem = m_treeCtrl.GetChildItem(hModel);
	//while (hItem != nullptr)
	//{
	//	CItemData * pItem = (CItemData *)m_treeCtrl.GetItemData(hItem);
	//	ASSERT(pItem != nullptr);
	//	ASSERT(pItem->GetType() == enumItemType::Instance);

	//	CInstanceData * pInstanceItem = dynamic_cast<CInstanceData *>(pItem);
	//	ASSERT(pInstanceItem != nullptr);

	//	if (pInstanceItem->GetInstance()->isReferenced())
	//	{
	//		vecObsoleteItems.push_back(hItem);
	//	}
	//	else
	//	{
	//		vecInstances.push_back(pInstanceItem->GetInstance());
	//	}

	//	hItem = m_treeCtrl.GetNextSiblingItem(hItem);
	//} // while (hChild != nullptr)

	///*
	//* Delete the items with references
	//*/
	//for (int64_t iItem = 0; iItem < (int64_t)vecObsoleteItems.size(); iItem++)
	//{
	//	RemoveItemData(vecObsoleteItems[iItem]);

	//	vector<HTREEITEM> vecDescendants;
	//	GetDescendants(vecObsoleteItems[iItem], vecDescendants);

	//	for (size_t iDescendant = 0; iDescendant < vecDescendants.size(); iDescendant++)
	//	{
	//		RemoveItemData(vecDescendants[iDescendant]);
	//	}

	//	m_treeCtrl.DeleteItem(vecObsoleteItems[iItem]);
	//} // for (int64_t iItem = ...

	///*
	//* Add the missing items without references
	//*/
	//map<int64_t, CInstance *>::const_iterator itRFDInstances = mapInstances.begin();
	//for (; itRFDInstances != mapInstances.end(); itRFDInstances++)
	//{
	//	CInstance * pInstance = itRFDInstances->second;

	//	if (pInstance->isReferenced())
	//	{
	//		continue;
	//	}

	//	if (pInstance->GetModel() != iModel)
	//	{
	//		continue;
	//	}

	//	if (find(vecInstances.begin(), vecInstances.end(), pInstance) != vecInstances.end())
	//	{
	//		continue;
	//	}

	//	AddInstance(hModel, pInstance);
	//} // for (; itRFDInstances != ...
//}

/////////////////////////////////////////////////////////////////////////////
// CDesignTreeView

CDesignTreeView::CDesignTreeView()
	: m_pPropertyProvider(nullptr)
	, m_mapInstance2Item()
	, m_hSelectedItem(nullptr)
	, m_bInitInProgress(false)
	//, m_pSearchDialog(nullptr)
{
}

CDesignTreeView::~CDesignTreeView()
{	
	delete m_pPropertyProvider;

	for (auto itInstance2Data : m_mapInstance2Item)
	{
		delete itInstance2Data.second;
	}
	m_mapInstance2Item.clear();

	/*map<int64_t, map<int64_t, CRDFPropertyItem *> >::iterator itInstance2Properties = m_mapInstance2Properties.begin();
	for (; itInstance2Properties != m_mapInstance2Properties.end(); itInstance2Properties++)
	{
		map<int64_t, CRDFPropertyItem *>::iterator itPropertyItem = itInstance2Properties->second.begin();
		for (; itPropertyItem != itInstance2Properties->second.end(); itPropertyItem++)
		{
			delete itPropertyItem->second;
		}
	}

	m_mapInstance2Properties.clear();	*/
}

BEGIN_MESSAGE_MAP(CDesignTreeView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_INSTANCE_VIEW, OnSelectedItemChanged)
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_TREE_INSTANCE_VIEW, OnItemExpanding)
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorkspaceBar message handlers

int CDesignTreeView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	ASSERT(GetController() != nullptr);
	GetController()->RegisterView(this);

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create view:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS;

	if (!m_treeCtrl.Create(dwViewStyle, rectDummy, this, IDC_TREE_INSTANCE_VIEW))
	{
		TRACE0("Failed to create file view\n");
		return -1;      // fail to create
	}

	// State provider
	m_treeCtrl.SetItemStateProvider(this);

	// Load view images:
	m_images.Create(IDB_CLASS_VIEW, 16, 0, RGB(255, 0, 0));
	m_treeCtrl.SetImageList(&m_images, TVSIL_NORMAL);

	m_toolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_SORT_INSTANCES);
	m_toolBar.LoadToolBar(IDR_SORT_INSTANCES, 0, 0, TRUE /* Is locked */);

	OnChangeVisualStyle();

	m_toolBar.SetPaneStyle(m_toolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);

	m_toolBar.SetPaneStyle(m_toolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	m_toolBar.SetOwner(this);

	// All commands will be routed via this control , not via the parent frame:
	m_toolBar.SetRouteCommandsViaFrame(FALSE);

	/*CMenu menuSort;
	menuSort.LoadMenu(IDR_POPUP_SORT_INSTANCES);

	m_toolBar.ReplaceButton(ID_SORT_MENU, CDesignTreeViewMenuButton(menuSort.GetSubMenu(0)->GetSafeHmenu()));

	CDesignTreeViewMenuButton* pButton = DYNAMIC_DOWNCAST(CDesignTreeViewMenuButton, m_toolBar.GetButton(0));

	if (pButton != nullptr)
	{
		pButton->m_bText = FALSE;
		pButton->m_bImage = TRUE;
		pButton->SetImage(GetCmdMgr()->GetCmdImage(m_nCurrSort));
		pButton->SetMessageWnd(this);
	}

	AdjustLayout();*/

	//  Search
	//m_pSearchDialog = new CSearchInstancesDialog(&m_treeCtrl);
	//m_pSearchDialog->Create(IDD_DIALOG_SEARCH_INSTANCES, this);

	return 0;
}

void CDesignTreeView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CDesignTreeView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CTreeCtrl* pWndTree = (CTreeCtrl*) &m_treeCtrl;
	ASSERT_VALID(pWndTree);

	if (pWnd != pWndTree)
	{
		CDockablePane::OnContextMenu(pWnd, point);
		return;
	}

	if (point == CPoint(-1, -1))
	{
		return;
	}

	// Select clicked item:
	CPoint ptTree = point;
	pWndTree->ScreenToClient(&ptTree);

	UINT flags = 0;
	HTREEITEM hTreeItem = pWndTree->HitTest(ptTree, &flags);
	if (hTreeItem == nullptr)
	{
		return;
	}

	pWndTree->SelectItem(hTreeItem);

	//pWndTree->SetFocus();

	//auto pItem = (CItemData *)m_treeCtrl.GetItemData(hTreeItem);
	//if (pItem == nullptr)
	//{
	//	return;
	//}

	//if (pItem->GetType() != enumItemType::Instance)
	//{
	//	return;
	//}

	//if (GetController() == nullptr)
	//{
	//	ASSERT(FALSE);

	//	return;
	//}

	//auto pModel = GetController()->GetModel();
	//if (pModel == nullptr)
	//{
	//	ASSERT(FALSE);

	//	return;
	//}

	//auto& mapInstances = pModel->GetInstances();

	//auto pInstanceItem = dynamic_cast<CInstanceData*>(pItem);
	//auto pInstance = pInstanceItem->GetInstance();

	///*
	//* Instances with a geometry
	//*/
	//if (pInstance->hasGeometry())
	//{
	//	CMenu menu;
	//	VERIFY(menu.LoadMenuW(IDR_POPUP_INSTANCES));

	//	auto pPopup = menu.GetSubMenu(0);

	//	// Enable
	//	if (pInstance->IsEnabled())
	//	{
	//		pPopup->CheckMenuItem(ID_INSTANCES_ENABLE, MF_BYCOMMAND | MF_CHECKED);
	//	}

	//	// Zoom to
	//	if (!pInstance->IsEnabled())
	//	{
	//		pPopup->EnableMenuItem(ID_INSTANCES_ZOOM_TO, MF_BYCOMMAND | MF_DISABLED);
	//	}

	//	UINT uiCommand = pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, point.x, point.y, pWndTree);
	//	if (uiCommand == 0)
	//	{
	//		return;
	//	}		

	//	switch (uiCommand)
	//	{
	//		case ID_INSTANCES_ZOOM_TO:
	//		{
	//			GetController()->ZoomToInstance(pInstance->GetInstance());
	//		}
	//		break;

	//		case ID_VIEW_ZOOM_OUT:
	//		{
	//			GetController()->ZoomOut();
	//		}
	//		break;

	//		case ID_INSTANCES_BASE_INFORMATION:
	//		{
	//			GetController()->ShowBaseInformation(pInstance);
	//		}
	//		break;

	//		case ID_INSTANCES_META_INFORMATION:
	//		{
	//			GetController()->ShowMetaInformation(pInstance);
	//		}
	//		break;

	//		case ID_INSTANCES_SAVE:
	//		{
	//			GetController()->Save(pInstance);
	//		}
	//		break;

	//		case ID_INSTANCES_DISABLE_ALL_BUT_THIS:
	//		{
	//			auto itRFDInstances = mapInstances.begin();
	//			for (; itRFDInstances != mapInstances.end(); itRFDInstances++)
	//			{
	//				if (pInstance->GetModel() != itRFDInstances->second->GetModel())
	//				{
	//					continue;
	//				}

	//				if (itRFDInstances->second == pInstance)
	//				{
	//					itRFDInstances->second->setEnable(true);

	//					continue;
	//				}

	//				itRFDInstances->second->setEnable(false);
	//			}

	//			GetController()->OnInstancesEnabledStateChanged();
	//		}
	//		break;

	//		case ID_INSTANCES_ENABLE_ALL:
	//		{
	//			auto itRFDInstances = mapInstances.begin();
	//			for (; itRFDInstances != mapInstances.end(); itRFDInstances++)
	//			{
	//				itRFDInstances->second->setEnable(true);
	//			}

	//			GetController()->OnInstancesEnabledStateChanged();
	//		}
	//		break;
	//		 
	//		case ID_INSTANCES_ENABLE_ALL_UNREFERENCED:
	//		{
	//			auto itRFDInstances = mapInstances.begin();
	//			for (; itRFDInstances != mapInstances.end(); itRFDInstances++)
	//			{
	//				itRFDInstances->second->setEnable(!pInstance->isReferenced());
	//			}

	//			GetController()->OnInstancesEnabledStateChanged();
	//		}
	//		break;

	//		case ID_INSTANCES_ENABLE_ALL_REFERENCED:
	//		{
	//			auto itRFDInstances = mapInstances.begin();
	//			for (; itRFDInstances != mapInstances.end(); itRFDInstances++)
	//			{
	//				itRFDInstances->second->setEnable(pInstance->isReferenced());
	//			}

	//			GetController()->OnInstancesEnabledStateChanged();
	//		}
	//		break;

	//		case ID_INSTANCES_ENABLE:
	//		{
	//			pInstance->setEnable(!pInstance->IsEnabled());

	//			GetController()->OnInstancesEnabledStateChanged();
	//		}
	//		break;

	//		case ID_INSTANCES_REMOVE:
	//		{
	//			if (pInstance->isReferenced())
	//			{
	//				MessageBox(L"The instance is referenced and can't be removed.", L"Error", MB_ICONERROR | MB_OK);

	//				return;
	//			}

	//			GetController()->DeleteInstance(this, pInstance);
	//		}
	//		break;

	//		case ID_INSTANCES_REMOVE_TREE:
	//		{
	//			if (pInstance->isReferenced())
	//			{
	//				MessageBox(L"The instance is referenced and can't be removed.", L"Error", MB_ICONERROR | MB_OK);

	//				return;
	//			}

	//			GetController()->DeleteInstanceTree(this, pInstance);
	//		}
	//		break;

	//		case ID_INSTANCES_ADD_MEASUREMENTS:
	//		{
	//			GetController()->AddMeasurements(this, pInstance);
	//		}
	//		break;

	//		case ID_INSTANCES_SEARCH:
	//		{
	//			if (!m_pSearchDialog->IsWindowVisible())
	//			{
	//				m_pSearchDialog->ShowWindow(SW_SHOW);
	//			}
	//			else
	//			{
	//				m_pSearchDialog->ShowWindow(SW_HIDE);
	//			}
	//		}
	//		break;

	//		default:
	//		{
	//			ASSERT(false);
	//		}
	//		break;
	//	} // switch (uiCommand)	

	//	return;
	//} // if (pInstance->hasGeometry())	

	///*
	//* Instances without a geometry
	//*/
	//CMenu menu;
	//VERIFY(menu.LoadMenuW(IDR_POPUP_INSTANCES_NO_GEOMETRY));

	//auto pPopup = menu.GetSubMenu(0);

	//UINT uiCommand = pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, point.x, point.y, pWndTree);
	//if (uiCommand == 0)
	//{
	//	return;
	//}

	//switch (uiCommand)
	//{
	//	case ID_INSTANCES_ENABLE_ALL:
	//	{
	//		auto itRFDInstances = mapInstances.begin();
	//		for (; itRFDInstances != mapInstances.end(); itRFDInstances++)
	//		{
	//			itRFDInstances->second->setEnable(true);
	//		}

	//		GetController()->OnInstancesEnabledStateChanged();
	//	}
	//	break;

	//	case ID_INSTANCES_ENABLE_ALL_UNREFERENCED:
	//	{
	//		auto itRFDInstances = mapInstances.begin();
	//		for (; itRFDInstances != mapInstances.end(); itRFDInstances++)
	//		{
	//			itRFDInstances->second->setEnable(!pInstance->isReferenced());
	//		}

	//		GetController()->OnInstancesEnabledStateChanged();
	//	}
	//	break;

	//	case ID_INSTANCES_ENABLE_ALL_REFERENCED:
	//	{
	//		auto itRFDInstances = mapInstances.begin();
	//		for (; itRFDInstances != mapInstances.end(); itRFDInstances++)
	//		{
	//			itRFDInstances->second->setEnable(pInstance->isReferenced());
	//		}

	//		GetController()->OnInstancesEnabledStateChanged();
	//	}
	//	break;

	//	case ID_INSTANCES_REMOVE:
	//	{
	//		if (pInstance->isReferenced())
	//		{
	//			MessageBox(L"The instance is referenced and can't be removed.", L"Error", MB_ICONERROR | MB_OK);

	//			return;
	//		}

	//		GetController()->DeleteInstance(this, pInstance);
	//	}
	//	break;

	//	case ID_INSTANCES_REMOVE_TREE:
	//	{
	//		if (pInstance->isReferenced())
	//		{
	//			MessageBox(L"The instance is referenced and can't be removed.", L"Error", MB_ICONERROR | MB_OK);

	//			return;
	//		}

	//		MessageBox(L"Not an option yet I.", L"Error", MB_ICONERROR | MB_OK);
	//		GetController()->DeleteInstance(this, pInstance);
	//	}
	//	break;

	//	case ID_INSTANCES_SEARCH:
	//	{
	//		if (!m_pSearchDialog->IsWindowVisible())
	//		{
	//			m_pSearchDialog->ShowWindow(SW_SHOW);
	//		}
	//		else
	//		{
	//			m_pSearchDialog->ShowWindow(SW_HIDE);
	//		}
	//	}
	//	break;

	//	default:
	//	{
	//		ASSERT(false);
	//	}
	//	break;
	//} // switch (uiCommand)
}

void CDesignTreeView::AdjustLayout()
{
	if (GetSafeHwnd() == nullptr)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_toolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_toolBar.SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_treeCtrl.SetWindowPos(nullptr, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CDesignTreeView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rectTree;
	m_treeCtrl.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CDesignTreeView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_treeCtrl.SetFocus();
}

void CDesignTreeView::OnChangeVisualStyle()
{
	m_images.DeleteImageList();

	UINT uiBmpId = theApp.m_bHiColorIcons ? IDB_CLASS_VIEW_24 : IDB_CLASS_VIEW;

	CBitmap bmp;
	if (!bmp.LoadBitmap(uiBmpId))
	{
		TRACE(_T("Can't load bitmap: %x\n"), uiBmpId);
		ASSERT(FALSE);
		return;
	}

	BITMAP bmpObj;
	bmp.GetBitmap(&bmpObj);

	UINT nFlags = ILC_MASK;

	nFlags |= (theApp.m_bHiColorIcons) ? ILC_COLOR24 : ILC_COLOR4;

	m_images.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
	m_images.Add(&bmp, RGB(255, 0, 0));

	m_treeCtrl.SetImageList(&m_images, TVSIL_NORMAL);

	m_toolBar.CleanUpLockedImages();
	m_toolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_SORT_INSTANCES_24 : IDR_SORT_INSTANCES, 0, 0, TRUE /* Locked */);
}

void CDesignTreeView::OnDestroy()
{
	ASSERT(GetController() != nullptr);
	GetController()->UnRegisterView(this);

	__super::OnDestroy();

	//delete m_pSearchDialog;
}

void CDesignTreeView::OnSelectedItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;

	if (m_bInitInProgress)
	{
		return;
	}

	NM_TREEVIEW * pNMTreeView = (NM_TREEVIEW *)pNMHDR;

	if (m_treeCtrl.GetParentItem(m_hSelectedItem) != nullptr)
	{
		// keep the roots always bold
		m_treeCtrl.SetItemState(m_hSelectedItem, 0, TVIS_BOLD);
	}

	m_treeCtrl.SetItemState(pNMTreeView->itemNew.hItem, TVIS_BOLD, TVIS_BOLD);
	m_hSelectedItem = pNMTreeView->itemNew.hItem;

	ASSERT(GetController() != nullptr);

	auto pItem = (CItemData*)m_treeCtrl.GetItemData(pNMTreeView->itemNew.hItem);
	if (pItem == nullptr)
	{
		GetController()->SelectInstance(this, nullptr);

		return;
	}	

	if (pItem->GetType() == enumItemType::Instance)
	{
		// TODO
		//auto pInstanceItem = dynamic_cast<CInstanceData *>(pItem);
		
		//GetController()->SelectInstance(this, pInstanceItem->GetInstance());		

		return;
	} // if (pItem->GetType() == enumItemType::Instance)	

	//if (pItem->GetType() == enumItemType::Property)
	//{
	//	CRDFPropertyItem * pPropertyItem = dynamic_cast<CRDFPropertyItem *>(pItem);
	//	
	//	GetController()->SelectInstanceProperty(pPropertyItem->GetInstance(), pPropertyItem->getProperty());		

	//	return;
	//} // if (pItem->GetType() == enumItemType::Property)

	GetController()->SelectInstance(this, nullptr);
}

void CDesignTreeView::OnItemExpanding(NMHDR * pNMHDR, LRESULT * pResult)
{
	*pResult = 0;

	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	
	if (m_treeCtrl.GetChildItem(pNMTreeView->itemNew.hItem) != nullptr)
	{
		// it is loaded already
		return;
	}

	auto pItem = (CItemData*)m_treeCtrl.GetItemData(pNMTreeView->itemNew.hItem);
	ASSERT(pItem != nullptr);
	ASSERT(pItem->GetType() == enumItemType::Instance);

	auto pInstanceData = dynamic_cast<CInstanceData *>(pItem);	
	AddProperties(pNMTreeView->itemNew.hItem, pInstanceData->GetInstance());
}

void CDesignTreeView::OnShowWindow(BOOL bShow, UINT nStatus)
{
	__super::OnShowWindow(bShow, nStatus);

	if (!bShow)
	{
		//m_pSearchDialog->ShowWindow(SW_HIDE);
	}
}
