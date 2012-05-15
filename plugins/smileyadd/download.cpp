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
#include "m_smileyadd.h"
#include "m_folders.h"

extern HANDLE hEvent1;
HANDLE hNetlibUser;
static HANDLE hFolder, hFolderHook;

struct QueueElem
{
	bkstring url;
	bkstring fname;
	bool needext;

	QueueElem(bkstring& purl, bkstring& pfname, bool ne) 
		: url(purl), fname(pfname), needext(ne) {}
};

static HANDLE g_hDlMutex;
static OBJLIST<QueueElem> dlQueue(10);

static TCHAR cachepath[MAX_PATH];
static bool threadRunning;

bool InternetDownloadFile(const char *szUrl, char* szDest, HANDLE &hHttpDwnl) 
{
	int result = 0xBADBAD;
	char* szRedirUrl  = NULL;
	NETLIBHTTPREQUEST nlhr = {0};

	// initialize the netlib request
	nlhr.cbSize = sizeof(nlhr);
	nlhr.requestType = REQUEST_GET;
	nlhr.flags =  NLHRF_NODUMP;
	nlhr.szUrl = (char*)szUrl;
	nlhr.nlc = hHttpDwnl;

	if (CallService(MS_SYSTEM_GETVERSION, 0, 0) >= PLUGIN_MAKE_VERSION(0,9,0,5))
		nlhr.flags |= NLHRF_HTTP11 | NLHRF_PERSISTENT | NLHRF_REDIRECT;

	// change the header so the plugin is pretended to be IE 6 + WinXP
	nlhr.headersCount = 2;
	nlhr.headers=(NETLIBHTTPHEADER*)alloca(sizeof(NETLIBHTTPHEADER)*nlhr.headersCount);
	nlhr.headers[0].szName   = "User-Agent";
	nlhr.headers[0].szValue = "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1)";
	nlhr.headers[1].szName  = "Connection";
	nlhr.headers[1].szValue = "close";

	while (result == 0xBADBAD)
	{
		// download the page
		NETLIBHTTPREQUEST *nlhrReply = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION,
			(WPARAM)hNetlibUser,(LPARAM)&nlhr);

		if (nlhrReply) 
		{
			hHttpDwnl = nlhrReply->nlc;
			// if the recieved code is 200 OK
			if(nlhrReply->resultCode == 200) 
			{
				char* delim = strrchr(szDest, '\\');
				if (delim) *delim = '\0';
				CallService(MS_UTILS_CREATEDIRTREE, 0, (LPARAM)szDest);
				if (delim) *delim = '\\';
				int res = -1;
				int fh = _open(szDest, _O_BINARY | _O_WRONLY | _O_CREAT, _S_IREAD | _S_IWRITE);
				if (fh != -1) 
				{
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
			else if(nlhrReply->resultCode == 302 || nlhrReply->resultCode == 301 || nlhrReply->resultCode == 307) // page moved
			{
				// get the url for the new location and save it to szInfo
				// look for the reply header "Location"
				for (int i=0; i<nlhrReply->headersCount; i++) 
				{
					if (!strcmp(nlhrReply->headers[i].szName, "Location")) 
					{
						size_t rlen = 0;
						if (nlhrReply->headers[i].szValue[0] == '/')
						{
							const char* szPath;
							const char* szPref = strstr(szUrl, "://");
							szPref = szPref ? szPref + 3 : szUrl;
							szPath = strchr(szPref, '/');
							rlen = szPath != NULL ? szPath - szUrl : strlen(szUrl); 
						}

						szRedirUrl = (char*)mir_realloc(szRedirUrl, 
							rlen + strlen(nlhrReply->headers[i].szValue)*3 + 1);

						strncpy(szRedirUrl, szUrl, rlen);
						strcpy(szRedirUrl+rlen, nlhrReply->headers[i].szValue); 

						nlhr.szUrl = szRedirUrl;
						break;
					}
				}
			}
			else 
				result = 1;
		}
		else 
		{
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
	while (!Miranda_Terminated() && dlQueue.getCount())
	{
		ReleaseMutex(g_hDlMutex);
		if (_taccess(dlQueue[0].fname.c_str(), 0) != 0)
		{
			InternetDownloadFile(T2A_SM(dlQueue[0].url.c_str()), T2A_SM(dlQueue[0].fname.c_str()), hHttpDwnl);
			WaitForSingleObject(g_hDlMutex, 3000);

			bkstring fname(dlQueue[0].fname);
			if (dlQueue[0].needext) { fname += GetImageExt(fname); needext = true; }
			_trename(dlQueue[0].fname.c_str(), fname.c_str());
		}
		else
			WaitForSingleObject(g_hDlMutex, 3000);

		dlQueue.remove(0);
	}
	dlQueue.destroy();
	if (hHttpDwnl) Netlib_CloseHandle(hHttpDwnl);
	threadRunning = false;
	ReleaseMutex(g_hDlMutex);

	if (!Miranda_Terminated())
	{
		if (needext)
			CallServiceSync(MS_SMILEYADD_RELOAD, 0, 0);
		else
			NotifyEventHooks(hEvent1, 0, 0);
	}
}


bool GetSmileyFile(bkstring& url, const bkstring& packstr)
{
	_TPattern * urlsplit = _TPattern::compile(_T(".*/(.*)"));
	_TMatcher * m0 = urlsplit->createTMatcher(url);

	m0->findFirstMatch();

	bkstring filename;
	filename.appendfmt(_T("%s\\%s\\"), cachepath, packstr.c_str());
	size_t pathpos = filename.size();
	filename += m0->getGroup(1);

	delete m0;
	delete urlsplit;

	bool needext = filename.find('.') == filename.npos;
	if (needext) filename += _T(".*");

	_tfinddata_t c_file;
	INT_PTR hFile = _tfindfirst((TCHAR*)filename.c_str(), &c_file);
	if (hFile > -1) 
	{
		_findclose(hFile);
		filename.erase(pathpos);
		filename += c_file.name;
		url = filename;
		return false;
	}
	if (needext) filename.erase(filename.size()-1);

	WaitForSingleObject(g_hDlMutex, 3000);
	dlQueue.insert(new QueueElem(url, filename, needext));
	ReleaseMutex(g_hDlMutex);

	if (!threadRunning)
	{
		threadRunning = true;
		mir_forkthread(SmileyDownloadThread, NULL);
	}

	url = filename;
	return false;
}

void GetDefaultSmileyCacheFolder(TCHAR* szPath, size_t cbLen)
{
	TCHAR *tmpPath = Utils_ReplaceVarsT(_T("%miranda_userdata%\\SmileyCache"));
	if ((INT_PTR)tmpPath != CALLSERVICE_NOTFOUND)
	{
		mir_sntprintf(szPath, cbLen, _T("%s"), tmpPath);
		mir_free(tmpPath);
	}
	else
	{
		char dbPath[MAX_PATH];
		CallService(MS_DB_GETPROFILEPATH, SIZEOF(dbPath), (LPARAM)dbPath);
		mir_sntprintf(szPath, cbLen, _T("%s\\SmileyCache"), A2T_SM(dbPath));
	}
}

int FolderChanged(WPARAM, LPARAM)
{
	FOLDERSGETDATA fgd = {0};
	fgd.cbSize = sizeof(FOLDERSGETDATA);
	fgd.nMaxPathSize = SIZEOF(cachepath);
	fgd.szPathT = cachepath;
	if (CallService(MS_FOLDERS_GET_PATH, (WPARAM) hFolder, (LPARAM) &fgd))
	{
		GetDefaultSmileyCacheFolder(cachepath, SIZEOF(cachepath));
	}

	return 0;
}

void GetSmileyCacheFolder(void)
{
	TCHAR defaultPath[MAX_PATH];

	GetDefaultSmileyCacheFolder(defaultPath, SIZEOF(defaultPath));

	FOLDERSDATA fd = {0};
	fd.cbSize = sizeof(FOLDERSDATA);
	strcpy(fd.szSection, "SmileyAdd");
	strcpy(fd.szName,"Smiley Cache");
	fd.szFormatT = defaultPath;
	fd.flags = FF_TCHAR;
	hFolder = (HANDLE)CallService(MS_FOLDERS_REGISTER_PATH, 0, (LPARAM) &fd);

	FolderChanged(0, 0);

	hFolderHook = HookEvent(ME_FOLDERS_PATH_CHANGED, FolderChanged);
}

void DownloadInit(void) 
{
	NETLIBUSER nlu = {0};
	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_OUTGOING|NUF_HTTPCONNS|NUF_NOHTTPSOPTION;
	nlu.szSettingsModule = "SmileyAdd";
	nlu.szDescriptiveName = Translate("SmileyAdd HTTP connections");
	hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	GetSmileyCacheFolder();
	g_hDlMutex = CreateMutex(NULL, FALSE, NULL);
}

void DownloadClose(void) 
{
	UnhookEvent(hFolderHook);
	CloseHandle(g_hDlMutex);
	Netlib_CloseHandle(hNetlibUser);
}
