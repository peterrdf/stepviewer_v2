
#pragma once

#include "STEPView.h"

#include <map>
#include <string>

using namespace std;

class CPropertiesToolBar : public CMFCToolBar
{
public:
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

// ------------------------------------------------------------------------------------------------
enum class enumPropertyType
{
	ShowFaces,
	CullFaces,
	ShowFacesWireframes,
	ShowConceptualFacesWireframes,
	ShowLines,
	LineWidth,
	ShowPoints,
	PointSize,
	ShowNormalVectors,
	ShowTangenVectors,
	ShowBiNormalVectors,
	ScaleVectors,
	ShowBoundingBoxes,
	PointLightingLocation,
	AmbientLightWeighting,
	SpecularLightWeighting,
	DiffuseLightWeighting,
	MaterialShininess,
	Contrast,
	Brightness,
	Gamma,
	VisibleValuesCountLimit,
	ScalelAndCenter,
};

// ------------------------------------------------------------------------------------------------
class CApplicationPropertyData
{

private:  // Members

	// --------------------------------------------------------------------------------------------
	// Type
	enumPropertyType m_enPropertyType;

public: // Methods

	// --------------------------------------------------------------------------------------------
	// ctor
	CApplicationPropertyData(enumPropertyType enPropertyType);

	// --------------------------------------------------------------------------------------------
	// Getter
	enumPropertyType GetType() const;
};

// ------------------------------------------------------------------------------------------------
class CApplicationProperty : public CMFCPropertyGridProperty
{

public: // Methods

	// --------------------------------------------------------------------------------------------
	// ctor
	CApplicationProperty(const CString& strName, const COleVariant& vtValue, LPCTSTR szDescription, DWORD_PTR dwData);

	// --------------------------------------------------------------------------------------------
	// ctor
	CApplicationProperty(const CString& strGroupName, DWORD_PTR dwData, BOOL bIsValueList);

	// --------------------------------------------------------------------------------------------
	// dtor
	virtual ~CApplicationProperty();
};

class CPropertiesWnd
	: public CDockablePane
	, public CSTEPView
{

public: // Methods

	// --------------------------------------------------------------------------------------------
	// CSTEPView
	virtual void OnModelChanged();
	virtual void OnShowMetaInformation();
	virtual void OnInstanceSelected(CSTEPView* pSender);
	virtual void OnInstancePropertySelected();

protected: // Methods

	// --------------------------------------------------------------------------------------------
	// Support for properties
	afx_msg LRESULT OnPropertyChanged(__in WPARAM wparam, __in LPARAM lparam);

private: // Methods

// Construction
public:
	CPropertiesWnd();

	void AdjustLayout();

// Attributes
public:
	void SetVSDotNetLook(BOOL bSet)
	{
		m_wndPropList.SetVSDotNetLook(bSet);
		m_wndPropList.SetGroupNameFullWidth(bSet);
	}

protected:
	CFont m_fntPropList;
	CComboBox m_wndObjectCombo;
	CPropertiesToolBar m_wndToolBar;
	CMFCPropertyGridCtrl m_wndPropList;

// Implementation
public:
	virtual ~CPropertiesWnd();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnExpandAllProperties();
	afx_msg void OnUpdateExpandAllProperties(CCmdUI* pCmdUI);
	afx_msg void OnSortProperties();
	afx_msg void OnUpdateSortProperties(CCmdUI* pCmdUI);
	afx_msg void OnProperties1();
	afx_msg void OnUpdateProperties1(CCmdUI* pCmdUI);
	afx_msg void OnProperties2();
	afx_msg void OnUpdateProperties2(CCmdUI* pCmdUI);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnViewModeChanged();
	afx_msg LRESULT OnLoadInstancePropertyValues(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLoadInstanceProperties(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

	void LoadApplicationProperties();
	void LoadInstanceProperties();
	void LoadMetaInformation();
	void SetPropListFont();

	int m_nComboHeight;
public:
	afx_msg void OnDestroy();
};

