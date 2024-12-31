#pragma once

#include "_ap_mvc.h"

#include "TreeCtrlEx.h"
#include "StructureViewConsts.h"

// ************************************************************************************************
class CModelStructureViewBase : public _view
{

protected: // Members

	CTreeCtrlEx* m_pTreeCtrl;

public: // Methods

	CModelStructureViewBase(CTreeCtrlEx* pTreeCtrl)
		: _view()
		, m_pTreeCtrl(pTreeCtrl)
	{
		ASSERT(m_pTreeCtrl != nullptr);
	}

	virtual ~CModelStructureViewBase()
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

	virtual void OnShowWindow(BOOL /*bShow*/, UINT /*nStatus*/) {}
	virtual void OnTreeItemClick(NMHDR* /*pNMHDR*/, LRESULT* pResult) {	*pResult = 0; }
	virtual void OnTreeItemExpanding(NMHDR* /*pNMHDR*/, LRESULT* pResult) { *pResult = 0; }
	virtual void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/) {}
	virtual void OnSearch() {}

protected: // Methods

	void LoadHeader(_ap_model* pModel, HTREEITEM hModel)
	{
		// Header
		TV_INSERTSTRUCT tvInsertStruct;
		tvInsertStruct.hParent = hModel;
		tvInsertStruct.hInsertAfter = TVI_LAST;
		tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
		tvInsertStruct.item.pszText = L"Header";
		tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY_SET;
		tvInsertStruct.item.lParam = NULL;
		HTREEITEM hHeader = m_pTreeCtrl->InsertItem(&tvInsertStruct);

		wchar_t* szText = nullptr;

		// Descriptions
		{
			tvInsertStruct.hParent = hHeader;
			tvInsertStruct.hInsertAfter = TVI_LAST;
			tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
			tvInsertStruct.item.pszText = L"Descriptions";
			tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY_SET;
			tvInsertStruct.item.lParam = NULL;
			HTREEITEM hDescriptions = m_pTreeCtrl->InsertItem(&tvInsertStruct);

			int_t iItem = 0;
			if (!GetSPFFHeaderItem(pModel->getSdaiModel(), 0, iItem, sdaiUNICODE, (char**)&szText))
			{
				while (!GetSPFFHeaderItem(pModel->getSdaiModel(), 0, iItem++, sdaiUNICODE, (char**)&szText))
				{
					tvInsertStruct.hParent = hDescriptions;
					tvInsertStruct.hInsertAfter = TVI_LAST;
					tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
					tvInsertStruct.item.pszText = szText;
					tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
					tvInsertStruct.item.lParam = NULL;
					m_pTreeCtrl->InsertItem(&tvInsertStruct);
				}
			} // if (!GetSPFFHeaderItem(...
		}

		// ImplementationLevel
		{
			GetSPFFHeaderItem(pModel->getSdaiModel(), 1, 0, sdaiUNICODE, (char**)&szText);

			wstring strItem = L"ImplementationLevel";
			strItem += L" = '";
			strItem += szText != nullptr ? szText : L"";
			strItem += L"'";

			tvInsertStruct.hParent = hHeader;
			tvInsertStruct.hInsertAfter = TVI_LAST;
			tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
			tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
			tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
			tvInsertStruct.item.lParam = NULL;
			m_pTreeCtrl->InsertItem(&tvInsertStruct);
		}

		// Name
		{
			GetSPFFHeaderItem(pModel->getSdaiModel(), 2, 0, sdaiUNICODE, (char**)&szText);

			wstring strItem = L"Name";
			strItem += L" = '";
			strItem += szText != nullptr ? szText : L"";
			strItem += L"'";

			tvInsertStruct.hParent = hHeader;
			tvInsertStruct.hInsertAfter = TVI_LAST;
			tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
			tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
			tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
			tvInsertStruct.item.lParam = NULL;
			m_pTreeCtrl->InsertItem(&tvInsertStruct);
		}

		// TimeStamp
		{
			GetSPFFHeaderItem(pModel->getSdaiModel(), 3, 0, sdaiUNICODE, (char**)&szText);

			wstring strItem = L"TimeStamp";
			strItem += L" = '";
			strItem += szText != nullptr ? szText : L"";
			strItem += L"'";

			tvInsertStruct.hParent = hHeader;
			tvInsertStruct.hInsertAfter = TVI_LAST;
			tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
			tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
			tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
			tvInsertStruct.item.lParam = NULL;
			m_pTreeCtrl->InsertItem(&tvInsertStruct);
		}

		// Authors
		{
			tvInsertStruct.hParent = hHeader;
			tvInsertStruct.hInsertAfter = TVI_LAST;
			tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
			tvInsertStruct.item.pszText = L"Authors";
			tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY_SET;
			tvInsertStruct.item.lParam = NULL;
			HTREEITEM hDescriptions = m_pTreeCtrl->InsertItem(&tvInsertStruct);

			int_t iItem = 0;
			if (!GetSPFFHeaderItem(pModel->getSdaiModel(), 4, iItem, sdaiUNICODE, (char**)&szText))
			{
				while (!GetSPFFHeaderItem(pModel->getSdaiModel(), 4, iItem++, sdaiUNICODE, (char**)&szText))
				{
					tvInsertStruct.hParent = hDescriptions;
					tvInsertStruct.hInsertAfter = TVI_LAST;
					tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
					tvInsertStruct.item.pszText = szText;
					tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
					tvInsertStruct.item.lParam = NULL;
					m_pTreeCtrl->InsertItem(&tvInsertStruct);
				}
			} // if (!GetSPFFHeaderItem(...
		}

		// Organizations
		{
			tvInsertStruct.hParent = hHeader;
			tvInsertStruct.hInsertAfter = TVI_LAST;
			tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
			tvInsertStruct.item.pszText = L"Organizations";
			tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY_SET;
			tvInsertStruct.item.lParam = NULL;

			HTREEITEM hDescriptions = m_pTreeCtrl->InsertItem(&tvInsertStruct);

			int_t iItem = 0;
			if (!GetSPFFHeaderItem(pModel->getSdaiModel(), 5, iItem, sdaiUNICODE, (char**)&szText))
			{
				while (!GetSPFFHeaderItem(pModel->getSdaiModel(), 5, iItem++, sdaiUNICODE, (char**)&szText))
				{
					tvInsertStruct.hParent = hDescriptions;
					tvInsertStruct.hInsertAfter = TVI_LAST;
					tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
					tvInsertStruct.item.pszText = szText;
					tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
					tvInsertStruct.item.lParam = NULL;
					m_pTreeCtrl->InsertItem(&tvInsertStruct);
				}
			} // if (!GetSPFFHeaderItem(...
		}

		// PreprocessorVersion
		{
			GetSPFFHeaderItem(pModel->getSdaiModel(), 6, 0, sdaiUNICODE, (char**)&szText);

			wstring strItem = L"PreprocessorVersion";
			strItem += L" = '";
			strItem += szText != nullptr ? szText : L"";
			strItem += L"'";

			tvInsertStruct.hParent = hHeader;
			tvInsertStruct.hInsertAfter = TVI_LAST;
			tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
			tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
			tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
			tvInsertStruct.item.lParam = NULL;
			m_pTreeCtrl->InsertItem(&tvInsertStruct);
		}

		// OriginatingSystem
		{
			GetSPFFHeaderItem(pModel->getSdaiModel(), 7, 0, sdaiUNICODE, (char**)&szText);

			wstring strItem = L"OriginatingSystem";
			strItem += L" = '";
			strItem += szText != nullptr ? szText : L"";
			strItem += L"'";

			tvInsertStruct.hParent = hHeader;
			tvInsertStruct.hInsertAfter = TVI_LAST;
			tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
			tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
			tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
			tvInsertStruct.item.lParam = NULL;
			m_pTreeCtrl->InsertItem(&tvInsertStruct);
		}

		// Authorization
		{
			GetSPFFHeaderItem(pModel->getSdaiModel(), 8, 0, sdaiUNICODE, (char**)&szText);

			wstring strItem = L"Authorization";
			strItem += L" = '";
			strItem += szText != nullptr ? szText : L"";
			strItem += L"'";

			tvInsertStruct.hParent = hHeader;
			tvInsertStruct.hInsertAfter = TVI_LAST;
			tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
			tvInsertStruct.item.pszText = (LPWSTR)strItem.c_str();
			tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
			tvInsertStruct.item.lParam = NULL;
			m_pTreeCtrl->InsertItem(&tvInsertStruct);
		}

		// FileSchemas
		{
			tvInsertStruct.hParent = hHeader;
			tvInsertStruct.hInsertAfter = TVI_LAST;
			tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
			tvInsertStruct.item.pszText = L"FileSchemas";
			tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY_SET;
			tvInsertStruct.item.lParam = NULL;
			HTREEITEM hDescriptions = m_pTreeCtrl->InsertItem(&tvInsertStruct);

			int_t iItem = 0;
			if (!GetSPFFHeaderItem(pModel->getSdaiModel(), 9, iItem, sdaiUNICODE, (char**)&szText))
			{
				while (!GetSPFFHeaderItem(pModel->getSdaiModel(), 9, iItem++, sdaiUNICODE, (char**)&szText))
				{
					tvInsertStruct.hParent = hDescriptions;
					tvInsertStruct.hInsertAfter = TVI_LAST;
					tvInsertStruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
					tvInsertStruct.item.pszText = szText;
					tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = IMAGE_PROPERTY;
					tvInsertStruct.item.lParam = NULL;
					m_pTreeCtrl->InsertItem(&tvInsertStruct);
				}
			} // if (!GetSPFFHeaderItem(...
		}
	}
};

