#pragma once

#include "_mvc.h"

#include <map>
#include <string>
using namespace std;

// ************************************************************************************************
class CPropertiesToolBar : public CMFCToolBar
{

public:

	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

// ************************************************************************************************
class CApplicationPropertyData
{

private:  // Members

	enumApplicationProperty m_enApplicationProperty;

public: // Methods

	CApplicationPropertyData(enumApplicationProperty enApplicationProperty);

public: // Properties

	enumApplicationProperty GetType() const;
};

// ************************************************************************************************
class CApplicationProperty : public CMFCPropertyGridProperty
{

public: // Methods

	CApplicationProperty(const CString& strName, const COleVariant& vtValue, LPCTSTR szDescription, DWORD_PTR dwData);
	CApplicationProperty(const CString& strGroupName, DWORD_PTR dwData, BOOL bIsValueList);
	virtual ~CApplicationProperty();
};

// ************************************************************************************************
class CPropertiesWnd
	: public CDockablePane
	, public _view
{

public: // Methods

	// _view
	virtual void onModelLoaded();
	virtual void onShowMetaInformation();
	virtual void onInstanceSelected(_view* pSender);
	virtual void onApplicationPropertyChanged(_view* pSender, enumApplicationProperty enApplicationProperty) override;

private: // Methods

	_ap_model* GetModelByInstance(SdaiModel sdaiModel);

protected: // Methods

	// Support for properties
	afx_msg LRESULT OnPropertyChanged(__in WPARAM wparam, __in LPARAM lparam);

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
	CPropertiesToolBar m_toolBar;
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
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnViewModeChanged();

	DECLARE_MESSAGE_MAP()

	void LoadApplicationProperties();
	void LoadInstanceProperties();
	void LoadSTEPInstanceProperties();
	void LoadIFCInstanceProperties(_ap_model* pModel, _ap_instance* pInstance);
	void LoadCIS2InstanceProperties();
	void SetPropListFont();

	int m_nComboHeight;
public:
	afx_msg void OnDestroy();
};

