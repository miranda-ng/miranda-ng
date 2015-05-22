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

void GGPROTO::getAvatarFilename(MCONTACT hContact, TCHAR *pszDest, int cbLen)
{
	int tPathLen = mir_sntprintf(pszDest, cbLen, _T("%s\\%S"), VARST( _T("%miranda_avatarcache%")), m_szModuleName);

	if (_taccess(pszDest, 0)) {
		int ret = CreateDirectoryTreeT(pszDest);
		if (ret == 0)
			debugLog(_T("getAvatarFilename(): Created new directory for avatar cache: %s."), pszDest);
		else {
			debugLog(_T("getAvatarFilename(): Can not create directory for avatar cache: %s. errno=%d: %s"), pszDest, errno, strerror(errno));
			TCHAR error[512];
			mir_sntprintf(error, SIZEOF(error), TranslateT("Cannot create avatars cache directory. ERROR: %d: %s\n%s"), errno, _tcserror(errno), pszDest);
			showpopup(m_tszUserName, error, GG_POPUP_ERROR | GG_POPUP_ALLOW_MSGBOX | GG_POPUP_ONCE);
		}
	}

	const TCHAR *avatartype = ProtoGetAvatarExtension(getByte(hContact, GG_KEY_AVATARTYPE, GG_KEYDEF_AVATARTYPE));

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
	mir_snprintf(szUrl, SIZEOF(szUrl), "http://api.gadu-gadu.pl/avatars/%d/0.xml", uin);

	NETLIBHTTPREQUEST req = { sizeof(req) };
	req.requestType = REQUEST_GET;
	req.szUrl = szUrl;
	req.flags = NLHRF_NODUMP | NLHRF_HTTP11 | NLHRF_REDIRECT;

	NETLIBHTTPREQUEST *resp = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)m_hNetlibUser, (LPARAM)&req);
	if (resp == NULL) {
		debugLogA("getAvatarFileInfo(): No response from HTTP request");
		return false;
	}

	if (resp->resultCode != 200 || !resp->dataLength || !resp->pData) {
		debugLogA("getAvatarFileInfo(): Invalid response code from HTTP request");
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
		return false;
	}

	if ((strncmp(resp->pData, "<result>", 8) == 0) || (strncmp(resp->pData, "<?xml", 5) == 0)){

		//if this url returned xml data (before and after 11.2013 gg convention)
		TCHAR *xmlAction = mir_a2t(resp->pData);
		HXML hXml = xi.parseString(xmlAction, 0, _T("result"));
		if (hXml != NULL) {
			HXML node = xi.getChildByPath(hXml, _T("users/user/avatars/avatar"), 0);
			const TCHAR *blank = (node != NULL) ? xi.getAttrValue(node, _T("blank")) : NULL;
			if (blank != NULL && mir_tstrcmp(blank, _T("1"))) {
				node = xi.getChildByPath(hXml, _T("users/user/avatars/avatar/timestamp"), 0);
				*avatarts = node != NULL ? mir_t2a(xi.getText(node)) : NULL;
				node = xi.getChildByPath(hXml, _T("users/user/avatars/avatar/bigavatar"), 0); //new gg convention
				if (node == NULL){
					node = xi.getChildByPath(hXml, _T("users/user/avatars/avatar/originBigAvatar"), 0); //old gg convention
				}
				*avatarurl = node != NULL ? mir_t2a(xi.getText(node)) : NULL;
			}
			xi.destroyNode(hXml);
		}
		mir_free(xmlAction);

	} else if (strncmp(resp->pData, "{\"result\":", 10) == 0){

		//if this url returns json data (11.2013 gg convention)
		JSONROOT respJSON(resp->pData);
		if (respJSON != NULL) {
			JSONNODE* respJSONavatars = json_get(json_get(json_get(json_get(respJSON, "result"), "users"), "user"), "avatars");
			if (respJSONavatars != NULL) {
				JSONNODE *respJSONavatar = json_at(respJSONavatars, 0);
				ptrT respJSON_blank(json_as_string(json_get(respJSONavatar, "_blank")));
				ptrT respJSONoriginBigAvatar(json_as_string(json_get(respJSONavatar, "originBigAvatar")));
				ptrT respJSONtimestamp(json_as_string(json_get(respJSONavatar, "timestamp")));
				if (respJSON_blank && mir_tstrcmp(respJSON_blank, TEXT("1")) && respJSONoriginBigAvatar && respJSONtimestamp){
					*avatarurl = mir_t2a(respJSONoriginBigAvatar);
					*avatarts = mir_t2a(respJSONtimestamp);
				}
			}
		}

	} else {
		debugLogA("getAvatarFileInfo(): Invalid response code from HTTP request, unknown format");
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
		return false;
	}

	CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
	return true;
}

char *gg_avatarhash(char *param)
{
	char *result;

	if (param == NULL || (result = (char *)mir_alloc(MIR_SHA1_HASH_SIZE * 2 + 1)) == NULL)
		return NULL;

	BYTE digest[MIR_SHA1_HASH_SIZE];
	mir_sha1_hash((BYTE*)param, (int)mir_strlen(param), digest);
	return bin2hex(digest, sizeof(digest), result);
}

void GGPROTO::requestAvatarTransfer(MCONTACT hContact, char *szAvatarURL)
{
	if (pth_avatar.dwThreadId == NULL) {
		debugLogA("requestAvatarTransfer(): Can not list_add element to avatar_transfers list. No pth_avatar.dwThreadId");
		return;
	}

	gg_EnterCriticalSection(&avatar_mutex, "requestAvatarTransfer", 1, "avatar_mutex", 1);
	if (avatar_transfers.getIndex((GGGETAVATARDATA*)&hContact) == -1) {
		GGGETAVATARDATA *data = (GGGETAVATARDATA*)mir_alloc(sizeof(GGGETAVATARDATA) + mir_strlen(szAvatarURL)+1);
		data->hContact = hContact;
		data->szAvatarURL = mir_strcpy((char*)(data+1), szAvatarURL);
		avatar_transfers.insert(data);
	}
	gg_LeaveCriticalSection(&avatar_mutex, "requestAvatarTransfer", 1, 1, "avatar_mutex", 1);
}

void GGPROTO::requestAvatarInfo(MCONTACT hContact, int iWaitFor)
{
	if (pth_avatar.dwThreadId == NULL) {
		debugLogA("requestAvatarInfo(): Can not list_add element to avatar_requests list. No pth_avatar.dwThreadId");
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
	debugLogA("avatarrequestthread() started. Avatar Request Thread Starting");
	while (pth_avatar.dwThreadId)
	{
		gg_EnterCriticalSection(&avatar_mutex, "avatarrequestthread", 3, "avatar_mutex", 1);
		if (avatar_requests.getCount()) {
			GGREQUESTAVATARDATA *data = avatar_requests[0];
			int iWaitFor = data->iWaitFor;
			MCONTACT hContact = data->hContact;
			avatar_requests.remove(0);
			mir_free(data);
			gg_LeaveCriticalSection(&avatar_mutex, "avatarrequestthread", 3, 1, "avatar_mutex", 1);
			
			uin_t uin = (uin_t)getDword(hContact, GG_KEY_UIN, 0);
			debugLogA("avatarrequestthread() new avatar_requests item for uin=%d.", uin);

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
			debugLogA("avatarrequestthread() new avatar_transfers item for url=%s.", data->szAvatarURL);

			int result = 0;

			PROTO_AVATAR_INFORMATIONT pai = { sizeof(pai) };
			pai.hContact = data->hContact;
			pai.format = getByte(pai.hContact, GG_KEY_AVATARTYPE, GG_KEYDEF_AVATARTYPE);

			NETLIBHTTPREQUEST req = { sizeof(req) };
			req.requestType = REQUEST_GET;
			req.szUrl = data->szAvatarURL;
			req.flags = NLHRF_NODUMP | NLHRF_HTTP11 | NLHRF_REDIRECT;

			NETLIBHTTPREQUEST *resp = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)m_hNetlibUser, (LPARAM)&req);
			if (resp) {
				if (resp->resultCode == 200 && resp->dataLength > 0 && resp->pData) {
					int file_fd;

					int avatarType = PA_FORMAT_UNKNOWN;
					if (strncmp(resp->pData,"\xFF\xD8",2) == 0) avatarType = PA_FORMAT_JPEG;
					if (strncmp(resp->pData,"\x47\x49\x46\x38",4) == 0) avatarType = PA_FORMAT_GIF;
					if (strncmp(resp->pData,"\x89\x50\x4E\x47\x0D\x0A\x1A\x0A",8) == 0) avatarType = PA_FORMAT_PNG;
					setByte(data->hContact, GG_KEY_AVATARTYPE, (BYTE)avatarType);

					getAvatarFilename(pai.hContact, pai.filename, sizeof(pai.filename));
					file_fd = _topen(pai.filename, _O_WRONLY | _O_TRUNC | _O_BINARY | _O_CREAT, _S_IREAD | _S_IWRITE);
					if (file_fd != -1) {
						_write(file_fd, resp->pData, resp->dataLength);
						_close(file_fd);
						result = 1;
						debugLog(_T("avatarrequestthread() new avatar_transfers item. Saved data from url=%s to file=%s."), data->szAvatarURL, pai.filename);
					} else {
						debugLog(_T("avatarrequestthread(): _topen file %s error. errno=%d: %s"), pai.filename, errno, strerror(errno));
						TCHAR error[512];
						mir_sntprintf(error, SIZEOF(error), TranslateT("Cannot create avatar file. ERROR: %d: %s\n%s"), errno, _tcserror(errno), pai.filename);
						showpopup(m_tszUserName, error, GG_POPUP_ERROR);
					}
				}
				else debugLogA("avatarrequestthread(): Invalid response code from HTTP request");
				CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
			}
			else debugLogA("avatarrequestthread(): No response from HTTP request");

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
	debugLogA("avatarrequestthread(): end. Avatar Request Thread Ending");
}

void GGPROTO::initavatarrequestthread()
{
	DWORD exitCode = 0;

	GetExitCodeThread(pth_avatar.hThread, &exitCode);
	if (exitCode != STILL_ACTIVE) {
		avatar_requests.destroy();
		avatar_transfers.destroy();
#ifdef DEBUGMODE
		debugLogA("initavatarrequestthread(): ForkThreadEx 1 GGPROTO::avatarrequestthread");
#endif
		pth_avatar.hThread = ForkThreadEx(&GGPROTO::avatarrequestthread, NULL, &pth_avatar.dwThreadId);
	}
}

void __cdecl GGPROTO::getOwnAvatarThread(void*)
{
	debugLogA("getOwnAvatarThread() started");

	char *AvatarURL, *AvatarTs;
	if (getAvatarFileInfo(getDword(GG_KEY_UIN, 0), &AvatarURL, &AvatarTs)) {
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
	debugLogA("getOwnAvatarThread(): end");
#endif
}

void GGPROTO::getOwnAvatar()
{
	if (getByte(GG_KEY_ENABLEAVATARS, GG_KEYDEF_ENABLEAVATARS) && getDword(GG_KEY_UIN, 0)){
#ifdef DEBUGMODE
		debugLogA("getOwnAvatar(): ForkThread 2 GGPROTO::getOwnAvatarThread");
#endif
		ForkThread(&GGPROTO::getOwnAvatarThread, NULL);
	}
}

void __cdecl GGPROTO::setavatarthread(void *param)
{

	debugLogA("setavatarthread(): started. Trying to set user avatar.");

	//read file
	TCHAR *szFilename = (TCHAR*)param;
	int file_fd = _topen(szFilename, _O_RDONLY | _O_BINARY, _S_IREAD);
	if (file_fd == -1) {
		debugLogA("setavatarthread(): Failed to open avatar file errno=%d: %s.", errno, strerror(errno));
		TCHAR error[512];
		mir_sntprintf(error, SIZEOF(error), TranslateT("Cannot open avatar file. ERROR: %d: %s\n%s"), errno, _tcserror(errno), szFilename);
		showpopup(m_tszUserName, error, GG_POPUP_ERROR);
		mir_free(szFilename);
		int prevType = getByte(GG_KEY_AVATARTYPEPREV, -1);
		if (prevType != -1)
			setByte(GG_KEY_AVATARTYPE, prevType);
		delSetting(GG_KEY_AVATARTYPEPREV);
		getOwnAvatar();
		#ifdef DEBUGMODE
		debugLogA("setavatarthread(): end. err1");
		#endif
		return;
	}

	long avatarFileLen = _filelength(file_fd);
	char* avatarFile = (char*)mir_alloc(avatarFileLen);
	_read(file_fd, avatarFile, avatarFileLen);
	_close(file_fd);

	ptrA avatarFileB64(mir_base64_encode((PBYTE)avatarFile, avatarFileLen));
	mir_free(avatarFile);

	ptrA avatarFileB64Enc(mir_urlEncode(avatarFileB64));
	size_t avatarFileB64EncLen = mir_strlen(avatarFileB64Enc);

	char dataPrefix[64];
	mir_snprintf(dataPrefix, SIZEOF(dataPrefix), "uin=%d&photo=", getDword(GG_KEY_UIN, 0));
	size_t dataPrefixLen = mir_strlen(dataPrefix);

	size_t dataLen = dataPrefixLen + avatarFileB64EncLen;
	char* data = (char*)mir_alloc(dataLen);
	memcpy(data, dataPrefix, dataPrefixLen);
	memcpy(data + dataPrefixLen, avatarFileB64Enc, avatarFileB64EncLen);

	//check if we have token, if no - receive new one
	oauth_checktoken(0);
	char* token = getStringA(GG_KEY_TOKEN);

	//construct request
	NETLIBHTTPREQUEST req = {0};
	req.cbSize = sizeof(req);
	req.requestType = REQUEST_POST;
	req.szUrl = "http://avatars.nowe.gg/upload";
	req.flags = NLHRF_NODUMP | NLHRF_HTTP11;
	req.headersCount = 10;
	NETLIBHTTPHEADER httpHeaders[10];
	httpHeaders[0].szName  = "X-Request";
	httpHeaders[0].szValue = "JSON";
	httpHeaders[1].szName  = "Authorization";
	httpHeaders[1].szValue = token;
	httpHeaders[2].szName  = "X-Requested-With";
	httpHeaders[2].szValue = "XMLHttpRequest";
	httpHeaders[3].szName  = "From";
	httpHeaders[3].szValue = "avatars to avatars";
	httpHeaders[4].szName  = "X-IM-Web-App-Version";
	httpHeaders[4].szValue = "10,5,2,13164";
	httpHeaders[5].szName  = "User-Agent";
	httpHeaders[5].szValue = "avatars to avatars";
	httpHeaders[6].szName  = "From";
	httpHeaders[6].szValue = NETLIB_USER_AGENT;
	httpHeaders[7].szName  = "Content-type";
	httpHeaders[7].szValue = "application/x-www-form-urlencoded; charset=utf-8";
	httpHeaders[8].szName  = "Accept";
	httpHeaders[8].szValue = "application/json";
	httpHeaders[9].szName  = "Referer";
	httpHeaders[9].szValue = "http://avatars.nowe.gg/.static/index_new_22.0.2_595nwh.html";
	req.headers = httpHeaders;
	req.pData = data;
	req.dataLength = int(dataLen);

	//send request
	int resultSuccess = 0;
	int needRepeat = 0;
	NETLIBHTTPREQUEST* resp = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)m_hNetlibUser, (LPARAM)&req);
	if (resp) {
		if (resp->resultCode == 200 && resp->dataLength > 0 && resp->pData) {
			debugLogA("setavatarthread(): 1 resp.data= %s", resp->pData);
			resultSuccess = 1;
		} else {
			debugLogA("setavatarthread() Invalid response code from HTTP request [%d]", resp->resultCode);
			if (resp->resultCode == 399 || resp->resultCode == 403 || resp->resultCode == 401){
				needRepeat = 1;
			}
		}
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
	} else {
		debugLogA("setavatarthread(): No response from HTTP request");
	}

	//check if we should repeat request
	if (needRepeat) { 
		// Access Token expired - force obtain new
		oauth_checktoken(1);
		mir_free(token);
		token = getStringA(GG_KEY_TOKEN);
		httpHeaders[1].szValue = token;

		//construct 2nd request
		memset(&req, 0, sizeof(req));
		req.cbSize = sizeof(req);
		req.requestType = REQUEST_POST;
		req.szUrl = "http://avatars.nowe.gg/upload";
		req.flags = NLHRF_NODUMP | NLHRF_HTTP11;
		req.headersCount = 10;
		req.headers = httpHeaders;
		req.pData = data;
		req.dataLength = int(dataLen);

		resp = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)m_hNetlibUser, (LPARAM)&req);
		if (resp) {
			if (resp->resultCode == 200 && resp->dataLength > 0 && resp->pData) {
				debugLogA("setavatarthread(): 2 resp.data= %s", resp->pData);
				resultSuccess = 1;
			} else {
				debugLogA("setavatarthread(): Invalid response code from HTTP request [%d]", resp->resultCode);
			}
			CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
		} else {
			debugLogA("setavatarthread(): No response from HTTP request");
		}
	}

	//clean and end thread
	mir_free(token);
	mir_free(data);

	if (resultSuccess) {
		debugLogA("setavatarthread(): User avatar set successfully.");
	} else {
		int prevType = getByte(GG_KEY_AVATARTYPEPREV, -1);
		if (prevType != -1)
			setByte(GG_KEY_AVATARTYPE, prevType);
		debugLogA("setavatarthread(): Failed to set user avatar.");
	}
	delSetting(GG_KEY_AVATARTYPEPREV);

	mir_free(szFilename);
	getOwnAvatar();
	#ifdef DEBUGMODE
	debugLogA("setavatarthread(): end.");
	#endif

}

void GGPROTO::setAvatar(const TCHAR *szFilename)
{
#ifdef DEBUGMODE
	debugLogA("setAvatar(): ForkThread 3 GGPROTO::setavatarthread");
#endif
	ForkThread(&GGPROTO::setavatarthread, mir_tstrdup(szFilename));
}
