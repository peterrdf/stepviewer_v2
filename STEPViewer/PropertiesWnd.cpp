
#include "stdafx.h"

#include "PropertiesWnd.h"
#include "Resource.h"
#include "MainFrm.h"
#include "STEPViewer.h"
#include "OpenGLSTEPView.h"
#include "STEPModel.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar

#define WM_LOAD_INSTANCE_PROPERTY_VALUES WM_USER + 1
#define WM_LOAD_INSTANCE_PROPERTIES WM_USER + 2

#define TRUE_VALUE_PROPERTY L"Yes"
#define FALSE_VALUE_PROPERTY L"No"
#define REMOVE_OBJECT_PROPERTY_COMMAND L"---<REMOVE>---"
#define SELECT_OBJECT_PROPERTY_COMMAND L"---<SELECT>---"


// ------------------------------------------------------------------------------------------------
CApplicationPropertyData::CApplicationPropertyData(enumPropertyType enPropertyType)
{
	m_enPropertyType = enPropertyType;
}

// ------------------------------------------------------------------------------------------------
enumPropertyType CApplicationPropertyData::GetType() const
{
	return m_enPropertyType;
}

// ------------------------------------------------------------------------------------------------
CApplicationProperty::CApplicationProperty(const CString& strName, const COleVariant& vtValue, LPCTSTR szDescription, DWORD_PTR dwData)
	: CMFCPropertyGridProperty(strName, vtValue, szDescription, dwData)
{
}

// ------------------------------------------------------------------------------------------------
CApplicationProperty::CApplicationProperty(const CString& strGroupName, DWORD_PTR dwData, BOOL bIsValueList)
	: CMFCPropertyGridProperty(strGroupName, dwData, bIsValueList)
{
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ CApplicationProperty::~CApplicationProperty()
{
	delete (CApplicationPropertyData *)GetData();
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CPropertiesWnd::OnModelChanged()
{
	m_wndObjectCombo.SetCurSel(0 /*Application*/);

	LoadApplicationProperties();
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CPropertiesWnd::OnShowMetaInformation()
{
	m_wndObjectCombo.SetCurSel(1 /*Properties*/);

	LoadMetaInformation();
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CPropertiesWnd::OnInstanceSelected(CSTEPView* /*pSender*/)
{
	m_wndObjectCombo.SetCurSel(1 /*Properties*/);

	LoadInstanceProperties();
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CPropertiesWnd::OnInstancePropertySelected()
{
	m_wndObjectCombo.SetCurSel(1 /*Properties*/);

	LoadInstanceProperties();
}

// ------------------------------------------------------------------------------------------------
/*afx_msg*/ LRESULT CPropertiesWnd::OnPropertyChanged(__in WPARAM /*wparam*/, __in LPARAM /*lparam*/)
{
	ASSERT(0); // todo

	//ASSERT(GetController() != nullptr);

	///*
	//* Application
	//*/
	//if (m_wndObjectCombo.GetCurSel() == 0)
	//{
	//	COpenGLSTEPView * pOpenGLView = GetController()->GetView<COpenGLSTEPView>();
	//	ASSERT(pOpenGLView != nullptr);

	//	CApplicationProperty * pApplicationProperty = dynamic_cast<CApplicationProperty *>((CMFCPropertyGridProperty *)lparam);
	//	if (pApplicationProperty != nullptr)
	//	{
	//		CString strValue = pApplicationProperty->GetValue();

	//		CApplicationPropertyData * pData = (CApplicationPropertyData *)pApplicationProperty->GetData();
	//		ASSERT(pData != nullptr);

	//		if ((pData->GetType() == ptLightPosition) || ((pData->GetType() == ptLightPositionItem)))
	//		{
	//			CMFCPropertyGridProperty * pPositionProperty = pData->GetType() == ptLightPosition ? pApplicationProperty : pApplicationProperty->GetParent();
	//			ASSERT(pPositionProperty->GetSubItemsCount() == 4);

	//			CLightPropertyData * pLightData = (CLightPropertyData *)pApplicationProperty->GetData();
	//			ASSERT(pLightData != nullptr);

	//			/*
	//			* X
	//			*/
	//			CMFCPropertyGridProperty * pX = pPositionProperty->GetSubItem(0);

	//			/*
	//			* Y
	//			*/
	//			CMFCPropertyGridProperty * pY = pPositionProperty->GetSubItem(1);

	//			/*
	//			* Z
	//			*/
	//			CMFCPropertyGridProperty * pZ = pPositionProperty->GetSubItem(2);

	//			/*
	//			* W
	//			*/
	//			CMFCPropertyGridProperty * pW = pPositionProperty->GetSubItem(3);

	//			/*
	//			* Update the light
	//			*/
	//			const vector<COpenGLLight> & vecOGLLights = pOpenGLView->GetOGLLights();

	//			COpenGLLight newLight(vecOGLLights[pLightData->GetLightIndex()]);
	//			newLight.setPosition(
	//				(float)_wtof((LPCTSTR)(CString)pX->GetValue()), 
	//				(float)_wtof((LPCTSTR)(CString)pY->GetValue()), 
	//				(float)_wtof((LPCTSTR)(CString)pZ->GetValue()), 
	//				(float)_wtof((LPCTSTR)(CString)pW->GetValue()));

	//			pOpenGLView->SetOGLLight(pLightData->GetLightIndex(), newLight);
	//		} // if ((pData->GetType() == ptLightPosition) || ...
	//		else
	//		{
	//			switch (pData->GetType())
	//			{
	//			case ptShowFaces:
	//			{
	//				pOpenGLView->ShowFaces(strValue == TRUE_VALUE_PROPERTY ? TRUE : FALSE);
	//			}
	//			break;

	//			case ptShowFacesWireframes:
	//			{
	//				pOpenGLView->ShowFacesPolygons(strValue == TRUE_VALUE_PROPERTY ? TRUE : FALSE);
	//			}
	//			break;

	//			case ptShowConceptualFacesWireframes:
	//			{
	//				pOpenGLView->ShowConceptualFacesPolygons(strValue == TRUE_VALUE_PROPERTY ? TRUE : FALSE);
	//			}
	//			break;

	//			case ptShowLines:
	//			{
	//				pOpenGLView->ShowLines(strValue == TRUE_VALUE_PROPERTY ? TRUE : FALSE);
	//			}
	//			break;

	//			case ptLineWidth:
	//			{
	//				int iValue = _wtoi((LPCTSTR)strValue);

	//				pOpenGLView->SetLineWidth((GLfloat)iValue);
	//			}
	//			break;

	//			case ptShowPoints:
	//			{
	//				pOpenGLView->ShowPoints(strValue == TRUE_VALUE_PROPERTY ? TRUE : FALSE);
	//			}
	//			break;

	//			case ptPointSize:
	//			{
	//				int iValue = _wtoi((LPCTSTR)strValue);

	//				pOpenGLView->SetPointSize((GLfloat)iValue);
	//			}
	//			break;

	//			case ptShowNormalVectors:
	//			{
	//				pOpenGLView->ShowNormalVectors(strValue == TRUE_VALUE_PROPERTY ? TRUE : FALSE);
	//			}
	//			break;

	//			case ptShowTangenVectors:
	//			{
	//				pOpenGLView->ShowTangentVectors(strValue == TRUE_VALUE_PROPERTY ? TRUE : FALSE);
	//			}
	//			break;

	//			case ptShowBiNormalVectors:
	//			{
	//				pOpenGLView->ShowBiNormalVectors(strValue == TRUE_VALUE_PROPERTY ? TRUE : FALSE);
	//			}
	//			break;

	//			case ptScaleVectors:
	//			{
	//				pOpenGLView->ScaleVectors(strValue == TRUE_VALUE_PROPERTY ? TRUE : FALSE);
	//			}
	//			break;

	//			case ptShowBoundingBoxes:
	//			{
	//				pOpenGLView->ShowBoundingBoxes(strValue == TRUE_VALUE_PROPERTY ? TRUE : FALSE);
	//			}
	//			break;

	//			case ptLightModelLocalViewer:
	//			{
	//				pOpenGLView->SetLightModelLocalViewer(strValue == TRUE_VALUE_PROPERTY ? true : false);
	//			}
	//			break;

	//			case ptLightModel2Sided:
	//			{
	//				pOpenGLView->SetLightModel2Sided(strValue == TRUE_VALUE_PROPERTY ? true : false);
	//			}
	//			break;

	//			case ptLightIsEnabled:
	//			{
	//				CLightPropertyData * pLightData = (CLightPropertyData *)pApplicationProperty->GetData();

	//				/*
	//				* Update the light
	//				*/
	//				const vector<COpenGLLight> & vecOGLLights = pOpenGLView->GetOGLLights();

	//				COpenGLLight newLight(vecOGLLights[pLightData->GetLightIndex()]);
	//				newLight.enable(strValue == TRUE_VALUE_PROPERTY ? true : false);

	//				pOpenGLView->SetOGLLight(pLightData->GetLightIndex(), newLight);
	//			}
	//			break;

	//			case ptVisibleValuesCountLimit:
	//			{
	//				int iValue = _wtoi((LPCTSTR)strValue);

	//				GetController()->SetVisibleValuesCountLimit(iValue);
	//			}
	//			break;

	//			case ptScalelAndCenter:
	//			{
	//				GetController()->SetScaleAndCenter(strValue == TRUE_VALUE_PROPERTY ? TRUE : FALSE);
	//			}
	//			break;

	//			default:
	//				ASSERT(false);
	//				break;
	//			} // switch (pData->GetType())
	//		} // else if ((pData->GetType() == ptLightPosition) || ...			

	//		return 0;
	//	} // if (pApplicationProperty != nullptr)

	//	CColorApplicationProperty * pColorApplicationProperty = dynamic_cast<CColorApplicationProperty *>((CMFCPropertyGridProperty *)lparam);
	//	if (pColorApplicationProperty != nullptr)
	//	{
	//		float fR = GetRValue(pColorApplicationProperty->GetColor()) / 255.f;
	//		float fG = GetGValue(pColorApplicationProperty->GetColor()) / 255.f;
	//		float fB = GetBValue(pColorApplicationProperty->GetColor()) / 255.f;			

	//		CLightPropertyData * pData = (CLightPropertyData *)pColorApplicationProperty->GetData();
	//		ASSERT(pData != nullptr);

	//		const vector<COpenGLLight> & vecOGLLights = pOpenGLView->GetOGLLights();

	//		switch (pData->GetType())
	//		{
	//		case ptLightModelAmbient:
	//		{
	//			pOpenGLView->SetLightModelAmbient(fR, fG, fB);
	//		}
	//		break;

	//		case ptAmbientLight:
	//		{
	//			COpenGLLight newLight(vecOGLLights[pData->GetLightIndex()]);
	//			newLight.setAmbient(fR, fG, fB);

	//			pOpenGLView->SetOGLLight(pData->GetLightIndex(), newLight);
	//		}
	//		break;

	//		case ptDiffuseLight:
	//		{
	//			COpenGLLight newLight(vecOGLLights[pData->GetLightIndex()]);
	//			newLight.setDiffuse(fR, fG, fB);

	//			pOpenGLView->SetOGLLight(pData->GetLightIndex(), newLight);
	//		}
	//		break;

	//		case ptSpecularLight:
	//		{
	//			COpenGLLight newLight(vecOGLLights[pData->GetLightIndex()]);
	//			newLight.setSpecular(fR, fG, fB);

	//			pOpenGLView->SetOGLLight(pData->GetLightIndex(), newLight);
	//		}
	//		break;

	//		default:
	//		{
	//			ASSERT(false); // unexpected!
	//		}
	//		break;
	//		} // switch (pData->GetType())

	//		return 0;
	//	} // if (pColorApplicationProperty != nullptr)

	//	ASSERT(false); // unexpected!
	//} // if (m_wndObjectCombo.GetCurSel() == 0)

	///*
	//* Properties
	//*/
	//if (m_wndObjectCombo.GetCurSel() == 1)
	//{
	//	/*
	//	* Object properties
	//	*/
	//	CRDFInstanceObjectProperty * pObjectProperty = dynamic_cast<CRDFInstanceObjectProperty *>((CMFCPropertyGridProperty *)lparam);
	//	if (pObjectProperty != nullptr)
	//	{
	//		CString strValue = pObjectProperty->GetValue();

	//		CRDFInstancePropertyData * pData = (CRDFInstancePropertyData *)pObjectProperty->GetData();
	//		ASSERT(pData != nullptr);

	//		int64_t * piInstances = nullptr;
	//		int64_t iCard = 0;
	//		GetObjectProperty(pData->GetInstance()->getInstance(), pData->GetProperty()->getInstance(), &piInstances, &iCard);

	//		ASSERT(iCard > 0);

	//		vector<int64_t> vecValues;
	//		vecValues.assign(piInstances, piInstances + iCard);

	//		if (strValue == REMOVE_OBJECT_PROPERTY_COMMAND)
	//		{
	//			/*
	//			* Remove the value
	//			*/
	//			int64_t	iMinCard = 0;
	//			int64_t iMaxCard = 0;
	//			pData->GetProperty()->GetRestrictions(pData->GetInstance()->getInstance(), iMinCard, iMaxCard);				

	//			ASSERT((iCard - 1) >= (((iMinCard == -1) && (iMaxCard == -1)) ? 0 : iMinCard));

	//			/*
	//			* Remove a value
	//			*/
	//			vecValues.erase(vecValues.begin() + pData->GetCard());

	//			SetObjectProperty(pData->GetInstance()->getInstance(), pData->GetProperty()->getInstance(), vecValues.data(), vecValues.size());

	//			/*
	//			* Notify
	//			*/
	//			ASSERT(pData->GetController() != nullptr);
	//			pData->GetController()->OnInstancePropertyEdited(pData->GetInstance(), pData->GetProperty());

	//			/*
	//			* Delete this item
	//			*/
	//			CMFCPropertyGridProperty * pProperty = pObjectProperty->GetParent();
	//			ASSERT(pProperty->GetSubItemsCount() >= 3/*range, cardinality and at least 1 value*/);

	//			CMFCPropertyGridProperty * pValue = pProperty->GetSubItem((int)pData->GetCard() + 2/*range and cardinality*/);
	//			PostMessage(WM_LOAD_INSTANCE_PROPERTY_VALUES, (WPARAM)pValue, 0);
	//		} // if (strValue == REMOVE_OBJECT_PROPERTY_COMMAND)
	//		else
	//		{
	//			if (strValue == SELECT_OBJECT_PROPERTY_COMMAND)
	//			{
	//				CObjectRDFProperty* pObjectRDFProperty = dynamic_cast<CObjectRDFProperty*>(pData->GetProperty());
	//				ASSERT(pObjectRDFProperty != nullptr);

	//				ASSERT(pData->GetController() != nullptr);
	//				CSelectInstanceDialog dlgSelectInstanceDialog(GetController(), pData->GetInstance(), pObjectRDFProperty, pData->GetCard());
	//				if (dlgSelectInstanceDialog.DoModal() == IDOK)
	//				{
	//					ASSERT(dlgSelectInstanceDialog.m_iInstance != -1);						

	//					/*
	//					* Update the value
	//					*/						
	//					vecValues[pData->GetCard()] = dlgSelectInstanceDialog.m_iInstance;

	//					SetObjectProperty(pData->GetInstance()->getInstance(), pData->GetProperty()->getInstance(), vecValues.data(), vecValues.size());

	//					/*
	//					* Notify
	//					*/
	//					ASSERT(pData->GetController() != nullptr);
	//					GetController()->OnInstancePropertyEdited(pData->GetInstance(), pData->GetProperty());

	//					/*
	//					* Value
	//					*/
	//					ASSERT(!dlgSelectInstanceDialog.m_strInstanceUniqueName.IsEmpty());

	//					pObjectProperty->SetValue((_variant_t)dlgSelectInstanceDialog.m_strInstanceUniqueName);
	//				}
	//				else
	//				{
	//					/*
	//					* Value
	//					*/
	//					ASSERT(!dlgSelectInstanceDialog.m_strOldInstanceUniqueName.IsEmpty());

	//					pObjectProperty->SetValue((_variant_t)dlgSelectInstanceDialog.m_strOldInstanceUniqueName);
	//				}
	//			}
	//			else
	//			{
	//				/*
	//				* Update the value
	//				*/
	//				int64_t iInstance = pObjectProperty->GetInstance((LPCTSTR)strValue);
	//				vecValues[pData->GetCard()] = iInstance;

	//				SetObjectProperty(pData->GetInstance()->getInstance(), pData->GetProperty()->getInstance(), vecValues.data(), vecValues.size());

	//				/*
	//				* Notify
	//				*/
	//				ASSERT(pData->GetController() != nullptr);
	//				GetController()->OnInstancePropertyEdited(pData->GetInstance(), pData->GetProperty());
	//			}				
	//		} // else if (strValue == REMOVE_OBJECT_PROPERTY_COMMAND)

	//		return 0;
	//	} // if (pObjectProperty != nullptr)

	//	/*
	//	* ColorComponent properties
	//	*/
	//	CSTEPColorSelectorProperty * pColorSelectorProperty = dynamic_cast<CSTEPColorSelectorProperty *>((CMFCPropertyGridProperty *)lparam);
	//	if (pColorSelectorProperty != nullptr)
	//	{	
	//		CRDFInstancePropertyData * pData = (CRDFInstancePropertyData *)pColorSelectorProperty->GetData();
	//		ASSERT(pData != nullptr);

	//		CSTEPModel * pModel = GetController()->GetModel();
	//		ASSERT(pModel != nullptr);

	//		ASSERT(pData->GetInstance()->getClassInstance() == GetClassByName(pModel->GetModel(), "ColorComponent"));			

	//		const map<int64_t, CRDFProperty *> & mapRDFProperties = pModel->GetRDFProperties();

	//		/*
	//		* R
	//		*/
	//		int64_t iRProperty = GetPropertyByName(pModel->GetModel(), "R");

	//		double dR = GetRValue(pColorSelectorProperty->GetColor()) / 255.;
	//		SetDatatypeProperty(pData->GetInstance()->getInstance(), iRProperty, &dR, 1);

	//		map<int64_t, CRDFProperty *>::const_iterator itRDFProperty = mapRDFProperties.find(iRProperty);
	//		ASSERT(itRDFProperty != mapRDFProperties.end());

	//		/*
	//		* Notify
	//		*/
	//		GetController()->OnInstancePropertyEdited(pData->GetInstance(), itRDFProperty->second);

	//		/*
	//		* G
	//		*/
	//		int64_t iGProperty = GetPropertyByName(pModel->GetModel(), "G");

	//		double dG = GetGValue(pColorSelectorProperty->GetColor()) / 255.;
	//		SetDatatypeProperty(pData->GetInstance()->getInstance(), iGProperty, &dG, 1);

	//		itRDFProperty = mapRDFProperties.find(iGProperty);
	//		ASSERT(itRDFProperty != mapRDFProperties.end());

	//		/*
	//		* Notify
	//		*/
	//		GetController()->OnInstancePropertyEdited(pData->GetInstance(), itRDFProperty->second);

	//		/*
	//		* B
	//		*/
	//		int64_t iBProperty = GetPropertyByName(pModel->GetModel(), "B");

	//		double dB = GetBValue(pColorSelectorProperty->GetColor()) / 255.;
	//		SetDatatypeProperty(pData->GetInstance()->getInstance(), iBProperty, &dB, 1);

	//		itRDFProperty = mapRDFProperties.find(iBProperty);
	//		ASSERT(itRDFProperty != mapRDFProperties.end());

	//		/*
	//		* Notify
	//		*/
	//		GetController()->OnInstancePropertyEdited(pData->GetInstance(), itRDFProperty->second);

	//		/*
	//		* Update UI
	//		*/
	//		PostMessage(WM_LOAD_INSTANCE_PROPERTIES, 0, 0);
	//	} // if (pColorSelectorProperty != nullptr)

	//	/*
	//	* Data properties
	//	*/
	//	CRDFInstanceProperty * pProperty = dynamic_cast<CRDFInstanceProperty *>((CMFCPropertyGridProperty *)lparam);
	//	if (pProperty == nullptr)
	//	{
	//		return 0;
	//	}

	//	CString strValue = pProperty->GetValue();

	//	CRDFInstancePropertyData * pData = (CRDFInstancePropertyData *)pProperty->GetData();
	//	ASSERT(pData != nullptr);

	//	switch(pData->GetProperty()->getType())
	//	{
	//	case TYPE_BOOL_DATATYPE:
	//	{
	//		/*
	//		* Read the original values
	//		*/
	//		int64_t iCard = 0;
	//		bool* pbValue = nullptr;
	//		GetDatatypeProperty(pData->GetInstance()->getInstance(), pData->GetProperty()->getInstance(), (void**)&pbValue, &iCard);

	//		ASSERT(iCard > 0);

	//		vector<bool> vecValues;
	//		vecValues.assign(pbValue, pbValue + iCard);

	//		/*
	//		* Update the modified value
	//		*/

	//		bool bValue = strValue != L"0";
	//		vecValues[pData->GetCard()] = bValue;

	//		bool* pbNewValues = new bool[vecValues.size()];
	//		for (size_t iValue = 0; iValue < vecValues.size(); iValue++)
	//		{
	//			pbNewValues[iValue] = vecValues[iValue];
	//		}

	//		SetDatatypeProperty(pData->GetInstance()->getInstance(), pData->GetProperty()->getInstance(), (void**)pbNewValues, vecValues.size());

	//		delete[] pbNewValues;
	//		pbNewValues = nullptr;

	//		ASSERT(GetController() != nullptr);

	//		GetController()->OnInstancePropertyEdited(pData->GetInstance(), pData->GetProperty());
	//	} // case TYPE_BOOL_DATATYPE:
	//	break;

	//	case TYPE_CHAR_DATATYPE:
	//	{
	//		/*
	//		* Read the original values
	//		*/
	//		int64_t iCard = 0;
	//		char ** szValue = nullptr;
	//		GetDatatypeProperty(pData->GetInstance()->getInstance(), pData->GetProperty()->getInstance(), (void **)&szValue, &iCard);

	//		ASSERT(iCard > 0);

	//		char ** szNewValues = (char **)new size_t[iCard];
	//		for (int iValue = 0; iValue < iCard; iValue++)
	//		{
	//			szNewValues[iValue] = new char[strlen(szValue[iValue]) + 1];
	//			strcpy(szNewValues[iValue], szValue[iValue]);
	//		}

	//		/*
	//		* Update the modified value
	//		*/
	//		delete[] szNewValues[pData->GetCard()];
	//		szNewValues[pData->GetCard()] = new char[strlen(CW2A(strValue)) + 1];
	//		strcpy(szNewValues[pData->GetCard()], CW2A(strValue));

	//		SetDatatypeProperty(pData->GetInstance()->getInstance(), pData->GetProperty()->getInstance(), (void **)szNewValues, iCard);

	//		for (int iValue = 0; iValue < iCard; iValue++)
	//		{
	//			delete [] szNewValues[iValue];
	//		}
	//		delete [] szNewValues;

	//		ASSERT(GetController() != nullptr);

	//		GetController()->OnInstancePropertyEdited(pData->GetInstance(), pData->GetProperty());
	//	} // case TYPE_CHAR_DATATYPE:
	//	break;

	//	case TYPE_DOUBLE_DATATYPE:
	//	{
	//		/*
	//		* Read the original values
	//		*/
	//		int64_t iCard = 0;
	//		double * pdValue = nullptr;
	//		GetDatatypeProperty(pData->GetInstance()->getInstance(), pData->GetProperty()->getInstance(), (void **)&pdValue, &iCard);
	//		
	//		ASSERT(iCard > 0);

	//		vector<double> vecValues;
	//		vecValues.assign(pdValue, pdValue + iCard);

	//		/*
	//		* Update the modified value
	//		*/

	//		double dValue = _wtof((LPCTSTR)strValue);
	//		vecValues[pData->GetCard()] = dValue;

	//		SetDatatypeProperty(pData->GetInstance()->getInstance(), pData->GetProperty()->getInstance(), (void **)vecValues.data(), vecValues.size());

	//		ASSERT(GetController() != nullptr);

	//		GetController()->OnInstancePropertyEdited(pData->GetInstance(), pData->GetProperty());
	//	} // case TYPE_DOUBLE_DATATYPE:
	//	break;

	//	case TYPE_INT_DATATYPE:
	//	{
	//		/*
	//		* Read the original values
	//		*/
	//		int64_t iCard = 0;
	//		int64_t * piValue = nullptr;
	//		GetDatatypeProperty(pData->GetInstance()->getInstance(), pData->GetProperty()->getInstance(), (void **)&piValue, &iCard);

	//		ASSERT(iCard > 0);

	//		vector<int64_t> vecValues;
	//		vecValues.assign(piValue, piValue + iCard);

	//		/*
	//		* Update the modified value
	//		*/
	//		int64_t iValue = _wtoi64((LPCTSTR)strValue);
	//		vecValues[pData->GetCard()] = iValue;

	//		SetDatatypeProperty(pData->GetInstance()->getInstance(), pData->GetProperty()->getInstance(), (void **)vecValues.data(), vecValues.size());

	//		ASSERT(GetController() != nullptr);

	//		GetController()->OnInstancePropertyEdited(pData->GetInstance(), pData->GetProperty());
	//	} // case TYPE_INT_DATATYPE:
	//	break;

	//	default:
	//	{
	//		ASSERT(false); // unknown type
	//	}
	//	break;
	//	} // switch(pData->GetProperty()->getType())

	//	return 0;
	//} // if (m_wndObjectCombo.GetCurSel() == 1)

	//ASSERT(false); // unknown property
	
	return 0;
}

CPropertiesWnd::CPropertiesWnd()
{
	m_nComboHeight = 0;
}

CPropertiesWnd::~CPropertiesWnd()
{
}

BEGIN_MESSAGE_MAP(CPropertiesWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_EXPAND_ALL, OnExpandAllProperties)
	ON_UPDATE_COMMAND_UI(ID_EXPAND_ALL, OnUpdateExpandAllProperties)
	ON_COMMAND(ID_SORTPROPERTIES, OnSortProperties)
	ON_UPDATE_COMMAND_UI(ID_SORTPROPERTIES, OnUpdateSortProperties)
	ON_COMMAND(ID_PROPERTIES1, OnProperties1)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES1, OnUpdateProperties1)
	ON_COMMAND(ID_PROPERTIES2, OnProperties2)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES2, OnUpdateProperties2)
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
	ON_WM_DESTROY()
	ON_REGISTERED_MESSAGE(AFX_WM_PROPERTY_CHANGED, OnPropertyChanged)
	ON_CBN_SELENDOK(ID_COMBO_PROPERTIES_VIEW, OnViewModeChanged)
	ON_MESSAGE(WM_LOAD_INSTANCE_PROPERTY_VALUES, OnLoadInstancePropertyValues)
	ON_MESSAGE(WM_LOAD_INSTANCE_PROPERTIES, OnLoadInstanceProperties)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar message handlers

void CPropertiesWnd::AdjustLayout()
{
	if (GetSafeHwnd () == nullptr || (AfxGetMainWnd() != nullptr && AfxGetMainWnd()->IsIconic()))
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndObjectCombo.SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), m_nComboHeight, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndToolBar.SetWindowPos(nullptr, rectClient.left, rectClient.top + m_nComboHeight, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndPropList.SetWindowPos(nullptr, rectClient.left, rectClient.top + m_nComboHeight + cyTlb, rectClient.Width(), rectClient.Height() -(m_nComboHeight+cyTlb), SWP_NOACTIVATE | SWP_NOZORDER);
}

int CPropertiesWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	ASSERT(GetController() != nullptr);
	GetController()->RegisterView(this);

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create combo:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_BORDER | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if (!m_wndObjectCombo.Create(dwViewStyle, rectDummy, this, ID_COMBO_PROPERTIES_VIEW))
	{
		TRACE0("Failed to create Properties Combo \n");
		return -1;      // fail to create
	}

	m_wndObjectCombo.AddString(_T("Application"));
	m_wndObjectCombo.AddString(_T("Properties"));
	m_wndObjectCombo.SetCurSel(0);

	CRect rectCombo;
	m_wndObjectCombo.GetClientRect (&rectCombo);

	m_nComboHeight = rectCombo.Height();

	if (!m_wndPropList.Create(WS_VISIBLE | WS_CHILD, rectDummy, this, 2))
	{
		TRACE0("Failed to create Properties Grid \n");
		return -1;      // fail to create
	}

	OnViewModeChanged();

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_PROPERTIES);
	m_wndToolBar.LoadToolBar(IDR_PROPERTIES, 0, 0, TRUE /* Is locked */);
	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_PROPERTIES_HC : IDR_PROPERTIES, 0, 0, TRUE /* Locked */);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetOwner(this);

	// All commands will be routed via this control , not via the parent frame:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	AdjustLayout();
	return 0;
}

void CPropertiesWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CPropertiesWnd::OnExpandAllProperties()
{
	m_wndPropList.ExpandAll();
}

void CPropertiesWnd::OnUpdateExpandAllProperties(CCmdUI* /* pCmdUI */)
{
}

void CPropertiesWnd::OnSortProperties()
{
	m_wndPropList.SetAlphabeticMode(!m_wndPropList.IsAlphabeticMode());
}

void CPropertiesWnd::OnUpdateSortProperties(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndPropList.IsAlphabeticMode());
}

void CPropertiesWnd::OnProperties1()
{
	// TODO: Add your command handler code here
}

void CPropertiesWnd::OnUpdateProperties1(CCmdUI* /*pCmdUI*/)
{
	// TODO: Add your command update UI handler code here
}

void CPropertiesWnd::OnProperties2()
{
	// TODO: Add your command handler code here
}

void CPropertiesWnd::OnUpdateProperties2(CCmdUI* /*pCmdUI*/)
{
	// TODO: Add your command update UI handler code here
}

// ------------------------------------------------------------------------------------------------
void CPropertiesWnd::LoadApplicationProperties()
{
	m_wndPropList.RemoveAll();
	m_wndPropList.AdjustLayout();

	SetPropListFont();

	m_wndPropList.EnableHeaderCtrl(FALSE);
	m_wndPropList.EnableDescriptionArea();
	m_wndPropList.SetVSDotNetLook();
	m_wndPropList.MarkModifiedProperties();

	auto pController = GetController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pOpenGLView = GetController()->GetView<COpenGLView>();
	if (pOpenGLView == nullptr)
	{
		return;
	}

#pragma region View
	auto pViewGroup = new CMFCPropertyGridProperty(_T("View"));	

	{
		auto pProperty = new CApplicationProperty(_T("Faces"),
			pOpenGLView->AreFacesShown() ? TRUE_VALUE_PROPERTY : FALSE_VALUE_PROPERTY, _T("Faces"),
			(DWORD_PTR)new CApplicationPropertyData(enumPropertyType::ShowFaces));
		pProperty->AddOption(TRUE_VALUE_PROPERTY);
		pProperty->AddOption(FALSE_VALUE_PROPERTY);
		pProperty->AllowEdit(FALSE);

		pViewGroup->AddSubItem(pProperty);
	}

	{
		auto pProperty = new CApplicationProperty(_T("Conceptual faces wireframes"),
			pOpenGLView->AreConceptualFacesPolygonsShown() ? TRUE_VALUE_PROPERTY : FALSE_VALUE_PROPERTY,
			_T("Conceptual faces wireframes"),
			(DWORD_PTR)new CApplicationPropertyData(enumPropertyType::ShowConceptualFacesWireframes));
		pProperty->AddOption(TRUE_VALUE_PROPERTY);
		pProperty->AddOption(FALSE_VALUE_PROPERTY);
		pProperty->AllowEdit(FALSE);

		pViewGroup->AddSubItem(pProperty);
	}

	{
		auto pProperty = new CApplicationProperty(_T("Lines"), pOpenGLView->AreLinesShown() ? TRUE_VALUE_PROPERTY : FALSE_VALUE_PROPERTY,
			_T("Lines"),
			(DWORD_PTR)new CApplicationPropertyData(enumPropertyType::ShowLines));
		pProperty->AddOption(TRUE_VALUE_PROPERTY);
		pProperty->AddOption(FALSE_VALUE_PROPERTY);
		pProperty->AllowEdit(FALSE);

		pViewGroup->AddSubItem(pProperty);
	}

	{
		auto pProperty = new CApplicationProperty(_T("Points"), pOpenGLView->ArePointsShown() ? TRUE_VALUE_PROPERTY : FALSE_VALUE_PROPERTY,
			_T("Points"),
			(DWORD_PTR)new CApplicationPropertyData(enumPropertyType::ShowPoints));
		pProperty->AddOption(TRUE_VALUE_PROPERTY);
		pProperty->AddOption(FALSE_VALUE_PROPERTY);
		pProperty->AllowEdit(FALSE);

		pViewGroup->AddSubItem(pProperty);
	}
#pragma endregion
	
#pragma region OpenGL
	auto ioglRender = dynamic_cast<_ioglRenderer*>(pOpenGLView);
	ASSERT(ioglRender != nullptr);

	auto pBlinnPhongProgram = ioglRender->_getOGLProgramAs<_oglBlinnPhongProgram>();
	if (pBlinnPhongProgram != nullptr)
	{
		auto pOpenGL = new CMFCPropertyGridProperty(_T("OpenGL"));
		pViewGroup->AddSubItem(pOpenGL);

#pragma region Point light position
		{
			auto pPointLightingLocation = new CApplicationProperty(_T("Point lighting location"),
				(DWORD_PTR)new CApplicationPropertyData(enumPropertyType::PointLightingLocation), TRUE);
			pPointLightingLocation->AllowEdit(FALSE);

			// X
			{
				auto pProperty = new CApplicationProperty(
					_T("X"),
					(_variant_t)pBlinnPhongProgram->_getPointLightingLocation().x,
					_T("X"),
					(DWORD_PTR)new CApplicationPropertyData(enumPropertyType::PointLightingLocation));
				pPointLightingLocation->AddSubItem(pProperty);
			}

			// Y
			{
				auto pProperty = new CApplicationProperty(
					_T("Y"),
					(_variant_t)pBlinnPhongProgram->_getPointLightingLocation().y,
					_T("Y"),
					(DWORD_PTR)new CApplicationPropertyData(enumPropertyType::PointLightingLocation));
				pPointLightingLocation->AddSubItem(pProperty);
			}

			// Z
			{
				auto pProperty = new CApplicationProperty(
					_T("Z"),
					(_variant_t)pBlinnPhongProgram->_getPointLightingLocation().z,
					_T("Z"),
					(DWORD_PTR)new CApplicationPropertyData(enumPropertyType::PointLightingLocation));
				pPointLightingLocation->AddSubItem(pProperty);
			}

			pOpenGL->AddSubItem(pPointLightingLocation);
		}
#pragma endregion

#pragma region Ambient light weighting
		{
			auto pAmbientLightWeighting = new CApplicationProperty(_T("Ambient light weighting"),
				(DWORD_PTR)new CApplicationPropertyData(enumPropertyType::AmbientLightWeighting), TRUE);
			pAmbientLightWeighting->AllowEdit(FALSE);

			// X
			{
				auto pProperty = new CApplicationProperty(
					_T("X"),
					(_variant_t)pBlinnPhongProgram->_getAmbientLightWeighting().x,
					_T("[0.0 - 1.0]"),
					(DWORD_PTR)new CApplicationPropertyData(enumPropertyType::AmbientLightWeighting));
				pAmbientLightWeighting->AddSubItem(pProperty);
			}

			// Y
			{
				auto pProperty = new CApplicationProperty(
					_T("Y"),
					(_variant_t)pBlinnPhongProgram->_getAmbientLightWeighting().y,
					_T("[0.0 - 1.0]"),
					(DWORD_PTR)new CApplicationPropertyData(enumPropertyType::AmbientLightWeighting));
				pAmbientLightWeighting->AddSubItem(pProperty);
			}

			// Z
			{
				auto pProperty = new CApplicationProperty(
					_T("Z"),
					(_variant_t)pBlinnPhongProgram->_getAmbientLightWeighting().z,
					_T("[0.0 - 1.0]"),
					(DWORD_PTR)new CApplicationPropertyData(enumPropertyType::AmbientLightWeighting));
				pAmbientLightWeighting->AddSubItem(pProperty);
			}

			pOpenGL->AddSubItem(pAmbientLightWeighting);
		}
#pragma endregion

#pragma region Diffuse light weighting
		{
			auto pDiffuseLightWeighting = new CApplicationProperty(_T("Diffuse light weighting"),
				(DWORD_PTR)new CApplicationPropertyData(enumPropertyType::DiffuseLightWeighting), TRUE);
			pDiffuseLightWeighting->AllowEdit(FALSE);

			// X
			{
				auto pProperty = new CApplicationProperty(
					_T("X"),
					(_variant_t)pBlinnPhongProgram->_getDiffuseLightWeighting().x,
					_T("[0.0 - 1.0]"),
					(DWORD_PTR)new CApplicationPropertyData(enumPropertyType::DiffuseLightWeighting));
				pDiffuseLightWeighting->AddSubItem(pProperty);
			}

			// X
			{
				auto pProperty = new CApplicationProperty(
					_T("Y"),
					(_variant_t)pBlinnPhongProgram->_getDiffuseLightWeighting().y,
					_T("[0.0 - 1.0]"),
					(DWORD_PTR)new CApplicationPropertyData(enumPropertyType::DiffuseLightWeighting));
				pDiffuseLightWeighting->AddSubItem(pProperty);
			}

			// Z
			{
				auto pProperty = new CApplicationProperty(
					_T("Z"),
					(_variant_t)pBlinnPhongProgram->_getDiffuseLightWeighting().z,
					_T("[0.0 - 1.0]"),
					(DWORD_PTR)new CApplicationPropertyData(enumPropertyType::DiffuseLightWeighting));
				pDiffuseLightWeighting->AddSubItem(pProperty);
			}

			pOpenGL->AddSubItem(pDiffuseLightWeighting);
		}
#pragma endregion

#pragma region Specular light weighting
		{
			auto pSpecularLightWeighting = new CApplicationProperty(_T("Specular light weighting"),
				(DWORD_PTR)new CApplicationPropertyData(enumPropertyType::SpecularLightWeighting), TRUE);
			pSpecularLightWeighting->AllowEdit(FALSE);

			// X
			{
				auto pProperty = new CApplicationProperty(
					_T("X"),
					(_variant_t)pBlinnPhongProgram->_getSpecularLightWeighting().x,
					_T("[0.0 - 1.0]"),
					(DWORD_PTR)new CApplicationPropertyData(enumPropertyType::SpecularLightWeighting));
				pSpecularLightWeighting->AddSubItem(pProperty);
			}

			// X
			{
				auto pProperty = new CApplicationProperty(
					_T("Y"),
					(_variant_t)pBlinnPhongProgram->_getSpecularLightWeighting().y,
					_T("[0.0 - 1.0]"),
					(DWORD_PTR)new CApplicationPropertyData(enumPropertyType::SpecularLightWeighting));
				pSpecularLightWeighting->AddSubItem(pProperty);
			}

			// Z
			{
				auto pProperty = new CApplicationProperty(
					_T("Z"),
					(_variant_t)pBlinnPhongProgram->_getSpecularLightWeighting().z,
					_T("[0.0 - 1.0]"),
					(DWORD_PTR)new CApplicationPropertyData(enumPropertyType::SpecularLightWeighting));
				pSpecularLightWeighting->AddSubItem(pProperty);
			}

			pOpenGL->AddSubItem(pSpecularLightWeighting);
		}
#pragma endregion

#pragma region Material shininess
		{
			auto pMaterialShininess = new CApplicationProperty(
				_T("Material shininess"),
				(_variant_t)pBlinnPhongProgram->_getMaterialShininess(),
				_T("[0.0 - 1.0]"),
				(DWORD_PTR)new CApplicationPropertyData(enumPropertyType::MaterialShininess));

			pOpenGL->AddSubItem(pMaterialShininess);
		}
#pragma endregion

#pragma region Contrast
		{
			auto pContrast = new CApplicationProperty(
				_T("Contrast"),
				(_variant_t)pBlinnPhongProgram->_getContrast(),
				_T("Contrast"),
				(DWORD_PTR)new CApplicationPropertyData(enumPropertyType::Contrast));

			pOpenGL->AddSubItem(pContrast);
		}
#pragma endregion

#pragma region Brightness
		{
			auto pBrightness = new CApplicationProperty(
				_T("Brightness"),
				(_variant_t)pBlinnPhongProgram->_getBrightness(),
				_T("Brightness"),
				(DWORD_PTR)new CApplicationPropertyData(enumPropertyType::Brightness));

			pOpenGL->AddSubItem(pBrightness);
		}
#pragma endregion

#pragma region Gamma
		{
			auto pGamma = new CApplicationProperty(
				_T("Gamma"),
				(_variant_t)pBlinnPhongProgram->_getGamma(),
				_T("Gamma"),
				(DWORD_PTR)new CApplicationPropertyData(enumPropertyType::Gamma));

			pOpenGL->AddSubItem(pGamma);
		}
#pragma endregion
	} // if (pBlinnPhongProgram != nullptr)
#pragma endregion

	m_wndPropList.AddProperty(pViewGroup);
}

// ------------------------------------------------------------------------------------------------
void CPropertiesWnd::LoadInstanceProperties()
{
	m_wndPropList.RemoveAll();
	m_wndPropList.AdjustLayout();

	SetPropListFont();

	m_wndPropList.EnableHeaderCtrl(FALSE);
	m_wndPropList.EnableDescriptionArea();
	m_wndPropList.SetVSDotNetLook();
	m_wndPropList.MarkModifiedProperties();

	auto pContoller = GetController();
	if (pContoller == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pSelectedInstance = GetController()->GetSelectedInstance() != nullptr ? dynamic_cast<CProductInstance*>(GetController()->GetSelectedInstance()) : nullptr;
	if (pSelectedInstance == nullptr)
	{
		return;
	}

	/*
	* Instance
	*/
	auto pInstanceGroup = new CMFCPropertyGridProperty(pSelectedInstance->getProductDefinition()->getId());	

	/*
	* Properties
	*/
	int_t* propertyDefinitionInstances = sdaiGetEntityExtentBN(pContoller->GetModel()->GetInstance(), "PROPERTY_DEFINITION"),
		noPropertyDefinitionInstances = sdaiGetMemberCount(propertyDefinitionInstances);
	for (int_t i = 0; i < noPropertyDefinitionInstances; i++) {
		int_t propertyDefinitionInstance = 0;
		engiGetAggrElement(propertyDefinitionInstances, i, sdaiINSTANCE, &propertyDefinitionInstance);

		int_t definitionInstance = 0;
		sdaiGetAttrBN(propertyDefinitionInstance, "definition", sdaiINSTANCE, &definitionInstance);
		if (definitionInstance == pSelectedInstance->getProductDefinition()->getInstance()) {
			CString strValue;
			strValue.Format(L"property (#%i = PROPERTY_DEFINITION( ... ))", (int)internalGetP21Line(propertyDefinitionInstance));

			auto pPropertyGroup = new CMFCPropertyGridProperty(strValue);
			pInstanceGroup->AddSubItem(pPropertyGroup);

			char* name = nullptr;
			sdaiGetAttrBN(propertyDefinitionInstance, "name", sdaiSTRING, &name);
			
			auto pProperty = new CMFCPropertyGridProperty(L"name", (_variant_t)name, L"name");
			pProperty->AllowEdit(FALSE);
			pPropertyGroup->AddSubItem(pProperty);

			char* description = nullptr;
			sdaiGetAttrBN(propertyDefinitionInstance, "description", sdaiSTRING, &description);

			pProperty = new CMFCPropertyGridProperty(L"description", (_variant_t)name, L"description");
			pProperty->AllowEdit(FALSE);
			pPropertyGroup->AddSubItem(pProperty);

			//
			//	Lookup value (not using inverse relations)
			//
			int_t* propertyDefinitionRepresentationInstances = sdaiGetEntityExtentBN(pContoller->GetModel()->GetInstance(), "PROPERTY_DEFINITION_REPRESENTATION"),
				noPropertyDefinitionRepresentationInstances = sdaiGetMemberCount(propertyDefinitionRepresentationInstances);
			for (int_t j = 0; j < noPropertyDefinitionRepresentationInstances; j++) {
				int_t propertyDefinitionRepresentationInstance = 0;
				engiGetAggrElement(propertyDefinitionRepresentationInstances, j, sdaiINSTANCE, &propertyDefinitionRepresentationInstance);

				int_t PDR_definitionInstance = 0;
				sdaiGetAttrBN(propertyDefinitionRepresentationInstance, "definition", sdaiINSTANCE, &PDR_definitionInstance);
				if (PDR_definitionInstance == propertyDefinitionInstance) {
					int_t representationInstance = 0;
					sdaiGetAttrBN(propertyDefinitionRepresentationInstance, "used_representation", sdaiINSTANCE, &representationInstance);

					int_t* aggrItems = nullptr;
					sdaiGetAttrBN(representationInstance, "items", sdaiAGGR, &aggrItems);
					int_t	noAggrItems = sdaiGetMemberCount(aggrItems);
					for (int_t k = 0; k < noAggrItems; k++) {
						int_t representationItemInstance = 0;
						engiGetAggrElement(aggrItems, k, sdaiINSTANCE, &representationItemInstance);

						if (sdaiGetInstanceType(representationItemInstance) == sdaiGetEntity(pContoller->GetModel()->GetInstance(), "DESCRIPTIVE_REPRESENTATION_ITEM")) {
							char* valueDescription = nullptr;
							sdaiGetAttrBN(representationItemInstance, "description", sdaiSTRING, &valueDescription);

							pProperty = new CMFCPropertyGridProperty(L"value", (_variant_t)valueDescription, L"value");
							pProperty->AllowEdit(FALSE);
							pPropertyGroup->AddSubItem(pProperty);
						}
						else if (sdaiGetInstanceType(representationItemInstance) == sdaiGetEntity(pContoller->GetModel()->GetInstance(), "VALUE_REPRESENTATION_ITEM")) {
							int_t* valueComponentADB = nullptr;
							sdaiGetAttrBN(representationItemInstance, "value_component", sdaiADB, &valueComponentADB);

							char* typePath = sdaiGetADBTypePath(valueComponentADB, 0);
							switch (sdaiGetADBType(valueComponentADB)) {
							case  sdaiINTEGER:
							{
								int_t	value = 0;
								sdaiGetADBValue(valueComponentADB, sdaiINTEGER, (void*)&value);

								strValue.Format(L"%i [%s]", (int)value, (LPCWSTR)CA2W(typePath));

								pProperty = new CMFCPropertyGridProperty(L"value", (_variant_t)(LPCTSTR)strValue, L"value");
								pProperty->AllowEdit(FALSE);
								pPropertyGroup->AddSubItem(pProperty);
							}
							break;
							case  sdaiREAL:
							{
								double	value = 0;
								sdaiGetADBValue(valueComponentADB, sdaiREAL, (void*)&value);

								strValue.Format(L"%f [%s]", value, (LPCWSTR)CA2W(typePath));

								pProperty = new CMFCPropertyGridProperty(L"value", (_variant_t)(LPCTSTR)strValue, L"value");
								pProperty->AllowEdit(FALSE);
								pPropertyGroup->AddSubItem(pProperty);
							}
							break;
							case  sdaiSTRING:
							{
								char* value = nullptr;
								sdaiGetADBValue(valueComponentADB, sdaiSTRING, (void*)&value);

								strValue.Format(L"%s [%s]", (LPCWSTR)CA2W(value), (LPCWSTR)CA2W(typePath));

								pProperty = new CMFCPropertyGridProperty(L"value", (_variant_t)(LPCTSTR)strValue, L"value");
								pProperty->AllowEdit(FALSE);
								pPropertyGroup->AddSubItem(pProperty);
							}
							break;
							default:
								ASSERT(false);
								break;
							}
						}
						else {							
							//ASSERT(false);
						}
					}
				}
			}
		}
	}

	m_wndPropList.AddProperty(pInstanceGroup);	
}

// ------------------------------------------------------------------------------------------------
//void CPropertiesWnd::AddInstanceProperty(CMFCPropertyGridProperty * pInstanceGroup, CRDFInstance * pRDFInstance, CRDFProperty * pRDFProperty)
//{
//	CMFCPropertyGridProperty * pPropertyGroup = new CMFCPropertyGridProperty(pRDFProperty->getName());
//	pInstanceGroup->AddSubItem(pPropertyGroup);
//
//	/*
//	* range
//	*/
//	if (pRDFProperty->getType() == TYPE_OBJECTTYPE)
//	{
//		ASSERT(GetController() != nullptr);
//
//		CObjectRDFProperty * pObjectRDFProperty = dynamic_cast<CObjectRDFProperty *>(pRDFProperty);
//		ASSERT(pObjectRDFProperty != nullptr);
//
//		wstring strRange;
//
//		const vector<int64_t> & vecRestrictions = pObjectRDFProperty->getRestrictions();
//		ASSERT(!vecRestrictions.empty());
//
//		for (size_t iRestriction = 0; iRestriction < vecRestrictions.size(); iRestriction++)
//		{
//			char * szClassName = nullptr;
//			GetNameOfClass(vecRestrictions[iRestriction], &szClassName);
//
//			if (!strRange.empty())
//			{
//				strRange += L"; ";
//			}
//
//			strRange += CA2W(szClassName);
//		}
//
//		CMFCPropertyGridProperty * pRange = new CMFCPropertyGridProperty(L"rdfs:range", (_variant_t)strRange.c_str(), pRDFProperty->getName());
//		pRange->AllowEdit(FALSE);
//
//		pPropertyGroup->AddSubItem(pRange);
//	} // if (pRDFProperty->getType() == TYPE_OBJECTTYPE)
//	else
//	{
//		CMFCPropertyGridProperty * pRange = new CMFCPropertyGridProperty(L"rdfs:range", (_variant_t)pRDFProperty->getRange().c_str(), pRDFProperty->getName());
//		pRange->AllowEdit(FALSE);
//
//		pPropertyGroup->AddSubItem(pRange);
//	}	
//
//	/*
//	* cardinality
//	*/
//	AddInstancePropertyCardinality(pPropertyGroup, pRDFInstance, pRDFProperty);
//
//	/*
//	* values
//	*/
//	AddInstancePropertyValues(pPropertyGroup, pRDFInstance, pRDFProperty);	
//}

// ------------------------------------------------------------------------------------------------
//void CPropertiesWnd::AddInstancePropertyCardinality(CMFCPropertyGridProperty * pPropertyGroup, CRDFInstance * pRDFInstance, CRDFProperty * pRDFProperty)
//{
//	switch (pRDFProperty->getType())
//	{
//	case TYPE_OBJECTTYPE:
//	{
//		int64_t * piObjectInstances = nullptr;
//		int64_t iCard = 0;
//		GetObjectProperty(pRDFInstance->getInstance(), pRDFProperty->getInstance(), &piObjectInstances, &iCard);
//
//		/*
//		* owl:cardinality
//		*/
//		wstring strCardinality = pRDFProperty->getCardinality(pRDFInstance->getInstance());
//
//		CAddRDFInstanceProperty * pCardinality = new CAddRDFInstanceProperty(L"owl:cardinality", (_variant_t)strCardinality.c_str(), pRDFProperty->getName(),
//			(DWORD_PTR)new CRDFInstancePropertyData(GetController(), pRDFInstance, pRDFProperty, iCard));
//		pCardinality->AllowEdit(FALSE);
//
//		pPropertyGroup->AddSubItem(pCardinality);
//	} // case TYPE_OBJECTTYPE:
//	break;
//
//	case TYPE_BOOL_DATATYPE:
//	{
//		int64_t iCard = 0;
//		bool* pbValue = nullptr;
//		GetDatatypeProperty(pRDFInstance->getInstance(), pRDFProperty->getInstance(), (void**)&pbValue, &iCard);
//
//		/*
//		* owl:cardinality
//		*/
//		wstring strCardinality = pRDFProperty->getCardinality(pRDFInstance->getInstance());
//
//		CAddRDFInstanceProperty* pCardinality = new CAddRDFInstanceProperty(L"owl:cardinality", (_variant_t)strCardinality.c_str(), pRDFProperty->getName(),
//			(DWORD_PTR)new CRDFInstancePropertyData(GetController(), pRDFInstance, pRDFProperty, iCard));
//		pCardinality->AllowEdit(FALSE);
//
//		pPropertyGroup->AddSubItem(pCardinality);
//	} // case TYPE_BOOL_DATATYPE:
//	break;
//
//	case TYPE_CHAR_DATATYPE:
//	{
//		int64_t iCard = 0;
//		char ** szValue = nullptr;
//		GetDatatypeProperty(pRDFInstance->getInstance(), pRDFProperty->getInstance(), (void **)&szValue, &iCard);
//
//		/*
//		* owl:cardinality
//		*/
//		wstring strCardinality = pRDFProperty->getCardinality(pRDFInstance->getInstance());
//
//		CAddRDFInstanceProperty * pCardinality = new CAddRDFInstanceProperty(L"owl:cardinality", (_variant_t)strCardinality.c_str(), pRDFProperty->getName(),
//			(DWORD_PTR)new CRDFInstancePropertyData(GetController(), pRDFInstance, pRDFProperty, iCard));
//		pCardinality->AllowEdit(FALSE);
//
//		pPropertyGroup->AddSubItem(pCardinality);
//	} // case TYPE_CHAR_DATATYPE:
//	break;
//
//	case TYPE_DOUBLE_DATATYPE:
//	{
//		int64_t iCard = 0;
//		double * pdValue = nullptr;
//		GetDatatypeProperty(pRDFInstance->getInstance(), pRDFProperty->getInstance(), (void **)&pdValue, &iCard);
//
//		/*
//		* owl:cardinality
//		*/
//		wstring strCardinality = pRDFProperty->getCardinality(pRDFInstance->getInstance());
//
//		CAddRDFInstanceProperty * pCardinality = new CAddRDFInstanceProperty(L"owl:cardinality", (_variant_t)strCardinality.c_str(), pRDFProperty->getName(),
//			(DWORD_PTR)new CRDFInstancePropertyData(GetController(), pRDFInstance, pRDFProperty, iCard));
//		pCardinality->AllowEdit(FALSE);
//
//		pPropertyGroup->AddSubItem(pCardinality);
//	} // case TYPE_DOUBLE_DATATYPE:
//	break;
//
//	case TYPE_INT_DATATYPE:
//	{
//		int64_t iCard = 0;
//		int64_t * piValue = nullptr;
//		GetDatatypeProperty(pRDFInstance->getInstance(), pRDFProperty->getInstance(), (void **)&piValue, &iCard);
//
//		/*
//		* owl:cardinality
//		*/
//		wstring strCardinality = pRDFProperty->getCardinality(pRDFInstance->getInstance());
//
//		CAddRDFInstanceProperty * pCardinality = new CAddRDFInstanceProperty(L"owl:cardinality", (_variant_t)strCardinality.c_str(), pRDFProperty->getName(),
//			(DWORD_PTR)new CRDFInstancePropertyData(GetController(), pRDFInstance, pRDFProperty, iCard));
//		pCardinality->AllowEdit(FALSE);
//
//		pPropertyGroup->AddSubItem(pCardinality);
//	} // case TYPE_INT_DATATYPE:
//	break;
//
//	default:
//	{
//		ASSERT(false); // unknown property
//	}
//	break;
//	} // switch (pRDFProperty->getType())
//}

// ------------------------------------------------------------------------------------------------
// Moved in CSelectInstanceDialog
//bool IsUsedRecursively(int64_t RDFInstanceI, int64_t RDFInstanceII)
//{
//	int64_t myInstance = GetInstanceInverseReferencesByIterator(RDFInstanceII, 0);
//	while (myInstance) {
//		if ((myInstance == RDFInstanceI) ||
//			IsUsedRecursively(RDFInstanceI, myInstance)) {
//			return true;
//		}
//		myInstance = GetInstanceInverseReferencesByIterator(RDFInstanceII, myInstance);
//	}
//
//	return false;
//}

// ------------------------------------------------------------------------------------------------
//void CPropertiesWnd::AddInstancePropertyValues(CMFCPropertyGridProperty * pPropertyGroup, CRDFInstance * pRDFInstance, CRDFProperty * pRDFProperty)
//{
//	switch (pRDFProperty->getType())
//	{
//	case TYPE_OBJECTTYPE:
//	{	
//		LOG_DEBUG("CPropertiesWnd::AddInstancePropertyValues() - TYPE_OBJECTTYPE BEGIN");
//
//		int64_t * piInstances = nullptr;
//		int64_t iCard = 0;
//		GetObjectProperty(pRDFInstance->getInstance(), pRDFProperty->getInstance(), &piInstances, &iCard);
//
//		if (iCard > 0)
//		{
//			int64_t	iMinCard = 0;
//			int64_t iMaxCard = 0;
//			pRDFProperty->GetRestrictions(pRDFInstance->getInstance(), iMinCard, iMaxCard);
//
//			if ((iMinCard == -1) && (iMaxCard == -1))
//			{
//				iMinCard = 0;
//			}
//
//			ASSERT(GetController() != nullptr);
//
//			CSTEPModel * pModel = GetController()->GetModel();
//			ASSERT(pModel != nullptr);
//
//			const map<int64_t, CRDFInstance *> & mapRFDInstances = pModel->GetRDFInstances();
//
//			/*
//			* Compatible instances
//			*/
//			// Moved in CSelectInstanceDialog
//			//CObjectRDFProperty * pObjectRDFProperty = dynamic_cast<CObjectRDFProperty *>(pRDFProperty);
//			//ASSERT(pObjectRDFProperty != nullptr);
//
//			// Moved in CSelectInstanceDialog
//			//vector<int64_t> vecCompatibleInstances;
//			//pModel->GetCompatibleInstances(pRDFInstance, pObjectRDFProperty, vecCompatibleInstances);
//
//			int64_t iValuesCount = iCard;
//			for (int64_t iValue = 0; iValue < iValuesCount; iValue++)
//			{
//				CRDFInstanceObjectProperty * pProperty = nullptr;
//
//				if (piInstances[iValue] != 0)
//				{
//					map<int64_t, CRDFInstance *>::const_iterator itInstanceValue = mapRFDInstances.find(piInstances[iValue]);
//					ASSERT(itInstanceValue != mapRFDInstances.end());
//
//					pProperty = new CRDFInstanceObjectProperty(L"value", (_variant_t)itInstanceValue->second->getUniqueName(), pRDFProperty->getName(),
//						(DWORD_PTR)new CRDFInstancePropertyData(GetController(), pRDFInstance, pRDFProperty, iValue));
//				}
//				else
//				{
//					pProperty = new CRDFInstanceObjectProperty(L"value", (_variant_t)EMPTY_INSTANCE, pRDFProperty->getName(),
//						(DWORD_PTR)new CRDFInstancePropertyData(GetController(), pRDFInstance, pRDFProperty, iValue));
//				}
//
//				// Moved in CSelectInstanceDialog
//				//LOG_DEBUG("Compatible instances: " << (int)vecCompatibleInstances.size());
//
//				//for (size_t iCompatibleInstance = 0; iCompatibleInstance < vecCompatibleInstances.size(); iCompatibleInstance++)
//				//{
//				//	map<int64_t, CRDFInstance *>::const_iterator itInstanceValue = mapRFDInstances.find(vecCompatibleInstances[iCompatibleInstance]);
//				//	ASSERT(itInstanceValue != mapRFDInstances.end());
//
//				//	wstring strInstanceUniqueName = itInstanceValue->second->getUniqueName();
//				//	if ((piInstances[iValue] != vecCompatibleInstances[iCompatibleInstance]) && IsUsedRecursively(pRDFInstance->getInstance(), itInstanceValue->second->getInstance()))
//				//	{
//				//		strInstanceUniqueName += USED_SUFFIX;
//				//	}
//
//				//	pProperty->AddOption(strInstanceUniqueName.c_str());
//				//	pProperty->AddValue(strInstanceUniqueName.c_str(), vecCompatibleInstances[iCompatibleInstance]);
//				//} // for (size_t iCompatibleInstance = ...
//
//				/*
//				* Empty command
//				*/
//				pProperty->AddOption(EMPTY_INSTANCE);
//				pProperty->AddValue(EMPTY_INSTANCE, 0);
//
//				/*
//				* Remove command
//				*/
//				if (iCard > iMinCard)
//				{
//					pProperty->AddOption(REMOVE_OBJECT_PROPERTY_COMMAND);
//				}				
//
//				/*
//				* Select
//				*/
//				pProperty->AddOption(SELECT_OBJECT_PROPERTY_COMMAND);
//
//				pProperty->AllowEdit(FALSE);
//
//				pPropertyGroup->AddSubItem(pProperty);
//
//				if ((iValue + 1) >= GetController()->GetVisibleValuesCountLimit())
//				{
//					break;
//				}
//			} // for (int64_t iValue = ...
//
//			if (iValuesCount > GetController()->GetVisibleValuesCountLimit())
//			{
//				CMFCPropertyGridProperty * pProperty = new CMFCPropertyGridProperty(L"...", (_variant_t)L"...", pRDFProperty->getName());
//				pProperty->AllowEdit(FALSE);
//
//				pPropertyGroup->AddSubItem(pProperty);
//			}
//		} // if (iCard > 0)
//
//		LOG_DEBUG("CPropertiesWnd::AddInstancePropertyValues() - TYPE_OBJECTTYPE END");
//	} // case TYPE_OBJECTTYPE:
//	break;
//
//	case TYPE_BOOL_DATATYPE:
//	{
//		int64_t iCard = 0;
//		bool* pbValue = nullptr;
//		GetDatatypeProperty(pRDFInstance->getInstance(), pRDFProperty->getInstance(), (void**)&pbValue, &iCard);
//
//		if (iCard > 0)
//		{
//			int64_t iValuesCount = iCard;
//			for (int64_t iValue = 0; iValue < iValuesCount; iValue++)
//			{
//				CRDFInstanceProperty* pProperty = new CRDFInstanceProperty(L"value", (_variant_t)pbValue[iValue], pRDFProperty->getName(),
//					(DWORD_PTR)new CRDFInstancePropertyData(GetController(), pRDFInstance, pRDFProperty, iValue));
//
//				pPropertyGroup->AddSubItem(pProperty);
//
//				if ((iValue + 1) >= GetController()->GetVisibleValuesCountLimit())
//				{
//					break;
//				}
//			} // for (int64_t iValue = ...
//
//			if (iValuesCount > GetController()->GetVisibleValuesCountLimit())
//			{
//				CMFCPropertyGridProperty* pProperty = new CMFCPropertyGridProperty(L"...", (_variant_t)L"...", pRDFProperty->getName());
//				pProperty->AllowEdit(FALSE);
//
//				pPropertyGroup->AddSubItem(pProperty);
//			}
//		} // if (iCard > 0)
//	} // case TYPE_BOOL_DATATYPE:
//	break;
//
//	case TYPE_CHAR_DATATYPE:
//	{
//		int64_t iCard = 0;
//		char ** szValue = nullptr;
//		GetDatatypeProperty(pRDFInstance->getInstance(), pRDFProperty->getInstance(), (void **)&szValue, &iCard);
//
//		if (iCard > 0)
//		{
//			int64_t iValuesCount = iCard;
//			for (int64_t iValue = 0; iValue < iValuesCount; iValue++)
//			{
//				CRDFInstanceProperty * pProperty = new CRDFInstanceProperty(L"value", (_variant_t)szValue[iValue], pRDFProperty->getName(),
//					(DWORD_PTR)new CRDFInstancePropertyData(GetController(), pRDFInstance, pRDFProperty, iValue));
//
//				pPropertyGroup->AddSubItem(pProperty);
//
//				if ((iValue + 1) >= GetController()->GetVisibleValuesCountLimit())
//				{
//					break;
//				}
//			} // for (int64_t iValue = ...
//
//			if (iValuesCount > GetController()->GetVisibleValuesCountLimit())
//			{
//				CMFCPropertyGridProperty * pProperty = new CMFCPropertyGridProperty(L"...", (_variant_t)L"...", pRDFProperty->getName());
//				pProperty->AllowEdit(FALSE);
//
//				pPropertyGroup->AddSubItem(pProperty);
//			}
//		} // if (iCard > 0)
//	} // case TYPE_CHAR_DATATYPE:
//	break;
//
//	case TYPE_DOUBLE_DATATYPE:
//	{
//		int64_t iCard = 0;
//		double * pdValue = nullptr;
//		GetDatatypeProperty(pRDFInstance->getInstance(), pRDFProperty->getInstance(), (void **)&pdValue, &iCard);
//
//		if (iCard > 0)
//		{
//			int64_t iValuesCount = iCard;
//			for (int64_t iValue = 0; iValue < iValuesCount; iValue++)
//			{
//				CRDFInstanceProperty * pProperty = new CRDFInstanceProperty(L"value", (_variant_t)pdValue[iValue], pRDFProperty->getName(),
//					(DWORD_PTR)new CRDFInstancePropertyData(GetController(), pRDFInstance, pRDFProperty, iValue));
//
//				pPropertyGroup->AddSubItem(pProperty);
//
//				if ((iValue + 1) >= GetController()->GetVisibleValuesCountLimit())
//				{
//					break;
//				}
//			} // for (int64_t iValue = ...
//
//			if (iValuesCount > GetController()->GetVisibleValuesCountLimit())
//			{
//				CMFCPropertyGridProperty * pProperty = new CMFCPropertyGridProperty(L"...", (_variant_t)L"...", pRDFProperty->getName());
//				pProperty->AllowEdit(FALSE);
//
//				pPropertyGroup->AddSubItem(pProperty);
//			}
//		} // if (iCard > 0)
//	} // case TYPE_DOUBLE_DATATYPE:
//	break;
//
//	case TYPE_INT_DATATYPE:
//	{
//		int64_t iCard = 0;
//		int64_t * piValue = nullptr;
//		GetDatatypeProperty(pRDFInstance->getInstance(), pRDFProperty->getInstance(), (void **)&piValue, &iCard);
//
//		if (iCard > 0)
//		{
//			int64_t iValuesCount = iCard;
//			for (int64_t iValue = 0; iValue < iValuesCount; iValue++)
//			{
//				CRDFInstanceProperty * pProperty = new CRDFInstanceProperty(L"value", (_variant_t)piValue[iValue], pRDFProperty->getName(),
//					(DWORD_PTR)new CRDFInstancePropertyData(GetController(), pRDFInstance, pRDFProperty, iValue));
//				pProperty->EnableSpinControlInt64();
//
//				pPropertyGroup->AddSubItem(pProperty);
//
//				if ((iValue + 1) >= GetController()->GetVisibleValuesCountLimit())
//				{
//					break;
//				}
//			} // for (int64_t iValue = ...
//
//			if (iValuesCount > GetController()->GetVisibleValuesCountLimit())
//			{
//				CMFCPropertyGridProperty * pProperty = new CMFCPropertyGridProperty(L"...", (_variant_t)L"...", pRDFProperty->getName());
//				pProperty->AllowEdit(FALSE);
//
//				pPropertyGroup->AddSubItem(pProperty);
//			}
//		} // if (iCard > 0)
//	} // case TYPE_INT_DATATYPE:
//	break;
//
//	default:
//	{
//		ASSERT(false); // unknown property
//	}
//	break;
//	} // switch (pRDFProperty->getType())
//}

// ------------------------------------------------------------------------------------------------
void CPropertiesWnd::LoadMetaInformation()
{
	ASSERT(0); // todo

	//m_wndPropList.RemoveAll();
	//m_wndPropList.AdjustLayout();

	//SetPropListFont();

	//m_wndPropList.EnableHeaderCtrl(FALSE);
	//m_wndPropList.EnableDescriptionArea();
	//m_wndPropList.SetVSDotNetLook();
	//m_wndPropList.MarkModifiedProperties();

	//ASSERT(GetController() != nullptr);

	//CRDFInstance * pRDFInstance = GetController()->GetSelectedInstance();
	//if (pRDFInstance == nullptr)
	//{
	//	return;
	//}

	//CMFCPropertyGridProperty * pMetaInfoGroup = new CMFCPropertyGridProperty(L"Meta information");
	//CMFCPropertyGridProperty * pInstanceGroup = new CMFCPropertyGridProperty(pRDFInstance->getName());
	//pMetaInfoGroup->AddSubItem(pInstanceGroup);

	///*
	//* Centroid, Volume
	//*/
	//{
	//	double arCentroid[3];
	//	double dVolume = GetCentroid(pRDFInstance->getInstance(), nullptr, nullptr, arCentroid);

	//	wchar_t szBuffer[100];
	//	swprintf(szBuffer, 100, L"%.6f, %.6f, %.6f", arCentroid[0], arCentroid[1], arCentroid[2]);

	//	CMFCPropertyGridProperty * pCentroid = new CMFCPropertyGridProperty(L"Centroid", (_variant_t)szBuffer, L"Centroid");
	//	pCentroid->AllowEdit(FALSE);

	//	pInstanceGroup->AddSubItem(pCentroid);

	//	swprintf(szBuffer, 100, L"%.6f", dVolume);

	//	CMFCPropertyGridProperty * pVolume = new CMFCPropertyGridProperty(L"Volume", (_variant_t)szBuffer, L"Volume");
	//	pVolume->AllowEdit(FALSE);

	//	pInstanceGroup->AddSubItem(pVolume);
	//}	

	///*
	//* Area
	//*/
	//{
	//	double dArea = GetArea(pRDFInstance->getInstance(), nullptr, nullptr);

	//	wchar_t szBuffer[100];
	//	swprintf(szBuffer, 100, L"%.6f", dArea);

	//	CMFCPropertyGridProperty * pProperty = new CMFCPropertyGridProperty(L"Area", (_variant_t)szBuffer, L"Area");
	//	pProperty->AllowEdit(FALSE);

	//	pInstanceGroup->AddSubItem(pProperty);
	//}

	///*
	//* Perimeter
	//*/
	//{
	//	double dPerimeter = GetPerimeter(pRDFInstance->getInstance());

	//	wchar_t szBuffer[100];
	//	swprintf(szBuffer, 100, L"%.6f", dPerimeter);

	//	CMFCPropertyGridProperty * pProperty = new CMFCPropertyGridProperty(L"Perimeter", (_variant_t)szBuffer, L"Perimeter");
	//	pProperty->AllowEdit(FALSE);

	//	pInstanceGroup->AddSubItem(pProperty);
	//}

	///*
	//* Geometry
	//*/
	//CMFCPropertyGridProperty * pGeometryGroup = new CMFCPropertyGridProperty(L"Geometry");
	//pMetaInfoGroup->AddSubItem(pGeometryGroup);

	///*
	//* Geometry - Number of conceptual faces
	//*/
	//{
	//	int64_t iConceptualFacesCount = pRDFInstance->getConceptualFacesCount();

	//	wchar_t szBuffer[100];
	//	swprintf(szBuffer, 100, L"%lld", iConceptualFacesCount);

	//	CMFCPropertyGridProperty * pProperty = new CMFCPropertyGridProperty(L"Number of conceptual faces", (_variant_t)szBuffer, L"Number of conceptual faces");
	//	pProperty->AllowEdit(FALSE);

	//	pGeometryGroup->AddSubItem(pProperty);
	//}


	///*
	//* Geometry - Number of triangles
	//*/
	//{
	//	int64_t iTrianglesCount = 0;
	//	for (size_t iTriangle = 0; iTriangle < pRDFInstance->getTriangles().size(); iTriangle++)
	//	{
	//		iTrianglesCount += pRDFInstance->getTriangles()[iTriangle].second / 3;
	//	}

	//	wchar_t szBuffer[100];
	//	swprintf(szBuffer, 100, L"%lld", iTrianglesCount);

	//	CMFCPropertyGridProperty * pProperty = new CMFCPropertyGridProperty(L"Number of triangles", (_variant_t)szBuffer, L"Number of triangles");
	//	pProperty->AllowEdit(FALSE);

	//	pGeometryGroup->AddSubItem(pProperty);
	//}

	///*
	//* Geometry - Number of lines
	//*/
	//{
	//	int64_t iLinesCount = 0;
	//	for (size_t iLine = 0; iLine < pRDFInstance->getLines().size(); iLine++)
	//	{
	//		iLinesCount += pRDFInstance->getLines()[iLine].second / 2;
	//	}

	//	wchar_t szBuffer[100];
	//	swprintf(szBuffer, 100, L"%lld", iLinesCount);

	//	CMFCPropertyGridProperty * pProperty = new CMFCPropertyGridProperty(L"Number of lines", (_variant_t)szBuffer, L"Number of lines");
	//	pProperty->AllowEdit(FALSE);

	//	pGeometryGroup->AddSubItem(pProperty);
	//}

	///*
	//* Geometry - Number of points
	//*/
	//{
	//	int64_t iPointsCount = pRDFInstance->getPoints().size();

	//	wchar_t szBuffer[100];
	//	swprintf(szBuffer, 100, L"%lld", iPointsCount);

	//	CMFCPropertyGridProperty * pProperty = new CMFCPropertyGridProperty(L"Number of points", (_variant_t)szBuffer, L"Number of points");
	//	pProperty->AllowEdit(FALSE);

	//	pGeometryGroup->AddSubItem(pProperty);
	//}

	///*
	//*   bit 0:   Check Design Tree Consistency
	//*   bit 1:   Check Consistency for Triangle Output (through API)
	//*   bit 2:   Check Consistency for Line Output (through API)
	//*   bit 3:   Check Consistency for Point Output (through API)
	//*   bit 4:   Check Consistency for Generated Surfaces (through API)
	//*   bit 5:   Check Consistency for Generated Surfaces (internal)
	//*   bit 6:   Check Consistency for Generated Solids (through API)
	//*   bit 7:   Check Consistency for Generated Solids (internal)
	//*   bit 8:   Check Consistency for BoundingBox's
	//*   bit 9:   Check Consistency for Triangulation
	//*   bit 10:  Check Consistency for Relations (through API)
	//*
	//*   bit 16:   Contains (Closed) Solid(s)
	//*   bit 18:   Contains (Closed) Infinite Solid(s)
	//*   bit 20:   Contains Closed Surface(s)
	//*   bit 21:   Contains Open Surface(s)
	//*   bit 22:   Contains Closed Infinite Surface(s)
	//*   bit 23:   Contains Open Infinite Surface(s)
	//*   bit 24:   Contains Closed Line(s)
	//*   bit 25:   Contains Open Line(s)
	//*   bit 26:   Contains Closed Infinite Line(s) [i.e. both ends in infinity]
	//*   bit 27:   Contains Open Infinite Line(s) [i.e. one end in infinity]
	//*   bit 28:   Contains (Closed) Point(s)
	//*/

	///*
	//* Consistency
	//*/
	//CMFCPropertyGridProperty * pConsistencyGroup = new CMFCPropertyGridProperty(L"Consistency");
	//pMetaInfoGroup->AddSubItem(pConsistencyGroup);

	///*
	//*   bit 0:   Check Design Tree Consistency
	//*/
	//{
	//	wchar_t* szResult = CheckInstanceConsistency(pRDFInstance->getInstance(), flagbit0) > 0 ? L"FAILED" : L"OK";

	//	CMFCPropertyGridProperty* pProperty = new CMFCPropertyGridProperty(L"Check Design Tree Consistency", (_variant_t)szResult, L"Check Design Tree Consistency");
	//	pProperty->AllowEdit(FALSE);

	//	pConsistencyGroup->AddSubItem(pProperty);
	//}

	///*
	//*   bit 1:   Check Consistency for Triangle Output (through API)
	//*/
	//{
	//	wchar_t* szResult = CheckInstanceConsistency(pRDFInstance->getInstance(), flagbit1) > 0 ? L"FAILED" : L"OK";

	//	CMFCPropertyGridProperty* pProperty = new CMFCPropertyGridProperty(L"Check Consistency for Triangle Output (through API)", (_variant_t)szResult, L"Check Consistency for Triangle Output (through API)");
	//	pProperty->AllowEdit(FALSE);

	//	pConsistencyGroup->AddSubItem(pProperty);
	//}

	///*
	//*   bit 2:   Check Consistency for Line Output (through API)
	//*/
	//{
	//	wchar_t* szResult = CheckInstanceConsistency(pRDFInstance->getInstance(), flagbit2) > 0 ? L"FAILED" : L"OK";

	//	CMFCPropertyGridProperty* pProperty = new CMFCPropertyGridProperty(L"Check Consistency for Line Output (through API)", (_variant_t)szResult, L"Check Consistency for Line Output (through API)");
	//	pProperty->AllowEdit(FALSE);

	//	pConsistencyGroup->AddSubItem(pProperty);
	//}

	///*
	//*   bit 3:   Check Consistency for Point Output (through API)
	//*/
	//{
	//	wchar_t* szResult = CheckInstanceConsistency(pRDFInstance->getInstance(), flagbit3) > 0 ? L"FAILED" : L"OK";

	//	CMFCPropertyGridProperty* pProperty = new CMFCPropertyGridProperty(L"Check Consistency for Point Output (through API)", (_variant_t)szResult, L"Check Consistency for Point Output (through API)");
	//	pProperty->AllowEdit(FALSE);

	//	pConsistencyGroup->AddSubItem(pProperty);
	//}

	///*
	//*   bit 4:   Check Consistency for Generated Surfaces (through API)
	//*/
	//{
	//	wchar_t* szResult = CheckInstanceConsistency(pRDFInstance->getInstance(), flagbit4) > 0 ? L"FAILED" : L"OK";

	//	CMFCPropertyGridProperty* pProperty = new CMFCPropertyGridProperty(L"Check Consistency for Generated Surfaces (through API)", (_variant_t)szResult, L"Check Consistency for Generated Surfaces (through API)");
	//	pProperty->AllowEdit(FALSE);

	//	pConsistencyGroup->AddSubItem(pProperty);
	//}

	///*
	//*   bit 5:   Check Consistency for Generated Surfaces (internal)
	//*/
	//{
	//	wchar_t* szResult = CheckInstanceConsistency(pRDFInstance->getInstance(), flagbit5) > 0 ? L"FAILED" : L"OK";

	//	CMFCPropertyGridProperty* pProperty = new CMFCPropertyGridProperty(L"Check Consistency for Generated Surfaces (internal)", (_variant_t)szResult, L"Check Consistency for Generated Surfaces (internal)");
	//	pProperty->AllowEdit(FALSE);

	//	pConsistencyGroup->AddSubItem(pProperty);
	//}

	///*
	//*   bit 6:   Check Consistency for Generated Solids (through API)
	//*/
	//{
	//	wchar_t* szResult = CheckInstanceConsistency(pRDFInstance->getInstance(), flagbit6) > 0 ? L"FAILED" : L"OK";

	//	CMFCPropertyGridProperty* pProperty = new CMFCPropertyGridProperty(L"Check Consistency for Generated Solids (through API)", (_variant_t)szResult, L"Check Consistency for Generated Solids (through API)");
	//	pProperty->AllowEdit(FALSE);

	//	pConsistencyGroup->AddSubItem(pProperty);
	//}

	///*
	//*   bit 7:   Check Consistency for Generated Solids (internal)
	//*/
	//{
	//	wchar_t* szResult = CheckInstanceConsistency(pRDFInstance->getInstance(), flagbit7) > 0 ? L"FAILED" : L"OK";

	//	CMFCPropertyGridProperty* pProperty = new CMFCPropertyGridProperty(L"Check Consistency for Generated Solids (internal)", (_variant_t)szResult, L"Check Consistency for Generated Solids (internal)");
	//	pProperty->AllowEdit(FALSE);

	//	pConsistencyGroup->AddSubItem(pProperty);
	//}

	///*
	//*   bit 8:   Check Consistency for BoundingBox's
	//*/
	//{
	//	wchar_t* szResult = CheckInstanceConsistency(pRDFInstance->getInstance(), flagbit8) > 0 ? L"FAILED" : L"OK";

	//	CMFCPropertyGridProperty* pProperty = new CMFCPropertyGridProperty(L"Check Consistency for BoundingBox's", (_variant_t)szResult, L"Check Consistency for BoundingBox's");
	//	pProperty->AllowEdit(FALSE);

	//	pConsistencyGroup->AddSubItem(pProperty);
	//}

	///*
	//*   bit 9:   Check Consistency for Triangulation
	//*/
	//{
	//	wchar_t* szResult = CheckInstanceConsistency(pRDFInstance->getInstance(), flagbit9) > 0 ? L"FAILED" : L"OK";

	//	CMFCPropertyGridProperty* pProperty = new CMFCPropertyGridProperty(L"Check Consistency for Triangulation", (_variant_t)szResult, L"Check Consistency for Triangulation");
	//	pProperty->AllowEdit(FALSE);

	//	pConsistencyGroup->AddSubItem(pProperty);
	//}

	///*
	//*   bit 10:  Check Consistency for Relations (through API)
	//*/
	//{
	//	wchar_t* szResult = CheckInstanceConsistency(pRDFInstance->getInstance(), flagbit10) > 0 ? L"FAILED" : L"OK";

	//	CMFCPropertyGridProperty* pProperty = new CMFCPropertyGridProperty(L"Check Consistency for Relations (through API)", (_variant_t)szResult, L"Check Consistency for Relations (through API)");
	//	pProperty->AllowEdit(FALSE);

	//	pConsistencyGroup->AddSubItem(pProperty);
	//}

	///*
	//*   bit 16:   Contains (Closed) Solid(s)
	//*/
	//{
	//	wchar_t* szResult = CheckInstanceConsistency(pRDFInstance->getInstance(), flagbit16) > 0 ? L"YES" : L"-";

	//	CMFCPropertyGridProperty* pProperty = new CMFCPropertyGridProperty(L"Contains (Closed) Solid(s)", (_variant_t)szResult, L"Contains (Closed) Solid(s)");
	//	pProperty->AllowEdit(FALSE);

	//	pConsistencyGroup->AddSubItem(pProperty);
	//}

	///*
	//*   bit 18:   Contains (Closed) Infinite Solid(s)
	//*/
	//{
	//	wchar_t* szResult = CheckInstanceConsistency(pRDFInstance->getInstance(), flagbit18) > 0 ? L"YES" : L"-";

	//	CMFCPropertyGridProperty* pProperty = new CMFCPropertyGridProperty(L"Contains (Closed) Infinite Solid(s)", (_variant_t)szResult, L"Contains (Closed) Infinite Solid(s)");
	//	pProperty->AllowEdit(FALSE);

	//	pConsistencyGroup->AddSubItem(pProperty);
	//}

	///*
	//*   bit 20:   Contains Closed Surface(s)
	//*/
	//{
	//	wchar_t* szResult = CheckInstanceConsistency(pRDFInstance->getInstance(), flagbit20) > 0 ? L"YES" : L"-";

	//	CMFCPropertyGridProperty* pProperty = new CMFCPropertyGridProperty(L"Contains Closed Surface(s)", (_variant_t)szResult, L"Contains Closed Surface(s)");
	//	pProperty->AllowEdit(FALSE);

	//	pConsistencyGroup->AddSubItem(pProperty);
	//}

	///*
	//*   bit 21:   Contains Open Surface(s)
	//*/
	//{
	//	wchar_t* szResult = CheckInstanceConsistency(pRDFInstance->getInstance(), flagbit21) > 0 ? L"YES" : L"-";

	//	CMFCPropertyGridProperty* pProperty = new CMFCPropertyGridProperty(L"Contains Open Surface(s)", (_variant_t)szResult, L"Contains Open Surface(s)");
	//	pProperty->AllowEdit(FALSE);

	//	pConsistencyGroup->AddSubItem(pProperty);
	//}

	///*
	//*   bit 22:   Contains Closed Infinite Surface(s)
	//*/
	//{
	//	wchar_t* szResult = CheckInstanceConsistency(pRDFInstance->getInstance(), flagbit22) > 0 ? L"YES" : L"-";

	//	CMFCPropertyGridProperty* pProperty = new CMFCPropertyGridProperty(L"Contains Closed Infinite Surface(s)", (_variant_t)szResult, L"Contains Closed Infinite Surface(s)");
	//	pProperty->AllowEdit(FALSE);

	//	pConsistencyGroup->AddSubItem(pProperty);
	//}

	///*
	//*   bit 23:   Contains Open Infinite Surface(s)
	//*/
	//{
	//	wchar_t* szResult = CheckInstanceConsistency(pRDFInstance->getInstance(), flagbit23) > 0 ? L"YES" : L"-";

	//	CMFCPropertyGridProperty* pProperty = new CMFCPropertyGridProperty(L"Contains Open Infinite Surface(s)", (_variant_t)szResult, L"Contains Open Infinite Surface(s)");
	//	pProperty->AllowEdit(FALSE);

	//	pConsistencyGroup->AddSubItem(pProperty);
	//}

	///*
	//*   bit 24:   Contains Closed Line(s)
	//*/
	//{
	//	wchar_t* szResult = CheckInstanceConsistency(pRDFInstance->getInstance(), flagbit24) > 0 ? L"YES" : L"-";

	//	CMFCPropertyGridProperty* pProperty = new CMFCPropertyGridProperty(L"Contains Closed Line(s)", (_variant_t)szResult, L"Contains Closed Line(s)");
	//	pProperty->AllowEdit(FALSE);

	//	pConsistencyGroup->AddSubItem(pProperty);
	//}

	///*
	//*   bit 25:   Contains Open Line(s)
	//*/
	//{
	//	wchar_t* szResult = CheckInstanceConsistency(pRDFInstance->getInstance(), flagbit25) > 0 ? L"YES" : L"-";

	//	CMFCPropertyGridProperty* pProperty = new CMFCPropertyGridProperty(L"Contains Open Line(s)", (_variant_t)szResult, L"Contains Open Line(s)");
	//	pProperty->AllowEdit(FALSE);

	//	pConsistencyGroup->AddSubItem(pProperty);
	//}

	///*
	//*   bit 26:   Contains Closed Infinite Line(s) [i.e. both ends in infinity]
	//*/
	//{
	//	wchar_t* szResult = CheckInstanceConsistency(pRDFInstance->getInstance(), flagbit26) > 0 ? L"YES" : L"-";

	//	CMFCPropertyGridProperty* pProperty = new CMFCPropertyGridProperty(L"Contains Closed Infinite Line(s)", (_variant_t)szResult, L"Contains Closed Infinite Line(s) [i.e. both ends in infinity]");
	//	pProperty->AllowEdit(FALSE);

	//	pConsistencyGroup->AddSubItem(pProperty);
	//}

	///*
	//*   bit 27:   Contains Open Infinite Line(s) [i.e. one end in infinity]
	//*/
	//{
	//	wchar_t* szResult = CheckInstanceConsistency(pRDFInstance->getInstance(), flagbit27) > 0 ? L"YES" : L"-";

	//	CMFCPropertyGridProperty* pProperty = new CMFCPropertyGridProperty(L"Contains Open Infinite Line(s)", (_variant_t)szResult, L"Contains Open Infinite Line(s) [i.e. one end in infinity]");
	//	pProperty->AllowEdit(FALSE);

	//	pConsistencyGroup->AddSubItem(pProperty);
	//}

	///*
	//*   bit 28:   Contains (Closed) Point(s)
	//*/
	//{
	//	wchar_t * szResult = CheckInstanceConsistency(pRDFInstance->getInstance(), flagbit28) > 0 ? L"YES" : L"-";

	//	CMFCPropertyGridProperty * pProperty = new CMFCPropertyGridProperty(L"Contains (Closed) Point(s)", (_variant_t)szResult, L"Contains (Closed) Point(s)");
	//	pProperty->AllowEdit(FALSE);

	//	pConsistencyGroup->AddSubItem(pProperty);
	//}

	//m_wndPropList.AddProperty(pMetaInfoGroup);
}

void CPropertiesWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndPropList.SetFocus();
}

void CPropertiesWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CDockablePane::OnSettingChange(uFlags, lpszSection);
	SetPropListFont();
}

void CPropertiesWnd::OnViewModeChanged()
{
	switch (m_wndObjectCombo.GetCurSel())
	{
		case 0: // Application Properties
		{
			LoadApplicationProperties();
		}
		break;

		case 1: // Instance Properties
		{
			LoadInstanceProperties();
		}
		break;

		default:
		{
			ASSERT(false); // unknown mode
		}
		break;
	}
}

LRESULT CPropertiesWnd::OnLoadInstancePropertyValues(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	ASSERT(0); // todo

	//ASSERT(wParam != 0);	

	//CMFCPropertyGridProperty * pValueProperty = (CMFCPropertyGridProperty *)wParam;	
	//CRDFInstancePropertyData * pValueData = (CRDFInstancePropertyData *)pValueProperty->GetData();
	//ASSERT(pValueData != nullptr);

	//CRDFInstance * pRDFInstance = pValueData->GetInstance();
	//CRDFProperty * pRDFProperty = pValueData->GetProperty();

	///*
	//* Select the property
	//*/
	//CMFCPropertyGridProperty * pPropertyGroup = pValueProperty->GetParent();	
	//m_wndPropList.SetCurSel(pPropertyGroup);

	///*
	//* Update the cardinality
	//*/
	//CAddRDFInstanceProperty * pCardinalityProperty = (CAddRDFInstanceProperty *)pPropertyGroup->GetSubItem(1);

	//wstring strCardinality = pRDFProperty->getCardinality(pRDFInstance->getInstance());

	//pCardinalityProperty->SetValue((_variant_t)strCardinality.c_str());
	//pCardinalityProperty->SetModified((_variant_t)strCardinality.c_str() != pCardinalityProperty->GetOriginalValue());

	///*
	//* Disable the drawing
	//*/
	//m_wndPropList.SendMessage(WM_SETREDRAW, 0, 0);

	///*
	//* Remove all values	
	//*/
	//while (pPropertyGroup->GetSubItemsCount() > 2/*keep range and cardinality*/)
	//{
	//	CMFCPropertyGridProperty * pValue = (CRDFInstanceProperty *)(pPropertyGroup->GetSubItem(2));

	//	m_wndPropList.DeleteProperty(pValue);
	//}	

	///*
	//* Add the values
	//*/
	//AddInstancePropertyValues(pPropertyGroup, pRDFInstance, pRDFProperty);

	///*
	//* Enable the drawing
	//*/
	//m_wndPropList.SendMessage(WM_SETREDRAW, 1, 0);

	//m_wndPropList.AdjustLayout();

	return 0;
}

LRESULT CPropertiesWnd::OnLoadInstanceProperties(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	LoadInstanceProperties();

	return 0;
}

void CPropertiesWnd::SetPropListFont()
{
	::DeleteObject(m_fntPropList.Detach());

	LOGFONT lf;
	afxGlobalData.fontRegular.GetLogFont(&lf);

	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);

	afxGlobalData.GetNonClientMetrics(info);

	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;

	m_fntPropList.CreateFontIndirect(&lf);

	m_wndPropList.SetFont(&m_fntPropList);
	m_wndObjectCombo.SetFont(&m_fntPropList);
}


void CPropertiesWnd::OnDestroy()
{
	ASSERT(GetController() != nullptr);
	GetController()->UnRegisterView(this);

	__super::OnDestroy();
}
