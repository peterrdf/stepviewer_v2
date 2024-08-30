
#include "stdafx.h"

#include "PropertiesWnd.h"
#include "Resource.h"
#include "MainFrm.h"
#include "STEPViewer.h"
#include "OpenGLSTEPView.h"
#include "STEPModel.h"
#include "IFCModel.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar

#define TRUE_VALUE_PROPERTY L"Yes"
#define FALSE_VALUE_PROPERTY L"No"
#define ROTATION_MODE_XY L"2D"
#define ROTATION_MODE_XYZ L"3D"

// ------------------------------------------------------------------------------------------------
CApplicationPropertyData::CApplicationPropertyData(enumApplicationProperty enApplicationProperty)
{
	m_enApplicationProperty = enApplicationProperty;
}

// ------------------------------------------------------------------------------------------------
enumApplicationProperty CApplicationPropertyData::GetType() const
{
	return m_enApplicationProperty;
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

	ASSERT(FALSE); // TODO
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CPropertiesWnd::OnInstanceSelected(CViewBase* /*pSender*/)
{
	m_wndObjectCombo.SetCurSel(1 /*Properties*/);

	LoadInstanceProperties();
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CPropertiesWnd::OnApplicationPropertyChanged(CViewBase* pSender, enumApplicationProperty /*enApplicationProperty*/)
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

// ------------------------------------------------------------------------------------------------
/*afx_msg*/ LRESULT CPropertiesWnd::OnPropertyChanged(__in WPARAM /*wparam*/, __in LPARAM lparam)
{
	auto pController = GetController();
	if (pController == nullptr)
	{
		ASSERT(FALSE);

		return 0;
	}

#pragma region Application
	if (m_wndObjectCombo.GetCurSel() == 0)
	{
		auto pRenderer = GetController()->GetView<_oglRenderer>();
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

					GetController()->OnApplicationPropertyChanged(this, enumApplicationProperty::ShowFaces);
				}
				break;

				case enumApplicationProperty::CullFaces:
				{
					pRenderer->setCullFacesMode(strValue);

					GetController()->OnApplicationPropertyChanged(this, enumApplicationProperty::CullFaces);
				}
				break;

				case enumApplicationProperty::ShowConceptualFacesWireframes:
				{
					pRenderer->setShowConceptualFacesPolygons(strValue == TRUE_VALUE_PROPERTY ? TRUE : FALSE);

					GetController()->OnApplicationPropertyChanged(this, enumApplicationProperty::ShowConceptualFacesWireframes);
				}
				break;

				case enumApplicationProperty::ShowLines:
				{
					pRenderer->setShowLines(strValue == TRUE_VALUE_PROPERTY ? TRUE : FALSE);

					GetController()->OnApplicationPropertyChanged(this, enumApplicationProperty::ShowLines);
				}
				break;

				case enumApplicationProperty::ShowPoints:
				{
					pRenderer->setShowPoints(strValue == TRUE_VALUE_PROPERTY ? TRUE : FALSE);

					GetController()->OnApplicationPropertyChanged(this, enumApplicationProperty::ShowPoints);
				}
				break;

				case enumApplicationProperty::RotationMode:
				{
					pRenderer->_setRotationMode(strValue == ROTATION_MODE_XY ? enumRotationMode::XY : enumRotationMode::XYZ);

					GetController()->OnApplicationPropertyChanged(this, enumApplicationProperty::RotationMode);
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

					GetController()->OnApplicationPropertyChanged(this, enumApplicationProperty::PointLightingLocation);
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

					GetController()->OnApplicationPropertyChanged(this, enumApplicationProperty::AmbientLightWeighting);
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

					GetController()->OnApplicationPropertyChanged(this, enumApplicationProperty::DiffuseLightWeighting);
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

					GetController()->OnApplicationPropertyChanged(this, enumApplicationProperty::SpecularLightWeighting);
				}
				break;

				case enumApplicationProperty::MaterialShininess:
				{
					ASSERT(pBlinnPhongProgram != nullptr);

					float fValue = (float)_wtof((LPCTSTR)strValue);

					pBlinnPhongProgram->_setMaterialShininess(fValue);

					GetController()->OnApplicationPropertyChanged(this, enumApplicationProperty::MaterialShininess);
				}
				break;

				case enumApplicationProperty::Contrast:
				{
					ASSERT(pBlinnPhongProgram != nullptr);

					float fValue = (float)_wtof((LPCTSTR)strValue);

					pBlinnPhongProgram->_setContrast(fValue);

					GetController()->OnApplicationPropertyChanged(this, enumApplicationProperty::Contrast);
				}
				break;

				case enumApplicationProperty::Brightness:
				{
					ASSERT(pBlinnPhongProgram != nullptr);

					float fValue = (float)_wtof((LPCTSTR)strValue);

					pBlinnPhongProgram->_setBrightness(fValue);

					GetController()->OnApplicationPropertyChanged(this, enumApplicationProperty::Brightness);
				}
				break;

				case enumApplicationProperty::Gamma:
				{
					ASSERT(pBlinnPhongProgram != nullptr);

					float fValue = (float)_wtof((LPCTSTR)strValue);

					pBlinnPhongProgram->_setGamma(fValue);

					GetController()->OnApplicationPropertyChanged(this, enumApplicationProperty::Gamma);
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
	ON_COMMAND(ID_PROPERTIES1, OnProperties1)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES1, OnUpdateProperties1)
	ON_COMMAND(ID_PROPERTIES2, OnProperties2)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES2, OnUpdateProperties2)
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

	ASSERT(GetController() != nullptr);
	GetController()->RegisterView(this);

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

	auto pRenderer = GetController()->GetView<_oglRenderer>();
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

	if (GetController()->GetSelectedInstance() == nullptr)
	{
		return;
	}

	auto pModel = pContoller->GetModel();
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	switch (pModel->GetType())
	{
		case enumModelType::STEP:
		{
			LoadSTEPInstanceProperties();
		}
		break;

		case enumModelType::IFC:
		{
			LoadIFCInstanceProperties();
		}
		break;

		default:
		{
			ASSERT(FALSE); // Unknown
		}
		break;
	} // switch (pModel ->GetType())
}

// ------------------------------------------------------------------------------------------------
void CPropertiesWnd::LoadSTEPInstanceProperties()
{
	auto pContoller = GetController();
	if (pContoller == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pModel = pContoller->GetModel();
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pSelectedInstance = dynamic_cast<CProductInstance*>(GetController()->GetSelectedInstance());
	if (pSelectedInstance == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	/*
	* Instance
	*/
	auto pInstanceGroup = new CMFCPropertyGridProperty(pSelectedInstance->GetProductDefinition()->GetId());

	/*
	* Properties
	*/
	int_t* propertyDefinitionInstances = sdaiGetEntityExtentBN(pContoller->GetModel()->GetInstance(), "PROPERTY_DEFINITION"),
		noPropertyDefinitionInstances = sdaiGetMemberCount(propertyDefinitionInstances);
	for (int_t i = 0; i < noPropertyDefinitionInstances; i++) {
		int_t propertyDefinitionInstance = 0;
		sdaiGetAggrByIndex(propertyDefinitionInstances, i, sdaiINSTANCE, &propertyDefinitionInstance);

		int_t definitionInstance = 0;
		sdaiGetAttrBN(propertyDefinitionInstance, "definition", sdaiINSTANCE, &definitionInstance);
		if (definitionInstance == pSelectedInstance->GetProductDefinition()->GetInstance()) {
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
				sdaiGetAggrByIndex(propertyDefinitionRepresentationInstances, j, sdaiINSTANCE, &propertyDefinitionRepresentationInstance);

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
						sdaiGetAggrByIndex(aggrItems, k, sdaiINSTANCE, &representationItemInstance);

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

							const char* typePath = sdaiGetADBTypePath(valueComponentADB, 0);
							switch (sdaiGetADBType(valueComponentADB)) {
							case  sdaiINTEGER:
							{
								int_t	value = 0;
								sdaiGetADBValue(valueComponentADB, sdaiINTEGER, (void*)&value);

								strValue.Format(L"%i [%s]", (int)value, (LPCTSTR)CA2W(typePath));

								pProperty = new CMFCPropertyGridProperty(L"value", (_variant_t)(LPCTSTR)strValue, L"value");
								pProperty->AllowEdit(FALSE);
								pPropertyGroup->AddSubItem(pProperty);
							}
							break;
							case  sdaiREAL:
							{
								double	value = 0;
								sdaiGetADBValue(valueComponentADB, sdaiREAL, (void*)&value);

								strValue.Format(L"%f [%s]", value, (LPCTSTR)CA2W(typePath));

								pProperty = new CMFCPropertyGridProperty(L"value", (_variant_t)(LPCTSTR)strValue, L"value");
								pProperty->AllowEdit(FALSE);
								pPropertyGroup->AddSubItem(pProperty);
							}
							break;
							case  sdaiSTRING:
							{
								char* value = nullptr;
								sdaiGetADBValue(valueComponentADB, sdaiSTRING, (void*)&value);

								strValue.Format(L"%s [%s]", (LPCWSTR)CA2W(value), (LPCTSTR)CA2W(typePath));

								pProperty = new CMFCPropertyGridProperty(L"value", (_variant_t)(LPCTSTR)strValue, L"value");
								pProperty->AllowEdit(FALSE);
								pPropertyGroup->AddSubItem(pProperty);
							}
							break;
							default:
								ASSERT(FALSE);
								break;
							}
						}
						else {
							//ASSERT(FALSE);
						}
					}
				}
			}
		}
	}

	m_wndPropList.AddProperty(pInstanceGroup);
}

// ------------------------------------------------------------------------------------------------
void CPropertiesWnd::LoadIFCInstanceProperties()
{
	auto pContoller = GetController();
	if (pContoller == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pModel = pContoller->GetModel();
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pIFCModel = dynamic_cast<CIFCModel*>(pModel);
	if (pIFCModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pPropertyProvider = pIFCModel->GetPropertyProvider();

	auto pInstance = dynamic_cast<CIFCInstance*>(GetController()->GetSelectedInstance());
	if (pInstance == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	auto pPropertySetCollection = pPropertyProvider->GetPropertySetCollection(pInstance->GetInstance());
	if (pPropertySetCollection == nullptr)
	{
		return;
	}

	auto pInstanceGridGroup = new CMFCPropertyGridProperty(pInstance->GetName().c_str());

	for (auto pPropertySet : pPropertySetCollection->PropertySets())
	{
		auto pPropertySetGroup = new CMFCPropertyGridProperty(pPropertySet->GetName().c_str());

		pInstanceGridGroup->AddSubItem(pPropertySetGroup);

		for (auto pProperty : pPropertySet->Properties())
		{
			auto pGridProperty = new CMFCPropertyGridProperty(
				pProperty->GetName().c_str(), 
				(_variant_t)pProperty->GetValue().c_str(), 
				L""); // Description
			pGridProperty->AllowEdit(FALSE);

			pPropertySetGroup->AddSubItem(pGridProperty);
		}
	}

	m_wndPropList.AddProperty(pInstanceGridGroup);

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
	ASSERT(GetController() != nullptr);
	GetController()->UnRegisterView(this);

	__super::OnDestroy();
}
