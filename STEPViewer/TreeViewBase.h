#pragma once

#include "ViewBase.h"

// ------------------------------------------------------------------------------------------------
class CTreeViewBase : public CViewBase
{

public: // Methods

	// --------------------------------------------------------------------------------------------
	CTreeViewBase()
		: CViewBase()
	{
	}

	// --------------------------------------------------------------------------------------------
	virtual ~CTreeViewBase()
	{
		GetController()->UnRegisterView(this);
	}

	// --------------------------------------------------------------------------------------------
	// CViewBase
	virtual void OnControllerChanged()
	{
		ASSERT(GetController() != nullptr);

		GetController()->RegisterView(this);
	}

	// --------------------------------------------------------------------------------------------
	virtual void Load() PURE;

	// --------------------------------------------------------------------------------------------
	virtual CImageList* GetImageList() const PURE;

	// --------------------------------------------------------------------------------------------
	virtual void OnShowWindow(BOOL /*bShow*/, UINT /*nStatus*/)
	{
	}

	// --------------------------------------------------------------------------------------------
	virtual void OnTreeItemClick(NMHDR* /*pNMHDR*/, LRESULT* pResult)
	{
		*pResult = 0;
	}

	// --------------------------------------------------------------------------------------------
	virtual void OnTreeItemExpanding(NMHDR* /*pNMHDR*/, LRESULT* pResult)
	{
		*pResult = 0;
	}

	// --------------------------------------------------------------------------------------------
	virtual void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/)
	{
	}

	// --------------------------------------------------------------------------------------------
	virtual void OnSearch()
	{
	}
};

