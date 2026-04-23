
#include "stdafx.h"

#include "_ifc_model.h"
#include "_ifc_instance.h"
#include "_ap242_model.h"

#include "_ap242_product_definition.h"
#include "_ap242_instance.h"
#include "_ptr.h"

#include "PropertiesWnd.h"
#include "Resource.h"
#include "MainFrm.h"
#include "STEPViewer.h"
#include "AP242OpenGLView.h"

#include "CIS2Model.h"
#include "CIS2Instance.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

// ************************************************************************************************
#define TRUE_VALUE_PROPERTY L"Yes"
#define FALSE_VALUE_PROPERTY L"No"
#define ROTATION_MODE_XY L"2D"
#define ROTATION_MODE_XYZ L"3D"

// ************************************************************************************************
static bool operator==(const CValueLocator& lhs, const CValueLocator& rhs)
{
	return lhs.sdaiInst == rhs.sdaiInst
		&& lhs.sdaiAttr == rhs.sdaiAttr
		&& lhs.indexes == rhs.indexes;
}

static SdaiPrimitiveType GetEditArgumentType(SdaiInstance inst, SdaiAttr attr)
{
	//if argument is set, use its type
	auto sdaiType = engiGetInstanceAttrType(inst, attr);

	//for unset argument, we will create new from attribute definition
	if (!sdaiType) {
		sdaiType = engiGetAttrType(attr);
		sdaiType &= ~engiTypeFlagAggrOption;
		if (sdaiType & engiTypeFlagAggr) {
			sdaiType = sdaiAGGR;
		}
	}

	ASSERT(sdaiType);
	return sdaiType;
}

static CStringA GetPrimitiveSdaiTypeName(SdaiPrimitiveType type)
{
	switch (type) {
		case	sdaiBINARY:		return "sdaiBINARY";
		case	sdaiBOOLEAN:	return "sdaiBOOLEAN";
		case	sdaiENUM:		return "sdaiENUM";
		case	sdaiINSTANCE:	return "sdaiINSTANCE";
		case	sdaiINTEGER:	return "sdaiINTEGER";
		case	sdaiLOGICAL:	return "sdaiLOGICAL";
		case	sdaiREAL:		return "sdaiREAL";
		case	sdaiSTRING:		return "sdaiSTRING";
		default: ASSERT(false);	return "";
	}
}

// ************************************************************************************************
CApplicationPropertyData::CApplicationPropertyData(enumApplicationProperty enApplicationProperty)
{
	m_enApplicationProperty = enApplicationProperty;
}

enumApplicationProperty CApplicationPropertyData::GetType() const
{
	return m_enApplicationProperty;
}

CApplicationProperty::CApplicationProperty(const CString& strName, const COleVariant& vtValue, LPCTSTR szDescription, DWORD_PTR dwData)
	: CMFCPropertyGridProperty(strName, vtValue, szDescription, dwData)
{}

CApplicationProperty::CApplicationProperty(const CString& strGroupName, DWORD_PTR dwData, BOOL bIsValueList)
	: CMFCPropertyGridProperty(strGroupName, dwData, bIsValueList)
{}

/*virtual*/ CApplicationProperty::~CApplicationProperty()
{
	delete (CApplicationPropertyData*)GetData();
}

// ************************************************************************************************
CInstanceAttributeProperty::CInstanceAttributeProperty(const CString& strName, const COleVariant& vtValue, LPCTSTR szDescription, DWORD_PTR dwData)
	: CMFCPropertyGridProperty(strName, vtValue, szDescription, dwData)
{}

/*virtual*/ CInstanceAttributeProperty::~CInstanceAttributeProperty()
{
	auto data = (CPropertyGridData*)GetData();
	if (data)
		delete data;
}

/*virtual*/ CString CInstanceAttributeProperty::FormatProperty()
{
	if (m_varValue.vt != VT_I8) {
		return __super::FormatProperty();
	}

	CString strValue;
	strValue.Format(L"%lld", m_varValue.llVal);

	return strValue;
}

/*virtual*/ BOOL CInstanceAttributeProperty::TextToVar(const CString& strText)
{
	/*
	* Support for int64_t
	*/
	if (m_varValue.vt != VT_I8) {
		return __super::TextToVar(strText);
	}

	m_varValue.llVal = _ttoi64(strText);

	return TRUE;
}

/*virtual*/ CWnd* CInstanceAttributeProperty::CreateInPlaceEdit(CRect rectEdit, BOOL& bDefaultFormat)
{
	/*
	* Support for int64_t
	*/
	bool bInt64 = false;
	if (m_varValue.vt == VT_I8) {
		// Cheat the base class
		m_varValue.vt = VT_I4;
		bInt64 = true;
	}

	CWnd* pWnd = __super::CreateInPlaceEdit(rectEdit, bDefaultFormat);

	if (bInt64) {
		// Restore type
		m_varValue.vt = VT_I8;
	}

	return pWnd;
}

/*virtual*/ void CInstanceAttributeProperty::EnableSpinControlInt64()
{
	ASSERT(m_varValue.vt == VT_I8);

	// Cheat the base class
	m_varValue.vt = VT_I4;

	EnableSpinControl(TRUE, 0, INT_MAX);

	m_varValue.vt = VT_I8;
}

// ************************************************************************************************
CColorSelectorProperty::CColorSelectorProperty(const CString& strName, const COLORREF& color, CPalette* pPalette, LPCTSTR szDescription, DWORD_PTR dwData)
	: CMFCPropertyGridColorProperty(strName, color, pPalette, szDescription, dwData)
{}

/*virtual*/ CColorSelectorProperty::~CColorSelectorProperty()
{
	delete (CApplicationPropertyData*)GetData();
}

COLORREF CColorSelectorProperty::GetSelectedColor() const
{
	if (GetColor() == (COLORREF)-1) {
		return GetAutomaticColor();
	}

	return GetColor();
}

COLORREF CColorSelectorProperty::GetAutomaticColor() const
{
	return m_ColorAutomatic;
}

// ************************************************************************************************
/*virtual*/ void CPropertiesWnd::postModelLoaded()
{
	m_wndObjectCombo.SetCurSel(0 /*Application*/);

	LoadApplicationProperties();
}

/*virtual*/ void CPropertiesWnd::onInstanceSelected(_view* /*pSender*/)
{
	if (!getController()->getSelectedInstances().empty()) {
		if (m_wndObjectCombo.GetCurSel() == 0) {
			m_wndObjectCombo.SetCurSel(1 /*Properties*/);
		}
	} 
	else {
		m_wndObjectCombo.SetCurSel(0 /*Application*/);
	}

	SetInstancesToExplore();

	OnViewModeChanged();
}

/*virtual*/ void CPropertiesWnd::onApplicationPropertyChanged(_view* pSender, enumApplicationProperty /*enApplicationProperty*/)
{
	if (pSender == this) {
		return;
	}

	if (m_wndObjectCombo.GetCurSel() == 0) {
		LoadApplicationProperties();
	}
}

_ap_model* CPropertiesWnd::GetModelByInstance(SdaiModel sdaiModel)
{
	for (auto pModel : getController()->getModels()) {
		if (!pModel->getEnable()) {
			continue;
		}

		_ptr<_ap_model> apModel(pModel);
		if (apModel->getSdaiModel() == sdaiModel) {
			return apModel;
		}
	}

	return nullptr;
}

/*afx_msg*/ LRESULT CPropertiesWnd::OnPropertyChanged(__in WPARAM /*wparam*/, __in LPARAM lparam)
{
	auto pController = getController();
	if (pController == nullptr) {
		ASSERT(FALSE);

		return 0;
	}

#pragma region Application
	if (m_wndObjectCombo.GetCurSel() == 0) {
		auto pOGLRenderer = getController()->getViewAs<_oglRenderer>();
		if (pOGLRenderer == nullptr) {
			ASSERT(FALSE);
			return 0;
		}

		auto pBlinnPhongProgram = pOGLRenderer->_getOGLProgramAs<_oglBlinnPhongProgram>();
		auto pApplicationProperty = dynamic_cast<CApplicationProperty*>((CMFCPropertyGridProperty*)lparam);
		if (pApplicationProperty != nullptr) {
			CString strValue = pApplicationProperty->GetValue();

			auto pData = (CApplicationPropertyData*)pApplicationProperty->GetData();
			if (pData == nullptr) {
				ASSERT(FALSE);

				return 0;
			}

			switch (pData->GetType()) {
				case enumApplicationProperty::ShowFaces:
					{
						pOGLRenderer->setShowFaces(strValue == TRUE_VALUE_PROPERTY ? TRUE : FALSE);

						getController()->onApplicationPropertyChanged(this, enumApplicationProperty::ShowFaces);
					}
					break;

				case enumApplicationProperty::GhostView:
					{
						pOGLRenderer->setGhostView(strValue == TRUE_VALUE_PROPERTY ? TRUE : FALSE);

						getController()->onApplicationPropertyChanged(this, enumApplicationProperty::GhostView);
					}
					break;

				case enumApplicationProperty::GhostViewTransparency:
					{
						float fTransparency = (float)_wtof((LPCTSTR)strValue);

						// Validate
						if (fTransparency > 1.f) {
							fTransparency = 1.f;

							pApplicationProperty->SetValue(fTransparency);
						}
						else if (fTransparency < 0.f) {
							fTransparency = 0.f;

							pApplicationProperty->SetValue(fTransparency);
						}

						pOGLRenderer->setGhostViewTransparency((float)atof(CW2A((LPCWSTR)strValue)));

						getController()->onApplicationPropertyChanged(this, enumApplicationProperty::GhostViewTransparency);
					}
					break;

				case enumApplicationProperty::CullFaces:
					{
						pOGLRenderer->setCullFacesMode(strValue);

						getController()->onApplicationPropertyChanged(this, enumApplicationProperty::CullFaces);
					}
					break;

				case enumApplicationProperty::ShowConceptualFacesWireframes:
					{
						pOGLRenderer->setShowConceptualFacesPolygons(strValue == TRUE_VALUE_PROPERTY ? TRUE : FALSE);

						getController()->onApplicationPropertyChanged(this, enumApplicationProperty::ShowConceptualFacesWireframes);
					}
					break;

				case enumApplicationProperty::ShowLines:
					{
						pOGLRenderer->setShowLines(strValue == TRUE_VALUE_PROPERTY ? TRUE : FALSE);

						getController()->onApplicationPropertyChanged(this, enumApplicationProperty::ShowLines);
					}
					break;

				case enumApplicationProperty::ShowPoints:
					{
						pOGLRenderer->setShowPoints(strValue == TRUE_VALUE_PROPERTY ? TRUE : FALSE);

						getController()->onApplicationPropertyChanged(this, enumApplicationProperty::ShowPoints);
					}
					break;

				case enumApplicationProperty::ShowBoundingBoxes:
					{
						pOGLRenderer->setShowBoundingBoxes(strValue == TRUE_VALUE_PROPERTY ? TRUE : FALSE);

						getController()->onApplicationPropertyChanged(this, enumApplicationProperty::ShowBoundingBoxes);
					}
					break;

				case enumApplicationProperty::RotationMode:
					{
						pOGLRenderer->_setRotationMode(strValue == ROTATION_MODE_XY ? enumRotationMode::XY : enumRotationMode::XYZ);

						getController()->onApplicationPropertyChanged(this, enumApplicationProperty::RotationMode);
					}
					break;

				case enumApplicationProperty::ShowCoordinateSystem:
					{
						pOGLRenderer->setShowCoordinateSystem(strValue == TRUE_VALUE_PROPERTY ? TRUE : FALSE);

						getController()->onApplicationPropertyChanged(this, enumApplicationProperty::ShowCoordinateSystem);
					}
					break;

				case enumApplicationProperty::CoordinateSystemType:
					{
						pOGLRenderer->setModelCoordinateSystem(strValue == TRUE_VALUE_PROPERTY ? TRUE : FALSE);

						getController()->onApplicationPropertyChanged(this, enumApplicationProperty::CoordinateSystemType);
					}
					break;

				case enumApplicationProperty::ShowNavigator:
					{
						pOGLRenderer->setShowNavigator(strValue == TRUE_VALUE_PROPERTY ? TRUE : FALSE);

						getController()->onApplicationPropertyChanged(this, enumApplicationProperty::ShowNavigator);
					}
					break;

				case enumApplicationProperty::PointLightingLocation:
					{
						ASSERT(pBlinnPhongProgram != nullptr);

						auto pProperty = pApplicationProperty->GetParent();
						ASSERT(pProperty != nullptr);
						ASSERT(dynamic_cast<CApplicationProperty*>(pProperty) != nullptr);
						ASSERT(((CApplicationPropertyData*)dynamic_cast<CApplicationProperty*>(pProperty)->
							GetData())->GetType() == enumApplicationProperty::PointLightingLocation);
						ASSERT(pProperty->GetSubItemsCount() == 3);

						auto pX = pProperty->GetSubItem(0);
						auto pY = pProperty->GetSubItem(1);
						auto pZ = pProperty->GetSubItem(2);

						pBlinnPhongProgram->_setPointLightingLocation(glm::vec3(
							(float)_wtof((LPCTSTR)(CString)pX->GetValue()),
							(float)_wtof((LPCTSTR)(CString)pY->GetValue()),
							(float)_wtof((LPCTSTR)(CString)pZ->GetValue()))
						);

						getController()->onApplicationPropertyChanged(this, enumApplicationProperty::PointLightingLocation);
					}
					break;

				case enumApplicationProperty::AmbientLightWeighting:
					{
						ASSERT(pBlinnPhongProgram != nullptr);

						auto pProperty = pApplicationProperty->GetParent();
						ASSERT(pProperty != nullptr);
						ASSERT(dynamic_cast<CApplicationProperty*>(pProperty) != nullptr);
						ASSERT(((CApplicationPropertyData*)dynamic_cast<CApplicationProperty*>(pProperty)->
							GetData())->GetType() == enumApplicationProperty::AmbientLightWeighting);
						ASSERT(pProperty->GetSubItemsCount() == 3);

						auto pX = pProperty->GetSubItem(0);
						auto pY = pProperty->GetSubItem(1);
						auto pZ = pProperty->GetSubItem(2);

						pBlinnPhongProgram->_setAmbientLightWeighting(
							(float)_wtof((LPCTSTR)(CString)pX->GetValue()),
							(float)_wtof((LPCTSTR)(CString)pY->GetValue()),
							(float)_wtof((LPCTSTR)(CString)pZ->GetValue())
						);

						getController()->onApplicationPropertyChanged(this, enumApplicationProperty::AmbientLightWeighting);
					}
					break;

				case enumApplicationProperty::DiffuseLightWeighting:
					{
						ASSERT(pBlinnPhongProgram != nullptr);

						auto pProperty = pApplicationProperty->GetParent();
						ASSERT(pProperty != nullptr);
						ASSERT(dynamic_cast<CApplicationProperty*>(pProperty) != nullptr);
						ASSERT(((CApplicationPropertyData*)dynamic_cast<CApplicationProperty*>(pProperty)->
							GetData())->GetType() == enumApplicationProperty::DiffuseLightWeighting);
						ASSERT(pProperty->GetSubItemsCount() == 3);

						auto pX = pProperty->GetSubItem(0);
						auto pY = pProperty->GetSubItem(1);
						auto pZ = pProperty->GetSubItem(2);

						pBlinnPhongProgram->_setDiffuseLightWeighting(
							(float)_wtof((LPCTSTR)(CString)pX->GetValue()),
							(float)_wtof((LPCTSTR)(CString)pY->GetValue()),
							(float)_wtof((LPCTSTR)(CString)pZ->GetValue())
						);

						getController()->onApplicationPropertyChanged(this, enumApplicationProperty::DiffuseLightWeighting);
					}
					break;

				case enumApplicationProperty::SpecularLightWeighting:
					{
						ASSERT(pBlinnPhongProgram != nullptr);

						auto pProperty = pApplicationProperty->GetParent();
						ASSERT(pProperty != nullptr);
						ASSERT(dynamic_cast<CApplicationProperty*>(pProperty) != nullptr);
						ASSERT(((CApplicationPropertyData*)dynamic_cast<CApplicationProperty*>(pProperty)->
							GetData())->GetType() == enumApplicationProperty::SpecularLightWeighting);
						ASSERT(pProperty->GetSubItemsCount() == 3);

						auto pX = pProperty->GetSubItem(0);
						auto pY = pProperty->GetSubItem(1);
						auto pZ = pProperty->GetSubItem(2);

						pBlinnPhongProgram->_setSpecularLightWeighting(
							(float)_wtof((LPCTSTR)(CString)pX->GetValue()),
							(float)_wtof((LPCTSTR)(CString)pY->GetValue()),
							(float)_wtof((LPCTSTR)(CString)pZ->GetValue())
						);

						getController()->onApplicationPropertyChanged(this, enumApplicationProperty::SpecularLightWeighting);
					}
					break;

				case enumApplicationProperty::MaterialShininess:
					{
						ASSERT(pBlinnPhongProgram != nullptr);

						float fValue = (float)_wtof((LPCTSTR)strValue);
						pBlinnPhongProgram->_setMaterialShininess(fValue);

						getController()->onApplicationPropertyChanged(this, enumApplicationProperty::MaterialShininess);
					}
					break;

				case enumApplicationProperty::Contrast:
					{
						ASSERT(pBlinnPhongProgram != nullptr);

						float fValue = (float)_wtof((LPCTSTR)strValue);
						pBlinnPhongProgram->_setContrast(fValue);

						getController()->onApplicationPropertyChanged(this, enumApplicationProperty::Contrast);
					}
					break;

				case enumApplicationProperty::Brightness:
					{
						ASSERT(pBlinnPhongProgram != nullptr);

						float fValue = (float)_wtof((LPCTSTR)strValue);
						pBlinnPhongProgram->_setBrightness(fValue);

						getController()->onApplicationPropertyChanged(this, enumApplicationProperty::Brightness);
					}
					break;

				case enumApplicationProperty::Gamma:
					{
						ASSERT(pBlinnPhongProgram != nullptr);

						float fValue = (float)_wtof((LPCTSTR)strValue);
						pBlinnPhongProgram->_setGamma(fValue);

						getController()->onApplicationPropertyChanged(this, enumApplicationProperty::Gamma);
					}
					break;

				case enumApplicationProperty::SelectionMaterial:
					{
						OnSelectionMaterialPropertyChanged(pApplicationProperty);
					}
					break;

				case enumApplicationProperty::HighlightMaterial:
					{
						OnHighlightMaterialPropertyChanged(pApplicationProperty);
					}
					break;

				default:
					ASSERT(FALSE);
					break;
			} // switch (pData->GetType())

			return 0;
		} // if (pApplicationProperty != nullptr)

		auto pColorSelectorProperty = dynamic_cast<CColorSelectorProperty*>((CMFCPropertyGridProperty*)lparam);
		if (pColorSelectorProperty != nullptr) {
			auto pData = (CApplicationPropertyData*)pColorSelectorProperty->GetData();
			if (pData == nullptr) {
				ASSERT(FALSE);
				return 0;
			}

			switch (pData->GetType()) {
				case enumApplicationProperty::BackgroundColor:
					{
						pOGLRenderer->setBackgroundColor(
							(float)GetRValue(pColorSelectorProperty->GetSelectedColor()) / 255.f,
							(float)GetGValue(pColorSelectorProperty->GetSelectedColor()) / 255.f,
							(float)GetBValue(pColorSelectorProperty->GetSelectedColor()) / 255.f);
						getController()->onApplicationPropertyChanged(this, enumApplicationProperty::BackgroundColor);
					}
					break;

				case enumApplicationProperty::SelectionMaterial:
					{
						OnSelectionMaterialPropertyChanged(pColorSelectorProperty);
					}
					break;

				case enumApplicationProperty::HighlightMaterial:
					{
						OnHighlightMaterialPropertyChanged(pColorSelectorProperty);
					}
					break;

				default:
					ASSERT(FALSE);
					break;
			} // switch (pData->GetType())

			return 0;
		} // if (pColorSelectorProperty != nullptr)

		ASSERT(FALSE); // unexpected!
	} // if (m_wndObjectCombo.GetCurSel() == 0)
#pragma endregion

	return 0;
}

void CPropertiesWnd::OnSelectionMaterialPropertyChanged(CMFCPropertyGridProperty* pProp)
{
	if (pProp == nullptr) {
		ASSERT(FALSE);
		return;
	}

	auto pMaterialProperty = pProp->GetParent();
	ASSERT(pMaterialProperty->GetSubItemsCount() == 3);
	//ASSERT(pMaterialProperty->GetSubItemsCount() == 5);

	// Validate transparency value
	auto strValue = pMaterialProperty->GetSubItem(2)->GetValue();
	//auto strValue = pMaterialProperty->GetSubItem(4)->GetValue();
	float fTransparency = (float)_wtof(((LPCTSTR)(CString)strValue));
	if (fTransparency > 1.f) {
		fTransparency = 1.f;
		pMaterialProperty->GetSubItem(2)->SetValue(fTransparency);
		//pMaterialProperty->GetSubItem(4)->SetValue(fTransparency);		
	}
	else if (fTransparency < 0.f) {
		fTransparency = 0.f;
		pMaterialProperty->GetSubItem(2)->SetValue(fTransparency);
		//pMaterialProperty->GetSubItem(4)->SetValue(fTransparency);
	}

	_material material;
	material.init(
		(float)GetRValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(0)))->GetSelectedColor()) / 255.f,
		(float)GetGValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(0)))->GetSelectedColor()) / 255.f,
		(float)GetBValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(0)))->GetSelectedColor()) / 255.f,
		(float)GetRValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(1)))->GetSelectedColor()) / 255.f,
		(float)GetGValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(1)))->GetSelectedColor()) / 255.f,
		(float)GetBValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(1)))->GetSelectedColor()) / 255.f,
		(float)GetRValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(1)))->GetSelectedColor()) / 255.f,
		(float)GetGValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(1)))->GetSelectedColor()) / 255.f,
		(float)GetBValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(1)))->GetSelectedColor()) / 255.f,
		(float)GetRValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(1)))->GetSelectedColor()) / 255.f,
		(float)GetGValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(1)))->GetSelectedColor()) / 255.f,
		(float)GetBValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(1)))->GetSelectedColor()) / 255.f,
		fTransparency,
		nullptr,
		false);
	/*material.init(
		(float)GetRValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(0)))->GetSelectedColor()) / 255.f,
		(float)GetGValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(0)))->GetSelectedColor()) / 255.f,
		(float)GetBValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(0)))->GetSelectedColor()) / 255.f,
		(float)GetRValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(1)))->GetSelectedColor()) / 255.f,
		(float)GetGValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(1)))->GetSelectedColor()) / 255.f,
		(float)GetBValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(1)))->GetSelectedColor()) / 255.f,
		(float)GetRValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(2)))->GetSelectedColor()) / 255.f,
		(float)GetGValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(2)))->GetSelectedColor()) / 255.f,
		(float)GetBValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(2)))->GetSelectedColor()) / 255.f,
		(float)GetRValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(3)))->GetSelectedColor()) / 255.f,
		(float)GetGValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(3)))->GetSelectedColor()) / 255.f,
		(float)GetBValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(3)))->GetSelectedColor()) / 255.f,
		fTransparency,
		nullptr,
		false);*/

	auto pOGLRenderer = getController()->getViewAs<_oglRenderer>();
	if (pOGLRenderer != nullptr) {
		pOGLRenderer->setSelectedInstanceMaterial(material);
		getController()->onApplicationPropertyChanged(this, enumApplicationProperty::SelectionMaterial);
	}
}

void CPropertiesWnd::OnHighlightMaterialPropertyChanged(CMFCPropertyGridProperty* pProp)
{
	if (pProp == nullptr) {
		ASSERT(FALSE);
		return;
	}

	auto pMaterialProperty = pProp->GetParent();
	ASSERT(pMaterialProperty->GetSubItemsCount() == 3);
	//ASSERT(pMaterialProperty->GetSubItemsCount() == 5);

	// Validate transparency value
	auto strValue = pMaterialProperty->GetSubItem(2)->GetValue();
	//auto strValue = pMaterialProperty->GetSubItem(4)->GetValue();
	float fTransparency = (float)_wtof(((LPCTSTR)(CString)strValue));
	if (fTransparency > 1.f) {
		fTransparency = 1.f;
		pMaterialProperty->GetSubItem(2)->SetValue(fTransparency);
		//pMaterialProperty->GetSubItem(4)->SetValue(fTransparency);		
	}
	else if (fTransparency < 0.f) {
		fTransparency = 0.f;
		pMaterialProperty->GetSubItem(2)->SetValue(fTransparency);
		//pMaterialProperty->GetSubItem(4)->SetValue(fTransparency);
	}

	_material material;
	material.init(
		(float)GetRValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(0)))->GetSelectedColor()) / 255.f,
		(float)GetGValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(0)))->GetSelectedColor()) / 255.f,
		(float)GetBValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(0)))->GetSelectedColor()) / 255.f,
		(float)GetRValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(1)))->GetSelectedColor()) / 255.f,
		(float)GetGValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(1)))->GetSelectedColor()) / 255.f,
		(float)GetBValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(1)))->GetSelectedColor()) / 255.f,
		(float)GetRValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(1)))->GetSelectedColor()) / 255.f,
		(float)GetGValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(1)))->GetSelectedColor()) / 255.f,
		(float)GetBValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(1)))->GetSelectedColor()) / 255.f,
		(float)GetRValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(1)))->GetSelectedColor()) / 255.f,
		(float)GetGValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(1)))->GetSelectedColor()) / 255.f,
		(float)GetBValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(1)))->GetSelectedColor()) / 255.f,
		fTransparency,
		nullptr,
		false);
	/*material.init(
		(float)GetRValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(0)))->GetSelectedColor()) / 255.f,
		(float)GetGValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(0)))->GetSelectedColor()) / 255.f,
		(float)GetBValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(0)))->GetSelectedColor()) / 255.f,
		(float)GetRValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(1)))->GetSelectedColor()) / 255.f,
		(float)GetGValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(1)))->GetSelectedColor()) / 255.f,
		(float)GetBValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(1)))->GetSelectedColor()) / 255.f,
		(float)GetRValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(2)))->GetSelectedColor()) / 255.f,
		(float)GetGValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(2)))->GetSelectedColor()) / 255.f,
		(float)GetBValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(2)))->GetSelectedColor()) / 255.f,
		(float)GetRValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(3)))->GetSelectedColor()) / 255.f,
		(float)GetGValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(3)))->GetSelectedColor()) / 255.f,
		(float)GetBValue(((CColorSelectorProperty*)(pMaterialProperty->GetSubItem(3)))->GetSelectedColor()) / 255.f,
		fTransparency,
		nullptr,
		false);*/

	auto pOGLRenderer = getController()->getViewAs<_oglRenderer>();
	if (pOGLRenderer != nullptr) {
		pOGLRenderer->setPointedInstanceMaterial(material);
		getController()->onApplicationPropertyChanged(this, enumApplicationProperty::HighlightMaterial);
	}
}

CPropertiesWnd::CPropertiesWnd()
{
	m_nComboHeight = 0;
	m_calculateDerivedAttributes = FALSE;
}

CPropertiesWnd::~CPropertiesWnd()
{}

BEGIN_MESSAGE_MAP(CPropertiesWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_EXPAND_ALL, OnExpandAllProperties)
	ON_UPDATE_COMMAND_UI(ID_EXPAND_ALL, OnUpdateExpandAllProperties)
	ON_COMMAND(ID_SORTPROPERTIES, OnSortProperties)
	ON_UPDATE_COMMAND_UI(ID_SORTPROPERTIES, OnUpdateSortProperties)
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
	ON_WM_DESTROY()
	ON_REGISTERED_MESSAGE(AFX_WM_PROPERTY_CHANGED, OnPropertyChanged)
	ON_CBN_SELENDOK(ID_COMBO_PROPERTIES_VIEW, OnViewModeChanged)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar message handlers

void CPropertiesWnd::AdjustLayout()
{
	if (GetSafeHwnd() == nullptr || (AfxGetMainWnd() != nullptr && AfxGetMainWnd()->IsIconic())) {
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_toolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndObjectCombo.SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), m_nComboHeight, SWP_NOACTIVATE | SWP_NOZORDER);
	m_toolBar.SetWindowPos(nullptr, rectClient.left, rectClient.top + m_nComboHeight, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndPropList.SetWindowPos(nullptr, rectClient.left, rectClient.top + m_nComboHeight + cyTlb, rectClient.Width(), rectClient.Height() - (m_nComboHeight + cyTlb), SWP_NOACTIVATE | SWP_NOZORDER);
}

int CPropertiesWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	ASSERT(getController() != nullptr);
	getController()->registerView(this);

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create combo:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_BORDER | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if (!m_wndObjectCombo.Create(dwViewStyle, rectDummy, this, ID_COMBO_PROPERTIES_VIEW)) {
		ASSERT(FALSE);

		return -1;
	}

	m_wndObjectCombo.AddString(_T("Application"));
	m_wndObjectCombo.AddString(_T("Properties"));
	m_wndObjectCombo.AddString(_T("Attributes"));
	m_wndObjectCombo.SetCurSel(0);

	CRect rectCombo;
	m_wndObjectCombo.GetClientRect(&rectCombo);

	m_nComboHeight = rectCombo.Height();

	if (!m_wndPropList.Create(WS_VISIBLE | WS_CHILD, rectDummy, this, 2)) {
		ASSERT(FALSE);

		return -1;
	}

	OnViewModeChanged();

	m_toolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_PROPERTIES);
	m_toolBar.LoadToolBar(IDR_PROPERTIES, 0, 0, TRUE /* Is locked */);
	m_toolBar.CleanUpLockedImages();
	m_toolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_PROPERTIES_HC : IDR_PROPERTIES, 0, 0, TRUE /* Locked */);

	m_toolBar.SetPaneStyle(m_toolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_toolBar.SetPaneStyle(m_toolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_toolBar.SetOwner(this);

	// All commands will be routed via this control , not via the parent frame:
	m_toolBar.SetRouteCommandsViaFrame(FALSE);

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
{}

void CPropertiesWnd::OnSortProperties()
{
	m_wndPropList.SetAlphabeticMode(!m_wndPropList.IsAlphabeticMode());
}

void CPropertiesWnd::OnUpdateSortProperties(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndPropList.IsAlphabeticMode());
}

void CPropertiesWnd::LoadApplicationProperties()
{
	m_wndPropList.RemoveAll();
	m_wndPropList.AdjustLayout();

	SetPropListFont();

	m_wndPropList.EnableHeaderCtrl(FALSE);
	m_wndPropList.EnableDescriptionArea();
	m_wndPropList.SetVSDotNetLook();
	m_wndPropList.MarkModifiedProperties();

	auto pController = getController();
	if (pController == nullptr) {
		ASSERT(FALSE);

		return;
	}

	auto pOGLRenderer = getController()->getViewAs<_oglRenderer>();
	if (pOGLRenderer == nullptr) {
		return;
	}

#pragma region View
	auto pViewGroup = new CMFCPropertyGridProperty(_T("View"));

	{
		auto pProperty = new CApplicationProperty(_T("Ghost View"),
			pOGLRenderer->getGhostView() ? TRUE_VALUE_PROPERTY : FALSE_VALUE_PROPERTY, _T("Ghost View"),
			(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::GhostView));
		pProperty->AddOption(TRUE_VALUE_PROPERTY);
		pProperty->AddOption(FALSE_VALUE_PROPERTY);
		pProperty->AllowEdit(FALSE);

		pViewGroup->AddSubItem(pProperty);
	}

	{
		auto pProperty = new CApplicationProperty(_T("Ghost View Transparency"),
			(_variant_t)pOGLRenderer->getGhostViewTransparency(),
			_T("Ghost View Transparency"),
			(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::GhostViewTransparency));
		pProperty->AllowEdit(TRUE);
		pViewGroup->AddSubItem(pProperty);
	}

	{
		auto pProperty = new CApplicationProperty(_T("Faces"),
			pOGLRenderer->getShowFaces() ? TRUE_VALUE_PROPERTY : FALSE_VALUE_PROPERTY, _T("Faces"),
			(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::ShowFaces));
		pProperty->AddOption(TRUE_VALUE_PROPERTY);
		pProperty->AddOption(FALSE_VALUE_PROPERTY);
		pProperty->AllowEdit(FALSE);

		pViewGroup->AddSubItem(pProperty);
	}

	{
		CString strCullFacesMode = pOGLRenderer->getCullFacesMode();

		auto pProperty = new CApplicationProperty(
			_T("Cull Faces"),
			strCullFacesMode == CULL_FACES_FRONT ? CULL_FACES_FRONT : strCullFacesMode == CULL_FACES_BACK ? CULL_FACES_BACK : CULL_FACES_NONE,
			_T("Cull Faces"), (DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::CullFaces));
		pProperty->AddOption(CULL_FACES_NONE);
		pProperty->AddOption(CULL_FACES_FRONT);
		pProperty->AddOption(CULL_FACES_BACK);
		pProperty->AllowEdit(FALSE);

		pViewGroup->AddSubItem(pProperty);
	}

	{
		auto pProperty = new CApplicationProperty(_T("Conceptual faces wireframes"),
			pOGLRenderer->getShowConceptualFacesPolygons() ? TRUE_VALUE_PROPERTY : FALSE_VALUE_PROPERTY,
			_T("Conceptual faces wireframes"),
			(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::ShowConceptualFacesWireframes));
		pProperty->AddOption(TRUE_VALUE_PROPERTY);
		pProperty->AddOption(FALSE_VALUE_PROPERTY);
		pProperty->AllowEdit(FALSE);

		pViewGroup->AddSubItem(pProperty);
	}

	{
		auto pProperty = new CApplicationProperty(_T("Lines"), pOGLRenderer->getShowLines() ? TRUE_VALUE_PROPERTY : FALSE_VALUE_PROPERTY,
			_T("Lines"),
			(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::ShowLines));
		pProperty->AddOption(TRUE_VALUE_PROPERTY);
		pProperty->AddOption(FALSE_VALUE_PROPERTY);
		pProperty->AllowEdit(FALSE);

		pViewGroup->AddSubItem(pProperty);
	}

	{
		auto pProperty = new CApplicationProperty(_T("Points"), pOGLRenderer->getShowPoints() ? TRUE_VALUE_PROPERTY : FALSE_VALUE_PROPERTY,
			_T("Points"),
			(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::ShowPoints));
		pProperty->AddOption(TRUE_VALUE_PROPERTY);
		pProperty->AddOption(FALSE_VALUE_PROPERTY);
		pProperty->AllowEdit(FALSE);

		pViewGroup->AddSubItem(pProperty);
	}

	{
		auto pProperty = new CApplicationProperty(_T("Bounding boxes"),
			pOGLRenderer->getShowBoundingBoxes() ? TRUE_VALUE_PROPERTY : FALSE_VALUE_PROPERTY,
			_T("Bounding boxes"),
			(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::ShowBoundingBoxes));
		pProperty->AddOption(FALSE_VALUE_PROPERTY);
		pProperty->AddOption(TRUE_VALUE_PROPERTY);
		pProperty->AllowEdit(FALSE);

		pViewGroup->AddSubItem(pProperty);
	}

	{
		auto pProperty = new CApplicationProperty(
			_T("Rotation mode"),
			pOGLRenderer->_getRotationMode() == enumRotationMode::XY ? ROTATION_MODE_XY : ROTATION_MODE_XYZ,
			_T("XY/XYZ"),
			(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::RotationMode));
		pProperty->AddOption(ROTATION_MODE_XY);
		pProperty->AddOption(ROTATION_MODE_XYZ);
		pProperty->AllowEdit(FALSE);

		pViewGroup->AddSubItem(pProperty);
	}

	{
		auto pProperty = new CApplicationProperty(_T("Show Coordinate System"),
			pOGLRenderer->getShowCoordinateSystem() ? TRUE_VALUE_PROPERTY : FALSE_VALUE_PROPERTY, _T("Show Coordinate System"),
			(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::ShowCoordinateSystem));
		pProperty->AddOption(TRUE_VALUE_PROPERTY);
		pProperty->AddOption(FALSE_VALUE_PROPERTY);
		pProperty->AllowEdit(FALSE);

		pViewGroup->AddSubItem(pProperty);
	}

	{
		auto pProperty = new CApplicationProperty(_T("Model Coordinate System"),
			pOGLRenderer->getModelCoordinateSystem() ? TRUE_VALUE_PROPERTY : FALSE_VALUE_PROPERTY, _T("Model Coordinate System"),
			(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::CoordinateSystemType));
		pProperty->AddOption(TRUE_VALUE_PROPERTY);
		pProperty->AddOption(FALSE_VALUE_PROPERTY);
		pProperty->AllowEdit(FALSE);

		pViewGroup->AddSubItem(pProperty);
	}

	{
		auto pProperty = new CApplicationProperty(_T("Navigator"),
			pOGLRenderer->getShowNavigator() ? TRUE_VALUE_PROPERTY : FALSE_VALUE_PROPERTY, _T("Navigator"),
			(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::ShowNavigator));
		pProperty->AddOption(TRUE_VALUE_PROPERTY);
		pProperty->AddOption(FALSE_VALUE_PROPERTY);
		pProperty->AllowEdit(FALSE);

		pViewGroup->AddSubItem(pProperty);
	}

	// Background Color
	{
		auto pColor = pOGLRenderer->getBackgroundColor();
		auto pProperty = new CColorSelectorProperty(L"Background Color",
			RGB((BYTE)(pColor->r() * 255.f),
				(BYTE)(pColor->g() * 255.f),
				(BYTE)(pColor->b() * 255.f)),
			nullptr,
			L"Background Color",
			(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::BackgroundColor));
		pProperty->EnableOtherButton(_T("Other..."));
		pProperty->EnableAutomaticButton(_T("Default"), RGB(230, 230, 230));

		pViewGroup->AddSubItem(pProperty);
	}

	// Selection Material
	{
		auto pMaterial = pOGLRenderer->getSelectedInstanceMaterial();

		auto pSelectedInstanceMateriaGroup = new CMFCPropertyGridProperty(_T("Selection Material"));

		// Ambient
		{
			auto pProperty = new CColorSelectorProperty(L"Ambient",
				RGB((BYTE)(pMaterial->getAmbientColor().r() * 255.f),
					(BYTE)(pMaterial->getAmbientColor().g() * 255.f),
					(BYTE)(pMaterial->getAmbientColor().b() * 255.f)),
				nullptr,
				L"Selection Color",
				(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::SelectionMaterial));
			pProperty->EnableOtherButton(_T("Other..."));
			pProperty->EnableAutomaticButton(_T("Default"), RGB(255, 0, 0));

			pSelectedInstanceMateriaGroup->AddSubItem(pProperty);
		}

		// Diffuse
		{
			auto pProperty = new CColorSelectorProperty(L"Diffuse",
				RGB((BYTE)(pMaterial->getDiffuseColor().r() * 255.f),
					(BYTE)(pMaterial->getDiffuseColor().g() * 255.f),
					(BYTE)(pMaterial->getDiffuseColor().b() * 255.f)),
				nullptr,
				L"Selection Color",
				(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::SelectionMaterial));
			pProperty->EnableOtherButton(_T("Other..."));
			pProperty->EnableAutomaticButton(_T("Default"), RGB(255, 0, 0));

			pSelectedInstanceMateriaGroup->AddSubItem(pProperty);
		}

		// Emissive
		/*{
			auto pProperty = new CColorSelectorProperty(L"Emissive",
				RGB((BYTE)(pMaterial->getEmissiveColor().r() * 255.f),
					(BYTE)(pMaterial->getEmissiveColor().g() * 255.f),
					(BYTE)(pMaterial->getEmissiveColor().b() * 255.f)),
				nullptr,
				L"Selection Color",
				(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::SelectionMaterial));
			pProperty->EnableOtherButton(_T("Other..."));
			pProperty->EnableAutomaticButton(_T("Default"), RGB(255, 0, 0));

			pSelectedInstanceMateriaGroup->AddSubItem(pProperty);
		}*/

		// Specular
		/*{
			auto pProperty = new CColorSelectorProperty(L"Specular",
				RGB((BYTE)(pMaterial->getSpecularColor().r() * 255.f),
					(BYTE)(pMaterial->getSpecularColor().g() * 255.f),
					(BYTE)(pMaterial->getSpecularColor().b() * 255.f)),
				nullptr,
				L"Selection Color",
				(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::SelectionMaterial));
			pProperty->EnableOtherButton(_T("Other..."));
			pProperty->EnableAutomaticButton(_T("Default"), RGB(255, 0, 0));

			pSelectedInstanceMateriaGroup->AddSubItem(pProperty);
		}*/

		// Transparency
		{
			auto pProperty = new CApplicationProperty(_T("Transparency"),
				(_variant_t)pMaterial->getA(),
				_T("Transparency"),
				(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::SelectionMaterial));
			pProperty->AllowEdit(TRUE);

			pSelectedInstanceMateriaGroup->AddSubItem(pProperty);
		}

		pViewGroup->AddSubItem(pSelectedInstanceMateriaGroup);
	}
	// Selection Material

	// Highlight Material
	{
		auto pMaterial = pOGLRenderer->getPointedInstanceMaterial();

		auto pPointedInstanceMateriaGroup = new CMFCPropertyGridProperty(_T("Highlight Material"));

		// Ambient
		{
			auto pProperty = new CColorSelectorProperty(L"Ambient",
				RGB((BYTE)(pMaterial->getAmbientColor().r() * 255.f),
					(BYTE)(pMaterial->getAmbientColor().g() * 255.f),
					(BYTE)(pMaterial->getAmbientColor().b() * 255.f)),
				nullptr,
				L"Highlight Color",
				(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::HighlightMaterial));
			pProperty->EnableOtherButton(_T("Other..."));
			pProperty->EnableAutomaticButton(_T("Default"), RGB(0, 0, 255));

			pPointedInstanceMateriaGroup->AddSubItem(pProperty);
		}

		// Diffuse
		{
			auto pProperty = new CColorSelectorProperty(L"Diffuse",
				RGB((BYTE)(pMaterial->getDiffuseColor().r() * 255.f),
					(BYTE)(pMaterial->getDiffuseColor().g() * 255.f),
					(BYTE)(pMaterial->getDiffuseColor().b() * 255.f)),
				nullptr,
				L"Highlight Color",
				(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::HighlightMaterial));
			pProperty->EnableOtherButton(_T("Other..."));
			pProperty->EnableAutomaticButton(_T("Default"), RGB(0, 0, 255));

			pPointedInstanceMateriaGroup->AddSubItem(pProperty);
		}

		// Emissive
		/*{
			auto pProperty = new CColorSelectorProperty(L"Emissive",
				RGB((BYTE)(pMaterial->getEmissiveColor().r() * 255.f),
					(BYTE)(pMaterial->getEmissiveColor().g() * 255.f),
					(BYTE)(pMaterial->getEmissiveColor().b() * 255.f)),
				nullptr,
				L"Highlight Color",
				(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::HighlightMaterial));
			pProperty->EnableOtherButton(_T("Other..."));
			pProperty->EnableAutomaticButton(_T("Default"), RGB(0, 0, 255));

			pPointedInstanceMateriaGroup->AddSubItem(pProperty);
		}*/

		// Specular
		/*{
			auto pProperty = new CColorSelectorProperty(L"Specular",
				RGB((BYTE)(pMaterial->getSpecularColor().r() * 255.f),
					(BYTE)(pMaterial->getSpecularColor().g() * 255.f),
					(BYTE)(pMaterial->getSpecularColor().b() * 255.f)),
				nullptr,
				L"Highlight Color",
				(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::HighlightMaterial));
			pProperty->EnableOtherButton(_T("Other..."));
			pProperty->EnableAutomaticButton(_T("Default"), RGB(0, 0, 255));

			pPointedInstanceMateriaGroup->AddSubItem(pProperty);
		}*/

		// Transparency
		{
			auto pProperty = new CApplicationProperty(_T("Transparency"),
				(_variant_t)pMaterial->getA(),
				_T("Transparency"),
				(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::HighlightMaterial));
			pProperty->AllowEdit(TRUE);

			pPointedInstanceMateriaGroup->AddSubItem(pProperty);
		}

		pViewGroup->AddSubItem(pPointedInstanceMateriaGroup);
	}
	// Highlight Material
#pragma endregion

#pragma region OpenGL
	auto pBlinnPhongProgram = pOGLRenderer->_getOGLProgramAs<_oglBlinnPhongProgram>();
	if (pBlinnPhongProgram != nullptr) {
		auto pOpenGL = new CMFCPropertyGridProperty(_T("OpenGL"));
		pViewGroup->AddSubItem(pOpenGL);

#pragma region Point light position
		{
			auto pPointLightingLocation = new CApplicationProperty(_T("Point lighting location"),
				(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::PointLightingLocation), TRUE);
			pPointLightingLocation->AllowEdit(FALSE);

			// X
			{
				auto pProperty = new CApplicationProperty(
					_T("X"),
					(_variant_t)pBlinnPhongProgram->_getPointLightingLocation().x,
					_T("X"),
					(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::PointLightingLocation));
				pPointLightingLocation->AddSubItem(pProperty);
			}

			// Y
			{
				auto pProperty = new CApplicationProperty(
					_T("Y"),
					(_variant_t)pBlinnPhongProgram->_getPointLightingLocation().y,
					_T("Y"),
					(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::PointLightingLocation));
				pPointLightingLocation->AddSubItem(pProperty);
			}

			// Z
			{
				auto pProperty = new CApplicationProperty(
					_T("Z"),
					(_variant_t)pBlinnPhongProgram->_getPointLightingLocation().z,
					_T("Z"),
					(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::PointLightingLocation));
				pPointLightingLocation->AddSubItem(pProperty);
			}

			pOpenGL->AddSubItem(pPointLightingLocation);
		}
#pragma endregion

#pragma region Ambient light weighting
		{
			auto pAmbientLightWeighting = new CApplicationProperty(_T("Ambient light weighting"),
				(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::AmbientLightWeighting), TRUE);
			pAmbientLightWeighting->AllowEdit(FALSE);

			// X
			{
				auto pProperty = new CApplicationProperty(
					_T("X"),
					(_variant_t)pBlinnPhongProgram->_getAmbientLightWeighting().x,
					_T("[0.0 - 1.0]"),
					(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::AmbientLightWeighting));
				pAmbientLightWeighting->AddSubItem(pProperty);
			}

			// Y
			{
				auto pProperty = new CApplicationProperty(
					_T("Y"),
					(_variant_t)pBlinnPhongProgram->_getAmbientLightWeighting().y,
					_T("[0.0 - 1.0]"),
					(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::AmbientLightWeighting));
				pAmbientLightWeighting->AddSubItem(pProperty);
			}

			// Z
			{
				auto pProperty = new CApplicationProperty(
					_T("Z"),
					(_variant_t)pBlinnPhongProgram->_getAmbientLightWeighting().z,
					_T("[0.0 - 1.0]"),
					(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::AmbientLightWeighting));
				pAmbientLightWeighting->AddSubItem(pProperty);
			}

			pOpenGL->AddSubItem(pAmbientLightWeighting);
		}
#pragma endregion

#pragma region Diffuse light weighting
		{
			auto pDiffuseLightWeighting = new CApplicationProperty(_T("Diffuse light weighting"),
				(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::DiffuseLightWeighting), TRUE);
			pDiffuseLightWeighting->AllowEdit(FALSE);

			// X
			{
				auto pProperty = new CApplicationProperty(
					_T("X"),
					(_variant_t)pBlinnPhongProgram->_getDiffuseLightWeighting().x,
					_T("[0.0 - 1.0]"),
					(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::DiffuseLightWeighting));
				pDiffuseLightWeighting->AddSubItem(pProperty);
			}

			// X
			{
				auto pProperty = new CApplicationProperty(
					_T("Y"),
					(_variant_t)pBlinnPhongProgram->_getDiffuseLightWeighting().y,
					_T("[0.0 - 1.0]"),
					(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::DiffuseLightWeighting));
				pDiffuseLightWeighting->AddSubItem(pProperty);
			}

			// Z
			{
				auto pProperty = new CApplicationProperty(
					_T("Z"),
					(_variant_t)pBlinnPhongProgram->_getDiffuseLightWeighting().z,
					_T("[0.0 - 1.0]"),
					(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::DiffuseLightWeighting));
				pDiffuseLightWeighting->AddSubItem(pProperty);
			}

			pOpenGL->AddSubItem(pDiffuseLightWeighting);
		}
#pragma endregion

#pragma region Specular light weighting
		{
			auto pSpecularLightWeighting = new CApplicationProperty(_T("Specular light weighting"),
				(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::SpecularLightWeighting), TRUE);
			pSpecularLightWeighting->AllowEdit(FALSE);

			// X
			{
				auto pProperty = new CApplicationProperty(
					_T("X"),
					(_variant_t)pBlinnPhongProgram->_getSpecularLightWeighting().x,
					_T("[0.0 - 1.0]"),
					(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::SpecularLightWeighting));
				pSpecularLightWeighting->AddSubItem(pProperty);
			}

			// X
			{
				auto pProperty = new CApplicationProperty(
					_T("Y"),
					(_variant_t)pBlinnPhongProgram->_getSpecularLightWeighting().y,
					_T("[0.0 - 1.0]"),
					(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::SpecularLightWeighting));
				pSpecularLightWeighting->AddSubItem(pProperty);
			}

			// Z
			{
				auto pProperty = new CApplicationProperty(
					_T("Z"),
					(_variant_t)pBlinnPhongProgram->_getSpecularLightWeighting().z,
					_T("[0.0 - 1.0]"),
					(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::SpecularLightWeighting));
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
				(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::MaterialShininess));

			pOpenGL->AddSubItem(pMaterialShininess);
		}
#pragma endregion

#pragma region Contrast
		{
			auto pContrast = new CApplicationProperty(
				_T("Contrast"),
				(_variant_t)pBlinnPhongProgram->_getContrast(),
				_T("Contrast"),
				(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::Contrast));

			pOpenGL->AddSubItem(pContrast);
		}
#pragma endregion

#pragma region Brightness
		{
			auto pBrightness = new CApplicationProperty(
				_T("Brightness"),
				(_variant_t)pBlinnPhongProgram->_getBrightness(),
				_T("Brightness"),
				(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::Brightness));

			pOpenGL->AddSubItem(pBrightness);
		}
#pragma endregion

#pragma region Gamma
		{
			auto pGamma = new CApplicationProperty(
				_T("Gamma"),
				(_variant_t)pBlinnPhongProgram->_getGamma(),
				_T("Gamma"),
				(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::Gamma));

			pOpenGL->AddSubItem(pGamma);
		}
#pragma endregion
	} // if (pBlinnPhongProgram != nullptr)
#pragma endregion

	m_wndPropList.AddProperty(pViewGroup);
}

void CPropertiesWnd::LoadInstanceProperties()
{
	m_wndPropList.RemoveAll();
	m_wndPropList.AdjustLayout();

	SetPropListFont();

	m_wndPropList.EnableHeaderCtrl(FALSE);
	m_wndPropList.EnableDescriptionArea();
	m_wndPropList.SetVSDotNetLook();
	m_wndPropList.MarkModifiedProperties();

	auto pController = getController();
	if (pController == nullptr) {
		ASSERT(FALSE);

		return;
	}

	if (getController()->getSelectedInstances().empty()) {
		return;
	}

	for (auto pInstance : getController()->getSelectedInstances()) {
		_ptr<_ap_instance> apInstance(pInstance);

		auto pModel = GetModelByInstance(sdaiGetInstanceModel(apInstance->getSdaiInstance()));
		if (pModel == nullptr) {
			return;
		}

		switch (pModel->getAP()) {
			case enumAP::STEP:
				{
					ASSERT(getController()->getSelectedInstances().size() == 1);

					LoadSTEPInstanceProperties();
				}
				break;

			case enumAP::IFC:
				{
					LoadIFCInstanceProperties(pModel, apInstance);
				}
				break;

			case enumAP::CIS2:
				{
					ASSERT(getController()->getSelectedInstances().size() == 1);

					LoadCIS2InstanceProperties();
				}
				break;

			default:
				{
					ASSERT(FALSE); // Unknown
				}
				break;
		} // switch (pModel->getAP())
	} // for (auto pInstance : ...
}

void CPropertiesWnd::LoadSTEPInstanceProperties()
{
	auto pController = getController();
	if (pController == nullptr) {
		ASSERT(FALSE);

		return;
	}

	ASSERT(pController->getModels().size() == 1);

	_ptr<_ap242_model> ap242Model(pController->getModels()[0], false);
	if (!ap242Model) {
		return;
	}

	auto pSelectedInstance = dynamic_cast<_ap242_instance*>(getController()->getSelectedInstance());
	if (pSelectedInstance == nullptr) {
		// Annotation Planes and Draughting Callouts have no properties
		return;
	}

	auto pPropertyProvider = ap242Model->getPropertyProvider();
	if (pPropertyProvider == nullptr) {
		return;
	}

	/*
	* Instance
	*/
	CString strGroupName;
	_ptr<_ap242_product_definition> apProductDefinition(pSelectedInstance->getGeometry(), false);
	if (apProductDefinition) {
		strGroupName.Format(L"#%lld %s", apProductDefinition->getExpressID(), apProductDefinition->getProductName());
	}
	else {
		strGroupName = ((_geometry*)pSelectedInstance->getGeometry())->getName();
	}

	/*
	* Properties
	*/
	SdaiInstance sdaiInstance = pSelectedInstance->getSdaiInstance();
	auto pPropertyCollection = pPropertyProvider->getPropertyCollection(sdaiInstance);
	if ((pPropertyCollection != nullptr) && !pPropertyCollection->properties().empty()) {
		auto pInstanceGroup = new CMFCPropertyGridProperty(strGroupName);

		for (auto pAP242Property : pPropertyCollection->properties()) {
			CString strExpressId;
			strExpressId.Format(L"property (#%i = PROPERTY_DEFINITION( ... ))", (int)internalGetP21Line(pAP242Property->getSdaiInstance()));

			auto pPropertyGroup = new CMFCPropertyGridProperty(strExpressId);
			pInstanceGroup->AddSubItem(pPropertyGroup);

			auto pProperty = new CMFCPropertyGridProperty(L"name", (_variant_t)(LPCSTR)CW2A(pAP242Property->getName().c_str()), L"name");
			pProperty->AllowEdit(FALSE);
			pPropertyGroup->AddSubItem(pProperty);

			pProperty = new CMFCPropertyGridProperty(L"description", (_variant_t)(LPCSTR)CW2A(pAP242Property->getDescription().c_str()), L"description");
			pProperty->AllowEdit(FALSE);
			pPropertyGroup->AddSubItem(pProperty);

			pProperty = new CMFCPropertyGridProperty(L"value", (_variant_t)(LPCSTR)CW2A(pAP242Property->getValue().c_str()), L"value");
			pProperty->AllowEdit(FALSE);
			pPropertyGroup->AddSubItem(pProperty);
		}

		m_wndPropList.AddProperty(pInstanceGroup);
	}
}

void CPropertiesWnd::LoadIFCInstanceProperties(_ap_model* pModel, _ap_instance* pInstance)
{
	ASSERT(pModel != nullptr);
	ASSERT(pInstance != nullptr);

	auto pController = getController();
	if (pController == nullptr) {
		ASSERT(FALSE);

		return;
	}

	auto pIFCModel = dynamic_cast<_ifc_model*>(pModel);
	if (pIFCModel == nullptr) {
		ASSERT(FALSE);

		return;
	}

	auto pPropertyProvider = pIFCModel->getPropertyProvider();

	auto pPropertySetCollection = pPropertyProvider->getPropertySetCollection(pInstance->getSdaiInstance());
	if (pPropertySetCollection == nullptr) {
		return;
	}

	auto pInstanceGridGroup = new CMFCPropertyGridProperty(pInstance->getName());

	for (auto pPropertySet : pPropertySetCollection->propertySets()) {
		auto pPropertySetGroup = new CMFCPropertyGridProperty(pPropertySet->getName().c_str());

		pInstanceGridGroup->AddSubItem(pPropertySetGroup);

		for (auto pProperty : pPropertySet->properties()) {
			wstring strValue = pProperty->getValue();
			if (!pProperty->getUnit().empty()) {
				strValue += L" ";
				strValue += pProperty->getUnit();
			}
			auto pGridProperty = new CMFCPropertyGridProperty(
				pProperty->getName().c_str(),
				(_variant_t)strValue.c_str(),
				pProperty->getDescription().c_str());
			pGridProperty->AllowEdit(FALSE);

			pPropertySetGroup->AddSubItem(pGridProperty);
		}
	}

	m_wndPropList.AddProperty(pInstanceGridGroup);

}

void CPropertiesWnd::LoadCIS2InstanceProperties()
{
	auto pController = getController();
	if (pController == nullptr) {
		ASSERT(FALSE);

		return;
	}

	ASSERT(pController->getModels().size() == 1);

	auto pModel = pController->getModels()[0];
	if (pModel == nullptr) {
		ASSERT(FALSE);

		return;
	}

	auto pCIS2Model = dynamic_cast<CCIS2Model*>(pModel);
	if (pCIS2Model == nullptr) {
		ASSERT(FALSE);

		return;
	}

	auto pInstance = dynamic_cast<CCIS2Instance*>(getController()->getSelectedInstance());
	if (pInstance == nullptr) {
		ASSERT(FALSE);

		return;
	}

	//#todo
	//auto pPropertyProvider = pCIS2Model->GetPropertyProvider();

	//auto pPropertySetCollection = pPropertyProvider->GetPropertySetCollection(pInstance->GetInstance());
	//if (pPropertySetCollection == nullptr)
	//{
	//	return;
	//}

	//auto pInstanceGridGroup = new CMFCPropertyGridProperty(pInstance->GetName().c_str());

	//for (auto pPropertySet : pPropertySetCollection->PropertySets())
	//{
	//	auto pPropertySetGroup = new CMFCPropertyGridProperty(pPropertySet->GetName().c_str());

	//	pInstanceGridGroup->AddSubItem(pPropertySetGroup);

	//	for (auto pProperty : pPropertySet->Properties())
	//	{
	//		auto pGridProperty = new CMFCPropertyGridProperty(
	//			pProperty->GetName().c_str(),
	//			(_variant_t)pProperty->GetValue().c_str(),
	//			pProperty->getDescription().c_str());
	//		pGridProperty->AllowEdit(FALSE);

	//		pPropertySetGroup->AddSubItem(pGridProperty);
	//	}
	//}

	//m_wndPropList.AddProperty(pInstanceGridGroup);
}

void CPropertiesWnd::SetInstancesToExplore()
{
	m_exploringStack.clear();

	if (auto pContoller = getController()) {
		for (auto& inst : pContoller->getSelectedInstances()) {
			if (inst) {
				_ptr<_ap_instance> apInstance(getController()->getSelectedInstances().front());
				if (auto sdaiInst = apInstance->getSdaiInstance()) {
					m_exploringStack.push_back({ sdaiInst, NULL });
				}
			}
		}
	}
}

void CPropertiesWnd::LoadInstanceAttributes()
{
	//
	m_wndPropList.SetRedraw(FALSE);

	CValueLocator selection = GetSelectedValueLocator();

	m_wndPropList.RemoveAll();
	m_wndPropList.AdjustLayout();

	SetPropListFont();

	m_wndPropList.EnableHeaderCtrl(FALSE);
	m_wndPropList.EnableDescriptionArea();
	m_wndPropList.SetVSDotNetLook();
	m_wndPropList.MarkModifiedProperties();

	if (!m_exploringStack.empty()) {

		CMFCPropertyGridProperty* pRoot = nullptr;
		CMFCPropertyGridProperty* pInstanceGroup = nullptr;
		AddInstanceNode(pRoot, pInstanceGroup);

		if (pInstanceGroup) {

			//add instance attributes
			CValueLocator locator;
			locator.sdaiInst = m_exploringStack.back().inst;

			auto sdaiModel = sdaiGetInstanceModel(locator.sdaiInst);
			engiEnableDerivedAttributes(sdaiModel, m_calculateDerivedAttributes);

			SdaiEntity entity = sdaiGetInstanceType(locator.sdaiInst);
			if (engiIsComplexEntity(entity)) {
				AddComplexInstanceProperties(pInstanceGroup, locator);
			}
			else {
				AddInstanceProperties(pInstanceGroup, locator, NULL);
			}
		}

		if (pRoot) {
			m_wndPropList.AddProperty(pRoot);
			SelectValue(selection);
		}
	}

	m_wndPropList.SetRedraw(TRUE);
	m_wndPropList.Invalidate();
}

void CPropertiesWnd::AddInstanceNode(CMFCPropertyGridProperty*& pRootNode, CMFCPropertyGridProperty*& pInstanceGroup)
{
	pRootNode = nullptr;
	pInstanceGroup = nullptr;

	for (auto& elem : m_exploringStack) {

		auto text = _ap_geometry::getDisplayString(elem.inst, false);

		if (elem.attrToNext) {
			auto attrName = engiGetAttrName(elem.attrToNext);
			text += L".";
			text += attrName ? CA2W(attrName) : L"?";
		}

		auto details = _ap_geometry::getDisplayString(elem.inst, true);

		//show parents in details
		auto entity = sdaiGetInstanceType(elem.inst);
		if (!engiIsComplexEntity(entity)) { //parents of complex entity embedded in entity name

			SdaiInteger iParent = 0;
			while (auto parent = engiGetEntityParentEx(entity, iParent++)) {
				if (iParent < 2)
					details += L"\nSUBTYPE OF ";
				else
					details += L", ";

				details += CA2W(engiGetEntityName(parent, sdaiSTRING));
			}
		}

		//
		auto pNode = new CMFCPropertyGridProperty(text.c_str());
		pNode->SetDescription(details.c_str());

		if (!pRootNode) {
			pRootNode = pNode;
		}

		if (pInstanceGroup) {
			pInstanceGroup->AddSubItem(pNode);
		}
		pInstanceGroup = pNode;
	}
}

void CPropertiesWnd::AddComplexInstanceProperties(CMFCPropertyGridProperty* pPropGroup, const CValueLocator& locator)
{
	auto entity = sdaiGetInstanceType(locator.sdaiInst);
	ASSERT(engiIsComplexEntity(entity));

	SdaiInteger i = 0;
	while (auto parent = engiGetEntityParentEx(entity, i++)) {

		if (auto parentName = (LPCTSTR)engiGetEntityName(parent, sdaiUNICODE)) {

			if (auto pParentGroup = new CMFCPropertyGridProperty(parentName)) {
				pPropGroup->AddSubItem(pParentGroup);

				AddInstanceProperties(pParentGroup, locator, parent);
			}
		}
	}
}

void CPropertiesWnd::AddInstanceProperties(CMFCPropertyGridProperty* pPropGroup, CValueLocator locator, SdaiEntity partOfComplex)
{
	SdaiEntity attrSource = partOfComplex ? partOfComplex : sdaiGetInstanceType(locator.sdaiInst);

	//add attributes	
	SdaiAttr attr = NULL;
	while (NULL != (attr = engiGetEntityAttributeByIterator(attrSource, attr))) {

		locator.sdaiAttr = attr;

		if (partOfComplex) {
			auto definedBy = engiGetAttrDefiningEntity(attr);
			if (definedBy != partOfComplex)
				continue; //>>>>
		}

		if (auto prop = CreateAttributeProperty(locator)) {
			pPropGroup->AddSubItem(prop);
		}
		else ASSERT(false);
	}
}

static CStringA Card2Str(SdaiInteger card)
{
	if (card < 0)
		return "*";

	CStringA str;
	str.Format("%lld", card);
	return str;
}

static CStringA GetAttributeDetails(SdaiEntity entity, SdaiAttr attr)
{
	CStringA details;

	if (engiIsAttrInverse(attr)) {
		details += "INVERSE ";
	}

	auto derived = engiGetAttrDerived(NULL/*do not check redeclarations*/, attr);
	if (derived) {
		details += "DERIVED ";
	}

	auto definingEntity = engiGetAttrDefiningEntity(attr);
	details += engiGetEntityName(definingEntity, sdaiSTRING);
	details += ".";

	details += engiGetAttrName(attr);
	details += " : ";

	if (engiIsAttrOptional(attr)) {
		details += "OPTIONAL ";
	}

	auto aggr = engiGetAttrAggregation(attr);
	while (aggr) {
		enum_express_aggr aggrType = enum_express_aggr::__NONE;
		SdaiInteger cardMin = -1;
		SdaiInteger cardMax = -1;
		bool optional = false;
		bool unique = false;
		engiGetAggregationDefinition(aggr, &aggrType, &cardMin, &cardMax, &optional, &unique, &aggr);

		switch (aggrType) {
			case enum_express_aggr::__ARRAY:		details += "ARRAY OF ";		break;
			case enum_express_aggr::__BAG:			details += "BAG OF ";		break;
			case enum_express_aggr::__LIST:			details += "LIST OF";		break;
			case enum_express_aggr::__SET:			details += "SET OF";		break;
			case enum_express_aggr::__AGGREGATE:	details += "AGGRGATE OF ";	break;
			default: ASSERT(false);
		}

		details += "[";
		details += Card2Str(cardMin);
		details += ":";
		details += Card2Str(cardMax);
		details += "] ";

		if (optional)
			details += "OPTIONAL ";
		if (unique)
			details += "UNIQUE ";
	}

	auto type = engiGetExpressAttrType(attr);
	switch (type) {
		case enum_express_data_type::__NONE:			break; //will take domain later
		case enum_express_data_type::__BINARY:
		case enum_express_data_type::__BINARY_32:		details += "BINARY ";		break;
		case enum_express_data_type::__BOOLEAN:			details += "BOOLEAN ";		break;
		case enum_express_data_type::__ENUMERATION:		details += "ENUM ";			break;
		case enum_express_data_type::__INTEGER:			details += "INTEGER ";		break;
		case enum_express_data_type::__LOGICAL:			details += "LOGICAL ";		break;
		case enum_express_data_type::__NUMBER:			details += "NUMBER ";		break;
		case enum_express_data_type::__REAL:			details += "REAL ";			break;
		case enum_express_data_type::__SELECT:			details += "SELECT ";		break;
		case enum_express_data_type::__STRING:			details += "STRING ";		break;
		case enum_express_data_type::__GENERIC:			details += "GENERIC ";		break;
		default: ASSERT(FALSE);
	}

	auto domain = engiGetAttrDomainName(attr);
	if (domain && *domain) {
		details += domain;
		details += " ";
	}

	if (derived) {
		//TODO give information where it is derived
		details += " := ";

		SdaiString label = NULL;
		SdaiString text = NULL;
		engiGetScriptText(derived, &label, &text);
		if (label && *label) {
			ASSERT(!"Not expected");
			details += label;
			details += ": ";
		}
		if (text && *text) {
			details += text;
		}
	}

	SdaiAttr redeclaration = NULL;
	while (NULL != (redeclaration = engiGetAttrRedeclarationByIterator(entity, attr, redeclaration))) {
		details += "\n  REDECLARATION ";
		details += GetAttributeDetails(engiGetAttrDefiningEntity(redeclaration), redeclaration);
	}

	return details;
}

static CStringA GetNullTypeName(const char* val)
{
	/*	if (*val == '$')
			return "unset";
		if (*val == '*')
			return "<not calculated>";
		if (!val)*/
	return "";
}

CMFCPropertyGridProperty* CPropertiesWnd::CreateAttributeProperty(const CValueLocator& locator)
{
	auto entity = sdaiGetInstanceType(locator.sdaiInst);
	bool complex = engiIsComplexEntity(entity);
	auto definingEntity = engiGetAttrDefiningEntity(locator.sdaiAttr);
	auto sdaiType = engiGetInstanceAttrType(locator.sdaiInst, locator.sdaiAttr);
	auto entityName = engiGetEntityName(definingEntity, sdaiSTRING);
	auto attrName = engiGetAttrName(locator.sdaiAttr);

	CStringA valueName;
	if (engiIsAttrExplicit(locator.sdaiAttr)) {
		int_t argNum = engiGetEntityAttributePosition(complex ? definingEntity : entity, locator.sdaiAttr, !complex);
		ASSERT(argNum >= 0);
		if (argNum >= 0)
			valueName.Format("%d. ", (int)argNum + 1);
	}
	else if (engiIsAttrInverse(locator.sdaiAttr)) {
		valueName += "inverse ";
	}
	else if (engiGetAttrDerived(entity, locator.sdaiAttr)) {
		valueName += "derived ";
	}
	else {
		ASSERT(FALSE);
	}

	if (!sdaiType && engiGetAttrDerived(entity, locator.sdaiAttr) && m_calculateDerivedAttributes) {
		sdaiType = sdaiADB;
	}

	valueName += attrName;

	CStringA details = GetAttributeDetails(entity, locator.sdaiAttr);

	GridPropertyInfo prop;

	switch (sdaiType) {
		case sdaiADB:
			{
				SdaiADB adb = NULL;
				auto res = sdaiGetAttr(locator.sdaiInst, locator.sdaiAttr, sdaiADB, &adb);
				if (res && adb) {
					prop = CreateADBGridProperty(adb, locator, details);
				}
				else {
					ASSERT(engiGetAttrDerived(entity, locator.sdaiAttr)); //derived attribute can return ?-undefined 
					prop.pProp = CreateValueGridProperty("$", locator, details);
					prop.typeName = GetNullTypeName("undefined");
				}
			}
			break;

		case sdaiAGGR:
			{
				SdaiAggr aggr = NULL;
				auto res = sdaiGetAttr(locator.sdaiInst, locator.sdaiAttr, sdaiAGGR, &aggr);
				ASSERT(res && aggr);
				prop = CreateAggrGridProperty(aggr, locator, details);
			}
			break;

		case sdaiINSTANCE:
			{
				SdaiInstance inst = NULL;
				auto res = sdaiGetAttr(locator.sdaiInst, locator.sdaiAttr, sdaiType, &inst);
				if (!inst) {
					ASSERT(!engiIsAttrExplicit(locator.sdaiAttr));
					prop.pProp = CreateValueGridProperty("", locator, details);
				}
				else {
					prop.pProp = CreateInstanceGridProperty(inst, locator, details);
				}
				prop.typeName = GetPrimitiveSdaiTypeName(sdaiType);
			}
			break;

		case 0:
			{
				const char* val = NULL;
				sdaiGetAttr(locator.sdaiInst, locator.sdaiAttr, sdaiEXPRESSSTRING, &val);
				ASSERT(val || !engiIsAttrExplicit(locator.sdaiAttr));
				prop.pProp = CreateValueGridProperty(val, locator, details);
				prop.typeName = GetNullTypeName(val);
			}
			break;

		default:
			{
				char* sz = NULL;
				auto res = sdaiGetAttr(locator.sdaiInst, locator.sdaiAttr, sdaiSTRING, &sz);
				ASSERT(res && sz);
				prop.pProp = CreateValueGridProperty(sz, locator, details);
				prop.typeName = GetPrimitiveSdaiTypeName(sdaiType);
			}
	} // switch sdaiType

	if (prop.pProp) {
		if (!prop.typeName.IsEmpty()) {
			valueName += " : " + prop.typeName;
		}
		prop.pProp->SetName((LPCTSTR)CA2W(valueName));
	}

	return prop.pProp;
}

static bool IsPersistent(SdaiInstance inst)
{
	return internalGetP21Line(inst) > 0;
}


CMFCPropertyGridProperty* CPropertiesWnd::CreateValueGridProperty(LPCSTR value, const CValueLocator& locator, LPCSTR details)
{
	CString propDetails = CA2W(details);
	CString propValue = CA2W(value);
	auto* data = new CPropertyGridData;
	data->valueLocator = locator;

	auto pAttributeValue = new CInstanceAttributeProperty(L"Will set later", propValue, propDetails, (DWORD_PTR)data);

	pAttributeValue->AllowEdit(IsPersistent(locator.sdaiInst) && engiIsAttrExplicit(locator.sdaiAttr));
	pAttributeValue->Enable(IsPersistent(locator.sdaiInst) && engiIsAttrExplicit(locator.sdaiAttr));

	return pAttributeValue;
}

CMFCPropertyGridProperty* CPropertiesWnd::CreateInstanceGridProperty(SdaiInstance inst, const CValueLocator& locator, LPCSTR details)
{
	auto stepId = internalGetP21Line(inst);
	auto entity = sdaiGetInstanceType(inst);
	auto entityName = engiGetEntityName(entity, sdaiSTRING);

	CStringA value;
	value.Format("#%lld %s", stepId, entityName ? entityName : "");

	return CreateValueGridProperty(value, locator, details);
}

GridPropertyInfo CPropertiesWnd::CreateADBGridProperty(SdaiADB adb, const CValueLocator& locator, LPCSTR details)
{
	GridPropertyInfo prop;

	auto typePath = sdaiGetADBTypePath(adb, 0);
	if (typePath && *typePath) {
		prop.typeName = typePath;
		prop.typeName += " as ";
	}

	auto sdaiType = sdaiGetADBType(adb);

	switch (sdaiType) {
		case sdaiADB:
			{
				ASSERT(!"Not expected");
				SdaiADB sub = NULL;
				auto res = sdaiGetADBValue(adb, sdaiADB, &sub);
				ASSERT(res && sub && sub != adb);
				auto subProp = CreateADBGridProperty(sub, locator, details);
				prop.typeName += subProp.typeName;
				prop.pProp = subProp.pProp;
			}
			break;

		case sdaiAGGR:
			{
				SdaiAggr aggr = NULL;
				auto res = sdaiGetADBValue(adb, sdaiAGGR, &aggr);
				ASSERT(res && aggr);
				auto subProp = CreateAggrGridProperty(aggr, locator, details);
				prop.typeName += subProp.typeName;
				prop.pProp = subProp.pProp;
			}
			break;

		case sdaiINSTANCE:
			{
				SdaiInstance inst = NULL;
				auto res = sdaiGetADBValue(adb, sdaiINSTANCE, &inst);
				ASSERT(res && inst);
				prop.typeName += GetPrimitiveSdaiTypeName(sdaiType);
				prop.pProp = CreateInstanceGridProperty(inst, locator, details);
			}
			break;

		case 0:
			{
				ASSERT(!"Not expected");
				const char* val = NULL;
				sdaiGetADBValue(adb, sdaiEXPRESSSTRING, &val);
				ASSERT(val);
				prop.typeName += GetNullTypeName(val);
				prop.pProp = CreateValueGridProperty(val, locator, details);
			}
			break;

		default:
			{
				char* sz = NULL;
				auto res = sdaiGetADBValue(adb, sdaiSTRING, &sz);
				ASSERT(res && sz);
				prop.typeName += GetPrimitiveSdaiTypeName(sdaiType);
				prop.pProp = CreateValueGridProperty(sz, locator, details);
			}
	} // switch sdaiType

	return prop;
}


GridPropertyInfo CPropertiesWnd::CreateAggrGridProperty(SdaiAggr aggr, const CValueLocator& locator, LPCSTR details)
{
	GridPropertyInfo prop;

	SdaiInteger iMemberCount = sdaiGetMemberCount(aggr);

	prop.pProp = new CMFCPropertyGridProperty(L"WILL SET LATER");
	prop.pProp->SetDescription((LPCTSTR)CA2W(details));

	CValueLocator itemLocator = locator; // For nested structures we need to update locator with attribute index

	itemLocator.indexes.push_back(-1);
	SdaiInteger& ind = itemLocator.indexes.back(); // Reference to index for easier update in the loop

	for (ind = 0; ind < iMemberCount && ind < 10; ind++) {

		auto item = CreateAggrItemGridProperty(aggr, ind, itemLocator, details);

		if (item.pProp) {
			prop.pProp->AddSubItem(item.pProp);

			CStringA indexName;
			indexName.Format("[%lld] %s", ind, item.typeName.GetString());
			item.pProp->SetName((LPCTSTR)CA2W(indexName));

			if (prop.typeName.IsEmpty()) {
				prop.typeName = item.typeName;
			}
			else if (prop.typeName != item.typeName) {
				prop.typeName = "<mixed types>";
			}
		}
	}

	if (ind < iMemberCount) {
		auto more = new CMFCPropertyGridProperty(L"...", L"...");
		more->AllowEdit(FALSE);
		prop.pProp->AddSubItem(more);
	}

	prop.typeName = "AGGR (" + prop.typeName + ")";

	return prop;
}

GridPropertyInfo CPropertiesWnd::CreateAggrItemGridProperty(SdaiAggr aggr, SdaiInteger index, const CValueLocator& itemLocator, LPCSTR details)
{
	GridPropertyInfo prop;

	SdaiPrimitiveType sdaiType = 0;
	engiGetAggrType(aggr, &sdaiType);

	switch (sdaiType) {
		case sdaiADB:
			{
				SdaiADB adb = NULL;
				auto res = sdaiGetAggrByIndex(aggr, index, sdaiADB, &adb);
				ASSERT(res && adb);
				return CreateADBGridProperty(adb, itemLocator, details);
			}
			break;

		case sdaiAGGR:
			{
				SdaiAggr sub = NULL;
				auto res = sdaiGetAggrByIndex(aggr, index, sdaiAGGR, &sub);
				ASSERT(res && sub);
				return CreateAggrGridProperty(sub, itemLocator, details);
			}
			break;

		case sdaiINSTANCE:
			{
				SdaiInstance inst = NULL;
				auto res = sdaiGetAggrByIndex(aggr, index, sdaiINSTANCE, &inst);
				ASSERT(res && inst);
				auto typeName = GetPrimitiveSdaiTypeName(sdaiType);
				auto pProp = CreateInstanceGridProperty(inst, itemLocator, details);
				return { pProp, typeName };
			}
			break;

		case 0:
			{
				const char* val = NULL;
				sdaiGetAggrByIndex(aggr, index, sdaiEXPRESSSTRING, &val);
				ASSERT(val);
				auto typeName = GetNullTypeName(val);
				auto pProp = CreateValueGridProperty(val, itemLocator, details);
				return { pProp, typeName };
			}

		default:
			{
				char* sz = NULL;
				auto res = sdaiGetAggrByIndex(aggr, index, sdaiSTRING, &sz);
				ASSERT(res && sz);
				auto typeName = GetPrimitiveSdaiTypeName(sdaiType);
				auto pProp = CreateValueGridProperty(sz, itemLocator, details);
				return { pProp, typeName };
			}
	} // switch sdaiType
}

CValueLocator CPropertiesWnd::GetSelectedValueLocator() const
{
	auto prop = m_wndPropList.GetCurSel();
	return GetValueLocator(prop);
}

CValueLocator CPropertiesWnd::GetValueLocator(CMFCPropertyGridProperty* pProp) const
{
	auto propData = pProp ? (CPropertyGridData*)pProp->GetData() : NULL;
	if (propData) {
		return propData->valueLocator;
	}
	else {
		return CValueLocator();
	}
}

bool CPropertiesWnd::SelectValue(const CValueLocator& locator)
{
	if (locator.sdaiInst && locator.sdaiAttr) {
		auto N = m_wndPropList.GetPropertyCount();
		for (int i = 0; i < N; i++) {
			auto prop = m_wndPropList.GetProperty(i);
			if (SelectValue(prop, locator)) {
				return true;
			}
		}
	}
	return false;
}

bool CPropertiesWnd::SelectValue(CMFCPropertyGridProperty* scope, const CValueLocator& locator)
{
	auto loc = GetValueLocator(scope);
	if (loc == locator) {
		m_wndPropList.SetCurSel(scope);
		return true;
	}

	int N = scope->GetSubItemsCount();
	for (int i = 0; i < N; i++) {
		auto prop = scope->GetSubItem(i);
		if (SelectValue(prop, locator)) {
			return true;
		}
	}

	return false;
}

static SdaiInstance GetInstanceFromAggregation(SdaiAggr aggr, const std::vector<SdaiInteger>& indexes, size_t level)
{
	if (aggr && indexes.size()) {
		if (level == indexes.size() - 1) {
			SdaiInstance inst = NULL;
			sdaiGetAggrByIndex(aggr, indexes[level], sdaiINSTANCE, &inst);
			return inst;
		}
		else if (level < indexes.size() - 1) {
			SdaiAggr nested = NULL;
			sdaiGetAggrByIndex(aggr, indexes[level], sdaiAGGR, &nested);
			return GetInstanceFromAggregation(nested, indexes, level + 1);
		}
	}
	return NULL;
}

SdaiInstance CPropertiesWnd::GetSelectedValueInstance()
{
	auto selection = GetSelectedValueLocator();
	if (selection.sdaiInst && selection.sdaiAttr) {

		//try instance
		SdaiInstance inst = NULL;
		sdaiGetAttr(selection.sdaiInst, selection.sdaiAttr, sdaiINSTANCE, &inst);
		if (inst)
			return inst;

		//try aggregation
		SdaiAggr aggr = NULL;
		sdaiGetAttr(selection.sdaiInst, selection.sdaiAttr, sdaiAGGR, &aggr);
		return GetInstanceFromAggregation(aggr, selection.indexes, 0);
	}
	return NULL;
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
	switch (m_wndObjectCombo.GetCurSel()) {
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

		case 2: // Instance Attributes
			{
				LoadInstanceAttributes();
			}
			break;

		default:
			{
				ASSERT(FALSE); // unknown mode
			}
			break;
	}
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
	ASSERT(getController() != nullptr);
	getController()->unRegisterView(this);

	__super::OnDestroy();
}
