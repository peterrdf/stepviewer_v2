
#include "stdafx.h"
#include "mainfrm.h"
#include "RelationsView.h"
#include "Resource.h"
#include "STEPViewer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define IMAGE_MODEL 				2
#define IMAGE_INSTANCE				0
#define IMAGE_ENTITY				1
#define IMAGE_SUB_TYPES				2
#define IMAGE_ATTRIBUTES			2
#define IMAGE_INVERSE_ATTRIBUTE		3
#define IMAGE_ATTRIBUTE				5

#define ITEM_SUB_TYPES	  L"Sub-types"
#define ITEM_ATTRIBUTES	  L"Attributes"
#define ITEM_PENDING_LOAD L"***..........***"

///////////////////////////////////////////////////////////////////////////////////////////////////
// CRelationsView


// ------------------------------------------------------------------------------------------------
/*virtual*/ void CRelationsView::OnModelChanged() /*override*/
{
	LoadProperties(0, nullptr, vector<int_t>());
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CRelationsView::OnInstanceSelected(CSTEPView* pSender) /*override*/
{
	if (pSender == this)
	{
		return;
	}

	auto pController = GetController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	vector<int_t> vecInstances;

	auto pInstance = GetController()->GetSelectedInstance();
	if (pInstance != nullptr)
	{
		vecInstances.push_back(pInstance->GetInstance());
	}
	
	LoadInstances(vecInstances);
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CRelationsView::OnViewRelations(CSTEPView* pSender, int64_t iInstance) /*override*/
{
	if (pSender == this)
	{
		return;
	}

	if (iInstance == 0)
	{
		ASSERT(FALSE);

		return;
	}

	vector<int_t> vecInstances;
	vecInstances.push_back(iInstance);

	LoadProperties(
		CInstance::GetEntity(iInstance),
		CInstance::GetEntityName(iInstance),
		vecInstances);

	ShowPane(TRUE, TRUE, TRUE);
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CRelationsView::OnViewRelations(CSTEPView* pSender, CEntity* pEntity) /*override*/
{
	if (pSender == this)
	{
		return;
	}
	
	LoadProperties(
		pEntity->GetEntity(),
		pEntity->GetName(),
		pEntity->GetInstances());
	
	ShowPane(TRUE, TRUE, TRUE);
}

// ------------------------------------------------------------------------------------------------
CModel* CRelationsView::GetModel() const
{
	auto pController = GetController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return nullptr;
	}

	auto pModel = pController->GetModel();
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return nullptr;
	}

	return pModel;
}

// ------------------------------------------------------------------------------------------------
void CRelationsView::LoadInstances(const vector<int_t>& vecInstances)
{
	m_treeCtrl.DeleteAllItems();
	Clean();

	auto pModel = GetModel();
	if (pModel == nullptr)
	{
		return;
	}

	// ******************************************************************************************** //
	// Model
	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = nullptr;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = (LPWSTR)pModel->GetModelName();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_MODEL;
	tvInsertStruct.item.lParam = NULL;

	HTREEITEM hModel = m_treeCtrl.InsertItem(&tvInsertStruct);
	// ******************************************************************************************** //	

	// ******************************************************************************************** //
	// Instances
	for (auto iInstance : vecInstances)
	{
		LoadInstance(
			CInstance::GetEntity(iInstance), 
			CInstance::GetEntityName(iInstance),
			iInstance, 
			hModel);
	}
	// ******************************************************************************************** //

	m_treeCtrl.Expand(hModel, TVE_EXPAND);
}

// ------------------------------------------------------------------------------------------------
void CRelationsView::LoadProperties(int_t iEntity, const wchar_t* szEntity, const vector<int_t>& vecInstances)
{
	m_treeCtrl.DeleteAllItems();
	Clean();

	auto pModel = GetModel();
	if (pModel == nullptr)
	{
		return;
	}

	// ******************************************************************************************** //
	// Model
	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = nullptr;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = (LPWSTR)pModel->GetModelName();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_MODEL;
	tvInsertStruct.item.lParam = NULL;

	HTREEITEM hModel = m_treeCtrl.InsertItem(&tvInsertStruct);
	m_treeCtrl.SetItemState(hModel, TVIS_BOLD, TVIS_BOLD);
	// ******************************************************************************************** //	

	// ******************************************************************************************** //
	// Instances
	for (auto iInstance : vecInstances)
	{
		LoadInstance(iEntity, szEntity, iInstance, hModel);
	}
	// ******************************************************************************************** //

	m_treeCtrl.Expand(hModel, TVE_EXPAND);
}

// ------------------------------------------------------------------------------------------------
void CRelationsView::LoadInstance(int_t iEntity, const wchar_t* szEntity, int_t iInstance, HTREEITEM hParent)
{	
	ASSERT(iEntity != 0);
	ASSERT(iInstance != 0);

	wchar_t* szName = nullptr;
	sdaiGetAttrBN(iInstance, "Name", sdaiUNICODE, &szName);

	wchar_t* szDescription = nullptr;
	sdaiGetAttrBN(iInstance, "Description", sdaiUNICODE, &szDescription);

	int_t iValue = internalGetP21Line(iInstance);

	CString strPrefix;
	strPrefix.Format(_T("# %lld ="), iValue);
	
	wstring strItem = strPrefix;
	strItem += L" ";
	strItem += szEntity;
	if ((szName != nullptr) && (wcslen(szName) > 0))
	{
		strItem += L" '";
		strItem += szName;
		strItem += L"'";
	}

	if ((szDescription != nullptr) && (wcslen(szDescription) > 0))
	{
		strItem += L" (";
		strItem += szDescription;
		strItem += L")";
	}

	/*
	* Data
	*/
	auto pInstanceData = new CInstanceData(iInstance, iEntity);
	m_vecItemDataCache.push_back(pInstanceData);

	/*
	* Instance
	*/
	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = hParent;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_INSTANCE;
	tvInsertStruct.item.lParam = (LPARAM)pInstanceData;

	HTREEITEM hInstance = m_treeCtrl.InsertItem(&tvInsertStruct);	

	LoadInstanceAttributes(CInstance::GetEntity(iInstance), iInstance, hInstance);
}

// ------------------------------------------------------------------------------------------------
int_t CRelationsView::LoadInstanceAttributes(int_t iEntity, int_t iInstance, HTREEITEM hParent)
{
	if (iEntity == 0)
	{
		return 0;
	}

	ASSERT(iInstance != 0);

	int_t iAttrubutesCount = LoadInstanceAttributes(engiGetEntityParent(iEntity), iInstance, hParent);

	HTREEITEM hAttributesParent = NULL;
	switch (m_enMode)
	{
		case enumRelationsViewMode::Hierarchy:
		{
			TV_INSERTSTRUCT tvInsertStruct;
			tvInsertStruct.hParent = hParent;
			tvInsertStruct.hInsertAfter = TVI_FIRST;
			tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
			tvInsertStruct.item.pszText = (LPWSTR)CEntity::GetName(iEntity);
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

    SdaiInteger index = 0;
    SdaiAttr    iAttribute = engiGetEntityAttributeByIndex(
                                    iEntity,
                                    index++,
                                    false,
                                    true
                                );
    while (iAttribute) {
		const char	* attributeName = nullptr;
		engiGetEntityArgumentName(iEntity, iAttrubutesCount++, sdaiSTRING, &attributeName);

		AddInstanceAttribute(iEntity, iInstance, iAttribute, attributeName, hAttributesParent);
        iAttribute = engiGetEntityAttributeByIndex(
							iEntity,
							index++,
							false,
							true
						);
    }

/*	...

	while (iAttrubutesCount < engiGetEntityNoAttributes(iEntity))
	{
		char* szAttributeName = 0;
		engiGetEntityArgumentName(iEntity, iAttrubutesCount, sdaiSTRING, (const char**)&szAttributeName);		

		int_t iAttributeType = engiGetAttrTypeBN(iInstance, (char*)szAttributeName);
		if (iAttributeType & 128)
		{
			iAttributeType = sdaiAGGR;

			LoadAGGR(iInstance, iEntity, szAttributeName, iAttributeType, hAttributesParent);
		} 
		else if (iAttributeType == sdaiADB)
		{
			int_t ADB = 0;
			sdaiGetAttrBN(iInstance, (char*)szAttributeName, sdaiADB, &ADB);

			if (ADB == 0)
			{
				ASSERT(FALSE);

				iAttributeType = sdaiINSTANCE;
			}

			LoadADB(&ADB, iEntity, szAttributeName, iAttributeType, hAttributesParent);
		}
		else
		{
			AddInstanceAttribute(iInstance, iEntity, szAttributeName, iAttributeType, hAttributesParent);
		}

		iAttrubutesCount++;
	} // while (iAttrubutesCount < ..	//	*/

	return iAttrubutesCount;
}

// ------------------------------------------------------------------------------------------------
void CRelationsView::LoadADB(SdaiADB ADB, int_t iEntity, const char* szAttributeName, int_t iAttributeType, HTREEITEM hParent)
{
	ADB = ADB;
	iEntity = iEntity;
	char szAttributeN = szAttributeName[0];
	szAttributeN = szAttributeN;
	iAttributeType = iAttributeType;
	hParent = hParent;
//	ASSERT(0);//todo

	//switch (sdaiGetADBType(ADB))
	//{
	//	case  sdaiADB:
	//	{
	//		SdaiADB attributeDataBlock = 0;
	//		if (sdaiGetADBValue(ADB, sdaiADB, &attributeDataBlock)) 
	//		{
	//			ASSERT(FALSE);
	//			//CreateDetailedADB___cout(attributeDataBlock);
	//		}
	//		else 
	//		{
	//			ASSERT(FALSE);
	//		}
	//	}
	//	break;

	//	case  sdaiAGGR:
	//	{
	//		ASSERT(FALSE); //todo
	//		/*SdaiAggr        valueAggr = nullptr;
	//		SdaiInstance    valueInstance = 0;
	//		if (sdaiGetADBValue(ADB, sdaiAGGR, &valueAggr)) {
	//			cout << "(";
	//			CreateDetailedAggregation___cout(valueAggr);
	//			cout << ")";
	//		}
	//		else if (sdaiGetADBValue(ADB, sdaiINSTANCE, &valueInstance)) {
	//			CreateInstance___cout(valueInstance);
	//		}
	//		else {
	//			assert(false);
	//		}*/
	//	}
	//	break;

	//	default:
	//	{
	//		AddInstanceAttribute(iInstance, iEntity, szAttributeName, iAttributeType, hParent);
	//	}
	//	break;
	//} // switch (sdaiGetADBType(ADB))
}

// ------------------------------------------------------------------------------------------------
/*void CRelationsView::LoadAGGR(int_t iInstance, int_t iEntity, const char* szAttributeName, int_t iAttributeType, HTREEITEM hParent)
{
	switch (iAttributeType)
	{
		case sdaiADB:
		{
			ASSERT(0);//todo
			//int_t ADB = 0;
			//sdaiGetAttrBN(iInstance, (char*)szAttributeName, sdaiADB, &ADB);

			//if (ADB != 0)
			//{
			//	wchar_t* szPath = (wchar_t*)sdaiGetADBTypePath((void*)ADB, sdaiUNICODE);
			//	strLabel += szPath;

			//	strLabel += L"(";
			//	CreateAttributeLabelADB(ADB, strLabel, bHasChildren);
			//	strLabel += L")";
			//} // if (ADB != 0) 
			//else
			//{
			//	strLabel += L"NA";
			//}
		} // case sdaiADB:
		break;

		case sdaiAGGR:
		{
			int_t* pValues = nullptr;
			sdaiGetAttrBN(iInstance, (char*)szAttributeName, sdaiAGGR, &pValues);

			if (pValues != nullptr)
			{
				int_t iCount = sdaiGetMemberCount(pValues);
				for (int_t iIndex = 0; iIndex < iCount; iIndex++)
				{
					SdaiAggr sdaiAggr = nullptr;
					SdaiInstance sdaiInstance = 0;

					if (engiGetAggrElement(pValues, iIndex, sdaiAGGR, &sdaiAggr))
					{
						ASSERT(FALSE);
						/*cout << "(";
						CreateDetailedAggregation___cout(valueAggr);
						cout << ")";* /
					}
					else if (engiGetAggrElement(pValues, iIndex, sdaiINSTANCE, &sdaiInstance))
					{
						ASSERT(FALSE);
						//CreateInstance___cout(valueInstance);
					}
					else 
					{
						//AddInstanceAttribute(pValues[iIndex], iEntity, szAttributeName, iAttributeType, hParent);
					}
				} // for (int_t iIndex = ...
			} // if (pValues != nullptr) 
			else
			{
				ASSERT(FALSE);
			}
		} // case sdaiAGGR:
		break;		

		default:
		{
			AddInstanceAttribute(iInstance, iEntity, szAttributeName, iAttributeType, hParent);
		}
		break;
	} // switch (iAttributeType)
}	//	*/

// ------------------------------------------------------------------------------------------------
void CRelationsView::AddInstanceAttribute(SdaiEntity iEntity, SdaiInstance iInstance, SdaiAttr iAttribute, const char * szAttributeName, HTREEITEM hParent)
{
	wstring strLabel;
	bool bInverse = false;

	bool bHasChildren = CreateAttributeLabel(iInstance, iAttribute, strLabel);
	if (!bHasChildren)
	{
		bInverse = engiGetAttrInverseBN(iEntity, szAttributeName) != 0;
	}

	wstring strAttribute = CA2W(szAttributeName);
	strAttribute += L" = ";
//	strAttribute += strLabel.empty() || (strLabel == L"()") || (strLabel == L"''") ? 
//		L"NA" : strLabel;
	strAttribute += strLabel.empty() ? L"$" : strLabel;

	auto pAttributeData = new CAttributeData(iInstance, iEntity, szAttributeName, 2); //, iAttributeType);
	m_vecItemDataCache.push_back(pAttributeData);

	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = hParent;
	tvInsertStruct.hInsertAfter = TVI_LAST;
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

void CRelationsView::CreateAttributeLabelInstance(SdaiInstance instance, wstring& strLabel)
{
    assert(instance);

	CString strValue;
	strValue.Format(_T("%lld"), internalGetP21Line(instance));

	strLabel += L"#";
	strLabel += strValue;
}

void CRelationsView::CreateAttributeLabelBoolean(bool value, wstring& strLabel)
{
    if (value)
		strLabel += L".T.";
    else
		strLabel += L".F.";
}

void CRelationsView::CreateAttributeLabelLogical(char * value, wstring& strLabel)
{
	strLabel += L".";
	strLabel += CA2W(value);
	strLabel += L".";
}

void CRelationsView::CreateAttributeLabelEnumeration(char * value, wstring& strLabel)
{
	strLabel += L".";
	strLabel += CA2W(value);
	strLabel += L".";
}

void CRelationsView::CreateAttributeLabelReal(double value, wstring& strLabel)
{
	CString strValue;
	strValue.Format(_T("%f"), value);

	strLabel += strValue;
}

void CRelationsView::CreateAttributeLabelInteger(int_t value, wstring& strLabel)
{
	CString strValue;
	strValue.Format(_T("%lld"), value);

	strLabel += strValue;
}

void CRelationsView::CreateAttributeLabelString(wchar_t * value, wstring& strLabel)
{
	if (value) {
		strLabel += L"'";
		strLabel += value;
		strLabel += L"'";
	}
    else
		strLabel += L"''";
}

// ------------------------------------------------------------------------------------------------
bool CRelationsView::CreateAttributeLabelADB(SdaiADB ADB, wstring& strLabel)
{
	bool bHasChildren = false;

	strLabel += (const wchar_t*) sdaiGetADBTypePath(ADB, sdaiUNICODE);

    switch (sdaiGetADBType(ADB)) {
        case  sdaiADB:
            {
                SdaiADB    attributeDataBlock = 0;
                if (sdaiGetADBValue(ADB, sdaiADB, &attributeDataBlock)) {
					bHasChildren |= CreateAttributeLabelADB(attributeDataBlock, strLabel);
                }
                else {
                    assert(false);
					strLabel += L"$";
                }
            }
            break;
        case  sdaiAGGR:
            {
                SdaiAggr        valueAggr = nullptr;
                SdaiInstance    valueInstance = 0;
                if (sdaiGetADBValue(ADB, sdaiAGGR, &valueAggr)) {
					strLabel += L"(";
					bHasChildren |= CreateAttributeLabelAggregation(valueAggr, strLabel);
					strLabel += L")";
                }
                else if (sdaiGetADBValue(ADB, sdaiINSTANCE, &valueInstance) || valueInstance) {
					CreateAttributeLabelInstance(valueInstance, strLabel);
					bHasChildren = true;
                }
                else {
                    assert(false);
                }
            }
            break;
        case  sdaiINSTANCE:
            {
                SdaiInstance    value = 0;
                if (sdaiGetADBValue(ADB, sdaiINSTANCE, &value) || value) {
					CreateAttributeLabelInstance(value, strLabel);
					bHasChildren = true;
                }
                else {
                    assert(false);
					strLabel += L"$";
                }
            }
            break;
        case  sdaiBOOLEAN:
            {
                bool    value = false;
                if (sdaiGetADBValue(ADB, sdaiBOOLEAN, &value)) {
					CreateAttributeLabelBoolean(value, strLabel);
                }
                else {
                    assert(false);
					strLabel += L"$";
                }
            }
            break;
        case  sdaiLOGICAL:
            {
                char    * value = nullptr;
                if (sdaiGetADBValue(ADB, sdaiLOGICAL, &value)) {
					CreateAttributeLabelLogical(value, strLabel);
                }
                else {
                    assert(false);
					strLabel += L"$";
                }
            }
            break;
        case  sdaiENUM:
            {
                char    * value = nullptr;
                if (sdaiGetADBValue(ADB, sdaiENUM, &value)) {
					CreateAttributeLabelEnumeration(value, strLabel);
                }
                else {
                    assert(false);
					strLabel += L"$";
                }
            }
            break;
        case  sdaiREAL:
            {
                double  value = 0.;
                if (sdaiGetADBValue(ADB, sdaiREAL, &value)) {
					CreateAttributeLabelReal(value, strLabel);
                }
                else {
                    assert(false);
					strLabel += L"$";
                }
            }
            break;
        case  sdaiINTEGER:
            {
                int_t   value = 0;
                if (sdaiGetADBValue(ADB, sdaiINTEGER, &value)) {
					CreateAttributeLabelInteger(value, strLabel);
                }
                else {
                    assert(false);
					strLabel += L"$";
                }
            }
            break;
        case  sdaiSTRING:
            {
                wchar_t	* value = nullptr;
                if (sdaiGetADBValue(ADB, sdaiUNICODE, &value)) {
					CreateAttributeLabelString(value, strLabel);
                }
                else {
                    assert(false);
					strLabel += L"$";
                }
            }
            break;
        default:
            assert(false);
            break;
    }

	strLabel += L")";

	return	bHasChildren;
}

// ------------------------------------------------------------------------------------------------
bool CRelationsView::CreateAttributeLabelAggregationElement(SdaiAggr aggregation, int_t aggrType, SdaiInteger index, wstring& strLabel)
{
	bool bHasChildren = false;

    switch (aggrType) {
        case  sdaiADB:
        {
            SdaiADB    attributeDataBlock = 0;
            if (engiGetAggrElement(aggregation, index, sdaiADB, &attributeDataBlock)) {
				bHasChildren |= CreateAttributeLabelADB(attributeDataBlock, strLabel);
            }
            else {
                assert(false);
				strLabel += L"$";
            }
        }
        break;

        case  sdaiAGGR:
        {
            SdaiAggr        valueAggr = nullptr;
            SdaiInstance    valueInstance = 0;
            if (engiGetAggrElement(aggregation, index, sdaiAGGR, &valueAggr)) {
				strLabel += L"(";
				bHasChildren |= CreateAttributeLabelAggregation(valueAggr, strLabel);
				strLabel += L")";
            }
            else if (engiGetAggrElement(aggregation, index, sdaiINSTANCE, &valueInstance) || valueInstance) {
				CreateAttributeLabelInstance(valueInstance, strLabel);
				bHasChildren = true;
			}
            else {
                assert(false);
            }
        }
        break;

		case  sdaiINSTANCE:
        {
            SdaiInstance    value = 0;
            if (engiGetAggrElement(aggregation, index, sdaiINSTANCE, &value) || value) {
				CreateAttributeLabelInstance(value, strLabel);
				bHasChildren = true;
			}
			else {
				assert(false);
				strLabel += L"$";
			}
		}
        break;

		case  sdaiBOOLEAN:
        {
            bool    value = false;
            if (engiGetAggrElement(aggregation, index, sdaiBOOLEAN, &value)) {
				CreateAttributeLabelBoolean(value, strLabel);
            }
            else {
                assert(false);
				strLabel += L"$";
            }
        }
        break;

		case  sdaiLOGICAL:
        {
            char    * value = nullptr;
            if (engiGetAggrElement(aggregation, index, sdaiLOGICAL, &value)) {
				CreateAttributeLabelLogical(value, strLabel);
            }
            else {
                assert(false);
				strLabel += L"$";
            }
        }
        break;

        case  sdaiENUM:
        {
            char    * value = nullptr;
            if (engiGetAggrElement(aggregation, index, sdaiENUM, &value)) {
				CreateAttributeLabelEnumeration(value, strLabel);
            }
            else {
                assert(false);
				strLabel += L"$";
            }
        }
        break;

        case  sdaiREAL:
        {
            double  value = 0.;
            if (engiGetAggrElement(aggregation, index, sdaiREAL, &value)) {
				CreateAttributeLabelReal(value, strLabel);
            }
            else {
                assert(false);
				strLabel += L"$";
            }
        }
        break;

        case  sdaiINTEGER:
        {
            int_t   value = 0;
            if (engiGetAggrElement(aggregation, index, sdaiINTEGER, &value)) {
				CreateAttributeLabelInteger(value, strLabel);
            }
            else {
                assert(false);
				strLabel += L"$";
            }
        }
        break;

        case  sdaiSTRING:
        {
            wchar_t	* value = nullptr;
            if (engiGetAggrElement(aggregation, index, sdaiUNICODE, &value)) {
				CreateAttributeLabelString(value, strLabel);
            }
            else {
                assert(false);
				strLabel += L"$";
            }
        }
        break;

        default:
        assert(false);
        break;
    }

	return	bHasChildren;
}

// ------------------------------------------------------------------------------------------------
bool CRelationsView::CreateAttributeLabelAggregation(SdaiAggr aggregation, wstring& strLabel)
{
	bool bHasChildren = false;

    SdaiInteger memberCount = sdaiGetMemberCount(aggregation);
    if (memberCount == 0)
        return  bHasChildren;

    int_t   aggrType = 0;
    engiGetAggrType(aggregation, &aggrType);

    SdaiInteger index = 0;
	bHasChildren |= CreateAttributeLabelAggregationElement(aggregation, aggrType, index++, strLabel);
    while (index < memberCount) {
		strLabel += L", ";
		bHasChildren |= CreateAttributeLabelAggregationElement(aggregation, aggrType, index++, strLabel);
    }

	return	bHasChildren;
}

// ------------------------------------------------------------------------------------------------
bool CRelationsView::CreateAttributeLabel(SdaiInstance iInstance, SdaiAttr iAttribute, wstring& strLabel)
{	
	strLabel = _T("");
	bool bHasChildren = false;

    int_t   iAttributeType = engiGetAttrType(iAttribute);
    if (iAttributeType & engiTypeFlagAggr ||
		iAttributeType & engiTypeFlagAggrOption)
		iAttributeType = sdaiAGGR;

	switch (iAttributeType)
	{
        case  0:
		strLabel += L"$";
        break;

		case  sdaiADB:
		{
			SdaiADB    attributeDataBlock = 0;
			if (sdaiGetAttr(iInstance, iAttribute, sdaiADB, &attributeDataBlock)) {
				assert(attributeDataBlock);
				bHasChildren |= CreateAttributeLabelADB(attributeDataBlock, strLabel);
			}
			else {
				assert(false);
				strLabel += L"$";
			}
		}
		break;

		case  sdaiAGGR:
		{
			SdaiAggr        valueAggr = nullptr;
			SdaiInstance    valueInstance = 0;
			if (sdaiGetAttr(iInstance, iAttribute, sdaiAGGR, &valueAggr)) {
				strLabel += L"(";
				bHasChildren |= CreateAttributeLabelAggregation(valueAggr, strLabel);
				strLabel += L")";
			}
			else if (sdaiGetAttr(iInstance, iAttribute, sdaiINSTANCE, &valueInstance) || valueInstance) {
				CreateAttributeLabelInstance(valueInstance, strLabel);
				bHasChildren = true;
			}
			else {
				strLabel += L"$";
			}
		}
		break;

        case  sdaiINSTANCE:
        {
            SdaiInstance    value = 0;
            if (sdaiGetAttr(iInstance, iAttribute, sdaiINSTANCE, &value) || value) {
				CreateAttributeLabelInstance(value, strLabel);
				bHasChildren = true;
			}
            else {
                if (engiGetAttrDerived(sdaiGetInstanceType(iInstance), iAttribute)) {
					strLabel += L"*";
                }
                else {
					strLabel += L"$";
                }
            }
        }
        break;

        case  sdaiBOOLEAN:
        {
            bool    value = false;
            if (sdaiGetAttr(iInstance, iAttribute, sdaiBOOLEAN, &value)) {
				CreateAttributeLabelBoolean(value, strLabel);
            }
            else {
				strLabel += L"$";
            }
        }
        break;

        case  sdaiLOGICAL:
        {
            char    * value = nullptr;
            if (sdaiGetAttr(iInstance, iAttribute, sdaiLOGICAL, &value)) {
				CreateAttributeLabelLogical(value, strLabel);
            }
            else {
				strLabel += L"$";
            }
        }
        break;

		case  sdaiENUM:
        {
            char    * value = nullptr;
            if (sdaiGetAttr(iInstance, iAttribute, sdaiENUM, &value)) {
				CreateAttributeLabelEnumeration(value, strLabel);
            }
            else {
				strLabel += L"$";
            }
        }
        break;

        case  sdaiREAL:
        {
            double  value = 0.;
            if (sdaiGetAttr(iInstance, iAttribute, sdaiREAL, &value)) {
				CreateAttributeLabelReal(value, strLabel);
            }
            else {
				strLabel += L"$";
            }
        }
        break;

        case  sdaiINTEGER:
        {
            int_t   value = 0;
            if (sdaiGetAttr(iInstance, iAttribute, sdaiINTEGER, &value)) {
				CreateAttributeLabelInteger(value, strLabel);
            }
            else {
				strLabel += L"$";
            }
        }
        break;

		case  sdaiSTRING:
        {
            wchar_t	* value = nullptr;
            if (sdaiGetAttr(iInstance, iAttribute, sdaiUNICODE, &value)) {
				CreateAttributeLabelString(value, strLabel);
            }
            else {
				strLabel += L"$";
            }
        }
        break;

		default:
        assert(false);
        break;
	}

	return bHasChildren;
}

/* ------------------------------------------------------------------------------------------------
void CRelationsView::CreateAttributeLabelADB(int_t ADB, wstring& strLabel, bool& bHasChildren)
{
	switch (sdaiGetADBType((void *)ADB)) 
	{
		case sdaiADB:
		{
			int_t ADBsub = 0;
			sdaiGetADBValue((void *)ADB, sdaiADB, &ADBsub);

			if (ADBsub != 0) 
			{
				wchar_t* szPath = (wchar_t *)sdaiGetADBTypePath((void*)ADBsub, sdaiUNICODE);
				strLabel += szPath;

				strLabel += L"(";
				CreateAttributeLabelADB(ADB, strLabel, bHasChildren);
				strLabel += L")";
			}
			else 
			{
				strLabel += L"NA";
			}
		} // case sdaiADB:
		break;

		case sdaiAGGR:
		{
			int_t * pValue = 0;
			sdaiGetADBValue((void *)ADB, sdaiAGGR, &pValue);

			if (pValue != nullptr)
			{
				strLabel += L"(";
				
				int_t i = 0;
				int_t iCount = sdaiGetMemberCount(pValue);
				
				while (i < iCount) 
				{
					if (i != 0) 
					{
						strLabel += L", ";
					}

					strLabel += L"???";
					i++;
				}

				strLabel += L")";
			}
			else 
			{
				strLabel += L"NA";
			}
		} // case sdaiAGGR:
		break;

		case sdaiBOOLEAN:
		case sdaiENUM:
		case sdaiLOGICAL:
		{
			wchar_t* szValue = 0;
			sdaiGetADBValue((void *)ADB, sdaiUNICODE, (char**)&szValue);

			if (szValue != nullptr)
			{
				strLabel += szValue;
			}
			else
			{
				strLabel += L"NA";
			}
		} // case sdaiBOOLEAN:
		break;

		case sdaiINSTANCE:
		{
			int_t iAttributeInstance = 0;
			sdaiGetADBValue((void *)ADB, sdaiINSTANCE, &iAttributeInstance);

			if (iAttributeInstance != 0)
			{
				strLabel += L"#";

				int_t iValue = internalGetP21Line(iAttributeInstance);

				CString strValue;
				strValue.Format(_T("%lld"), iValue);

				strLabel += strValue;

				bHasChildren = true;
			}
			else 
			{
				strLabel += L"NA";
			}
		} // case sdaiINSTANCE:
		break;

		case sdaiINTEGER:
		{
			int_t iValue = 0;
			sdaiGetADBValue((void *)ADB, sdaiINTEGER, &iValue);

			CString strValue;
			strValue.Format(_T("%lld"), iValue);

			strLabel += strValue;
		} // case sdaiINTEGER:
		break;

		case sdaiSTRING:
		{
			wchar_t	* szValue = 0;
			sdaiGetADBValue((void *)ADB, sdaiUNICODE, (char **)&szValue);

			if (szValue != nullptr)
			{
				strLabel += L"'";
				strLabel += szValue;
				strLabel += L"'";
			}
			else
			{
				strLabel += L"NA";
			}
		} // case sdaiSTRING:
		break;

		case sdaiREAL:
		{
			double dValue = 0;
			sdaiGetADBValue((void*)ADB, sdaiREAL, &dValue);

			CString strValue;
			strValue.Format(_T("%f"), dValue);

			strLabel += strValue;
		} // case sdaiREAL:
		break;

		default:
			ASSERT(FALSE);
			break;
	} // switch (sdaiGetADBType((void *)ADB)) 
}	//	*/

/* ------------------------------------------------------------------------------------------------
void CRelationsView::CreateAttributeLabelAGGR(int_t* pAggregate, int_t iElementIndex, wstring& strLabel, bool& bHasChildren)
{
	int_t iAggregateType = 0;
	engiGetAggrType(pAggregate, &iAggregateType);

	switch (iAggregateType) 
	{
		case sdaiADB:
		{
			int_t ADB = 0;
			engiGetAggrElement(pAggregate, iElementIndex, sdaiADB, &ADB);

			if (ADB != 0) 
			{
				wchar_t	* szPath = (wchar_t *)sdaiGetADBTypePath((void*)ADB, sdaiUNICODE);
				strLabel += szPath;

				strLabel += L"(";
				CreateAttributeLabelADB(ADB, strLabel, bHasChildren);
				strLabel += L")";
			}
			else 
			{
				strLabel += L"NA";
			}
		} // case sdaiADB:
		break;

		case sdaiAGGR:
		{
			int_t* pValue = 0;
			engiGetAggrElement(pAggregate, iElementIndex, sdaiAGGR, &pValue);

			if (pValue != nullptr)
			{
				strLabel += L"(";

				int_t i = 0;
				int_t iCount = sdaiGetMemberCount(pValue);

				while ((i < iCount)  &&  (i < 8)) 
				{
					if (i != 0) 
					{
						strLabel += L", ";
					}

					CreateAttributeLabelAGGR(pValue, i, strLabel, bHasChildren);

					i++;
				}

				if (i < iCount)
				{
					strLabel += L", ...";
				}

				strLabel += L")";
			}
			else 
			{
				strLabel += L"NA";
			}
		} // case sdaiAGGR:
		break;

		case sdaiBOOLEAN:
		case sdaiENUM:
		case sdaiLOGICAL:
		{
			wchar_t* szValue = 0;
			engiGetAggrElement(pAggregate, iElementIndex, sdaiUNICODE, (char**)&szValue);

			if (szValue != nullptr) 
			{
				strLabel += szValue;
			}
			else 
			{
				strLabel += L"NA";
			}
		} // case sdaiBOOLEAN:
		break;

		case sdaiINSTANCE:
		{
			int_t iAttributeInstance = 0;
			engiGetAggrElement(pAggregate, iElementIndex, sdaiINSTANCE, &iAttributeInstance);

			if (iAttributeInstance != 0)
			{
				strLabel += L"#";

				int_t iValue = internalGetP21Line(iAttributeInstance);

				CString strValue;
				strValue.Format(_T("%lld"), iValue);

				strLabel += strValue;

				bHasChildren = true;
			}
			else 
			{
				strLabel += L"NA";
			}
		} // case sdaiINSTANCE:
		break;

		case sdaiINTEGER:
		{
			int_t iValue = 0;
			engiGetAggrElement(pAggregate, iElementIndex, sdaiINTEGER, &iValue);

			CString strValue;
			strValue.Format(_T("%lld"), iValue);

			strLabel += strValue;
		} // case sdaiINTEGER:
		break;

		case sdaiSTRING:
		{
			wchar_t* szValue = 0;
			engiGetAggrElement(pAggregate, iElementIndex, sdaiUNICODE, (char **)&szValue);

			if (szValue != nullptr) 
			{
				strLabel += L"'";
				strLabel += szValue;
				strLabel += L"'";
			}
			else 
			{
				strLabel += L"NA";
			}
		} // case sdaiSTRING:
		break;

		case sdaiREAL:
		{
			double dValue = 0;
			engiGetAggrElement(pAggregate, iElementIndex, sdaiREAL, &dValue);

			CString strValue;
			strValue.Format(_T("%f"), dValue);

			strLabel += strValue;
		} // case sdaiREAL:
		break;

	default:
		ASSERT(false);
		break;
	} // switch (iAggregateType) 
}	//	*/

// ------------------------------------------------------------------------------------------------
void CRelationsView::GetAttributeReferencesADB(SdaiADB ADB, HTREEITEM hParent)
{
    switch (sdaiGetADBType(ADB)) {
        case  sdaiADB:
        {
            SdaiADB    attributeDataBlock = 0;
            if (sdaiGetADBValue(ADB, sdaiADB, &attributeDataBlock)) {
				GetAttributeReferencesADB(attributeDataBlock, hParent);
            }
        }
        break;

        case  sdaiAGGR:
        {
            SdaiAggr        valueAggr = nullptr;
            SdaiInstance    valueInstance = 0;
            if (sdaiGetADBValue(ADB, sdaiAGGR, &valueAggr)) {
				GetAttributeReferencesAggregation(valueAggr, hParent);
            }
            else if (sdaiGetADBValue(ADB, sdaiINSTANCE, &valueInstance) || valueInstance) {
				int_t iEntity = sdaiGetInstanceType(valueInstance);
				LoadInstance(
					iEntity,
					CEntity::GetName(iEntity),
					valueInstance,
					hParent);
			}
            else {
                assert(false);
            }
        }
        break;

        case  sdaiINSTANCE:
        {
            SdaiInstance    value = 0;
            if (sdaiGetADBValue(ADB, sdaiINSTANCE, &value) || value) {
				int_t iEntity = sdaiGetInstanceType(value);
				LoadInstance(
					iEntity,
					CEntity::GetName(iEntity),
					value,
					hParent);
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
        assert(false);
        break;
    }
}

// ------------------------------------------------------------------------------------------------
void CRelationsView::GetAttributeReferencesAggregationElement(SdaiAggr aggregation, int_t aggrType, SdaiInteger index, HTREEITEM hParent)
{
    switch (aggrType) {
        case  sdaiADB:
        {
            SdaiADB    attributeDataBlock = 0;
            if (engiGetAggrElement(aggregation, index, sdaiADB, &attributeDataBlock)) {
				GetAttributeReferencesADB(attributeDataBlock, hParent);
            }
        }
        break;

		case  sdaiAGGR:
        {
            SdaiAggr        valueAggr = nullptr;
            SdaiInstance    valueInstance = 0;
            if (engiGetAggrElement(aggregation, index, sdaiAGGR, &valueAggr)) {
				GetAttributeReferencesAggregation(valueAggr, hParent);
            }
            else if (engiGetAggrElement(aggregation, index, sdaiINSTANCE, &valueInstance) || valueInstance) {
				int_t iEntity = sdaiGetInstanceType(valueInstance);
				LoadInstance(
					iEntity,
					CEntity::GetName(iEntity), 
					valueInstance,
					hParent);
            }
            else {
                assert(false);
            }
        }
        break;

		case  sdaiINSTANCE:
        {
            SdaiInstance    value = 0;
            if (engiGetAggrElement(aggregation, index, sdaiINSTANCE, &value) || value) {
				int_t iEntity = sdaiGetInstanceType(value);
				LoadInstance(
					iEntity,
					CEntity::GetName(iEntity),
					value,
					hParent);
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
        assert(false);
        break;
    }
}

// ------------------------------------------------------------------------------------------------
void CRelationsView::GetAttributeReferencesAggregation(SdaiAggr aggregation, HTREEITEM hParent)
{
    SdaiInteger memberCount = sdaiGetMemberCount(aggregation);
    if (memberCount == 0)
        return;

    int_t   aggrType = 0;
    engiGetAggrType(aggregation, &aggrType);

    SdaiInteger index = 0;
	GetAttributeReferencesAggregationElement(aggregation, aggrType, index++, hParent);
    while (index < memberCount) {
		GetAttributeReferencesAggregationElement(aggregation, aggrType, index++, hParent);
    }
}

// ------------------------------------------------------------------------------------------------
void CRelationsView::GetAttributeReferences(SdaiInstance iInstance, SdaiAttr iAttribute, const char* /*szAttributeName*/, HTREEITEM hParent)
{
    int_t   attrType = engiGetAttrType(iAttribute);
    if (attrType & engiTypeFlagAggr ||
        attrType & engiTypeFlagAggrOption)
        attrType = sdaiAGGR;

    switch (attrType) {
        case  0:
        break;

        case  sdaiADB:
		{
            SdaiADB    attributeDataBlock = 0;
            if (sdaiGetAttr(iInstance, iAttribute, sdaiADB, &attributeDataBlock)) {
                assert(attributeDataBlock);
				GetAttributeReferencesADB(attributeDataBlock, hParent);
            }
        }
        break;

        case  sdaiAGGR:
        {
            SdaiAggr        valueAggr = nullptr;
            SdaiInstance    valueInstance = 0;
            if (sdaiGetAttr(iInstance, iAttribute, sdaiAGGR, &valueAggr)) {
				GetAttributeReferencesAggregation(valueAggr, hParent);
            }
            else if (sdaiGetAttr(iInstance, iAttribute, sdaiINSTANCE, &valueInstance) || valueInstance) {
				int_t iEntity = sdaiGetInstanceType(valueInstance);
				LoadInstance(
					iEntity,
					CEntity::GetName(iEntity),
					valueInstance,
					hParent);
			}
        }
        break;

        case  sdaiINSTANCE:
        {
            SdaiInstance    value = 0;
            if (sdaiGetAttr(iInstance, iAttribute, sdaiINSTANCE, &value) || value) {
				int_t iEntity = sdaiGetInstanceType(value);
				LoadInstance(
					iEntity,
					CEntity::GetName(iEntity), 
					value, 
					hParent);
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
        assert(false);
        break;
    }

/*	...
	switch (iAttributeType) 
	{
		case sdaiADB:
		{
			int_t ADB = 0;
			sdaiGetAttrBN(iInstance, (char *)szAttributeName, sdaiADB, &ADB);

			if (ADB != 0) 
			{
				GetAttributeReferencesADB(ADB, hParent);
			}
		} // case sdaiADB:
		break;

		case sdaiAGGR:
		{
			int_t* pValue = nullptr;
			sdaiGetAttrBN(iInstance, (char *)szAttributeName, sdaiAGGR, &pValue);

			if (pValue != nullptr) 
			{
				int_t i = 0;
				int_t iCount = sdaiGetMemberCount(pValue);

				while (i < iCount) 
				{
					GetAttributeReferencesAGGR(pValue, i, hParent);

					i++;
				}
			}
		} // case sdaiAGGR:
		break;

		case  sdaiBOOLEAN:
		case  sdaiENUM:
		case  sdaiLOGICAL:
			break;

		case sdaiINSTANCE:
		{
			int_t iAttributeInstance = 0;
			sdaiGetAttrBN(iInstance, (char *)szAttributeName, sdaiINSTANCE, &iAttributeInstance);

			if (iAttributeInstance != 0) 
			{
				int_t iEntity = sdaiGetInstanceType(iAttributeInstance);

				LoadInstance(
					iEntity,
					CEntity::GetName(iEntity), 
					iAttributeInstance, 
					hParent);
			}
		} // case sdaiINSTANCE:
		break;

		case  sdaiINTEGER:
			break;

		case  sdaiSTRING:
			break;

		case  sdaiREAL:
			break;

		default:
			ASSERT(FALSE);
			break;
	} // switch (iAttributeType)	//	*/
}

/*
// ------------------------------------------------------------------------------------------------
void CRelationsView::GetAttributeReferencesADB(int_t ADB, HTREEITEM hParent)
{
	switch (sdaiGetADBType((void *)ADB)) 
	{
		case sdaiADB:
		{
			int_t ADBsub = 0;
			sdaiGetADBValue((void *)ADB, sdaiADB, &ADBsub);

			if (ADBsub != 0) 
			{
				GetAttributeReferencesADB(ADBsub, hParent);
			}
		} // case sdaiADB:
		break;

		case sdaiAGGR:
		{
			int_t * pValue = nullptr;
			sdaiGetADBValue((void *)ADB, sdaiAGGR, &pValue);

			if (pValue != nullptr) 
			{
				int_t i = 0;
				int_t iCount = sdaiGetMemberCount(pValue);

				while (i < iCount) 
				{
					GetAttributeReferencesAGGR(pValue, i, hParent);

					i++;
				}
			}
		} // case sdaiAGGR:
		break;

		case sdaiBOOLEAN:
		case sdaiENUM:
		case sdaiLOGICAL:
			break;

		case sdaiINSTANCE:
		{
			int_t iAttributeInstance = 0;
			sdaiGetADBValue((void*)ADB, sdaiINSTANCE, &iAttributeInstance);

			if (iAttributeInstance != 0) 
			{
				int_t iEntity = sdaiGetInstanceType(iAttributeInstance);

				wchar_t* szEntity = nullptr;
				engiGetEntityName(iEntity, sdaiUNICODE, (const char**)&szEntity);

				LoadInstance(iEntity, szEntity, iAttributeInstance, hParent);
			}
		}
		break;

		case sdaiINTEGER:
			break;

		case sdaiSTRING:
			break;

		case sdaiREAL:
			break;

		default:
			ASSERT(FALSE);
			break;
	} // switch (sdaiGetADBType((void *)ADB))
}

// ------------------------------------------------------------------------------------------------
void CRelationsView::GetAttributeReferencesAGGR(int_t* pAggregate, int_t iElementIndex, HTREEITEM hParent)
{
	int_t iAggregateType = 0;
	engiGetAggrType(pAggregate, &iAggregateType);

	switch (iAggregateType)
	{
		case sdaiADB:
		{
			int_t ADB = 0;
			engiGetAggrElement(pAggregate, iElementIndex, sdaiADB, &ADB);

			if (ADB != 0) 
			{
				GetAttributeReferencesADB(ADB, hParent);
			}
		}
		break;

		case sdaiAGGR:
		{
			int_t * pValue = nullptr;
			engiGetAggrElement(pAggregate, iElementIndex, sdaiAGGR, &pValue);

			if (pValue != nullptr) 
			{
				int_t i = 0;
				int_t iCount = sdaiGetMemberCount(pValue);

				while (i < iCount)
				{
					GetAttributeReferencesAGGR(pValue, i, hParent);

					i++;
				}
			}
		} // case sdaiAGGR:
		break;

		case  sdaiBOOLEAN:
		case  sdaiENUM:
		case  sdaiLOGICAL:
			break;

		case sdaiINSTANCE:
		{
			int_t iAttributeInstance = 0;
			engiGetAggrElement(pAggregate, iElementIndex, sdaiINSTANCE, &iAttributeInstance);

			if (iAttributeInstance != 0)
			{
				int_t iEntity = sdaiGetInstanceType(iAttributeInstance);

				wchar_t* szEntity = nullptr;
				engiGetEntityName(iEntity, sdaiUNICODE, (const char**)&szEntity);

				LoadInstance(iEntity, szEntity, iAttributeInstance, hParent);
			}
		} // case sdaiINSTANCE:
		break;

		case sdaiINTEGER:
			break;

		case sdaiSTRING:
			break;

		case sdaiREAL:
			break;

		default:
			ASSERT(FALSE);
			break;
	} // switch (iAggregateType)
}	//	*/

// ------------------------------------------------------------------------------------------------
void CRelationsView::GetEntityHierarchy(int_t iEntity, vector<wstring>& vecHierarchy) const
{
	ASSERT(iEntity != 0);

	wstring strEntity = CEntity::GetName(iEntity);
	if (engiGetEntityIsAbstract(iEntity))
	{
		strEntity += L" (ABSTRACT)";
	}

	vecHierarchy.push_back(strEntity);

	int_t iParent = engiGetEntityParent(iEntity);
	while (iParent != 0)
	{
		strEntity = CEntity::GetName(iParent);
		if (engiGetEntityIsAbstract(iParent))
		{
			strEntity += L" (ABSTRACT)";
		}

		vecHierarchy.insert(vecHierarchy.begin(), strEntity);

		iParent = engiGetEntityParent(iParent);
	}
}

// ------------------------------------------------------------------------------------------------
void CRelationsView::Clean()
{
	for (auto pInstanceData : m_vecItemDataCache)
	{
		delete pInstanceData;
	}
	m_vecItemDataCache.clear();
}

// ------------------------------------------------------------------------------------------------
void CRelationsView::OnNMClickTree(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	*pResult = 0;
}

// ------------------------------------------------------------------------------------------------
void CRelationsView::OnNMRClickTree(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	*pResult = 0;
}

// ------------------------------------------------------------------------------------------------
void CRelationsView::OnTVNItemexpandingTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	*pResult = 0;

	int iImage, iSelectedImage = -1;
	m_treeCtrl.GetItemImage(pNMTreeView->itemNew.hItem, iImage, iSelectedImage);

	ASSERT(iImage == iSelectedImage);

	if ((iImage == IMAGE_INSTANCE) && (pNMTreeView->itemNew.cChildren == 1))
	{
		HTREEITEM hChild = m_treeCtrl.GetChildItem(pNMTreeView->itemNew.hItem);
		if (hChild == nullptr)
		{
			ASSERT(FALSE);

			return;
		}

		if (m_treeCtrl.GetItemText(hChild) != ITEM_PENDING_LOAD)
		{
			return;
		}		

		auto pAttributeData = (CAttributeData*)m_treeCtrl.GetItemData(hChild);
		ASSERT(pAttributeData != nullptr);

		m_treeCtrl.DeleteItem(hChild);
		
		GetAttributeReferences(
				pAttributeData->getInstance(),
				sdaiGetAttrDefinition(sdaiGetInstanceType(pAttributeData->getInstance()), pAttributeData->getName()),
				pAttributeData->getName(),
//				pAttributeData->getType(),
				pNMTreeView->itemNew.hItem);
	} // if ((iImage == IMAGE_INSTANCE) && ...
}

// ------------------------------------------------------------------------------------------------
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
			m_strTooltip = CA2W(pAttributeData->getName());
			if (engiGetAttrInverseBN(pAttributeData->getEntity(), pAttributeData->getName()))
			{
				m_strTooltip += L" (INVERSE)";
			}

			m_strTooltip += L"\n";

			m_strTooltip += CEntity::GetName(pAttributeData->getEntity());
			if (engiGetEntityIsAbstract(pAttributeData->getEntity()))
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
			GetEntityHierarchy(pInstanceData->getEntity(), vecHierarchy);

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

// ------------------------------------------------------------------------------------------------
CRelationsView::CRelationsView()
	: m_enMode(enumRelationsViewMode::Flat)
	, m_vecItemDataCache()
	, m_pSearchDialog(nullptr)
	, m_strTooltip(L"")
{
}

// ------------------------------------------------------------------------------------------------
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

	ASSERT(GetController() != nullptr);
	GetController()->RegisterView(this);

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
	m_pSearchDialog = new CSearchAttributeDialog(GetController(), &m_treeCtrl);
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
