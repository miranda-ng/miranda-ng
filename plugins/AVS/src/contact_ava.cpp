/*
Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

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

class CContactAvatarDlg : public CDlgBase
{
	MCONTACT m_hContact;
	HANDLE   m_hHook = 0;

	UI_MESSAGE_MAP(CContactAvatarDlg, CDlgBase);
		UI_MESSAGE(WM_DRAWITEM, OnDrawItem);
		UI_MESSAGE(DM_AVATARCHANGED, OnAvatarChanged);
	UI_MESSAGE_MAP_END();

	INT_PTR OnAvatarChanged(UINT, WPARAM, LPARAM)
	{
		InvalidateRect(GetDlgItem(m_hwnd, IDC_PROTOPIC), nullptr, TRUE);
		return 0;
	}

	INT_PTR OnDrawItem(UINT, WPARAM, LPARAM lParam)
	{
		LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
		if (dis->CtlType == ODT_BUTTON && dis->CtlID == IDC_PROTOPIC) {
			AVATARDRAWREQUEST avdrq = { 0 };
			GetClientRect(GetDlgItem(m_hwnd, IDC_PROTOPIC), &avdrq.rcDraw);

			FillRect(dis->hDC, &avdrq.rcDraw, GetSysColorBrush(COLOR_BTNFACE));

			avdrq.hContact = m_hContact;
			avdrq.hTargetDC = dis->hDC;
			avdrq.dwFlags |= AVDRQ_DRAWBORDER;
			avdrq.clrBorder = GetSysColor(COLOR_BTNTEXT);
			avdrq.radius = 6;
			if (!CallService(MS_AV_DRAWAVATAR, 0, (LPARAM)&avdrq)) {
				// Get text rectangle
				RECT rc = avdrq.rcDraw;
				rc.top += 10;
				rc.bottom -= 10;
				rc.left += 10;
				rc.right -= 10;

				// Calc text size
				RECT rc_ret = rc;
				DrawText(dis->hDC, TranslateT("Contact has no avatar"), -1, &rc_ret,
					DT_WORDBREAK | DT_NOPREFIX | DT_CENTER | DT_CALCRECT);

				// Calc needed size
				rc.top += ((rc.bottom - rc.top) - (rc_ret.bottom - rc_ret.top)) / 2;
				rc.bottom = rc.top + (rc_ret.bottom - rc_ret.top);
				DrawText(dis->hDC, TranslateT("Contact has no avatar"), -1, &rc,
					DT_WORDBREAK | DT_NOPREFIX | DT_CENTER);
			}

			FrameRect(dis->hDC, &avdrq.rcDraw, GetSysColorBrush(COLOR_BTNSHADOW));
		}
		return TRUE;
	}

	void ReloadAvatar()
	{
		SaveTransparentData(m_hwnd, m_hContact, chkProtect.IsChecked());
		ChangeAvatar(m_hContact, true);
		OnAvatarChanged(0, 0, 0);
	}

	void SetAvatarName()
	{
		uint8_t is_locked = db_get_b(m_hContact, "ContactPhoto", "Locked", 0);

		wchar_t szFinalName[MAX_PATH];
		szFinalName[0] = 0;

		DBVARIANT dbv = {};
		if (is_locked && !db_get_ws(m_hContact, "ContactPhoto", "Backup", &dbv)) {
			MyPathToAbsolute(dbv.pwszVal, szFinalName);
			db_free(&dbv);
		}
		else if (!db_get_ws(m_hContact, "ContactPhoto", "RFile", &dbv)) {
			MyPathToAbsolute(dbv.pwszVal, szFinalName);
			db_free(&dbv);
		}
		else if (!db_get_ws(m_hContact, "ContactPhoto", "File", &dbv)) {
			MyPathToAbsolute(dbv.pwszVal, szFinalName);
			db_free(&dbv);
		}
		szFinalName[MAX_PATH - 1] = 0;
		SetDlgItemText(m_hwnd, IDC_AVATARNAME, szFinalName);
	}

	CCtrlSpin spin1, spin2;
	CCtrlCheck chkProtect, chkMakeTrans, chkHide;
	CCtrlButton btnUseDefault, btnChange, btnReset, btnDefault;

public:
	CContactAvatarDlg(MCONTACT hContact) :
		CDlgBase(g_plugin, IDD_AVATAROPTIONS),
		m_hContact(hContact),
		spin1(this, IDC_BKG_NUM_POINTS_SPIN, 8, 2),
		spin2(this, IDC_BKG_COLOR_DIFFERENCE_SPIN, 100),
		btnReset(this, IDC_RESET),
		btnChange(this, IDC_CHANGE),
		btnDefault(this, IDC_DELETE),
		btnUseDefault(this, ID_USE_DEFAULTS),
		chkHide(this, IDC_HIDEAVATAR),
		chkProtect(this, IDC_PROTECTAVATAR),
		chkMakeTrans(this, IDC_MAKETRANSPBKG)
	{
		btnReset.OnClick = Callback(this, &CContactAvatarDlg::onClick_Reset);
		btnChange.OnClick = Callback(this, &CContactAvatarDlg::onClick_Change);
		btnDefault.OnClick = Callback(this, &CContactAvatarDlg::onClick_Default);
		btnUseDefault.OnClick = Callback(this, &CContactAvatarDlg::onClick_UseDefault);

		chkProtect.OnChange = Callback(this, &CContactAvatarDlg::onChange_Protect);
		chkMakeTrans.OnChange = Callback(this, &CContactAvatarDlg::onChange_MakeTrans);
	}
	
	bool OnInitDialog() override
	{
		m_hHook = HookEventMessage(ME_AV_AVATARCHANGED, m_hwnd, DM_AVATARCHANGED);

		if (m_hContact) {
			wchar_t szTitle[512];
			mir_snwprintf(szTitle, TranslateT("Set avatar options for %s"), Clist_GetContactDisplayName(m_hContact));
			SetWindowText(m_hwnd, szTitle);
		}

		SetAvatarName();
		ShowWindow(m_hwnd, SW_SHOWNORMAL);
		OnAvatarChanged(0, 0, 0);
		chkProtect.SetState(db_get_b(m_hContact, "ContactPhoto", "Locked", 0));
		chkHide.SetState(Contact::IsHidden(m_hContact));

		LoadTransparentData(m_hwnd, GetContactThatHaveTheAvatar(m_hContact));
		SendMessage(m_hwnd, WM_SETICON, IMAGE_ICON, (LPARAM)g_plugin.getIcon(IDI_AVATAR));
		return true;
	}

	bool OnApply() override
	{
		bool locked = chkProtect.IsChecked();
		bool hidden = chkHide.IsChecked();

		SetAvatarAttribute(m_hContact, AVS_HIDEONCLIST, hidden);
		if (hidden != Contact::IsHidden(m_hContact))
			Contact::Hide(m_hContact, hidden);

		if (!locked && db_get_b(m_hContact, "ContactPhoto", "NeedUpdate", 0))
			QueueAdd(m_hContact);
		return true;
	}

	void OnDestroy() override
	{
		UnhookEvent(m_hHook);
	}

	void onClick_UseDefault(CCtrlButton *)
	{
		MCONTACT hContact = GetContactThatHaveTheAvatar(m_hContact);

		db_unset(hContact, "ContactPhoto", "MakeTransparentBkg");
		db_unset(hContact, "ContactPhoto", "TranspBkgNumPoints");
		db_unset(hContact, "ContactPhoto", "TranspBkgColorDiff");

		LoadTransparentData(m_hwnd, hContact);
		ReloadAvatar();
	}
	
	void onClick_Change(CCtrlButton *)
	{
		SetAvatar(m_hContact, 0);
		SetAvatarName();
		chkProtect.SetState(db_get_b(m_hContact, "ContactPhoto", "Locked"));
	}

	void onClick_Reset(CCtrlButton *)
	{
		ProtectAvatar(m_hContact, 0);
		if (MessageBox(nullptr, TranslateT("Delete picture file from disk (may be necessary to force a reload, but will delete local pictures)?"), TranslateT("Reset contact picture"), MB_YESNO) == IDYES) {
			DBVARIANT dbv = { 0 };
			if (!db_get_ws(m_hContact, "ContactPhoto", "File", &dbv)) {
				DeleteFileW(dbv.pwszVal);
				db_free(&dbv);
			}
		}
		db_unset(m_hContact, "ContactPhoto", "Locked");
		db_unset(m_hContact, "ContactPhoto", "Backup");
		db_unset(m_hContact, "ContactPhoto", "RFile");
		db_unset(m_hContact, "ContactPhoto", "File");
		db_unset(m_hContact, "ContactPhoto", "Format");

		db_unset(m_hContact, Proto_GetBaseAccountName(m_hContact), "AvatarHash");
		DeleteAvatarFromCache(m_hContact, FALSE);

		QueueAdd(m_hContact);
		DestroyWindow(m_hwnd);
	}

	void onClick_Default(CCtrlButton *)
	{
		if (MessageBoxW(nullptr, TranslateT("Delete picture file from disk (may be necessary to force a reload, but will delete local pictures)?"), TranslateT("Reset contact picture"), MB_YESNO) == IDYES) {
			DBVARIANT dbv = { 0 };
			ProtectAvatar(m_hContact, 0);
			if (!db_get_ws(m_hContact, "ContactPhoto", "File", &dbv)) {
				DeleteFileW(dbv.pwszVal);
				db_free(&dbv);
			}
		}
		db_unset(m_hContact, "ContactPhoto", "Locked");
		db_unset(m_hContact, "ContactPhoto", "Backup");
		db_unset(m_hContact, "ContactPhoto", "RFile");
		db_unset(m_hContact, "ContactPhoto", "File");
		db_unset(m_hContact, "ContactPhoto", "Format");
		DeleteAvatarFromCache(m_hContact, FALSE);
		SetAvatarName();
		OnAvatarChanged(0, 0, 0);
	}

	void onChange_Protect(CCtrlCheck *)
	{
		ProtectAvatar(m_hContact, chkProtect.IsChecked());
	}

	void onChange_MakeTrans(CCtrlCheck *)
	{
		bool enable = chkMakeTrans.IsChecked();
		EnableWindow(GetDlgItem(m_hwnd, IDC_BKG_NUM_POINTS_L), enable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_BKG_NUM_POINTS_SPIN), enable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_BKG_NUM_POINTS), enable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_BKG_COLOR_DIFFERENCE_L), enable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_BKG_COLOR_DIFFERENCE_SPIN), enable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_BKG_COLOR_DIFFERENCE), enable);

		ReloadAvatar();
	}
};

INT_PTR ContactOptions(WPARAM wParam, LPARAM lParam)
{
	if (wParam) {
		auto *pDlg = new CContactAvatarDlg(wParam);
		if (lParam)
			pDlg->SetParent((HWND)lParam);
		pDlg->Create();
	}
	return 0;
}
