/*
Miranda Crash Dumper Plugin
Copyright (C) 2008 - 2012 Boris Krasnovskiy All Rights Reserved

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

static void arrayToHex(uint8_t* data, size_t datasz, char* res)
{
	char* resptr = res;
	for (unsigned i = 0; i < datasz; i++) {
		const uint8_t ch = data[i];

		const char ch0 = (char)(ch >> 4);
		*resptr++ = (char)((ch0 <= 9) ? ('0' + ch0) : (('a' - 10) + ch0));

		const char ch1 = (char)(ch & 0xF);
		*resptr++ = (char)((ch1 <= 9) ? ('0' + ch1) : (('a' - 10) + ch1));
	}
	*resptr = '\0';
}

void GetLoginStr(char* user, size_t szuser, char* pass)
{
	DBVARIANT dbv;

	if (g_plugin.getString("Username", &dbv) == 0) {
		strncpy_s(user, szuser, dbv.pszVal, _TRUNCATE);
		db_free(&dbv);
	}
	else
		user[0] = 0;

	if (g_plugin.getString("Password", &dbv) == 0) {
		uint8_t hash[16];
		mir_md5_state_t context;

		mir_md5_init(&context);
		mir_md5_append(&context, (uint8_t*)dbv.pszVal, (int)mir_strlen(dbv.pszVal));
		mir_md5_finish(&context, hash);

		arrayToHex(hash, sizeof(hash), pass);

		db_free(&dbv);
	}
	else
		pass[0] = 0;
}

void OpenAuthUrl(const char* url)
{
	char user[64], pass[40];
	GetLoginStr(user, sizeof(user), pass);

	if (user[0] && pass[0]) {
		char str[256];
		mir_snprintf(str, url, user); // XXX: fix me
		mir_snprintf(str, "https://vi.miranda-ng.org/detail/%s", user);
		Utils_OpenUrl(str);
	}
	else Utils_OpenUrl("https://vi.miranda-ng.org/");
}

void CreateAuthString(char* auth)
{
	char user[64], pass[40];
	GetLoginStr(user, sizeof(user), pass);

	char str[110];
	int len = mir_snprintf(str, "%s:%s", user, pass);
	mir_snprintf(auth, 250, "Basic %s", ptrA(mir_base64_encode(str, len)));
}


bool InternetDownloadFile(const char *szUrl, VerTrnsfr* szReq)
{
	int result = 0xBADBAD;
	char* szRedirUrl = nullptr;
	NETLIBHTTPREQUEST nlhr = {};

	// initialize the netlib request
	nlhr.cbSize = sizeof(nlhr);
	nlhr.requestType = REQUEST_POST;
	nlhr.flags = NLHRF_HTTP11 | NLHRF_NODUMP;
	nlhr.szUrl = (char*)szUrl;

	nlhr.headersCount = 6;
	nlhr.headers = (NETLIBHTTPHEADER*)mir_alloc(sizeof(NETLIBHTTPHEADER)*nlhr.headersCount);
	nlhr.headers[0].szName = "Connection";
	nlhr.headers[0].szValue = "close";
	nlhr.headers[1].szName = "Cache-Control";
	nlhr.headers[1].szValue = "no-cache";
	nlhr.headers[2].szName = "Pragma";
	nlhr.headers[2].szValue = "no-cache";
	nlhr.headers[3].szName = "Content-Type";
	nlhr.headers[3].szValue = "text/plain; charset=utf-8";
	nlhr.headers[4].szName = "AutoUpload";
	nlhr.headers[4].szValue = (char*)(szReq->autot ? "1" : "0");
	nlhr.headers[5].szName = "Authorization";

	char auth[256];
	CreateAuthString(auth);
	nlhr.headers[5].szValue = auth;

	nlhr.pData = szReq->buf;
	nlhr.dataLength = (int)mir_strlen(szReq->buf);

	while (result == 0xBADBAD) {
		// download the page
		NLHR_PTR nlhrReply(Netlib_HttpTransaction(hNetlibUser, &nlhr));
		if (nlhrReply) {
			// if the recieved code is 200 OK
			switch (nlhrReply->resultCode) {
			case 200:
				if (g_plugin.bUploadChanged)
					ProcessVIHash(true);

				ShowMessage(1, TranslateT("Version Info upload successful."));
				result = 0;
				break;

			case 401:
				ShowMessage(0, TranslateT("Cannot upload Version Info. Incorrect username or password"));
				result = 1;
				break;

			case 510:
				ShowMessage(0, TranslateT("Cannot upload Version Info. User is banned"));
				result = 1;
				break;

			case 511:
				ShowMessage(0, TranslateT("Cannot upload Version Info. Daily upload limit exceeded"));
				result = 1;
				break;

			case 301:
			case 302:
			case 307:
				// get the url for the new location and save it to szInfo
				// look for the reply header "Location"
				if (auto *pszUrl = Netlib_GetHeader(nlhrReply, "Location")) {
					size_t rlen = 0;
					if (pszUrl[0] == '/') {
						const char* szPath;
						const char* szPref = strstr(szUrl, "://");
						szPref = szPref ? szPref + 3 : szUrl;
						szPath = strchr(szPref, '/');
						rlen = szPath != nullptr ? szPath - szUrl : mir_strlen(szUrl);
					}

					szRedirUrl = (char*)mir_realloc(szRedirUrl,
						rlen + mir_strlen(pszUrl) * 3 + 1);

					strncpy(szRedirUrl, szUrl, rlen);
					mir_strcpy(szRedirUrl + rlen, pszUrl);

					nlhr.szUrl = szRedirUrl;
				}
				break;

			default:
				result = 1;
				ShowMessage(0, TranslateT("Cannot upload Version Info. Unknown error"));
			}
		}
		else {
			result = 1;
			ShowMessage(0, TranslateT("Cannot upload Version Info. Host unreachable."));
		}
	}

	mir_free(szRedirUrl);
	mir_free(nlhr.headers);

	return result == 0;
}

void __cdecl VersionInfoUploadThread(void* arg)
{
	VerTrnsfr* trn = (VerTrnsfr*)arg;

	char user[64], pass[40];
	GetLoginStr(user, sizeof(user), pass);
	char str[256];
	mir_snprintf(str, "https://vi.miranda-ng.org/en/upload?login=%s&pass=%s", user, pass);

	InternetDownloadFile(str, trn);
	mir_free(trn->buf);
	mir_free(trn);
}

void UploadInit(void)
{
	NETLIBUSER nlu = {};
	nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_NOHTTPSOPTION | NUF_UNICODE;
	nlu.szSettingsModule = (char*)MODULENAME;
	nlu.szDescriptiveName.w = TranslateT("Crash Dumper HTTP connections");
	hNetlibUser = Netlib_RegisterUser(&nlu);
}

void UploadClose(void)
{
	Netlib_CloseHandle(hNetlibUser);
}

bool ProcessVIHash(bool store)
{
	CMStringW buffer;
	PrintVersionInfo(buffer, 0);

	uint8_t hash[16];
	mir_md5_state_t context;

	mir_md5_init(&context);
	mir_md5_append(&context, (uint8_t*)buffer.c_str(), buffer.GetLength()*sizeof(wchar_t));
	mir_md5_finish(&context, hash);

	char hashstr[40];
	arrayToHex(hash, sizeof(hash), hashstr);

	if (store) {
		g_plugin.setString("VIHash", hashstr);
		return true;
	}

	ptrA VIHash(g_plugin.getStringA("VIHash"));
	if (VIHash == NULL)
		return false;

	return mir_strcmp(hashstr, VIHash) == 0;
}
