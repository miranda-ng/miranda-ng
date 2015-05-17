/*
Paste It plugin
Copyright (C) 2011 Krzysztof Kral

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

#include "StdAfx.h"

PasteToWeb::PasteToWeb()
{
}


PasteToWeb::~PasteToWeb()
{
}

struct FromClipboardData
{
	std::wstring content;
	int page;
	std::wstring format;
};

struct FromFileData
{
	char* content;
	int contentLen;
	wchar_t* contentW;
	UINT codepage;
	std::wstring *fileName;
	int page;
	std::wstring format;
	bool sendFileName;
};

INT_PTR CALLBACK DlgProcFromClipboard(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
	{
		TranslateDialogDefault(hwndDlg);
		FromClipboardData* data = (FromClipboardData*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		int ts = 4;
		Edit_SetTabStops(GetDlgItem(hwndDlg, IDC_CLIPBOARD_DATA), 1, &ts);
		SetDlgItemText(hwndDlg, IDC_CLIPBOARD_DATA, data->content.c_str());
		int sel = 0;
		int i = 0;
		std::wstring &defFormat = Options::instance->webOptions[data->page]->defFormatId;
		HWND cb = GetDlgItem(hwndDlg, IDC_FORMAT);
		for (std::list<PasteFormat>::iterator it = Options::instance->webOptions[data->page]->formats.begin(); it != Options::instance->webOptions[data->page]->formats.end(); ++it)
		{
			ComboBox_AddString(cb, it->name.c_str());
			if (it->id == defFormat)
				sel = i;
			++i;
		}
		if (!Options::instance->webOptions[data->page]->formats.empty())
		{
			ComboBox_SetCurSel(cb, sel);
		}
		return TRUE;
	}
	case WM_COMMAND:
	{
		if (HIWORD(wParam) == BN_CLICKED)
		{
			if (LOWORD(wParam) == IDOK)
			{
				FromClipboardData *clipboardData = (FromClipboardData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				int sel = ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_FORMAT));
				for (std::list<PasteFormat>::iterator it = Options::instance->webOptions[clipboardData->page]->formats.begin(); it != Options::instance->webOptions[clipboardData->page]->formats.end(); ++it)
				{
					if (sel-- <= 0)
					{
						clipboardData->format = it->id;
						break;
					}
				}

				EndDialog(hwndDlg, IDC_BTN_OK);
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

void RecodeDlg(HWND hwndDlg)
{
	ShowWindow(GetDlgItem(hwndDlg, IDC_RECODE), SW_HIDE);
	FromFileData *fromFileData = (FromFileData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	unsigned int cp = Options::GetCodepageCB(GetDlgItem(hwndDlg, IDC_CODEPAGE), false, fromFileData->codepage);
	mir_free(fromFileData->contentW);
	int cbLen = 0;
	if (cp == 1200 || cp == 1201)
	{
		// UTF-16
		cbLen = fromFileData->contentLen / 2;
	}
	else
	{
		cbLen = MultiByteToWideChar(cp, 0, fromFileData->content, fromFileData->contentLen, NULL, 0);
	}

	fromFileData->contentW = (wchar_t*)mir_alloc(sizeof(wchar_t)*(cbLen + 1));
	if (fromFileData->contentW != NULL)
	{
		if (cp == 1200)
		{
			memcpy_s(fromFileData->contentW, sizeof(wchar_t)*(cbLen + 1), fromFileData->content, sizeof(wchar_t)*cbLen);
		}
		else if (cp == 1201)
		{
			for (int i = 0; i < cbLen; ++i)
			{
				fromFileData->contentW[i] = ((unsigned char)fromFileData->content[i * 2] << 8) | (unsigned char)fromFileData->content[i * 2 + 1];
			}
		}
		else
		{
			MultiByteToWideChar(cp, 0, fromFileData->content, fromFileData->contentLen, fromFileData->contentW, cbLen);
		}

		fromFileData->contentW[cbLen] = 0;
		SetDlgItemText(hwndDlg, IDC_FILE_DATA, fromFileData->contentW);
	}
	else
	{
		SetDlgItemText(hwndDlg, IDC_FILE_DATA, _T(""));
	}

	fromFileData->codepage = cp;
}

INT_PTR CALLBACK DlgProcFromFile(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
	{
		TranslateDialogDefault(hwndDlg);
		FromFileData *fromFileData = (FromFileData*)lParam;
		int ts = 4;
		Edit_SetTabStops(GetDlgItem(hwndDlg, IDC_FILE_DATA), 1, &ts);
		SetDlgItemText(hwndDlg, IDC_FILE_DATA, fromFileData->contentW);
		SetDlgItemText(hwndDlg, IDC_FILE_PATH, fromFileData->fileName->c_str());
		Options::InitCodepageCB(GetDlgItem(hwndDlg, IDC_CODEPAGE), fromFileData->codepage);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		ShowWindow(GetDlgItem(hwndDlg, IDC_RECODE), SW_HIDE);

		int sel = 0;
		int i = 0;
		std::wstring &defFormat = Options::instance->webOptions[fromFileData->page]->defFormatId;
		HWND cb = GetDlgItem(hwndDlg, IDC_FORMAT);
		for (std::list<PasteFormat>::iterator it = Options::instance->webOptions[fromFileData->page]->formats.begin(); it != Options::instance->webOptions[fromFileData->page]->formats.end(); ++it)
		{
			ComboBox_AddString(cb, it->name.c_str());
			if (it->id == defFormat)
				sel = i;
			++i;
		}
		if (!Options::instance->webOptions[fromFileData->page]->formats.empty())
		{
			ComboBox_SetCurSel(cb, sel);
		}

		if (Options::instance->webOptions[fromFileData->page]->isSendFileName)
		{
			bool sendFileName = Options::instance->webOptions[fromFileData->page]->sendFileName;
			CheckDlgButton(hwndDlg, IDC_AUTOFORMAT, sendFileName ? BST_CHECKED : BST_UNCHECKED);
			Button_Enable(GetDlgItem(hwndDlg, IDC_FORMAT), sendFileName ? 0 : 1);
			Button_Enable(GetDlgItem(hwndDlg, IDC_FORMATTEXT), sendFileName ? 0 : 1);
		}
		else
			Button_Enable(GetDlgItem(hwndDlg, IDC_AUTOFORMAT), FALSE);
		return TRUE;
	}
	case WM_COMMAND:
	{
		if (HIWORD(wParam) == BN_CLICKED)
		{
			if (LOWORD(wParam) == IDOK)
			{
				FromFileData *fromFileData = (FromFileData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				int sel = ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_FORMAT));
				for (std::list<PasteFormat>::iterator it = Options::instance->webOptions[fromFileData->page]->formats.begin(); it != Options::instance->webOptions[fromFileData->page]->formats.end(); ++it)
				{
					if (sel-- <= 0)
					{
						fromFileData->format = it->id;
						break;
					}
				}

				if (Options::instance->webOptions[fromFileData->page]->isSendFileName)
					fromFileData->sendFileName = IsDlgButtonChecked(hwndDlg, IDC_AUTOFORMAT) ? true : false;
				else
					fromFileData->sendFileName = false;

				EndDialog(hwndDlg, IDC_BTN_OK);
			}
			else if (LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hwndDlg, IDCANCEL);
			}
			else if (LOWORD(wParam) == IDC_RECODE)
			{
				RecodeDlg(hwndDlg);
			}
			else if (LOWORD(wParam) == IDC_AUTOFORMAT)
			{
				UINT sendFileName = IsDlgButtonChecked(hwndDlg, IDC_AUTOFORMAT);
				Button_Enable(GetDlgItem(hwndDlg, IDC_FORMAT), sendFileName ? 0 : 1);
				Button_Enable(GetDlgItem(hwndDlg, IDC_FORMATTEXT), sendFileName ? 0 : 1);
			}
		}
		else if (LOWORD(wParam) == IDC_CODEPAGE)
		{
			if (HIWORD(wParam) == CBN_SELCHANGE)
			{
				RecodeDlg(hwndDlg);
			}
			else if (HIWORD(wParam) == CBN_EDITCHANGE)
			{
				ShowWindow(GetDlgItem(hwndDlg, IDC_RECODE), SW_SHOW);
			}
			else if (HIWORD(wParam) == CBN_KILLFOCUS)
			{
				FromFileData *fromFileData = (FromFileData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				Options::GetCodepageCB(GetDlgItem(hwndDlg, IDC_CODEPAGE), true, fromFileData->codepage);
			}
		}

		break;
	}
	}

	return FALSE;
}

void PasteToWeb::FromClipboard()
{
	szFileLink[0] = 0;
	error = NULL;
	std::wstring str;
	BOOL isFile = 0;
	if (OpenClipboard(NULL))
	{
		HANDLE obj = GetClipboardData(CF_UNICODETEXT);
		if (obj != NULL)
		{
			LPCWSTR wStr = (LPCWSTR)GlobalLock(obj);
			str.append(wStr, wStr + mir_wstrlen(wStr));
			GlobalUnlock(obj);
			// Sometimes clipboard CF_UNICODETEXT format returns only 2 characters,
			// to fix this I check if CF_TEXT contains more characters,
			// if this is true, this mean that CF_UNICODETEXT is invalid.
			obj = GetClipboardData(CF_TEXT);
			if (obj != NULL)
			{
				LPCSTR cStr = (LPCSTR)GlobalLock(obj);
				if (strlen(cStr) > str.length())
				{
					str = L"";
					LPWSTR wStr = mir_a2u_cp(cStr, CP_ACP);
					str.append(wStr, wStr + mir_wstrlen(wStr));
					mir_free(wStr);
				}
				GlobalUnlock(obj);
			}
		}
		else
		{
			obj = GetClipboardData(CF_TEXT);
			if (obj != NULL)
			{
				LPCSTR cStr = (LPCSTR)GlobalLock(obj);
				LPWSTR wStr = mir_a2u_cp(cStr, CP_ACP);
				str.append(wStr, wStr + mir_wstrlen(wStr));
				mir_free(wStr);
				GlobalUnlock(obj);
			}
			else
			{
				obj = GetClipboardData(CF_HDROP);
				if (obj != NULL)
				{
					LPDROPFILES df = (LPDROPFILES)GlobalLock(obj);
					isFile = 1;
					if (df->fWide)
					{
						// Unicode
						WCHAR* file = (WCHAR*)((BYTE*)obj + df->pFiles);
						size_t len = mir_wstrlen(file);
						if (*(file + len + 1) == L'\0')
						{
							str.append(file, file + len);
						}
						else
						{
							error = TranslateT("You can only paste 1 file");
						}
					}
					else
					{
						// ANSI
						char* file = (char*)obj + df->pFiles;
						size_t len = mir_strlen(file);
						if (*(file + len + 1) == '\0')
						{
							LPWSTR wStr = mir_a2u_cp(file, CP_ACP);
							str.append(wStr, wStr + mir_wstrlen(wStr));
							mir_free(wStr);
						}
						else
						{
							error = TranslateT("You can only paste 1 file");
						}
					}
					GlobalUnlock(obj);
				}
			}
		}

		CloseClipboard();
	}

	if (str.length() > 0)
	{
		if (isFile)
			FromFile(str);
		else
		{
			FromClipboardData data;
			data.content = str;
			data.page = pageIndex;
			if (Options::instance->confDlg && DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DLG_FROM_CLIPBOARD), 0, DlgProcFromClipboard, (LPARAM)&data) != IDC_BTN_OK)
				return;

			SendToServer(str, L"", data.format);
		}
	}
	else if (error == NULL)
	{
		error = TranslateT("Cannot get data from clipboard");
	}
}

void PasteToWeb::FromFile(std::wstring file)
{
	error = NULL;
	szFileLink[0] = 0;
	HANDLE hFile = CreateFile(file.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	FromFileData fromFileData = { 0 };
	fromFileData.fileName = &file;
	if (hFile != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER fileSize;
		if (GetFileSizeEx(hFile, &fileSize))
		{
			if (fileSize.QuadPart <= 10485760LL)
			{
				if (fileSize.QuadPart > 512000LL)
				{
					mir_sntprintf(bufErr, SIZEOF(bufErr), TranslateT("File size is %d KB, do you really want to paste such a large file?"), fileSize.LowPart / 1024);
					if (MessageBox(NULL, bufErr, TranslateT("Are You sure?"), MB_YESNO | MB_ICONQUESTION) != IDYES)
					{
						CloseHandle(hFile);
						return;
					}
				}
				DWORD readed;
				fromFileData.contentLen = fileSize.LowPart;
				fromFileData.content = (char*)mir_alloc(fromFileData.contentLen);
				if (!ReadFile(hFile, fromFileData.content, fromFileData.contentLen, &readed, NULL))
				{
					mir_free(fromFileData.content);
					fromFileData.content = NULL;
					fromFileData.contentLen = 0;
					mir_sntprintf(bufErr, SIZEOF(bufErr), TranslateT("Cannot read file '%s'"), file.c_str());
					error = bufErr;
				}
			}
			else
			{
				error = TranslateT("File size is larger then 10 MB, cannot be sent");
			}
		}

		CloseHandle(hFile);
	}
	else
	{
		mir_sntprintf(bufErr, SIZEOF(bufErr), TranslateT("Cannot open file '%s'"), file.c_str());
		error = bufErr;
	}

	if (fromFileData.content != NULL)
	{
		int cbLen = 0;
		bool isDefTranslation = true;
		if (Options::instance->autoUTF)
		{
			isDefTranslation = false;
			fromFileData.codepage = CP_UTF8;
			cbLen = MultiByteToWideChar(fromFileData.codepage, MB_ERR_INVALID_CHARS, fromFileData.content, fromFileData.contentLen, NULL, 0);
			if (cbLen == 0)
			{
				int errorN = GetLastError();
				if (errorN == ERROR_NO_UNICODE_TRANSLATION)
				{
					isDefTranslation = true;
				}
			}
		}

		if (isDefTranslation)
		{
			fromFileData.codepage = Options::instance->codepage;
			if (fromFileData.codepage == 1200 || fromFileData.codepage == 1201)
			{
				// UTF-16
				cbLen = fromFileData.contentLen / 2;
			}
			else
			{
				cbLen = MultiByteToWideChar(fromFileData.codepage, 0, fromFileData.content, fromFileData.contentLen, NULL, 0);
			}
		}

		if (cbLen > 0)
		{
			fromFileData.contentW = (wchar_t*)mir_alloc(sizeof(wchar_t)*(cbLen + 1));
			if (fromFileData.contentW != NULL)
			{
				if (fromFileData.codepage == 1200)
				{
					memcpy_s(fromFileData.contentW, sizeof(wchar_t)*(cbLen + 1), fromFileData.content, sizeof(wchar_t)*cbLen);
				}
				else if (fromFileData.codepage == 1201)
				{
					for (int i = 0; i < cbLen; ++i)
					{
						fromFileData.contentW[i] = (fromFileData.content[i * 2] << 8) | fromFileData.content[i * 2 + 1];
					}
				}
				else
				{
					MultiByteToWideChar(fromFileData.codepage, 0, fromFileData.content, fromFileData.contentLen, fromFileData.contentW, cbLen);
				}

				fromFileData.contentW[cbLen] = 0;
				fromFileData.page = pageIndex;
				if (!Options::instance->confDlg || DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DLG_FROM_FILE), 0, DlgProcFromFile, (LPARAM)&fromFileData) == IDC_BTN_OK)
				{
					std::wstring fileName;
					std::wstring::size_type pos1 = file.find_last_of(L'\\');
					std::wstring::size_type pos2 = file.find_last_of(L'/');
					if (pos2 > pos1 && pos2 < file.length())
						pos1 = pos2;
					if (pos1 >= 0 && pos1 + 1 < file.length())
						fileName = file.substr(pos1 + 1);
					else
						fileName = file;
					SendToServer(fromFileData.contentW, fromFileData.sendFileName ? fileName : L"", fromFileData.format);
				}
				mir_free(fromFileData.contentW);
			}
		}
		else
		{
			mir_sntprintf(bufErr, SIZEOF(bufErr), TranslateT("File '%s' is empty"), file.c_str());
			error = bufErr;
		}
		mir_free(fromFileData.content);
	}
}

extern HANDLE g_hNetlibUser;

wchar_t* PasteToWeb::SendToWeb(char* url, std::map<std::string, std::string>& headers, std::wstring content)
{
	wchar_t* resCont = NULL;
	int cbLen = WideCharToMultiByte(CP_UTF8, 0, content.c_str(), -1, NULL, 0, NULL, NULL);
	char* contentBytes = (char*)mir_alloc(cbLen);
	if (contentBytes == NULL)
		return resCont;

	WideCharToMultiByte(CP_UTF8, 0, content.c_str(), -1, contentBytes, cbLen, NULL, NULL);
	--cbLen;

	int nHeaders = 0;
	for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
	{
		++nHeaders;
	}

	NETLIBHTTPREQUEST 	nlhr = { 0 };
	NETLIBHTTPHEADER*	httpHeaders = new NETLIBHTTPHEADER[nHeaders];
	nlhr.cbSize = sizeof(nlhr);
	nlhr.requestType = REQUEST_POST;
	nlhr.flags = NLHRF_NODUMPSEND | NLHRF_DUMPASTEXT | NLHPIF_HTTP11;
	nlhr.szUrl = url;
	nlhr.headers = httpHeaders;
	nlhr.pData = contentBytes;
	nlhr.dataLength = cbLen;
	nHeaders = 0;
	std::list<char*> mallBuf;
	for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
	{
		char* b1 = new char[it->first.length() + 1];
		char* b2 = new char[it->second.length() + 1];
		mir_strncpy(b1, it->first.c_str(), it->first.length() + 1);
		mir_strncpy(b2, it->second.c_str(), it->second.length() + 1);
		httpHeaders[nHeaders].szName = b1;
		httpHeaders[nHeaders].szValue = b2;
		mallBuf.push_back(b1);
		mallBuf.push_back(b2);
		++nHeaders;
	}

	nlhr.headersCount = nHeaders;
	NETLIBHTTPREQUEST* nlhrReply = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)g_hNetlibUser, (LPARAM)&nlhr);
	if (nlhrReply != NULL)
	{
		if (nlhrReply->resultCode == 200)
		{
			int resLen = MultiByteToWideChar(CP_UTF8, 0, nlhrReply->pData, nlhrReply->dataLength, NULL, 0);
			++resLen;
			resCont = (wchar_t*)mir_alloc(resLen * sizeof(wchar_t));
			if (resCont != NULL)
			{
				resLen = MultiByteToWideChar(CP_UTF8, 0, nlhrReply->pData, nlhrReply->dataLength, resCont, resLen);
				resCont[resLen] = 0;
			}
		}

		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)nlhrReply);
	}
	delete httpHeaders;
	for (std::list<char*>::iterator it = mallBuf.begin(); it != mallBuf.end(); ++it)
	{
		delete *it;
	}

	mir_free(contentBytes);
	return resCont;
}
