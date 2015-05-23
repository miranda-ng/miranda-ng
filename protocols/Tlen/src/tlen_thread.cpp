/*

Jabber Protocol Plugin for Miranda IM
Tlen Protocol Plugin for Miranda NG
Copyright (C) 2002-2004  Santithorn Bunchua
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

#include "commons.h"
#include "tlen_list.h"
#include "tlen_iq.h"
#include "resource.h"
#include "tlen_p2p_old.h"
#include "tlen_file.h"
#include "tlen_muc.h"
#include "tlen_voice.h"
#include "tlen_avatar.h"
#include "tlen_presence.h"
#include "tlen_picture.h"
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>


extern void __cdecl TlenProcessP2P(XmlNode *node, ThreadData *info);


//static void __cdecl TlenProcessInvitation(struct ThreadData *info);
static void __cdecl TlenKeepAliveThread(void *ptr);
static void TlenProcessStreamOpening(XmlNode *node, ThreadData *info);
static void TlenProcessStreamClosing(XmlNode *node, ThreadData *info);
static void TlenProcessProtocol(XmlNode *node, ThreadData *info);
static void TlenProcessMessage(XmlNode *node, ThreadData *info);
static void TlenProcessIq(XmlNode *node, ThreadData *info);
static void TlenProcessW(XmlNode *node, ThreadData *info);
static void TlenProcessM(XmlNode *node, ThreadData *info);
static void TlenProcessN(XmlNode *node, ThreadData *info);
static void TlenProcessP(XmlNode *node, ThreadData *info);
static void TlenProcessV(XmlNode *node, ThreadData *info);
static void TlenProcessAvatar(XmlNode* node, ThreadData *info);
static void TlenProcessCipher(XmlNode *node, ThreadData *info);

static VOID NTAPI TlenDummyApcFunc(ULONG_PTR param)
{
	return;
}

static char onlinePassword[128];
static HANDLE hEventPasswdDlg;

static INT_PTR CALLBACK TlenPasswordDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char text[128];

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		mir_snprintf(text, SIZEOF(text), Translate("Enter password for %s"), (char *) lParam);
		SetDlgItemTextA(hwndDlg, IDC_JID, text);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			GetDlgItemTextA(hwndDlg, IDC_PASSWORD, onlinePassword, SIZEOF(onlinePassword));
			//EndDialog(hwndDlg, (int) onlinePassword);
			//return TRUE;
			// Fall through
		case IDCANCEL:
			//EndDialog(hwndDlg, 0);
			SetEvent(hEventPasswdDlg);
			DestroyWindow(hwndDlg);
			return TRUE;
		}
		break;
	}

	return FALSE;
}

static VOID NTAPI TlenPasswordCreateDialogApcProc(ULONG_PTR param)
{
	CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_PASSWORD), NULL, TlenPasswordDlgProc, (LPARAM) param);
}

void __cdecl TlenServerThread(ThreadData *info)
{
	char *connectHost;
	int datalen;
	XmlState xmlState;
	int tlenNetworkBufferSize;
	int oldStatus = ID_STATUS_OFFLINE;
	int reconnectMaxTime;
	int numRetry;
	int reconnectTime;
	int loginErr = 0;
	info->proto->debugLogA("Thread started");

	// Normal server connection, we will fetch all connection parameters
	// e.g. username, password, etc. from the database.

	if (info->proto->threadData != NULL) {
		// Will not start another connection thread if a thread is already running.
		// Make APC call to the main thread. This will immediately wake the thread up
		// in case it is asleep in the reconnect loop so that it will immediately
		// reconnect.
		QueueUserAPC(TlenDummyApcFunc, info->proto->threadData->hThread, 0);
		info->proto->debugLogA("Thread ended, another normal thread is running");
		mir_free(info);
		return;
	}

	info->proto->threadData = info;

	DBVARIANT dbv;
	if (!db_get(NULL, info->proto->m_szModuleName, "LoginName", &dbv)) {
		strncpy(info->username, dbv.pszVal, sizeof(info->username));
		info->username[sizeof(info->username)-1] = '\0';
		_strlwr(info->username);
		db_set_s(NULL, info->proto->m_szModuleName, "LoginName", info->username);
		db_free(&dbv);

	} else {
		info->proto->debugLogA("Thread ended, login name is not configured");
		loginErr = LOGINERR_BADUSERID;
	}

	if (loginErr == 0) {
		if (!db_get(NULL, info->proto->m_szModuleName, "LoginServer", &dbv)) {
			strncpy(info->server, dbv.pszVal, sizeof(info->server));
			info->server[sizeof(info->server)-1] = '\0';
			_strlwr(info->server);
			db_set_s(NULL, info->proto->m_szModuleName, "LoginServer", info->server);
			db_free(&dbv);
		} else {
			info->proto->debugLogA("Thread ended, login server is not configured");
			loginErr = LOGINERR_NONETWORK;
		}
	}
	
	char jidStr[128];
	if (loginErr == 0) {
		if (!info->proto->tlenOptions.savePassword) {
			// Ugly hack: continue logging on only the return value is &(onlinePassword[0])
			// because if WM_QUIT while dialog box is still visible, p is returned with some
			// exit code which may not be NULL.
			// Should be better with modeless.
			onlinePassword[0] = (char) -1;
			hEventPasswdDlg = CreateEvent(NULL, FALSE, FALSE, NULL);
			QueueUserAPC(TlenPasswordCreateDialogApcProc, hMainThread, (DWORD) jidStr);
			WaitForSingleObject(hEventPasswdDlg, INFINITE);
			CloseHandle(hEventPasswdDlg);
			
			if (onlinePassword[0] != (char) -1) {
				strncpy(info->password, onlinePassword, sizeof(info->password));
				info->password[sizeof(info->password)-1] = '\0';
			}
			else {
				info->proto->debugLogA("Thread ended, password request dialog was canceled");
				loginErr = LOGINERR_BADUSERID;
			}
		}
		else {
			if (!db_get(NULL, info->proto->m_szModuleName, "Password", &dbv)) {
				strncpy(info->password, dbv.pszVal, sizeof(info->password));
				info->password[sizeof(info->password)-1] = '\0';
				db_free(&dbv);
			}
			else {
				info->proto->debugLogA("Thread ended, password is not configured");
				loginErr = LOGINERR_BADUSERID;
			}
		}
	}

	tlenNetworkBufferSize = 2048;
	char *buffer = (char *) mir_alloc(tlenNetworkBufferSize+1);	// +1 is for '\0' when debug logging this buffer
	if (buffer == NULL) {
		info->proto->debugLogA("Thread ended, network buffer cannot be allocated");
		loginErr = LOGINERR_NONETWORK;
	}

	if (loginErr != 0) {
		info->proto->threadData = NULL;
		oldStatus = info->proto->m_iStatus;
		info->proto->m_iStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(info->proto->m_szModuleName, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldStatus, info->proto->m_iStatus);
		ProtoBroadcastAck(info->proto->m_szModuleName, NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, loginErr);
		mir_free(info);
		mir_free(buffer);
		return;
	}
	
	mir_snprintf(jidStr, SIZEOF(jidStr), "%s@%s", info->username, info->server);
	db_set_s(NULL, info->proto->m_szModuleName, "jid", jidStr);

	if (!db_get(NULL, info->proto->m_szModuleName, "ManualHost", &dbv)) {
		strncpy(info->manualHost, dbv.pszVal, sizeof(info->manualHost));
		info->manualHost[sizeof(info->manualHost)-1] = '\0';
		db_free(&dbv);
	}
	info->port = db_get_w(NULL, info->proto->m_szModuleName, "ManualPort", TLEN_DEFAULT_PORT);
	info->useEncryption = info->proto->tlenOptions.useEncryption;

	if (info->manualHost[0])
		connectHost = info->manualHost;
	else
		connectHost = info->server;

	info->proto->debugLogA("Thread server='%s' port='%d'", connectHost, info->port);


	if (!db_get(NULL, info->proto->m_szModuleName, "AvatarHash", &dbv)) {
		strncpy(info->proto->threadData->avatarHash, dbv.pszVal, sizeof(info->proto->threadData->avatarHash)-1);
		db_free(&dbv);
	}
	info->avatarFormat = db_get_dw(NULL, info->proto->m_szModuleName, "AvatarFormat", PA_FORMAT_UNKNOWN);


	reconnectMaxTime = 10;
	numRetry = 0;

	for (;;) {	// Reconnect loop

		info->s = TlenWsConnect(info->proto, connectHost, info->port);
		if (info->s == NULL) {
			info->proto->debugLogA("Connection failed (%d)", WSAGetLastError());
			if (info->proto->threadData == info) {
				oldStatus = info->proto->m_iStatus;
				info->proto->m_iStatus = ID_STATUS_OFFLINE;
				ProtoBroadcastAck(info->proto->m_szModuleName, NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_NONETWORK);
				ProtoBroadcastAck(info->proto->m_szModuleName, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldStatus, info->proto->m_iStatus);
				if (info->proto->tlenOptions.reconnect == TRUE) {
					reconnectTime = rand() % reconnectMaxTime;
					info->proto->debugLogA("Sleeping %d seconds before automatic reconnecting...", reconnectTime);
					SleepEx(reconnectTime * 1000, TRUE);
					if (reconnectMaxTime < 10*60)	// Maximum is 10 minutes
						reconnectMaxTime *= 2;
					if (info->proto->threadData == info) {	// Make sure this is still the active thread for the main Tlen connection
						info->proto->debugLogA("Reconnecting to the network...");
						if (numRetry < MAX_CONNECT_RETRIES)
							numRetry++;
						oldStatus = info->proto->m_iStatus;
						info->proto->m_iStatus = ID_STATUS_CONNECTING + numRetry;
						ProtoBroadcastAck(info->proto->m_szModuleName, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldStatus, info->proto->m_iStatus);
						continue;
					}
					else {
						info->proto->debugLogA("Thread ended, connection failed");
						mir_free(buffer);
						mir_free(info);
						return;
					}
				}
				info->proto->threadData = NULL;
			}
			info->proto->debugLogA("Thread ended, connection failed");
			mir_free(buffer);
			mir_free(info);
			return;
		}

		// User may change status to OFFLINE while we are connecting above
		if (info->proto->m_iDesiredStatus != ID_STATUS_OFFLINE) {

			info->proto->isConnected = TRUE;
			forkthread(TlenKeepAliveThread, 0, info->proto);

			TlenXmlInitState(&xmlState);
			TlenXmlSetCallback(&xmlState, 1, ELEM_OPEN, (void (__cdecl *)(XmlNode *,void *))TlenProcessStreamOpening, info);
			TlenXmlSetCallback(&xmlState, 1, ELEM_CLOSE, (void (__cdecl *)(XmlNode *,void *))TlenProcessStreamClosing, info);
			TlenXmlSetCallback(&xmlState, 2, ELEM_CLOSE, (void (__cdecl *)(XmlNode *,void *))TlenProcessProtocol, info);

			info->useAES = FALSE;

			if (info->useEncryption) {
				TlenSend(info->proto, "<s s='1' v='9' t='06000106'>");

			} else {
				TlenSend(info->proto, "<s v='3'>");
			}

			info->proto->debugLogA("Entering main recv loop");
			datalen = 0;

			for (;;) {
				int recvResult, bytesParsed;

				if (info->useAES) {
					recvResult = TlenWsRecvAES(info->proto, buffer+datalen, tlenNetworkBufferSize-datalen, &info->aes_in_context, info->aes_in_iv);
				} else {
					recvResult = TlenWsRecv(info->proto, info->s, buffer+datalen, tlenNetworkBufferSize-datalen);
				}

				if (recvResult <= 0)
					break;
				datalen += recvResult;

				buffer[datalen] = '\0';
				info->proto->debugLogA("RECV:%s", buffer);

				bytesParsed = TlenXmlParse(&xmlState, buffer, datalen);
				info->proto->debugLogA("bytesParsed = %d", bytesParsed);
				if (bytesParsed > 0) {
					if (bytesParsed < datalen)
						memmove(buffer, buffer+bytesParsed, datalen-bytesParsed);
					datalen -= bytesParsed;
				}
				else if (datalen == tlenNetworkBufferSize) {
					tlenNetworkBufferSize += 2048;
					info->proto->debugLogA("Increasing network buffer size to %d", tlenNetworkBufferSize);
					if ((buffer=(char *) mir_realloc(buffer, tlenNetworkBufferSize+1)) == NULL) {
						info->proto->debugLogA("Cannot reallocate more network buffer, go offline now");
						break;
					}
				}
				else {
					info->proto->debugLogA("Unknown state: bytesParsed=%d, datalen=%d, tlenNetworkBufferSize=%d", bytesParsed, datalen, tlenNetworkBufferSize);
				}
			}

			TlenXmlDestroyState(&xmlState);

			info->proto->isOnline = FALSE;
			info->proto->isConnected = FALSE;

			CLISTMENUITEM mi = { sizeof(mi) };
			mi.flags = CMIM_FLAGS | CMIF_GRAYED;
			Menu_ModifyItem(info->proto->hMenuMUC, &mi);
			if (info->proto->hMenuChats != NULL)
				Menu_ModifyItem(info->proto->hMenuChats, &mi);

			// Set status to offline
			char *szProto = info->proto->m_szModuleName;
			oldStatus = info->proto->m_iStatus;
			info->proto->m_iStatus = ID_STATUS_OFFLINE;
			ProtoBroadcastAck(szProto, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldStatus, info->proto->m_iStatus);

			// Set all contacts to offline
			for (MCONTACT hContact = db_find_first(szProto); hContact; hContact = db_find_next(hContact, szProto))
				if (db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE)
					db_set_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);

			TlenListWipeSpecial(info->proto);
		}
		else {
			oldStatus = info->proto->m_iStatus;
			info->proto->m_iStatus = ID_STATUS_OFFLINE;
			ProtoBroadcastAck(info->proto->m_szModuleName, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldStatus, info->proto->m_iStatus);
		}

		Netlib_CloseHandle(info->s);

		if (info->proto->tlenOptions.reconnect == FALSE)
			break;

		if (info->proto->threadData != info)	// Make sure this is still the main Tlen connection thread
			break;
		reconnectTime = rand() % 10;
		info->proto->debugLogA("Sleeping %d seconds before automatic reconnecting...", reconnectTime);
		SleepEx(reconnectTime * 1000, TRUE);
		reconnectMaxTime = 20;
		if (info->proto->threadData != info)	// Make sure this is still the main Tlen connection thread
			break;
		info->proto->debugLogA("Reconnecting to the network...");
		info->proto->m_iDesiredStatus = oldStatus;	// Reconnect to my last status
		oldStatus = info->proto->m_iStatus;
		info->proto->m_iStatus = ID_STATUS_CONNECTING;
		numRetry = 1;
		ProtoBroadcastAck(info->proto->m_szModuleName, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldStatus, info->proto->m_iStatus);
	}

	info->proto->debugLogA("Thread ended: server='%s'", info->server);

	if (info->proto->threadData == info) {
		info->proto->threadData = NULL;
	}

	mir_free(buffer);
	if (info->streamId) mir_free(info->streamId);
	info->proto->debugLogA("Exiting ServerThread");
	mir_free(info);
}

static void TlenSendAuth(TlenProtocol *proto) {
	int iqId;
	char text[128];
	char *str = TlenPasswordHash(proto->threadData->password);
	mir_snprintf(text, SIZEOF(text), "%s%s", proto->threadData->streamId, str);
	mir_free(str);
	str = TlenSha1(text);
	char *p=TlenTextEncode(proto->threadData->username);
	if (p != NULL) {
		iqId = TlenSerialNext(proto->threadData->proto);
		TlenIqAdd(proto, iqId, IQ_PROC_NONE, TlenIqResultAuth);
		TlenSend(proto, "<iq type='set' id='"TLEN_IQID"%d'><query xmlns='jabber:iq:auth'><username>%s</username><digest>%s</digest><resource>t</resource><host>tlen.pl</host></query></iq>", iqId, p /*info->username*/, str);
		mir_free(p);
	}
	mir_free(str);
}

/* processing <s ... > tag sent from server on session opening */
static void TlenProcessStreamOpening(XmlNode *node, ThreadData *info)
{
	if (node->name == NULL || mir_strcmp(node->name, "s"))
		return;

	char *sid=TlenXmlGetAttrValue(node, "i");
	if (sid != NULL) {
		if (info->streamId) mir_free(info->streamId);
		info->streamId = mir_strdup(sid);
	}
	char *s=TlenXmlGetAttrValue(node, "s");
	if (s != NULL && !mir_strcmp(s, "1")) {
		int i;
		unsigned char aes_key[32];
		char aes_key_str[140], aes_iv_str[40];
		mpi k1_mpi, k2_mpi, aes_mpi;
		size_t slen;

		char *k1=TlenXmlGetAttrValue(node, "k1");
		char *k2=TlenXmlGetAttrValue(node, "k2");
		char *k3=TlenXmlGetAttrValue(node, "k3");

		memset(&info->aes_in_context, 0, sizeof (aes_context));
		memset(&info->aes_out_context, 0, sizeof (aes_context));
		memset(&aes_mpi, 0, sizeof (mpi));
		mpi_read_string(&aes_mpi, 16, k3);
		mpi_write_binary(&aes_mpi, info->aes_in_iv, 16);
		for (i = 0; i < 16; i++) {
			info->aes_out_iv[i] = rand();
			aes_key[i] = rand();
		}
		memset(&aes_mpi, 0, sizeof (mpi));
		mpi_read_binary(&aes_mpi, info->aes_out_iv, 16);
		slen = 40;
		mpi_write_string(&aes_mpi, 16, aes_iv_str, &slen);
		aes_setkey_dec(&info->aes_in_context, aes_key, 128);
		aes_setkey_enc(&info->aes_out_context, aes_key, 128);
		memset(&aes_mpi, 0, sizeof (mpi));
		mpi_read_binary(&aes_mpi, aes_key, 16);
		memset(&k1_mpi, 0, sizeof (mpi));
		mpi_read_string( &k1_mpi, 16, k1 );
		memset(&k2_mpi, 0, sizeof (mpi));
		mpi_read_string( &k2_mpi, 16, k2 );
		memset(&aes_mpi, 0, sizeof (mpi));
		mpi_read_binary(&aes_mpi, (unsigned char *)aes_key, 16);
		mpi_exp_mod( &aes_mpi, &aes_mpi, &k1_mpi, &k2_mpi, NULL );
		slen = 140;
		mpi_write_string(&aes_mpi, 16, aes_key_str, &slen);
		TlenSend(info->proto, "<cipher k1='%s' k2='%s'/>", aes_key_str, aes_iv_str);
	} else {
		TlenSendAuth(info->proto);
	}
}

/* processing </s> tag sent from server on session close */
static void TlenProcessStreamClosing(XmlNode *node, ThreadData *info)
{
	Netlib_CloseHandle(info->proto);
	if (node->name && !mir_strcmp(node->name, "stream:error") && node->text){
		char buffer[1024];
		mir_snprintf(buffer, SIZEOF(buffer), "%s\n%s", Translate("Tlen Connection Error"), Translate(node->text));
		PUShowMessage(buffer, SM_WARNING);
	} else if (!mir_strcmp(node->name, "s")){
		info->proto->debugLogA("Disconnected server message");
	}
}

/* processing session tags sent from server */
static void TlenProcessProtocol(XmlNode *node, ThreadData *info)
{
	if (!mir_strcmp(node->name, "message"))
		TlenProcessMessage(node, info);
	else if (!mir_strcmp(node->name, "presence"))
		TlenProcessPresence(node, info->proto);
	else if (!mir_strcmp(node->name, "iq"))
		TlenProcessIq(node, info);
	else if (!mir_strcmp(node->name, "f"))
		TlenProcessF(node, info);
	else if (!mir_strcmp(node->name, "w"))
		TlenProcessW(node, info);
	else if (!mir_strcmp(node->name, "m"))
		TlenProcessM(node, info);
	else if (!mir_strcmp(node->name, "n"))
		TlenProcessN(node, info);
	else if (!mir_strcmp(node->name, "p"))
		TlenProcessP(node, info);
	else if (!mir_strcmp(node->name, "v"))
		TlenProcessV(node, info);
	else if (!mir_strcmp(node->name, "avatar"))
		TlenProcessAvatar(node, info);
	else if (!mir_strcmp(node->name, "cipher"))
		TlenProcessCipher(node, info);
	else
		info->proto->debugLogA("Invalid top-level tag (only <message/> <presence/> <iq/> <f/> <w/> <m/> <n/> <p/> <v/> <cipher/> and <avatar/> allowed)");

}

static void TlenProcessCipher(XmlNode *node, ThreadData *info)
{
	char *type=TlenXmlGetAttrValue(node, "type");
	info->useAES = TRUE;
	TlenSend(info->proto, "<cipher type='ok'/>");
	TlenSendAuth(info->proto);
}

static void TlenProcessIqGetVersion(TlenProtocol *proto, XmlNode *node)
{
	OSVERSIONINFO osvi = { 0 };
	char mversion[256];
	char *mver;
	char* os = NULL;

	if (proto->m_iStatus == ID_STATUS_INVISIBLE) return;
	if (!proto->tlenOptions.enableVersion) return;
	char *from = TlenXmlGetAttrValue(node, "from");
	if (from == NULL)
		return;
	
	TLEN_LIST_ITEM *item = TlenListGetItemPtr(proto, LIST_ROSTER, from);
	if (item == NULL)
		return;

	char *version = TlenTextEncode(TLEN_VERSION_STRING);
	osvi.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
	if ( GetVersionEx( &osvi )) {
		switch ( osvi.dwPlatformId ) {
		case VER_PLATFORM_WIN32_NT:
			if ( osvi.dwMajorVersion == 5 ) {
				if ( osvi.dwMinorVersion == 2 ) os = TlenTextEncode("Windows Server 2003");
				else if ( osvi.dwMinorVersion == 1 ) os = TlenTextEncode("Windows XP");
				else if ( osvi.dwMinorVersion == 0 ) os = TlenTextEncode("Windows 2000");
			}
			else if ( osvi.dwMajorVersion <= 4 ) {
				os = TlenTextEncode("Windows NT");
			}
			break;
		case VER_PLATFORM_WIN32_WINDOWS:
			if ( osvi.dwMajorVersion == 4 ) {
				if ( osvi.dwMinorVersion == 0 ) os = TlenTextEncode("Windows 95");
				if ( osvi.dwMinorVersion == 10 ) os = TlenTextEncode("Windows 98");
				if ( osvi.dwMinorVersion == 90 ) os = TlenTextEncode("Windows ME");
			}
			break;
	}	}

	if ( os == NULL ) os = TlenTextEncode("Windows");

	mir_strcpy(mversion, "Miranda NG ");
	CallService(MS_SYSTEM_GETVERSIONTEXT, sizeof( mversion ) - 11, ( LPARAM )mversion + 11 );
	mir_strcat(mversion, " (Tlen v.");
	mir_strcat(mversion, TLEN_VERSION_STRING);
	mir_strcat(mversion, ")");
	mver = TlenTextEncode( mversion );
	TlenSend( proto, "<message to='%s' type='iq'><iq type='result'><query xmlns='jabber:iq:version'><name>%s</name><version>%s</version><os>%s</os></query></iq></message>", from, mver?mver:"", version?version:"", os?os:"" );
	if (!item->versionRequested) {
		item->versionRequested = TRUE;
		TlenSend(proto, "<message to='%s' type='iq'><iq type='get'><query xmlns='jabber:iq:version'/></iq></message>", from);
	}

	if ( mver ) mir_free( mver );
	if ( version ) mir_free( version );
	if ( os ) mir_free( os );
}

// Support for Tlen avatars - avatar token used to access web interface
static void TlenProcessAvatar(XmlNode* node, ThreadData *info)
{
	XmlNode *tokenNode = TlenXmlGetChild(node, "token");
	XmlNode *aNode = TlenXmlGetChild(node, "a");
	if (tokenNode != NULL) {
		char *token = tokenNode->text;
		strncpy(info->avatarToken, token, sizeof(info->avatarToken)-1);
	}
	if (aNode != NULL) {
		if (TlenProcessAvatarNode(info->proto, node, NULL)) {
		}
	}
}

static void TlenProcessMessage(XmlNode *node, ThreadData *info)
{
	MCONTACT hContact;
	XmlNode *bodyNode, *subjectNode, *xNode, *n;
	char *nick, *p, *localMessage, *idStr;
	DWORD msgTime;
	BOOL delivered, composing;
	int i;

	if (!node->name || mir_strcmp(node->name, "message")) return;

	char *type=TlenXmlGetAttrValue(node, "type");
	if (type != NULL && !mir_strcmp(type, "error")) {
	}
	else {
		char *from=TlenXmlGetAttrValue(node, "from");
		if (from != NULL) {
			if (info->proto->tlenOptions.ignoreAdvertisements && strstr(from, "b73@tlen.pl") == from) {
				return;
			}
			char *fromJid = TlenLoginFromJID(from);
			// If message is from a stranger (not in roster), item is NULL
			TLEN_LIST_ITEM *item = TlenListGetItemPtr(info->proto, LIST_ROSTER, fromJid);
			BOOL isChatRoomJid = TlenListExist(info->proto, LIST_CHATROOM, from);

			if (isChatRoomJid && type != NULL && !mir_strcmp(type, "groupchat")) {
				//TlenGroupchatProcessMessage(node, userdata);
			} else if (type != NULL && !mir_strcmp(type, "pic")) {
				TlenProcessPic(node, info->proto);
			} else if (type != NULL && !mir_strcmp(type, "iq")) {
				XmlNode *iqNode;
				// Tlen-compatible iq
				if ((iqNode=TlenXmlGetChild(node, "iq")) != NULL) {
					TlenXmlAddAttr(iqNode, "from", from);
					TlenProcessIq(iqNode, info);
				}
			} else {
				if ((bodyNode=TlenXmlGetChild(node, "body")) != NULL) {
					if (bodyNode->text != NULL) {
						if ((subjectNode=TlenXmlGetChild(node, "subject")) != NULL && subjectNode->text != NULL && subjectNode->text[0] != '\0') {
							size_t size = mir_strlen(subjectNode->text)+mir_strlen(bodyNode->text)+5;
							p = (char *)mir_alloc(size);
							mir_snprintf(p, size, "%s\r\n%s", subjectNode->text, bodyNode->text);
							localMessage = TlenTextDecode(p);
							mir_free(p);
						} else {
							localMessage = TlenTextDecode(bodyNode->text);
						}

						msgTime = 0;
						delivered = composing = FALSE;
						i = 1;
						while ((xNode=TlenXmlGetNthChild(node, "x", i)) != NULL) {
							if ((p=TlenXmlGetAttrValue(xNode, "xmlns")) != NULL) {
								if (!mir_strcmp(p, "jabber:x:delay") && msgTime==0) {
									if ((p=TlenXmlGetAttrValue(xNode, "stamp")) != NULL) {
										msgTime = TlenIsoToUnixTime(p);
									}
								}
								else if (!mir_strcmp(p, "jabber:x:event")) {
									// Check whether any event is requested
									if (!delivered && (n=TlenXmlGetChild(xNode, "delivered")) != NULL) {
										delivered = TRUE;
										idStr = TlenXmlGetAttrValue(node, "id");
										TlenSend(info->proto, "<message to='%s'><x xmlns='jabber:x:event'><delivered/><id>%s</id></x></message>", from, (idStr != NULL)?idStr:"");
									}
									if (item != NULL && TlenXmlGetChild(xNode, "composing") != NULL) {
										composing = TRUE;
										if (item->messageEventIdStr)
											mir_free(item->messageEventIdStr);
										idStr = TlenXmlGetAttrValue(node, "id");
										item->messageEventIdStr = (idStr == NULL)?NULL:mir_strdup(idStr);
									}
								}
							}
							i++;
						}

						if (item != NULL) {
							item->wantComposingEvent = composing;
							if (item->isTyping) {
								item->isTyping = FALSE;
								if ((hContact=TlenHContactFromJID(info->proto, fromJid)) != NULL)
									CallService(MS_PROTO_CONTACTISTYPING, hContact, PROTOTYPE_CONTACTTYPING_OFF);
							}
						}

						if ((hContact=TlenHContactFromJID(info->proto, fromJid)) == NULL) {
							// Create a temporary contact
							if (isChatRoomJid) {
								if ((p=strchr(from, '/')) != NULL && p[1]!='\0')
									p++;
								else
									p = from;
								nick = TlenTextEncode(p);
								hContact = TlenDBCreateContact(info->proto, from, nick, TRUE);
							}
							else {
								nick = TlenLocalNickFromJID(from);
								hContact = TlenDBCreateContact(info->proto, from, nick, TRUE);
							}
							mir_free(nick);
						}

						if (msgTime == 0) {
							msgTime = time(NULL);
						} else {
							MEVENT hDbEvent = db_event_last(hContact);
							if (hDbEvent != NULL) {
								DBEVENTINFO dbei = { sizeof(dbei) }; 
								db_event_get( hDbEvent, &dbei);
								if (msgTime < dbei.timestamp) {
									msgTime = dbei.timestamp + 1;
								}
							}
							if (msgTime > (DWORD)time(NULL)) {
								msgTime = time(NULL);
							}
						}
						PROTORECVEVENT recv = { 0 };
						recv.timestamp = (DWORD) msgTime;
						recv.szMessage = localMessage;
						recv.lParam = 0;
						ProtoChainRecvMsg(hContact, &recv);
						mir_free(localMessage);
					}
				}
			}
			mir_free(fromJid);
		}
	}
}

static void TlenProcessIq(XmlNode *node, ThreadData *info)
{
	MCONTACT hContact;
	XmlNode *queryNode = NULL;
	char *jid, *nick;
	char *xmlns = NULL;
	char *idStr, *str;
	int id;
	int i;

	if (!node->name || mir_strcmp(node->name, "iq")) return;
	char *type=TlenXmlGetAttrValue(node, "type");
//	if ((type=TlenXmlGetAttrValue(node, "type")) == NULL) return;

	id = -1;
	if ((idStr=TlenXmlGetAttrValue(node, "id")) != NULL) {
		if (!strncmp(idStr, TLEN_IQID, mir_strlen(TLEN_IQID)))
			id = atoi(idStr+mir_strlen(TLEN_IQID));
	}

	queryNode = TlenXmlGetChild(node, "query");
	if (queryNode != NULL) {
		xmlns = TlenXmlGetAttrValue(queryNode, "xmlns");
	}


	/////////////////////////////////////////////////////////////////////////
	// MATCH BY ID
	/////////////////////////////////////////////////////////////////////////
	TLEN_IQ_PFUNC pfunc=TlenIqFetchFunc(info->proto, id);
	if (pfunc != NULL) {
		info->proto->debugLogA("Handling iq request for id=%d", id);
		pfunc(info->proto, node);
	/////////////////////////////////////////////////////////////////////////
	// MORE GENERAL ROUTINES, WHEN ID DOES NOT MATCH
	/////////////////////////////////////////////////////////////////////////
	// new p2p connections
	} else if (xmlns != NULL && !mir_strcmp(xmlns, "p2p")) {
		if (info->proto->tlenOptions.useNewP2P) {
			TlenProcessP2P(node, info);
		}
	}
	// RECVED: <iq type='set'><query ...
	else if (!mir_strcmp(type, "set") && queryNode != NULL && xmlns != NULL) {

		// RECVED: roster push
		// ACTION: similar to iqIdGetRoster above
		if (!mir_strcmp(xmlns, "jabber:iq:roster")) {
			XmlNode *itemNode, *groupNode;
			TLEN_LIST_ITEM *item;
			char *name;

			info->proto->debugLogA("<iq/> Got roster push, query has %d children", queryNode->numChild);
			for (i=0; i<queryNode->numChild; i++) {
				itemNode = queryNode->child[i];
				if (!mir_strcmp(itemNode->name, "item")) {
					if ((jid=TlenXmlGetAttrValue(itemNode, "jid")) != NULL) {
						if ((str=TlenXmlGetAttrValue(itemNode, "subscription")) != NULL) {
							// we will not add new account when subscription=remove
							if (!mir_strcmp(str, "to") || !mir_strcmp(str, "both") || !mir_strcmp(str, "from") || !mir_strcmp(str, "none")) {
								if ((name=TlenXmlGetAttrValue(itemNode, "name")) != NULL) {
									nick = TlenTextDecode(name);
								} else {
									nick = TlenLocalNickFromJID(jid);
								}
								if (nick != NULL) {
									if ((item=TlenListAdd(info->proto, LIST_ROSTER, jid)) != NULL) {
										if (item->nick) mir_free(item->nick);
										item->nick = nick;
										if ((hContact=TlenHContactFromJID(info->proto, jid)) == NULL) {
											// Received roster has a new JID.
											// Add the jid (with empty resource) to Miranda contact list.
											hContact = TlenDBCreateContact(info->proto, jid, nick, FALSE);
										}
										db_set_s(hContact, "CList", "MyHandle", nick);
										if (item->group) mir_free(item->group);
										if ((groupNode=TlenXmlGetChild(itemNode, "group")) != NULL && groupNode->text != NULL) {
											item->group = TlenGroupDecode(groupNode->text);
											Clist_CreateGroup(0, _A2T(item->group));
											db_set_s(hContact, "CList", "Group", item->group);
										}
										else {
											item->group = NULL;
											db_unset(hContact, "CList", "Group");
										}
										if (!mir_strcmp(str, "none") || (!mir_strcmp(str, "from") && strchr(jid, '@') != NULL)) {
											if (db_get_w(hContact, info->proto->m_szModuleName, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE)
												db_set_w(hContact, info->proto->m_szModuleName, "Status", ID_STATUS_OFFLINE);
										}
									}
									else {
										mir_free(nick);
									}
								}
							}
							if ((item=TlenListGetItemPtr(info->proto, LIST_ROSTER, jid)) != NULL) {
								if (!mir_strcmp(str, "both"))
									item->subscription = SUB_BOTH;
								else if (!mir_strcmp(str, "to"))
									item->subscription = SUB_TO;
								else if (!mir_strcmp(str, "from"))
									item->subscription = SUB_FROM;
								else
									item->subscription = SUB_NONE;
								info->proto->debugLogA("Roster push for jid=%s, set subscription to %s", jid, str);
								// subscription = remove is to remove from roster list
								// but we will just set the contact to offline and not actually
								// remove, so that history will be retained.
								if (!mir_strcmp(str, "remove")) {
									if ((hContact=TlenHContactFromJID(info->proto, jid)) != NULL) {
										if (db_get_w(hContact, info->proto->m_szModuleName, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE)
											db_set_w(hContact, info->proto->m_szModuleName, "Status", ID_STATUS_OFFLINE);
										TlenListRemove(info->proto, LIST_ROSTER, jid);
									}
								}
							}
						}
					}
				}
			}
		}

	}
	// RECVED: <iq type='get'><query ...
	else if ( !mir_strcmp( type, "get" ) && queryNode != NULL && xmlns != NULL ) {
		// RECVED: software version query
		// ACTION: return my software version
		if ( !mir_strcmp( xmlns, "jabber:iq:version" )) TlenProcessIqGetVersion(info->proto, node);
	}
	// RECVED: <iq type='result'><query ...
	else if ( !mir_strcmp( type, "result") && queryNode != NULL) {
		if (xmlns != NULL ) {
			if ( !mir_strcmp(xmlns, "jabber:iq:roster" )) {
				TlenIqResultRoster(info->proto, node);
			} else if ( !mir_strcmp( xmlns, "jabber:iq:version" )) {
				TlenIqResultVersion(info->proto, node);
			} else if ( !mir_strcmp( xmlns, "jabber:iq:info" )) {
				TlenIqResultInfo(info->proto, node);
			}
		} else {
			char *from;
			if (( from=TlenXmlGetAttrValue( node, "from" )) != NULL ) {
				if ( !mir_strcmp(from, "tcfg" )) {
					TlenIqResultTcfg(info->proto, node);
				}
			}
		}
	}
	// RECVED: <iq type='error'> ...
	else if (!mir_strcmp(type, "error")) {
		TLEN_LIST_ITEM *item;
		// Check for file transfer deny by comparing idStr with ft->iqId
		i = 0;
		while ((i=TlenListFindNext(info->proto, LIST_FILE, i)) >= 0) {
			item = TlenListGetItemPtrFromIndex(info->proto,i);
			if (item->ft->state==FT_CONNECTING && !mir_strcmp(idStr, item->ft->iqId)) {
				item->ft->state = FT_DENIED;
				if (item->ft->hFileEvent != NULL)
					SetEvent(item->ft->hFileEvent);	// Simulate the termination of file server connection
			}
			i++;
		}
	}
}

/*
 * Web messages
 */
static void TlenProcessW(XmlNode *node, ThreadData *info)
{
	char *e, *s;
	char *str, *localMessage;
	int strSize;

	if (!node->name || mir_strcmp(node->name, "w"))
		return;

	char *body=node->text;
	if (body == NULL)
		return;

	char *f = TlenXmlGetAttrValue(node, "f");
	if (f != NULL) {
		char webContactName[128];
		mir_snprintf(webContactName, SIZEOF(webContactName), Translate("%s Web Messages"), info->proto->m_szModuleName);
		MCONTACT hContact = TlenHContactFromJID(info->proto, webContactName);
		if (hContact == NULL) {
			hContact = TlenDBCreateContact(info->proto, webContactName, webContactName, TRUE);
		}

		s = TlenXmlGetAttrValue(node, "s");
		e = TlenXmlGetAttrValue(node, "e");

		str = NULL;
		strSize = 0;
		TlenStringAppend(&str, &strSize, "%s\r\n%s: ", Translate("Web message"), Translate("From"));

		if (f != NULL)
			TlenStringAppend(&str, &strSize, "%s", f);
		TlenStringAppend(&str, &strSize, "\r\n%s: ", Translate("E-mail"));
		if (e != NULL)
			TlenStringAppend(&str, &strSize, "%s", e);
		TlenStringAppend(&str, &strSize, "\r\n\r\n%s", body);

		localMessage = TlenTextDecode(str);

		PROTORECVEVENT recv = { 0 };
		recv.timestamp = (DWORD) time(NULL);
		recv.szMessage = localMessage;
		ProtoChainRecvMsg(hContact, &recv);	

		mir_free(localMessage);
		mir_free(str);
	}
}

/*
 * Typing notification, multi-user conference messages and invitations
 */
static void TlenProcessM(XmlNode *node, ThreadData *info)
{
	char *p, *n, *r, *s, *str, *localMessage;
	int i;
	XmlNode *xNode, *invNode, *bNode;

	if (!node->name || mir_strcmp(node->name, "m")) return;

	char *f = TlenXmlGetAttrValue(node, "f"); //, *from;//username
	if (f != NULL) {
		char *fLogin = TlenLoginFromJID(f);
		MCONTACT hContact=TlenHContactFromJID(info->proto, fLogin);
		if (hContact != NULL) {
			char *tp=TlenXmlGetAttrValue(node, "tp");//typing start/stop
			if (tp != NULL) {
				TLEN_LIST_ITEM *item = TlenListGetItemPtr(info->proto, LIST_ROSTER, fLogin);
				if (!mir_strcmp(tp, "t")) { //contact is writing
					if (item != NULL ) {
						item->isTyping = TRUE;
						CallService(MS_PROTO_CONTACTISTYPING, hContact, (LPARAM)PROTOTYPE_CONTACTTYPING_INFINITE);
					}
				}
				else if (!mir_strcmp(tp, "u")) {//contact stopped writing
					if (item != NULL) {
						item->isTyping = FALSE;
						CallService(MS_PROTO_CONTACTISTYPING, hContact, (LPARAM)PROTOTYPE_CONTACTTYPING_OFF);
					}
				}
				else if (!mir_strcmp(tp, "a")) {//alert was received
					int bAlert = TRUE;
					if (info->proto->tlenOptions.alertPolicy == TLEN_ALERTS_IGNORE_ALL) {
						bAlert = FALSE;
					} else if (info->proto->tlenOptions.alertPolicy == TLEN_ALERTS_IGNORE_NIR) {
						bAlert = IsAuthorized(info->proto, fLogin);
					}
					if (bAlert) {
						if (info->proto->tlenOptions.useNudge)
							NotifyEventHooks(info->proto->hTlenNudge, hContact, 0);
						else {
							if (info->proto->tlenOptions.logAlerts)
								TlenLogMessage(info->proto, hContact, 0, Translate("An alert has been received."));
							SkinPlaySound("TlenAlertNotify");
						}
					}
				}
			}
		}
		mir_free(fLogin);
		if ((p=strchr(f, '@')) != NULL) {
			if ((p=strchr(p, '/')) != NULL && p[1]!='\0') { // message from user
				time_t timestamp;
				s = TlenXmlGetAttrValue(node, "s");
				if (s != NULL) {
					timestamp = TlenTimeToUTC(atol(s));
					if (timestamp > time(NULL)) {
						timestamp = time(NULL);
					}
				} else {
					timestamp = time(NULL);
				}
				char *tp=TlenXmlGetAttrValue(node, "tp");//typing start/stop
				bNode = TlenXmlGetChild(node, "b");
				f = TlenTextDecode(f);
				if (bNode != NULL && bNode->text != NULL) {
					if (tp != NULL && !mir_strcmp(tp, "p")) {
						/* MUC private message */
						str = TlenResourceFromJID(f);
						hContact = TlenDBCreateContact(info->proto, f, str, TRUE);
						db_set_b(hContact, info->proto->m_szModuleName, "bChat", TRUE);
						mir_free(str);
						localMessage = TlenTextDecode(bNode->text);

						PROTORECVEVENT recv = { 0 };
						recv.timestamp = (DWORD) timestamp;
						recv.szMessage = localMessage;
						ProtoChainRecvMsg(hContact, &recv);
						mir_free(localMessage);
					}
				}
				mir_free(f);
			}
		}
		i=1;
		while ((xNode=TlenXmlGetNthChild(node, "x", i)) != NULL) {
			invNode=TlenXmlGetChild(xNode, "inv");
			if (invNode != NULL) {
				r = TlenTextDecode(f);
				f = TlenXmlGetAttrValue(invNode, "f");
				f = TlenTextDecode(f);
				n = TlenXmlGetAttrValue(invNode, "n");
				if (n != NULL && strstr(r, n) != r) {
					n = TlenTextDecode(n);
				} else {
					n = mir_strdup(Translate("Private conference"));
					//n = TlenNickFromJID(r);
				}
				TlenMUCRecvInvitation(info->proto, r, n, f, "");
				mir_free(n);
				mir_free(r);
				mir_free(f);
				break;
			}
			i++;
		}
	}
}

static void TlenMailPopup(TlenProtocol *proto, char *title, char *emailInfo)
{
	if ( !ServiceExists(MS_POPUP_ADDPOPUPT))
		return;
	if (!db_get_b(NULL, proto->m_szModuleName, "MailPopupEnabled", TRUE))
		return;

	POPUPDATAT ppd = { 0 };
	ppd.lchIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_MAIL));
	_tcsncpy(ppd.lptzContactName, _A2T(title), MAX_CONTACTNAME -1);
	_tcsncpy(ppd.lptzText, _A2T(emailInfo), MAX_SECONDLINE - 1);
	ppd.colorBack = db_get_dw(NULL, proto->m_szModuleName, "MailPopupBack", 0);
	ppd.colorText = db_get_dw(NULL, proto->m_szModuleName, "MailPopupText", 0);
	BYTE delayMode = db_get_b(NULL, proto->m_szModuleName, "MailPopupDelayMode", 0);
	if (delayMode == 1)
		ppd.iSeconds = db_get_dw(NULL, proto->m_szModuleName, "MailPopupDelay", 4);
	else if (delayMode == 2)
		ppd.iSeconds = -1;
	PUAddPopupT(&ppd);
}
/*
 * Incoming e-mail notification
 */
static void TlenProcessN(XmlNode *node, ThreadData *info)
{
	char *str, *popupTitle, *popupText;
	int strSize;

	if (!node->name || mir_strcmp(node->name, "n")) return;

	char *s = TlenXmlGetAttrValue(node, "s");
	char *f = TlenXmlGetAttrValue(node, "f");
	if (s != NULL && f != NULL) {
		str = NULL;
		strSize = 0;

		TlenStringAppend(&str, &strSize, Translate("%s mail"), info->proto->m_szModuleName);
		popupTitle = TlenTextDecode(str);
		mir_free(str);

		str = NULL;
		strSize = 0;

		TlenStringAppend(&str, &strSize, "%s: %s\n", Translate("From"), f);
		TlenStringAppend(&str, &strSize, "%s: %s", Translate("Subject"), s);
		popupText = TlenTextDecode(str);
		TlenMailPopup(info->proto, popupTitle, popupText);
		SkinPlaySound("TlenMailNotify");

		mir_free(popupTitle);
		mir_free(popupText);
		mir_free(str);
	}
}

/*
 * Presence is chat rooms
 */
static void TlenProcessP(XmlNode *node, ThreadData *info)
{
	char *f, *a, *k;
	XmlNode *sNode, *xNode, *iNode, *kNode;
	int status, flags;

	if (!node->name || mir_strcmp(node->name, "p")) return;

// presence from users in chat room
	flags = 0;
	status = ID_STATUS_ONLINE;
	f = TlenXmlGetAttrValue(node, "f");
	xNode = TlenXmlGetChild(node, "x");
	if (xNode != NULL) { // x subtag present (message from chat room) - change user rights only
		char *temp, *iStr;
		iNode = TlenXmlGetChild(xNode, "i");
		if (iNode != NULL) {
			iStr = TlenXmlGetAttrValue(iNode, "i");
			temp = (char*)mir_alloc(mir_strlen(f)+mir_strlen(iStr)+2);
			mir_strcpy(temp, f);
			mir_strcat(temp, "/");
			mir_strcat(temp, iStr);
			f = TlenTextDecode(temp);
			mir_free(temp);
			node = iNode;
			status = 0;
		} else {
			f = TlenTextDecode(f);
		}
	} else {
		f = TlenTextDecode(f);
	}
	a = TlenXmlGetAttrValue(node, "z");
	if (a != NULL) {
		if (atoi(a) &1 ) {
			flags |= USER_FLAGS_REGISTERED;
		}
	}
	a = TlenXmlGetAttrValue(node, "a");
	if (a != NULL) {
		if (atoi(a) == 2) {
			flags |= USER_FLAGS_ADMIN;
		}
		if (atoi(a) == 1) {
			flags |= USER_FLAGS_OWNER;
		}
		if (atoi(a) == 3) {
			//flags |= USER_FLAGS_MEMBER;
		}
		if (atoi(a) == 5) {
			flags |= USER_FLAGS_GLOBALOWNER;
		}
	}
	sNode = TlenXmlGetChild(node, "s");
	if (sNode != NULL) {
		if (!mir_strcmp(sNode->text, "unavailable")) {
			status = ID_STATUS_OFFLINE;
		}
	}
	kNode = TlenXmlGetChild(node, "kick");
	k = NULL;
	if (kNode != NULL) {
		k = TlenXmlGetAttrValue(kNode, "r");
		if (k == NULL) {
			k = "";
		}
		k = TlenTextDecode(k);
	}
	if (k != NULL) {
		mir_free(k);
	}
	mir_free(f);
}
/*
 * Voice chat
 */
static void TlenProcessV(XmlNode *node, ThreadData *info)
{
	char jid[128];
	TLEN_LIST_ITEM *item;
	char *id, *e, *p;
//	if (!node->name || mir_strcmp(node->name, "v")) return;

	char *from=TlenXmlGetAttrValue(node, "f");
	if (from != NULL) {
		if (strchr(from, '@') == NULL) {
			mir_snprintf(jid, SIZEOF(jid), "%s@%s", from, info->server);
		} else {
			strncpy_s(jid, from, _TRUNCATE);
		}
		if ((e=TlenXmlGetAttrValue(node, "e")) != NULL) {
			if (!mir_strcmp(e, "1")) {
				if ((id=TlenXmlGetAttrValue(node, "i")) != NULL) {
					SkinPlaySound("TlenVoiceNotify");
					TlenVoiceAccept(info->proto, id, from);
				}
			} else if (!mir_strcmp(e, "3")) {
				// FILE_RECV : e='3' : invalid transfer error
				if ((p=TlenXmlGetAttrValue(node, "i")) != NULL) {
					if ((item=TlenListGetItemPtr(info->proto, LIST_VOICE, p)) != NULL) {
						if (item->ft != NULL) {
							HANDLE  hEvent = item->ft->hFileEvent;
							item->ft->hFileEvent = NULL;
							item->ft->state = FT_ERROR;
							if (item->ft->s != NULL) {
								Netlib_CloseHandle(item->ft->s);
								item->ft->s = NULL;
								if (hEvent != NULL) {
									SetEvent(hEvent);
								}
							} else {
								TlenP2PFreeFileTransfer(item->ft);
							}
						} else {
							TlenListRemove(info->proto, LIST_VOICE, p);
						}
					}
				}
			} else if (!mir_strcmp(e, "4")) {
				// FILE_SEND : e='4' : File sending request was denied by the remote client
				if ((p=TlenXmlGetAttrValue(node, "i")) != NULL) {
					if ((item=TlenListGetItemPtr(info->proto, LIST_VOICE, p)) != NULL) {
						if (!mir_strcmp(item->ft->jid, jid)) {
							TlenVoiceCancelAll(info->proto);
							//TlenListRemove(info->proto, LIST_VOICE, p);
						}
					}
				}
			} else if (!mir_strcmp(e, "5")) {
			// FILE_SEND : e='5' : Voice request was accepted
				if ((p=TlenXmlGetAttrValue(node, "i")) != NULL) {
					if ((item=TlenListGetItemPtr(info->proto, LIST_VOICE, p)) != NULL) {
						info->proto->debugLogA("should start voice 1 ? %s ?? %s", jid, item->ft->jid);
						if (!mir_strcmp(item->ft->jid, jid)) {
							info->proto->debugLogA("starting voice 1");
							TlenVoiceStart(item->ft, 1);
						}
					}
				}
			} else if (!mir_strcmp(e, "6")) {
				// FILE_RECV : e='6' : IP and port information to connect to get file
				if ((p=TlenXmlGetAttrValue(node, "i")) != NULL) {
					if ((item=TlenListGetItemPtr(info->proto, LIST_VOICE, p)) != NULL) {
						if ((p=TlenXmlGetAttrValue(node, "a")) != NULL) {
							item->ft->hostName = mir_strdup(p);
							if ((p=TlenXmlGetAttrValue(node, "p")) != NULL) {
								item->ft->wPort = atoi(p);
								TlenVoiceStart(item->ft, 0);
								//forkthread((void (__cdecl *)(void*))TlenVoiceReceiveThread, 0, item->ft);
							}
						}
					}
				}
			}
			else if (!mir_strcmp(e, "7")) {
				// FILE_RECV : e='7' : IP and port information to connect to send file
				// in case the conection to the given server was not successful
				if ((p=TlenXmlGetAttrValue(node, "i")) != NULL) {
					if ((item=TlenListGetItemPtr(info->proto, LIST_VOICE, p)) != NULL) {
						if ((p=TlenXmlGetAttrValue(node, "a")) != NULL) {
							if (item->ft->hostName != NULL) mir_free(item->ft->hostName);
							item->ft->hostName = mir_strdup(p);
							if ((p=TlenXmlGetAttrValue(node, "p")) != NULL) {
								item->ft->wPort = atoi(p);
								item->ft->state = FT_SWITCH;
								SetEvent(item->ft->hFileEvent);
							}
						}
					}
				}
			}
			else if (!mir_strcmp(e, "8")) {
				// FILE_RECV : e='8' : transfer error
				if ((p=TlenXmlGetAttrValue(node, "i")) != NULL) {
					if ((item=TlenListGetItemPtr(info->proto, LIST_VOICE, p)) != NULL) {
						item->ft->state = FT_ERROR;
						SetEvent(item->ft->hFileEvent);
					}
				}
			}

		}
	}
}

static void __cdecl TlenKeepAliveThread(void *ptr)
{
	NETLIBSELECT nls = {0};

	TlenProtocol *proto = (TlenProtocol *)ptr;
	nls.cbSize = sizeof(NETLIBSELECT);
	nls.dwTimeout = 60000;	// 60000 millisecond (1 minute)
	nls.hExceptConns[0] = proto->threadData->s;
	for (;;) {
		if (CallService(MS_NETLIB_SELECT, 0, (LPARAM) &nls) != 0)
			break;
		if (proto->tlenOptions.sendKeepAlive)
			TlenSend(proto, " \t ");
	}
	proto->debugLogA("Exiting KeepAliveThread");
}

