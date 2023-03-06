
#pragma once

#include "SearchAttributeDialog.h"
#include "STEPView.h"
#include "ViewTree.h"
#include "IFCUnit.h"
#include "IFCInstance.h"

#include <map>
using namespace std;

class CIFCRelationsViewToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

// ------------------------------------------------------------------------------------------------
// Load on demand
class CIFCInstanceData
{
private: // Members

	// --------------------------------------------------------------------------------------------
	// Instance
	int_t m_iInstance;

	// --------------------------------------------------------------------------------------------
	// Entity
	int_t m_iEntity;

	// --------------------------------------------------------------------------------------------
	// Entity
	wstring m_srtEntity;

public: // Methods

	CIFCInstanceData(int_t iInstance, int_t iEntity, const wchar_t* szEntity)
		: m_iInstance(iInstance)
		, m_iEntity(iEntity)
		, m_srtEntity(szEntity)
	{
	}

	// --------------------------------------------------------------------------------------------
	// Getter
	int_t getInstance() const
	{
		return m_iInstance;
	}

	// --------------------------------------------------------------------------------------------
	// Getter
	int_t getEntity() const
	{
		return m_iEntity;
	}

	// --------------------------------------------------------------------------------------------
	// Getter
	const wchar_t* getEntityName() const
	{
		return m_srtEntity.c_str();
	}
};

// ------------------------------------------------------------------------------------------------
// Load on demand
class CIFCAttributeData
{
private: // Members

	// --------------------------------------------------------------------------------------------
	// Instance
	int_t m_iInstance;

	// --------------------------------------------------------------------------------------------
	// Attribute
	string m_srtName;

	// --------------------------------------------------------------------------------------------
	// Attribute
	int_t m_iType;

public: // Methods

	CIFCAttributeData(int_t iInstance, const char * szName, int_t iType)
		: m_iInstance(iInstance)
		, m_srtName(szName)
		, m_iType(iType)
	{
	}

	// --------------------------------------------------------------------------------------------
	// Getter
	int_t getInstance() const
	{
		return m_iInstance;
	}

	// --------------------------------------------------------------------------------------------
	// Getter
	const char * getName() const
	{
		return m_srtName.c_str();
	}

	// --------------------------------------------------------------------------------------------
	// Getter
	int_t getType() const
	{
		return m_iType;
	}
};

/**************************************************************************************************
* Attributes
*/

// ------------------------------------------------------------------------------------------------
class CIFCRelationsView 
	: public CDockablePane
	, public CSTEPView
{

private: // Members
	
	// Cache
	vector<CIFCInstanceData*> m_vecIFCInstancesCache;
	vector<CIFCAttributeData*> m_vecIFCAttributesCache;

	// Search
	CSearchAttributeDialog* m_pSearchDialog;

public: // Methods
	
	// CSTEPView
	virtual void OnModelChanged() override;
	virtual void OnInstanceSelected(CSTEPView* pSender) override;
	virtual void OnViewRelations(CSTEPView* pSender, CInstance* pInstance) override;
	virtual void OnViewRelations(CSTEPView* pSender, CEntity* pEntity) override;
	

private: // Methods

	CIFCModel* GetModel() const;
	
	void LoadInstances(const vector<CIFCInstance*>& vecInstances);
	void LoadProperties(int_t iEntity, const wchar_t* szEntity, const vector<int_t>& vecInstances);
	void LoadInstance(int_t iEntity, const wchar_t* szEntity, int_t iInstance, HTREEITEM hParent);
	int_t LoadInstanceAttributes(int_t iEntity, int_t iInstance, HTREEITEM hParent);
	void AddInstanceAttribute(int_t iInstance, const wchar_t* szAttributeName, int_t iAttributeType, HTREEITEM hParent);
	void CreateAttributeText(bool* pbChildren, int_t iInstance, const char* szAttributeName, int_t iAttributeType, wstring& strText);
	void CreateAttributeTextADB(bool* pbChildren, int_t ADB, wstring& strText);
	void CreateAttributeTextAGGR(bool* pbChildren, int_t* pAggregate, int_t iElementIndex, wstring& strText);
	void GetAttributeReferences(int_t iInstance, const char* szAttributeName, int_t iAttributeType, HTREEITEM hParent);
	void GetAttributeReferencesADB(int_t ADB, HTREEITEM hParent);
	void GetAttributeReferencesAGGR(int_t* pAggregate, int_t iElementIndex, HTREEITEM hParent);
	bool IsAttributeIgnored(int_t iEntity, const wchar_t* szAttributeName) const;

// Construction
public:
	CIFCRelationsView();

	void AdjustLayout();
	void OnChangeVisualStyle();

// Attributes
protected:

	CViewTree m_ifcTreeCtrl;
	CImageList m_imageList;
	CIFCRelationsViewToolBar m_wndToolBar;

protected:	

// Implementation
public:
	virtual ~CIFCRelationsView();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnProperties();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnNMClickTreeIFC(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickTreeIFC(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnItemexpandingTreeIFC(NMHDR *pNMHDR, LRESULT *pResult);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};

