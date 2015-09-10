#include "stdafx.h"

HANDLE CToxProto::hProfileFolderPath;

std::tstring CToxProto::GetToxProfilePath()
{
	return GetToxProfilePath(m_tszUserName);
}

std::tstring CToxProto::GetToxProfilePath(const TCHAR *accountName)
{
	TCHAR profilePath[MAX_PATH];
	TCHAR profileRootPath[MAX_PATH];
	FoldersGetCustomPathT(hProfileFolderPath, profileRootPath, _countof(profileRootPath), VARST(_T("%miranda_userdata%")));
	mir_sntprintf(profilePath, MAX_PATH, _T("%s\\%s.tox"), profileRootPath, accountName);
	return profilePath;
}

bool CToxProto::LoadToxProfile(Tox_Options *options)
{
	debugLogA(__FUNCTION__": loading tox profile");

	mir_cslock locker(profileLock);

	size_t size = 0;
	uint8_t *data = NULL;
	
	std::tstring profilePath = GetToxProfilePath();
	if (!IsFileExists(profilePath))
		return false;

	FILE *profile = _tfopen(profilePath.c_str(), _T("rb"));
	if (profile == NULL)
	{
		ShowNotification(TranslateT("Unable to open Tox profile"), MB_ICONERROR);
		debugLogA(__FUNCTION__": failed to open tox profile");
		return false;
	}

	fseek(profile, 0, SEEK_END);
	size = ftell(profile);
	rewind(profile);
	if (size < 0)
	{
		fclose(profile);
		size = 0;
	}

	data = (uint8_t*)mir_calloc(size);
	if (fread((char*)data, sizeof(char), size, profile) != size)
	{
		fclose(profile);
		ShowNotification(TranslateT("Unable to read Tox profile"), MB_ICONERROR);
		debugLogA(__FUNCTION__": failed to read tox profile");
		mir_free(data);
		return false;
	}
	fclose(profile);

	if (data && tox_is_data_encrypted(data))
	{
		pass_ptrA password(mir_utf8encodeW(pass_ptrT(getTStringA("Password"))));
		if (password == NULL || mir_strlen(password) == 0)
		{
			CToxPasswordEditor passwordEditor(this);
			if (!passwordEditor.DoModal())
			{
				mir_free(data);
				return false;
			}
		}
		uint8_t *encryptedData = (uint8_t*)mir_calloc(size - TOX_PASS_ENCRYPTION_EXTRA_LENGTH);
		TOX_ERR_DECRYPTION coreDecryptError;
		if (!tox_pass_decrypt(data, size, (uint8_t*)(char*)password, mir_strlen(password), encryptedData, &coreDecryptError))
		{
			ShowNotification(TranslateT("Unable to decrypt Tox profile"), MB_ICONERROR);
			debugLogA(__FUNCTION__": failed to decrypt tox profile (%d)", coreDecryptError);
			mir_free(data);
			return false;
		}
		mir_free(data);
		data = encryptedData;
		size -= TOX_PASS_ENCRYPTION_EXTRA_LENGTH;
	}

	if (data)
	{
		options->savedata_data = data;
		options->savedata_length = size;
		options->savedata_type = TOX_SAVEDATA_TYPE_TOX_SAVE;
		return true;
	}

	return false;
}

void CToxProto::SaveToxProfile()
{
	mir_cslock locker(profileLock);

	if (!toxThread)
		return;

	size_t size = tox_get_savedata_size(toxThread->tox);
	uint8_t *data = (uint8_t*)mir_calloc(size + TOX_PASS_ENCRYPTION_EXTRA_LENGTH);
	tox_get_savedata(toxThread->tox, data);

	pass_ptrA password(mir_utf8encodeW(pass_ptrT(getTStringA("Password"))));
	if (password && mir_strlen(password))
	{
		TOX_ERR_ENCRYPTION coreEncryptError;
		if (!tox_pass_encrypt(data, size, (uint8_t*)(char*)password, mir_strlen(password), data, &coreEncryptError))
		{
			debugLogA(__FUNCTION__": failed to encrypt tox profile");
			mir_free(data);
			return;
		}
		size += TOX_PASS_ENCRYPTION_EXTRA_LENGTH;
	}

	std::tstring profilePath = GetToxProfilePath();
	FILE *profile = _tfopen(profilePath.c_str(), _T("wb"));
	if (profile == NULL)
	{
		debugLogA(__FUNCTION__": failed to open tox profile");
		mir_free(data);
		return;
	}

	size_t written = fwrite(data, sizeof(char), size, profile);
	if (size != written)
	{
		debugLogA(__FUNCTION__": failed to write tox profile");
	}

	fclose(profile);
	mir_free(data);
}

INT_PTR CToxProto::OnCopyToxID(WPARAM, LPARAM)
{
	ptrA address(getStringA(TOX_SETTINGS_ID));
	size_t length = mir_strlen(address) + 1;
	if (OpenClipboard(NULL))
	{
		EmptyClipboard();
		HGLOBAL hMemory = GlobalAlloc(GMEM_FIXED, length);
		memcpy(GlobalLock(hMemory), address, length);
		GlobalUnlock(hMemory);
		SetClipboardData(CF_TEXT, hMemory);
		CloseClipboard();
	}
	return 0;
}

CToxPasswordEditor::CToxPasswordEditor(CToxProto *proto) :
	CToxDlgBase(proto, IDD_PASSWORD, false), ok(this, IDOK),
	password(this, IDC_PASSWORD), savePermanently(this, IDC_SAVEPERMANENTLY)
{
	ok.OnClick = Callback(this, &CToxPasswordEditor::OnOk);
}

void CToxPasswordEditor::OnOk(CCtrlButton*)
{
	pass_ptrT tszPassword(password.GetText());
	if (savePermanently.Enabled())
		m_proto->setTString("Password", tszPassword);

	EndDialog(m_hwnd, 1);
}
