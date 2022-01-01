/*

Copyright 2000-12 Miranda IM, 2012-22 Miranda NG team,
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
#include "msgs.h"

class CErrorDlg : public CDlgBase
{
	TMsgQueue *m_item;
	CMStringW  m_wszErr;
	CCtrlBase  m_errorText, m_msgText;

public:
	CErrorDlg(TMsgQueue *item, const wchar_t *err, HWND hwndParent) :
		CDlgBase(g_plugin, IDD_MSGSENDERROR),
		m_item(item),
		m_msgText(this, IDC_MSGTEXT),
		m_errorText(this, IDC_ERRORTEXT)
	{
		if (err)
			m_wszErr = err;

		SetParent(hwndParent);
	}

	bool OnInitDialog() override
	{
		if (m_wszErr.IsEmpty())
			m_errorText.SetText(TranslateT("An unknown error has occurred."));
		else
			m_errorText.SetText(m_wszErr);

		m_msgText.SetText(ptrW(mir_utf8decodeW(m_item->szMsg)));

		if (m_hwndParent != nullptr) {
			RECT rc, rcParent;
			if (GetWindowRect(m_hwnd, &rc))
				if (GetWindowRect(m_hwndParent, &rcParent))
					SetWindowPos(m_hwnd, nullptr, (rcParent.left + rcParent.right - (rc.right - rc.left)) / 2, (rcParent.top + rcParent.bottom - (rc.bottom - rc.top)) / 2, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		}
		return true;
	}

	bool OnApply() override
	{
		SendMessageDirect(ptrW(mir_utf8decodeW(m_item->szMsg)), m_item->hContact);
		return true;
	}

	void OnDestroy() override
	{
		mir_free(m_item->szMsg);
		mir_free(m_item);
	}
};

void MessageFailureProcess(TMsgQueue *item, const wchar_t *err)
{
	MCONTACT hContact = db_mc_tryMeta(item->hContact);

	HWND hwnd = Srmm_FindWindow(hContact);
	if (hwnd == nullptr) {
		// If message window doesn't already exist, open a new one
		SendMessageCmd(item->hContact, nullptr);
		hwnd = Srmm_FindWindow(hContact);
	}
	else {
		auto *pDlg = (CMsgDialog *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		pDlg->RemakeLog();
	}

	Skin_PlaySound("SendError");

	auto *pDlg = new CErrorDlg(item, err, hwnd);
	pDlg->Show();
}
