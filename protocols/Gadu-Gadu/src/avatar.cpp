////////////////////////////////////////////////////////////////////////////////
// Gadu-Gadu Plugin for Miranda IM
//
// Copyright (c) 2009-2012 Bartosz Bia³ek
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
////////////////////////////////////////////////////////////////////////////////

#include "gg.h"
#include <io.h>
#include <fcntl.h>
#include "protocol.h"

//////////////////////////////////////////////////////////
// Avatars support
void GGPROTO::getAvatarFilename(HANDLE hContact, TCHAR *pszDest, int cbLen)
{
	int tPathLen;
	TCHAR *path = (TCHAR*)alloca(cbLen * sizeof(TCHAR));
	TCHAR *avatartype = NULL;

	if (hAvatarsFolder == NULL || FoldersGetCustomPathT(hAvatarsFolder, path, cbLen, _T(""))) {
		mir_ptr<TCHAR> tmpPath( Utils_ReplaceVarsT( _T("%miranda_avatarcache%")));
		tPathLen = mir_sntprintf(pszDest, cbLen, _T("%s\\%s"), (TCHAR*)tmpPath, m_tszUserName);
	}
	else {
		_tcscpy(pszDest, path);
		tPathLen = (int)_tcslen(pszDest);
	}

	if (_taccess(pszDest, 0))
		CallService(MS_UTILS_CREATEDIRTREET, 0, (LPARAM)pszDest);

	switch (db_get_b(hContact, m_szModuleName, GG_KEY_AVATARTYPE, GG_KEYDEF_AVATARTYPE)) {
		case PA_FORMAT_JPEG: avatartype = _T("jpg"); break;
		case PA_FORMAT_GIF: avatartype = _T("gif"); break;
		case PA_FORMAT_PNG: avatartype = _T("png"); break;
	}

	if (hContact != NULL) {
		DBVARIANT dbv;
		if (!db_get_s(hContact, m_szModuleName, GG_KEY_AVATARHASH, &dbv, DBVT_ASCIIZ)) {
			TCHAR* avatarHashT = mir_a2t(dbv.pszVal);
			mir_sntprintf(pszDest + tPathLen, cbLen - tPathLen, _T("\\%s.%s"), avatarHashT, avatartype);
			mir_free(avatarHashT);
			DBFreeVariant(&dbv);
		}
	} else {
		mir_sntprintf(pszDest + tPathLen, cbLen - tPathLen, _T("\\%s avatar.%s"), m_tszUserName, avatartype);
	}
}

void GGPROTO::getAvatarFileInfo(uin_t uin, char **avatarurl, int *type)
{
	NETLIBHTTPREQUEST req = {0};
	NETLIBHTTPREQUEST *resp;
	char szUrl[128];
	*avatarurl = NULL;
	*type = PA_FORMAT_UNKNOWN;

	req.cbSize = sizeof(req);
	req.requestType = REQUEST_GET;
	req.szUrl = szUrl;
	mir_snprintf(szUrl, 128, "http://api.gadu-gadu.pl/avatars/%d/0.xml", uin);
	req.flags = NLHRF_NODUMP | NLHRF_HTTP11 | NLHRF_REDIRECT;
	resp = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)netlib, (LPARAM)&req);
	if (resp) {
		if (resp->resultCode == 200 && resp->dataLength > 0 && resp->pData) {
			HXML hXml;
			TCHAR *xmlAction;
			TCHAR *tag;

			xmlAction = mir_a2t(resp->pData);
			tag = mir_a2t("result");
			hXml = xi.parseString(xmlAction, 0, tag);

			if (hXml != NULL) {
				HXML node;
				char *blank;

				mir_free(tag); tag = mir_a2t("users/user/avatars/avatar");
				node = xi.getChildByPath(hXml, tag, 0);
				mir_free(tag); tag = mir_a2t("blank");
				blank = (node != NULL) ? mir_t2a(xi.getAttrValue(node, tag)) : NULL;

				if (blank != NULL && strcmp(blank, "1")) {
					mir_free(tag); tag = mir_a2t("users/user/avatars/avatar/bigAvatar");
					node = xi.getChildByPath(hXml, tag, 0);
					*avatarurl = node != NULL ? mir_t2a(xi.getText(node)) : NULL;

					mir_free(tag); tag = mir_a2t("users/user/avatars/avatar/originBigAvatar");
					node = xi.getChildByPath(hXml, tag, 0);
					if (node != NULL) {
						char *orgavurl = mir_t2a(xi.getText(node));
						char *avtype = strrchr(orgavurl, '.');
						avtype++;
						if (!_stricmp(avtype, "jpg"))
							*type = PA_FORMAT_JPEG;
						else if (!_stricmp(avtype, "gif"))
							*type = PA_FORMAT_GIF;
						else if (!_stricmp(avtype, "png"))
							*type = PA_FORMAT_PNG;
						mir_free(orgavurl);
					}
				}
				else *avatarurl = mir_strdup("");
				mir_free(blank);
				xi.destroyNode(hXml);
			}
			mir_free(tag);
			mir_free(xmlAction);
		}
		else netlog("getAvatarFileInfo(): Invalid response code from HTTP request");
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
	}
	else netlog("getAvatarFileInfo(): No response from HTTP request");
}

char *gg_avatarhash(char *param)
{
	mir_sha1_byte_t digest[MIR_SHA1_HASH_SIZE];
	char *result;
	int i;

	if (param == NULL || (result = (char *)mir_alloc(MIR_SHA1_HASH_SIZE * 2 + 1)) == NULL)
		return NULL;

	mir_sha1_hash((BYTE*)param, (int)strlen(param), digest);
	for (i = 0; i < MIR_SHA1_HASH_SIZE; i++)
		sprintf(result + (i<<1), "%02x", digest[i]);

	return result;
}

typedef struct
{
	HANDLE hContact;
	char *AvatarURL;
} GGGETAVATARDATA;

void GGPROTO::getAvatar(HANDLE hContact, char *szAvatarURL)
{
#ifdef DEBUGMODE
	netlog("getAvatar(): start");
#endif
	if (pth_avatar.dwThreadId) {
		GGGETAVATARDATA *data = (GGGETAVATARDATA*)mir_alloc(sizeof(GGGETAVATARDATA));
		data->hContact = hContact;
		data->AvatarURL = mir_strdup(szAvatarURL);
		gg_EnterCriticalSection(&avatar_mutex, "getAvatar", 1, "avatar_mutex", 1);
		list_add(&avatar_transfers, data, 0);
		gg_LeaveCriticalSection(&avatar_mutex, "getAvatar", 1, 1, "avatar_mutex", 1);
	}
}

typedef struct
{
	HANDLE hContact;
	int iWaitFor;
} GGREQUESTAVATARDATA;

void GGPROTO::requestAvatar(HANDLE hContact, int iWaitFor)
{
#ifdef DEBUGMODE
	netlog("requestAvatar(): start");
#endif

	if (db_get_b(NULL, m_szModuleName, GG_KEY_ENABLEAVATARS, GG_KEYDEF_ENABLEAVATARS)
		&& pth_avatar.dwThreadId) {
		GGREQUESTAVATARDATA *data = (GGREQUESTAVATARDATA*)mir_alloc(sizeof(GGREQUESTAVATARDATA));
		data->hContact = hContact;
		data->iWaitFor = iWaitFor;
		gg_EnterCriticalSection(&avatar_mutex, "requestAvatar", 2, "avatar_mutex", 1);
		list_add(&avatar_requests, data, 0);
		gg_LeaveCriticalSection(&avatar_mutex, "requestAvatar", 2, 1, "avatar_mutex", 1);
	}
}

void __cdecl GGPROTO::avatarrequestthread(void*)
{
	list_t l;

	netlog("avatarrequestthread() started. Avatar Request Thread Starting");
	while (pth_avatar.dwThreadId)
	{
		gg_EnterCriticalSection(&avatar_mutex, "avatarrequestthread", 3, "avatar_mutex", 1);
		if (avatar_requests) {
			GGREQUESTAVATARDATA *data = (GGREQUESTAVATARDATA *)avatar_requests->data;
			char *AvatarURL;
			int AvatarType, iWaitFor = data->iWaitFor;
			HANDLE hContact = data->hContact;

			list_remove(&avatar_requests, data, 0);
			mir_free(data);
			gg_LeaveCriticalSection(&avatar_mutex, "avatarrequestthread", 3, 1, "avatar_mutex", 1);

			getAvatarFileInfo( db_get_dw(hContact, m_szModuleName, GG_KEY_UIN, 0), &AvatarURL, &AvatarType);
			if (AvatarURL != NULL && strlen(AvatarURL) > 0)
				db_set_s(hContact, m_szModuleName, GG_KEY_AVATARURL, AvatarURL);
			else
				db_unset(hContact, m_szModuleName, GG_KEY_AVATARURL);
			db_set_b(hContact, m_szModuleName, GG_KEY_AVATARTYPE, (BYTE)AvatarType);
			db_set_b(hContact, m_szModuleName, GG_KEY_AVATARREQUESTED, 1);

			if (iWaitFor) {
				PROTO_AVATAR_INFORMATIONT pai = {0};
				pai.cbSize = sizeof(pai);
				pai.hContact = hContact;
				if (getavatarinfo((WPARAM)GAIF_FORCE, (LPARAM)&pai) != GAIR_WAITFOR)
					ProtoBroadcastAck(m_szModuleName, hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)&pai, 0);
			}
			else ProtoBroadcastAck(m_szModuleName, hContact, ACKTYPE_AVATAR, ACKRESULT_STATUS, 0, 0);
		} else {
			gg_LeaveCriticalSection(&avatar_mutex, "avatarrequestthread", 3, 2, "avatar_mutex", 1);
		}

		gg_EnterCriticalSection(&avatar_mutex, "avatarrequestthread", 4, "avatar_mutex", 1);
		if (avatar_transfers) {
			GGGETAVATARDATA *data = (GGGETAVATARDATA *)avatar_transfers->data;
			NETLIBHTTPREQUEST req = {0};
			NETLIBHTTPREQUEST *resp;
			PROTO_AVATAR_INFORMATIONT pai = {0};
			int result = 0;

			pai.cbSize = sizeof(pai);
			pai.hContact = data->hContact;
			pai.format = db_get_b(pai.hContact, m_szModuleName, GG_KEY_AVATARTYPE, GG_KEYDEF_AVATARTYPE);

			req.cbSize = sizeof(req);
			req.requestType = REQUEST_GET;
			req.szUrl = data->AvatarURL;
			req.flags = NLHRF_NODUMP | NLHRF_HTTP11 | NLHRF_REDIRECT;
			resp = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)netlib, (LPARAM)&req);
			if (resp) {
				if (resp->resultCode == 200 && resp->dataLength > 0 && resp->pData) {
					int file_fd;

					getAvatarFilename(pai.hContact, pai.filename, sizeof(pai.filename));
					file_fd = _topen(pai.filename, _O_WRONLY | _O_TRUNC | _O_BINARY | _O_CREAT, _S_IREAD | _S_IWRITE);
					if (file_fd != -1) {
						_write(file_fd, resp->pData, resp->dataLength);
						_close(file_fd);
						result = 1;
					}
				}
				else netlog("avatarrequestthread(): Invalid response code from HTTP request");
				CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
			}
			else netlog("avatarrequestthread(): No response from HTTP request");

			ProtoBroadcastAck(m_szModuleName, pai.hContact, ACKTYPE_AVATAR,
				result ? ACKRESULT_SUCCESS : ACKRESULT_FAILED, (HANDLE)&pai, 0);

			if (!pai.hContact)
				CallService(MS_AV_REPORTMYAVATARCHANGED, (WPARAM)m_szModuleName, 0);

			list_remove(&avatar_transfers, data, 0);
			mir_free(data->AvatarURL);
			mir_free(data);
		}
		gg_LeaveCriticalSection(&avatar_mutex, "avatarrequestthread", 4, 1, "avatar_mutex", 1);
		gg_sleep(100, FALSE, "avatarrequestthread", 101, 1);
	}

	for (l = avatar_requests; l; l = l->next) {
		GGREQUESTAVATARDATA *data = (GGREQUESTAVATARDATA *)l->data;
		mir_free(data);
	}
	for (l = avatar_transfers; l; l = l->next) {
		GGGETAVATARDATA *data = (GGGETAVATARDATA *)l->data;
		mir_free(data->AvatarURL);
		mir_free(data);
	}
	list_destroy(avatar_requests, 0);
	list_destroy(avatar_transfers, 0);
	netlog("avatarrequestthread(): end. Avatar Request Thread Ending");
}

void GGPROTO::initavatarrequestthread()
{
	DWORD exitCode = 0;

	GetExitCodeThread(pth_avatar.hThread, &exitCode);
	if (exitCode != STILL_ACTIVE) {
		avatar_requests = avatar_transfers = NULL;
#ifdef DEBUGMODE
		netlog("initavatarrequestthread(): forkthreadex 1 GGPROTO::avatarrequestthread");
#endif
		pth_avatar.hThread = forkthreadex(&GGPROTO::avatarrequestthread, NULL, &pth_avatar.dwThreadId);
	}
}

void GGPROTO::uninitavatarrequestthread()
{
	pth_avatar.dwThreadId = 0;
#ifdef DEBUGMODE
	netlog("initavatarrequestthread() Waiting pth_avatar thread. Waiting until Avatar Request Thread finished, if needed.");
#endif
	threadwait(&pth_avatar);
#ifdef DEBUGMODE
	netlog("initavatarrequestthread() Waiting pth_avatar thread - OK");
#endif
}

void __cdecl GGPROTO::getuseravatarthread(void*)
{
	char *AvatarURL;
	int AvatarType;

	netlog("getuseravatarthread() started");

	getAvatarFileInfo( db_get_dw(NULL, m_szModuleName, GG_KEY_UIN, 0), &AvatarURL, &AvatarType);
	if (AvatarURL != NULL && strlen(AvatarURL) > 0)
		db_set_s(NULL, m_szModuleName, GG_KEY_AVATARURL, AvatarURL);
	else
		db_unset(NULL, m_szModuleName, GG_KEY_AVATARURL);
	db_set_b(NULL, m_szModuleName, GG_KEY_AVATARTYPE, (BYTE)AvatarType);
	db_set_b(NULL, m_szModuleName, GG_KEY_AVATARREQUESTED, 1);
	mir_free(AvatarURL);

	PROTO_AVATAR_INFORMATIONT pai = {0};
	pai.cbSize = sizeof(pai);
	getavatarinfo((WPARAM)GAIF_FORCE, (LPARAM)&pai);
#ifdef DEBUGMODE
	netlog("getuseravatarthread(): end");
#endif
}

void GGPROTO::getUserAvatar()
{
	if (db_get_b(NULL, m_szModuleName, GG_KEY_ENABLEAVATARS, GG_KEYDEF_ENABLEAVATARS)
		&& db_get_dw(NULL, m_szModuleName, GG_KEY_UIN, 0)){
#ifdef DEBUGMODE
		netlog("getUserAvatar(): forkthread 2 GGPROTO::getuseravatarthread");
#endif
		forkthread(&GGPROTO::getuseravatarthread, NULL);
	}
}

void __cdecl GGPROTO::setavatarthread(void *param)
{
	NETLIBHTTPHEADER httpHeaders[4];
	NETLIBHTTPREQUEST req = {0};
	NETLIBHTTPREQUEST *resp;
	TCHAR *szFilename = (TCHAR*)param;
	const char *contentend = "\r\n--AaB03x--\r\n";
	char szUrl[128], uin[32], *authHeader, *data, *avatardata, content[256], image_ext[4], image_type[11];
	int file_fd, avatardatalen, datalen, contentlen, contentendlen, res = 0, repeat = 0;

	netlog("setavatarthread(): started. Trying to set user avatar using %s...", szFilename);
	UIN2ID( db_get_dw(NULL, m_szModuleName, GG_KEY_UIN, 0), uin);

	file_fd = _topen(szFilename, _O_RDONLY | _O_BINARY, _S_IREAD);
	if (file_fd == -1) {
		netlog("setavatarthread(): Failed to open avatar file (%s).", strerror(errno));
		mir_free(szFilename);
		getUserAvatar();
#ifdef DEBUGMODE
		netlog("setavatarthread(): end. err1");
#endif
		return;
	}
	avatardatalen = _filelength(file_fd);
	avatardata = (char *)mir_alloc(avatardatalen);

	_read(file_fd, avatardata, avatardatalen);
	_close(file_fd);

	TCHAR *fileext = _tcsrchr(szFilename, '.');
	fileext++;
	if (!_tcsicmp(fileext, _T("jpg"))) {
		strcpy(image_ext, "jpg");
		strcpy(image_type, "image/jpeg");
	}
	else if (!_tcsicmp(fileext, _T("gif"))) {
		strcpy(image_ext, "gif");
		strcpy(image_type, "image/gif");
	}
	else {
		strcpy(image_ext, "png");
		strcpy(image_type, "image/png");
	}

	mir_snprintf(content, 256, "--AaB03x\r\nContent-Disposition: form-data; name=\"_method\"\r\n\r\nPUT\r\n--AaB03x\r\nContent-Disposition: form-data; name=\"avatar\"; filename=\"%s.%s\"\r\nContent-Type: %s\r\n\r\n",
		uin, image_ext, image_type);
	contentlen = (int)strlen(content);
	contentendlen = (int)strlen(contentend);

	datalen = contentlen + avatardatalen + contentendlen;
	data = (char *)mir_alloc(datalen);
	memcpy(data, content, contentlen);
	memcpy(data + contentlen, avatardata, avatardatalen);
	memcpy(data + contentlen + avatardatalen, contentend, contentendlen);

	mir_snprintf(szUrl, 128, "http://api.gadu-gadu.pl/avatars/%s/0.xml", uin);
	oauth_checktoken(0);
	authHeader = oauth_header("PUT", szUrl);

	req.cbSize = sizeof(req);
	req.requestType = REQUEST_POST;
	req.szUrl = szUrl;
	req.flags = NLHRF_NODUMP | NLHRF_HTTP11;
	req.headersCount = 4;
	req.headers = httpHeaders;
	httpHeaders[0].szName   = "User-Agent";
	httpHeaders[0].szValue = GG8_VERSION;
	httpHeaders[1].szName  = "Authorization";
	httpHeaders[1].szValue = authHeader;
	httpHeaders[2].szName  = "Accept";
	httpHeaders[2].szValue = "*/*";
	httpHeaders[3].szName  = "Content-Type";
	httpHeaders[3].szValue = "multipart/form-data; boundary=AaB03x";
	req.pData = data;
	req.dataLength = datalen;

	resp = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)netlib, (LPARAM)&req);
	if (resp) {
		if (resp->resultCode == 200 && resp->dataLength > 0 && resp->pData) {
#ifdef DEBUGMODE
			netlog("setavatarthread(): 1 resp.data= %s", resp->pData);
#endif
			res = 1;
		}
		else netlog("setavatarthread() Invalid response code from HTTP request");
		if (resp->resultCode == 403 || resp->resultCode == 401)
			repeat = 1;
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
	}
	else netlog("setavatarthread(): No response from HTTP request");

	if (repeat) { // Access Token expired - we need to obtain new
		mir_free(authHeader);
		oauth_checktoken(1);
		authHeader = oauth_header("PUT", szUrl);

		ZeroMemory(&req, sizeof(req));
		req.cbSize = sizeof(req);
		req.requestType = REQUEST_POST;
		req.szUrl = szUrl;
		req.flags = NLHRF_NODUMP | NLHRF_HTTP11;
		req.headersCount = 4;
		req.headers = httpHeaders;
		req.pData = data;
		req.dataLength = datalen;

		resp = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)netlib, (LPARAM)&req);
		if (resp) {
			if (resp->resultCode == 200 && resp->dataLength > 0 && resp->pData) {
#ifdef DEBUGMODE
				netlog("setavatarthread(): 2 resp.data= %s", resp->pData);
#endif
				res = 1;
			}
			else netlog("setavatarthread(): Invalid response code from HTTP request");
			CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
		}
		else netlog("setavatarthread(): No response from HTTP request");
	}

	mir_free(authHeader);
	mir_free(avatardata);
	mir_free(data);

	if (res)
		netlog("setavatarthread(): User avatar set successfully.");
	else
		netlog("setavatarthread(): Failed to set user avatar.");

	mir_free(szFilename);
	getUserAvatar();
#ifdef DEBUGMODE
	netlog("setavatarthread(): end.");
#endif
}

void GGPROTO::setAvatar(const TCHAR *szFilename)
{
#ifdef DEBUGMODE
		netlog("setAvatar(): forkthread 3 GGPROTO::setavatarthread");
#endif
	forkthread(&GGPROTO::setavatarthread, mir_tstrdup(szFilename));
}
