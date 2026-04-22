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
class CColorSelectorProperty : public CMFCPropertyGridColorProperty
{

public: // Methods

	CColorSelectorProperty(const CString& strName, const COLORREF& color, CPalette* pPalette, LPCTSTR szDescription, DWORD_PTR dwData);
	virtual ~CColorSelectorProperty();

	COLORREF GetSelectedColor() const;
	COLORREF GetAutomaticColor() const;	
};

// ************************************************************************************************
class CInstanceAttributeProperty : public CMFCPropertyGridProperty
{

public: // Methods

	CInstanceAttributeProperty(const CString& strName, const COleVariant& vtValue, LPCTSTR szDescription, DWORD_PTR dwData);
	virtual ~CInstanceAttributeProperty();

	virtual CString FormatProperty();
	virtual BOOL TextToVar(const CString& strText);
	virtual CWnd* CreateInPlaceEdit(CRect rectEdit, BOOL& bDefaultFormat);
	void EnableSpinControlInt64();
};

// ************************************************************************************************
struct ExploringInstance
{
	SdaiInstance inst = NULL;
	SdaiAttr	 attrToNext = NULL;
};

typedef	std::list<ExploringInstance> ExploringStack;

struct GridPropertyInfo
{
	CMFCPropertyGridProperty* pProp = NULL;
	CStringA				  typeName;
};

// ************************************************************************************************
struct CValueLocator
{
	SdaiInstance	sdaiInst = NULL;
	SdaiAttr		sdaiAttr = NULL;
	std::vector<SdaiInteger>  indexes;
};

struct CPropertyGridData
{
	CValueLocator valueLocator;
};

// ************************************************************************************************
class CPropertiesWnd
	: public CDockablePane
	, public _ap_view
{

public: // Methods

	// _view
	virtual void postModelLoaded() override;
	virtual void onInstanceSelected(_view* pSender) override;
	virtual void onApplicationPropertyChanged(_view* pSender, enumApplicationProperty enApplicationProperty) override;

private: // Methods

	_ap_model* GetModelByInstance(SdaiModel sdaiModel);

protected: // Methods

	// Support for properties
	afx_msg LRESULT OnPropertyChanged(__in WPARAM wparam, __in LPARAM lparam);
	void OnSelectionMaterialPropertyChanged(CMFCPropertyGridProperty* pProp);
	void OnHighlightMaterialPropertyChanged(CMFCPropertyGridProperty* pProp);

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

	BOOL m_calculateDerivedAttributes;
	ExploringStack m_exploringStack;

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
	
	void SetInstancesToExplore();
	void LoadInstanceAttributes();

	void AddInstanceNode(CMFCPropertyGridProperty*& pRootNode, CMFCPropertyGridProperty*& pInstanceGroup);
	void AddComplexInstanceProperties(CMFCPropertyGridProperty* pPropGroup, const CValueLocator& locator);
	void AddInstanceProperties(CMFCPropertyGridProperty* pPropGroup, CValueLocator locator, SdaiEntity partOfComplex);

	CMFCPropertyGridProperty* CreateAttributeProperty(const CValueLocator& locator);
	GridPropertyInfo CreateADBGridProperty(SdaiADB adb, const CValueLocator& locator, LPCSTR details);
	GridPropertyInfo CreateAggrGridProperty(SdaiAggr aggr, const CValueLocator& locator, LPCSTR details);
	GridPropertyInfo CreateAggrItemGridProperty(SdaiAggr aggr, SdaiInteger index, const CValueLocator& itemLocator, LPCSTR details);
	CMFCPropertyGridProperty* CreateValueGridProperty(LPCSTR value, const CValueLocator& locator, LPCSTR details);
	CMFCPropertyGridProperty* CreateInstanceGridProperty(SdaiInstance inst, const CValueLocator& locator, LPCSTR details);

	CValueLocator GetSelectedValueLocator() const;
	CValueLocator GetValueLocator(CMFCPropertyGridProperty* pProp) const;
	bool SelectValue(const CValueLocator& locator);
	bool SelectValue(CMFCPropertyGridProperty* scope, const CValueLocator& locator);

	SdaiInstance GetSelectedValueInstance();

	void SetPropListFont();

	int m_nComboHeight;
public:
	afx_msg void OnDestroy();
};

