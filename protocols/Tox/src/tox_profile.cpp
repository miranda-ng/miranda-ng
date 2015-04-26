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
	FoldersGetCustomPathT(hProfileFolderPath, profileRootPath, SIZEOF(profileRootPath), VARST(_T("%miranda_userdata%")));
	mir_sntprintf(profilePath, MAX_PATH, _T("%s\\%s.tox"), profileRootPath, accountName);
	return profilePath;
}

bool CToxProto::LoadToxProfile(Tox_Options *options)
{
	debugLogA(__FUNCTION__": loading tox profile");

	size_t size = 0;
	uint8_t *data = NULL;
	
	std::tstring profilePath = GetToxProfilePath();
	if (!IsFileExists(profilePath))
		return false;

	FILE *profile = _tfopen(profilePath.c_str(), _T("rb"));
	if (profile == NULL)
	{
		debugLogA(__FUNCTION__": failed to open tox profile");
		return false;
	}

	fseek(profile, 0, SEEK_END);
	size = ftell(profile);
	rewind(profile);
	if (size == 0)
	{
		fclose(profile);
		debugLogA(__FUNCTION__": tox profile is empty");
		return true;
	}

	data = (uint8_t*)mir_calloc(size);
	if (fread((char*)data, sizeof(char), size, profile) != size)
	{
		fclose(profile);
		debugLogA(__FUNCTION__": failed to read tox profile");
		mir_free(data);
		return false;
	}
	fclose(profile);

	if (data != NULL && tox_is_data_encrypted(data))
	{
		password = mir_utf8encodeW(ptrT(getTStringA("Password")));
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
		if (!tox_pass_decrypt(data, size, (uint8_t*)password, mir_strlen(password), encryptedData, &coreDecryptError))
		{
			debugLogA(__FUNCTION__": failed to load tox profile (%d)", coreDecryptError);
			mir_free(data);
			return false;
		}
		mir_free(data);
		data = encryptedData;
		size -= TOX_PASS_ENCRYPTION_EXTRA_LENGTH;
	}

	TOX_ERR_NEW initError;
	tox = tox_new(options, data, size, &initError);
	if (initError != TOX_ERR_NEW_OK)
	{
		debugLogA(__FUNCTION__": failed to load tox profile (%d)", initError);
		mir_free(data);
		return false;
	}
	mir_free(data);
	debugLogA(__FUNCTION__": tox profile load successfully");
	return true;
}

void CToxProto::SaveToxProfile()
{
	size_t size = tox_get_savedata_size(tox);
	uint8_t *data = (uint8_t*)mir_calloc(size + TOX_PASS_ENCRYPTION_EXTRA_LENGTH);
	tox_get_savedata(tox, data);

	size_t passwordLen = mir_strlen(password);
	if (password && passwordLen)
	{
		TOX_ERR_ENCRYPTION coreEncryptError;
		if (!tox_pass_encrypt(data, size, (uint8_t*)password, passwordLen, data, &coreEncryptError))
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
	ptrT tszPassword(password.GetText());
	if (savePermanently.Enabled())
		m_proto->setTString("Password", tszPassword);
	if (m_proto->password != NULL)
		mir_free(m_proto->password);
	m_proto->password = mir_utf8encodeW(tszPassword);

	EndDialog(m_hwnd, 1);
}
