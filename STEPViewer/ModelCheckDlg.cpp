// ModelCheckDlg.cpp : implementation file
//

#include "stdafx.h"

#include "_ptr.h"

#include "resource.h"

#include "MainFrm.h"
#include "STEPViewerDoc.h"

#include "ModelCheckDlg.h"

struct IssueData
{
	int_t stepId = -1;
	std::set<int_t> relatingInstances;
	bool relatingInstancesCollected = false;
};

///
struct ReferencingInstancesCollector
{
	/// <summary>
	/// 
	/// </summary>
	ReferencingInstancesCollector(std::set<int_t>& referencingInstances, int_t* searchEntities)
		: m_referencingInstances(referencingInstances)
		, m_searchEntities(searchEntities)
	{}

	/// <summary>
	/// 
	/// </summary>
	void CollectReferencingTo(SdaiInstance referencedInstance)
	{
		//if this instance of desired class - get it and stop
		if (referencedInstance) {
			for (int i = 0; m_searchEntities[i]; i++) {
				if (sdaiIsKindOf(referencedInstance, m_searchEntities[i])) {
					m_referencingInstances.insert(referencedInstance);
					return; //>>>>>>>>>>>>>>>>>>>>
				}
			}
		}

		//now check all instances referencing thius
		m_referenceStack.push_back(referencedInstance);

		auto entity = sdaiGetInstanceType(referencedInstance);
		auto model = engiGetEntityModel(entity);
		auto allInst = xxxxGetAllInstances(model);
		auto NInst = sdaiGetMemberCount(allInst);
		for (int i = 0; i < NInst; i++) {
			int_t instance = 0;
			sdaiGetAggrByIndex(allInst, i, sdaiINSTANCE, &instance);
			if (instance) {
				VisitInstance(instance);
			}
		}

		m_referenceStack.pop_back();
	}

	/// <summary>
	/// 
	/// </summary>
private:
	bool VisitInstance(SdaiInstance thisInstance)
	{
		//if this instance on call stack - do not process it
		for (auto inst : m_referenceStack) {
			if (inst == thisInstance) {
				return true;
			}
		}

		//check if this item referencing top one
		auto referencedInstance = m_referenceStack.back();

		bool referencing = false;
		auto thisEntity = sdaiGetInstanceType(thisInstance);
		auto NArg = engiGetEntityNoAttributesEx(thisEntity, true, false);
		for (int_t i = 0; i < NArg && !referencing; i++) {
			SdaiAttr attr = engiGetEntityAttributeByIndex(thisEntity, i, true, false);
			referencing = DoArgumentsReferenceTo(thisInstance, attr, referencedInstance);
		}

		//collect if referencing
		if (referencing) {
			CollectReferencingTo(thisInstance);
		}

		return true;
	}

	/// <summary>
	/// 
	/// </summary>
private:
	bool DoArgumentsReferenceTo(SdaiInstance thisInstance, SdaiAttr attr, SdaiInstance referencedInstance)
	{
		auto type = engiGetInstanceAttrType(thisInstance, attr);
		switch (type) {
			case sdaiINSTANCE:
			{
				SdaiInstance inst = 0;
				sdaiGetAttr(thisInstance, attr, sdaiINSTANCE, &inst);
				return inst == referencedInstance;
			}
			case sdaiAGGR:
			{
				bool refers = false;
				SdaiAggr aggr = 0;
				sdaiGetAttr(thisInstance, attr, sdaiAGGR, &aggr);
				auto cnt = sdaiGetMemberCount(aggr);
				for (int_t i = 0; i < cnt && !refers; i++) {
					SdaiInstance inst = 0;
					sdaiGetAggrByIndex(aggr, i, sdaiINSTANCE, &inst);
					refers = (inst == referencedInstance);
				}
				return refers;
			}
			default:
				return false;
		}
	}

private:
	std::set<int_t>& m_referencingInstances;
	SdaiEntity* m_searchEntities;
	std::list<SdaiInstance> m_referenceStack;
};


extern void CollectReferencingInstancesRecursive(std::set<SdaiInstance>& referencingInstances, SdaiInstance referencedInstance, SdaiEntity* searchEntities /*NULL-terminated array*/)
{
	ReferencingInstancesCollector collector(referencingInstances, searchEntities);
	collector.CollectReferencingTo(referencedInstance);
}

static const char* GetEntityName(ValidationIssue issue)
{
	auto ent = validateGetEntity(issue);
	if (ent) {
		const char* name = 0;
		engiGetEntityName(ent, sdaiSTRING, &name);
		return name;
	}
	else {
		return NULL;
	}
}

static const char* GetAttrName(ValidationIssue issue)
{
	auto attr = validateGetAttr(issue);
	if (attr) {
		const char* name = 0;
		//error LNK2019: unresolved external symbol __imp__engiGetAttributeTraits@32 referenced in function "char const * __cdecl GetAttrName(void *)" (?GetAttrName@@YAPBDPAX@Z)
		//engiGetAttributeTraits(attr, &name, 0, 0, 0, 0, 0, 0);
		return name;
	}
	else {
		return NULL;
	}
}

static int_t GetAttrIndex(ValidationIssue issue)
{
	auto ent = validateGetEntity(issue);
	const char* name = GetAttrName(issue);
	if (ent && name) {
		CString str(name);
		return engiGetEntityAttributeIndexEx(ent, (char*)str.GetBuffer(), true, false);
	}
	else {
		return -1;
	}
}

static int64_t GetStepId(ValidationIssue issue)
{
	auto inst = validateGetInstance(issue);
	if (inst) {
		return internalGetP21Line(inst);
	}
	else {
		return -1;
	}
}

static const char* GetIssueId(ValidationIssue issue)
{
	auto type = validateGetIssueType(issue);
	switch (type) {
		case enum_validation_type::__NO_OF_ARGUMENTS: return "number of arguments";
		case enum_validation_type::__ARGUMENT_EXPRESS_TYPE: return "argument value is correct entity, defined type or enumeration value";
		case enum_validation_type::__ARGUMENT_PRIM_TYPE: return "argument value has correct primitive type";
		case enum_validation_type::__REQUIRED_ARGUMENTS: return "non-optional arguments values are provided";
		case enum_validation_type::__ARRGEGATION_EXPECTED: return "aggregation is provided when expected";
		case enum_validation_type::__AGGREGATION_NOT_EXPECTED: return "aggregation is not used when not expected";
		case enum_validation_type::__AGGREGATION_SIZE: return "aggregation size";
		case enum_validation_type::__AGGREGATION_UNIQUE: return "elements in aggregations are unique when required";
		case enum_validation_type::__COMPLEX_INSTANCE: return "complex instances contains full parent chains";
		case enum_validation_type::__REFERENCE_EXISTS: return "referenced instance exists";
		case enum_validation_type::__ABSTRACT_ENTITY: return "abstract entity should not instantiate";
		case enum_validation_type::__WHERE_RULE: return "where-rule check";
		case enum_validation_type::__UNIQUE_RULE: return "unique-rule check";
		case enum_validation_type::__STAR_USAGE: return "* is used only for derived arguments";
		case enum_validation_type::__CALL_ARGUMENT: return "validateModel/validateInstance function argument should be model/instance";
		case enum_validation_type::__INTERNAL_ERROR: return "unspecified error";
		default:
			assert(0);
			return "Unknown";
	}
}


// CModelCheckDlg dialog

IMPLEMENT_DYNAMIC(CModelCheckDlg, CDialog)

/// <summary>
/// 
/// </summary>
CModelCheckDlg::CModelCheckDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_MODELCHECK, pParent)
	, m_nSortColumn(-1)
	, m_bSortAscending(false)
	, m_model (NULL)
{

}

/// <summary>
/// 
/// </summary>
CModelCheckDlg::~CModelCheckDlg()
{
}


/// <summary>
/// 
/// </summary>
bool CModelCheckDlg::IsVisible()
{
	return IsWindow (GetSafeHwnd()) && IsWindowVisible();
}

/// <summary>
/// 
/// </summary>
void CModelCheckDlg::Show()
{
	BeginWaitCursor();
	if (!IsWindow(GetSafeHwnd())) {
		Create(IDD_MODELCHECK);
	}
	ShowWindow(SW_SHOW);
	FillIssueList(false);
	EndWaitCursor();
}

/// <summary>
/// 
/// </summary>
void CModelCheckDlg::Hide(bool deleteContent)
{
	if (IsWindow(GetSafeHwnd())) {
		ShowWindow(SW_HIDE);
		if (deleteContent) {
			m_wndIssueList.DeleteAllItems();
		}
	}

	if (deleteContent) {
		m_model = NULL;
	}
}


/// <summary>
/// 
/// </summary>
void CModelCheckDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ISSUELIST, m_wndIssueList);
	DDX_Control(pDX, IDC_VIEW_ALL_ISSUES, m_btnViewAll);
}


BEGIN_MESSAGE_MAP(CModelCheckDlg, CDialog)
	ON_NOTIFY(LVN_DELETEITEM, IDC_ISSUELIST, &CModelCheckDlg::OnDeleteitemIssuelist)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_ISSUELIST, &CModelCheckDlg::OnColumnclickIssuelist)
	ON_NOTIFY(NM_DBLCLK, IDC_ISSUELIST, &CModelCheckDlg::OnDblclkIssuelist)
	ON_NOTIFY(NM_CLICK, IDC_ISSUELIST, &CModelCheckDlg::OnClickIssuelist)
	ON_BN_CLICKED(IDC_VIEW_ALL_ISSUES, &CModelCheckDlg::OnClickedViewAllIssues)
END_MESSAGE_MAP()


// CModelCheckDlg message handlers


/// <summary>
/// 
/// </summary>
BOOL CModelCheckDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	FormatIssueList();
	FillIssueList(false);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


/// <summary>
/// 
/// </summary>
void CModelCheckDlg::FormatIssueList()
{
	const wchar_t* rTitle[] = {L"Description", L"#", L"Entity", L"Attribute"};
	
	for (int i = 0; i < _countof(rTitle); i++) {
		m_wndIssueList.InsertColumn(i, rTitle[i]);
		
		auto w = m_wndIssueList.GetStringWidth(rTitle[i]);
		m_wndIssueList.SetColumnWidth(i, w + 10 * GetSystemMetrics(SM_CXBORDER));
	}
}

/// <summary>
/// 
/// </summary>
void CModelCheckDlg::FillIssueList(bool all)
{
	if (!all && m_model == GetActiveSdaiModel()) {
		return; //do not reload existing model
	}

	m_model = GetActiveSdaiModel();

	m_wndIssueList.DeleteAllItems();
	m_btnViewAll.EnableWindow(!all);

	if (GetActiveSdaiModel()) {

		if (all) {
			validateSetOptions(-1, -1, false, 0, 0);
		}
		else {
			validateSetOptions(5, 100, true, 0, 0);
		}

		auto checks = validateModel(GetActiveSdaiModel());

		int rWidth[4] = {0,0,0,0};
	
		for (auto issue = validateGetFirstIssue(checks); issue; issue = validateGetNextIssue (issue))
		{
			AddIssue(issue, rWidth);
		}

		validateFreeResults(checks);

		for (int i = 0; i < 4; i++) {
			m_wndIssueList.SetColumnWidth(i, rWidth[i] + 20 * GetSystemMetrics(SM_CXBORDER));
		}
	}
}

void CModelCheckDlg::AddIssue(ValidationIssue issue, int rWidth[4])
{
	CString text (validateGetDescription(issue));
	auto item = m_wndIssueList.InsertItem(0, text);
	rWidth[0] = max(rWidth[0], m_wndIssueList.GetStringWidth(text));

	CString id;
	id.Format(L"%I64d", GetStepId (issue));
	m_wndIssueList.SetItemText(item, 1, id);
	rWidth[1] = max(rWidth[1], m_wndIssueList.GetStringWidth(id));

	CString entity(GetEntityName (issue));
	m_wndIssueList.SetItemText(item, 2, entity);
	rWidth[2] = max(rWidth[2], m_wndIssueList.GetStringWidth(entity));

	CString aggrIndex;
	for (int i = 0; i < validateGetAggrLevel (issue); i++) {
		if (aggrIndex.IsEmpty()) {
			aggrIndex.Append(L"[");
		}
		else {
			aggrIndex.Append(L",");
		}

		CString ind;
		ind.Format(L"%I64d", validateGetAggrIndArray (issue)[i]);
		aggrIndex.Append(ind);
	}
	if (!aggrIndex.IsEmpty()) {
		aggrIndex.Append(L"]");
	}

	CString attr(GetAttrName (issue));
	if (!aggrIndex.IsEmpty())
		attr += aggrIndex;
	m_wndIssueList.SetItemText(item, 3, attr);
	rWidth[3] = max(rWidth[3], m_wndIssueList.GetStringWidth(attr));

	auto data = new IssueData();
	data->stepId = (int_t)GetStepId (issue);
	m_wndIssueList.SetItemData(item, (DWORD_PTR)data);
}

void CModelCheckDlg::OnDeleteitemIssuelist(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	auto data = m_wndIssueList.GetItemData (pNMLV->iItem);
	if (data) {
		auto p = (IssueData*) data;
		delete p;
	}
	*pResult = 0;
}

int CALLBACK CModelCheckDlg::SortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamMe)
{
	CModelCheckDlg& me = *(CModelCheckDlg*) lParamMe;

	CString s1 = me.m_wndIssueList.GetItemText((int)lParam1, me.m_nSortColumn);
	CString s2 = me.m_wndIssueList.GetItemText((int)lParam2, me.m_nSortColumn);

	if (me.m_nSortColumn == 1) {
		int a1 = _wtoi(s1);
		int a2 = _wtoi(s2);

		if (a1 > a2)
			return me.m_bSortAscending ? 1 : -1;
		else if (a2 > a1)
			return me.m_bSortAscending ? -1 : 1;
		else
			return 0;
	}
	else {
		if (me.m_bSortAscending)
			return s1.Compare(s2);
		else
			return s2.Compare(s1);
	}
}

void CModelCheckDlg::OnColumnclickIssuelist(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	
	if (pNMLV->iSubItem == m_nSortColumn) {
		m_bSortAscending = !m_bSortAscending;
	}
	else {
		m_nSortColumn = pNMLV->iSubItem;
		m_bSortAscending = true;
	}

	m_wndIssueList.SortItemsEx(SortFunc, (DWORD_PTR)this);
	*pResult = 0;
}


void CModelCheckDlg::OnActivateListItem(int iItem)
{
	auto pDoc = GetActiveDoc();
	if (!pDoc)
		return;
	auto pModel = GetActiveModel();
	if (!pModel)
		return;

	if (iItem >= 0) {
		auto data = m_wndIssueList.GetItemData(iItem);
		if (data) {
			auto p = (IssueData*)data;

			if (!p->relatingInstancesCollected) {
				p->relatingInstancesCollected = true;
				auto instance = internalGetInstanceFromP21Line(GetActiveSdaiModel(), p->stepId);
				if (instance) {

					int_t searchEntities[3] = {
					sdaiGetEntity(GetActiveSdaiModel(), "IfcProduct"),
					sdaiGetEntity(GetActiveSdaiModel(), "IfcProject"),
					0 };

					if (pModel && pModel->getAP() == enumAP::STEP) {
						searchEntities[0] = sdaiGetEntity(GetActiveSdaiModel(), "PRODUCT_DEFINITION");
						searchEntities[1] = 0;
					}

					BeginWaitCursor();
					CollectReferencingInstancesRecursive(p->relatingInstances, instance, searchEntities);
					EndWaitCursor();
				}

			}

			for (auto inst : p->relatingInstances) {
				if (auto stepId = internalGetP21Line(inst)) {
					//if (auto pInst = pModel->GetInstanceByExpressID(stepId)) {#todo
					//	pDoc->selectInstance(NULL, pInst);
					//	return; //>>>>
					//}
				}
			}
		}
	}
}

void CModelCheckDlg::OnDblclkIssuelist(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	//LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	//OnActivateListItem(pNMItemActivate->iItem);
	*pResult = 0;
}


void CModelCheckDlg::OnClickIssuelist(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	OnActivateListItem(pNMItemActivate->iItem);
	*pResult = 0;
}


void CModelCheckDlg::OnClickedViewAllIssues()
{
	auto dlgres = AfxMessageBox(L"Getting all issues may take significant time. Are you sure to proceed?", MB_OKCANCEL);
	if (dlgres == IDOK) {
		BeginWaitCursor();
		FillIssueList(true);
		EndWaitCursor();
	}
}

SdaiModel CModelCheckDlg::GetActiveSdaiModel()
{
	if (auto pModel = GetActiveModel()) {
		return dynamic_cast<_ap_model*>(pModel)->getSdaiInstance();
	}
	return NULL;
}

_ap_model* CModelCheckDlg::GetActiveModel()
{
	if (auto pDoc = GetActiveDoc()) {
		return _ptr<_ap_model>(pDoc->getModel());
	}
	return NULL;
}

CMySTEPViewerDoc* CModelCheckDlg::GetActiveDoc()
{
	if (auto pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd())) {
		return DYNAMIC_DOWNCAST(CMySTEPViewerDoc, pMainFrame->GetActiveDocument());
	}
	return NULL;
}