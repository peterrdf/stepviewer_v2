#pragma once

#include "_mvc.h"

// ************************************************************************************************
class CTreeViewBase : public _view
{

public: // Methods

	CTreeViewBase()
		: _view()
	{
	}

	virtual ~CTreeViewBase()
	{
		getController()->unRegisterView(this);
	}

	// _view
	virtual void onControllerChanged()
	{
		ASSERT(getController() != nullptr);

		getController()->registerView(this);
	}

	virtual void Load() PURE;

	virtual CImageList* GetImageList() const PURE;

	virtual void OnShowWindow(BOOL /*bShow*/, UINT /*nStatus*/)
	{
	}

	virtual void OnTreeItemClick(NMHDR* /*pNMHDR*/, LRESULT* pResult)
	{
		*pResult = 0;
	}

	virtual void OnTreeItemExpanding(NMHDR* /*pNMHDR*/, LRESULT* pResult)
	{
		*pResult = 0;
	}

	virtual void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/)
	{
	}

	virtual void OnSearch()
	{
	}
};

