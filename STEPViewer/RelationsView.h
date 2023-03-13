
#pragma once

#include "SearchAttributeDialog.h"
#include "STEPView.h"
#include "ViewTree.h"

#include <map>
using namespace std;

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
enum class enumRelationsViewMode
{
	Hierarchy = 0,
	Flat,
};

// ------------------------------------------------------------------------------------------------
class CRelationsView 
	: public CDockablePane
	, public CSTEPView
{

private: // Classes

	// -----------------------------------------------------------------------------------------------	
	class CItemData
	{

	private: // Members

		int_t m_iInstance;
		int_t m_iEntity;

	public: // Methods

		CItemData(int_t iInstance, int_t iEntity)
			: m_iInstance(iInstance)
			, m_iEntity(iEntity)
		{
			ASSERT(m_iInstance != 0);
			ASSERT(m_iEntity != 0);
		}

		virtual ~CItemData() {}

		int_t getInstance() const { return m_iInstance; }
		int_t getEntity() const { return m_iEntity; }
		const wchar_t* getEntityName() const { return CEntity::GetName(m_iEntity); }
	};

	// -----------------------------------------------------------------------------------------------
	class CInstanceData : public CItemData
	{

	private: // Members

		int_t m_iInstance;
		int_t m_iEntity;

	public: // Methods

		CInstanceData(int_t iInstance, int_t iEntity)
			: CItemData(iInstance, iEntity)
		{}

		virtual ~CInstanceData() {}
	};

	// -----------------------------------------------------------------------------------------------
	class CAttributeData : public CItemData
	{

	private: // Members
		
		string m_srtName;
		int_t m_iType;

	public: // Methods

		CAttributeData(int_t iInstance, int_t iEntity, const char* szName, int_t iType)
			: CItemData(iInstance, iEntity)
			, m_srtName(szName)
			, m_iType(iType)
		{}

		virtual ~CAttributeData() {}

		const char* getName() const { return m_srtName.c_str(); }
		int_t getType() const { return m_iType; }
	};

private: // Members

	// View
	enumRelationsViewMode m_enMode;
	
	// Cache
	vector<CItemData*> m_vecItemDataCache;

	// Search
	CSearchAttributeDialog* m_pSearchDialog;

public: // Methods
	
	// CSTEPView
	virtual void OnModelChanged() override;
	virtual void OnInstanceSelected(CSTEPView* pSender) override;
	virtual void OnViewRelations(CSTEPView* pSender, int64_t iInstance) override;
	virtual void OnViewRelations(CSTEPView* pSender, CEntity* pEntity) override;
	

private: // Methods

	CModel* GetModel() const;
	
	void LoadInstances(const vector<int_t>& vecInstances);
	void LoadProperties(int_t iEntity, const wchar_t* szEntity, const vector<int_t>& vecInstances);
	void LoadInstance(int_t iEntity, const wchar_t* szEntity, int_t iInstance, HTREEITEM hParent);

	int_t LoadInstanceAttributes(int_t iEntity, int_t iInstance, HTREEITEM hParent);
	
	void LoadADB(SdaiADB ADB, int_t iEntity, const char* szAttributeName, int_t iAttributeType, HTREEITEM hParent);
	void LoadAGGR(int_t iInstance, int_t iEntity, const char* szAttributeName, int_t iAttributeType, HTREEITEM hParent);
	void AddInstanceAttribute(int_t iInstance, int_t iEntity, const char* szAttributeName, int_t iAttributeType, HTREEITEM hParent);
	bool CreateAttributeLabel(int_t iInstance, const char* szAttributeName, int_t iAttributeType, wstring& strLabel);

	void CreateAttributeLabelADB(int_t ADB, wstring& strText, bool& bHasChildren);
	void CreateAttributeLabelAGGR(int_t* pAggregate, int_t iElementIndex, wstring& strLabel, bool& bHasChildren);

	void GetAttributeReferences(int_t iInstance, const char* szAttributeName, int_t iAttributeType, HTREEITEM hParent);
	void GetAttributeReferencesADB(int_t ADB, HTREEITEM hParent);
	void GetAttributeReferencesAGGR(int_t* pAggregate, int_t iElementIndex, HTREEITEM hParent);

	void GetEntityHierarchy(int_t iEntity, vector<wstring>& vecHierarchy) const;

	void Clean();

// Construction
public:
	CRelationsView();

	void AdjustLayout();
	void OnChangeVisualStyle();

// Attributes
protected:

	CViewTree m_treeCtrl;	
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

