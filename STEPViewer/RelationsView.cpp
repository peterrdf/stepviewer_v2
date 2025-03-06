
#include "stdafx.h"

#include "_ifc_model.h"
#include "_ptr.h"

#include "mainfrm.h"
#include "RelationsView.h"
#include "Resource.h"
#include "STEPViewer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define IMAGE_MODEL 			2
#define IMAGE_INSTANCE			0
#define IMAGE_ENTITY			1
#define IMAGE_ATTRIBUTES		2
#define IMAGE_INVERSE_ATTRIBUTE	3
#define IMAGE_ATTRIBUTE			5

#define ITEM_SUB_TYPES			L"Sub-types"
#define ITEM_ATTRIBUTES			L"Attributes"
#define ITEM_PENDING_LOAD		L"***..........***"

#define MAX_LABEL_SIZE			50
#define LOAD_ATTRIBUTES_LIMIT	50

///////////////////////////////////////////////////////////////////////////////////////////////////
// CRelationsView

/*virtual*/ void CRelationsView::onModelLoaded() /*override*/
{
	LoadProperties(0, vector<SdaiInstance>());
}

/*virtual*/ void CRelationsView::onInstanceSelected(_view* pSender) /*override*/
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

	ResetView();

	if (getController()->getSelectedInstances().empty())
	{
		return;
	}

	map<_ap_model*, vector<SdaiInstance>> mapInstances;
	for (auto pInstance : getController()->getSelectedInstances())
	{
		_ptr<_ap_instance> apInstance(pInstance);

		auto pModel = GetModelByInstance(sdaiGetInstanceModel(apInstance->getSdaiInstance()));
		if (pModel == nullptr)
		{
			ASSERT(FALSE);

			continue;
		}

		auto itInstances = mapInstances.find(pModel);
		if (itInstances != mapInstances.end())
		{
			itInstances->second.push_back(apInstance->getSdaiInstance());
		}
		else
		{
			mapInstances[pModel] = vector<SdaiInstance>{ apInstance->getSdaiInstance() };
		}
	}
	
	for (auto itInstances : mapInstances)
	{
		LoadInstances(itInstances.first, itInstances.second);
	}	
}

/*virtual*/ void CRelationsView::onViewRelations(_view* pSender, SdaiInstance sdaiInstance) /*override*/
{
	if (pSender == this)
	{
		return;
	}

	if (sdaiInstance == 0)
	{
		ASSERT(FALSE);

		return;
	}

	vector<SdaiInstance> vecInstances;
	vecInstances.push_back(sdaiInstance);

	LoadProperties(
		_ap_geometry::getSdaiEntity(sdaiInstance),
		vecInstances);

	ShowPane(TRUE, TRUE, TRUE);
}

/*virtual*/ void CRelationsView::onViewRelations(_view* pSender, _entity* pEntity) /*override*/
{
	if (pSender == this)
	{
		return;
	}
	
	LoadProperties(
		pEntity->getEntity(),
		pEntity->getInstances());
	
	ShowPane(TRUE, TRUE, TRUE);
}

/*virtual*/ CTreeCtrlEx* CRelationsView::GetTreeView() /*override*/
{
	return &m_treeCtrl;
}

/*virtual*/ vector<CString> CRelationsView::GetSearchFilters() /*override*/
{
	return vector<CString>
	{
		_T("(All)"),
		_T("Express ID"),
	};
}

/*virtual*/ CString CRelationsView::GetSearchFilterType(const CString& strFilter) /*override*/
{
	if (strFilter == _T("Express ID"))
	{
		return _T("Select");
	}

	return __super::GetSearchFilterType(strFilter);
}

/*virtual*/ void CRelationsView::LoadChildrenIfNeeded(HTREEITEM hItem) /*override*/
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

/*virtual*/ BOOL CRelationsView::ProcessSearch(int iFilter, const CString& strSearchText) /*override*/
{
	auto pController = getAPController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return FALSE;
	}

	// ExpressID
	if (iFilter == (int)enumSearchFilter::ExpressID)
	{
		int64_t iExpressID = _wtoi64((LPCTSTR)strSearchText);

		for (auto pModel : pController->getModels())
		{
			if (!pModel->getEnable())
			{
				continue;
			}

			auto pAPModel = dynamic_cast<_ap_model*>(pModel);
			if (pAPModel == nullptr)
			{
				continue;
			}

			SdaiInstance sdaiInstance = internalGetInstanceFromP21Line(pAPModel->getSdaiModel(), iExpressID);
			if (sdaiInstance != 0)
			{
				pController->onViewRelations(
					nullptr, /*Attributes View will be updated also*/
					sdaiInstance);

				return TRUE;
			}
		}

		::MessageBox(::AfxGetMainWnd()->GetSafeHwnd(), L"Invalid Express ID.", L"Search", MB_ICONERROR | MB_OK);

		return FALSE;
	}

	return FALSE;
}

/*virtual*/ BOOL CRelationsView::ContainsText(int iFilter, HTREEITEM hItem, const CString& strText) /*override*/
{
	UNREFERENCED_PARAMETER(iFilter);

	if (hItem == NULL)
	{
		ASSERT(FALSE);

		return FALSE;
	}	

	ASSERT(hItem != nullptr);

	CString strItemText = GetTreeView()->GetItemText(hItem);
	strItemText.MakeLower();

	CString strTextLower = strText;
	strTextLower.MakeLower();

	// All
	return strItemText.Find(strTextLower, 0) != -1;
}

_ap_model* CRelationsView::GetModelByInstance(SdaiModel sdaiModel)
{
	for (auto pModel : getController()->getModels())
	{
		if (!pModel->getEnable())
		{
			continue;
		}

		_ptr<_ap_model> apModel(pModel);
		if (apModel->getSdaiModel() == sdaiModel)
		{
			return apModel;
		}
	}

	return nullptr;
}

void CRelationsView::LoadInstances(_ap_model* pModel, const vector<SdaiInstance>& vecInstances)
{

	// ******************************************************************************************** //
	// Model
	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = nullptr;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = (LPWSTR)pModel->getPath();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_MODEL;
	tvInsertStruct.item.lParam = NULL;
	HTREEITEM hModel = m_treeCtrl.InsertItem(&tvInsertStruct);
	// ******************************************************************************************** //	

	// ******************************************************************************************** //
	// Instances
	for (auto sdaiInstance : vecInstances)
	{
		LoadInstance(
			_ap_geometry::getSdaiEntity(sdaiInstance),
			sdaiInstance,
			hModel);
	}
	// ******************************************************************************************** //

	m_treeCtrl.Expand(hModel, TVE_EXPAND);
}

void CRelationsView::LoadProperties(SdaiEntity sdaiEntity, const vector<SdaiInstance>& vecSdaiInstances)
{
	ResetView();

	if ((sdaiEntity == 0) || vecSdaiInstances.empty())
	{
		return;
	}

	auto pModel = GetModelByInstance(engiGetEntityModel(sdaiEntity));
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	// ******************************************************************************************** //
	// Model
	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = nullptr;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = (LPWSTR)pModel->getPath();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_MODEL;
	tvInsertStruct.item.lParam = NULL;
	HTREEITEM hModel = m_treeCtrl.InsertItem(&tvInsertStruct);
	// ******************************************************************************************** //	

	// ******************************************************************************************** //
	// Instances
	for (auto sdaiInstance : vecSdaiInstances)
	{
		LoadInstance(sdaiEntity, sdaiInstance, hModel);
	}
	// ******************************************************************************************** //

	m_treeCtrl.Expand(hModel, TVE_EXPAND);
}

void CRelationsView::LoadInstance(SdaiEntity sdaiEntity, SdaiInstance sdaiInstance, HTREEITEM hParent)
{	
	ASSERT(sdaiEntity != 0);
	ASSERT(sdaiInstance != 0);

	/*
	* Data
	*/
	auto pInstanceData = new CInstanceData(sdaiInstance, sdaiEntity);
	m_vecItemDataCache.push_back(pInstanceData);

	/*
	* Instance
	*/
	wstring strItem = _ap_instance::getName(sdaiInstance);

	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = hParent;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_INSTANCE;
	tvInsertStruct.item.lParam = (LPARAM)pInstanceData;
	HTREEITEM hInstance = m_treeCtrl.InsertItem(&tvInsertStruct);	

	CAttributeSet attributeSet(sdaiInstance, sdaiEntity);
	GetInstanceAttributes(sdaiEntity, sdaiInstance, hInstance, &attributeSet);

	// Load first page
	size_t iAttributeStart = 0;
	size_t iAttributeEnd = iAttributeStart + LOAD_ATTRIBUTES_LIMIT;
	if (iAttributeEnd >= attributeSet.Attributes().size())
	{
		iAttributeEnd = attributeSet.Attributes().size();
	}

	for (size_t iAttribute = iAttributeStart; (iAttribute < iAttributeEnd); iAttribute++)
	{
		const char* szAttributeName = nullptr;
		engiGetEntityArgumentName(attributeSet.GetEntity(), 
			attributeSet.Attributes()[iAttribute].second, 
			sdaiSTRING, 
			&szAttributeName);

		LoadInstanceAttribute(
			sdaiEntity,
			sdaiInstance,
			attributeSet.Attributes()[iAttribute].first,
			szAttributeName,
			hInstance,
			TVI_LAST);

		iAttributeStart++;
	}

	// Load on demand
	while (iAttributeStart < attributeSet.Attributes().size())
	{
		iAttributeEnd = iAttributeStart + LOAD_ATTRIBUTES_LIMIT;
		if (iAttributeEnd >= attributeSet.Attributes().size())
		{
			iAttributeEnd = attributeSet.Attributes().size();
		}

		auto pAttributeSet = new CAttributeSet(sdaiInstance, sdaiEntity);
		m_vecItemDataCache.push_back(pAttributeSet);

		for (size_t iAttribute = iAttributeStart; (iAttribute < iAttributeEnd); iAttribute++)
		{
			pAttributeSet->Attributes().push_back(attributeSet.Attributes()[iAttribute]);
		}

		CString strPage;
		strPage.Format(L"[%lld - %lld]", (int64_t)iAttributeStart + 1, (int64_t)iAttributeEnd);

		// Pending load
		tvInsertStruct.hParent = hInstance;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
		tvInsertStruct.item.pszText = strPage.GetBuffer();
		tvInsertStruct.item.cChildren = 1;
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_ATTRIBUTES;
		tvInsertStruct.item.lParam = NULL;
		HTREEITEM hAttributes = m_treeCtrl.InsertItem(&tvInsertStruct);

		// Add a fake item - load on demand
		tvInsertStruct.hParent = hAttributes;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = ITEM_PENDING_LOAD;
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_INSTANCE;
		tvInsertStruct.item.lParam = (LPARAM)pAttributeSet;
		m_treeCtrl.InsertItem(&tvInsertStruct);

		iAttributeStart = iAttributeEnd;
	} // while (iAttributeStart < ...
}

SdaiInteger CRelationsView::GetInstanceAttributes(SdaiEntity sdaiEntity, SdaiInstance sdaiInstance, HTREEITEM hParent, CAttributeSet* pAttributeSet)
{
	if (sdaiEntity == 0)
	{
		return 0;
	}

	ASSERT(sdaiInstance != 0);

	SdaiInteger iAttrubutesCount = GetInstanceAttributes(engiGetEntityParent(sdaiEntity), sdaiInstance, hParent, pAttributeSet);

	SdaiInteger iIndex = 0;
	SdaiAttr sdaiAttribute = engiGetEntityAttributeByIndex(
		sdaiEntity,
		iIndex++,
		false,
		true);

	while (sdaiAttribute != nullptr)
	{
		pAttributeSet->Attributes().push_back(pair<SdaiAttr, SdaiInteger>(sdaiAttribute, iAttrubutesCount++));

		sdaiAttribute = engiGetEntityAttributeByIndex(
			sdaiEntity,
			iIndex++,
			false,
			true);
	}

	return iAttrubutesCount;
}

void CRelationsView::LoadInstanceAttribute(SdaiEntity sdaiEntity, SdaiInstance sdaiInstance, SdaiAttr sdaiAttribute, const char* szAttributeName, HTREEITEM hParent, HTREEITEM hInsertAfter)
{
	if ((sdaiEntity == 0) || (sdaiInstance == 0))
	{
		ASSERT(FALSE);

		return;
	}

	HTREEITEM hAttributesParent = NULL;
	switch (m_enMode)
	{
		case enumRelationsViewMode::Hierarchy:
		{
			TV_INSERTSTRUCT tvInsertStruct;
			tvInsertStruct.hParent = hParent;
			tvInsertStruct.hInsertAfter = TVI_FIRST;
			tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
			tvInsertStruct.item.pszText = (LPWSTR)_entity::getName(sdaiEntity);
			tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_ENTITY;
			tvInsertStruct.item.lParam = NULL;

			hAttributesParent = m_treeCtrl.InsertItem(&tvInsertStruct);
		} // case enumRelationsViewMode::Hierarchy:
		break;

		case enumRelationsViewMode::Flat:
		{
			hAttributesParent = hParent;
		} // case enumRelationsViewMode::Flat:
		break;

		default:
		{
			ASSERT(FALSE); // Not supported!
		}
		break;
	} // switch (m_enMode)

	AddInstanceAttribute(sdaiEntity, sdaiInstance, sdaiAttribute, szAttributeName, hAttributesParent, hInsertAfter);
}

void CRelationsView::AddInstanceAttribute(SdaiEntity sdaiEntity, SdaiInstance sdaiInstance, SdaiAttr sdaiAttribute, const char* szAttributeName, HTREEITEM hParent, HTREEITEM hInsertAfter)
{
	wstring strLabel;
	bool bInverse = false;

	bool bHasChildren = CreateAttributeLabel(sdaiInstance, sdaiAttribute, strLabel);
	if (!bHasChildren)
	{
		bInverse = engiGetAttrInverseBN(sdaiEntity, szAttributeName) != 0;
	}

	wstring strAttribute = CA2W(szAttributeName);
	strAttribute += L" = ";
	strAttribute += strLabel.empty() ? L"$" : strLabel;

	auto pAttributeData = new CAttributeData(sdaiInstance, sdaiEntity, szAttributeName);
	m_vecItemDataCache.push_back(pAttributeData);

	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = hParent;
	tvInsertStruct.hInsertAfter = hInsertAfter;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
	tvInsertStruct.item.pszText = (LPWSTR) strAttribute.c_str();
	tvInsertStruct.item.cChildren = bHasChildren ? 1 : 0;
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = 
		bHasChildren ? IMAGE_INSTANCE : bInverse ? 
			IMAGE_INVERSE_ATTRIBUTE : IMAGE_ATTRIBUTE;
	tvInsertStruct.item.lParam = (LPARAM)pAttributeData;
	HTREEITEM hAttribute = m_treeCtrl.InsertItem(&tvInsertStruct);

	if (bHasChildren)
	{
		/*
		* Add a fake item - load on demand
		*/
		tvInsertStruct.hParent = hAttribute;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = ITEM_PENDING_LOAD;
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_INSTANCE;
		tvInsertStruct.item.lParam = (LPARAM)pAttributeData;

		m_treeCtrl.InsertItem(&tvInsertStruct);
	}
}

void CRelationsView::CreateAttributeLabelInstance(SdaiInstance sdaiInstance, wstring& strLabel)
{
    ASSERT(sdaiInstance != 0);

	CString strValue;
	strValue.Format(_T("%lld"), internalGetP21Line(sdaiInstance));

	strLabel += L"#";
	strLabel += strValue;
}

void CRelationsView::CreateAttributeLabelBoolean(bool bValue, wstring& strLabel)
{
	strLabel += bValue ? L".T." : L".F.";
}

void CRelationsView::CreateAttributeLabelLogical(char* szValue, wstring& strLabel)
{
	strLabel += L".";
	strLabel += CA2W(szValue);
	strLabel += L".";
}

void CRelationsView::CreateAttributeLabelEnumeration(char* szValue, wstring& strLabel)
{
	strLabel += L".";
	strLabel += CA2W(szValue);
	strLabel += L".";
}

void CRelationsView::CreateAttributeLabelReal(double dValue, wstring& strLabel)
{
	CString strValue;
	strValue.Format(_T("%f"), dValue);

	strLabel += strValue;
}

void CRelationsView::CreateAttributeLabelInteger(SdaiInteger iValue, wstring& strLabel)
{
	CString strValue;
	strValue.Format(_T("%lld"), (int64_t)iValue);

	strLabel += strValue;
}

void CRelationsView::CreateAttributeLabelString(wchar_t* szValue, wstring& strLabel)
{
	if (szValue != nullptr) 
	{
		strLabel += L"'";
		strLabel += szValue;
		strLabel += L"'";
	}
	else
	{
		strLabel += L"''";
	}	
}

bool CRelationsView::CreateAttributeLabelADB(SdaiADB ADB, wstring& strLabel)
{
	bool bHasChildren = false;

	strLabel += (const wchar_t*) sdaiGetADBTypePath(ADB, sdaiUNICODE);

	strLabel += L" (";

    switch (sdaiGetADBType(ADB)) 
	{
        case  sdaiADB:
            {
                SdaiADB attributeDataBlock = 0;
                if (sdaiGetADBValue(ADB, sdaiADB, &attributeDataBlock)) 
				{
					bHasChildren |= CreateAttributeLabelADB(attributeDataBlock, strLabel);
                }
                else 
				{
					strLabel += L"$";
                }
            }
            break;

        case  sdaiAGGR:
            {
                SdaiAggr valueAggr = nullptr;
                SdaiInstance iValueInstance = 0;
                if (sdaiGetADBValue(ADB, sdaiAGGR, &valueAggr)) {
					strLabel += L"(";
					bHasChildren |= CreateAttributeLabelAggregation(valueAggr, strLabel);
					strLabel += L")";
                }
                else if (sdaiGetADBValue(ADB, sdaiINSTANCE, &iValueInstance)) 
				{
					CreateAttributeLabelInstance(iValueInstance, strLabel);

					bHasChildren = true;
                }
                else 
				{
					ASSERT(iValueInstance == 0);
                    ASSERT(FALSE);
                }
            }
            break;

        case  sdaiINSTANCE:
            {
                SdaiInstance iValue = 0;
                if (sdaiGetADBValue(ADB, sdaiINSTANCE, &iValue))
				{
					CreateAttributeLabelInstance(iValue, strLabel);

					bHasChildren = true;
                }
                else 
				{
					ASSERT(iValue == 0);

					strLabel += L"$";
                }
            }
            break;

        case  sdaiBOOLEAN:
            {
                bool bValue = false;
                if (sdaiGetADBValue(ADB, sdaiBOOLEAN, &bValue)) 
				{
					CreateAttributeLabelBoolean(bValue, strLabel);
                }
                else 
				{
					strLabel += L"$";
                }
            }
            break;

        case  sdaiLOGICAL:
            {
                char* szValue = nullptr;
                if (sdaiGetADBValue(ADB, sdaiLOGICAL, &szValue)) 
				{
					CreateAttributeLabelLogical(szValue, strLabel);
                }
                else 
				{
					strLabel += L"$";
                }
            }
            break;

        case  sdaiENUM:
            {
                char* szValue = nullptr;
                if (sdaiGetADBValue(ADB, sdaiENUM, &szValue)) 
				{
					CreateAttributeLabelEnumeration(szValue, strLabel);
                }
                else 
				{
					strLabel += L"$";
                }
            }
            break;

        case  sdaiREAL:
            {
                double dValue = 0.;
                if (sdaiGetADBValue(ADB, sdaiREAL, &dValue)) 
				{
					CreateAttributeLabelReal(dValue, strLabel);
                }
                else 
				{
					strLabel += L"$";
                }
            }
            break;

        case  sdaiINTEGER:
            {
				SdaiInteger iValue = 0;
                if (sdaiGetADBValue(ADB, sdaiINTEGER, &iValue)) 
				{
					CreateAttributeLabelInteger(iValue, strLabel);
                }
                else 
				{
					strLabel += L"$";
                }
            }
            break;

        case  sdaiSTRING:
            {
                wchar_t* szValue = nullptr;
                if (sdaiGetADBValue(ADB, sdaiUNICODE, &szValue))
				{
					CreateAttributeLabelString(szValue, strLabel);
                }
                else 
				{
					strLabel += L"$";
                }
            }
            break;

        default:
		{
			ASSERT(FALSE);
		}
		break;
    } // switch (sdaiGetADBType(ADB)) 

	strLabel += L")";

	return	bHasChildren;
}

bool CRelationsView::CreateAttributeLabelAggregationElement(SdaiAggr aggregation, SdaiPrimitiveType sdaiPrimitiveType, SdaiInteger iIndex, wstring& strLabel)
{
	bool bHasChildren = false;

    switch (sdaiPrimitiveType)
	{
        case sdaiADB:
        {
            SdaiADB attributeDataBlock = 0;
            if (sdaiGetAggrByIndex(aggregation, iIndex, sdaiADB, &attributeDataBlock)) 
			{
				bHasChildren |= CreateAttributeLabelADB(attributeDataBlock, strLabel);
            }
            else 
			{
				strLabel += L"$";
            }
        }
        break;

        case sdaiAGGR:
        {
            SdaiAggr valueAggr = nullptr;
            SdaiInstance    iValueInstance = 0;
            if (sdaiGetAggrByIndex(aggregation, iIndex, sdaiAGGR, &valueAggr))
			{
				strLabel += L"(";
				bHasChildren |= CreateAttributeLabelAggregation(valueAggr, strLabel);
				strLabel += L")";
            }
            else if (sdaiGetAggrByIndex(aggregation, iIndex, sdaiINSTANCE, &iValueInstance))
			{
				CreateAttributeLabelInstance(iValueInstance, strLabel);

				bHasChildren = true;
			}
            else 
			{
				ASSERT(iValueInstance == 0);
				ASSERT(FALSE);
			}
        }
        break;

		case sdaiINSTANCE:
        {
            SdaiInstance iValue = 0;
            if (sdaiGetAggrByIndex(aggregation, iIndex, sdaiINSTANCE, &iValue))
			{
				CreateAttributeLabelInstance(iValue, strLabel);

				bHasChildren = true;
			}
			else 
			{
				ASSERT(iValue == 0);

				strLabel += L"$";
			}
		}
        break;

		case sdaiBOOLEAN:
        {
            bool bValue = false;
            if (sdaiGetAggrByIndex(aggregation, iIndex, sdaiBOOLEAN, &bValue))
			{
				CreateAttributeLabelBoolean(bValue, strLabel);
            }
            else 
			{
				strLabel += L"$";
            }
        }
        break;

		case sdaiLOGICAL:
        {
            char* szValue = nullptr;
            if (sdaiGetAggrByIndex(aggregation, iIndex, sdaiLOGICAL, &szValue))
			{
				CreateAttributeLabelLogical(szValue, strLabel);
            }
            else 
			{
				strLabel += L"$";
            }
        }
        break;

        case sdaiENUM:
        {
            char* szValue = nullptr;
            if (sdaiGetAggrByIndex(aggregation, iIndex, sdaiENUM, &szValue))
			{
				CreateAttributeLabelEnumeration(szValue, strLabel);
            }
            else 
			{
				strLabel += L"$";
            }
        }
        break;

        case sdaiREAL:
        {
            double dValue = 0.;
            if (sdaiGetAggrByIndex(aggregation, iIndex, sdaiREAL, &dValue))
			{
				CreateAttributeLabelReal(dValue, strLabel);
            }
            else 
			{
				strLabel += L"$";
            }
        }
        break;

        case sdaiINTEGER:
        {
			SdaiInteger iValue = 0;
            if (sdaiGetAggrByIndex(aggregation, iIndex, sdaiINTEGER, &iValue))
			{
				CreateAttributeLabelInteger(iValue, strLabel);
            }
            else
			{
				strLabel += L"$";
            }
        }
        break;

        case sdaiSTRING:
        {
            wchar_t* szVlue = nullptr;
            if (sdaiGetAggrByIndex(aggregation, iIndex, sdaiUNICODE, &szVlue))
			{
				CreateAttributeLabelString(szVlue, strLabel);
            }
            else 
			{
				strLabel += L"$";
            }
        }
        break;

        default:
		{
			ASSERT(FALSE);
		}
        break;
    } // switch (sdaiPrimitiveType)

	return	bHasChildren;
}

bool CRelationsView::CreateAttributeLabelAggregation(SdaiAggr sdaiAggregation, wstring& strLabel)
{
	bool bHasChildren = false;

    SdaiInteger iMemberCount = sdaiGetMemberCount(sdaiAggregation);
	if (iMemberCount == 0)
	{
		return  bHasChildren;
	}

    SdaiPrimitiveType sdaiPrimitiveType = 0;
    engiGetAggrType(sdaiAggregation, &sdaiPrimitiveType);

    SdaiInteger iIndex = 0;
	bHasChildren |= CreateAttributeLabelAggregationElement(sdaiAggregation, sdaiPrimitiveType, iIndex++, strLabel);

    while (iIndex < iMemberCount)
	{
		strLabel += L", ";

		bHasChildren |= CreateAttributeLabelAggregationElement(sdaiAggregation, sdaiPrimitiveType, iIndex++, strLabel);
    }

	return	bHasChildren;
}

bool CRelationsView::CreateAttributeLabel(SdaiInstance sdaiInstance, SdaiAttr sdaiAttribute, wstring& strLabel)
{	
	strLabel = _T("");
	bool bHasChildren = false;

	SdaiPrimitiveType sdaiPrimitiveType = engiGetAttrType(sdaiAttribute);
    if (sdaiPrimitiveType & engiTypeFlagAggr ||
		sdaiPrimitiveType & engiTypeFlagAggrOption)
		sdaiPrimitiveType = sdaiAGGR;

	switch (sdaiPrimitiveType)
	{
        case 0:
		{
			strLabel += L"$";
		}		
        break;

		case sdaiADB:
		{
			SdaiADB attributeDataBlock = 0;
			if (sdaiGetAttr(sdaiInstance, sdaiAttribute, sdaiADB, &attributeDataBlock))
			{
				ASSERT(attributeDataBlock != nullptr);

				bHasChildren |= CreateAttributeLabelADB(attributeDataBlock, strLabel);
			}
			else
			{
				strLabel += L"$";
			}
		}
		break;

		case sdaiAGGR:
		{
			SdaiAggr valueAggr = nullptr;
			SdaiInstance iValueInstance = 0;
			if (sdaiGetAttr(sdaiInstance, sdaiAttribute, sdaiAGGR, &valueAggr))
			{
				wstring strAggrLabel;
				bHasChildren |= CreateAttributeLabelAggregation(valueAggr, strAggrLabel);

				if (strAggrLabel.size() > MAX_LABEL_SIZE)
				{
					strAggrLabel = strAggrLabel.substr(0, MAX_LABEL_SIZE);
					strAggrLabel += L"...";
				}

				strLabel += L"(";
				strLabel += strAggrLabel;
				strLabel += L")";
			}
			else if (sdaiGetAttr(sdaiInstance, sdaiAttribute, sdaiINSTANCE, &iValueInstance))
			{
				CreateAttributeLabelInstance(iValueInstance, strLabel);

				bHasChildren = true;
			}
			else 
			{
				ASSERT(iValueInstance == 0);

				strLabel += L"$";
			}
		}
		break;

        case sdaiINSTANCE:
        {
            SdaiInstance iValue = 0;
            if (sdaiGetAttr(sdaiInstance, sdaiAttribute, sdaiINSTANCE, &iValue))
			{
				CreateAttributeLabelInstance(iValue, strLabel);

				bHasChildren = true;
			}
            else
			{
				ASSERT(iValue == 0);
				if (engiGetAttrDerived(sdaiGetInstanceType(sdaiInstance), sdaiAttribute))
				{
					strLabel += L"*";
                }
                else 
				{
					strLabel += L"$";
                }
            }
        }
        break;

        case sdaiBOOLEAN:
        {
            bool bValue = false;
            if (sdaiGetAttr(sdaiInstance, sdaiAttribute, sdaiBOOLEAN, &bValue))
			{
				CreateAttributeLabelBoolean(bValue, strLabel);
            }
            else 
			{
				strLabel += L"$";
            }
        }
        break;

        case sdaiLOGICAL:
        {
            char* szValue = nullptr;
            if (sdaiGetAttr(sdaiInstance, sdaiAttribute, sdaiLOGICAL, &szValue))
			{
				CreateAttributeLabelLogical(szValue, strLabel);
            }
            else 
			{
				strLabel += L"$";
            }
        }
        break;

		case sdaiENUM:
        {
            char* szValue = nullptr;
            if (sdaiGetAttr(sdaiInstance, sdaiAttribute, sdaiENUM, &szValue))
			{
				CreateAttributeLabelEnumeration(szValue, strLabel);
            }
            else 
			{
				strLabel += L"$";
            }
        }
        break;

        case sdaiREAL:
        {
            double dValue = 0.;
            if (sdaiGetAttr(sdaiInstance, sdaiAttribute, sdaiREAL, &dValue)) {
				CreateAttributeLabelReal(dValue, strLabel);
            }
            else 
			{
				strLabel += L"$";
            }
        }
        break;

        case sdaiINTEGER:
        {
			SdaiInteger iValue = 0;
            if (sdaiGetAttr(sdaiInstance, sdaiAttribute, sdaiINTEGER, &iValue))
			{
				CreateAttributeLabelInteger(iValue, strLabel);
            }
            else 
			{
				strLabel += L"$";
            }
        }
        break;

		case sdaiSTRING:
        {
            wchar_t* szValue = nullptr;
            if (sdaiGetAttr(sdaiInstance, sdaiAttribute, sdaiUNICODE, &szValue))
			{
				CreateAttributeLabelString(szValue, strLabel);
            }
            else 
			{
				strLabel += L"$";
            }
        }
        break;

		default:
		{
			ASSERT(FALSE);
		}
        break;
	} // switch (sdaiPrimitiveType)

	return bHasChildren;
}

void CRelationsView::GetAttributeReferencesADB(SdaiADB ADB, HTREEITEM hParent)
{
    switch (sdaiGetADBType(ADB)) 
	{
        case sdaiADB:
        {
            SdaiADB attributeDataBlock = 0;
            if (sdaiGetADBValue(ADB, sdaiADB, &attributeDataBlock)) 
			{
				GetAttributeReferencesADB(attributeDataBlock, hParent);
            }
        }
        break;

        case sdaiAGGR:
        {
            SdaiAggr valueAggr = nullptr;
            SdaiInstance iValueInstance = 0;
            if (sdaiGetADBValue(ADB, sdaiAGGR, &valueAggr)) {
				GetAttributeReferencesAggregation(valueAggr, hParent);
            }
            else if (sdaiGetADBValue(ADB, sdaiINSTANCE, &iValueInstance)) 
			{
				SdaiEntity sdaiEntity = sdaiGetInstanceType(iValueInstance);
				LoadInstance(
					sdaiEntity,
					iValueInstance,
					hParent);
			}
            else 
			{
				ASSERT(iValueInstance == 0);
				ASSERT(FALSE);
			}
        }
        break;

        case sdaiINSTANCE:
        {
            SdaiInstance iValue = 0;
            if (sdaiGetADBValue(ADB, sdaiINSTANCE, &iValue))
			{
				SdaiEntity sdaiEntity = sdaiGetInstanceType(iValue);
				LoadInstance(
					sdaiEntity,
					iValue,
					hParent);
			}
			else
			{
				ASSERT(iValue == 0);
			}
        }
        break;

        case  sdaiBOOLEAN:
        case  sdaiLOGICAL:
        case  sdaiENUM:
        case  sdaiREAL:
        case  sdaiINTEGER:
        case  sdaiSTRING:
			break;

        default:
		{
			ASSERT(FALSE);
		}
        break;
    } // switch (sdaiGetADBType(ADB)) 
}


void CRelationsView::GetAttributeReferencesAggregationElement(SdaiAggr aggregation, SdaiPrimitiveType sdaiPrimitiveType, SdaiInteger iIndex, HTREEITEM hParent)
{
    switch (sdaiPrimitiveType)
	{
        case sdaiADB:
        {
            SdaiADB attributeDataBlock = 0;
            if (sdaiGetAggrByIndex(aggregation, iIndex, sdaiADB, &attributeDataBlock))
			{
				GetAttributeReferencesADB(attributeDataBlock, hParent);
            }
        }
        break;

		case sdaiAGGR:
        {
            SdaiAggr valueAggr = nullptr;
            SdaiInstance iValueInstance = 0;
            if (sdaiGetAggrByIndex(aggregation, iIndex, sdaiAGGR, &valueAggr)) {
				GetAttributeReferencesAggregation(valueAggr, hParent);
            }
            else if (sdaiGetAggrByIndex(aggregation, iIndex, sdaiINSTANCE, &iValueInstance)) {
				SdaiEntity sdaiEntity = sdaiGetInstanceType(iValueInstance);
				LoadInstance(
					sdaiEntity,
					iValueInstance,
					hParent);
            }
            else 
			{
				ASSERT(iValueInstance == 0);
				ASSERT(FALSE);
            }
        }
        break;

		case sdaiINSTANCE:
        {
            SdaiInstance iValue = 0;
            if (sdaiGetAggrByIndex(aggregation, iIndex, sdaiINSTANCE, &iValue)) 
			{
				SdaiEntity sdaiEntity = sdaiGetInstanceType(iValue);
				LoadInstance(
					sdaiEntity,
					iValue,
					hParent);
			}
			else
			{
				ASSERT(iValue == 0);
			}
        }
        break;

        case sdaiBOOLEAN:
        case sdaiLOGICAL:
        case sdaiENUM:
        case sdaiREAL:
        case sdaiINTEGER:
        case sdaiSTRING:
			break;

		default:
		{
			ASSERT(FALSE);
		}
        break;
    } // switch (sdaiPrimitiveType)
}

void CRelationsView::GetAttributeReferencesAggregation(SdaiAggr aggregation, HTREEITEM hParent)
{
    SdaiInteger iMemberCount = sdaiGetMemberCount(aggregation);
	if (iMemberCount == 0)
	{
		return;
	}        

	SdaiPrimitiveType sdaiPrimitiveType = 0;
    engiGetAggrType(aggregation, &sdaiPrimitiveType);

    SdaiInteger iIndex = 0;
	GetAttributeReferencesAggregationElement(aggregation, sdaiPrimitiveType, iIndex++, hParent);

    while (iIndex < iMemberCount)
	{
		GetAttributeReferencesAggregationElement(aggregation, sdaiPrimitiveType, iIndex++, hParent);
    }
}

void CRelationsView::GetAttributeReferences(SdaiInstance sdaiInstance, SdaiAttr sdaiAttribute, HTREEITEM hParent)
{
	SdaiPrimitiveType sdaiPrimitiveType = engiGetAttrType(sdaiAttribute);
    if (sdaiPrimitiveType & engiTypeFlagAggr ||
		sdaiPrimitiveType & engiTypeFlagAggrOption)
		sdaiPrimitiveType = sdaiAGGR;

    switch (sdaiPrimitiveType)
	{
        case 0:
		{
		}
        break;

        case sdaiADB:
		{
            SdaiADB attributeDataBlock = 0;
            if (sdaiGetAttr(sdaiInstance, sdaiAttribute, sdaiADB, &attributeDataBlock))
			{
                ASSERT(attributeDataBlock != nullptr);

				GetAttributeReferencesADB(attributeDataBlock, hParent);
            }
        }
        break;

        case sdaiAGGR:
        {
            SdaiAggr valueAggr = nullptr;
            SdaiInstance iValueInstance = 0;
            if (sdaiGetAttr(sdaiInstance, sdaiAttribute, sdaiAGGR, &valueAggr)) {
				GetAttributeReferencesAggregation(valueAggr, hParent);
            }
            else if (sdaiGetAttr(sdaiInstance, sdaiAttribute, sdaiINSTANCE, &iValueInstance)) {
				SdaiEntity sdaiEntity = sdaiGetInstanceType(iValueInstance);
				LoadInstance(
					sdaiEntity,
					iValueInstance,
					hParent);
			}
			else
			{
				ASSERT(iValueInstance == 0);
			}
        }
        break;

        case sdaiINSTANCE:
        {
            SdaiInstance iValue = 0;
            if (sdaiGetAttr(sdaiInstance, sdaiAttribute, sdaiINSTANCE, &iValue)) {
				SdaiEntity sdaiEntity = sdaiGetInstanceType(iValue);
				LoadInstance(
					sdaiEntity,
					iValue,
					hParent);
            }
			else
			{
				ASSERT(iValue == 0);
			}
        }
        break;

        case sdaiBOOLEAN:
        case sdaiLOGICAL:
        case sdaiENUM:
        case sdaiREAL:
		case sdaiINTEGER:
		case sdaiSTRING:
			break;

		default:
		{
			ASSERT(FALSE);
		}
        break;
    } // switch (sdaiPrimitiveType)
}

void CRelationsView::GetEntityHierarchy(SdaiEntity sdaiEntity, vector<wstring>& vecHierarchy) const
{
	ASSERT(sdaiEntity != 0);

	wstring strEntity = _entity::getName(sdaiEntity);
	if (engiGetEntityIsAbstract(sdaiEntity))
	{
		strEntity += L" (ABSTRACT)";
	}

	vecHierarchy.push_back(strEntity);

	SdaiEntity sdaiParentEntity = engiGetEntityParent(sdaiEntity);
	while (sdaiParentEntity != 0)
	{
		strEntity = _entity::getName(sdaiParentEntity);
		if (engiGetEntityIsAbstract(sdaiParentEntity))
		{
			strEntity += L" (ABSTRACT)";
		}

		vecHierarchy.insert(vecHierarchy.begin(), strEntity);

		sdaiParentEntity = engiGetEntityParent(sdaiParentEntity);
	}
}

void CRelationsView::Clean()
{
	for (auto pInstanceData : m_vecItemDataCache)
	{
		delete pInstanceData;
	}
	m_vecItemDataCache.clear();
}

void CRelationsView::ResetView()
{
	// UI
	m_treeCtrl.DeleteAllItems();
	m_pSearchDialog->Reset();

	// Data
	Clean();
}

void CRelationsView::OnNMClickTree(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	*pResult = 0;
}

void CRelationsView::OnNMRClickTree(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	*pResult = 0;
}

void CRelationsView::OnTVNItemexpandingTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	*pResult = 0;

	int iImage, iSelectedImage = -1;
	m_treeCtrl.GetItemImage(pNMTreeView->itemNew.hItem, iImage, iSelectedImage);

	ASSERT(iImage == iSelectedImage);

	if (((iImage == IMAGE_INSTANCE) || (iImage == IMAGE_ATTRIBUTES)) && 
		(pNMTreeView->itemNew.cChildren == 1))
	{
		HTREEITEM hChild = m_treeCtrl.GetChildItem(pNMTreeView->itemNew.hItem);
		if (hChild == NULL)
		{
			ASSERT(FALSE);

			return;
		}

		if (m_treeCtrl.GetItemText(hChild) != ITEM_PENDING_LOAD)
		{
			return;
		}		

		if (iImage == IMAGE_INSTANCE)
		{
			auto pAttributeData = (CAttributeData*)m_treeCtrl.GetItemData(hChild);
			ASSERT(pAttributeData != nullptr);

			m_treeCtrl.DeleteItem(hChild);

			GetAttributeReferences(
				pAttributeData->GetInstance(),
				sdaiGetAttrDefinition(sdaiGetInstanceType(pAttributeData->GetInstance()), pAttributeData->GetName()),
				pNMTreeView->itemNew.hItem);
		}
		else if (iImage == IMAGE_ATTRIBUTES)
		{
			auto pAttributeSet = (CAttributeSet*)m_treeCtrl.GetItemData(hChild);
			ASSERT(pAttributeSet != nullptr);

			HTREEITEM hParent = m_treeCtrl.GetParentItem(pNMTreeView->itemNew.hItem);
			ASSERT(hParent != NULL);			

			for (size_t iAttribute = 0; (iAttribute < pAttributeSet->Attributes().size()); iAttribute++)
			{
				const char* szAttributeName = nullptr;
				engiGetEntityArgumentName(pAttributeSet->GetEntity(),
					pAttributeSet->Attributes()[iAttribute].second,
					sdaiSTRING,
					&szAttributeName);

				LoadInstanceAttribute(
					pAttributeSet->GetEntity(),
					pAttributeSet->GetInstance(),
					pAttributeSet->Attributes()[iAttribute].first,
					szAttributeName,
					hParent,
					pNMTreeView->itemNew.hItem);
			}

			m_treeCtrl.DeleteItem(pNMTreeView->itemNew.hItem);
		} // else if (iImage == IMAGE_ATTRIBUTES)
	} // if ((iImage == IMAGE_INSTANCE) && ...
}

void CRelationsView::OnTVNGetInfoTip(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;

	auto pNMTVGetInfoTip = reinterpret_cast<LPNMTVGETINFOTIPW>(pNMHDR);

	m_strTooltip = pNMTVGetInfoTip->pszText;

	int iImage, iSelectedImage = -1;
	m_treeCtrl.GetItemImage(pNMTVGetInfoTip->hItem, iImage, iSelectedImage);

	ASSERT(iImage == iSelectedImage);

	if ((iImage == IMAGE_ATTRIBUTE) || (iImage == IMAGE_INVERSE_ATTRIBUTE))
	{
		auto pAttributeData = (CAttributeData*)m_treeCtrl.GetItemData(pNMTVGetInfoTip->hItem);
		if (pAttributeData != nullptr)
		{
			m_strTooltip = CA2W(pAttributeData->GetName());
			if (engiGetAttrInverseBN(pAttributeData->GetEntity(), pAttributeData->GetName()))
			{
				m_strTooltip += L" (INVERSE)";
			}

			m_strTooltip += L"\n";

			m_strTooltip += _entity::getName(pAttributeData->GetEntity());
			if (engiGetEntityIsAbstract(pAttributeData->GetEntity()))
			{
				m_strTooltip += L" (ABSTRACT)";
			}
		} // if (pAttributeData != nullptr)
		else
		{
			ASSERT(FALSE); // Internal error!
		}
	} // if ((iImage == IMAGE_ATTRIBUTE) || ...
	else if (iImage == IMAGE_INSTANCE)
	{
		auto pInstanceData = (CInstanceData*)m_treeCtrl.GetItemData(pNMTVGetInfoTip->hItem);
		if (pInstanceData != nullptr)
		{
			vector<wstring> vecHierarchy;
			GetEntityHierarchy(pInstanceData->GetEntity(), vecHierarchy);

			m_strTooltip = L"";
			for (auto strEntity : vecHierarchy)
			{
				if (!m_strTooltip.empty())
				{
					m_strTooltip += L"\n";
				}

				m_strTooltip += strEntity;
			}
		} // if (pAttributeData != nullptr)
		else
		{
			ASSERT(FALSE); // Internal error!
		}
	} // if (iImage == IMAGE_INSTANCE)

	pNMTVGetInfoTip->pszText = (LPWSTR)m_strTooltip.c_str();	
}

CRelationsView::CRelationsView()
	: m_enMode(enumRelationsViewMode::Flat)
	, m_vecItemDataCache()
	, m_pSearchDialog(nullptr)
	, m_strTooltip(L"")
{
}

CRelationsView::~CRelationsView()
{
	Clean();
}

BEGIN_MESSAGE_MAP(CRelationsView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_PROPERTIES, OnProperties)
	ON_WM_CONTEXTMENU()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_NOTIFY(NM_CLICK, IDC_TREE_IFC, &CRelationsView::OnNMClickTree)
	ON_NOTIFY(NM_RCLICK, IDC_TREE_IFC, &CRelationsView::OnNMRClickTree)
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_TREE_IFC, &CRelationsView::OnTVNItemexpandingTree)
	ON_NOTIFY(TVN_GETINFOTIP, IDC_TREE_IFC, &CRelationsView::OnTVNGetInfoTip)
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorkspaceBar message handlers

int CRelationsView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	ASSERT(getController() != nullptr);
	getController()->registerView(this);

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create view:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_INFOTIP;

	if (!m_treeCtrl.Create(dwViewStyle, rectDummy, this, IDC_TREE_IFC))
	{
		ASSERT(FALSE);

		return -1;
	}

	// Load view images:
	m_imageList.Create(IDB_CLASS_VIEW, 16, 0, RGB(255, 0, 0));
	m_treeCtrl.SetImageList(&m_imageList, TVSIL_NORMAL);

	m_toolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_EXPLORER);
	m_toolBar.LoadToolBar(IDR_EXPLORER, 0, 0, TRUE /* Is locked */);

	OnChangeVisualStyle();

	m_toolBar.SetPaneStyle(m_toolBar.GetPaneStyle() 
		| CBRS_TOOLTIPS | CBRS_FLYBY);

	m_toolBar.SetPaneStyle(m_toolBar.GetPaneStyle() & 
		~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	m_toolBar.SetOwner(this);

	// All commands will be routed via this control , not via the parent frame:
	m_toolBar.SetRouteCommandsViaFrame(FALSE);

	AdjustLayout();

	//  Search
	m_pSearchDialog = new CSearchTreeCtrlDialog(this);
	m_pSearchDialog->Create(IDD_DIALOG_SEARCH, this);

	return 0;
}

void CRelationsView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CRelationsView::AdjustLayout()
{
	if (GetSafeHwnd() == nullptr)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_toolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_toolBar.SetWindowPos(
		nullptr, 
		rectClient.left, 
		rectClient.top, 
		rectClient.Width(), 
		cyTlb, 
		SWP_NOACTIVATE | SWP_NOZORDER);

	m_treeCtrl.SetWindowPos(
		nullptr, rectClient.left + 1, 
		rectClient.top + cyTlb + 1, 
		rectClient.Width() - 2, 
		rectClient.Height() - cyTlb - 2, 
		SWP_NOACTIVATE | SWP_NOZORDER);
}

void CRelationsView::OnProperties()
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

void CRelationsView::OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/)
{	
}

void CRelationsView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rectTree;
	m_treeCtrl.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CRelationsView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_treeCtrl.SetFocus();
}

void CRelationsView::OnChangeVisualStyle()
{
	m_imageList.DeleteImageList();

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

	m_imageList.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
	m_imageList.Add(&bmp, RGB(255, 0, 0));

	m_treeCtrl.SetImageList(&m_imageList, TVSIL_NORMAL);

	m_toolBar.CleanUpLockedImages();
	m_toolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_EXPLORER_24 : IDR_EXPLORER, 0, 0, TRUE /* Locked */);
}

void CRelationsView::OnDestroy()
{
	ASSERT(getController() != nullptr);
	getController()->unRegisterView(this);

	__super::OnDestroy();

	delete m_pSearchDialog;
}

void CRelationsView::OnShowWindow(BOOL bShow, UINT nStatus)
{
	__super::OnShowWindow(bShow, nStatus);

	if (!bShow)
	{
		m_pSearchDialog->ShowWindow(SW_HIDE);
	}
}
