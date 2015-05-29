/*
Copyright (c) 2013-15 Miranda NG project (http://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

struct WALLPOST_FORM_PARAMS
{
	TCHAR* ptszMsg;
	TCHAR* ptszUrl;
	TCHAR* ptszNick;
	bool bFriendsOnly;

	WALLPOST_FORM_PARAMS(TCHAR* nick) :
		ptszNick(nick),
		bFriendsOnly(false)
	{
		ptszMsg = ptszUrl = NULL;
	}

	~WALLPOST_FORM_PARAMS()
	{
		mir_free(ptszMsg);
		mir_free(ptszUrl);
		mir_free(ptszNick);
	}
};

static INT_PTR CALLBACK WallPostFormDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	WALLPOST_FORM_PARAMS *param = (WALLPOST_FORM_PARAMS *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (msg) {
	case WM_INITDIALOG:
		param = (WALLPOST_FORM_PARAMS *)lParam;
		TranslateDialogDefault(hwndDlg);
		{
			SetDlgItemText(hwndDlg, IDC_ST_WARNING, _T(""));
			CMString tszTitle;
			tszTitle.AppendFormat(_T("%s %s"), TranslateT("Wall message for"), param->ptszNick);
			SetWindowText(hwndDlg, tszTitle);
		}		
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)param);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			return TRUE;

		case IDOK:
			TCHAR tszMsg[4096], tszUrl[4096];
			GetDlgItemText(hwndDlg, IDC_ED_MSG, tszMsg, SIZEOF(tszMsg));
			GetDlgItemText(hwndDlg, IDC_ED_URL, tszUrl, SIZEOF(tszUrl));

			if (IsEmpty(tszMsg) && IsEmpty(tszUrl)) {
				SetDlgItemText(hwndDlg, IDC_ST_WARNING, TranslateT("Attention! Message body or url should not be empty!"));
				return FALSE;
			}

			if (!IsEmpty(tszMsg))
				param->ptszMsg = mir_tstrdup(tszMsg); 
			if (!IsEmpty(tszUrl))
				param->ptszUrl = mir_tstrdup(tszUrl);
			
			param->bFriendsOnly = IsDlgButtonChecked(hwndDlg, IDC_ONLY_FRIENDS) == BST_CHECKED;
			EndDialog(hwndDlg, (INT_PTR)param);
			return TRUE;
		}
	}

	return FALSE;
}

INT_PTR __cdecl CVkProto::SvcWallPost(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::SvcWallPost");

	WALLPOST_FORM_PARAMS param(db_get_tsa(hContact, m_szModuleName, "Nick"));
	if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_WALLPOST), NULL, WallPostFormDlgProc, (LPARAM)&param) == 0)
		return 1;

	WallPost((MCONTACT)hContact, param.ptszMsg, param.ptszUrl, param.bFriendsOnly);
	return 0;
}

void CVkProto::WallPost(MCONTACT hContact, TCHAR *ptszMsg, TCHAR *ptszUrl, bool bFriendsOnly)
{
	debugLogA("CVkProto::WallPost");
	if (!IsOnline() || (IsEmpty(ptszMsg) && IsEmpty(ptszUrl)))
		return;

	LONG userID = hContact ? m_myUserId : getDword(hContact, "ID", -1);
	if (userID == -1 || userID == VK_FEED_USER)
		return;

	AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_POST, "/method/wall.post.json", true, &CVkProto::OnReceiveSmth)
		<< INT_PARAM("owner_id", userID)
		<< INT_PARAM("friends_only", bFriendsOnly ? 1 : 0)
		<< VER_API;

	if (!IsEmpty(ptszMsg))
		pReq << TCHAR_PARAM("message", ptszMsg);

	if (!IsEmpty(ptszUrl))
		pReq << TCHAR_PARAM("attachments", ptszUrl);

	Push(pReq);
}