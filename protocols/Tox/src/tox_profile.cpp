#include "stdafx.h"

HANDLE CToxProto::hProfileFolderPath;

wchar_t* CToxProto::GetToxProfilePath()
{
	return GetToxProfilePath(m_tszUserName);
}

wchar_t* CToxProto::GetToxProfilePath(const wchar_t *accountName)
{
	wchar_t *profilePath = (wchar_t*)mir_calloc(MAX_PATH * sizeof(wchar_t) + 1);
	wchar_t profileRootPath[MAX_PATH];
	FoldersGetCustomPathW(hProfileFolderPath, profileRootPath, _countof(profileRootPath), VARSW(L"%miranda_userdata%"));
	mir_snwprintf(profilePath, MAX_PATH, L"%s\\%s.tox", profileRootPath, accountName);
	return profilePath;
}

/////////////////////////////////////////////////////////////////////////////////////////
// ENTER PASSWORD 

class CToxEnterPasswordDlg : public CToxDlgBase
{
	CCtrlEdit m_password;

	CCtrlButton m_ok;

public:
	CToxEnterPasswordDlg(CToxProto *proto) :
		CToxDlgBase(proto, IDD_PASSWORD_ENTER),
		m_password(this, IDC_PASSWORD),
		m_ok(this, IDOK)
	{
		m_password.OnChange = Callback(this, &CToxEnterPasswordDlg::Password_OnChange);
	}

	bool OnInitDialog() override
	{
		m_ok.Disable();
		return true;
	}

	bool OnApply() override
	{
		m_proto->setWString(TOX_SETTINGS_PASSWORD, pass_ptrW(m_password.GetText()));
		return true;
	}

	void Password_OnChange(CCtrlBase *)
	{
		m_ok.Enable(GetWindowTextLength(m_password.GetHwnd()) != 0);
	}
};

static INT_PTR CALLBACK EnterPassword(void *param)
{
	CToxProto *proto = (CToxProto*)param;

	pass_ptrW password(proto->getWStringA(TOX_SETTINGS_PASSWORD));
	if (mir_wstrlen(password) == 0) {
		CToxEnterPasswordDlg passwordDlg(proto);
		if (!passwordDlg.DoModal())
			return 0;
		password = proto->getWStringA(TOX_SETTINGS_PASSWORD);
	}
	return (INT_PTR)password.detach();
}

bool CToxProto::LoadToxProfile(Tox_Options *options)
{
	debugLogA(__FUNCTION__": loading tox profile");

	mir_cslock lock(m_profileLock);

	ptrW profilePath(GetToxProfilePath());
	if (!IsFileExists(profilePath))
		return false;

	FILE *profile = _wfopen(profilePath, L"rb");
	if (profile == nullptr) {
		ShowNotification(TranslateT("Unable to open Tox profile"), MB_ICONERROR);
		debugLogA(__FUNCTION__": failed to open tox profile");
		return false;
	}

	fseek(profile, 0, SEEK_END);
	long size = ftell(profile);
	rewind(profile);
	if (size < 0) {
		fclose(profile);
		return false;
	}

	if (size == 0) {
		fclose(profile);
		return true;
	}

	uint8_t *data = (uint8_t*)mir_calloc(size);
	if (fread((char*)data, sizeof(char), size, profile) != (size_t)size) {
		fclose(profile);
		ShowNotification(TranslateT("Unable to read Tox profile"), MB_ICONERROR);
		debugLogA(__FUNCTION__": failed to read tox profile");
		mir_free(data);
		return false;
	}
	fclose(profile);

	if (tox_is_data_encrypted(data)) {
		pass_ptrA password(mir_utf8encodeW(pass_ptrW((wchar_t*)CallFunctionSync(EnterPassword, this))));
		if (mir_strlen(password) == 0) {
			mir_free(data);
			return false;
		}

		size_t decryptedSize = size - TOX_PASS_ENCRYPTION_EXTRA_LENGTH;
		uint8_t *decryptedData = (uint8_t*)mir_calloc(decryptedSize);
		TOX_ERR_DECRYPTION coreDecryptError;
		if (!tox_pass_decrypt(data, size, (uint8_t*)(char*)password, mir_strlen(password), decryptedData, &coreDecryptError)) {
			ShowNotification(TranslateT("Unable to decrypt Tox profile"), MB_ICONERROR);
			debugLogA(__FUNCTION__": failed to decrypt tox profile (%d)", coreDecryptError);
			delSetting(TOX_SETTINGS_PASSWORD);
			mir_free(data);
			return false;
		}
		mir_free(data);
		data = decryptedData;
		size = (long)decryptedSize;
	}

	if (data) {
		options->savedata_data = data;
		options->savedata_length = size;
		options->savedata_type = TOX_SAVEDATA_TYPE_TOX_SAVE;
		return true;
	}

	return false;
}

void CToxProto::SaveToxProfile(Tox *tox)
{
	mir_cslock lock(m_profileLock);

	size_t size = tox_get_savedata_size(tox);
	uint8_t *data = (uint8_t*)mir_calloc(size);
	tox_get_savedata(tox, data);

	pass_ptrA password(mir_utf8encodeW(pass_ptrW(getWStringA(TOX_SETTINGS_PASSWORD))));
	if (password && mir_strlen(password)) {
		TOX_ERR_ENCRYPTION coreEncryptError;
		size_t encryptedSize = size + TOX_PASS_ENCRYPTION_EXTRA_LENGTH;
		uint8_t *encryptedData = (uint8_t*)mir_calloc(encryptedSize);
		if (!tox_pass_encrypt(data, size, (uint8_t*)(char*)password, mir_strlen(password), encryptedData, &coreEncryptError)) {
			debugLogA(__FUNCTION__": failed to encrypt tox profile");
			mir_free(data);
			mir_free(encryptedData);
			return;
		}
		mir_free(data);
		data = encryptedData;
		size = encryptedSize;
	}

	ptrW profilePath(GetToxProfilePath());
	FILE *profile = _wfopen(profilePath, L"wb");
	if (profile == nullptr) {
		debugLogA(__FUNCTION__": failed to open tox profile");
		mir_free(data);
		return;
	}

	size_t written = fwrite(data, sizeof(char), size, profile);
	if (size != written)
		debugLogA(__FUNCTION__": failed to write tox profile");

	fclose(profile);
	mir_free(data);
}

void CToxProto::OnErase()
{
	ptrW profilePath(GetToxProfilePath());
	_wunlink(profilePath);
}

INT_PTR CToxProto::OnCopyToxID(WPARAM, LPARAM)
{
	Utils_ClipboardCopy(ptrW(getWStringA(TOX_SETTINGS_ID)));
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// CREATE PASSWORD

class CToxCreatePasswordDlg : public CToxDlgBase
{
	CCtrlEdit m_newPassword;
	CCtrlEdit m_confirmPassword;
	CCtrlBase m_passwordValidation;
	CCtrlButton m_ok;

public:
	CToxCreatePasswordDlg(CToxProto *proto) :
		CToxDlgBase(proto, IDD_PASSWORD_CREATE),
		m_ok(this, IDOK),
		m_newPassword(this, IDC_PASSWORD_NEW),
		m_confirmPassword(this, IDC_PASSWORD_CONFIRM),
		m_passwordValidation(this, IDC_PASSWORD_VALIDATION)
	{
		m_newPassword.OnChange = Callback(this, &CToxCreatePasswordDlg::Password_OnChange);
		m_confirmPassword.OnChange = Callback(this, &CToxCreatePasswordDlg::Password_OnChange);
	}

	bool OnInitDialog() override
	{
		LOGFONT lf;
		HFONT hFont = (HFONT)m_passwordValidation.SendMsg(WM_GETFONT, 0, 0);
		GetObject(hFont, sizeof(lf), &lf);
		lf.lfWeight = FW_BOLD;
		m_passwordValidation.SendMsg(WM_SETFONT, (WPARAM)CreateFontIndirect(&lf), 0);

		m_ok.Disable();
		return true;
	}

	bool OnApply() override
	{
		m_proto->setWString(TOX_SETTINGS_PASSWORD, pass_ptrW(m_newPassword.GetText()));
		m_proto->SaveToxProfile(m_proto->m_tox);
		return true;
	}

	void Password_OnChange(CCtrlBase *)
	{
		pass_ptrW newPassword(m_newPassword.GetText());
		if (mir_wstrlen(newPassword) == 0) {
			m_ok.Disable();
			m_passwordValidation.SetText(TranslateT("New password is empty"));
			return;
		}

		pass_ptrW confirmPassword(m_confirmPassword.GetText());
		if (mir_wstrcmp(newPassword, confirmPassword) != 0) {
			m_ok.Disable();
			m_passwordValidation.SetText(TranslateT("New password is not equal to confirmation"));
			return;
		}

		m_passwordValidation.SetText(L"");
		m_ok.Enable();
	}
};

INT_PTR CToxProto::OnCreatePassword(WPARAM, LPARAM)
{
	pass_ptrW password(getWStringA(TOX_SETTINGS_PASSWORD));
	CToxCreatePasswordDlg(this).DoModal();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// CHANGE PASSWORD

class CToxChangePasswordDlg : public CToxDlgBase
{
	CCtrlEdit m_oldPassword;

	CCtrlEdit m_newPassword;
	CCtrlEdit m_confirmPassword;

	CCtrlBase m_passwordValidation;

	CCtrlButton m_ok;

public:
	CToxChangePasswordDlg(CToxProto *proto) :
		CToxDlgBase(proto, IDD_PASSWORD_CHANGE),
		m_oldPassword(this, IDC_PASSWORD),
		m_newPassword(this, IDC_PASSWORD_NEW),
		m_confirmPassword(this, IDC_PASSWORD_CONFIRM),
		m_passwordValidation(this, IDC_PASSWORD_VALIDATION),
		m_ok(this, IDOK)
	{
		m_oldPassword.OnChange = Callback(this, &CToxChangePasswordDlg::Password_OnChange);
		m_newPassword.OnChange = Callback(this, &CToxChangePasswordDlg::Password_OnChange);
		m_confirmPassword.OnChange = Callback(this, &CToxChangePasswordDlg::Password_OnChange);
	}

	bool OnInitDialog() override
	{
		LOGFONT lf;
		HFONT hFont = (HFONT)m_passwordValidation.SendMsg(WM_GETFONT, 0, 0);
		GetObject(hFont, sizeof(lf), &lf);
		lf.lfWeight = FW_BOLD;
		m_passwordValidation.SendMsg(WM_SETFONT, (WPARAM)CreateFontIndirect(&lf), 0);

		m_ok.Disable();
		return true;
	}

	bool OnApply() override
	{
		m_proto->setWString(TOX_SETTINGS_PASSWORD, pass_ptrW(m_newPassword.GetText()));
		m_proto->SaveToxProfile(m_proto->m_tox);
		return true;
	}

	void Password_OnChange(CCtrlBase *)
	{
		pass_ptrW dbPassword(m_proto->getWStringA(TOX_SETTINGS_PASSWORD));
		pass_ptrW oldPassword(m_oldPassword.GetText());
		if (mir_wstrlen(dbPassword) > 0 && mir_wstrcmp(dbPassword, oldPassword) != 0) {
			m_ok.Disable();
			m_passwordValidation.SetText(TranslateT("Old password is not valid"));
			return;
		}

		pass_ptrW newPassword(m_newPassword.GetText());
		if (mir_wstrlen(newPassword) == 0) {
			m_ok.Disable();
			m_passwordValidation.SetText(TranslateT("New password is empty"));
			return;
		}

		pass_ptrW confirmPassword(m_confirmPassword.GetText());
		if (mir_wstrcmp(newPassword, confirmPassword) != 0) {
			m_ok.Disable();
			m_passwordValidation.SetText(TranslateT("New password is not equal to confirmation"));
			return;
		}

		m_passwordValidation.SetText(L"");
		m_ok.Enable();
	}
};

INT_PTR CToxProto::OnChangePassword(WPARAM, LPARAM)
{
	CToxChangePasswordDlg passwordDlg(this);
	passwordDlg.DoModal();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CToxProto::OnRemovePassword(WPARAM, LPARAM)
{
	const wchar_t *message = TranslateT("Removing the password will lead to decryption of the profile.\r\nAre you sure to remove password?");
	int result = MessageBox(nullptr, message, TranslateT("Remove password"), MB_YESNO | MB_ICONQUESTION);
	if (result == IDYES) {
		delSetting(TOX_SETTINGS_PASSWORD);
		SaveToxProfile(m_tox);
	}
	return 0;
}
