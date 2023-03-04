
#include "stdafx.h"
#include "mainfrm.h"
#include "IFCRelationsView.h"
#include "Resource.h"
#include "IFCModel.h"
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
// CIFCRelationsView


// ------------------------------------------------------------------------------------------------
/*virtual*/ void CIFCRelationsView::OnModelChanged() /*override*/
{
	LoadProperties(0, NULL, vector<int_t>());
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CIFCRelationsView::OnInstanceSelected(CSTEPView* pSender) /*override*/
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

	auto pSelectedInstance = GetController()->GetSelectedInstance() != nullptr ?
		dynamic_cast<CIFCInstance*>(GetController()->GetSelectedInstance()) :
		nullptr;

	vector<CIFCInstance*> vecInstances;
	if (pSelectedInstance != nullptr)
	{
		vecInstances.push_back(pSelectedInstance);
	}
	
	LoadInstances(vecInstances);
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CIFCRelationsView::OnViewRelations(CSTEPView* pSender, CSTEPInstance* pInstance) /*override*/
{
	if (pSender == this)
	{
		return;
	}

	auto pIFCInstance = dynamic_cast<CIFCInstance*>(pInstance);
	if (pIFCInstance == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	vector<int_t> vecInstances;
	vecInstances.push_back(pIFCInstance->getInstance());

	LoadProperties(pIFCInstance->getEntity(), pIFCInstance->getEntityName(), vecInstances);

	ShowPane(TRUE, TRUE, TRUE);
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CIFCRelationsView::OnViewRelations(CSTEPView* pSender, CSTEPEntity* pEntity) /*override*/
{
	if (pSender == this)
	{
		return;
	}

	auto pIFCEntity = dynamic_cast<CIFCEntity*>(pEntity);
	if (pIFCEntity == nullptr)
	{
		ASSERT(FALSE);

		return;
	}
	
	LoadProperties(
		pIFCEntity->getEntity(), 
		pIFCEntity->getName(), 
		pIFCEntity->getInstances());
	
	ShowPane(TRUE, TRUE, TRUE);
}

// ------------------------------------------------------------------------------------------------
CIFCModel* CIFCRelationsView::GetModel() const
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

	switch (pModel->GetType())
	{
		case enumSTEPModelType::STEP:
		{
			// NA
		}
		break;

		case enumSTEPModelType::IFC:
		{
			return pController->GetModel()->As<CIFCModel>();
		}

		default:
		{
			ASSERT(FALSE); // Unknown
		}
		break;
	} // switch (pModel ->GetType())

	return nullptr;
}

// ------------------------------------------------------------------------------------------------
void CIFCRelationsView::LoadInstances(const vector<CIFCInstance*>& vecInstances)
{
	m_ifcTreeCtrl.DeleteAllItems();

	auto pModel = GetModel();
	if (pModel == NULL)
	{
		return;
	}

	// ******************************************************************************************** //
	// Model
	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = NULL;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = (LPWSTR)pModel->getModelName();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_MODEL;
	tvInsertStruct.item.lParam = NULL;

	HTREEITEM hModel = m_ifcTreeCtrl.InsertItem(&tvInsertStruct);
	m_ifcTreeCtrl.SetItemState(hModel, TVIS_BOLD, TVIS_BOLD);
	// ******************************************************************************************** //	

	// ******************************************************************************************** //
	// Instances
	for (size_t iIFCObject = 0; iIFCObject < vecInstances.size(); iIFCObject++)
	{
		LoadInstance(vecInstances[iIFCObject]->getEntity(), vecInstances[iIFCObject]->getEntityName(), vecInstances[iIFCObject]->getInstance(), hModel);
	}
	// ******************************************************************************************** //

	m_ifcTreeCtrl.Expand(hModel, TVE_EXPAND);
}

// ------------------------------------------------------------------------------------------------
void CIFCRelationsView::LoadProperties(int_t iEntity, const wchar_t* szEntity, const vector<int_t>& vecInstances)
{
	m_ifcTreeCtrl.DeleteAllItems();

	auto pModel = GetModel();
	if (pModel == NULL)
	{
		return;
	}

	// ******************************************************************************************** //
	// Model
	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = NULL;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = (LPWSTR)pModel->getModelName();
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_MODEL;
	tvInsertStruct.item.lParam = NULL;

	HTREEITEM hModel = m_ifcTreeCtrl.InsertItem(&tvInsertStruct);
	m_ifcTreeCtrl.SetItemState(hModel, TVIS_BOLD, TVIS_BOLD);
	// ******************************************************************************************** //	

	// ******************************************************************************************** //
	// Instances
	for (size_t iIFCObject = 0; iIFCObject < vecInstances.size(); iIFCObject++)
	{
		LoadInstance(iEntity, szEntity, vecInstances[iIFCObject], hModel);
	}
	// ******************************************************************************************** //

	m_ifcTreeCtrl.Expand(hModel, TVE_EXPAND);
}

// ------------------------------------------------------------------------------------------------
void CIFCRelationsView::LoadInstance(int_t iEntity, const wchar_t* szEntity, int_t iInstance, HTREEITEM hParent)
{	
	ASSERT(iEntity != 0);
	ASSERT(iInstance != 0);

	wchar_t * szName = NULL;
	sdaiGetAttrBN(iInstance, "Name", sdaiUNICODE, &szName);

	wchar_t * szDescription = NULL;
	sdaiGetAttrBN(iInstance, "Description", sdaiUNICODE, &szDescription);

	int_t iValue = internalGetP21Line(iInstance);

	CString strPrefix;
	strPrefix.Format(_T("# %lld ="), iValue);
	
	wstring strItem = strPrefix;
	strItem += L" ";
	strItem += szEntity;
	if ((szName != NULL) && (wcslen(szName) > 0))
	{
		strItem += L" '";
		strItem += szName;
		strItem += L"'";
	}

	if ((szDescription != NULL) && (wcslen(szDescription) > 0))
	{
		strItem += L" (";
		strItem += szDescription;
		strItem += L")";
	}

	/*
	* Data
	*/
	CIFCInstanceData* pIFCInstanceData = new CIFCInstanceData(iInstance, iEntity, szEntity);
	m_vecIFCInstancesCache.push_back(pIFCInstanceData);

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

	HTREEITEM hInstance = m_ifcTreeCtrl.InsertItem(&tvInsertStruct);

	LoadInstanceAttributes(sdaiGetInstanceType(iInstance), szEntity, iInstance, hInstance);
}

// ------------------------------------------------------------------------------------------------
int_t CIFCRelationsView::LoadInstanceAttributes(int_t iEntity, const wchar_t* szEntity, int_t iInstance, HTREEITEM hParent)
{
	if (iEntity == 0)
	{
		return 0;
	}

	ASSERT(iInstance != 0);

	/*
	* Entity
	*/
	wchar_t	* szEntityName = NULL;
	engiGetEntityName(iEntity, sdaiUNICODE, (char **)&szEntityName);

	TV_INSERTSTRUCT tvInsertStruct;
	tvInsertStruct.hParent = hParent;
	tvInsertStruct.hInsertAfter = TVI_LAST;
	tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvInsertStruct.item.pszText = (LPWSTR)szEntityName;
	tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_ENTITY;
	tvInsertStruct.item.lParam = NULL;

	HTREEITEM hEntity = m_ifcTreeCtrl.InsertItem(&tvInsertStruct);

	int_t iAttrubutesCount = LoadInstanceAttributes(engiGetEntityParent(iEntity), szEntity, iInstance, hParent);

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
void CIFCRelationsView::AddInstanceAttribute(int_t iInstance, const wchar_t* szAttributeName, int_t iAttributeType, HTREEITEM hParent)
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

	HTREEITEM hAttribute = m_ifcTreeCtrl.InsertItem(&tvInsertStruct);

	if (bChildren)
	{
		m_vecIFCAttributesCache.push_back(new CIFCAttributeData(iInstance, CW2A(szAttributeName), iAttributeType));

		/*
		* Add a fake item - load on demand
		*/
		tvInsertStruct.hParent = hAttribute;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = ITEM_PENDING_LOAD;
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_INSTANCE;
		tvInsertStruct.item.lParam = (LPARAM)m_vecIFCAttributesCache[m_vecIFCAttributesCache.size() - 1];

		m_ifcTreeCtrl.InsertItem(&tvInsertStruct);
	}
}

// ------------------------------------------------------------------------------------------------
void CIFCRelationsView::CreateAttributeText(bool* pbChildren, int_t iInstance, const char* szAttributeName, int_t iAttributeType, wstring& strText)
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
			int_t * pValue = NULL;
			sdaiGetAttrBN(iInstance, (char *)szAttributeName, sdaiAGGR, &pValue);

			if (pValue != NULL) 
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
			} // if (pValue != NULL) 
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
			wchar_t	* pValue = NULL;
			sdaiGetAttrBN(iInstance, (char *)szAttributeName, sdaiUNICODE, (char **)&pValue);

			if (pValue != NULL) 
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

			if (szValue != NULL) 
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
void CIFCRelationsView::CreateAttributeTextADB(bool* pbChildren, int_t ADB, wstring& strText)
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

			if (pValue != NULL)
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

			if (szValue != NULL)
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

			if (szValue != NULL)
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
void CIFCRelationsView::CreateAttributeTextAGGR(bool* pbChildren, int_t* pAggregate, int_t iElementIndex, wstring& strText)
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

			if (pValue != NULL)
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

			if (szValue != NULL) 
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

			if (szValue != NULL) 
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
void CIFCRelationsView::GetAttributeReferences(int_t iInstance, const char* szAttributeName, int_t iAttributeType, HTREEITEM hParent)
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
			int_t* pValue = NULL;
			sdaiGetAttrBN(iInstance, (char *)szAttributeName, sdaiAGGR, &pValue);

			if (pValue != NULL) 
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
				wchar_t	* szEntityName = NULL;
				engiGetEntityName(sdaiGetInstanceType(iAttributeInstance), sdaiUNICODE, (char **)&szEntityName);

				LoadInstance(sdaiGetInstanceType(iAttributeInstance), szEntityName, iAttributeInstance, hParent);
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
void CIFCRelationsView::GetAttributeReferencesADB(int_t ADB, HTREEITEM hParent)
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
			int_t * pValue = NULL;
			sdaiGetADBValue((void *)ADB, sdaiAGGR, &pValue);

			if (pValue != NULL) 
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
				wchar_t	* szEntityName = NULL;
				engiGetEntityName(sdaiGetInstanceType(iAttributeInstance), sdaiUNICODE, (char **)&szEntityName);

				LoadInstance(sdaiGetInstanceType(iAttributeInstance), szEntityName, iAttributeInstance, hParent);
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
void CIFCRelationsView::GetAttributeReferencesAGGR(int_t* pAggregate, int_t iElementIndex, HTREEITEM hParent)
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
			int_t * pValue = NULL;
			engiGetAggrElement(pAggregate, iElementIndex, sdaiAGGR, &pValue);

			if (pValue != NULL) 
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
				wchar_t	* szEntityName = NULL;
				engiGetEntityName(sdaiGetInstanceType(iAttributeInstance), sdaiUNICODE, (char **)&szEntityName);

				LoadInstance(sdaiGetInstanceType(iAttributeInstance), szEntityName, iAttributeInstance, hParent);
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
bool CIFCRelationsView::IsAttributeIgnored(int_t iEntity, const wchar_t* szAttributeName) const
{
	auto pModel = GetModel();
	ASSERT(pModel != NULL);

	auto& mapEntities = pModel->GetEntities();

	auto itEntity = mapEntities.find(iEntity);
	if (itEntity == mapEntities.end())
	{
		ASSERT(FALSE);

		return false;
	}

	return itEntity->second->isAttributeIgnored(szAttributeName);
}

// ----------------------------------------------------------------------------
void CIFCRelationsView::OnNMClickTreeIFC(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	*pResult = 0;
}

// ----------------------------------------------------------------------------
void CIFCRelationsView::OnNMRClickTreeIFC(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	*pResult = 0;
}

// ----------------------------------------------------------------------------
void CIFCRelationsView::OnTvnItemexpandingTreeIFC(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	*pResult = 0;

	int iImage = -1;
	int iSelectedImage = -1;
	m_ifcTreeCtrl.GetItemImage(pNMTreeView->itemNew.hItem, iImage, iSelectedImage);

	ASSERT(iImage == iSelectedImage);

	if ((iImage == IMAGE_INSTANCE) && (pNMTreeView->itemNew.cChildren == 1))
	{
		HTREEITEM hChild = m_ifcTreeCtrl.GetChildItem(pNMTreeView->itemNew.hItem);
		if (hChild == NULL)
		{
			ASSERT(FALSE);

			return;
		}

		if (m_ifcTreeCtrl.GetItemText(hChild) != ITEM_PENDING_LOAD)
		{
			return;
		}		

		CIFCAttributeData * pIFCAttributeData = (CIFCAttributeData *)m_ifcTreeCtrl.GetItemData(hChild);
		ASSERT(pIFCAttributeData != NULL);

		m_ifcTreeCtrl.DeleteItem(hChild);

		GetAttributeReferences(pIFCAttributeData->getInstance(), pIFCAttributeData->getName(), pIFCAttributeData->getType(), pNMTreeView->itemNew.hItem);
	} // if ((iImage == IMAGE_INSTANCE) && ...
}

CIFCRelationsView::CIFCRelationsView()
	: m_vecIFCInstancesCache()
	, m_vecIFCAttributesCache()
	, m_pSearchDialog(NULL)
{
}

CIFCRelationsView::~CIFCRelationsView()
{
	for (size_t iData = 0; iData < m_vecIFCInstancesCache.size(); iData++)
	{
		delete m_vecIFCInstancesCache[iData];
	}

	for (size_t iData = 0; iData < m_vecIFCAttributesCache.size(); iData++)
	{
		delete m_vecIFCAttributesCache[iData];
	}
}

BEGIN_MESSAGE_MAP(CIFCRelationsView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_PROPERTIES, OnProperties)
	ON_WM_CONTEXTMENU()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_NOTIFY(NM_CLICK, IDC_TREE_IFC, &CIFCRelationsView::OnNMClickTreeIFC)
	ON_NOTIFY(NM_RCLICK, IDC_TREE_IFC, &CIFCRelationsView::OnNMRClickTreeIFC)
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_TREE_IFC, &CIFCRelationsView::OnTvnItemexpandingTreeIFC)
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorkspaceBar message handlers

int CIFCRelationsView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	ASSERT(GetController() != NULL);
	GetController()->RegisterView(this);

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create view:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;

	if (!m_ifcTreeCtrl.Create(dwViewStyle, rectDummy, this, IDC_TREE_IFC))
	{
		TRACE0("Failed to create IFC Instances View\n");
		return -1;      // fail to create
	}

	// Load view images:
	m_imageList.Create(IDB_CLASS_VIEW, 16, 0, RGB(255, 0, 0));
	m_ifcTreeCtrl.SetImageList(&m_imageList, TVSIL_NORMAL);

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
	m_pSearchDialog = new CSearchAttributeDialog(GetController(), &m_ifcTreeCtrl);
	m_pSearchDialog->Create(IDD_DIALOG_SEARCH, this);

	return 0;
}

void CIFCRelationsView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CIFCRelationsView::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(
		NULL, 
		rectClient.left, 
		rectClient.top, 
		rectClient.Width(), 
		cyTlb, 
		SWP_NOACTIVATE | SWP_NOZORDER);

	m_ifcTreeCtrl.SetWindowPos(
		NULL, rectClient.left + 1, 
		rectClient.top + cyTlb + 1, 
		rectClient.Width() - 2, 
		rectClient.Height() - cyTlb - 2, 
		SWP_NOACTIVATE | SWP_NOZORDER);
}

void CIFCRelationsView::OnProperties()
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

void CIFCRelationsView::OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/)
{	
}

void CIFCRelationsView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rectTree;
	m_ifcTreeCtrl.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CIFCRelationsView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_ifcTreeCtrl.SetFocus();
}

void CIFCRelationsView::OnChangeVisualStyle()
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

	m_ifcTreeCtrl.SetImageList(&m_imageList, TVSIL_NORMAL);

	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_EXPLORER_24 : IDR_EXPLORER, 0, 0, TRUE /* Locked */);
}

void CIFCRelationsView::OnDestroy()
{
	__super::OnDestroy();

	delete m_pSearchDialog;
}

void CIFCRelationsView::OnShowWindow(BOOL bShow, UINT nStatus)
{
	__super::OnShowWindow(bShow, nStatus);

	if (!bShow)
	{
		m_pSearchDialog->ShowWindow(SW_HIDE);
	}
}
