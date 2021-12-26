/*
Miranda SmileyAdd Plugin
Copyright (C) 2007 - 2011 Boris Krasnovskiy

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

HNETLIBUSER hNetlibUser;
static HANDLE hFolderCache = 0, hFolderPacks = 0;

struct QueueElem
{
	CMStringW url;
	CMStringW fname;
	bool needext;

	QueueElem(CMStringW &purl, CMStringW &pfname, bool ne)
		: url(purl), fname(pfname), needext(ne)
	{
	}
};

static HANDLE g_hDlMutex;
static OBJLIST<QueueElem> dlQueue(10);

static wchar_t g_wszCachePath[MAX_PATH];
static bool threadRunning;

bool InternetDownloadFile(const char *szUrl, char *szDest, HNETLIBCONN &hHttpDwnl)
{
	int result = 0xBADBAD;
	char *szRedirUrl = nullptr;
	NETLIBHTTPREQUEST nlhr = {};

	// initialize the netlib request
	nlhr.cbSize = sizeof(nlhr);
	nlhr.requestType = REQUEST_GET;
	nlhr.flags = NLHRF_NODUMP | NLHRF_HTTP11 | NLHRF_PERSISTENT | NLHRF_REDIRECT;
	nlhr.szUrl = (char*)szUrl;
	nlhr.nlc = hHttpDwnl;

	// change the header so the plugin is pretended to be IE 6 + WinXP
	nlhr.headersCount = 2;
	nlhr.headers = (NETLIBHTTPHEADER*)alloca(sizeof(NETLIBHTTPHEADER)*nlhr.headersCount);
	nlhr.headers[0].szName = "User-Agent";
	nlhr.headers[0].szValue = NETLIB_USER_AGENT;
	nlhr.headers[1].szName = "Connection";
	nlhr.headers[1].szValue = "close";

	while (result == 0xBADBAD) {
		// download the page
		NLHR_PTR nlhrReply(Netlib_HttpTransaction(hNetlibUser, &nlhr));
		if (nlhrReply) {
			hHttpDwnl = nlhrReply->nlc;
			// if the recieved code is 200 OK
			if (nlhrReply->resultCode == 200) {
				char *delim = strrchr(szDest, '\\');
				if (delim) *delim = '\0';
				CreateDirectoryTree(szDest);
				if (delim) *delim = '\\';
				int res = -1;
				int fh = _open(szDest, _O_BINARY | _O_WRONLY | _O_CREAT, _S_IREAD | _S_IWRITE);
				if (fh != -1) {
					res = _write(fh, nlhrReply->pData, nlhrReply->dataLength);
					_close(fh);
				}
				if (res < 0)
					remove(szDest);
				else
					result = 0;
			}
			// if the recieved code is 302 Moved, Found, etc
			// workaround for url forwarding
			else if (nlhrReply->resultCode == 302 || nlhrReply->resultCode == 301 || nlhrReply->resultCode == 307) { // page moved
				// get the url for the new location and save it to szInfo
				// look for the reply header "Location"
				if (auto *pszUrl = Netlib_GetHeader(nlhrReply, "Location")) {
					size_t rlen = 0;
					if (pszUrl[0] == '/') {
						const char *szPref = strstr(szUrl, "://");
						szPref = szPref ? szPref + 3 : szUrl;
						const char *szPath = strchr(szPref, '/');
						rlen = szPath != nullptr ? szPath - szUrl : mir_strlen(szUrl);
					}

					szRedirUrl = (char *)mir_realloc(szRedirUrl, rlen + mir_strlen(pszUrl) * 3 + 1);

					strncpy(szRedirUrl, szUrl, rlen);
					mir_strcpy(szRedirUrl + rlen, pszUrl);

					nlhr.szUrl = szRedirUrl;
				}
			}
			else result = 1;
		}
		else {
			hHttpDwnl = nullptr;
			result = 1;
		}
	}

	mir_free(szRedirUrl);

	return result == 0;
}

void __cdecl SmileyDownloadThread(void*)
{
	Thread_SetName("SmileyAdd: SmileyDownloadThread");

	bool needext = false;
	HNETLIBCONN hHttpDwnl = nullptr;
	WaitForSingleObject(g_hDlMutex, 3000);
	while (!Miranda_IsTerminated() && dlQueue.getCount()) {
		ReleaseMutex(g_hDlMutex);
		if (_waccess(dlQueue[0].fname.c_str(), 0) != 0) {
			InternetDownloadFile(_T2A(dlQueue[0].url.c_str()), _T2A(dlQueue[0].fname.c_str()), hHttpDwnl);
			WaitForSingleObject(g_hDlMutex, 3000);

			CMStringW fname(dlQueue[0].fname);
			if (dlQueue[0].needext) {
				fname += ProtoGetAvatarExtension(ProtoGetAvatarFileFormat(fname));
				needext = true;
			}
			_wrename(dlQueue[0].fname.c_str(), fname.c_str());
		}
		else WaitForSingleObject(g_hDlMutex, 3000);

		dlQueue.remove(0);
	}
	dlQueue.destroy();
	Netlib_CloseHandle(hHttpDwnl);
	threadRunning = false;
	ReleaseMutex(g_hDlMutex);

	if (!Miranda_IsTerminated()) {
		if (needext)
			CallServiceSync(MS_SMILEYADD_RELOAD, 0, 0);
		else
			NotifyEventHooks(hEvent1, 0, 0);
	}
}

bool GetSmileyFile(CMStringW &url, const CMStringW &packstr)
{
	uint8_t hash[MIR_SHA1_HASH_SIZE];
	mir_sha1_hash((uint8_t*)url.c_str(), url.GetLength() * sizeof(wchar_t), hash);
	wchar_t wszHash[MIR_SHA1_HASH_SIZE * 2 + 1];
	bin2hexW(hash, sizeof(hash), wszHash);

	CMStringW filename;
	filename.AppendFormat(L"%s\\%s\\", g_wszCachePath, packstr.c_str());
	int pathpos = filename.GetLength();

	MRegexp16 urlsplit(L".*/(.*)");
	urlsplit.match(url);
	CMStringW urlFileName = urlsplit.getGroup(1);

	int iExtIdx = urlFileName.ReverseFind('.');
	if (iExtIdx != -1) {
		CMStringW wszExt = urlFileName.Mid(iExtIdx);
		if (ProtoGetAvatarFormat(wszExt) == PA_FORMAT_UNKNOWN)
			iExtIdx = -1;
		else
			filename += urlFileName;
	}

	if (iExtIdx == -1) {
		filename += wszHash;
		filename += L".*";
	}

	_wfinddata_t c_file;
	INT_PTR hFile = _wfindfirst(filename, &c_file);
	if (hFile > -1) {
		_findclose(hFile);
		filename.Truncate(pathpos);
		filename += c_file.name;
		url = filename;
		return false;
	}

	if (iExtIdx == -1) {
		filename.Truncate(pathpos);
		filename += wszHash;
		filename += L".";
	}

	WaitForSingleObject(g_hDlMutex, 3000);
	dlQueue.insert(new QueueElem(url, filename, iExtIdx == -1));
	ReleaseMutex(g_hDlMutex);

	if (!threadRunning) {
		threadRunning = true;
		mir_forkthread(SmileyDownloadThread);
	}

	url = filename;
	return false;
}

int FolderChanged(WPARAM, LPARAM)
{
	FoldersGetCustomPathW(hFolderCache, g_wszCachePath, MAX_PATH, L"");
	FoldersGetCustomPathW(hFolderPacks, g_plugin.wszDefaultPath, MAX_PATH, L"%miranda_path%");
	
	size_t len = mir_wstrlen(g_plugin.wszDefaultPath);
	if (len && g_plugin.wszDefaultPath[len - 1] != '\\')
		mir_wstrcpy(g_plugin.wszDefaultPath + len, L"\\");
	return 0;
}

void DownloadInit(void)
{
	NETLIBUSER nlu = {};
	nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_NOHTTPSOPTION | NUF_UNICODE;
	nlu.szSettingsModule = MODULENAME;
	nlu.szDescriptiveName.w = TranslateT("SmileyAdd HTTP connections");
	hNetlibUser = Netlib_RegisterUser(&nlu);

	hFolderPacks = FoldersRegisterCustomPathW(LPGEN("SmileyAdd"), LPGEN("Smiley packs' folder"), L"%miranda_path%");
	if (hFolderPacks) {
		hFolderCache = FoldersRegisterCustomPathW(LPGEN("SmileyAdd"), LPGEN("Smiley cache"), MIRANDA_USERDATAW L"\\SmileyCache");
		FolderChanged(0, 0);
		HookEvent(ME_FOLDERS_PATH_CHANGED, FolderChanged);
	}
	else {
		wcsncpy_s(g_wszCachePath, VARSW(L"%miranda_userdata%\\SmileyCache"), _TRUNCATE);
		wcsncpy_s(g_plugin.wszDefaultPath, VARSW(L"%miranda_path%\\"), _TRUNCATE);
	}

	g_hDlMutex = CreateMutex(nullptr, FALSE, nullptr);
}

void DownloadClose(void)
{
	CloseHandle(g_hDlMutex);
	Netlib_CloseHandle(hNetlibUser);
}
