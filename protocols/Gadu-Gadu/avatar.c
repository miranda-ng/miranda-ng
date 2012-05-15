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
void gg_getavatarfilename(GGPROTO *gg, HANDLE hContact, char *pszDest, int cbLen)
{
	int tPathLen;
	char *path = (char *)alloca(cbLen);
	char *avatartype = NULL;

	if (gg->hAvatarsFolder == NULL || FoldersGetCustomPath(gg->hAvatarsFolder, path, cbLen, "")) {
		char *tmpPath = Utils_ReplaceVars("%miranda_avatarcache%");
		tPathLen = mir_snprintf(pszDest, cbLen, "%s\\%s", tmpPath, GG_PROTO);
		mir_free(tmpPath);
	}
	else {
		strcpy(pszDest, path);
		tPathLen = (int)strlen(pszDest);
	}

	if (_access(pszDest, 0))
		CallService(MS_UTILS_CREATEDIRTREE, 0, (LPARAM)pszDest);

	switch (DBGetContactSettingByte(hContact, GG_PROTO, GG_KEY_AVATARTYPE, GG_KEYDEF_AVATARTYPE)) {
		case PA_FORMAT_JPEG: avatartype = "jpg"; break;
		case PA_FORMAT_GIF: avatartype = "gif"; break;
		case PA_FORMAT_PNG: avatartype = "png"; break;
	}

	if (hContact != NULL) {
		DBVARIANT dbv;
		if (!DBGetContactSettingString(hContact, GG_PROTO, GG_KEY_AVATARHASH, &dbv)) {
			mir_snprintf(pszDest + tPathLen, cbLen - tPathLen, "\\%s.%s", dbv.pszVal, avatartype);
			DBFreeVariant(&dbv);
		}
	}
	else
		mir_snprintf(pszDest + tPathLen, cbLen - tPathLen, "\\%s avatar.%s", GG_PROTO, avatartype);
}

void gg_getavatarfileinfo(GGPROTO *gg, uin_t uin, char **avatarurl, int *type)
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
	resp = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)gg->netlib, (LPARAM)&req);
	if (resp) {
		if (resp->resultCode == 200 && resp->dataLength > 0 && resp->pData) {
			HXML hXml;
			TCHAR *xmlAction;
			TCHAR *tag;

			xmlAction = gg_a2t(resp->pData);
			tag = gg_a2t("result");
			hXml = xi.parseString(xmlAction, 0, tag);

			if (hXml != NULL) {
				HXML node;
				char *blank;

				mir_free(tag); tag = gg_a2t("users/user/avatars/avatar");
				node = xi.getChildByPath(hXml, tag, 0);
				mir_free(tag); tag = gg_a2t("blank");
				blank = node != NULL ? gg_t2a(xi.getAttrValue(node, tag)) : NULL;

				if (blank != NULL && strcmp(blank, "1")) {
					mir_free(tag); tag = gg_a2t("users/user/avatars/avatar/bigAvatar");
					node = xi.getChildByPath(hXml, tag, 0);
					*avatarurl = node != NULL ? gg_t2a(xi.getText(node)) : NULL;

					mir_free(tag); tag = gg_a2t("users/user/avatars/avatar/originBigAvatar");
					node = xi.getChildByPath(hXml, tag, 0);
					if (node != NULL) {
						char *orgavurl = gg_t2a(xi.getText(node));
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
		else gg_netlog(gg, "gg_getavatarfileinfo(): Invalid response code from HTTP request");
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
	}
	else gg_netlog(gg, "gg_getavatarfileinfo(): No response from HTTP request");
}

char *gg_avatarhash(char *param)
{
	mir_sha1_byte_t digest[MIR_SHA1_HASH_SIZE];
	char *result;
	int i;

	if (param == NULL || (result = (char *)mir_alloc(MIR_SHA1_HASH_SIZE * 2 + 1)) == NULL)
		return NULL;

	mir_sha1_hash(param, (int)strlen(param), digest);
	for (i = 0; i < MIR_SHA1_HASH_SIZE; i++)
		sprintf(result + (i<<1), "%02x", digest[i]);

	return result;
}

typedef struct
{
	HANDLE hContact;
	char *AvatarURL;
} GGGETAVATARDATA;

void gg_getavatar(GGPROTO *gg, HANDLE hContact, char *szAvatarURL)
{
	if (gg->pth_avatar.dwThreadId) {
		GGGETAVATARDATA *data = mir_alloc(sizeof(GGGETAVATARDATA));
		data->hContact = hContact;
		data->AvatarURL = mir_strdup(szAvatarURL);
		EnterCriticalSection(&gg->avatar_mutex);
		list_add(&gg->avatar_transfers, data, 0);
		LeaveCriticalSection(&gg->avatar_mutex);
	}
}

typedef struct
{
	HANDLE hContact;
	int iWaitFor;
} GGREQUESTAVATARDATA;

void gg_requestavatar(GGPROTO *gg, HANDLE hContact, int iWaitFor)
{
	if (DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_ENABLEAVATARS, GG_KEYDEF_ENABLEAVATARS)
		&& gg->pth_avatar.dwThreadId) {
		GGREQUESTAVATARDATA *data = mir_alloc(sizeof(GGREQUESTAVATARDATA));
		data->hContact = hContact;
		data->iWaitFor = iWaitFor;
		EnterCriticalSection(&gg->avatar_mutex);
		list_add(&gg->avatar_requests, data, 0);
		LeaveCriticalSection(&gg->avatar_mutex);
	}
}

void __cdecl gg_avatarrequestthread(GGPROTO *gg, void *empty)
{
	list_t l;

	gg_netlog(gg, "gg_avatarrequestthread(): Avatar Request Thread Starting");
	while (gg->pth_avatar.dwThreadId)
	{
		EnterCriticalSection(&gg->avatar_mutex);
		if (gg->avatar_requests) {
			GGREQUESTAVATARDATA *data = (GGREQUESTAVATARDATA *)gg->avatar_requests->data;
			char *AvatarURL;
			int AvatarType, iWaitFor = data->iWaitFor;
			HANDLE hContact = data->hContact;

			list_remove(&gg->avatar_requests, data, 0);
			mir_free(data);
			LeaveCriticalSection(&gg->avatar_mutex);

			gg_getavatarfileinfo(gg, DBGetContactSettingDword(hContact, GG_PROTO, GG_KEY_UIN, 0), &AvatarURL, &AvatarType);
			if (AvatarURL != NULL && strlen(AvatarURL) > 0)
				DBWriteContactSettingString(hContact, GG_PROTO, GG_KEY_AVATARURL, AvatarURL);
			else
				DBDeleteContactSetting(hContact, GG_PROTO, GG_KEY_AVATARURL);
			DBWriteContactSettingByte(hContact, GG_PROTO, GG_KEY_AVATARTYPE, (BYTE)AvatarType);
			DBWriteContactSettingByte(hContact, GG_PROTO, GG_KEY_AVATARREQUESTED, 1);

			if (iWaitFor) {
				PROTO_AVATAR_INFORMATION pai = {0};
				pai.cbSize = sizeof(pai);
				pai.hContact = hContact;
				if (gg_getavatarinfo(gg, (WPARAM)GAIF_FORCE, (LPARAM)&pai) != GAIR_WAITFOR)
					ProtoBroadcastAck(GG_PROTO, hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)&pai, 0);
			}
			else ProtoBroadcastAck(GG_PROTO, hContact, ACKTYPE_AVATAR, ACKRESULT_STATUS, 0, 0);
		}
		else LeaveCriticalSection(&gg->avatar_mutex);

		EnterCriticalSection(&gg->avatar_mutex);
		if (gg->avatar_transfers) {
			GGGETAVATARDATA *data = (GGGETAVATARDATA *)gg->avatar_transfers->data;
			NETLIBHTTPREQUEST req = {0};
			NETLIBHTTPREQUEST *resp;
			PROTO_AVATAR_INFORMATION pai = {0};
			int result = 0;

			pai.cbSize = sizeof(pai);
			pai.hContact = data->hContact;
			pai.format = DBGetContactSettingByte(pai.hContact, GG_PROTO, GG_KEY_AVATARTYPE, GG_KEYDEF_AVATARTYPE);

			req.cbSize = sizeof(req);
			req.requestType = REQUEST_GET;
			req.szUrl = data->AvatarURL;
			req.flags = NLHRF_NODUMP | NLHRF_HTTP11 | NLHRF_REDIRECT;
			resp = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)gg->netlib, (LPARAM)&req);
			if (resp) {
				if (resp->resultCode == 200 && resp->dataLength > 0 && resp->pData) {
					int file_fd;

					gg_getavatarfilename(gg, pai.hContact, pai.filename, sizeof(pai.filename));
					file_fd = _open(pai.filename, _O_WRONLY | _O_TRUNC | _O_BINARY | _O_CREAT, _S_IREAD | _S_IWRITE);
					if (file_fd != -1) {
						_write(file_fd, resp->pData, resp->dataLength);
						_close(file_fd);
						result = 1;
					}
				}
				else gg_netlog(gg, "gg_avatarrequestthread(): Invalid response code from HTTP request");
				CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
			}
			else gg_netlog(gg, "gg_avatarrequestthread(): No response from HTTP request");

			ProtoBroadcastAck(GG_PROTO, pai.hContact, ACKTYPE_AVATAR,
				result ? ACKRESULT_SUCCESS : ACKRESULT_FAILED, (HANDLE)&pai, 0);

			if (!pai.hContact)
				CallService(MS_AV_REPORTMYAVATARCHANGED, (WPARAM)GG_PROTO, 0);

			list_remove(&gg->avatar_transfers, data, 0);
			mir_free(data->AvatarURL);
			mir_free(data);
		}
		LeaveCriticalSection(&gg->avatar_mutex);
		SleepEx(100, FALSE);
	}

	for (l = gg->avatar_requests; l; l = l->next) {
		GGREQUESTAVATARDATA *data = (GGREQUESTAVATARDATA *)l->data;
		mir_free(data);
	}
	for (l = gg->avatar_transfers; l; l = l->next) {
		GGGETAVATARDATA *data = (GGGETAVATARDATA *)l->data;
		mir_free(data->AvatarURL);
		mir_free(data);
	}
	list_destroy(gg->avatar_requests, 0);
	list_destroy(gg->avatar_transfers, 0);
	gg_netlog(gg, "gg_avatarrequestthread(): Avatar Request Thread Ending");
}

void gg_initavatarrequestthread(GGPROTO *gg)
{
	DWORD exitCode = 0;

	GetExitCodeThread(gg->pth_avatar.hThread, &exitCode);
	if (exitCode != STILL_ACTIVE) {
		gg->avatar_requests = gg->avatar_transfers = NULL;
		gg->pth_avatar.hThread = gg_forkthreadex(gg, gg_avatarrequestthread, NULL, &gg->pth_avatar.dwThreadId);
	}
}

void gg_uninitavatarrequestthread(GGPROTO *gg)
{
	gg->pth_avatar.dwThreadId = 0;
#ifdef DEBUGMODE
	gg_netlog(gg, "gg_uninitavatarrequestthread(): Waiting until Avatar Request Thread finished, if needed.");
#endif
	gg_threadwait(gg, &gg->pth_avatar);
}

void __cdecl gg_getuseravatarthread(GGPROTO *gg, void *empty)
{
	PROTO_AVATAR_INFORMATION pai = {0};
	char *AvatarURL;
	int AvatarType;

	gg_getavatarfileinfo(gg, DBGetContactSettingDword(NULL, GG_PROTO, GG_KEY_UIN, 0), &AvatarURL, &AvatarType);
	if (AvatarURL != NULL && strlen(AvatarURL) > 0)
		DBWriteContactSettingString(NULL, GG_PROTO, GG_KEY_AVATARURL, AvatarURL);
	else
		DBDeleteContactSetting(NULL, GG_PROTO, GG_KEY_AVATARURL);
	DBWriteContactSettingByte(NULL, GG_PROTO, GG_KEY_AVATARTYPE, (BYTE)AvatarType);
	DBWriteContactSettingByte(NULL, GG_PROTO, GG_KEY_AVATARREQUESTED, 1);

	pai.cbSize = sizeof(pai);
	gg_getavatarinfo(gg, (WPARAM)GAIF_FORCE, (LPARAM)&pai);
}

void gg_getuseravatar(GGPROTO *gg)
{
	if (DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_ENABLEAVATARS, GG_KEYDEF_ENABLEAVATARS)
		&& DBGetContactSettingDword(NULL, GG_PROTO, GG_KEY_UIN, 0))
		gg_forkthread(gg, gg_getuseravatarthread, NULL);
}

void __cdecl gg_setavatarthread(GGPROTO *gg, void *param)
{
	NETLIBHTTPHEADER httpHeaders[4];
	NETLIBHTTPREQUEST req = {0};
	NETLIBHTTPREQUEST *resp;
	char *szFilename = (char *)param;
	const char *contentend = "\r\n--AaB03x--\r\n";
	char szUrl[128], uin[32], *authHeader, *data, *avatardata, content[256],
		*fileext, image_ext[4], image_type[11];
	int file_fd, avatardatalen, datalen, contentlen, contentendlen, res = 0, repeat = 0;

	gg_netlog(gg, "gg_setavatar(): Trying to set user avatar using %s...", szFilename);
	UIN2ID(DBGetContactSettingDword(NULL, GG_PROTO, GG_KEY_UIN, 0), uin);

	file_fd = _open(szFilename, _O_RDONLY | _O_BINARY, _S_IREAD);
	if (file_fd == -1) {
		gg_netlog(gg, "gg_setavatar(): Failed to open avatar file (%s).", strerror(errno));
		mir_free(szFilename);
		gg_getuseravatar(gg);
		return;
	}
	avatardatalen = _filelength(file_fd);
	avatardata = (char *)mir_alloc(avatardatalen);

	_read(file_fd, avatardata, avatardatalen);
	_close(file_fd);

	fileext = strrchr(szFilename, '.');
	fileext++;
	if (!_stricmp(fileext, "jpg")) {
		strcpy(image_ext, "jpg");
		strcpy(image_type, "image/jpeg");
	}
	else if (!_stricmp(fileext, "gif")) {
		strcpy(image_ext, "gif");
		strcpy(image_type, "image/gif");
	}
	else /*if (!_stricmp(fileext, "png"))*/ {
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
	gg_oauth_checktoken(gg, 0);
	authHeader = gg_oauth_header(gg, "PUT", szUrl);

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

	resp = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)gg->netlib, (LPARAM)&req);
	if (resp) {
		if (resp->resultCode == 200 && resp->dataLength > 0 && resp->pData) {
#ifdef DEBUGMODE
			gg_netlog(gg, "%s", resp->pData);
#endif
			res = 1;
		}
		else gg_netlog(gg, "gg_setavatar(): Invalid response code from HTTP request");
		if (resp->resultCode == 403 || resp->resultCode == 401)
			repeat = 1;
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
	}
	else gg_netlog(gg, "gg_setavatar(): No response from HTTP request");

	if (repeat) { // Access Token expired - we need to obtain new
		mir_free(authHeader);
		gg_oauth_checktoken(gg, 1);
		authHeader = gg_oauth_header(gg, "PUT", szUrl);

		ZeroMemory(&req, sizeof(req));
		req.cbSize = sizeof(req);
		req.requestType = REQUEST_POST;
		req.szUrl = szUrl;
		req.flags = NLHRF_NODUMP | NLHRF_HTTP11;
		req.headersCount = 4;
		req.headers = httpHeaders;
		req.pData = data;
		req.dataLength = datalen;

		resp = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)gg->netlib, (LPARAM)&req);
		if (resp) {
			if (resp->resultCode == 200 && resp->dataLength > 0 && resp->pData) {
#ifdef DEBUGMODE
				gg_netlog(gg, "%s", resp->pData);
#endif
				res = 1;
			}
			else gg_netlog(gg, "gg_setavatar(): Invalid response code from HTTP request");
			CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
		}
		else gg_netlog(gg, "gg_setavatar(): No response from HTTP request");
	}

	mir_free(authHeader);
	mir_free(avatardata);
	mir_free(data);

	if (res)
		gg_netlog(gg, "gg_setavatar(): User avatar set successfully.");
	else
		gg_netlog(gg, "gg_setavatar(): Failed to set user avatar.");

	mir_free(szFilename);
	gg_getuseravatar(gg);
}

void gg_setavatar(GGPROTO *gg, const char *szFilename)
{
	gg_forkthread(gg, gg_setavatarthread, (void*)mir_strdup(szFilename));
}
