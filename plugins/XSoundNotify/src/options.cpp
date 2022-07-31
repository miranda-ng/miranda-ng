/*
Miranda plugin template, originally by Richard Hughes
http://miranda-icq.sourceforge.net/

This file is placed in the public domain. Anybody is free to use or
modify it as they wish with no restriction.
There is no warranty.
*/

#include "stdafx.h"

bool IsSuitableProto(PROTOACCOUNT *pa)
{
	if (pa == nullptr)
		return false;

	if (pa->bDynDisabled || !pa->bIsEnabled)
		return false;

	if (!(CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IMRECV))
		return false;

	return true;
}

class COptionsDlg : public CDlgBase
{
	CCtrlBase label;
	CCtrlCheck chkIgnore;
	CCtrlCombo comboAcc, comboUser;
	CCtrlButton btnTest, btnReset, btnChoose;

	XSN_Data* ObtainData()
	{
		LPARAM hContact = comboUser.GetCurData();
		if (hContact == -1) {
			PROTOACCOUNT *pa = (PROTOACCOUNT *)comboAcc.GetCurData();
			XSN_Data *p = XSN_Users.find((XSN_Data *)&pa->szModuleName);
			if (p == nullptr)
				XSN_Users.insert(p = new XSN_Data(pa->szModuleName, chkIgnore.GetState()));
			return p;
		}

		XSN_Data *p = XSN_Users.find((XSN_Data *)&hContact);
		if (p == nullptr)
			XSN_Users.insert(p = new XSN_Data(hContact, chkIgnore.GetState()));
		return p;
	}

public:
	COptionsDlg() :
		CDlgBase(g_plugin, IDD_OPTIONS),
		label(this, IDC_OPT_LABEL_SOUND),
		btnTest(this, IDC_OPT_BUTTON_TEST_PLAY),
		btnReset(this, IDC_OPT_BUTTON_RESET_SOUND),
		btnChoose(this, IDC_OPT_BUTTON_CHOOSE_SOUND),
		chkIgnore(this, IDC_OPT_IGNORE_SOUND),
		comboAcc(this, IDC_OPT_COMBO_PROTO),
		comboUser(this, IDC_OPT_COMBO_USERS)
	{
		btnTest.OnClick = Callback(this, &COptionsDlg::onClick_Test);
		btnReset.OnClick = Callback(this, &COptionsDlg::onClick_Reset);
		btnChoose.OnClick = Callback(this, &COptionsDlg::onClick_Choose);

		comboAcc.OnSelChanged = Callback(this, &COptionsDlg::onSelChanged_Proto);
		comboUser.OnSelChanged = Callback(this, &COptionsDlg::onSelChanged_Users);

		chkIgnore.OnChange = Callback(this, &COptionsDlg::onChange_Ignore);
	}

	bool OnInitDialog() override
	{
		for (auto &pa : Accounts())
			if (IsSuitableProto(pa))
				comboAcc.AddString(pa->tszAccountName, (LPARAM)pa);

		return true;
	}

	bool OnApply() override
	{
		for (auto &it : XSN_Users) {
			if (it->path[0] != 0) {
				wchar_t shortpath[MAX_PATH];
				PathToRelativeW(it->path, shortpath);
				if (it->bIsContact)
					g_plugin.setWString(it->hContact, SETTINGSKEY, shortpath);
				else
					g_plugin.setWString((LPCSTR)it->hContact, shortpath);
			}
			else g_plugin.delSetting(it->hContact, SETTINGSKEY);

			if (it->bIsContact)
				g_plugin.setByte(it->hContact, SETTINGSIGNOREKEY, it->bIgnore);
			else {
				size_t value_max_len = mir_strlen((const char *)it->hContact) + 8;
				char *value = (char *)mir_alloc(sizeof(char) * value_max_len);
				mir_snprintf(value, value_max_len, "%s_ignore", (const char *)it->hContact);
				g_plugin.setByte(value, it->bIgnore);
				mir_free(value);
			}
		}
		return true;
	}

	void onSelChanged_Proto(CCtrlCombo *)
	{
		comboUser.Enable();
		comboUser.ResetContent();

		btnTest.Disable();
		btnReset.Disable();
		chkIgnore.Disable();
		chkIgnore.SetState(false);
		label.SetText(TranslateT("Not set"));

		PROTOACCOUNT *pa = (PROTOACCOUNT *)comboAcc.GetCurData();
		comboUser.AddString(TranslateT("All contacts"), -1);

		for (auto &hContact : Contacts(pa->szModuleName)) {
			ptrW uid(Contact::GetInfo(CNF_UNIQUEID, hContact));
			CMStringW value(FORMAT, L"%s (%s)", Clist_GetContactDisplayName(hContact), uid.get());
			comboUser.AddString(value, hContact);
		}
	}

	void onSelChanged_Users(CCtrlCombo *)
	{
		btnChoose.Enable();
		chkIgnore.Enable();

		LPARAM hContact = comboUser.GetCurData();
		if (hContact == -1) {
			PROTOACCOUNT *pa = (PROTOACCOUNT *)comboAcc.GetCurData();
			ptrW wszText(g_plugin.getWStringA(pa->szModuleName));
			if (wszText) {
				btnTest.Enable();
				btnReset.Enable();
				label.SetText(PathFindFileNameW(wszText));
			}
			else {
				btnTest.Disable();
				btnReset.Disable();
				label.SetText(TranslateT("Not set"));
			}
			
			size_t value_max_len = mir_strlen(pa->szModuleName) + 8;
			char *value = (char *)mir_alloc(sizeof(char) * value_max_len);
			mir_snprintf(value, value_max_len, "%s_ignore", pa->szModuleName);
			chkIgnore.SetState(g_plugin.getByte(value, 0));
			mir_free(value);
		}
		else {
			ptrW wszPath(g_plugin.getWStringA(hContact, SETTINGSKEY));
			if (wszPath) {
				btnTest.Enable();
				btnReset.Enable();
				label.SetText(PathFindFileNameW(wszPath));
			}
			else {
				btnTest.Disable();
				btnReset.Disable();
				label.SetText(TranslateT("Not set"));
			}
			chkIgnore.SetState(g_plugin.getByte(hContact, SETTINGSIGNOREKEY, 0));
		}
	}

	void onClick_Test(CCtrlButton*)
	{
		isIgnoreSound = 0;

		auto *p = ObtainData();
		wchar_t longpath[MAX_PATH];
		PathToAbsoluteW(p->path, longpath);
		Skin_PlaySoundFile(longpath);
	}

	void onChange_Ignore(CCtrlCheck *)
	{
		if (!m_bInitialized)
			return;

		auto *p = ObtainData();
		p->bIgnore = chkIgnore.GetState();
	}

	void onClick_Choose(CCtrlButton *)
	{
		wchar_t tmp[MAX_PATH];
		if (GetModuleHandle(L"bass_interface.dll"))
			mir_snwprintf(tmp, L"%s (*.wav, *.mp3, *.ogg)%c*.wav;*.mp3;*.ogg%c%c", TranslateT("Sound files"), 0, 0, 0);
		else
			mir_snwprintf(tmp, L"%s (*.wav)%c*.wav%c%c", TranslateT("WAV files"), 0, 0, 0);

		wchar_t FileName[MAX_PATH]; FileName[0] = '\0';
		ptrW tszMirDir(Utils_ReplaceVarsW(L"%miranda_path%"));

		OPENFILENAME ofn = { 0 };
		ofn.lStructSize = sizeof(ofn);
		ofn.lpstrFilter = tmp;
		ofn.hwndOwner = nullptr;
		ofn.lpstrFile = FileName;
		ofn.nMaxFile = MAX_PATH;
		ofn.nMaxFileTitle = MAX_PATH;
		ofn.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
		ofn.lpstrInitialDir = tszMirDir;
		ofn.lpstrDefExt = L"";
		if (!GetOpenFileName(&ofn))
			return;

		label.SetText(PathFindFileNameW(FileName));

		auto *p = ObtainData();
		wcsncpy_s(p->path, FileName, _TRUNCATE);
		p->bIgnore = chkIgnore.GetState();

		btnTest.Enable();
		btnReset.Enable();
		NotifyChange();
	}

	void onClick_Reset(CCtrlButton *)
	{
		btnTest.Disable();
		btnReset.Disable();
		chkIgnore.SetState(false);
		label.SetText(TranslateT("Not set"));

		LPARAM hContact = comboUser.GetCurData();
		if (hContact == -1) {
			PROTOACCOUNT *pa = (PROTOACCOUNT *)comboAcc.GetCurData();
			XSN_Data *p = XSN_Users.find((XSN_Data *)&pa->szModuleName);
			if (p != nullptr) {
				XSN_Users.remove(p);
				delete p;
				NotifyChange();
			}
			g_plugin.delSetting(pa->szModuleName);
			size_t value_max_len = mir_strlen(pa->szModuleName) + 8;
			char *value = (char *)mir_alloc(sizeof(char) * value_max_len);
			mir_snprintf(value, value_max_len, "%s_ignore", pa->szModuleName);
			g_plugin.delSetting(value);
			mir_free(value);
		}
		else {
			XSN_Data *p = XSN_Users.find((XSN_Data *)&hContact);
			if (p != nullptr) {
				XSN_Users.remove(p);
				delete p;
				NotifyChange();
			}
			g_plugin.delSetting(hContact, SETTINGSKEY);
			g_plugin.delSetting(hContact, SETTINGSIGNOREKEY);
		}
	}
};

INT OptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 100000000;
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
	odp.szGroup.w = LPGENW("Sounds");
	odp.szTitle.w = LPGENW("XSound Notify");
	odp.pDialog = new COptionsDlg();
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
