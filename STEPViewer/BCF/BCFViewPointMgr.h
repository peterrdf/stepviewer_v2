#pragma once

class CBCFView;
class _instance;

class CBCFViewPointMgr
{
public:
	CBCFViewPointMgr(CBCFView& view) : m_view(view){}

	void SetViewFromComment(BCFComment& comment);

	bool SaveCurrentViewToComent(BCFComment& comment);

private:
	void ViewSelection(BCFViewPoint& vp);
	bool SaveSelection(BCFViewPoint& vp);
	
	_instance* SearchComponent(BCFComponent& comp);
	_instance* SearchIfcComponent(const char* ifcGuid);

private:
	CBCFView& m_view;
};
