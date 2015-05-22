/*

Tlen Protocol Plugin for Miranda NG
Copyright (C) 2004-2007  Piotr Piastucki

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/


#include "tlen.h"
#include "tlen_list.h"
#include "tlen_avatar.h"
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

/* TlenGetAvatarFileName() - gets a file name for the avatar image */

void TlenGetAvatarFileName(TlenProtocol *proto, TLEN_LIST_ITEM *item, TCHAR* ptszDest, int cbLen)
{
	
	int tPathLen = mir_sntprintf(ptszDest, cbLen, TEXT("%s\\%S"), VARST( TEXT("%miranda_avatarcache%")), proto->m_szModuleName);
	if (_taccess(ptszDest, 0)) {
		int ret = CreateDirectoryTreeT(ptszDest);
		if (ret == 0)
			proto->debugLog(_T("getAvatarFilename(): Created new directory for avatar cache: %s."), ptszDest);
		else {
			proto->debugLog(_T("getAvatarFilename(): Can not create directory for avatar cache: %s. errno=%d: %s"), ptszDest, errno, strerror(errno));
			TCHAR buffer[512];
			mir_sntprintf(buffer, SIZEOF(buffer), TranslateT("Cannot create avatars cache directory. ERROR: %d: %s\n%s"), errno, _tcserror(errno), ptszDest);
			PUShowMessageT(buffer, SM_WARNING);
		}
	}

	int format = PA_FORMAT_PNG;

	ptszDest[ tPathLen++ ] = '\\';
	if (item != NULL)
		format = item->avatarFormat;
	else if (proto->threadData != NULL)
		format = proto->threadData->avatarFormat;
	else
		format = db_get_dw(NULL, proto->m_szModuleName, "AvatarFormat", PA_FORMAT_UNKNOWN);

	const TCHAR *tszFileType = ProtoGetAvatarExtension(format);
	if ( item != NULL )
		mir_sntprintf(ptszDest + tPathLen, MAX_PATH - tPathLen, TEXT("%S%s"), ptrA( TlenSha1(item->jid)), tszFileType);
	else
		mir_sntprintf(ptszDest + tPathLen, MAX_PATH - tPathLen, TEXT("%S_avatar%s"), proto->m_szModuleName, tszFileType);
}

static void RemoveAvatar(TlenProtocol *proto, MCONTACT hContact) {
	TCHAR tFileName[ MAX_PATH ];
	if (hContact == NULL) {
		proto->threadData->avatarHash[0] = '\0';
	}
	TlenGetAvatarFileName( proto, NULL, tFileName, SIZEOF(tFileName)-1);
	DeleteFile(tFileName);
	db_unset(hContact, "ContactPhoto", "File");
	db_unset(hContact, proto->m_szModuleName, "AvatarHash");
	db_unset(hContact, proto->m_szModuleName, "AvatarFormat");
	ProtoBroadcastAck(proto->m_szModuleName, NULL, ACKTYPE_AVATAR, ACKRESULT_STATUS, NULL, 0);
}

static void SetAvatar(TlenProtocol *proto, MCONTACT hContact, TLEN_LIST_ITEM *item, char *data, int len, DWORD format) {
	TCHAR filename[MAX_PATH];
	char md5[33];
	mir_md5_state_t ctx;
	DWORD digest[4];

	if (format == PA_FORMAT_UNKNOWN && len > 4)
		format = ProtoGetBufferFormat(data);

	mir_md5_init( &ctx );
	mir_md5_append( &ctx, ( BYTE* )data, len);
	mir_md5_finish( &ctx, ( BYTE* )digest );

	sprintf( md5, "%08x%08x%08x%08x", (int)htonl(digest[0]), (int)htonl(digest[1]), (int)htonl(digest[2]), (int)htonl(digest[3])); //!!!!!!!!!!!!!!
	if (item != NULL) {
		char *hash = item->avatarHash;
		item->avatarFormat = format;
		item->avatarHash = mir_strdup(md5);
		mir_free(hash);
	} else {
		proto->threadData->avatarFormat = format;
		strcpy(proto->threadData->avatarHash, md5);
	}
	TlenGetAvatarFileName(proto, item, filename, SIZEOF(filename)-1);
	DeleteFile(filename);
	FILE *out = _tfopen(filename, TEXT("wb") );
	if (out != NULL) {
		fwrite(data, len, 1, out);
		fclose(out);
		db_set_ts(hContact, "ContactPhoto", "File", filename );
		db_set_s(hContact, proto->m_szModuleName, "AvatarHash",  md5);
		db_set_dw(hContact, proto->m_szModuleName, "AvatarFormat",  format);
	} else {
		TCHAR buffer[128];
		mir_sntprintf(buffer, SIZEOF(buffer), TranslateT("Cannot save new avatar file \"%s\" Error:\n\t%s (Error: %d)"), filename, _tcserror(errno), errno);
		PUShowMessageT(buffer, SM_WARNING);
		proto->debugLog(buffer);
		return;
	}
	ProtoBroadcastAck( proto->m_szModuleName, hContact, ACKTYPE_AVATAR, ACKRESULT_STATUS, NULL , 0);
}

int TlenProcessAvatarNode(TlenProtocol *proto, XmlNode *avatarNode, TLEN_LIST_ITEM *item) {
	XmlNode *aNode;
	char *oldHash = NULL;
	char *md5 = NULL, *type = NULL;
	MCONTACT hContact = NULL;
	if (item != NULL) {
		if ((hContact=TlenHContactFromJID(proto, item->jid)) == NULL) return 0;
	}
	if (item == NULL) {
		oldHash = proto->threadData->avatarHash;
	} else {
		oldHash = item->avatarHash;
	}
	if (avatarNode != NULL) {
		aNode = TlenXmlGetChild(avatarNode, "a");
		if (aNode != NULL) {
			type = TlenXmlGetAttrValue(aNode, "type");
			md5 = TlenXmlGetAttrValue(aNode, "md5");
		}
	}
	if (md5 != NULL) {
		/* check contact's avatar hash - md5 */
		if (oldHash == NULL || mir_strcmp(oldHash, md5)) {
			if (item != NULL) {
				item->newAvatarDownloading = TRUE;
			}
			TlenGetAvatar(proto, hContact);
			return 1;
		}
	} else {
		/* remove avatar */
		if (oldHash != NULL) {
			if (item != NULL) {
				item->avatarHash = NULL;
				mir_free(oldHash);
				item->newAvatarDownloading = FALSE;
			}
			RemoveAvatar(proto, hContact);
			return 1;
		}
	}
	return 0;
}

void TlenProcessPresenceAvatar(TlenProtocol *proto, XmlNode *node, TLEN_LIST_ITEM *item) {
	MCONTACT hContact=TlenHContactFromJID(proto, item->jid);
	if (hContact != NULL)
		TlenProcessAvatarNode(proto, TlenXmlGetChild(node, "avatar"), item);
}


static char *replaceTokens(const char *base, const char *uri, const char *login, const char* token, int type, int access) {
	char *result;
	int i, l, size;
	l = (int)mir_strlen(uri);
	size = (int)mir_strlen(base);
	for (i = 0; i < l; ) {
		if (!strncmp(uri + i, "^login^", 7)) {
			size += (int)mir_strlen(login);
			i += 7;
		} else if (!strncmp(uri + i, "^type^", 6)) {
			size++;
			i += 6;
		} else if (!strncmp(uri + i, "^token^", 7)) {
			size += (int)mir_strlen(token);
			i += 7;
		} else if (!strncmp(uri + i, "^access^", 8)) {
			size++;
			i += 8;
		} else {
			size++;
			i++;
		}
	}
	result = (char *)mir_alloc(size +1);
	strcpy(result, base);
	size = (int)mir_strlen(base);
	for (i = 0; i < l; ) {
		if (!strncmp(uri + i, "^login^", 7)) {
			strcpy(result + size, login);
			size += (int)mir_strlen(login);
			i += 7;
		} else if (!strncmp(uri + i, "^type^", 6)) {
			result[size++] = '0' + type;
			i += 6;
		} else if (!strncmp(uri + i, "^token^", 7)) {
			strcpy(result + size, token);
			size += (int)mir_strlen(token);
			i += 7;
		} else if (!strncmp(uri + i, "^access^", 8)) {
			result[size++] = '0' + access;
			i += 8;
		} else {
			result[size++] = uri[i++];
		}
	}
	result[size] = '\0';
	return result;
}


static int getAvatarMutex = 0;

typedef struct {
	TlenProtocol *proto;
	MCONTACT hContact;
} TLENGETAVATARTHREADDATA;

static void TlenGetAvatarThread(void *ptr) {

	TLEN_LIST_ITEM *item = NULL;
	TLENGETAVATARTHREADDATA *data = (TLENGETAVATARTHREADDATA *)ptr;
	MCONTACT hContact = data->hContact;
	char *login = NULL;
	if (hContact != NULL) {
		char *jid = TlenJIDFromHContact(data->proto, hContact);
		login = TlenNickFromJID(jid);
		item = TlenListGetItemPtr(data->proto, LIST_ROSTER, jid);
		mir_free(jid);
	} else {
		if (data->proto->threadData != NULL)
			login = mir_strdup(data->proto->threadData->username);
	}
	if ((data->proto->threadData != NULL && hContact == NULL) || item != NULL) {
		DWORD format = PA_FORMAT_UNKNOWN;
		if (item != NULL) {
			item->newAvatarDownloading = TRUE;
		}
		char *request = replaceTokens(data->proto->threadData->tlenConfig.mailBase, data->proto->threadData->tlenConfig.avatarGet, login, data->proto->threadData->avatarToken, 0, 0);
		NETLIBHTTPREQUEST req;
		memset(&req, 0, sizeof(req));
		req.cbSize = sizeof(req);
		req.requestType = data->proto->threadData->tlenConfig.avatarGetMthd;
		req.flags = 0;
		req.headersCount = 0;
		req.headers = NULL;
		req.dataLength = 0;
		req.szUrl = request;
		NETLIBHTTPREQUEST *resp = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)data->proto->m_hNetlibUser, (LPARAM)&req);
		if (item != NULL) {
			item->newAvatarDownloading = FALSE;
		}
		if (resp != NULL) {
			if (resp->resultCode/100 == 2) {
				if (resp->dataLength > 0) {
					int i;
					for (i=0; i<resp->headersCount; i++ ) {
						if (!strcmpi(resp->headers[i].szName, "Content-Type")) {
							if (!strcmpi(resp->headers[i].szValue, "image/png"))
								format = PA_FORMAT_PNG;
							else if (!strcmpi(resp->headers[i].szValue, "image/x-png"))
								format = PA_FORMAT_PNG;
							else if (!strcmpi(resp->headers[i].szValue, "image/jpeg"))
								format = PA_FORMAT_JPEG;
							else if (!strcmpi(resp->headers[i].szValue, "image/jpg"))
								format = PA_FORMAT_JPEG;
							else if (!strcmpi(resp->headers[i].szValue, "image/gif"))
								format = PA_FORMAT_GIF;
							else if (!strcmpi(resp->headers[i].szValue, "image/bmp"))
								format = PA_FORMAT_BMP;

							break;
						}
					}
					SetAvatar(data->proto, hContact, item, resp->pData, resp->dataLength, format);
				} else {
					RemoveAvatar(data->proto, hContact);
				}
			}
			CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
		}
		mir_free(request);
		mir_free(login);
	}
	if (hContact == NULL) {
		getAvatarMutex = 0;
	} 
	mir_free(login);
	mir_free(data);
}

void TlenGetAvatar(TlenProtocol *proto, MCONTACT hContact) {
	if (hContact == NULL) {
		if (getAvatarMutex != 0) {
			return;
		}
		getAvatarMutex = 1;
	}
	{
		TLENGETAVATARTHREADDATA *data = (TLENGETAVATARTHREADDATA *)mir_alloc(sizeof(TLENGETAVATARTHREADDATA));
		data->proto = proto;
		data->hContact = hContact;
		forkthread(TlenGetAvatarThread, 0, data);
	}
}

typedef struct {
	TlenProtocol *proto;
	NETLIBHTTPREQUEST *req;
} TLENREMOVEAVATARTHREADDATA;

static void TlenRemoveAvatarRequestThread(void *ptr) {
	TLENREMOVEAVATARTHREADDATA *data = (TLENREMOVEAVATARTHREADDATA*)ptr;
	NETLIBHTTPREQUEST *req = (NETLIBHTTPREQUEST *)data->req;
	NETLIBHTTPREQUEST *resp = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)data->proto->m_hNetlibUser, (LPARAM)req);
	mir_free(req->szUrl);
	mir_free(req->headers);
	mir_free(req->pData);
	mir_free(req);
	if (resp != NULL) {
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
		RemoveAvatar(data->proto, NULL);
	}
	mir_free(data);

}

typedef struct {
	TlenProtocol *proto;
	NETLIBHTTPREQUEST *req;
	char *data;
	int  length;
} TLENUPLOADAVATARTHREADDATA;

boolean checkUploadAvatarResponse(TlenProtocol *proto, NETLIBHTTPREQUEST *resp){
	if (resp == NULL){
		proto->debugLogA("Error while setting avatar on Tlen account (no response)");
		PUShowMessageT(TranslateT("Error while setting avatar on Tlen account (no response)"), SM_WARNING);
		return false;
	}
	if (resp->resultCode != 200 || !resp->dataLength || !resp->pData) {
		proto->debugLogA("Error while setting avatar on Tlen account (invalid response) resultCode=%d, dataLength=%d", resp->resultCode, resp->dataLength);
		PUShowMessageT(TranslateT("Error while setting avatar on Tlen account (invalid response)"), SM_WARNING);
		return false;
	}
	if (strncmp(resp->pData, "<ok", 3)){
		proto->debugLogA("Error while setting avatar on Tlen account: %s", resp->pData);
		PUShowMessageT(TranslateT("Error while setting avatar on Tlen account"), SM_WARNING);
		return false;
	}
	return true;
}

static void TlenUploadAvatarRequestThread(void *ptr) {
	TLENUPLOADAVATARTHREADDATA *data = (TLENUPLOADAVATARTHREADDATA *) ptr;
	NETLIBHTTPREQUEST *req = data->req;
	NETLIBHTTPREQUEST *resp = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)data->proto->m_hNetlibUser, (LPARAM)req);
	if (checkUploadAvatarResponse(data->proto, resp)) {
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
		SetAvatar(data->proto, NULL, NULL, data->data, data->length, PA_FORMAT_PNG);
	}
	mir_free(req->szUrl);
	mir_free(req->headers);
	mir_free(req->pData);
	mir_free(req);
	mir_free(data->data);
	mir_free(data);
}

void TlenRemoveAvatar(TlenProtocol *proto) {
	if (proto->threadData != NULL) {
		TLENREMOVEAVATARTHREADDATA *data = (TLENREMOVEAVATARTHREADDATA *)mir_alloc(sizeof(TLENREMOVEAVATARTHREADDATA));
		NETLIBHTTPREQUEST *req = (NETLIBHTTPREQUEST *)mir_alloc(sizeof(NETLIBHTTPREQUEST));
		data->proto =proto;
		data->req = req;
		char *request = replaceTokens(proto->threadData->tlenConfig.mailBase, proto->threadData->tlenConfig.avatarRemove, "", proto->threadData->avatarToken, 0, 0);
		memset(req, 0, sizeof(NETLIBHTTPREQUEST));
		req->cbSize = sizeof(NETLIBHTTPREQUEST);
		req->requestType = proto->threadData->tlenConfig.avatarGetMthd;
		req->szUrl = request;
		forkthread(TlenRemoveAvatarRequestThread, 0, data);
	}
}


void TlenUploadAvatar(TlenProtocol *proto, unsigned char *data, int dataLen, int access) {
	NETLIBHTTPHEADER *headers;
	unsigned char *buffer;
	if (proto->threadData != NULL && dataLen > 0 && data != NULL) {
		char *mpartHead =  "--AaB03x\r\nContent-Disposition: form-data; name=\"filename\"; filename=\"plik.png\"\r\nContent-Type: image/png\r\n\r\n";
		char *mpartTail =  "\r\n--AaB03x--\r\n";
		int size, sizeHead = (int)mir_strlen(mpartHead), sizeTail = (int)mir_strlen(mpartTail);
		char *request = replaceTokens(proto->threadData->tlenConfig.mailBase, proto->threadData->tlenConfig.avatarUpload, "", proto->threadData->avatarToken, 0, access);
		TLENUPLOADAVATARTHREADDATA *threadData = (TLENUPLOADAVATARTHREADDATA *)mir_alloc(sizeof(TLENUPLOADAVATARTHREADDATA));
		threadData->proto = proto;
		NETLIBHTTPREQUEST *req = (NETLIBHTTPREQUEST *)mir_alloc(sizeof(NETLIBHTTPREQUEST));
		headers = (NETLIBHTTPHEADER *)mir_alloc(sizeof(NETLIBHTTPHEADER));
		memset(req, 0, sizeof(NETLIBHTTPREQUEST));
		req->cbSize = sizeof(NETLIBHTTPREQUEST);
		req->requestType = proto->threadData->tlenConfig.avatarUploadMthd;
		req->szUrl = request;
		req->flags = 0;
		headers[0].szName = "Content-Type";
		headers[0].szValue = "multipart/form-data; boundary=AaB03x";
		req->headersCount = 1;
		req->headers = headers;
		size = dataLen + sizeHead + sizeTail;
		buffer = (unsigned char *)mir_alloc(size);
		memcpy(buffer, mpartHead, sizeHead);
		memcpy(buffer + sizeHead, data, dataLen);
		memcpy(buffer + sizeHead + dataLen, mpartTail, sizeTail);
		req->dataLength = size;
		req->pData = (char*)buffer;
		threadData->req = req;
		threadData->data = (char *) mir_alloc(dataLen);
		memcpy(threadData->data, data, dataLen);
		threadData->length = dataLen;
		forkthread(TlenUploadAvatarRequestThread, 0, threadData);
	}
}

