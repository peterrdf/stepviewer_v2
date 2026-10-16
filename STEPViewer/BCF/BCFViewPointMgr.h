#pragma once

#include "bcfAPI.h"
#include <string>

class CMySTEPViewerDoc;
class CMySTEPViewerView;
class _instance;

class CBCFViewPointMgr
{
public:
	explicit CBCFViewPointMgr(CMySTEPViewerDoc& document);

	void SetViewFromComment(BCFComment& comment);

	bool SaveCurrentViewToComent(BCFComment& comment);
	void SetSelectionFromComment(BCFComment& comment);
	bool SaveCurrentSelectionToComment(BCFComment& comment);
	void SetVisibilityFromComment(BCFComment& comment);
	bool SaveCurrentVisibilityToComment(BCFComment& comment);
	bool SaveSelectedAsVisibleToComment(BCFComment& comment);
	std::wstring GetComponentDisplayName(BCFComponent& comp);

private:
	struct InstanceIDs
	{
		const char* ifcGuid = nullptr;
		std::string authoringToolId;
	};

private:
	void ApplySelectionToViewer(BCFViewPoint* vp);
	bool SaveSelection(BCFViewPoint& vp);

	void ApplyColoringToViewer(BCFViewPoint* vp);
	bool SaveColoring(BCFViewPoint& vp);

	void ApplyVisibilityToViewer(BCFViewPoint* vp);
	bool SaveVisibility(BCFViewPoint& vp);
	CMySTEPViewerDoc& GetViewerDoc();
	CMySTEPViewerView* GetViewerView();

	_instance* SearchComponent(BCFComponent& comp);
	_instance* SearchIfcGuid(const char* ifcGuid);
	_instance* SearchAuthoringToolId(const char* authoringToolId);
	void GetExpressIdPath(std::string& path, _instance* inst);

	bool IsSpaceBoundary(SdaiInstance /*inst*/) { return false; /*TODO*/ }

	InstanceIDs GetInstanceIDs(_instance* inst);

	COLORREF GetColorRef(const char* strcolor);
	std::string GetColorStr(COLORREF clr);

private:
	CMySTEPViewerDoc& m_document;
};
