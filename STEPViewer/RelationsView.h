
#pragma once

#include "TreeCtrlEx.h"
#include "SearchTreeCtrlDialog.h"
#include "Controller.h"

#include <map>
using namespace std;

// ************************************************************************************************
class CRelationsViewToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

/**************************************************************************************************
* Attributes
*/

/*
*** Nested ***
Instance
	Entity 1
		Attribute 1
		Attribute 2
		Attribute N
	Entity 2
		...
	Entity N
		...

*** Flat ***
Instance
Attribute 1		
Attribute 2
	=> ...
Attribute N
	=> ...
*/
enum class enumRelationsViewMode : int
{
	Hierarchy = 0,
	Flat,
};

// ************************************************************************************************
class CRelationsView 
	: public CDockablePane
	, public _view
	, public CSearchTreeCtrlDialogSite
{

private: // Classes

	class CItemData
	{

	private: // Members

		SdaiInstance m_sdaiInstance;
		SdaiEntity m_sdaiEntity;

	public: // Methods

		CItemData(SdaiInstance sdaiInstance, SdaiEntity sdaiEntity)
			: m_sdaiInstance(sdaiInstance)
			, m_sdaiEntity(sdaiEntity)
		{
			ASSERT(m_sdaiInstance != 0);
			ASSERT(m_sdaiEntity != 0);
		}

		virtual ~CItemData() {}

		SdaiInstance GetInstance() const { return m_sdaiInstance; }
		SdaiEntity GetEntity() const { return m_sdaiEntity; }
		const wchar_t* GetEntityName() const { return _entity::getName(m_sdaiEntity); }
	};

	class CInstanceData : public CItemData
	{

	public: // Methods

		CInstanceData(SdaiInstance sdaiInstance, SdaiEntity sdaiEntity)
			: CItemData(sdaiInstance, sdaiEntity)
		{}

		virtual ~CInstanceData() {}
	};

	class CAttributeData : public CItemData
	{

	private: // Members
		
		string m_srtName;

	public: // Methods

		CAttributeData(SdaiInstance sdaiInstance, SdaiEntity sdaiEntity, const char* szName)
			: CItemData(sdaiInstance, sdaiEntity)
			, m_srtName(szName)
		{}

		virtual ~CAttributeData() {}

		const char* GetName() const { return m_srtName.c_str(); }
	};

	class CAttributeSet : public CItemData
	{

	private: // Members

		vector<pair<SdaiAttr, SdaiInteger>> m_vecAttributes;

	public: // Methods

		CAttributeSet(SdaiInstance sdaiInstance, SdaiEntity sdaiEntity)
			: CItemData(sdaiInstance, sdaiEntity)
			, m_vecAttributes()
		{}

		virtual ~CAttributeSet() {}

		vector<pair<SdaiAttr, SdaiInteger>>& Attributes() { return m_vecAttributes; }
	};

	enum class enumSearchFilter : int {
		All = 0,
		ExpressID,
	};

private: // Members

	// View
	enumRelationsViewMode m_enMode;
	
	// Cache
	vector<CItemData*> m_vecItemDataCache;

	// Search
	CSearchTreeCtrlDialog* m_pSearchDialog;

public: // Methods
	
	// _view
	virtual void onModelLoaded() override;
	virtual void onInstanceSelected(_view* pSender) override;
	virtual void onViewRelations(_view* pSender, SdaiInstance sdaiInstance) override;
	virtual void onViewRelations(_view* pSender, _entity* pEntity) override;

	// CSearchTreeCtrlDialogSite
	virtual CTreeCtrlEx* GetTreeView() override;
	virtual vector<CString> GetSearchFilters() override;
	virtual CString GetSearchFilterType(const CString& strFilter) override;
	virtual void LoadChildrenIfNeeded(HTREEITEM hItem) override;
	virtual BOOL ProcessSearch(int iFilter, const CString& strSearchText) override;
	virtual BOOL ContainsText(int iFilter, HTREEITEM hItem, const CString& strText) override;	

private: // Methods

	_ap_model* GetModelByInstance(SdaiModel sdaiModel);
	
	void LoadInstances(_ap_model* pModel, const vector<SdaiInstance>& vecInstances);
	void LoadProperties(SdaiEntity sdaiEntity, const vector<SdaiInstance>& vecSdaiInstances);
	void LoadInstance(SdaiEntity sdaiEntity, SdaiInstance sdaiInstance, HTREEITEM hParent);
	SdaiInteger GetInstanceAttributes(SdaiEntity sdaiEntity, SdaiInstance sdaiInstance, HTREEITEM hParent, CAttributeSet* pAttributeSet);
	void LoadInstanceAttribute(SdaiEntity sdaiEntity, SdaiInstance sdaiInstance, SdaiAttr sdaiAttribute, const char* szAttributeName, HTREEITEM hParent, HTREEITEM hInsertAfter);
	void AddInstanceAttribute(SdaiEntity sdaiEntity, SdaiInstance sdaiInstance, SdaiAttr sdaiAttribute, const char* szAttributeName, HTREEITEM hParent, HTREEITEM hInsertAfter);

	void CreateAttributeLabelInstance(SdaiInstance sdaiInstance, wstring& strLabel);
	void CreateAttributeLabelBoolean(bool bValue, wstring& strLabel);
	void CreateAttributeLabelLogical(char* szValue, wstring& strLabel);
	void CreateAttributeLabelEnumeration(char* szValue, wstring& strLabel);
	void CreateAttributeLabelReal(double dValue, wstring& strLabel);
	void CreateAttributeLabelInteger(SdaiInteger iValue, wstring& strLabel);
	void CreateAttributeLabelString(wchar_t* szValue, wstring& strLabel);
	
	bool CreateAttributeLabel(SdaiInstance sdaiInstance, SdaiAttr sdaiAttribute, wstring& strLabel);

	bool CreateAttributeLabelAggregationElement(SdaiAggr aggregation, SdaiPrimitiveType sdaiPrimitiveType, SdaiInteger iIndex, wstring& strLabel);
	bool CreateAttributeLabelAggregation(SdaiAggr aggregation, wstring& strLabel);
	bool CreateAttributeLabelADB(SdaiADB ADB, wstring& strLabel);

	void GetAttributeReferencesADB(SdaiADB ADB, HTREEITEM hParent);
	void GetAttributeReferencesAggregationElement(SdaiAggr aggregation, SdaiPrimitiveType sdaiPrimitiveType, SdaiInteger iIndex, HTREEITEM hParent);
	void GetAttributeReferencesAggregation(SdaiAggr aggregation, HTREEITEM hParent);
	void GetAttributeReferences(SdaiInstance sdaiInstance, SdaiAttr sdaiAttribute, HTREEITEM hParent);

	void GetEntityHierarchy(SdaiEntity sdaiEntity, vector<wstring>& vecHierarchy) const;

	void Clean();

	void ResetView();

// Construction
public:
	CRelationsView();

	void AdjustLayout();
	void OnChangeVisualStyle();

// Attributes
protected:

	CTreeCtrlEx m_treeCtrl;	
	wstring m_strTooltip;
	CImageList m_imageList;
	CRelationsViewToolBar m_toolBar;

protected:	

// Implementation
public:
	virtual ~CRelationsView();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnProperties();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnNMClickTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTVNItemexpandingTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTVNGetInfoTip(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};

