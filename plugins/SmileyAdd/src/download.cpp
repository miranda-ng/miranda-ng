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

#include "general.h"

HANDLE hNetlibUser;
static HANDLE hFolder;

struct QueueElem
{
	CMString url;
	CMString fname;
	bool needext;

	QueueElem(CMString& purl, CMString& pfname, bool ne)
		: url(purl), fname(pfname), needext(ne) {}
};

static HANDLE g_hDlMutex;
static OBJLIST<QueueElem> dlQueue(10);

static TCHAR cachepath[MAX_PATH];
static bool threadRunning;

bool InternetDownloadFile(const char *szUrl, char* szDest, HANDLE &hHttpDwnl)
{
	int result = 0xBADBAD;
	char *szRedirUrl  = NULL;
	NETLIBHTTPREQUEST nlhr = {0};

	// initialize the netlib request
	nlhr.cbSize = sizeof(nlhr);
	nlhr.requestType = REQUEST_GET;
	nlhr.flags =  NLHRF_NODUMP | NLHRF_HTTP11 | NLHRF_PERSISTENT | NLHRF_REDIRECT;
	nlhr.szUrl = (char*)szUrl;
	nlhr.nlc = hHttpDwnl;

	// change the header so the plugin is pretended to be IE 6 + WinXP
	nlhr.headersCount = 2;
	nlhr.headers=(NETLIBHTTPHEADER*)alloca(sizeof(NETLIBHTTPHEADER)*nlhr.headersCount);
	nlhr.headers[0].szName   = "User-Agent";
	nlhr.headers[0].szValue = NETLIB_USER_AGENT;
	nlhr.headers[1].szName  = "Connection";
	nlhr.headers[1].szValue = "close";

	while (result == 0xBADBAD) {
		// download the page
		NETLIBHTTPREQUEST *nlhrReply = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)hNetlibUser,(LPARAM)&nlhr);
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
				for (int i=0; i<nlhrReply->headersCount; i++) {
					if (!mir_strcmp(nlhrReply->headers[i].szName, "Location")) {
						size_t rlen = 0;
						if (nlhrReply->headers[i].szValue[0] == '/') {
							const char* szPath;
							const char* szPref = strstr(szUrl, "://");
							szPref = szPref ? szPref + 3 : szUrl;
							szPath = strchr(szPref, '/');
							rlen = szPath != NULL ? szPath - szUrl : mir_strlen(szUrl);
						}

						szRedirUrl = (char*)mir_realloc(szRedirUrl, rlen + mir_strlen(nlhrReply->headers[i].szValue)*3 + 1);

						mir_strncpy(szRedirUrl, szUrl, rlen);
						mir_strcpy(szRedirUrl+rlen, nlhrReply->headers[i].szValue);

						nlhr.szUrl = szRedirUrl;
						break;
					}
				}
			}
			else result = 1;
		}
		else {
			hHttpDwnl = NULL;
			result = 1;
		}

		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT,0,(LPARAM)nlhrReply);
	}

	mir_free(szRedirUrl);

	return result == 0;
}

void __cdecl SmileyDownloadThread(void*)
{
	bool needext = false;
	HANDLE hHttpDwnl = NULL;
	WaitForSingleObject(g_hDlMutex, 3000);
	while (!Miranda_Terminated() && dlQueue.getCount()) {
		ReleaseMutex(g_hDlMutex);
		if (_taccess(dlQueue[0].fname.c_str(), 0) != 0) {
			InternetDownloadFile(T2A_SM(dlQueue[0].url.c_str()), T2A_SM(dlQueue[0].fname.c_str()), hHttpDwnl);
			WaitForSingleObject(g_hDlMutex, 3000);

			CMString fname(dlQueue[0].fname);
			if (dlQueue[0].needext) { fname += GetImageExt(fname); needext = true; }
			_trename(dlQueue[0].fname.c_str(), fname.c_str());
		}
		else WaitForSingleObject(g_hDlMutex, 3000);

		dlQueue.remove(0);
	}
	dlQueue.destroy();
	if (hHttpDwnl) Netlib_CloseHandle(hHttpDwnl);
	threadRunning = false;
	ReleaseMutex(g_hDlMutex);

	if (!Miranda_Terminated()) {
		if (needext)
			CallServiceSync(MS_SMILEYADD_RELOAD, 0, 0);
		else
			NotifyEventHooks(hEvent1, 0, 0);
	}
}

bool GetSmileyFile(CMString& url, const CMString& packstr)
{
	_TPattern *urlsplit = _TPattern::compile(_T(".*/(.*)"));
	_TMatcher *m0 = urlsplit->createTMatcher(url);

	m0->findFirstMatch();

	CMString filename;
	filename.AppendFormat(_T("%s\\%s\\"), cachepath, packstr.c_str());
	int pathpos = filename.GetLength();
	filename += m0->getGroup(1);

	delete m0;
	delete urlsplit;

	bool needext = filename.Find('.') == -1;
	if (needext)
		filename += _T(".*");

	_tfinddata_t c_file;
	INT_PTR hFile = _tfindfirst((TCHAR*)filename.c_str(), &c_file);
	if (hFile > -1) {
		_findclose(hFile);
		filename.Truncate(pathpos);
		filename += c_file.name;
		url = filename;
		return false;
	}
	if (needext)
		filename.Truncate(filename.GetLength() - 1);

	WaitForSingleObject(g_hDlMutex, 3000);
	dlQueue.insert(new QueueElem(url, filename, needext));
	ReleaseMutex(g_hDlMutex);

	if (!threadRunning) {
		threadRunning = true;
		mir_forkthread(SmileyDownloadThread, NULL);
	}

	url = filename;
	return false;
}

int FolderChanged(WPARAM, LPARAM)
{
	FoldersGetCustomPathT(hFolder, cachepath, MAX_PATH, _T(""));
	return 0;
}

void GetSmileyCacheFolder(void)
{
	hFolder = FoldersRegisterCustomPathT(LPGEN("SmileyAdd"), LPGEN("Smiley cache"), MIRANDA_USERDATAT _T("\\SmileyCache"));
	if (hFolder) {
		FoldersGetCustomPathT(hFolder, cachepath, MAX_PATH, _T(""));
		HookEvent(ME_FOLDERS_PATH_CHANGED, FolderChanged);
	}
	else mir_tstrncpy(cachepath, VARST( _T("%miranda_userdata%\\SmileyCache")), MAX_PATH);
}

void DownloadInit(void)
{
	NETLIBUSER nlu = { sizeof(nlu) };
	nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_NOHTTPSOPTION | NUF_TCHAR;
	nlu.szSettingsModule = "SmileyAdd";
	nlu.ptszDescriptiveName = TranslateT("SmileyAdd HTTP connections");
	hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	GetSmileyCacheFolder();
	g_hDlMutex = CreateMutex(NULL, FALSE, NULL);
}

void DownloadClose(void)
{
	CloseHandle(g_hDlMutex);
	Netlib_CloseHandle(hNetlibUser);
}
