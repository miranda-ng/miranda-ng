#include "common.h"

std::tstring CToxProto::GetToxProfilePath()
{
	std::tstring profilePath;
	TCHAR defaultPath[MAX_PATH];
	mir_sntprintf(defaultPath, MAX_PATH, _T("%s\\%s.tox"), VARST(_T("%miranda_userdata%")), m_tszUserName);
	profilePath = defaultPath;

	return profilePath;
}

void CToxProto::LoadToxProfile()
{
	std::tstring toxProfilePath = GetToxProfilePath();
	FILE *hFile = _wfopen(toxProfilePath.c_str(), _T("rb"));
	if (!hFile)
	{
		debugLogA("CToxProto::LoadToxData: could not open tox profile");
		return;
	}

	fseek(hFile, 0, SEEK_END);
	uint32_t size = ftell(hFile);
	rewind(hFile);
	if (size == 0)
	{
		debugLogA("CToxProto::LoadToxData: tox profile is empty");
		fclose(hFile);
		return;
	}

	uint8_t *data = (uint8_t*)mir_alloc(size);
	if (fread(data, sizeof(uint8_t), size, hFile) != size)
	{
		debugLogA("CToxProto::LoadToxData: could not read tox profile");
		fclose(hFile);
		mir_free(data);
		return;
	}

	if (tox_is_data_encrypted(data))
	{
		ptrT password(getTStringA("Password"));
		char *password_utf8 = mir_utf8encodeW(password);
		if (tox_encrypted_load(tox, data, size, (uint8_t*)password_utf8, strlen(password_utf8)) == TOX_ERROR)
		{
			debugLogA("CToxProto::LoadToxData: could not decrypt tox profile");
		}
		mir_free(password_utf8);
	}
	else
	{
		if (tox_load(tox, data, size) == TOX_ERROR)
		{
			debugLogA("CToxProto::LoadToxData: could not load tox profile");
		}
	}

	mir_free(data);
	fclose(hFile);
}

void CToxProto::SaveToxProfile()
{
	std::tstring toxProfilePath = GetToxProfilePath();
	FILE *hFile = _wfopen(toxProfilePath.c_str(), _T("wb"));
	if (!hFile)
	{
		debugLogA("CToxProto::LoadToxData: could not open tox profile");
		return;
	}

	uint32_t size;
	ptrT password(getTStringA("Password"));
	if (password && _tcslen(password))
		size = tox_encrypted_size(tox);
	else
		size = tox_size(tox);
	uint8_t *data = (uint8_t*)mir_alloc(size);
	if (password && _tcslen(password))
	{
		char *password_utf8 = mir_utf8encodeW(password);
		if (tox_encrypted_save(tox, data, (uint8_t*)password_utf8, strlen(password_utf8)) == TOX_ERROR)
		{
			debugLogA("CToxProto::LoadToxData: could not encrypt tox profile");
			mir_free(password_utf8);
			mir_free(data);
			fclose(hFile);
			return;
		}
		mir_free(password_utf8);
	}
	else
	{
		tox_save(tox, data);
	}

	if (fwrite(data, sizeof(uint8_t), size, hFile) != size)
	{
		debugLogA("CToxProto::LoadToxData: could not write tox profile");
	}

	mir_free(data);
	fclose(hFile);
}

INT_PTR CToxProto::ToxProfileManagerProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CToxProto *proto = (CToxProto*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	TCHAR *profilePath = (TCHAR*)GetWindowLongPtr(hwnd, DWLP_USER);

	switch (uMsg)
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		{
			proto = (CToxProto*)lParam;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);

			profilePath = (TCHAR*)mir_calloc(sizeof(TCHAR)*MAX_PATH);
			SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)profilePath);
		}
		return TRUE;

	case WM_CLOSE:
		{
			std::tstring defaultProfilePath = proto->GetToxProfilePath();
			fclose(_wfopen(defaultProfilePath.c_str(), _T("w")));
			EndDialog(hwnd, 0);
		}
		break;

	case WM_DESTROY:
		mir_free(profilePath);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_PROFILE_PATH:
			if ((HWND)lParam == GetFocus())
			{
				if (HIWORD(wParam) != EN_CHANGE) return 0;
				EnableWindow(GetDlgItem(hwnd, IDOK), TRUE);
			}
			break;

		case IDC_BROWSE_PROFILE:
			{
				TCHAR filter[MAX_PATH] = { 0 };
				mir_sntprintf(filter, MAX_PATH, _T("%s\0*.*"), TranslateT("All files (*.*)"));

				OPENFILENAME ofn = { sizeof(ofn) };
				ofn.hwndOwner = hwnd;
				ofn.lpstrFilter = filter;
				ofn.nFilterIndex = 1;
				ofn.lpstrFile = profilePath;
				ofn.lpstrTitle = TranslateT("Select tox profile");
				ofn.nMaxFile = MAX_PATH;
				ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER;

				if (GetOpenFileName(&ofn) && profilePath)
				{
					EnableWindow(GetDlgItem(hwnd, IDOK), TRUE);
					SetDlgItemText(hwnd, IDC_PROFILE_PATH, profilePath);
				}
			}
			break;

		case IDOK:
			{
				std::tstring defaultProfilePath = proto->GetToxProfilePath();
				if (profilePath && _tcslen(profilePath))
				{
					if (_tcsicmp(profilePath, defaultProfilePath.c_str()) != 0)
					{
						CopyFile(profilePath, defaultProfilePath.c_str(), FALSE);
					}
				}
				else
				{
					fclose(_wfopen(defaultProfilePath.c_str(), _T("w")));
				}
				EndDialog(hwnd, 1);
			}
			break;

		case IDCANCEL:
			{
				std::tstring defaultProfilePath = proto->GetToxProfilePath();
				fclose(_wfopen(defaultProfilePath.c_str(), _T("w")));
				EndDialog(hwnd, 0);
			}
			break;
		}
		break;
	}

	return FALSE;
}