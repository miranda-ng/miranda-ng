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

	QueueElem(const CMStringW &purl, const CMStringW &pfname, bool ne) :
		url(purl), fname(pfname), needext(ne)
	{
	}
};

static mir_cs csQueue;
static LIST<QueueElem> dlQueue(10);

static wchar_t g_wszCachePath[MAX_PATH];
static bool threadRunning;

static bool InternetDownloadFile(const char *szUrl, const wchar_t *szDest, HNETLIBCONN &hHttpDwnl)
{
	int result = 0xBADBAD;

	// initialize the netlib request
	MHttpRequest nlhr(REQUEST_GET);
	nlhr.flags = NLHRF_NODUMP | NLHRF_HTTP11 | NLHRF_PERSISTENT | NLHRF_REDIRECT;
	nlhr.m_szUrl = szUrl;
	nlhr.nlc = hHttpDwnl;

	// change the header so the plugin is pretended to be IE 6 + WinXP
	nlhr.AddHeader("User-Agent", NETLIB_USER_AGENT);
	nlhr.AddHeader("Connection", "close");

	CreatePathToFileW(szDest);

	while (result == 0xBADBAD) {
		// download the page
		NLHR_PTR nlhrReply(Netlib_DownloadFile(hNetlibUser, &nlhr, szDest));
		if (nlhrReply) {
			hHttpDwnl = nlhrReply->nlc;
			// if the recieved code is 200 OK
			if (nlhrReply->resultCode == 200)
				result = 0;
			else
				result = 1;
		}
		else {
			// retry
			hHttpDwnl = nullptr;
		}
	}

	return result == 0;
}

void __cdecl SmileyDownloadThread(void*)
{
	Thread_SetName("SmileyAdd: SmileyDownloadThread");

	bool needext = false;
	HNETLIBCONN hHttpDwnl = nullptr;

	while (!Miranda_IsTerminated()) {
		QueueElem *pItem = nullptr;
		{
			mir_cslock lck(csQueue);
			if (dlQueue.getCount()) {
				pItem = dlQueue[0];
				dlQueue.remove(int(0));
			}
		}

		if (pItem == nullptr) {
			SleepEx(3000, TRUE);
			continue;
		}

		if (_waccess(pItem->fname.c_str(), 0) != 0) {
			InternetDownloadFile(_T2A(pItem->url.c_str()), pItem->fname, hHttpDwnl);

			CMStringW fname(pItem->fname);
			if (pItem->needext) {
				fname += ProtoGetAvatarExtension(ProtoGetAvatarFileFormat(fname));
				needext = true;
			}
			_wrename(pItem->fname.c_str(), fname.c_str());
		}

		delete pItem;
	}

	for (auto &it : dlQueue)
		delete it;
	dlQueue.destroy();

	Netlib_CloseHandle(hHttpDwnl);
	threadRunning = false;

	if (!Miranda_IsTerminated()) {
		if (needext)
			CallServiceSync(MS_SMILEYADD_RELOAD, 0, 0);
		else
			NotifyEventHooks(g_hevOptionsChanged, 0, 0);
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

	{
		mir_cslock lck(csQueue);
		dlQueue.insert(new QueueElem(url, filename, iExtIdx == -1));
	}

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
	nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_NOHTTPSOPTION;
	nlu.szSettingsModule = MODULENAME;
	nlu.szDescriptiveName.a = MODULENAME;
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
}

void DownloadClose(void)
{
	Netlib_CloseHandle(hNetlibUser);
}
