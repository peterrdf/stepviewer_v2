#pragma once

class CBCFView;

class CBCFViewPointMgr
{
public:
	CBCFViewPointMgr(CBCFView& view) : m_view(view){}

	void SetViewFromComment(BCFComment& comment);

	bool SaveCurrentViewToComent(BCFComment& comment);

private:
	void ViewSelection(BCFViewPoint& vp);
	bool SaveSelection(BCFViewPoint& vp);

private:
	CBCFView& m_view;
};
