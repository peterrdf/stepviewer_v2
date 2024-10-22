
#pragma once

#include "ViewBase.h"
#include "TreeCtrlEx.h"
#include "SearchTreeCtrlDialog.h"

#include <map>
using namespace std;

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class CSTEPRelationsViewToolBar : public CMFCToolBar
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

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class CSTEPRelationsView 
	: public CDockablePane
	, public CViewBase
	, public CSearchTreeCtrlDialogSite
{

private: // Classes

	// -----------------------------------------------------------------------------------------------	
	class CItemData
	{

	private: // Members

		SdaiInstance m_iSdaiInstance;
		SdaiEntity m_iSdaiEntity;

	public: // Methods

		CItemData(SdaiInstance iSdaiInstance, SdaiEntity iSdaiEntity)
			: m_iSdaiInstance(iSdaiInstance)
			, m_iSdaiEntity(iSdaiEntity)
		{
			ASSERT(m_iSdaiInstance != 0);
			ASSERT(m_iSdaiEntity != 0);
		}

		virtual ~CItemData() {}

		SdaiInstance GetInstance() const { return m_iSdaiInstance; }
		SdaiEntity GetEntity() const { return m_iSdaiEntity; }
		const wchar_t* GetEntityName() const { return CEntity::GetName(m_iSdaiEntity); }
	};

	// -----------------------------------------------------------------------------------------------
	class CInstanceData : public CItemData
	{

	public: // Methods

		CInstanceData(SdaiInstance iInstance, SdaiEntity iEntity)
			: CItemData(iInstance, iEntity)
		{}

		virtual ~CInstanceData() {}
	};

	// -----------------------------------------------------------------------------------------------
	class CAttributeData : public CItemData
	{

	private: // Members
		
		string m_srtName;

	public: // Methods

		CAttributeData(SdaiInstance iInstance, SdaiEntity iEntity, const char* szName)
			: CItemData(iInstance, iEntity)
			, m_srtName(szName)
		{}

		virtual ~CAttributeData() {}

		const char* GetName() const { return m_srtName.c_str(); }
	};

	// -----------------------------------------------------------------------------------------------
	class CAttributeSet : public CItemData
	{

	private: // Members

		vector<pair<SdaiAttr, SdaiInteger>> m_vecAttributes;

	public: // Methods

		CAttributeSet(SdaiInstance iInstance, SdaiEntity iEntity)
			: CItemData(iInstance, iEntity)
			, m_vecAttributes()
		{}

		virtual ~CAttributeSet() {}

		vector<pair<SdaiAttr, SdaiInteger>>& Attributes() { return m_vecAttributes; }
	};

	// -----------------------------------------------------------------------------------------------
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
	
	// CViewBase
	virtual void OnModelChanged() override;
	virtual void OnInstanceSelected(CViewBase* pSender) override;
	virtual void OnViewRelations(CViewBase* pSender, SdaiInstance iInstance) override;
	virtual void OnViewRelations(CViewBase* pSender, CEntity* pEntity) override;

	// CSearchTreeCtrlDialogSite
	virtual CTreeCtrlEx* GetTreeView() override;
	virtual vector<CString> GetSearchFilters() override;
	virtual CString GetSearchFilterType(const CString& strFilter) override;
	virtual void LoadChildrenIfNeeded(HTREEITEM hItem) override;
	virtual BOOL ProcessSearch(int iFilter, const CString& strSearchText) override;
	virtual BOOL ContainsText(int iFilter, HTREEITEM hItem, const CString& strText) override;	

private: // Methods

	CModel* GetModel() const;
	
	void LoadInstances(const vector<SdaiInstance>& vecInstances);
	void LoadProperties(SdaiEntity iEntity, const vector<SdaiInstance>& vecInstances);
	void LoadInstance(SdaiEntity iEntity, SdaiInstance iInstance, HTREEITEM hParent);
	int_t GetInstanceAttributes(SdaiEntity iEntity, SdaiInstance iInstance, HTREEITEM hParent, CAttributeSet* pAttributeSet);
	void LoadInstanceAttribute(SdaiEntity iEntity, SdaiInstance iInstance, SdaiAttr sdaiAttribute, const char* szAttributeName, HTREEITEM hParent, HTREEITEM hInsertAfter);
	void AddInstanceAttribute(SdaiEntity iEntity, SdaiInstance iInstance, SdaiAttr iAttribute, const char* szAttributeName, HTREEITEM hParent, HTREEITEM hInsertAfter);

	void CreateAttributeLabelInstance(SdaiInstance iInstance, wstring& strLabel);
	void CreateAttributeLabelBoolean(bool bValue, wstring& strLabel);
	void CreateAttributeLabelLogical(char* szValue, wstring& strLabel);
	void CreateAttributeLabelEnumeration(char* szValue, wstring& strLabel);
	void CreateAttributeLabelReal(double dValue, wstring& strLabel);
	void CreateAttributeLabelInteger(int_t iValue, wstring& strLabel);
	void CreateAttributeLabelString(wchar_t* szValue, wstring& strLabel);
	
	bool CreateAttributeLabel(SdaiInstance iInstance, SdaiAttr iAttribute, wstring& strLabel);

	bool CreateAttributeLabelAggregationElement(SdaiAggr aggregation, int_t iAggrType, SdaiInteger iIndex, wstring& strLabel);
	bool CreateAttributeLabelAggregation(SdaiAggr aggregation, wstring& strLabel);
	bool CreateAttributeLabelADB(SdaiADB ADB, wstring& strLabel);

	void GetAttributeReferencesADB(SdaiADB ADB, HTREEITEM hParent);
	void GetAttributeReferencesAggregationElement(SdaiAggr aggregation, int_t iAggrType, SdaiInteger iIndex, HTREEITEM hParent);
	void GetAttributeReferencesAggregation(SdaiAggr aggregation, HTREEITEM hParent);
	void GetAttributeReferences(SdaiInstance iInstance, SdaiAttr iAttribute, HTREEITEM hParent);

	void GetEntityHierarchy(int_t iEntity, vector<wstring>& vecHierarchy) const;

	void Clean();

	void ResetView();

// Construction
public:
	CSTEPRelationsView();

	void AdjustLayout();
	void OnChangeVisualStyle();

// Attributes
protected:

	CTreeCtrlEx m_treeCtrl;	
	wstring m_strTooltip;
	CImageList m_imageList;
	CSTEPRelationsViewToolBar m_toolBar;

protected:	

// Implementation
public:
	virtual ~CSTEPRelationsView();

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

