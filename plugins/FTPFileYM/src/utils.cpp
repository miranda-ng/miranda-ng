/*
FTP File YM plugin
Copyright (C) 2007-2010 Jan Holub

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

#include "common.h"

extern Options &opt;
extern UploadDialog *uDlg;

int Utils::getDeleteTimeMin()
{
	switch (opt.timeRange)
	{
		case (Options::TR_MINUTES):	return (opt.iDeleteTime);
		case (Options::TR_HOURS):	return (opt.iDeleteTime * 60);
		case (Options::TR_DAYS):	return (opt.iDeleteTime * 60 * 24);
	}

	return -1;
}

int Utils::msgBox(TCHAR *stzMsg, UINT uType)
{
	HWND hwnd = (uDlg != NULL) ? uDlg->hwnd : 0;
	return MessageBox(hwnd, stzMsg, TranslateT("FTP File"), uType);
}

int Utils::msgBoxA(char *szMsg, UINT uType)
{
	HWND hwnd = (uDlg != NULL) ? uDlg->hwnd : 0;
	return MessageBoxA(hwnd, szMsg, Translate("FTP File"), uType);
}

HICON Utils::loadIconEx(char *szName)
{
	char buff[100];
	mir_snprintf(buff, SIZEOF(buff), "%s_%s", MODULE, szName);
	return Skin_GetIcon(buff);
}

TCHAR *Utils::getFileNameFromPath(TCHAR *stzPath)
{
	TCHAR *pch = _tcsrchr(stzPath, '\\');
	if (pch) return pch + 1;
	else return _T("file.zip");
}

TCHAR *Utils::getTextFragment(TCHAR *stzText, size_t length, TCHAR *buff)
{
	if (mir_tstrlen(stzText) > length)
	{
		mir_tstrcpy(buff, stzText);
		buff[length - 1] = 0;
		mir_tstrcat(buff, _T("..."));
		return buff;
	}

	return stzText;
}

void Utils::copyToClipboard(char *szText)
{
	if (szText)
	{
		if (OpenClipboard(NULL))
		{
			EmptyClipboard();
			HGLOBAL hClipboardData = GlobalAlloc(GMEM_DDESHARE, 1024);
			char *pchData = (char *)GlobalLock(hClipboardData);
			mir_strcpy(pchData, szText);
			GlobalUnlock(hClipboardData);
			SetClipboardData(CF_TEXT, hClipboardData);
			CloseClipboard();
		}
	}
}

const char from_chars[] = "‡·‚„‰Â∏ÊÁËÈÍÎÏÌÓÔÒÚÛÙıˆ˜¯˘˙˚¸˝˛ˇ¿¡¬√ƒ≈®∆«»… ÀÃÕŒœ–—“”‘’÷◊ÿŸ⁄€‹›ﬁﬂ !@#$%^&=,{}[];'`";
const char to_chars[]	= "abvgdeezziiklmnoprstufhccwwqyqeuaABVGDEEZZIIKLMNOPRSTUFHCCWWQYQEUA_________________";

char* Utils::makeSafeString(TCHAR *input, char *output)
{
	char *buff = mir_t2a(input);
	size_t length = mir_strlen(buff);

	for (UINT i = 0; i < length; i++)
	{
		for (int j = 0; from_chars[j] != 0; j++)
		{
			if (buff[i] == from_chars[j])
			{
				buff[i] = to_chars[j];
				break;
			}
		}
	}

	mir_strcpy(output, buff);
	FREE(buff);

	return output;
}

void Utils::curlSetOpt(CURL *hCurl, ServerList::FTP *ftp, char *url, struct curl_slist *headerList, char *errorBuff)
{
	char buff[256];

	curl_easy_setopt(hCurl, CURLOPT_ERRORBUFFER, errorBuff);

	curl_easy_setopt(hCurl, CURLOPT_POSTQUOTE, headerList);
	curl_easy_setopt(hCurl, CURLOPT_NOPROGRESS, 1);

	curl_easy_setopt(hCurl, CURLOPT_URL, url);
	curl_easy_setopt(hCurl, CURLOPT_PORT, ftp->iPort);
	curl_easy_setopt(hCurl, CURLOPT_CONNECTTIMEOUT, 30);
	curl_easy_setopt(hCurl, CURLOPT_FTP_RESPONSE_TIMEOUT, 20);

	curl_easy_setopt(hCurl, CURLOPT_FTP_USE_EPRT, 0);
	curl_easy_setopt(hCurl, CURLOPT_FTP_USE_EPSV, 0);

	if (ftp->bPassive)
		curl_easy_setopt(hCurl, CURLOPT_FTPPORT, 0);
	else if (!DB::getAString(0, MODULE, "LocalIP", buff))
		curl_easy_setopt(hCurl, CURLOPT_FTPPORT, buff);
	else
		curl_easy_setopt(hCurl, CURLOPT_FTPPORT, "-");

	mir_snprintf(buff, SIZEOF(buff), "%s:%s", ftp->szUser, ftp->szPass);
	curl_easy_setopt(hCurl, CURLOPT_USERPWD, buff);

	if (ftp->ftpProto == ServerList::FTP::FT_SSL_EXPLICIT || ftp->ftpProto == ServerList::FTP::FT_SSL_IMPLICIT)
	{
		curl_easy_setopt(hCurl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
		curl_easy_setopt(hCurl, CURLOPT_FTPSSLAUTH, CURLFTPAUTH_DEFAULT);
		curl_easy_setopt(hCurl, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt(hCurl, CURLOPT_SSL_VERIFYHOST, 2);
	}
	else if (ftp->ftpProto == ServerList::FTP::FT_SSH)
	{
		curl_easy_setopt(hCurl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_PASSWORD);
	}
}

INT_PTR CALLBACK Utils::DlgProcSetFileName(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TCHAR *fileName = (TCHAR *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg)
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);
			fileName = (TCHAR *)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)fileName);
			SetDlgItemText(hwndDlg, IDC_NAME, fileName);

			if (GetDlgCtrlID((HWND)wParam) != IDC_NAME)
			{
				SetFocus(GetDlgItem(hwndDlg, IDC_NAME));
				SendDlgItemMessage(hwndDlg, IDC_NAME, EM_SETSEL, 0, mir_tstrlen(fileName) - 4);
				return FALSE;
			}

			return TRUE;
		}
		case WM_COMMAND:
		{
			if (HIWORD(wParam) == BN_CLICKED)
			{
				if (LOWORD(wParam) == IDOK)
				{
					GetDlgItemText(hwndDlg, IDC_NAME, fileName, 64);
					EndDialog(hwndDlg, IDOK);
				}
				else if (LOWORD(wParam) == IDCANCEL)
				{
					EndDialog(hwndDlg, IDCANCEL);
				}
			}

			break;
		}
	}

	return FALSE;
}

bool Utils::setFileNameDlg(TCHAR *nameBuff)
{
	if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DLG_NAME), 0, DlgProcSetFileName, (LPARAM)nameBuff) == IDOK)
		return true;
	else
		return false;
}

bool Utils::setFileNameDlgA(char *nameBuff)
{
	TCHAR buff[64];
	TCHAR *tmp = mir_a2t(nameBuff);
	mir_tstrcpy(buff, tmp);
	FREE(tmp);

	bool res = setFileNameDlg(buff);
	if (res)
	{
		char *p = mir_t2a(buff);
		mir_strcpy(nameBuff, p);
		FREE(p);
	}

	return res;
}

void Utils::createFileDownloadLink(char *szUrl, char *fileName, char *buff, int buffSize)
{
	if (szUrl[mir_strlen(szUrl) - 1] == '/')
		mir_snprintf(buff, buffSize, "%s%s", szUrl, fileName);
	else
		mir_snprintf(buff, buffSize, "%s/%s", szUrl, fileName);
}