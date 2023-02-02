#pragma once

#include "STEPView.h"

// ------------------------------------------------------------------------------------------------
class CSTEPTreeViewBase : public CSTEPView
{

public: // Methods

	// --------------------------------------------------------------------------------------------
	CSTEPTreeViewBase()
		: CSTEPView()
	{
	}

	// --------------------------------------------------------------------------------------------
	virtual ~CSTEPTreeViewBase()
	{
		GetController()->UnRegisterView(this);
	}

	// --------------------------------------------------------------------------------------------
	// CSTEPView
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
};

