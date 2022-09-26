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

#include "stdafx.h"

int Utils::getDeleteTimeMin()
{
	switch (opt.timeRange) {
	case (Options::TR_MINUTES): return (opt.iDeleteTime);
	case (Options::TR_HOURS): return (opt.iDeleteTime * 60);
	case (Options::TR_DAYS): return (opt.iDeleteTime * 60 * 24);
	}

	return -1;
}

int Utils::msgBox(wchar_t *stzMsg, UINT uType)
{
	HWND hwnd = (uDlg != nullptr) ? uDlg->m_hwnd : nullptr;
	return MessageBox(hwnd, stzMsg, TranslateT("FTP File"), uType);
}

int Utils::msgBoxA(char *szMsg, UINT uType)
{
	HWND hwnd = (uDlg != nullptr) ? uDlg->m_hwnd : nullptr;
	return MessageBoxA(hwnd, szMsg, Translate("FTP File"), uType);
}

HICON Utils::loadIconEx(const char *szName)
{
	char buff[100];
	mir_snprintf(buff, "%s_%s", MODULENAME, szName);
	return IcoLib_GetIcon(buff);
}

HANDLE Utils::getIconHandle(const char *szName)
{
	char buff[100];
	mir_snprintf(buff, "%s_%s", MODULENAME, szName);
	return IcoLib_GetIconHandle(buff);
}

wchar_t* Utils::getFileNameFromPath(wchar_t *stzPath)
{
	wchar_t *pch = wcsrchr(stzPath, '\\');
	if (pch) return pch + 1;
	else return L"file.zip";
}

void Utils::copyToClipboard(char *szText)
{
	Utils_ClipboardCopy(szText);
}

void Utils::curlSetOpt(CURL *hCurl, ServerList::FTP *ftp, char *url, struct curl_slist *headerList, char *errorBuff)
{
	char buff[256];

	curl_easy_setopt(hCurl, CURLOPT_ERRORBUFFER, errorBuff);

	curl_easy_setopt(hCurl, CURLOPT_POSTQUOTE, headerList);
	curl_easy_setopt(hCurl, CURLOPT_NOPROGRESS, 1);

	curl_easy_setopt(hCurl, CURLOPT_URL, url);
	curl_easy_setopt(hCurl, CURLOPT_PORT, ftp->m_iPort);
	curl_easy_setopt(hCurl, CURLOPT_CONNECTTIMEOUT, 30);
	curl_easy_setopt(hCurl, CURLOPT_FTP_RESPONSE_TIMEOUT, 20);

	curl_easy_setopt(hCurl, CURLOPT_FTP_USE_EPRT, 0);
	curl_easy_setopt(hCurl, CURLOPT_FTP_USE_EPSV, 0);

	if (ftp->m_bPassive)
		curl_easy_setopt(hCurl, CURLOPT_FTPPORT, 0);
	else {
		ptrA localip(g_plugin.getStringA("LocalIP"));
		if (localip)
			curl_easy_setopt(hCurl, CURLOPT_FTPPORT, localip);
		else
			curl_easy_setopt(hCurl, CURLOPT_FTPPORT, "-");
	}

	mir_snprintf(buff, "%s:%s", ftp->m_szUser, ftp->m_szPass);
	curl_easy_setopt(hCurl, CURLOPT_USERPWD, buff);

	if (ftp->m_ftpProto == ServerList::FTP::FT_SSL_EXPLICIT || ftp->m_ftpProto == ServerList::FTP::FT_SSL_IMPLICIT) {
		curl_easy_setopt(hCurl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
		curl_easy_setopt(hCurl, CURLOPT_FTPSSLAUTH, CURLFTPAUTH_DEFAULT);
		curl_easy_setopt(hCurl, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt(hCurl, CURLOPT_SSL_VERIFYHOST, 2);
	}
	else if (ftp->m_ftpProto == ServerList::FTP::FT_SSH) {
		curl_easy_setopt(hCurl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_PASSWORD);
	}
}

INT_PTR CALLBACK Utils::DlgProcSetFileName(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	wchar_t *fileName = (wchar_t *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		fileName = (wchar_t *)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)fileName);
		SetDlgItemText(hwndDlg, IDC_NAME, fileName);

		if (GetDlgCtrlID((HWND)wParam) != IDC_NAME) {
			SetFocus(GetDlgItem(hwndDlg, IDC_NAME));
			SendDlgItemMessage(hwndDlg, IDC_NAME, EM_SETSEL, 0, mir_wstrlen(fileName) - 4);
			return FALSE;
		}

		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED) {
			if (LOWORD(wParam) == IDOK) {
				GetDlgItemText(hwndDlg, IDC_NAME, fileName, 64);
				EndDialog(hwndDlg, IDOK);
			}
			else if (LOWORD(wParam) == IDCANCEL) {
				EndDialog(hwndDlg, IDCANCEL);
			}
		}

		break;
	}

	return FALSE;
}

bool Utils::setFileNameDlg(wchar_t *nameBuff)
{
	if (DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_DLG_NAME), nullptr, DlgProcSetFileName, (LPARAM)nameBuff) == IDOK)
		return true;
	else
		return false;
}

void Utils::createFileDownloadLink(char *szUrl, char *fileName, char *buff, int buffSize)
{
	if (szUrl[mir_strlen(szUrl) - 1] == '/')
		mir_snprintf(buff, buffSize, "%s%s", szUrl, fileName);
	else
		mir_snprintf(buff, buffSize, "%s/%s", szUrl, fileName);
}
