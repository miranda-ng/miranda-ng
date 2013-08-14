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

	if (hAvatarsFolder == NULL || FoldersGetCustomPathT(hAvatarsFolder, path, cbLen, _T("")))
		tPathLen = mir_sntprintf(pszDest, cbLen, _T("%s\\%S"), VARST( _T("%miranda_avatarcache%")), m_szModuleName);
	else {
		_tcscpy(pszDest, path);
		tPathLen = (int)_tcslen(pszDest);
	}

	if (_taccess(pszDest, 0)) {
		int ret = CreateDirectoryTreeT(pszDest);
		if (ret == 0)
			netlog("getAvatarFilename(): Created new directory for avatar cache: %S.", pszDest);
		else {
			netlog("getAvatarFilename(): Can not create directory for avatar cache: %S. errno=%d: %s", pszDest, errno, strerror(errno));
			TCHAR error[512];
			mir_sntprintf(error, SIZEOF(error), TranslateT("Can not create avatars cache directory. ERROR: %d: %s\n%s"), errno, _tcserror(errno), pszDest);
			showpopup(m_tszUserName, error, GG_POPUP_ERROR | GG_POPUP_ALLOW_MSGBOX | GG_POPUP_ONCE);
		}
	}

	const TCHAR *avatartype = ProtoGetAvatarExtension( getByte(hContact, GG_KEY_AVATARTYPE, GG_KEYDEF_AVATARTYPE));

	if (hContact != NULL) {
		DBVARIANT dbv;
		if (!getString(hContact, GG_KEY_AVATARHASH, &dbv)) {
			TCHAR* avatarHashT = mir_a2t(dbv.pszVal);
			mir_sntprintf(pszDest + tPathLen, cbLen - tPathLen, _T("\\%s%s"), avatarHashT, avatartype);
			mir_free(avatarHashT);
			db_free(&dbv);
		}
	}
	else mir_sntprintf(pszDest + tPathLen, cbLen - tPathLen, _T("\\%S avatar%s"), m_szModuleName, avatartype);
}

bool GGPROTO::getAvatarFileInfo(uin_t uin, char **avatarurl, char **avatarts)
{
	*avatarurl = *avatarts = NULL;

	char szUrl[128];
	mir_snprintf(szUrl, 128, "http://api.gadu-gadu.pl/avatars/%d/0.xml", uin);

	NETLIBHTTPREQUEST req = { sizeof(req) };
	req.requestType = REQUEST_GET;
	req.szUrl = szUrl;
	req.flags = NLHRF_NODUMP | NLHRF_HTTP11 | NLHRF_REDIRECT;

	NETLIBHTTPREQUEST *resp = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)netlib, (LPARAM)&req);
	if (resp == NULL) {
		netlog("getAvatarFileInfo(): No response from HTTP request");
		return false;
	}

	if (resp->resultCode != 200 || !resp->dataLength || !resp->pData) {
		netlog("getAvatarFileInfo(): Invalid response code from HTTP request");
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
		return false;
	}

	TCHAR *xmlAction = mir_a2t(resp->pData);
	HXML hXml = xi.parseString(xmlAction, 0, _T("result"));
	if (hXml != NULL) {
		HXML node = xi.getChildByPath(hXml, _T("users/user/avatars/avatar"), 0);
		const TCHAR *blank = (node != NULL) ? xi.getAttrValue(node, _T("blank")) : NULL;
		if (blank != NULL && _tcscmp(blank, _T("1"))) {
			node = xi.getChildByPath(hXml, _T("users/user/avatars/avatar/originBigAvatar"), 0);
			*avatarurl = node != NULL ? mir_t2a(xi.getText(node)) : NULL;

			node = xi.getChildByPath(hXml, _T("users/user/avatars/avatar/timestamp"), 0);
			*avatarts = node != NULL ? mir_t2a(xi.getText(node)) : NULL;
		}
		xi.destroyNode(hXml);
	}
	mir_free(xmlAction);
	CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
	return true;
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
		mir_snprintf(result + (i<<1), 2, "%02x", digest[i]);

	return result;
}

void GGPROTO::requestAvatarTransfer(HANDLE hContact, char *szAvatarURL)
{
	if (pth_avatar.dwThreadId == NULL) {
		netlog("requestAvatarTransfer(): Can not list_add element to avatar_transfers list. No pth_avatar.dwThreadId");
		return;
	}

	gg_EnterCriticalSection(&avatar_mutex, "requestAvatarTransfer", 1, "avatar_mutex", 1);
	if (avatar_transfers.getIndex((GGGETAVATARDATA*)&hContact) == -1) {
		GGGETAVATARDATA *data = (GGGETAVATARDATA*)mir_alloc(sizeof(GGGETAVATARDATA) + strlen(szAvatarURL)+1);
		data->hContact = hContact;
		data->szAvatarURL = strcpy((char*)(data+1), szAvatarURL);
		avatar_transfers.insert(data);
	}
	gg_LeaveCriticalSection(&avatar_mutex, "requestAvatarTransfer", 1, 1, "avatar_mutex", 1);
}

void GGPROTO::requestAvatarInfo(HANDLE hContact, int iWaitFor)
{
	if (pth_avatar.dwThreadId == NULL) {
		netlog("requestAvatarInfo(): Can not list_add element to avatar_requests list. No pth_avatar.dwThreadId");
		return;
	}
	
	if (!getByte(GG_KEY_ENABLEAVATARS, GG_KEYDEF_ENABLEAVATARS))
		return;

	GGREQUESTAVATARDATA *data = NULL;
	gg_EnterCriticalSection(&avatar_mutex, "requestAvatarInfo", 2, "avatar_mutex", 1);
	if (avatar_requests.getIndex((GGREQUESTAVATARDATA*)&hContact) == -1) {
		data = (GGREQUESTAVATARDATA*)mir_alloc(sizeof(GGREQUESTAVATARDATA));
		data->hContact = hContact;
		data->iWaitFor = iWaitFor;
		avatar_requests.insert(data);
	}
	gg_LeaveCriticalSection(&avatar_mutex, "requestAvatarInfo", 2, 1, "avatar_mutex", 1);

	if (data != NULL)
		setByte(hContact, GG_KEY_AVATARREQUESTED, 1);
}

void __cdecl GGPROTO::avatarrequestthread(void*)
{
	netlog("avatarrequestthread() started. Avatar Request Thread Starting");
	while (pth_avatar.dwThreadId)
	{
		gg_EnterCriticalSection(&avatar_mutex, "avatarrequestthread", 3, "avatar_mutex", 1);
		if (avatar_requests.getCount()) {
			GGREQUESTAVATARDATA *data = avatar_requests[0];
			int iWaitFor = data->iWaitFor;
			HANDLE hContact = data->hContact;
			avatar_requests.remove(0);
			mir_free(data);
			gg_LeaveCriticalSection(&avatar_mutex, "avatarrequestthread", 3, 1, "avatar_mutex", 1);
			
			uin_t uin = (uin_t)getDword(hContact, GG_KEY_UIN, 0);
			netlog("avatarrequestthread() new avatar_requests item for uin=%d.", uin);

			char *AvatarURL, *AvatarTs;
			if (!getAvatarFileInfo(uin, &AvatarURL, &AvatarTs)) {
				if (iWaitFor)
					ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, NULL, 0);
			}
			else {
				if (AvatarURL == NULL && AvatarTs == NULL){
					delSetting(hContact, GG_KEY_AVATARURL);
					delSetting(hContact, GG_KEY_AVATARTS);
					if (iWaitFor)
						ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, NULL, 0);
				}
				else {
					setString(hContact, GG_KEY_AVATARURL, AvatarURL);
					setString(hContact, GG_KEY_AVATARTS, AvatarTs);
					mir_free(AvatarURL); mir_free(AvatarTs);

					if (iWaitFor) {
						PROTO_AVATAR_INFORMATIONT pai = {0};
						pai.cbSize = sizeof(pai);
						pai.hContact = hContact;
						INT_PTR res = getavatarinfo((WPARAM)GAIF_FORCE, (LPARAM)&pai);
						if (res == GAIR_NOAVATAR)
							ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, NULL, 0);
						else if (res == GAIR_SUCCESS)
							ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)&pai, 0);
					}
					else ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_STATUS, 0, 0);
					delSetting(hContact, GG_KEY_AVATARREQUESTED);
				}
			}
		}
		else gg_LeaveCriticalSection(&avatar_mutex, "avatarrequestthread", 3, 2, "avatar_mutex", 1);

		gg_EnterCriticalSection(&avatar_mutex, "avatarrequestthread", 4, "avatar_mutex", 1);
		if (avatar_transfers.getCount()) {
			GGGETAVATARDATA *data = avatar_transfers[0];
			avatar_transfers.remove(0);
			gg_LeaveCriticalSection(&avatar_mutex, "avatarrequestthread", 4, 1, "avatar_mutex", 1);
			netlog("avatarrequestthread() new avatar_transfers item for url=%s.", data->szAvatarURL);

			int result = 0;

			PROTO_AVATAR_INFORMATIONT pai = { sizeof(pai) };
			pai.hContact = data->hContact;
			pai.format = getByte(pai.hContact, GG_KEY_AVATARTYPE, GG_KEYDEF_AVATARTYPE);

			NETLIBHTTPREQUEST req = { sizeof(req) };
			req.requestType = REQUEST_GET;
			req.szUrl = data->szAvatarURL;
			req.flags = NLHRF_NODUMP | NLHRF_HTTP11 | NLHRF_REDIRECT;

			NETLIBHTTPREQUEST *resp = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)netlib, (LPARAM)&req);
			if (resp) {
				if (resp->resultCode == 200 && resp->dataLength > 0 && resp->pData) {
					int file_fd;

					int avatarType = PA_FORMAT_UNKNOWN;
					for (int i=0; i < resp->headersCount; i++)
					{
						NETLIBHTTPHEADER& tHeader = resp->headers[i];
						if (!_stricmp(tHeader.szName, "Content-Type")) {
							if (!_stricmp(tHeader.szValue, "image/jpeg"))
								avatarType = PA_FORMAT_JPEG;
							else if (!_stricmp(tHeader.szValue, "image/gif"))
								avatarType = PA_FORMAT_GIF;
							else if (!_stricmp(tHeader.szValue, "image/png"))
								avatarType = PA_FORMAT_PNG;
						}
					}
					setByte(data->hContact, GG_KEY_AVATARTYPE, (BYTE)avatarType);

					getAvatarFilename(pai.hContact, pai.filename, sizeof(pai.filename));
					file_fd = _topen(pai.filename, _O_WRONLY | _O_TRUNC | _O_BINARY | _O_CREAT, _S_IREAD | _S_IWRITE);
					if (file_fd != -1) {
						_write(file_fd, resp->pData, resp->dataLength);
						_close(file_fd);
						result = 1;
						netlog("avatarrequestthread() new avatar_transfers item. Saved data from url=%s to file=%S.", data->szAvatarURL, pai.filename);
					} else {
						netlog("avatarrequestthread(): _topen file %S error. errno=%d: %s", pai.filename, errno, strerror(errno));
						TCHAR error[512];
						mir_sntprintf(error, SIZEOF(error), TranslateT("Can not create avatar file. ERROR: %d: %s\n%s"), errno, _tcserror(errno), pai.filename);
						showpopup(m_tszUserName, error, GG_POPUP_ERROR);
					}
				}
				else netlog("avatarrequestthread(): Invalid response code from HTTP request");
				CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
			}
			else netlog("avatarrequestthread(): No response from HTTP request");

			ProtoBroadcastAck(pai.hContact, ACKTYPE_AVATAR, result ? ACKRESULT_SUCCESS : ACKRESULT_FAILED, (HANDLE)&pai, 0);

			if (!pai.hContact)
				CallService(MS_AV_REPORTMYAVATARCHANGED, (WPARAM)m_szModuleName, 0);

			mir_free(data);
		}
		else gg_LeaveCriticalSection(&avatar_mutex, "avatarrequestthread", 4, 2, "avatar_mutex", 1);

		gg_sleep(100, FALSE, "avatarrequestthread", 101, 1);
	}

	for (int i=0; i < avatar_requests.getCount(); i++)
		mir_free(avatar_requests[i]);

	for (int k=0; k < avatar_transfers.getCount(); k++)
		mir_free(avatar_transfers[k]);

	avatar_requests.destroy();
	avatar_transfers.destroy();
	netlog("avatarrequestthread(): end. Avatar Request Thread Ending");
}

void GGPROTO::initavatarrequestthread()
{
	DWORD exitCode = 0;

	GetExitCodeThread(pth_avatar.hThread, &exitCode);
	if (exitCode != STILL_ACTIVE) {
		avatar_requests.destroy();
		avatar_transfers.destroy();
#ifdef DEBUGMODE
		netlog("initavatarrequestthread(): ForkThreadEx 1 GGPROTO::avatarrequestthread");
#endif
		pth_avatar.hThread = ForkThreadEx(&GGPROTO::avatarrequestthread, NULL, &pth_avatar.dwThreadId);
	}
}

void __cdecl GGPROTO::getOwnAvatarThread(void*)
{
	netlog("getOwnAvatarThread() started");

	char *AvatarURL, *AvatarTs;
	if (getAvatarFileInfo( getDword(GG_KEY_UIN, 0), &AvatarURL, &AvatarTs)) {
		if (AvatarURL != NULL && AvatarTs != NULL > 0) {
			setString(GG_KEY_AVATARURL, AvatarURL);
			setString(GG_KEY_AVATARTS, AvatarTs);
			mir_free(AvatarURL); mir_free(AvatarTs);
		} else {
			delSetting(GG_KEY_AVATARURL);
			delSetting(GG_KEY_AVATARTS);
		}
		setByte(GG_KEY_AVATARREQUESTED, 1);

		PROTO_AVATAR_INFORMATIONT pai = {0};
		pai.cbSize = sizeof(pai);
		getavatarinfo((WPARAM)GAIF_FORCE, (LPARAM)&pai);
	}
#ifdef DEBUGMODE
	netlog("getOwnAvatarThread(): end");
#endif
}

void GGPROTO::getOwnAvatar()
{
	if (getByte(GG_KEY_ENABLEAVATARS, GG_KEYDEF_ENABLEAVATARS) && getDword(GG_KEY_UIN, 0)){
#ifdef DEBUGMODE
		netlog("getOwnAvatar(): ForkThread 2 GGPROTO::getOwnAvatarThread");
#endif
		ForkThread(&GGPROTO::getOwnAvatarThread, NULL);
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

	netlog("setavatarthread(): started. Trying to set user avatar.");
	UIN2IDA( getDword(GG_KEY_UIN, 0), uin);

	file_fd = _topen(szFilename, _O_RDONLY | _O_BINARY, _S_IREAD);
	if (file_fd == -1) {
		netlog("setavatarthread(): Failed to open avatar file errno=%d: %s.", errno, strerror(errno));
		TCHAR error[512];
		mir_sntprintf(error, SIZEOF(error), TranslateT("Can not open avatar file. ERROR: %d: %s\n%s"), errno, _tcserror(errno), szFilename);
		showpopup(m_tszUserName, error, GG_POPUP_ERROR);
		mir_free(szFilename);
		int prevType = getByte(GG_KEY_AVATARTYPEPREV, -1);
		if (prevType != -1)
			setByte(GG_KEY_AVATARTYPE, prevType);
		delSetting(GG_KEY_AVATARTYPEPREV);
		getOwnAvatar();
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

	if (res) {
		netlog("setavatarthread(): User avatar set successfully.");
	} else {
		int prevType = getByte(GG_KEY_AVATARTYPEPREV, -1);
		if (prevType != -1)
			setByte(GG_KEY_AVATARTYPE, prevType);
		netlog("setavatarthread(): Failed to set user avatar.");
	}
	delSetting(GG_KEY_AVATARTYPEPREV);

	mir_free(szFilename);
	getOwnAvatar();
#ifdef DEBUGMODE
	netlog("setavatarthread(): end.");
#endif
}

void GGPROTO::setAvatar(const TCHAR *szFilename)
{
#ifdef DEBUGMODE
	netlog("setAvatar(): ForkThread 3 GGPROTO::setavatarthread");
#endif
	ForkThread(&GGPROTO::setavatarthread, mir_tstrdup(szFilename));
}
