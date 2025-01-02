
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
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// ************************************************************************************************
#define TRUE_VALUE_PROPERTY L"Yes"
#define FALSE_VALUE_PROPERTY L"No"
#define ROTATION_MODE_XY L"2D"
#define ROTATION_MODE_XYZ L"3D"

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
{
}

CApplicationProperty::CApplicationProperty(const CString& strGroupName, DWORD_PTR dwData, BOOL bIsValueList)
	: CMFCPropertyGridProperty(strGroupName, dwData, bIsValueList)
{
}

/*virtual*/ CApplicationProperty::~CApplicationProperty()
{
	delete (CApplicationPropertyData *)GetData();
}

// ************************************************************************************************
/*virtual*/ void CPropertiesWnd::onModelLoaded()
{
	m_wndObjectCombo.SetCurSel(0 /*Application*/);

	LoadApplicationProperties();
}

/*virtual*/ void CPropertiesWnd::onShowMetaInformation()
{
	m_wndObjectCombo.SetCurSel(1 /*Properties*/);

	ASSERT(FALSE); // TODO
}

/*virtual*/ void CPropertiesWnd::onInstanceSelected(_view* /*pSender*/)
{
	m_wndObjectCombo.SetCurSel(1 /*Properties*/);

	LoadInstanceProperties();
}

/*virtual*/ void CPropertiesWnd::onApplicationPropertyChanged(_view* pSender, enumApplicationProperty /*enApplicationProperty*/)
{
	if (pSender == this)
	{
		return;
	}

	if (m_wndObjectCombo.GetCurSel() == 0)
	{
		LoadApplicationProperties();
	}
}

/*afx_msg*/ LRESULT CPropertiesWnd::OnPropertyChanged(__in WPARAM /*wparam*/, __in LPARAM lparam)
{
	auto pController = getController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return 0;
	}

#pragma region Application
	if (m_wndObjectCombo.GetCurSel() == 0)
	{
		auto pRenderer = getController()->getViewAs<_oglRenderer>();
		if (pRenderer == nullptr)
		{
			ASSERT(FALSE);

			return 0;
		}

		auto pBlinnPhongProgram = pRenderer->_getOGLProgramAs<_oglBlinnPhongProgram>();
		auto pApplicationProperty = dynamic_cast<CApplicationProperty*>((CMFCPropertyGridProperty*)lparam);
		if (pApplicationProperty != nullptr)
		{
			CString strValue = pApplicationProperty->GetValue();

			auto pData = (CApplicationPropertyData*)pApplicationProperty->GetData();
			if (pData == nullptr)
			{
				ASSERT(FALSE);

				return 0;
			}

			switch (pData->GetType())
			{
				case enumApplicationProperty::ShowFaces:
				{
					pRenderer->setShowFaces(strValue == TRUE_VALUE_PROPERTY ? TRUE : FALSE);

					getController()->onApplicationPropertyChanged(this, enumApplicationProperty::ShowFaces);
				}
				break;

				case enumApplicationProperty::CullFaces:
				{
					pRenderer->setCullFacesMode(strValue);

					getController()->onApplicationPropertyChanged(this, enumApplicationProperty::CullFaces);
				}
				break;

				case enumApplicationProperty::ShowConceptualFacesWireframes:
				{
					pRenderer->setShowConceptualFacesPolygons(strValue == TRUE_VALUE_PROPERTY ? TRUE : FALSE);

					getController()->onApplicationPropertyChanged(this, enumApplicationProperty::ShowConceptualFacesWireframes);
				}
				break;

				case enumApplicationProperty::ShowLines:
				{
					pRenderer->setShowLines(strValue == TRUE_VALUE_PROPERTY ? TRUE : FALSE);

					getController()->onApplicationPropertyChanged(this, enumApplicationProperty::ShowLines);
				}
				break;

				case enumApplicationProperty::ShowPoints:
				{
					pRenderer->setShowPoints(strValue == TRUE_VALUE_PROPERTY ? TRUE : FALSE);

					getController()->onApplicationPropertyChanged(this, enumApplicationProperty::ShowPoints);
				}
				break;

				case enumApplicationProperty::RotationMode:
				{
					pRenderer->_setRotationMode(strValue == ROTATION_MODE_XY ? enumRotationMode::XY : enumRotationMode::XYZ);

					getController()->onApplicationPropertyChanged(this, enumApplicationProperty::RotationMode);
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

				default:
					ASSERT(FALSE);
					break;
			} // switch (pData->GetType())

			return 0;
		} // if (pApplicationProperty != nullptr)
	} // if (m_wndObjectCombo.GetCurSel() == 0)
#pragma endregion

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
	if (GetSafeHwnd () == nullptr || (AfxGetMainWnd() != nullptr && AfxGetMainWnd()->IsIconic()))
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_toolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndObjectCombo.SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), m_nComboHeight, SWP_NOACTIVATE | SWP_NOZORDER);
	m_toolBar.SetWindowPos(nullptr, rectClient.left, rectClient.top + m_nComboHeight, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndPropList.SetWindowPos(nullptr, rectClient.left, rectClient.top + m_nComboHeight + cyTlb, rectClient.Width(), rectClient.Height() -(m_nComboHeight+cyTlb), SWP_NOACTIVATE | SWP_NOZORDER);
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

	if (!m_wndObjectCombo.Create(dwViewStyle, rectDummy, this, ID_COMBO_PROPERTIES_VIEW))
	{
		ASSERT(FALSE);

		return -1;
	}

	m_wndObjectCombo.AddString(_T("Application"));
	m_wndObjectCombo.AddString(_T("Properties"));
	m_wndObjectCombo.SetCurSel(0);

	CRect rectCombo;
	m_wndObjectCombo.GetClientRect (&rectCombo);

	m_nComboHeight = rectCombo.Height();

	if (!m_wndPropList.Create(WS_VISIBLE | WS_CHILD, rectDummy, this, 2))
	{
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
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pRenderer = getController()->getViewAs<_oglRenderer>();
	if (pRenderer == nullptr)
	{
		return;
	}

#pragma region View
	auto pViewGroup = new CMFCPropertyGridProperty(_T("View"));	

	{
		auto pProperty = new CApplicationProperty(_T("Faces"),
			pRenderer->getShowFaces(nullptr) ? TRUE_VALUE_PROPERTY : FALSE_VALUE_PROPERTY, _T("Faces"),
			(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::ShowFaces));
		pProperty->AddOption(TRUE_VALUE_PROPERTY);
		pProperty->AddOption(FALSE_VALUE_PROPERTY);
		pProperty->AllowEdit(FALSE);

		pViewGroup->AddSubItem(pProperty);
	}

	{
		CString strCullFacesMode = pRenderer->getCullFacesMode(nullptr);

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
			pRenderer->getShowConceptualFacesPolygons(nullptr) ? TRUE_VALUE_PROPERTY : FALSE_VALUE_PROPERTY,
			_T("Conceptual faces wireframes"),
			(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::ShowConceptualFacesWireframes));
		pProperty->AddOption(TRUE_VALUE_PROPERTY);
		pProperty->AddOption(FALSE_VALUE_PROPERTY);
		pProperty->AllowEdit(FALSE);

		pViewGroup->AddSubItem(pProperty);
	}

	{
		auto pProperty = new CApplicationProperty(_T("Lines"), pRenderer->getShowLines(nullptr) ? TRUE_VALUE_PROPERTY : FALSE_VALUE_PROPERTY,
			_T("Lines"),
			(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::ShowLines));
		pProperty->AddOption(TRUE_VALUE_PROPERTY);
		pProperty->AddOption(FALSE_VALUE_PROPERTY);
		pProperty->AllowEdit(FALSE);

		pViewGroup->AddSubItem(pProperty);
	}

	{
		auto pProperty = new CApplicationProperty(_T("Points"), pRenderer->getShowPoints(nullptr) ? TRUE_VALUE_PROPERTY : FALSE_VALUE_PROPERTY,
			_T("Points"),
			(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::ShowPoints));
		pProperty->AddOption(TRUE_VALUE_PROPERTY);
		pProperty->AddOption(FALSE_VALUE_PROPERTY);
		pProperty->AllowEdit(FALSE);

		pViewGroup->AddSubItem(pProperty);
	}

	{
		auto pProperty = new CApplicationProperty(
			_T("Rotation mode"), 
			pRenderer->_getRotationMode() == enumRotationMode::XY ? ROTATION_MODE_XY : ROTATION_MODE_XYZ,
			_T("XY/XYZ"),
			(DWORD_PTR)new CApplicationPropertyData(enumApplicationProperty::RotationMode));
		pProperty->AddOption(ROTATION_MODE_XY);
		pProperty->AddOption(ROTATION_MODE_XYZ);
		pProperty->AllowEdit(FALSE);

		pViewGroup->AddSubItem(pProperty);
	}
#pragma endregion
	
#pragma region OpenGL
	auto pBlinnPhongProgram = pRenderer->_getOGLProgramAs<_oglBlinnPhongProgram>();
	if (pBlinnPhongProgram != nullptr)
	{
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
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return;
	}	

	if (getController()->getSelectedInstance() == nullptr)
	{
		return;
	}

	_ptr<_ap_model> model(pController->getModel());
	if (!model)
	{
		return;
	}

	switch (model.p()->getAP())
	{
		case enumAP::STEP:
		{
			LoadSTEPInstanceProperties();
		}
		break;

		case enumAP::IFC:
		{
			LoadIFCInstanceProperties();
		}
		break;

		case enumAP::CIS2:
		{
			LoadCIS2InstanceProperties();
		}
		break;

		default:
		{
			ASSERT(FALSE); // Unknown
		}
		break;
	} // switch (model.p()->GetType())
}

void CPropertiesWnd::LoadSTEPInstanceProperties()
{
	auto pController = getController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	_ptr<_ap_model> model(pController->getModel());
	if (!model)
	{
		return;
	}

	auto pSelectedInstance = dynamic_cast<_ap242_instance*>(getController()->getSelectedInstance());
	if (pSelectedInstance == nullptr)
	{
		// Annotation Planes and Draughting Callouts have no properties
		return;
	}

	/*
	* Instance
	*/
	auto pInstanceGroup = new CMFCPropertyGridProperty(pSelectedInstance->getProductDefinition()->getId());

	/*
	* Properties
	*/
	SdaiAggr sdaiPropertyDefinitionAggr = sdaiGetEntityExtentBN(model.p()->getSdaiModel(), "PROPERTY_DEFINITION");
	SdaiInteger iPropertyDefinitionsCount = sdaiGetMemberCount(sdaiPropertyDefinitionAggr);
	for (SdaiInteger i = 0; i < iPropertyDefinitionsCount; i++)
	{
		SdaiInstance sdaiPropertyDefinitionInstance = 0;
		sdaiGetAggrByIndex(sdaiPropertyDefinitionAggr, i, sdaiINSTANCE, &sdaiPropertyDefinitionInstance);

		SdaiInstance sdaiDefinitionInstance = 0;
		sdaiGetAttrBN(sdaiPropertyDefinitionInstance, "definition", sdaiINSTANCE, &sdaiDefinitionInstance);
		if (sdaiDefinitionInstance == pSelectedInstance->getProductDefinition()->getSdaiInstance())
		{
			CString strValue;
			strValue.Format(L"property (#%i = PROPERTY_DEFINITION( ... ))", (int)internalGetP21Line(sdaiPropertyDefinitionInstance));

			auto pPropertyGroup = new CMFCPropertyGridProperty(strValue);
			pInstanceGroup->AddSubItem(pPropertyGroup);

			char* szName = nullptr;
			sdaiGetAttrBN(sdaiPropertyDefinitionInstance, "name", sdaiSTRING, &szName);

			auto pProperty = new CMFCPropertyGridProperty(L"name", (_variant_t)szName, L"name");
			pProperty->AllowEdit(FALSE);
			pPropertyGroup->AddSubItem(pProperty);

			char* szDescription = nullptr;
			sdaiGetAttrBN(sdaiPropertyDefinitionInstance, "description", sdaiSTRING, &szDescription);

			pProperty = new CMFCPropertyGridProperty(L"description", (_variant_t)szName, L"description");
			pProperty->AllowEdit(FALSE);
			pPropertyGroup->AddSubItem(pProperty);

			//
			//	Lookup value (not using inverse relations)
			//
			SdaiAggr sdaiPropertyDefinitionRepresentationAggr = sdaiGetEntityExtentBN(model.p()->getSdaiModel(), "PROPERTY_DEFINITION_REPRESENTATION");
			SdaiInteger	iPropertyDefinitionRepresentationsCount = sdaiGetMemberCount(sdaiPropertyDefinitionRepresentationAggr);
			for (SdaiInteger j = 0; j < iPropertyDefinitionRepresentationsCount; j++) 
			{
				SdaiInstance sdaiPropertyDefinitionRepresentationInstance = 0;
				sdaiGetAggrByIndex(sdaiPropertyDefinitionRepresentationAggr, j, sdaiINSTANCE, &sdaiPropertyDefinitionRepresentationInstance);

				SdaiInstance sdaiPDRDefinitionInstance = 0;
				sdaiGetAttrBN(sdaiPropertyDefinitionRepresentationInstance, "definition", sdaiINSTANCE, &sdaiPDRDefinitionInstance);
				if (sdaiPDRDefinitionInstance == sdaiPropertyDefinitionInstance)
				{
					SdaiInstance sdaiUsedRepresentationInstance = 0;
					sdaiGetAttrBN(sdaiPropertyDefinitionRepresentationInstance, "used_representation", sdaiINSTANCE, &sdaiUsedRepresentationInstance);

					SdaiAggr sdaiItemsAggr = nullptr;
					sdaiGetAttrBN(sdaiUsedRepresentationInstance, "items", sdaiAGGR, &sdaiItemsAggr);
					SdaiInteger	iItemsCount = sdaiGetMemberCount(sdaiItemsAggr);
					for (SdaiInteger k = 0; k < iItemsCount; k++)
					{
						SdaiInstance sdaiItemInstance = 0;
						sdaiGetAggrByIndex(sdaiItemsAggr, k, sdaiINSTANCE, &sdaiItemInstance);

						if (sdaiGetInstanceType(sdaiItemInstance) == sdaiGetEntity(model.p()->getSdaiModel(), "DESCRIPTIVE_REPRESENTATION_ITEM")) 
						{
							char* szDescription = nullptr;
							sdaiGetAttrBN(sdaiItemInstance, "description", sdaiSTRING, &szDescription);

							pProperty = new CMFCPropertyGridProperty(L"value", (_variant_t)szDescription, L"value");
							pProperty->AllowEdit(FALSE);
							pPropertyGroup->AddSubItem(pProperty);
						} // DESCRIPTIVE_REPRESENTATION_ITEM
						else if (sdaiGetInstanceType(sdaiItemInstance) == sdaiGetEntity(model.p()->getSdaiModel(), "VALUE_REPRESENTATION_ITEM")) 
						{
							SdaiADB sdaiValueComponentADB = nullptr;
							sdaiGetAttrBN(sdaiItemInstance, "value_component", sdaiADB, &sdaiValueComponentADB);

							const char* szTypePath = sdaiGetADBTypePath(sdaiValueComponentADB, 0);
							switch (sdaiGetADBType(sdaiValueComponentADB))
							{
								case sdaiINTEGER:
								{
									SdaiInteger iValue = 0;
									sdaiGetADBValue(sdaiValueComponentADB, sdaiINTEGER, (void*)&iValue);

									strValue.Format(L"%i [%s]", (int)iValue, (LPCTSTR)CA2W(szTypePath));

									pProperty = new CMFCPropertyGridProperty(L"value", (_variant_t)(LPCTSTR)strValue, L"value");
									pProperty->AllowEdit(FALSE);
									pPropertyGroup->AddSubItem(pProperty);
								}
								break;

								case sdaiREAL:
								{
									double dValue = 0;
									sdaiGetADBValue(sdaiValueComponentADB, sdaiREAL, (void*)&dValue);

									strValue.Format(L"%f [%s]", dValue, (LPCTSTR)CA2W(szTypePath));

									pProperty = new CMFCPropertyGridProperty(L"value", (_variant_t)(LPCTSTR)strValue, L"value");
									pProperty->AllowEdit(FALSE);
									pPropertyGroup->AddSubItem(pProperty);
								}
								break;

								case sdaiSTRING:
								{
									char* szValue = nullptr;
									sdaiGetADBValue(sdaiValueComponentADB, sdaiSTRING, (void*)&szValue);

									strValue.Format(L"%s [%s]", (LPCWSTR)CA2W(szValue), (LPCTSTR)CA2W(szTypePath));

									pProperty = new CMFCPropertyGridProperty(L"value", (_variant_t)(LPCTSTR)strValue, L"value");
									pProperty->AllowEdit(FALSE);
									pPropertyGroup->AddSubItem(pProperty);
								}
								break;

								default:
									ASSERT(FALSE);
									break;
							} // switch (sdaiGetADBType(valueComponentADB))
						} // VALUE_REPRESENTATION_ITEM
					} // for (SdaiInteger k = ...
				} // if (sdaiPDRDefinitionInstance == ...
			} // for (SdaiInteger j = ...
		} // if (sdaiDefinitionInstance == ... 
	} // for (SdaiInteger i = ...

	m_wndPropList.AddProperty(pInstanceGroup);
}

void CPropertiesWnd::LoadIFCInstanceProperties()
{
	auto pController = getController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pModel = pController->getModel();
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pIFCModel = dynamic_cast<_ifc_model*>(pModel);
	if (pIFCModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pPropertyProvider = pIFCModel->getPropertyProvider();

	auto pInstance = dynamic_cast<_ifc_instance*>(getController()->getSelectedInstance());
	if (pInstance == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pPropertySetCollection = pPropertyProvider->getPropertySetCollection(pInstance->getSdaiInstance());
	if (pPropertySetCollection == nullptr)
	{
		return;
	}

	auto pInstanceGridGroup = new CMFCPropertyGridProperty(pInstance->getName().c_str());

	for (auto pPropertySet : pPropertySetCollection->propertySets())
	{
		auto pPropertySetGroup = new CMFCPropertyGridProperty(pPropertySet->getName().c_str());

		pInstanceGridGroup->AddSubItem(pPropertySetGroup);

		for (auto pProperty : pPropertySet->properties())
		{
			auto pGridProperty = new CMFCPropertyGridProperty(
				pProperty->getName().c_str(), 
				(_variant_t)pProperty->getValue().c_str(), 
				L""); // Description
			pGridProperty->AllowEdit(FALSE);

			pPropertySetGroup->AddSubItem(pGridProperty);
		}
	}

	m_wndPropList.AddProperty(pInstanceGridGroup);

}

void CPropertiesWnd::LoadCIS2InstanceProperties()
{
	auto pController = getController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pModel = pController->getModel();
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pCIS2Model = dynamic_cast<CCIS2Model*>(pModel);
	if (pCIS2Model == nullptr)
	{
		ASSERT(FALSE);

		return;
	}	

	auto pInstance = dynamic_cast<CCIS2Instance*>(getController()->getSelectedInstance());
	if (pInstance == nullptr)
	{
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
	//			L""); // Description
	//		pGridProperty->AllowEdit(FALSE);

	//		pPropertySetGroup->AddSubItem(pGridProperty);
	//	}
	//}

	//m_wndPropList.AddProperty(pInstanceGridGroup);
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
