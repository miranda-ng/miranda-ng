#include "common.h"

std::tstring CToxProto::GetToxProfilePath()
{
	return GetToxProfilePath(m_tszUserName);
}

std::tstring CToxProto::GetToxProfilePath(const TCHAR *accountName)
{
	std::tstring profilePath;
	TCHAR defaultPath[MAX_PATH];
	mir_sntprintf(defaultPath, MAX_PATH, _T("%s\\%s.tox"), VARST(_T("%miranda_userdata%")), accountName);
	profilePath = defaultPath;

	return profilePath;
}

bool CToxProto::LoadToxProfile()
{
	std::tstring profilePath = GetToxProfilePath();
	if (!IsFileExists(profilePath))
	{
		return true;
	}

	FILE *profile = _tfopen(profilePath.c_str(), _T("rb"));
	if (profile == NULL)
	{
		debugLogA("CToxProto::LoadToxData: could not open tox profile");
		return false;
	}

	fseek(profile, 0, SEEK_END);
	size_t size = ftell(profile);
	rewind(profile);
	if (size == 0)
	{
		fclose(profile);
		debugLogA("CToxProto::LoadToxData: tox profile is empty");
		return true;
	}

	uint8_t *data = (uint8_t*)mir_calloc(size);
	if (fread((char*)data, sizeof(char), size, profile) != size)
	{
		fclose(profile);
		debugLogA("CToxProto::LoadToxData: could not read tox profile");
		mir_free(data);
		return false;
	}
	fclose(profile);

	if (tox_is_data_encrypted(data))
	{
		if (password == NULL || strlen(password) == 0)
		{
			if (!DialogBoxParam(
				g_hInstance,
				MAKEINTRESOURCE(IDD_PASSWORD),
				NULL,
				ToxProfilePasswordProc,
				(LPARAM)this))
			{
				return false;
			}
		}

		if (tox_encrypted_load(tox, data, size, (uint8_t*)password, mir_strlen(password)) == TOX_ERROR)
		{
			debugLogA("CToxProto::LoadToxData: could not decrypt tox profile");
			mir_free(data);
			return false;
		}
	}
	else
	{
		// it return -1 but load, wtf?
		if (tox_load(tox, data, size) > 0)
		{
			debugLogA("CToxProto::LoadToxData: could not load tox profile");
			mir_free(data);
			return false;
		}
	}

	mir_free(data);
	return true;
}

void CToxProto::SaveToxProfile()
{
	/*size_t size = 0;
	uint8_t *data = NULL;

	{
		mir_cslock lock(toxLock);

		if (password && strlen(password))
		{
			size = tox_encrypted_size(tox);
			data = (uint8_t*)mir_calloc(size);
			if (tox_encrypted_save(tox, data, (uint8_t*)password, strlen(password)) == TOX_ERROR)
			{
				debugLogA("CToxProto::LoadToxData: could not encrypt tox profile");
				mir_free(data);
				return;
			}
		}
		else
		{
			size = tox_size(tox);
			data = (uint8_t*)mir_calloc(size);
			tox_save(tox, data);
		}
	}*/

	size_t size = tox_size(tox);
	uint8_t *data = (uint8_t*)mir_calloc(size);
	tox_save(tox, data);

	std::tstring profilePath = GetToxProfilePath();
	FILE *profile = _tfopen(profilePath.c_str(), _T("wb"));
	if (profile == NULL)
	{
		debugLogA("CToxProto::LoadToxData: could not open tox profile");
		return;
	}

	size_t written = fwrite(data, sizeof(char), size, profile);
	if (size != written)
	{
		fclose(profile);
		debugLogA("CToxProto::LoadToxData: could not write tox profile");
	}

	fclose(profile);
	mir_free(data);
}

INT_PTR CToxProto::ToxProfilePasswordProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CToxProto *proto = (CToxProto*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (uMsg)
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		{
			proto = (CToxProto*)lParam;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			{
				TCHAR password[MAX_PATH];
				GetDlgItemText(hwnd, IDC_PASSWORD, password, SIZEOF(password));
				if (IsDlgButtonChecked(hwnd, IDC_SAVEPERMANENTLY))
				{
					proto->setTString("Password", password);
				}
				if (proto->password != NULL)
				{
					mir_free(proto->password);
				}
				proto->password = mir_utf8encodeW(password);

				EndDialog(hwnd, 1);
			}
			break;

		case IDCANCEL:
			EndDialog(hwnd, 0);
			break;
		}
		break;
	}

	return FALSE;
}