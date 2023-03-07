
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
		pEntity->getEntity(),
		pEntity->getName(),
		pEntity->getInstances());
	
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
		int_t iEntity = sdaiGetInstanceType(iInstance);

		wchar_t* szEntity = nullptr;
		engiGetEntityName(iEntity, sdaiUNICODE, (char**)&szEntity);

		LoadInstance(iEntity, szEntity, iInstance, hModel);
	}
	// ******************************************************************************************** //

	m_treeCtrl.Expand(hModel, TVE_EXPAND);
}

// ------------------------------------------------------------------------------------------------
void CRelationsView::LoadProperties(int_t iEntity, const wchar_t* szEntity, const vector<int_t>& vecInstances)
{
	m_treeCtrl.DeleteAllItems();

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
	for (size_t iIFCObject = 0; iIFCObject < vecInstances.size(); iIFCObject++)
	{
		LoadInstance(iEntity, szEntity, vecInstances[iIFCObject], hModel);
	}
	// ******************************************************************************************** //

	m_treeCtrl.Expand(hModel, TVE_EXPAND);
}

// ------------------------------------------------------------------------------------------------
void CRelationsView::LoadInstance(int_t iEntity, const wchar_t* szEntity, int_t iInstance, HTREEITEM hParent)
{	
	ASSERT(iEntity != 0);
	ASSERT(iInstance != 0);

	wchar_t * szName = nullptr;
	sdaiGetAttrBN(iInstance, "Name", sdaiUNICODE, &szName);

	wchar_t * szDescription = nullptr;
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
	CInstanceData* pIFCInstanceData = new CInstanceData(iInstance, iEntity, szEntity);
	m_vecInstancesCache.push_back(pIFCInstanceData);

	/*
	* Instance
	*/
	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = hParent;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_INSTANCE;
	tvInsertStruct.item.lParam = (LPARAM)pIFCInstanceData;

	HTREEITEM hInstance = m_treeCtrl.InsertItem(&tvInsertStruct);

	LoadInstanceAttributes(sdaiGetInstanceType(iInstance), iInstance, hInstance);
}

// ------------------------------------------------------------------------------------------------
int_t CRelationsView::LoadInstanceAttributes(int_t iEntity, int_t iInstance, HTREEITEM hParent)
{
	if (iEntity == 0)
	{
		return 0;
	}

	ASSERT(iInstance != 0);

	/*
	* Entity
	*/
	wchar_t* szEntity = nullptr;
	engiGetEntityName(iEntity, sdaiUNICODE, (char**)&szEntity);

	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = hParent;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = (LPWSTR)szEntity;
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_ENTITY;
	tvInsertStruct.item.lParam = NULL;

	HTREEITEM hEntity = m_treeCtrl.InsertItem(&tvInsertStruct);

	int_t iAttrubutesCount = LoadInstanceAttributes(engiGetEntityParent(iEntity), iInstance, hParent);
	while (iAttrubutesCount < engiGetEntityNoArguments(iEntity)) 
	{
		char * szAttributeName = 0;
		engiGetEntityArgumentName(iEntity, iAttrubutesCount, sdaiSTRING, (char **)&szAttributeName);

		wchar_t * szAttributeNameW = 0;
		engiGetEntityArgumentName(iEntity, iAttrubutesCount, sdaiUNICODE, (char **)&szAttributeNameW);

		if (!IsAttributeIgnored(iEntity, szAttributeNameW))
		{
			int_t iAttributeType = engiGetAttrTypeBN(iInstance, (char*)szAttributeName);
			if (iAttributeType & 128)
			{
				iAttributeType = sdaiAGGR;
			}

			if (iAttributeType == sdaiADB)
			{
				int_t ADB = 0;
				sdaiGetAttrBN(iInstance, (char*)szAttributeName, sdaiADB, &ADB);

				if (ADB == 0)
				{
					ASSERT(FALSE);

					iAttributeType = sdaiINSTANCE;
				}
			}

			AddInstanceAttribute(iInstance, szAttributeNameW, iAttributeType, hEntity);
		}

		iAttrubutesCount++;
	} // while (iAttrubutesCount < engiGetEntityNoArguments(iEntity))

	return iAttrubutesCount;
}


// ------------------------------------------------------------------------------------------------
void CRelationsView::AddInstanceAttribute(int_t iInstance, const wchar_t* szAttributeName, int_t iAttributeType, HTREEITEM hParent)
{
	ASSERT(iInstance != 0);

	wstring strText;
	bool bChildren = false;
	CreateAttributeText(&bChildren, iInstance, CW2A(szAttributeName), iAttributeType, strText);

	wstring strAttribute = szAttributeName;
	strAttribute += L" = ";
	strAttribute += strText;

	/*
	* Attribute
	*/
	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = hParent;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
	tvInsertStruct.item.pszText = (LPWSTR)strAttribute.c_str();
	tvInsertStruct.item.cChildren = bChildren ? 1 : 0;
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = bChildren ? IMAGE_INSTANCE : IMAGE_ATTRIBUTE;
	tvInsertStruct.item.lParam = NULL;

	HTREEITEM hAttribute = m_treeCtrl.InsertItem(&tvInsertStruct);

	if (bChildren)
	{
		m_vecAttributesCache.push_back(new CAttributeData(iInstance, CW2A(szAttributeName), iAttributeType));

		/*
		* Add a fake item - load on demand
		*/
		tvInsertStruct.hParent = hAttribute;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = ITEM_PENDING_LOAD;
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_INSTANCE;
		tvInsertStruct.item.lParam = (LPARAM)m_vecAttributesCache[m_vecAttributesCache.size() - 1];

		m_treeCtrl.InsertItem(&tvInsertStruct);
	}
}

// ------------------------------------------------------------------------------------------------
void CRelationsView::CreateAttributeText(bool* pbChildren, int_t iInstance, const char* szAttributeName, int_t iAttributeType, wstring& strText)
{
	strText = _T("");

	switch (iAttributeType)
	{
		case sdaiADB:
		{
			int_t ADB = 0;
			sdaiGetAttrBN(iInstance, (char *)szAttributeName, sdaiADB, &ADB);

			if (ADB != 0) 
			{
				wchar_t	* szPath = (wchar_t *)sdaiGetADBTypePath((void *)ADB, sdaiUNICODE);
				strText += szPath;

				strText += L"(";			
				CreateAttributeTextADB(pbChildren, ADB, strText);
				strText += L")";
			} // if (ADB != 0) 
			else 
			{
				strText += L"\u2205";
			}
		} // case sdaiADB:
		break;

		case sdaiAGGR:
		{
			int_t * pValue = nullptr;
			sdaiGetAttrBN(iInstance, (char *)szAttributeName, sdaiAGGR, &pValue);

			if (pValue != nullptr) 
			{
				strText += L"(";

				int_t i = 0;
				int_t iCount = sdaiGetMemberCount(pValue);

				while ((i < iCount)  &&  (i < 8))
				{
					if (i != 0) 
					{
						strText += L", ";
					}

					CreateAttributeTextAGGR(pbChildren, pValue, i, strText);

					i++;
				}

				if (i < iCount)
				{
					strText += L", ...";
				}

				strText += L")";
			} // if (pValue != nullptr) 
			else 
			{
				strText += L"\u2205";
			}
		} // case sdaiAGGR:
		break;

		case sdaiBOOLEAN:
		case sdaiENUM:
		case sdaiLOGICAL:
		{
			wchar_t	* pValue = nullptr;
			sdaiGetAttrBN(iInstance, (char *)szAttributeName, sdaiUNICODE, (char **)&pValue);

			if (pValue != nullptr) 
			{
				strText += pValue;
			}
			else 
			{
				strText += L"\u2205";
			}
		} // case sdaiBOOLEAN:
		break;

		case sdaiINSTANCE:
		{
			int_t iAttributeInstance = 0;
			sdaiGetAttrBN(iInstance, (char *)szAttributeName, sdaiINSTANCE, &iAttributeInstance);

			if (iAttributeInstance != 0) 
			{
				strText += L"#";

				int_t iValue = internalGetP21Line(iAttributeInstance);

				CString strValue;
				strValue.Format(_T("%lld"), iValue);

				strText += strValue;
			
				*pbChildren = true;
			}
			else 
			{
				strText += L"\u2205";
			}
		} // case sdaiINSTANCE:
		break;

		case sdaiINTEGER:
		{
			int_t iValue = 0;
			sdaiGetAttrBN(iInstance, (char *)szAttributeName, sdaiINTEGER, &iValue);

			CString strValue;
			strValue.Format(_T("%lld"), iValue);

			strText += strValue;
		} // case sdaiINTEGER:
		break;

		case sdaiSTRING:
		{
			wchar_t	* szValue = 0;
			sdaiGetAttrBN(iInstance, (char *)szAttributeName, sdaiUNICODE, (char **)&szValue);

			if (szValue != nullptr) 
			{
				strText += L"'";
				strText += szValue;
				strText += L"'";
			}
			else 
			{
				strText += L"\u2205";
			}
		} // case sdaiSTRING:
		break;

		case  sdaiREAL:
		{
			double	dValue = 0;
			sdaiGetAttrBN(iInstance, (char *)szAttributeName, sdaiREAL, &dValue);

			CString strValue;
			strValue.Format(_T("%f"), dValue);

			strText += strValue;
		}
		break;

		default:
			ASSERT(FALSE);
			break;
	} // switch (iAttributeType)
}

// ------------------------------------------------------------------------------------------------
void CRelationsView::CreateAttributeTextADB(bool* pbChildren, int_t ADB, wstring& strText)
{
	switch (sdaiGetADBType((void *)ADB)) 
	{
		case sdaiADB:
		{
			int_t ADBsub = 0;
			sdaiGetADBValue((void *)ADB, sdaiADB, &ADBsub);

			if (ADBsub != 0) 
			{
				wchar_t	* szPath = (wchar_t *)sdaiGetADBTypePath((void*)ADBsub, sdaiUNICODE);
				strText += szPath;

				strText += L"(";
				CreateAttributeTextADB(pbChildren, ADB, strText);
				strText += L")";
			}
			else 
			{
				strText += L"\u2205";
			}
		} // case sdaiADB:
		break;

		case sdaiAGGR:
		{
			int_t * pValue = 0;
			sdaiGetADBValue((void *)ADB, sdaiAGGR, &pValue);

			if (pValue != nullptr)
			{
				strText += L"(";
				
				int_t i = 0;
				int_t iCount = sdaiGetMemberCount(pValue);
				
				while (i < iCount) 
				{
					if (i != 0) 
					{
						strText += L", ";
					}

					strText += L"???";
					i++;
				}

				strText += L")";
			}
			else 
			{
				strText += L"\u2205";
			}
		} // case sdaiAGGR:
		break;

		case sdaiBOOLEAN:
		case sdaiENUM:
		case sdaiLOGICAL:
		{
			wchar_t	* szValue = 0;
			sdaiGetADBValue((void *)ADB, sdaiUNICODE, (char **)&szValue);

			if (szValue != nullptr)
			{
				strText += szValue;
			}
			else
			{
				strText += L"\u2205";
			}
		} // case sdaiBOOLEAN:
		break;

		case sdaiINSTANCE:
		{
			int_t iAttributeInstance = 0;
			sdaiGetADBValue((void *)ADB, sdaiINSTANCE, &iAttributeInstance);

			if (iAttributeInstance != 0)
			{
				strText += L"#";

				int_t iValue = internalGetP21Line(iAttributeInstance);

				CString strValue;
				strValue.Format(_T("%lld"), iValue);

				strText += strValue;

				*pbChildren = true;
			}
			else 
			{
				strText += L"\u2205";
			}
		} // case sdaiINSTANCE:
		break;

		case sdaiINTEGER:
		{
			int_t iValue = 0;
			sdaiGetADBValue((void *)ADB, sdaiINTEGER, &iValue);

			CString strValue;
			strValue.Format(_T("%lld"), iValue);

			strText += strValue;
		} // case sdaiINTEGER:
		break;

		case sdaiSTRING:
		{
			wchar_t	* szValue = 0;
			sdaiGetADBValue((void *)ADB, sdaiUNICODE, (char **)&szValue);

			if (szValue != nullptr)
			{
				strText += L"'";
				strText += szValue;
				strText += L"'";
			}
			else
			{
				strText += L"\u2205";
			}
		} // case sdaiSTRING:
		break;

		case sdaiREAL:
		{
			double dValue = 0;
			sdaiGetADBValue((void*)ADB, sdaiREAL, &dValue);

			CString strValue;
			strValue.Format(_T("%f"), dValue);

			strText += strValue;
		} // case sdaiREAL:
		break;

		default:
			ASSERT(FALSE);
			break;
	} // switch (sdaiGetADBType((void *)ADB)) 
}

// ------------------------------------------------------------------------------------------------
void CRelationsView::CreateAttributeTextAGGR(bool* pbChildren, int_t* pAggregate, int_t iElementIndex, wstring& strText)
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
				strText += szPath;

				strText += L"(";
				CreateAttributeTextADB(pbChildren, ADB, strText);
				strText += L")";
			}
			else 
			{
				strText += L"\u2205";
			}
		} // case sdaiADB:
		break;

		case sdaiAGGR:
		{
			int_t * pValue = 0;
			engiGetAggrElement(pAggregate, iElementIndex, sdaiAGGR, &pValue);

			if (pValue != nullptr)
			{
				strText += L"(";

				int_t i = 0;
				int_t iCount = sdaiGetMemberCount(pValue);

				while ((i < iCount)  &&  (i < 8)) 
				{
					if (i != 0) 
					{
						strText += L", ";
					}

					CreateAttributeTextAGGR(pbChildren, pValue, i, strText);

					i++;
				}

				if (i < iCount)
				{
					strText += L", ...";
				}

				strText += L")";
			}
			else 
			{
				strText += L"\u2205";
			}
		} // case sdaiAGGR:
		break;

		case sdaiBOOLEAN:
		case sdaiENUM:
		case sdaiLOGICAL:
		{
			wchar_t	* szValue = 0;
			engiGetAggrElement(pAggregate, iElementIndex, sdaiUNICODE, (char **)&szValue);

			if (szValue != nullptr) 
			{
				strText += szValue;
			}
			else 
			{
				strText += L"\u2205";
			}
		} // case sdaiBOOLEAN:
		break;

		case sdaiINSTANCE:
		{
			int_t iAttributeInstance = 0;
			engiGetAggrElement(pAggregate, iElementIndex, sdaiINSTANCE, &iAttributeInstance);

			if (iAttributeInstance != 0)
			{
				strText += L"#";

				int_t iValue = internalGetP21Line(iAttributeInstance);

				CString strValue;
				strValue.Format(_T("%lld"), iValue);

				strText += strValue;

				*pbChildren = true;
			}
			else 
			{
				strText += L"\u2205";
			}
		} // case sdaiINSTANCE:
		break;

		case sdaiINTEGER:
		{
			int_t iValue = 0;
			engiGetAggrElement(pAggregate, iElementIndex, sdaiINTEGER, &iValue);

			CString strValue;
			strValue.Format(_T("%lld"), iValue);

			strText += strValue;
		} // case sdaiINTEGER:
		break;

		case sdaiSTRING:
		{
			wchar_t* szValue = 0;
			engiGetAggrElement(pAggregate, iElementIndex, sdaiUNICODE, (char **)&szValue);

			if (szValue != nullptr) 
			{
				strText += L"'";
				strText += szValue;
				strText += L"'";
			}
			else 
			{
				strText += L"\u2205";
			}
		} // case sdaiSTRING:
		break;

		case sdaiREAL:
		{
			double dValue = 0;
			engiGetAggrElement(pAggregate, iElementIndex, sdaiREAL, &dValue);

			CString strValue;
			strValue.Format(_T("%f"), dValue);

			strText += strValue;
		} // case sdaiREAL:
		break;

	default:
		ASSERT(false);
		break;
	} // switch (iAggregateType) 
}

// ------------------------------------------------------------------------------------------------
void CRelationsView::GetAttributeReferences(int_t iInstance, const char* szAttributeName, int_t iAttributeType, HTREEITEM hParent)
{
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

				wchar_t* szEntity = nullptr;
				engiGetEntityName(iEntity, sdaiUNICODE, (char**)&szEntity);

				LoadInstance(iEntity, szEntity, iAttributeInstance, hParent);
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
	} // switch (iAttributeType) 
}

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
				engiGetEntityName(iEntity, sdaiUNICODE, (char**)&szEntity);

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
				engiGetEntityName(iEntity, sdaiUNICODE, (char**)&szEntity);

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
}

// ------------------------------------------------------------------------------------------------
bool CRelationsView::IsAttributeIgnored(int_t iEntity, const wchar_t* szAttributeName) const
{
	auto pModel = GetModel();
	auto pEntityProvider = pModel->GetEntityProvider();

	auto& mapEntities = pEntityProvider->GetEntities();

	auto itEntity = mapEntities.find(iEntity);
	if (itEntity == mapEntities.end())
	{
		ASSERT(FALSE);

		return false;
	}

	return itEntity->second->isAttributeIgnored(szAttributeName);
}

// ----------------------------------------------------------------------------
void CRelationsView::OnNMClickTree(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	*pResult = 0;
}

// ----------------------------------------------------------------------------
void CRelationsView::OnNMRClickTree(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	*pResult = 0;
}

// ----------------------------------------------------------------------------
void CRelationsView::OnTvnItemexpandingTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	*pResult = 0;

	int iImage = -1;
	int iSelectedImage = -1;
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

		CAttributeData * pIFCAttributeData = (CAttributeData *)m_treeCtrl.GetItemData(hChild);
		ASSERT(pIFCAttributeData != nullptr);

		m_treeCtrl.DeleteItem(hChild);

		GetAttributeReferences(pIFCAttributeData->getInstance(), pIFCAttributeData->getName(), pIFCAttributeData->getType(), pNMTreeView->itemNew.hItem);
	} // if ((iImage == IMAGE_INSTANCE) && ...
}

CRelationsView::CRelationsView()
	: m_vecInstancesCache()
	, m_vecAttributesCache()
	, m_pSearchDialog(nullptr)
{
}

CRelationsView::~CRelationsView()
{
	for (size_t iData = 0; iData < m_vecInstancesCache.size(); iData++)
	{
		delete m_vecInstancesCache[iData];
	}

	for (size_t iData = 0; iData < m_vecAttributesCache.size(); iData++)
	{
		delete m_vecAttributesCache[iData];
	}
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
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_TREE_IFC, &CRelationsView::OnTvnItemexpandingTree)
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
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;

	if (!m_treeCtrl.Create(dwViewStyle, rectDummy, this, IDC_TREE_IFC))
	{
		TRACE0("Failed to create IFC Instances View\n");
		return -1;      // fail to create
	}

	// Load view images:
	m_imageList.Create(IDB_CLASS_VIEW, 16, 0, RGB(255, 0, 0));
	m_treeCtrl.SetImageList(&m_imageList, TVSIL_NORMAL);

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_EXPLORER);
	m_wndToolBar.LoadToolBar(IDR_EXPLORER, 0, 0, TRUE /* Is locked */);

	OnChangeVisualStyle();

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() 
		| CBRS_TOOLTIPS | CBRS_FLYBY);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & 
		~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	m_wndToolBar.SetOwner(this);

	// All commands will be routed via this control , not via the parent frame:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

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

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(
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

	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_EXPLORER_24 : IDR_EXPLORER, 0, 0, TRUE /* Locked */);
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
