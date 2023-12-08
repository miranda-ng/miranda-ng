/*
Scriver

Copyright (c) 2000-09 Miranda ICQ/IM project,

all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

class CErrorDlg : public CDlgBase
{
	ptrW m_wszText;
	CMStringW m_wszName, m_wszDescr;
	CMsgDialog *m_pOwner;
	SendQueue::Item *m_queueItem;

	CCtrlBase m_errorText, m_msgText;

public:
	CErrorDlg(CMsgDialog *pOwner, const wchar_t *pwszDescr, SendQueue::Item *pItem) :
		CDlgBase(g_plugin, IDD_MSGSENDERROR),
		m_pOwner(pOwner),
		m_wszText(mir_utf8decodeW(pItem->sendBuffer)),
		m_wszDescr(pwszDescr != nullptr ? pwszDescr : TranslateT("An unknown error has occurred.")),
		m_queueItem(pItem),

		m_msgText(this, IDC_MSGTEXT),
		m_errorText(this, IDC_ERRORTEXT)
	{
		const wchar_t *pwszName = Clist_GetContactDisplayName(pItem->hContact);
		if (pwszName)
			m_wszName.Format(L"%s - %s", TranslateT("Send error"), pwszName);
		else
			m_wszName = TranslateT("Send error");
	}

	bool OnInitDialog() override
	{
		m_queueItem->hwndErrorDlg = m_hwnd;
		ShowWindow(GetParent(m_hwndParent), SW_RESTORE);

		m_errorText.SetText(m_wszDescr);
		SetWindowText(m_hwnd, m_wszName);

		SETTEXTEX st = { ST_DEFAULT, 1200 };
		m_msgText.SendMsg(EM_SETTEXTEX, (WPARAM)&st, (LPARAM)m_wszText.get());

		RECT rc, rcParent;
		GetWindowRect(m_hwnd, &rc);
		GetWindowRect(GetParent(m_hwndParent), &rcParent);
		SetWindowPos(m_hwnd, HWND_TOP, rcParent.left + (rcParent.right - rcParent.left - rc.right + rc.left) / 2, rcParent.top + (rcParent.bottom - rcParent.top - rc.bottom + rc.top) / 2, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
		return true;
	}

	void OnDestroy() override
	{
		m_queueItem->hwndErrorDlg = nullptr;
		m_pOwner->HandleError(m_bSucceeded, m_queueItem);
	}
};

void CMsgDialog::ShowError(const wchar_t *pwszMsg, SendQueue::Item *pItem)
{
	auto *pDlg = new CErrorDlg(this, pwszMsg, pItem);
	pDlg->SetParent(m_hwnd);
	pDlg->Show();
}

void CMsgDialog::HandleError(bool bRetry, SendQueue::Item *pItem)
{
	if (bRetry) {
		StartMessageSending();
		SendQueue::SendItem(pItem);
	}
	else {
		SendQueue::RemoveItem(pItem);
		SetFocus(m_message.GetHwnd());
	}
}
