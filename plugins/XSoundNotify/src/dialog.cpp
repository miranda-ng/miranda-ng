/*
Miranda plugin template, originally by Richard Hughes
http://miranda-icq.sourceforge.net/

This file is placed in the public domain. Anybody is free to use or
modify it as they wish with no restriction.
There is no warranty.
*/

#include "stdafx.h"

class CContactDlg : public CDlgBase
{
	XSN_Data *pData;

	CCtrlBase label;
	CCtrlCheck chkIgnore;
	CCtrlButton btnChoose, btnPlay, btnReset;

public:
	CContactDlg(LPARAM _h) :
		CDlgBase(g_plugin, IDD_CONTACTS),
		label(this, IDC_CONT_LABEL_SOUND),
		btnPlay(this, IDC_CONT_BUTTON_TEST_PLAY),
		btnReset(this, IDC_CONT_BUTTON_RESET_SOUND),
		btnChoose(this, IDC_CONT_BUTTON_CHOOSE_SOUND),
		chkIgnore(this, IDC_CONT_IGNORE_SOUND)
	{
		btnPlay.OnClick = Callback(this, &CContactDlg::onClick_Play);
		btnReset.OnClick = Callback(this, &CContactDlg::onClick_Reset);
		btnChoose.OnClick = Callback(this, &CContactDlg::onClick_Choose);

		chkIgnore.OnChange = Callback(this, &CContactDlg::onChange_Ignore);

		pData = XSN_Users.find((XSN_Data *)&_h);
		if (pData == nullptr)
			XSN_Users.insert(pData = new XSN_Data(_h, false));
		pData->bIgnore = g_plugin.getByte(_h, SETTINGSIGNOREKEY, 0);
	}

	bool OnInitDialog() override
	{
		WindowList_Add(hChangeSoundDlgList, m_hwnd, pData->hContact);
		Utils_RestoreWindowPositionNoSize(m_hwnd, pData->hContact, MODULENAME, "ChangeSoundDlg");

		ptrW uid(Contact::GetInfo(CNF_UNIQUEID, pData->hContact));
		wchar_t value[100];
		mir_snwprintf(value, TranslateT("Custom sound for %s (%s)"), Clist_GetContactDisplayName(pData->hContact), uid.get());
		SetWindowText(m_hwnd, value);

		DBVARIANT dbv = { 0 };
		if (!g_plugin.getWString(pData->hContact, SETTINGSKEY, &dbv)) {
			btnPlay.Enable();
			btnReset.Enable();
			label.SetText(PathFindFileNameW(dbv.pwszVal));
			db_free(&dbv);
		}
		else {
			btnPlay.Disable();
			btnReset.Disable();
			label.SetText(TranslateT("Not set"));
		}

		chkIgnore.SetState(pData->bIgnore);
		return true;
	}

	bool OnApply() override
	{
		if (pData->path[0] == 0) {
			g_plugin.delSetting(pData->hContact, SETTINGSKEY);
			g_plugin.delSetting(pData->hContact, SETTINGSIGNOREKEY);

			XSN_Users.remove(pData);
			delete pData;
		}
		else {
			wchar_t shortpath[MAX_PATH];
			PathToRelativeW(pData->path, shortpath);
			g_plugin.setWString(pData->hContact, SETTINGSKEY, shortpath);

			g_plugin.setByte(pData->hContact, SETTINGSIGNOREKEY, pData->bIgnore);
		}

		return true;
	}

	void OnDestroy() override
	{
		Utils_SaveWindowPosition(m_hwnd, pData->hContact, MODULENAME, "ChangeSoundDlg");
		WindowList_Remove(hChangeSoundDlgList, m_hwnd);
	}

	void onClick_Choose(CCtrlButton *)
	{
		wchar_t FileName[MAX_PATH];
		ptrW tszMirDir(Utils_ReplaceVarsW(L"%miranda_path%"));

		OPENFILENAME ofn = { 0 };
		ofn.lStructSize = sizeof(ofn);
		wchar_t tmp[MAX_PATH];
		if (GetModuleHandle(L"bass_interface.dll"))
			mir_snwprintf(tmp, L"%s (*.wav, *.mp3, *.ogg)%c*.wav;*.mp3;*.ogg%c%c", TranslateT("Sound files"), 0, 0, 0);
		else
			mir_snwprintf(tmp, L"%s (*.wav)%c*.wav%c%c", TranslateT("WAV files"), 0, 0, 0);
		ofn.lpstrFilter = tmp;
		ofn.hwndOwner = nullptr;
		ofn.lpstrFile = FileName;
		ofn.nMaxFile = MAX_PATH;
		ofn.nMaxFileTitle = MAX_PATH;
		ofn.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
		ofn.lpstrInitialDir = tszMirDir;
		*FileName = '\0';
		ofn.lpstrDefExt = L"";
		if (!GetOpenFileName(&ofn))
			return;

		label.SetText(PathFindFileNameW(FileName));

		wcsncpy_s(pData->path, FileName, _TRUNCATE);
		pData->bIgnore = chkIgnore.GetState();

		btnPlay.Enable();
		btnReset.Enable();
	}

	void onClick_Play(CCtrlButton*)
	{
		isIgnoreSound = 0;

		wchar_t longpath[MAX_PATH] = { 0 };
		PathToAbsoluteW(pData->path, longpath);
		Skin_PlaySoundFile(longpath);
	}

	void onClick_Reset(CCtrlButton *)
	{
		btnPlay.Disable();
		btnReset.Disable();
		chkIgnore.SetState(false);
		label.SetText(TranslateT("Not set"));

		pData->path[0] = 0;
		pData->bIgnore = false;
	}

	void onChange_Ignore(CCtrlCheck*)
	{
		pData->bIgnore = chkIgnore.GetState();
	}
};

INT_PTR ShowDialog(WPARAM wParam, LPARAM)
{
	HWND hChangeSoundDlg = WindowList_Find(hChangeSoundDlgList, wParam);
	if (!hChangeSoundDlg)
		(new CContactDlg(wParam))->Show();
	else {
		SetForegroundWindow(hChangeSoundDlg);
		SetFocus(hChangeSoundDlg);
	}
	return 0;
}
