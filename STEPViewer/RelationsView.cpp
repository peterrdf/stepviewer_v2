
#include "stdafx.h"

#include "_ap_mvc.h"
#include "_entity.h"

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
#define IMAGE_INVERSE_ATTRIBUTE	3
#define IMAGE_ATTRIBUTE			5
#define IMAGE_PENDING_LOAD		2

#define ITEM_SUB_TYPES			L"Sub-types"
#define ITEM_ATTRIBUTES			L"Attributes"
#define ITEM_PENDING_LOAD		L"***..........***"

#define MAX_LABEL_SIZE			50
#define LOAD_INSTANCES_LIMIT	500
#define LOAD_ATTRIBUTES_LIMIT	500

// ************************************************************************************************
/*virtual*/ void CRelationsView::onModelLoaded() /*override*/
{
	m_pEntity = nullptr;

	LoadInstances(vector<SdaiInstance>(), true, NULL);
}

/*virtual*/ void CRelationsView::onInstanceSelected(_view* pSender) /*override*/
{

	if (pSender == this) {
		return;
	}
}

/*virtual*/ void CRelationsView::onViewRelations(_view* pSender, SdaiInstance sdaiInstance) /*override*/
{
	if (pSender == this) {
		return;
	}

	if (sdaiInstance == 0) {
		ASSERT(FALSE);

		return;
	}

	LoadInstances(vector<SdaiInstance>{ sdaiInstance }, true, NULL);

	ShowPane(TRUE, TRUE, TRUE);
}

/*virtual*/ void CRelationsView::onViewRelations(_view* pSender, _entity* pEntity) /*override*/
{
	if (pSender == this) {
		return;
	}

	if (m_pEntity == pEntity) {
		return;
	}

	m_pEntity = pEntity;

	vector<SdaiInstance> vecInstances;
	vecInstances.insert(vecInstances.begin(), pEntity->getInstances().begin(), pEntity->getInstances().end());

	if (vecInstances.empty()) {
		// Retrieve all instances
		map<_entity*, vector<SdaiInstance>> mapEntity2Instances;
		GetAllInstances(pEntity, mapEntity2Instances);

		// Sort
		map<ExpressID, SdaiInstance> mapExpressID2Instance;
		for (const auto& itEntity2Instances : mapEntity2Instances) {
			for (auto iInstance : itEntity2Instances.second) {
				auto iExpressID = internalGetP21Line(iInstance);
				ASSERT(mapExpressID2Instance.find(iExpressID) == mapExpressID2Instance.end());

				mapExpressID2Instance[iExpressID] = iInstance;
			}
		}

		// Collect
		for (const auto& itExpressID2Instance : mapExpressID2Instance) {
			vecInstances.push_back(itExpressID2Instance.second);
		}
	} // if (vecInstances.empty())

	// Sort by ExpressID
	std::sort(vecInstances.begin(), vecInstances.end(),
		[](SdaiInstance a, SdaiInstance b) {
			return internalGetP21Line(a) < internalGetP21Line(b);
		});

	// Load
	LoadInstances(vecInstances, true, NULL);

	ShowPane(TRUE, TRUE, TRUE);
}

/*virtual*/ void CRelationsView::onInstanceAttributeEdited(_view* pSender, SdaiInstance sdaiInstance, SdaiAttr sdaiAttr) /*override*/
{
	if (pSender == this) {
		return;
	}

	auto itInstance = m_mapInstances.find(sdaiInstance);
	if (itInstance == m_mapInstances.end()) {
		return;
	}

	wstring strInstance = _ap_geometry::getDisplayString(sdaiInstance);
	for (auto hInstance : itInstance->second) {
		m_treeCtrl.SetItemText(hInstance, strInstance.c_str());
	}

	auto itInstanceAttributes = m_mapInstanceAttributes.find(sdaiInstance);
	ASSERT(itInstanceAttributes != m_mapInstanceAttributes.end());

	auto itAttribute = itInstanceAttributes->second.find(sdaiAttr);
	if (itAttribute == itInstanceAttributes->second.end()) {
		// Not loaded
		return;
	}

	for (auto hAttribute : itAttribute->second) {
		auto pAttributeData = (CAttributeData*)m_treeCtrl.GetItemData(hAttribute);
		if (pAttributeData == nullptr) {
			ASSERT(FALSE); // Internal error!

			return;
		}

		wstring strLabel;
		CreateAttributeLabel(sdaiInstance, sdaiAttr, strLabel);

		wstring strText = CA2W(pAttributeData->GetName());
		strText += L" = ";
		strText += strLabel.empty() ? L"$" : strLabel;

		m_treeCtrl.SetItemText(hAttribute, strText.c_str());
	} // for (auto hItem : ...	
}

/*virtual*/ bool CRelationsView::IsSelected(HTREEITEM hItem) /*override*/
{
	return m_treeCtrl.GetSelectedItem() == hItem;
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
	if (strFilter == _T("Express ID")) {
		return _T("Select");
	}

	return __super::GetSearchFilterType(strFilter);
}

/*virtual*/ void CRelationsView::LoadChildrenIfNeeded(HTREEITEM hItem) /*override*/
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

/*virtual*/ BOOL CRelationsView::ProcessSearch(int iFilter, const CString& strSearchText) /*override*/
{
	// ExpressID
	if (iFilter == (int)enumSearchFilter::ExpressID) {
		auto pController = getAPController();
		if (pController == nullptr) {
			ASSERT(FALSE);

			return FALSE;
		}

		auto pModel = pController->getModel();
		if (pModel == nullptr) {
			ASSERT(FALSE);

			return FALSE;
		}

		_ptr<_ap_model> apModel(pModel);
		if (apModel) {
			int64_t iExpressID = _wtoi64((LPCTSTR)strSearchText);

			SdaiInstance sdaiInstance = internalGetInstanceFromP21Line(apModel->getSdaiModel(), iExpressID);
			if (sdaiInstance != 0) {
				pController->onViewRelations(
					nullptr,  /*Attributes View will be updated also*/
					sdaiInstance);
			}
			else {
				::MessageBox(::AfxGetMainWnd()->GetSafeHwnd(), L"Invalid Express ID.", L"Search", MB_ICONERROR | MB_OK);
			}
		}
		else {
			ASSERT(FALSE); // Unknown
		}

		return TRUE;
	} // if (iFilter == (int)enumSearchFilter::ExpressID)

	return FALSE;
}

/*virtual*/ BOOL CRelationsView::ContainsText(int iFilter, HTREEITEM hItem, const CString& strText) /*override*/
{
	UNREFERENCED_PARAMETER(iFilter);

	if (hItem == NULL) {
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

_ap_model* CRelationsView::GetModel() const
{
	auto pController = getController();
	if (pController == nullptr) {
		ASSERT(FALSE);

		return nullptr;
	}

	auto pModel = pController->getModel();
	if (pModel == nullptr) {
		return nullptr;
	}

	return _ptr<_ap_model>(pModel);
}

void CRelationsView::LoadInstances(const vector<SdaiInstance>& vecInstances, bool bResetView, HTREEITEM hInsertAfter)
{
	auto pModel = GetModel();
	if (pModel == nullptr) {
		return;
	}

	HTREEITEM hModel = NULL;
	if (bResetView) {
		ResetView();

		TV_INSERTSTRUCT tvInsertStruct;
		tvInsertStruct.hParent = nullptr;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = (LPWSTR)pModel->getPath();
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_MODEL;
		tvInsertStruct.item.lParam = NULL;

		hModel = m_treeCtrl.InsertItem(&tvInsertStruct);
	}
	else {
		hModel = m_treeCtrl.GetRootItem();
	}

	/* Instances */

	// Load first page
	size_t iInstanceStart = 0;
	size_t iInstanceEnd = iInstanceStart + LOAD_INSTANCES_LIMIT;
	if (iInstanceEnd >= vecInstances.size()) {
		iInstanceEnd = vecInstances.size();
	}

	for (size_t iInstance = iInstanceStart; (iInstance < iInstanceEnd); iInstance++) {
		hInsertAfter = LoadInstance(vecInstances[iInstance], hModel, hInsertAfter);

		iInstanceStart++;
	}

	// Load on demand
	while (iInstanceStart < vecInstances.size()) {
		iInstanceEnd = iInstanceStart + LOAD_INSTANCES_LIMIT;
		if (iInstanceEnd >= vecInstances.size()) {
			iInstanceEnd = vecInstances.size();
		}

		auto pInstanceSet = new CInstanceSet();
		m_vecItemDataCache.push_back(pInstanceSet);

		for (size_t iInstance = iInstanceStart; (iInstance < iInstanceEnd); iInstance++) {
			pInstanceSet->Instances().push_back(vecInstances[iInstance]);
		}

		CString strPage;
		strPage.Format(L"[%lld - %lld] (#%lld - #%lld)", 
			iInstanceStart + 1, iInstanceEnd,
			internalGetP21Line(vecInstances[iInstanceStart]), internalGetP21Line(vecInstances[iInstanceEnd - 1]));

		// Pending load
		TV_INSERTSTRUCT tvInsertStruct;
		tvInsertStruct.hParent = hModel;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;;
		tvInsertStruct.item.pszText = strPage.GetBuffer();
		tvInsertStruct.item.cChildren = 1;
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PENDING_LOAD;
		tvInsertStruct.item.lParam = NULL;

		HTREEITEM hInstances = m_treeCtrl.InsertItem(&tvInsertStruct);

		// Add a fake item - load on demand		
		tvInsertStruct.hParent = hInstances;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = ITEM_PENDING_LOAD;
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_INSTANCE;
		tvInsertStruct.item.lParam = (LPARAM)pInstanceSet;

		m_treeCtrl.InsertItem(&tvInsertStruct);

		iInstanceStart = iInstanceEnd;
	} // while (iInstanceStart < ...

	m_treeCtrl.Expand(hModel, TVE_EXPAND);
}

HTREEITEM CRelationsView::LoadInstance(SdaiInstance sdaiInstance, HTREEITEM hParent, HTREEITEM hInsertAfter)
{
	ASSERT(sdaiInstance != 0);

	SdaiEntity sdaiEntity = _ap_geometry::getSdaiEntity(sdaiInstance);
	ASSERT(sdaiEntity != 0);

	/*
	* Data
	*/
	auto pInstanceData = new CInstanceData(sdaiInstance, sdaiEntity);
	m_vecItemDataCache.push_back(pInstanceData);

	/*
	* Instance
	*/
	wstring strItem = _ap_geometry::getDisplayString(sdaiInstance);

	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = hParent;
	tvInsertStruct.hInsertAfter = hInsertAfter != NULL ? hInsertAfter : TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_INSTANCE;
	tvInsertStruct.item.lParam = (LPARAM)pInstanceData;

	HTREEITEM hInstance = m_treeCtrl.InsertItem(&tvInsertStruct);

	CAttributeSet attributeSet(sdaiInstance, sdaiEntity);
	GetInstanceAttributes(sdaiEntity, &attributeSet);

	// Cache
	auto itInstance = m_mapInstances.find(sdaiInstance);
	if (itInstance == m_mapInstances.end()) {
		m_mapInstances[sdaiInstance] = vector<HTREEITEM>{ hInstance };
	}
	else {
		itInstance->second.push_back(hInstance);
	}

	// Load first page
	size_t iAttributeStart = 0;
	size_t iAttributeEnd = iAttributeStart + LOAD_ATTRIBUTES_LIMIT;
	if (iAttributeEnd >= attributeSet.Attributes().size()) {
		iAttributeEnd = attributeSet.Attributes().size();
	}

	for (size_t iAttribute = iAttributeStart; (iAttribute < iAttributeEnd); iAttribute++) {
		auto sdaiAttr = attributeSet.Attributes()[iAttribute];

		LoadInstanceAttribute(
			sdaiEntity,
			sdaiInstance,
			sdaiAttr,
			hInstance,
			TVI_LAST);

		iAttributeStart++;
	}

	// Load on demand
	while (iAttributeStart < attributeSet.Attributes().size()) {
		iAttributeEnd = iAttributeStart + LOAD_ATTRIBUTES_LIMIT;
		if (iAttributeEnd >= attributeSet.Attributes().size()) {
			iAttributeEnd = attributeSet.Attributes().size();
		}

		auto pAttributeSet = new CAttributeSet(sdaiInstance, sdaiEntity);
		m_vecItemDataCache.push_back(pAttributeSet);

		for (size_t iAttribute = iAttributeStart; (iAttribute < iAttributeEnd); iAttribute++) {
			pAttributeSet->Attributes().push_back(attributeSet.Attributes()[iAttribute]);
		}

		CString strPage;
		strPage.Format(L"[%lld - %lld]", iAttributeStart + 1, iAttributeEnd);

		// Pending load
		tvInsertStruct.hParent = hInstance;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;;
		tvInsertStruct.item.pszText = strPage.GetBuffer();
		tvInsertStruct.item.cChildren = 1;
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PENDING_LOAD;
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

	return hInstance;
}

int_t CRelationsView::GetInstanceAttributes(SdaiEntity sdaiEntity, CAttributeSet* pAttributeSet)
{
	if (sdaiEntity == 0) {
		return 0;
	}

	SdaiInteger iIndex = 0;
	while (SdaiAttr sdaiAttribute = engiGetEntityAttributeByIndex(sdaiEntity, iIndex++, true, true)) {
		pAttributeSet->Attributes().push_back(sdaiAttribute);
	}

	return iIndex;
}

void CRelationsView::LoadInstanceAttribute(SdaiEntity sdaiEntity, SdaiInstance sdaiInstance, SdaiAttr sdaiAttribute, HTREEITEM hParent, HTREEITEM hInsertAfter)
{
	if (!sdaiEntity || !sdaiInstance || !sdaiAttribute) {
		ASSERT(FALSE);

		return;
	}



	HTREEITEM hAttributesParent = NULL;
	switch (m_enMode) {
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

	AddInstanceAttribute(sdaiEntity, sdaiInstance, sdaiAttribute, hAttributesParent, hInsertAfter);
}

void CRelationsView::AddInstanceAttribute(SdaiEntity sdaiEntity, SdaiInstance sdaiInstance, SdaiAttr sdaiAttribute, HTREEITEM hParent, HTREEITEM hInsertAfter)
{
	wstring strLabel;
	bool bInverse = false;

	auto szAttributeName = engiGetAttrName(sdaiAttribute);
	bool bHasChildren = CreateAttributeLabel(sdaiInstance, sdaiAttribute, strLabel);
	if (!bHasChildren) {
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
	tvInsertStruct.item.pszText = (LPWSTR)strAttribute.c_str();
	tvInsertStruct.item.cChildren = bHasChildren ? 1 : 0;
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage =
		bHasChildren ? IMAGE_INSTANCE : bInverse ?
		IMAGE_INVERSE_ATTRIBUTE : IMAGE_ATTRIBUTE;
	tvInsertStruct.item.lParam = (LPARAM)pAttributeData;

	HTREEITEM hAttribute = m_treeCtrl.InsertItem(&tvInsertStruct);

	// Cache
	if ((tvInsertStruct.item.iImage == IMAGE_INVERSE_ATTRIBUTE) ||
		(tvInsertStruct.item.iImage == IMAGE_ATTRIBUTE)) {
		ASSERT(m_mapInstances.find(sdaiInstance) != m_mapInstances.end());

		auto itInstanceAttributes = m_mapInstanceAttributes.find(sdaiInstance);
		if (itInstanceAttributes == m_mapInstanceAttributes.end()) {
			m_mapInstanceAttributes[sdaiInstance] = map<SdaiAttr, vector<HTREEITEM>>();

			itInstanceAttributes = m_mapInstanceAttributes.find(sdaiInstance);
		}

		auto itAttribute = itInstanceAttributes->second.find(sdaiAttribute);
		if (itAttribute == itInstanceAttributes->second.end()) {
			itInstanceAttributes->second[sdaiAttribute] = vector<HTREEITEM>{ hAttribute };
		}
		else {
			itAttribute->second.push_back(hAttribute);
		}
	} // IMAGE_INVERSE_ATTRIBUTE || IMAGE_ATTRIBUTE

	if (bHasChildren) {
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

void CRelationsView::CreateAttributeLabelInteger(int_t iValue, wstring& strLabel)
{
	CString strValue;
	strValue.Format(_T("%lld"), iValue);

	strLabel += strValue;
}

void CRelationsView::CreateAttributeLabelString(wchar_t* szValue, wstring& strLabel)
{
	if (szValue != nullptr) {
		strLabel += L"'";
		strLabel += szValue;
		strLabel += L"'";
	}
	else {
		strLabel += L"''";
	}
}

bool CRelationsView::CreateAttributeLabelADB(SdaiADB ADB, wstring& strLabel)
{
	bool bHasChildren = false;

	strLabel += (const wchar_t*)sdaiGetADBTypePath(ADB, sdaiUNICODE);

	strLabel += L" (";

	auto sdaiType = sdaiGetADBType(ADB);
	switch (sdaiType) {
		case  sdaiADB:
			{
				SdaiADB attributeDataBlock = 0;
				if (sdaiGetADBValue(ADB, sdaiADB, &attributeDataBlock)) {
					bHasChildren |= CreateAttributeLabelADB(attributeDataBlock, strLabel);
				}
				else {
					strLabel += L"$";
				}
			}
			break;

		case  sdaiAGGR:
			{
				SdaiAggr sdaiValueAggr = nullptr;
				SdaiInstance sdaiValueInstance = 0;
				if (sdaiGetADBValue(ADB, sdaiAGGR, &sdaiValueAggr)) {
					strLabel += L"(";
					bHasChildren |= CreateAttributeLabelAggregation(sdaiValueAggr, strLabel);
					strLabel += L")";
				}
				else if (sdaiGetADBValue(ADB, sdaiINSTANCE, &sdaiValueInstance)) {
					CreateAttributeLabelInstance(sdaiValueInstance, strLabel);

					bHasChildren = true;
				}
				else {
					ASSERT(sdaiValueInstance == 0);
					ASSERT(FALSE);
				}
			}
			break;

		case  sdaiINSTANCE:
			{
				SdaiInstance sdaiValue = 0;
				if (sdaiGetADBValue(ADB, sdaiINSTANCE, &sdaiValue)) {
					CreateAttributeLabelInstance(sdaiValue, strLabel);

					bHasChildren = true;
				}
				else {
					ASSERT(sdaiValue == 0);

					strLabel += L"$";
				}
			}
			break;

		case  sdaiBOOLEAN:
			{
				bool bValue = false;
				if (sdaiGetADBValue(ADB, sdaiBOOLEAN, &bValue)) {
					CreateAttributeLabelBoolean(bValue, strLabel);
				}
				else {
					strLabel += L"$";
				}
			}
			break;

		case  sdaiLOGICAL:
			{
				char* szValue = nullptr;
				if (sdaiGetADBValue(ADB, sdaiLOGICAL, &szValue)) {
					CreateAttributeLabelLogical(szValue, strLabel);
				}
				else {
					strLabel += L"$";
				}
			}
			break;

		case  sdaiENUM:
			{
				char* szValue = nullptr;
				if (sdaiGetADBValue(ADB, sdaiENUM, &szValue)) {
					CreateAttributeLabelEnumeration(szValue, strLabel);
				}
				else {
					strLabel += L"$";
				}
			}
			break;

		case  sdaiREAL:
			{
				double dValue = 0.;
				if (sdaiGetADBValue(ADB, sdaiREAL, &dValue)) {
					CreateAttributeLabelReal(dValue, strLabel);
				}
				else {
					strLabel += L"$";
				}
			}
			break;

		case  sdaiINTEGER:
			{
				int_t iValue = 0;
				if (sdaiGetADBValue(ADB, sdaiINTEGER, &iValue)) {
					CreateAttributeLabelInteger(iValue, strLabel);
				}
				else {
					strLabel += L"$";
				}
			}
			break;

		case  sdaiSTRING:
		case  sdaiBINARY:
			{
				wchar_t* szValue = nullptr;
				if (sdaiGetADBValue(ADB, sdaiUNICODE, &szValue)) {
					CreateAttributeLabelString(szValue, strLabel);
				}
				else {
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

bool CRelationsView::CreateAttributeLabelAggregationElement(SdaiAggr aggregation, SdaiPrimitiveType sdaiAggrType, SdaiInteger iIndex, wstring& strLabel)
{
	bool bHasChildren = false;

	switch (sdaiAggrType) {
		case sdaiADB:
			{
				SdaiADB attributeDataBlock = 0;
				if (sdaiGetAggrByIndex(aggregation, iIndex, sdaiADB, &attributeDataBlock)) {
					bHasChildren |= CreateAttributeLabelADB(attributeDataBlock, strLabel);
				}
				else {
					strLabel += L"$";
				}
			}
			break;

		case sdaiAGGR:
			{
				SdaiAggr sdaiValueAggr = nullptr;
				SdaiInstance sdaiValueInstance = 0;
				if (sdaiGetAggrByIndex(aggregation, iIndex, sdaiAGGR, &sdaiValueAggr)) {
					strLabel += L"(";
					bHasChildren |= CreateAttributeLabelAggregation(sdaiValueAggr, strLabel);
					strLabel += L")";
				}
				else if (sdaiGetAggrByIndex(aggregation, iIndex, sdaiINSTANCE, &sdaiValueInstance)) {
					CreateAttributeLabelInstance(sdaiValueInstance, strLabel);

					bHasChildren = true;
				}
				else {
					ASSERT(sdaiValueInstance == 0);
					ASSERT(FALSE);
				}
			}
			break;

		case sdaiINSTANCE:
			{
				SdaiInstance sdaiValue = 0;
				if (sdaiGetAggrByIndex(aggregation, iIndex, sdaiINSTANCE, &sdaiValue)) {
					CreateAttributeLabelInstance(sdaiValue, strLabel);

					bHasChildren = true;
				}
				else {
					ASSERT(sdaiValue == 0);

					strLabel += L"$";
				}
			}
			break;

		case sdaiBOOLEAN:
			{
				bool bValue = false;
				if (sdaiGetAggrByIndex(aggregation, iIndex, sdaiBOOLEAN, &bValue)) {
					CreateAttributeLabelBoolean(bValue, strLabel);
				}
				else {
					strLabel += L"$";
				}
			}
			break;

		case sdaiLOGICAL:
			{
				char* szValue = nullptr;
				if (sdaiGetAggrByIndex(aggregation, iIndex, sdaiLOGICAL, &szValue)) {
					CreateAttributeLabelLogical(szValue, strLabel);
				}
				else {
					strLabel += L"$";
				}
			}
			break;

		case sdaiENUM:
			{
				char* szValue = nullptr;
				if (sdaiGetAggrByIndex(aggregation, iIndex, sdaiENUM, &szValue)) {
					CreateAttributeLabelEnumeration(szValue, strLabel);
				}
				else {
					strLabel += L"$";
				}
			}
			break;

		case sdaiREAL:
			{
				double dValue = 0.;
				if (sdaiGetAggrByIndex(aggregation, iIndex, sdaiREAL, &dValue)) {
					CreateAttributeLabelReal(dValue, strLabel);
				}
				else {
					strLabel += L"$";
				}
			}
			break;

		case sdaiINTEGER:
			{
				int_t iValue = 0;
				if (sdaiGetAggrByIndex(aggregation, iIndex, sdaiINTEGER, &iValue)) {
					CreateAttributeLabelInteger(iValue, strLabel);
				}
				else {
					strLabel += L"$";
				}
			}
			break;

		case sdaiSTRING:
			{
				wchar_t* szValue = nullptr;
				if (sdaiGetAggrByIndex(aggregation, iIndex, sdaiUNICODE, &szValue)) {
					CreateAttributeLabelString(szValue, strLabel);
				}
				else {
					strLabel += L"$";
				}
			}
			break;

		default:
			{
				ASSERT(FALSE);
			}
			break;
	} // switch (sdaiAggrType)

	return	bHasChildren;
}

bool CRelationsView::CreateAttributeLabelAggregation(SdaiAggr sdaiAggregation, wstring& strLabel)
{
	bool bHasChildren = false;

	SdaiInteger iMemberCount = sdaiGetMemberCount(sdaiAggregation);
	if (iMemberCount == 0) {
		return  bHasChildren;
	}

	SdaiPrimitiveType sdaiAggrType = 0;
	engiGetAggrType(sdaiAggregation, &sdaiAggrType);

	SdaiInteger iIndex = 0;
	bHasChildren |= CreateAttributeLabelAggregationElement(sdaiAggregation, sdaiAggrType, iIndex++, strLabel);

	while (iIndex < iMemberCount) {
		strLabel += L", ";

		bHasChildren |= CreateAttributeLabelAggregationElement(sdaiAggregation, sdaiAggrType, iIndex++, strLabel);
	}

	return	bHasChildren;
}

bool CRelationsView::CreateAttributeLabel(SdaiInstance sdaiInstance, SdaiAttr sdaiAttribute, wstring& strLabel)
{
	strLabel = _T("");
	bool bHasChildren = false;

	SdaiPrimitiveType sdaiAttributeType = engiGetAttrType(sdaiAttribute);
	if (sdaiAttributeType & engiTypeFlagAggr ||
		sdaiAttributeType & engiTypeFlagAggrOption)
		sdaiAttributeType = sdaiAGGR;

	switch (sdaiAttributeType) {
		case 0:
			{
				strLabel += L"$";
			}
			break;

		case sdaiADB:
			{
				SdaiADB attributeDataBlock = 0;
				if (sdaiGetAttr(sdaiInstance, sdaiAttribute, sdaiADB, &attributeDataBlock)) {
					ASSERT(attributeDataBlock != nullptr);

					bHasChildren |= CreateAttributeLabelADB(attributeDataBlock, strLabel);
				}
				else {
					strLabel += L"$";
				}
			}
			break;

		case sdaiAGGR:
			{
				SdaiAggr sdaiValueAggr = nullptr;
				SdaiInstance sdaiValueInstance = 0;
				if (sdaiGetAttr(sdaiInstance, sdaiAttribute, sdaiAGGR, &sdaiValueAggr)) {
					wstring strAggrLabel;
					bHasChildren |= CreateAttributeLabelAggregation(sdaiValueAggr, strAggrLabel);

					if (strAggrLabel.size() > MAX_LABEL_SIZE) {
						strAggrLabel = strAggrLabel.substr(0, MAX_LABEL_SIZE);
						strAggrLabel += L"...";
					}

					strLabel += L"(";
					strLabel += strAggrLabel;
					strLabel += L")";
				}
				else if (sdaiGetAttr(sdaiInstance, sdaiAttribute, sdaiINSTANCE, &sdaiValueInstance)) {
					CreateAttributeLabelInstance(sdaiValueInstance, strLabel);

					bHasChildren = true;
				}
				else {
					ASSERT(sdaiValueInstance == 0);

					strLabel += L"$";
				}
			}
			break;

		case sdaiINSTANCE:
			{
				SdaiInstance sdaiValue = 0;
				if (sdaiGetAttr(sdaiInstance, sdaiAttribute, sdaiINSTANCE, &sdaiValue)) {
					CreateAttributeLabelInstance(sdaiValue, strLabel);

					bHasChildren = true;
				}
				else {
					ASSERT(sdaiValue == 0);
					if (engiGetAttrDerived(sdaiGetInstanceType(sdaiInstance), sdaiAttribute)) {
						strLabel += L"*";
					}
					else {
						strLabel += L"$";
					}
				}
			}
			break;

		case sdaiBOOLEAN:
			{
				bool bValue = false;
				if (sdaiGetAttr(sdaiInstance, sdaiAttribute, sdaiBOOLEAN, &bValue)) {
					CreateAttributeLabelBoolean(bValue, strLabel);
				}
				else {
					strLabel += L"$";
				}
			}
			break;

		case sdaiLOGICAL:
			{
				char* szValue = nullptr;
				if (sdaiGetAttr(sdaiInstance, sdaiAttribute, sdaiLOGICAL, &szValue)) {
					CreateAttributeLabelLogical(szValue, strLabel);
				}
				else {
					strLabel += L"$";
				}
			}
			break;

		case sdaiENUM:
			{
				char* szValue = nullptr;
				if (sdaiGetAttr(sdaiInstance, sdaiAttribute, sdaiENUM, &szValue)) {
					CreateAttributeLabelEnumeration(szValue, strLabel);
				}
				else {
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
				else {
					strLabel += L"$";
				}
			}
			break;

		case sdaiINTEGER:
			{
				int_t iValue = 0;
				if (sdaiGetAttr(sdaiInstance, sdaiAttribute, sdaiINTEGER, &iValue)) {
					CreateAttributeLabelInteger(iValue, strLabel);
				}
				else {
					strLabel += L"$";
				}
			}
			break;

		case sdaiSTRING:
			{
				wchar_t* szValue = nullptr;
				if (sdaiGetAttr(sdaiInstance, sdaiAttribute, sdaiUNICODE, &szValue)) {
					CreateAttributeLabelString(szValue, strLabel);
				}
				else {
					strLabel += L"$";
				}
			}
			break;

		default:
			{
				ASSERT(FALSE);
			}
			break;
	} // switch (sdaiAttributeType)

	return bHasChildren;
}

void CRelationsView::GetAttributeReferencesADB(SdaiADB ADB, HTREEITEM hParent)
{
	switch (sdaiGetADBType(ADB)) {
		case sdaiADB:
			{
				SdaiADB attributeDataBlock = 0;
				if (sdaiGetADBValue(ADB, sdaiADB, &attributeDataBlock)) {
					GetAttributeReferencesADB(attributeDataBlock, hParent);
				}
			}
			break;

		case sdaiAGGR:
			{
				SdaiAggr sdaiValueAggr = nullptr;
				SdaiInstance sdaiValueInstance = 0;
				if (sdaiGetADBValue(ADB, sdaiAGGR, &sdaiValueAggr)) {
					GetAttributeReferencesAggregation(sdaiValueAggr, hParent);
				}
				else if (sdaiGetADBValue(ADB, sdaiINSTANCE, &sdaiValueInstance)) {
					LoadInstance(sdaiValueInstance, hParent, NULL);
				}
				else {
					ASSERT(sdaiValueInstance == 0);
					ASSERT(FALSE);
				}
			}
			break;

		case sdaiINSTANCE:
			{
				SdaiInstance sdaiValue = 0;
				if (sdaiGetADBValue(ADB, sdaiINSTANCE, &sdaiValue)) {
					LoadInstance(sdaiValue, hParent, NULL);
				}
				else {
					ASSERT(sdaiValue == 0);
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

void CRelationsView::GetAttributeReferencesAggregationElement(SdaiAggr aggregation, SdaiPrimitiveType sdaiAggrType, SdaiInteger iIndex, HTREEITEM hParent)
{
	switch (sdaiAggrType) {
		case sdaiADB:
			{
				SdaiADB  attributeDataBlock = 0;
				if (sdaiGetAggrByIndex(aggregation, iIndex, sdaiADB, &attributeDataBlock)) {
					GetAttributeReferencesADB(attributeDataBlock, hParent);
				}
			}
			break;

		case sdaiAGGR:
			{
				SdaiAggr sdaValueAggr = nullptr;
				SdaiInstance sdaiValueInstance = 0;
				if (sdaiGetAggrByIndex(aggregation, iIndex, sdaiAGGR, &sdaValueAggr)) {
					GetAttributeReferencesAggregation(sdaValueAggr, hParent);
				}
				else if (sdaiGetAggrByIndex(aggregation, iIndex, sdaiINSTANCE, &sdaiValueInstance)) {
					LoadInstance(sdaiValueInstance, hParent, NULL);
				}
				else {
					ASSERT(sdaiValueInstance == 0);
					ASSERT(FALSE);
				}
			}
			break;

		case sdaiINSTANCE:
			{
				SdaiInstance sdaiValue = 0;
				if (sdaiGetAggrByIndex(aggregation, iIndex, sdaiINSTANCE, &sdaiValue)) {
					LoadInstance(sdaiValue, hParent, NULL);
				}
				else {
					ASSERT(sdaiValue == 0);
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
	} // switch (sdaiAggrType)
}

void CRelationsView::GetAttributeReferencesAggregation(SdaiAggr aggregation, HTREEITEM hParent)
{
	SdaiInteger iMemberCount = sdaiGetMemberCount(aggregation);
	if (iMemberCount == 0) {
		return;
	}

	SdaiPrimitiveType sdaiAggrType = 0;
	engiGetAggrType(aggregation, &sdaiAggrType);

	SdaiInteger iIndex = 0;
	GetAttributeReferencesAggregationElement(aggregation, sdaiAggrType, iIndex++, hParent);

	while (iIndex < iMemberCount) {
		GetAttributeReferencesAggregationElement(aggregation, sdaiAggrType, iIndex++, hParent);
	}
}

void CRelationsView::GetAttributeReferences(SdaiInstance sdaiInstance, SdaiAttr sdaiAttribute, HTREEITEM hParent)
{
	SdaiPrimitiveType sdaiAttrType = engiGetAttrType(sdaiAttribute);
	if (sdaiAttrType & engiTypeFlagAggr ||
		sdaiAttrType & engiTypeFlagAggrOption)
		sdaiAttrType = sdaiAGGR;

	switch (sdaiAttrType) {
		case 0:
			{
			}
			break;

		case sdaiADB:
			{
				SdaiADB attributeDataBlock = 0;
				if (sdaiGetAttr(sdaiInstance, sdaiAttribute, sdaiADB, &attributeDataBlock)) {
					ASSERT(attributeDataBlock != nullptr);

					GetAttributeReferencesADB(attributeDataBlock, hParent);
				}
			}
			break;

		case sdaiAGGR:
			{
				SdaiAggr sdaValueAggr = nullptr;
				SdaiInstance sdaiValueInstance = 0;
				if (sdaiGetAttr(sdaiInstance, sdaiAttribute, sdaiAGGR, &sdaValueAggr)) {
					GetAttributeReferencesAggregation(sdaValueAggr, hParent);
				}
				else if (sdaiGetAttr(sdaiInstance, sdaiAttribute, sdaiINSTANCE, &sdaiValueInstance)) {
					LoadInstance(sdaiValueInstance, hParent, NULL);
				}
				else {
					ASSERT(sdaiValueInstance == 0);
				}
			}
			break;

		case sdaiINSTANCE:
			{
				SdaiInstance sdaiValue = 0;
				if (sdaiGetAttr(sdaiInstance, sdaiAttribute, sdaiINSTANCE, &sdaiValue)) {
					LoadInstance(sdaiValue, hParent, NULL);
				}
				else {
					ASSERT(sdaiValue == 0);
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
	} // switch (sdaiAttrType)
}

void CRelationsView::GetEntityHierarchy(SdaiEntity sdaiEntity, vector<wstring>& vecHierarchy) const
{
	ASSERT(sdaiEntity != 0);

	wstring strEntity = _entity::getName(sdaiEntity);
	if (engiGetEntityIsAbstract(sdaiEntity)) {
		strEntity += L" (ABSTRACT)";
	}

	vecHierarchy.push_back(strEntity);

	SdaiEntity sdaiParent = engiGetEntityParent(sdaiEntity);
	while (sdaiParent != 0) {
		strEntity = _entity::getName(sdaiParent);
		if (engiGetEntityIsAbstract(sdaiParent)) {
			strEntity += L" (ABSTRACT)";
		}

		vecHierarchy.insert(vecHierarchy.begin(), strEntity);

		sdaiParent = engiGetEntityParent(sdaiParent);
	}
}

void CRelationsView::GetAllInstances(_entity* pEntity, map<_entity*, vector<SdaiInstance>>& mapEntityInstances)
{
	for (auto pSubType : pEntity->getSubTypes()) {
		mapEntityInstances[pSubType] = pSubType->getInstances();

		GetAllInstances(pSubType, mapEntityInstances);
	}
}

void CRelationsView::Clean()
{
	for (auto pInstanceData : m_vecItemDataCache) {
		delete pInstanceData;
	}
	m_vecItemDataCache.clear();

	m_mapInstances.clear();
	m_mapInstanceAttributes.clear();
}

void CRelationsView::ResetView()
{
	// UI
	m_treeCtrl.DeleteAllItems();
	m_pSearchDialog->Reset();

	// Data
	Clean();
}

void CRelationsView::OnSelectedItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;

	auto pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	HTREEITEM hItem = pNMTreeView->itemNew.hItem;
	if (hItem == nullptr) {
		return;
	}

	auto pController = getController();
	if (pController == nullptr) {
		ASSERT(FALSE);

		return;
	}

	int iImage, iSelectedImage = -1;
	m_treeCtrl.GetItemImage(hItem, iImage, iSelectedImage);

	ASSERT(iImage == iSelectedImage);

	if (iImage != IMAGE_INSTANCE) {
		return;
	}

	auto pInstanceData = (CInstanceData*)m_treeCtrl.GetItemData(hItem);
	if (pInstanceData == nullptr) {
		ASSERT(FALSE);

		return;
	}

	auto pInstance = pController->loadInstance(pInstanceData->GetSdaiInstance());
	if (pInstance != nullptr) {
		pController->setTargetInstance(this, pInstance);
	}
}

void CRelationsView::OnNMRClickTree(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	*pResult = 0;

	DWORD dwPosition = GetMessagePos();
	CPoint point(LOWORD(dwPosition), HIWORD(dwPosition));
	m_treeCtrl.ScreenToClient(&point);

	UINT uFlags = 0;
	HTREEITEM hItem = m_treeCtrl.HitTest(point, &uFlags);

	if (hItem == nullptr) {
		return;
	}

	m_treeCtrl.SelectItem(hItem);
}

void CRelationsView::OnTVNItemexpandingTree(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	*pResult = 0;

	int iImage, iSelectedImage = -1;
	m_treeCtrl.GetItemImage(pNMTreeView->itemNew.hItem, iImage, iSelectedImage);

	ASSERT(iImage == iSelectedImage);

	if (((iImage == IMAGE_INSTANCE) || (iImage == IMAGE_PENDING_LOAD)) &&
		(pNMTreeView->itemNew.cChildren == 1)) {
		HTREEITEM hChild = m_treeCtrl.GetChildItem(pNMTreeView->itemNew.hItem);
		if (hChild == NULL) {
			ASSERT(FALSE);

			return;
		}

		if (m_treeCtrl.GetItemText(hChild) != ITEM_PENDING_LOAD) {
			return;
		}

		if (iImage == IMAGE_INSTANCE) {
			auto pAttributeData = (CAttributeData*)m_treeCtrl.GetItemData(hChild);
			ASSERT(pAttributeData != nullptr);

			m_treeCtrl.DeleteItem(hChild);

			GetAttributeReferences(
				pAttributeData->GetSdaiInstance(),
				sdaiGetAttrDefinition(sdaiGetInstanceType(pAttributeData->GetSdaiInstance()), pAttributeData->GetName()),
				pNMTreeView->itemNew.hItem);
		}
		else if (iImage == IMAGE_PENDING_LOAD) {
			auto pInstanceSet = dynamic_cast<CInstanceSet*>((CItemData*)m_treeCtrl.GetItemData(hChild));
			if (pInstanceSet != nullptr) {
				ASSERT(!pInstanceSet->Instances().empty());

				HTREEITEM hInsertAfter = m_treeCtrl.GetPrevVisibleItem(pNMTreeView->itemNew.hItem);
				ASSERT(hInsertAfter != NULL);

				LoadInstances(pInstanceSet->Instances(), false, hInsertAfter);
			} // if (pInstanceSet != nullptr)
			else {
				auto pAttributeSet = dynamic_cast<CAttributeSet*>((CItemData*)m_treeCtrl.GetItemData(hChild));
				if (pAttributeSet != nullptr) {
					HTREEITEM hParent = m_treeCtrl.GetParentItem(pNMTreeView->itemNew.hItem);
					ASSERT(hParent != NULL);

					for (size_t iAttribute = 0; (iAttribute < pAttributeSet->Attributes().size()); iAttribute++) {

						LoadInstanceAttribute(
							pAttributeSet->GetSdaiEntity(),
							pAttributeSet->GetSdaiInstance(),
							pAttributeSet->Attributes()[iAttribute],
							hParent,
							pNMTreeView->itemNew.hItem);
					}
				} // if (pAttributeSet != nullptr)
				else {
					ASSERT(FALSE); // Internal error!
				}
			} // else if (pInstanceSet != nullptr)

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

	if ((iImage == IMAGE_ATTRIBUTE) || (iImage == IMAGE_INVERSE_ATTRIBUTE)) {
		auto pAttributeData = (CAttributeData*)m_treeCtrl.GetItemData(pNMTVGetInfoTip->hItem);
		if (pAttributeData != nullptr) {
			m_strTooltip = CA2W(pAttributeData->GetName());
			if (engiGetAttrInverseBN(pAttributeData->GetSdaiEntity(), pAttributeData->GetName())) {
				m_strTooltip += L" (INVERSE)";
			}

			m_strTooltip += L"\n";

			m_strTooltip += _entity::getName(pAttributeData->GetSdaiEntity());
			if (engiGetEntityIsAbstract(pAttributeData->GetSdaiEntity())) {
				m_strTooltip += L" (ABSTRACT)";
			}
		} // if (pAttributeData != nullptr)
		else {
			ASSERT(FALSE); // Internal error!
		}
	} // if ((iImage == IMAGE_ATTRIBUTE) || ...
	else if (iImage == IMAGE_INSTANCE) {
		auto pInstanceData = (CInstanceData*)m_treeCtrl.GetItemData(pNMTVGetInfoTip->hItem);
		if (pInstanceData != nullptr) {
			vector<wstring> vecHierarchy;
			GetEntityHierarchy(pInstanceData->GetSdaiEntity(), vecHierarchy);

			m_strTooltip = L"";
			for (auto strEntity : vecHierarchy) {
				if (!m_strTooltip.empty()) {
					m_strTooltip += L"\n";
				}

				m_strTooltip += strEntity;
			}
		} // if (pAttributeData != nullptr)
		else {
			ASSERT(FALSE); // Internal error!
		}
	} // if (iImage == IMAGE_INSTANCE)

	pNMTVGetInfoTip->pszText = (LPWSTR)m_strTooltip.c_str();
}

CRelationsView::CRelationsView()
	: m_pEntity(nullptr)
	, m_enMode(enumRelationsViewMode::Flat)
	, m_vecItemDataCache()
	, m_mapInstances()
	, m_mapInstanceAttributes()
	, m_pSearchDialog(nullptr)
	, m_strTooltip(L"")
{
	// State provider
	m_treeCtrl.SetItemStateProvider(this);
}

CRelationsView::~CRelationsView()
{
	Clean();

	m_treeCtrl.SetItemStateProvider(nullptr);
}

BEGIN_MESSAGE_MAP(CRelationsView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_PROPERTIES, OnProperties)
	ON_WM_CONTEXTMENU()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_IFC, OnSelectedItemChanged)
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
	if (GetSafeHwnd() == nullptr) {
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
	if (!m_pSearchDialog->IsWindowVisible()) {
		m_pSearchDialog->ShowWindow(SW_SHOW);
	}
	else {
		m_pSearchDialog->ShowWindow(SW_HIDE);
	}
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
	if (!bmp.LoadBitmap(uiBmpId)) {
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
	getController()->unRegisterView(this);

	delete m_pSearchDialog;
	m_pSearchDialog = nullptr;

	__super::OnDestroy();
}

void CRelationsView::OnShowWindow(BOOL bShow, UINT nStatus)
{
	__super::OnShowWindow(bShow, nStatus);

	if (!bShow &&
		(m_pSearchDialog != nullptr) &&
		(m_pSearchDialog->GetSafeHwnd() != NULL)) {
		m_pSearchDialog->ShowWindow(SW_HIDE);
	}
}
