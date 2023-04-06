
#include "stdafx.h"
#include "ViewTree.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CViewTree

CViewTree::CViewTree()
	: m_pItemStateProvider(nullptr)
{
}

CViewTree::~CViewTree()
{
}

BEGIN_MESSAGE_MAP(CViewTree, CTreeCtrl)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CViewTree::OnNMCustomdraw)
    ON_WM_KEYUP()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewTree message handlers

// ------------------------------------------------------------------------------------------------
BOOL CViewTree::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	BOOL bRes = CTreeCtrl::OnNotify(wParam, lParam, pResult);

	NMHDR* pNMHDR = (NMHDR*)lParam;
	ASSERT(pNMHDR != nullptr);

	if (pNMHDR && pNMHDR->code == TTN_SHOW && GetToolTips() != nullptr)
	{
		GetToolTips()->SetWindowPos(&wndTop, -1, -1, -1, -1, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSIZE);
	}

	return bRes;
}

// ------------------------------------------------------------------------------------------------
void CViewTree::SetItemStateProvider(CItemStateProvider* pItemStateProvider)
{
    m_pItemStateProvider = pItemStateProvider;
}

// ------------------------------------------------------------------------------------------------
void CViewTree::OnNMCustomdraw(NMHDR* pNMHDR, LRESULT* pResult)
{
    NMTVCUSTOMDRAW* pNMCD = reinterpret_cast<NMTVCUSTOMDRAW*>(pNMHDR);

    switch (pNMCD->nmcd.dwDrawStage)
    {
        case CDDS_PREPAINT:
        {
            *pResult = CDRF_NOTIFYITEMDRAW;
        }
        break;

        case CDDS_ITEMPREPAINT:
        {
            if (m_pItemStateProvider != nullptr)
            {
                HTREEITEM hItem = (HTREEITEM)pNMCD->nmcd.dwItemSpec;
                if (m_pItemStateProvider->IsSelected(hItem))
                {
                    pNMCD->clrTextBk = RGB(255, 0, 0);
                }
            }

            *pResult = CDRF_DODEFAULT;
        }
        break;

        default:
        {
            *pResult = 0;
        }
        break;
    } // switch (pNMCD->nmcd.dwDrawStage)
}

// ------------------------------------------------------------------------------------------------
void CViewTree::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if ((GetKeyState(VK_CONTROL) & 0x8000) &&
        ((nChar == L'c') || (nChar == L'C')))
    {
        HTREEITEM hItem = GetSelectedItem();
        if (hItem != NULL)
        {
            CString strText = GetItemText(hItem);

            HTREEITEM hParent = GetParentItem(hItem);
            while (hParent != NULL)
            {
                strText += L" - ";
                strText += GetItemText(hParent);

                hParent = GetParentItem(hParent);
            }

            // https://www.codeproject.com/Articles/2242/Using-the-Clipboard-Part-I-Transferring-Simple-Tex
            if (OpenClipboard())
            {
                EmptyClipboard();

                HGLOBAL hClipboardData = GlobalAlloc(GMEM_DDESHARE, sizeof(wchar_t) * (wcslen(strText) + 1));
                if (hClipboardData != NULL)
                {
                    wchar_t* pchData = (wchar_t*)GlobalLock(hClipboardData);
                    if (pchData != nullptr)
                    {
                        wcscpy(pchData, (LPCTSTR)strText);

                        GlobalUnlock(hClipboardData);

                        SetClipboardData(CF_UNICODETEXT, hClipboardData);

                        CloseClipboard();

                    } // if (pchData != nullptr)
                } // if (hClipboardData != NULL)
            } // if (OpenClipboard())
        } // if (hItem != NULL)
    } // if ((GetKeyState(VK_CONTROL)

    CTreeCtrl::OnKeyUp(nChar, nRepCnt, nFlags);
}
