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

#include "jabber.h"

#include "commons.h"
#include "jabber_list.h"
#include "jabber_iq.h"
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
static void __cdecl JabberKeepAliveThread(void *ptr);
static void JabberProcessStreamOpening(XmlNode *node, ThreadData *info);
static void JabberProcessStreamClosing(XmlNode *node, ThreadData *info);
static void JabberProcessProtocol(XmlNode *node, ThreadData *info);
static void JabberProcessMessage(XmlNode *node, ThreadData *info);
static void JabberProcessIq(XmlNode *node, ThreadData *info);
static void TlenProcessW(XmlNode *node, ThreadData *info);
static void TlenProcessM(XmlNode *node, ThreadData *info);
static void TlenProcessN(XmlNode *node, ThreadData *info);
static void TlenProcessP(XmlNode *node, ThreadData *info);
static void TlenProcessV(XmlNode *node, ThreadData *info);
static void TlenProcessAvatar(XmlNode* node, ThreadData *info);
static void TlenProcessCipher(XmlNode *node, ThreadData *info);

static VOID NTAPI JabberDummyApcFunc(ULONG_PTR param)
{
	return;
}

static char onlinePassword[128];
static HANDLE hEventPasswdDlg;

static INT_PTR CALLBACK JabberPasswordDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char text[128];

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		mir_snprintf(text, SIZEOF(text), "%s %s", Translate("Enter password for"), (char *) lParam);
		SetDlgItemTextA(hwndDlg, IDC_JID, text);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			GetDlgItemTextA(hwndDlg, IDC_PASSWORD, onlinePassword, sizeof(onlinePassword));
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

static VOID NTAPI JabberPasswordCreateDialogApcProc(ULONG_PTR param)
{
	CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_PASSWORD), NULL, JabberPasswordDlgProc, (LPARAM) param);
}

void __cdecl JabberServerThread(ThreadData *info)
{
	DBVARIANT dbv;
	char jidStr[128];
	char *connectHost;
	char *buffer;
	int datalen;
	XmlState xmlState;
	int jabberNetworkBufferSize;
	int oldStatus = ID_STATUS_OFFLINE;
	int reconnectMaxTime;
	int numRetry;
	int reconnectTime;
	int loginErr = 0;
	JabberLog(info->proto, "Thread started");

	// Normal server connection, we will fetch all connection parameters
	// e.g. username, password, etc. from the database.

	if (info->proto->threadData != NULL) {
		// Will not start another connection thread if a thread is already running.
		// Make APC call to the main thread. This will immediately wake the thread up
		// in case it is asleep in the reconnect loop so that it will immediately
		// reconnect.
		QueueUserAPC(JabberDummyApcFunc, info->proto->threadData->hThread, 0);
		JabberLog(info->proto, "Thread ended, another normal thread is running");
		mir_free(info);
		return;
	}

	info->proto->threadData = info;

	if (!db_get(NULL, info->proto->m_szModuleName, "LoginName", &dbv)) {
		strncpy(info->username, dbv.pszVal, sizeof(info->username));
		info->username[sizeof(info->username)-1] = '\0';
		_strlwr(info->username);
		db_set_s(NULL, info->proto->m_szModuleName, "LoginName", info->username);
		db_free(&dbv);

	} else {
		JabberLog(info->proto, "Thread ended, login name is not configured");
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
			JabberLog(info->proto, "Thread ended, login server is not configured");
			loginErr = LOGINERR_NONETWORK;
		}
	}

	if (loginErr == 0) {
		if (!info->proto->tlenOptions.savePassword) {
			// Ugly hack: continue logging on only the return value is &(onlinePassword[0])
			// because if WM_QUIT while dialog box is still visible, p is returned with some
			// exit code which may not be NULL.
			// Should be better with modeless.
			onlinePassword[0] = (char) -1;
			hEventPasswdDlg = CreateEvent(NULL, FALSE, FALSE, NULL);
			QueueUserAPC(JabberPasswordCreateDialogApcProc, hMainThread, (DWORD) jidStr);
			WaitForSingleObject(hEventPasswdDlg, INFINITE);
			CloseHandle(hEventPasswdDlg);
			//if ((p=(char *)DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_PASSWORD), NULL, JabberPasswordDlgProc, (LPARAM) jidStr)) != onlinePassword) {
			if (onlinePassword[0] != (char) -1) {
				strncpy(info->password, onlinePassword, sizeof(info->password));
				info->password[sizeof(info->password)-1] = '\0';
			} else {
				JabberLog(info->proto, "Thread ended, password request dialog was canceled");
				loginErr = LOGINERR_BADUSERID;
			}
		} else {
			if (!db_get(NULL, info->proto->m_szModuleName, "Password", &dbv)) {
				CallService(MS_DB_CRYPT_DECODESTRING, strlen(dbv.pszVal)+1, (LPARAM) dbv.pszVal);
				strncpy(info->password, dbv.pszVal, sizeof(info->password));
				info->password[sizeof(info->password)-1] = '\0';
				db_free(&dbv);
			} else {
				JabberLog(info->proto, "Thread ended, password is not configured");
				loginErr = LOGINERR_BADUSERID;
			}
		}
	}

	jabberNetworkBufferSize = 2048;
	if ((buffer=(char *) mir_alloc(jabberNetworkBufferSize+1)) == NULL) {	// +1 is for '\0' when debug logging this buffer
		JabberLog(info->proto, "Thread ended, network buffer cannot be allocated");
		loginErr = LOGINERR_NONETWORK;
	}

	if (loginErr != 0) {
		info->proto->threadData = NULL;
		oldStatus = info->proto->m_iStatus;
		info->proto->m_iStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(info->proto->m_szModuleName, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldStatus, info->proto->m_iStatus);
		ProtoBroadcastAck(info->proto->m_szModuleName, NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, loginErr);
		mir_free(info);
		return;
	}

	mir_snprintf(jidStr, sizeof(jidStr), "%s@%s", info->username, info->server);
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

	JabberLog(info->proto, "Thread server='%s' port='%d'", connectHost, info->port);


	if (!db_get(NULL, info->proto->m_szModuleName, "AvatarHash", &dbv)) {
		strcpy(info->proto->threadData->avatarHash, dbv.pszVal);
		db_free(&dbv);
	}
	info->avatarFormat = db_get_dw(NULL, info->proto->m_szModuleName, "AvatarFormat", PA_FORMAT_UNKNOWN);


	reconnectMaxTime = 10;
	numRetry = 0;

	for (;;) {	// Reconnect loop

		info->s = JabberWsConnect(info->proto, connectHost, info->port);
		if (info->s == NULL) {
			JabberLog(info->proto, "Connection failed (%d)", WSAGetLastError());
			if (info->proto->threadData == info) {
				oldStatus = info->proto->m_iStatus;
				info->proto->m_iStatus = ID_STATUS_OFFLINE;
				ProtoBroadcastAck(info->proto->m_szModuleName, NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_NONETWORK);
				ProtoBroadcastAck(info->proto->m_szModuleName, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldStatus, info->proto->m_iStatus);
				if (info->proto->tlenOptions.reconnect == TRUE) {
					reconnectTime = rand() % reconnectMaxTime;
					JabberLog(info->proto, "Sleeping %d seconds before automatic reconnecting...", reconnectTime);
					SleepEx(reconnectTime * 1000, TRUE);
					if (reconnectMaxTime < 10*60)	// Maximum is 10 minutes
						reconnectMaxTime *= 2;
					if (info->proto->threadData == info) {	// Make sure this is still the active thread for the main Jabber connection
						JabberLog(info->proto, "Reconnecting to the network...");
						if (numRetry < MAX_CONNECT_RETRIES)
							numRetry++;
						oldStatus = info->proto->m_iStatus;
						info->proto->m_iStatus = ID_STATUS_CONNECTING + numRetry;
						ProtoBroadcastAck(info->proto->m_szModuleName, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldStatus, info->proto->m_iStatus);
						continue;
					}
					else {
						JabberLog(info->proto, "Thread ended, connection failed");
						mir_free(buffer);
						mir_free(info);
						return;
					}
				}
				info->proto->threadData = NULL;
			}
			JabberLog(info->proto, "Thread ended, connection failed");
			mir_free(buffer);
			mir_free(info);
			return;
		}

		// User may change status to OFFLINE while we are connecting above
		if (info->proto->m_iDesiredStatus != ID_STATUS_OFFLINE) {

			info->proto->isConnected = TRUE;
			JabberForkThread(JabberKeepAliveThread, 0, info->proto);

			JabberXmlInitState(&xmlState);
			JabberXmlSetCallback(&xmlState, 1, ELEM_OPEN, (void (__cdecl *)(XmlNode *,void *))JabberProcessStreamOpening, info);
			JabberXmlSetCallback(&xmlState, 1, ELEM_CLOSE, (void (__cdecl *)(XmlNode *,void *))JabberProcessStreamClosing, info);
			JabberXmlSetCallback(&xmlState, 2, ELEM_CLOSE, (void (__cdecl *)(XmlNode *,void *))JabberProcessProtocol, info);

			info->useAES = FALSE;

			if (info->useEncryption) {
				JabberSend(info->proto, "<s s='1' v='9' t='06000106'>");

			} else {
				JabberSend(info->proto, "<s v='3'>");
			}

			JabberLog(info->proto, "Entering main recv loop");
			datalen = 0;

			for (;;) {
				int recvResult, bytesParsed;

				if (info->useAES) {
					recvResult = JabberWsRecvAES(info->proto, buffer+datalen, jabberNetworkBufferSize-datalen, &info->aes_in_context, info->aes_in_iv);
				} else {
					recvResult = JabberWsRecv(info->proto, info->s, buffer+datalen, jabberNetworkBufferSize-datalen);
				}

				if (recvResult <= 0)
					break;
				datalen += recvResult;

				buffer[datalen] = '\0';
				JabberLog(info->proto, "RECV:%s", buffer);

				bytesParsed = JabberXmlParse(&xmlState, buffer, datalen);
				JabberLog(info->proto, "bytesParsed = %d", bytesParsed);
				if (bytesParsed > 0) {
					if (bytesParsed < datalen)
						memmove(buffer, buffer+bytesParsed, datalen-bytesParsed);
					datalen -= bytesParsed;
				}
				else if (datalen == jabberNetworkBufferSize) {
					jabberNetworkBufferSize += 2048;
					JabberLog(info->proto, "Increasing network buffer size to %d", jabberNetworkBufferSize);
					if ((buffer=(char *) mir_realloc(buffer, jabberNetworkBufferSize+1)) == NULL) {
						JabberLog(info->proto, "Cannot reallocate more network buffer, go offline now");
						break;
					}
				}
				else {
					JabberLog(info->proto, "Unknown state: bytesParsed=%d, datalen=%d, jabberNetworkBufferSize=%d", bytesParsed, datalen, jabberNetworkBufferSize);
				}
			}

			JabberXmlDestroyState(&xmlState);

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
			for (HANDLE hContact = db_find_first(szProto); hContact; hContact = db_find_next(hContact, szProto))
				if (db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE)
					db_set_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);

			JabberListWipeSpecial(info->proto);
		}
		else {
			oldStatus = info->proto->m_iStatus;
			info->proto->m_iStatus = ID_STATUS_OFFLINE;
			ProtoBroadcastAck(info->proto->m_szModuleName, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldStatus, info->proto->m_iStatus);
		}

		Netlib_CloseHandle(info->s);

		if (info->proto->tlenOptions.reconnect == FALSE)
			break;

		if (info->proto->threadData != info)	// Make sure this is still the main Jabber connection thread
			break;
		reconnectTime = rand() % 10;
		JabberLog(info->proto, "Sleeping %d seconds before automatic reconnecting...", reconnectTime);
		SleepEx(reconnectTime * 1000, TRUE);
		reconnectMaxTime = 20;
		if (info->proto->threadData != info)	// Make sure this is still the main Jabber connection thread
			break;
		JabberLog(info->proto, "Reconnecting to the network...");
		info->proto->m_iDesiredStatus = oldStatus;	// Reconnect to my last status
		oldStatus = info->proto->m_iStatus;
		info->proto->m_iStatus = ID_STATUS_CONNECTING;
		numRetry = 1;
		ProtoBroadcastAck(info->proto->m_szModuleName, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldStatus, info->proto->m_iStatus);
	}

	JabberLog(info->proto, "Thread ended: server='%s'", info->server);

	if (info->proto->threadData == info) {
		info->proto->threadData = NULL;
	}

	mir_free(buffer);
	if (info->streamId) mir_free(info->streamId);
	JabberLog(info->proto, "Exiting ServerThread");
	mir_free(info);
}

static void TlenSendAuth(TlenProtocol *proto) {
	int iqId;
	char *p;
	char *str;
	char text[128];
	str = TlenPasswordHash(proto->threadData->password);
	mir_snprintf(text, SIZEOF(text), "%s%s", proto->threadData->streamId, str);
	mir_free(str);
	str = JabberSha1(text);
	if ((p=JabberTextEncode(proto->threadData->username)) != NULL) {
		iqId = JabberSerialNext(proto->threadData->proto);
		JabberIqAdd(proto, iqId, IQ_PROC_NONE, JabberIqResultAuth);
		JabberSend(proto, "<iq type='set' id='"JABBER_IQID"%d'><query xmlns='jabber:iq:auth'><username>%s</username><digest>%s</digest><resource>t</resource><host>tlen.pl</host></query></iq>", iqId, p /*info->username*/, str);
		mir_free(p);
	}
	mir_free(str);
}

static void JabberProcessStreamOpening(XmlNode *node, ThreadData *info)
{
	char *sid, *s;

	if (node->name == NULL || strcmp(node->name, "s"))
		return;

	if ((sid=JabberXmlGetAttrValue(node, "i")) != NULL) {
		if (info->streamId) mir_free(info->streamId);
		info->streamId = mir_strdup(sid);
	}
	if ((s=JabberXmlGetAttrValue(node, "s")) != NULL && !strcmp(s, "1")) {
		int i;
		char *k1, *k2, *k3;
		unsigned char aes_key[32];
		char aes_key_str[140], aes_iv_str[40];
		mpi k1_mpi, k2_mpi, aes_mpi;
		size_t slen;

		k1=JabberXmlGetAttrValue(node, "k1");
		k2=JabberXmlGetAttrValue(node, "k2");
		k3=JabberXmlGetAttrValue(node, "k3");

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
		JabberSend(info->proto, "<cipher k1='%s' k2='%s'/>", aes_key_str, aes_iv_str);
	} else {
		TlenSendAuth(info->proto);
	}
}

static void JabberProcessStreamClosing(XmlNode *node, ThreadData *info)
{
	Netlib_CloseHandle(info->proto);
	if (node->name && !strcmp(node->name, "stream:error") && node->text)
		MessageBoxA(NULL, Translate(node->text), Translate("Jabber Connection Error"), MB_OK|MB_ICONERROR|MB_SETFOREGROUND);
}

static void JabberProcessProtocol(XmlNode *node, ThreadData *info)
{
	if (!strcmp(node->name, "message"))
		JabberProcessMessage(node, info);
	else if (!strcmp(node->name, "presence"))
		TlenProcessPresence(node, info->proto);
	else if (!strcmp(node->name, "iq"))
		JabberProcessIq(node, info);
	else if (!strcmp(node->name, "f"))
		TlenProcessF(node, info);
	else if (!strcmp(node->name, "w"))
		TlenProcessW(node, info);
	else if (!strcmp(node->name, "m"))
		TlenProcessM(node, info);
	else if (!strcmp(node->name, "n"))
		TlenProcessN(node, info);
	else if (!strcmp(node->name, "p"))
		TlenProcessP(node, info);
	else if (!strcmp(node->name, "v"))
		TlenProcessV(node, info);
	else if (!strcmp(node->name, "avatar"))
		TlenProcessAvatar(node, info);
	else if (!strcmp(node->name, "cipher"))
		TlenProcessCipher(node, info);
	else
		JabberLog(info->proto, "Invalid top-level tag (only <message/> <presence/> <iq/> <f/> <w/> <m/> <n/> <p/> <v/> <cipher/> and <avatar/> allowed)");

}

static void TlenProcessCipher(XmlNode *node, ThreadData *info)
{
	char *type;
	type=JabberXmlGetAttrValue(node, "type");
	info->useAES = TRUE;
	JabberSend(info->proto, "<cipher type='ok'/>");
	TlenSendAuth(info->proto);
}

static void TlenProcessIqGetVersion(TlenProtocol *proto, XmlNode* node)
{
	OSVERSIONINFO osvi = { 0 };
	char mversion[256];
	char* from, *version, *mver;
	char* os = NULL;
	JABBER_LIST_ITEM *item;

	if (proto->m_iStatus == ID_STATUS_INVISIBLE) return;
	if (!proto->tlenOptions.enableVersion) return;
	if (( from=JabberXmlGetAttrValue( node, "from" )) == NULL ) return;
	if (( item=JabberListGetItemPtr( proto, LIST_ROSTER, from )) ==NULL) return;
	version = JabberTextEncode( TLEN_VERSION_STRING );
	osvi.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
	if ( GetVersionEx( &osvi )) {
		switch ( osvi.dwPlatformId ) {
		case VER_PLATFORM_WIN32_NT:
			if ( osvi.dwMajorVersion == 5 ) {
				if ( osvi.dwMinorVersion == 2 ) os = JabberTextEncode( Translate( "Windows Server 2003" ));
				else if ( osvi.dwMinorVersion == 1 ) os = JabberTextEncode( Translate( "Windows XP" ));
				else if ( osvi.dwMinorVersion == 0 ) os = JabberTextEncode( Translate( "Windows 2000" ));
			}
			else if ( osvi.dwMajorVersion <= 4 ) {
				os = JabberTextEncode( Translate( "Windows NT" ));
			}
			break;
		case VER_PLATFORM_WIN32_WINDOWS:
			if ( osvi.dwMajorVersion == 4 ) {
				if ( osvi.dwMinorVersion == 0 ) os = JabberTextEncode( Translate( "Windows 95" ));
				if ( osvi.dwMinorVersion == 10 ) os = JabberTextEncode( Translate( "Windows 98" ));
				if ( osvi.dwMinorVersion == 90 ) os = JabberTextEncode( Translate( "Windows ME" ));
			}
			break;
	}	}

	if ( os == NULL ) os = JabberTextEncode( Translate( "Windows" ));

	strcpy(mversion, "Miranda NG ");
	CallService(MS_SYSTEM_GETVERSIONTEXT, sizeof( mversion ) - 11, ( LPARAM )mversion + 11 );
	strcat(mversion, " (Tlen v.");
	strcat(mversion, TLEN_VERSION_STRING);
	strcat(mversion, ")");
	mver = JabberTextEncode( mversion );
	JabberSend( proto, "<message to='%s' type='iq'><iq type='result'><query xmlns='jabber:iq:version'><name>%s</name><version>%s</version><os>%s</os></query></iq></message>", from, mver?mver:"", version?version:"", os?os:"" );
	if (!item->versionRequested) {
		item->versionRequested = TRUE;
		JabberSend(proto, "<message to='%s' type='iq'><iq type='get'><query xmlns='jabber:iq:version'/></iq></message>", from);
	}

	if ( mver ) mir_free( mver );
	if ( version ) mir_free( version );
	if ( os ) mir_free( os );
}

// Support for Tlen avatars - avatar token used to access web interface
static void TlenProcessAvatar(XmlNode* node, ThreadData *info)
{
	XmlNode *tokenNode, *aNode;
	tokenNode = JabberXmlGetChild(node, "token");
	aNode = JabberXmlGetChild(node, "a");
	if (tokenNode != NULL) {
		char *token = tokenNode->text;
		strcpy(info->avatarToken, token);
	}
	if (aNode != NULL) {
		if (TlenProcessAvatarNode(info->proto, node, NULL)) {
		}
	}
}

static void JabberProcessMessage(XmlNode *node, ThreadData *info)
{
	HANDLE hContact;
	PROTORECVEVENT recv;
	XmlNode *bodyNode, *subjectNode, *xNode, *n;
	char *from, *type, *nick, *p, *localMessage, *idStr;
	DWORD msgTime;
	BOOL delivered, composing;
	int i;
	JABBER_LIST_ITEM *item;
	BOOL isChatRoomJid;

	if (!node->name || strcmp(node->name, "message")) return;

	if ((type=JabberXmlGetAttrValue(node, "type")) != NULL && !strcmp(type, "error")) {
	}
	else {
		if ((from=JabberXmlGetAttrValue(node, "from")) != NULL) {
			char *fromJid = JabberLoginFromJID(from);
			if (info->proto->tlenOptions.ignoreAdvertisements && strstr(from, "b73@tlen.pl") == from) {
				return;
			}
			// If message is from a stranger (not in roster), item is NULL
			item = JabberListGetItemPtr(info->proto, LIST_ROSTER, fromJid);
			isChatRoomJid = JabberListExist(info->proto, LIST_CHATROOM, from);

			if (isChatRoomJid && type != NULL && !strcmp(type, "groupchat")) {
				//JabberGroupchatProcessMessage(node, userdata);
			} else if (type != NULL && !strcmp(type, "pic")) {
				TlenProcessPic(node, info->proto);
			} else if (type != NULL && !strcmp(type, "iq")) {
				XmlNode *iqNode;
				// Jabber-compatible iq
				if ((iqNode=JabberXmlGetChild(node, "iq")) != NULL) {
					JabberXmlAddAttr(iqNode, "from", from);
					JabberProcessIq(iqNode, info);
				}
			} else {
				if ((bodyNode=JabberXmlGetChild(node, "body")) != NULL) {
					if (bodyNode->text != NULL) {
						if ((subjectNode=JabberXmlGetChild(node, "subject")) != NULL && subjectNode->text != NULL && subjectNode->text[0] != '\0') {
							int size = strlen(subjectNode->text)+strlen(bodyNode->text)+5;
							p = (char *)mir_alloc(size);
							mir_snprintf(p, size, "%s\r\n%s", subjectNode->text, bodyNode->text);
							localMessage = JabberTextDecode(p);
							mir_free(p);
						} else {
							localMessage = JabberTextDecode(bodyNode->text);
						}

						msgTime = 0;
						delivered = composing = FALSE;
						i = 1;
						while ((xNode=JabberXmlGetNthChild(node, "x", i)) != NULL) {
							if ((p=JabberXmlGetAttrValue(xNode, "xmlns")) != NULL) {
								if (!strcmp(p, "jabber:x:delay") && msgTime==0) {
									if ((p=JabberXmlGetAttrValue(xNode, "stamp")) != NULL) {
										msgTime = JabberIsoToUnixTime(p);
									}
								}
								else if (!strcmp(p, "jabber:x:event")) {
									// Check whether any event is requested
									if (!delivered && (n=JabberXmlGetChild(xNode, "delivered")) != NULL) {
										delivered = TRUE;
										idStr = JabberXmlGetAttrValue(node, "id");
										JabberSend(info->proto, "<message to='%s'><x xmlns='jabber:x:event'><delivered/><id>%s</id></x></message>", from, (idStr != NULL)?idStr:"");
									}
									if (item != NULL && JabberXmlGetChild(xNode, "composing") != NULL) {
										composing = TRUE;
										if (item->messageEventIdStr)
											mir_free(item->messageEventIdStr);
										idStr = JabberXmlGetAttrValue(node, "id");
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
								if ((hContact=JabberHContactFromJID(info->proto, fromJid)) != NULL)
									CallService(MS_PROTO_CONTACTISTYPING, (WPARAM) hContact, PROTOTYPE_CONTACTTYPING_OFF);
							}
						}

						if ((hContact=JabberHContactFromJID(info->proto, fromJid)) == NULL) {
							// Create a temporary contact
							if (isChatRoomJid) {
								if ((p=strchr(from, '/')) != NULL && p[1]!='\0')
									p++;
								else
									p = from;
								nick = JabberTextEncode(p);
								hContact = JabberDBCreateContact(info->proto, from, nick, TRUE);
							}
							else {
								nick = JabberLocalNickFromJID(from);
								hContact = JabberDBCreateContact(info->proto, from, nick, TRUE);
							}
							mir_free(nick);
						}

						if (msgTime == 0) {
							msgTime = time(NULL);
						} else {
							HANDLE hDbEvent = db_event_last(hContact);
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
						recv.flags = 0;
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

static void JabberProcessIq(XmlNode *node, ThreadData *info)
{
	HANDLE hContact;
	XmlNode *queryNode = NULL;
	char *type, *jid, *nick;
	char *xmlns = NULL;
	char *idStr, *str;
	int id;
	int i;
	JABBER_IQ_PFUNC pfunc;

	if (!node->name || strcmp(node->name, "iq")) return;
	type=JabberXmlGetAttrValue(node, "type");
//	if ((type=JabberXmlGetAttrValue(node, "type")) == NULL) return;

	id = -1;
	if ((idStr=JabberXmlGetAttrValue(node, "id")) != NULL) {
		if (!strncmp(idStr, JABBER_IQID, strlen(JABBER_IQID)))
			id = atoi(idStr+strlen(JABBER_IQID));
	}

	queryNode = JabberXmlGetChild(node, "query");
	if (queryNode != NULL) {
		xmlns = JabberXmlGetAttrValue(queryNode, "xmlns");
	}


	/////////////////////////////////////////////////////////////////////////
	// MATCH BY ID
	/////////////////////////////////////////////////////////////////////////
	if ((pfunc=JabberIqFetchFunc(info->proto, id)) != NULL) {
		JabberLog(info->proto, "Handling iq request for id=%d", id);
		pfunc(info->proto, node);
	/////////////////////////////////////////////////////////////////////////
	// MORE GENERAL ROUTINES, WHEN ID DOES NOT MATCH
	/////////////////////////////////////////////////////////////////////////
	// new p2p connections
	} else if (xmlns != NULL && !strcmp(xmlns, "p2p")) {
		if (info->proto->tlenOptions.useNewP2P) {
			TlenProcessP2P(node, info);
		}
	}
	// RECVED: <iq type='set'><query ...
	else if (!strcmp(type, "set") && queryNode != NULL && xmlns != NULL) {

		// RECVED: roster push
		// ACTION: similar to iqIdGetRoster above
		if (!strcmp(xmlns, "jabber:iq:roster")) {
			XmlNode *itemNode, *groupNode;
			JABBER_LIST_ITEM *item;
			char *name;

			JabberLog(info->proto, "<iq/> Got roster push, query has %d children", queryNode->numChild);
			for (i=0; i<queryNode->numChild; i++) {
				itemNode = queryNode->child[i];
				if (!strcmp(itemNode->name, "item")) {
					if ((jid=JabberXmlGetAttrValue(itemNode, "jid")) != NULL) {
						if ((str=JabberXmlGetAttrValue(itemNode, "subscription")) != NULL) {
							// we will not add new account when subscription=remove
							if (!strcmp(str, "to") || !strcmp(str, "both") || !strcmp(str, "from") || !strcmp(str, "none")) {
								if ((name=JabberXmlGetAttrValue(itemNode, "name")) != NULL) {
									nick = JabberTextDecode(name);
								} else {
									nick = JabberLocalNickFromJID(jid);
								}
								if (nick != NULL) {
									if ((item=JabberListAdd(info->proto, LIST_ROSTER, jid)) != NULL) {
										if (item->nick) mir_free(item->nick);
										item->nick = nick;
										if ((hContact=JabberHContactFromJID(info->proto, jid)) == NULL) {
											// Received roster has a new JID.
											// Add the jid (with empty resource) to Miranda contact list.
											hContact = JabberDBCreateContact(info->proto, jid, nick, FALSE);
										}
										db_set_s(hContact, "CList", "MyHandle", nick);
										if (item->group) mir_free(item->group);
										if ((groupNode=JabberXmlGetChild(itemNode, "group")) != NULL && groupNode->text != NULL) {
											item->group = TlenGroupDecode(groupNode->text);
											JabberContactListCreateGroup(item->group);
											db_set_s(hContact, "CList", "Group", item->group);
										}
										else {
											item->group = NULL;
											db_unset(hContact, "CList", "Group");
										}
										if (!strcmp(str, "none") || (!strcmp(str, "from") && strchr(jid, '@') != NULL)) {
											if (db_get_w(hContact, info->proto->m_szModuleName, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE)
												db_set_w(hContact, info->proto->m_szModuleName, "Status", ID_STATUS_OFFLINE);
										}
									}
									else {
										mir_free(nick);
									}
								}
							}
							if ((item=JabberListGetItemPtr(info->proto, LIST_ROSTER, jid)) != NULL) {
								if (!strcmp(str, "both")) item->subscription = SUB_BOTH;
								else if (!strcmp(str, "to")) item->subscription = SUB_TO;
								else if (!strcmp(str, "from")) item->subscription = SUB_FROM;
								else item->subscription = SUB_NONE;
								JabberLog(info->proto, "Roster push for jid=%s, set subscription to %s", jid, str);
								// subscription = remove is to remove from roster list
								// but we will just set the contact to offline and not actually
								// remove, so that history will be retained.
								if (!strcmp(str, "remove")) {
									if ((hContact=JabberHContactFromJID(info->proto, jid)) != NULL) {
										if (db_get_w(hContact, info->proto->m_szModuleName, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE)
											db_set_w(hContact, info->proto->m_szModuleName, "Status", ID_STATUS_OFFLINE);
										JabberListRemove(info->proto, LIST_ROSTER, jid);
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
	else if ( !strcmp( type, "get" ) && queryNode != NULL && xmlns != NULL ) {
		// RECVED: software version query
		// ACTION: return my software version
		if ( !strcmp( xmlns, "jabber:iq:version" )) TlenProcessIqGetVersion(info->proto, node);
	}
	// RECVED: <iq type='result'><query ...
	else if ( !strcmp( type, "result") && queryNode != NULL) {
		if (xmlns != NULL ) {
			if ( !strcmp(xmlns, "jabber:iq:roster" )) {
				JabberIqResultRoster(info->proto, node);
			} else if ( !strcmp( xmlns, "jabber:iq:version" )) {
				TlenIqResultVersion(info->proto, node);
			} else if ( !strcmp( xmlns, "jabber:iq:info" )) {
				TlenIqResultInfo(info->proto, node);
			}
		} else {
			char *from;
			if (( from=JabberXmlGetAttrValue( node, "from" )) != NULL ) {
				if ( !strcmp(from, "tcfg" )) {
					TlenIqResultTcfg(info->proto, node);
				}
			}
		}
	}
	// RECVED: <iq type='error'> ...
	else if (!strcmp(type, "error")) {
		JABBER_LIST_ITEM *item;
		// Check for multi-user chat errors
		char *from;
		if ((from=JabberXmlGetAttrValue(node, "from")) != NULL) {
			if (strstr(from, "@c") != NULL || !strcmp(from, "c")) {
				TlenMUCRecvError(info->proto, from, node);
				return;
			}
		}

		// Check for file transfer deny by comparing idStr with ft->iqId
		i = 0;
		while ((i=JabberListFindNext(info->proto, LIST_FILE, i)) >= 0) {
			item = JabberListGetItemPtrFromIndex(info->proto,i);
			if (item->ft->state==FT_CONNECTING && !strcmp(idStr, item->ft->iqId)) {
				item->ft->state = FT_DENIED;
				if (item->ft->hFileEvent != NULL)
					SetEvent(item->ft->hFileEvent);	// Simulate the termination of file server connection
			}
			i++;
		}
	}
	// RECVED: <iq type='1'>...
	else if (!strcmp(type, "1")) { // Chat groups list result
		char *from;
		if ((from=JabberXmlGetAttrValue(node, "from")) != NULL) {
			if (strcmp(from, "c") == 0) {
				TlenIqResultChatGroups(info->proto, node);
			}
		}
	}
	else if (!strcmp(type, "2")) { // Chat rooms list result
		char *from;
		if ((from=JabberXmlGetAttrValue(node, "from")) != NULL) {
			if (strcmp(from, "c") == 0) {
				TlenIqResultChatRooms(info->proto, node);
			}
		}
	} else if (!strcmp(type, "3")) { // room search result - result to iq type 3 query
		char *from;
		if ((from=JabberXmlGetAttrValue(node, "from")) != NULL) {
			if (strcmp(from, "c") == 0) {
				TlenIqResultRoomSearch(info->proto, node);
			}
		}
	} else if (!strcmp(type, "4")) { // chat room users list
		char *from;
		if ((from=JabberXmlGetAttrValue(node, "from")) != NULL) {
			if (strstr(from, "@c") != NULL) {
				TlenIqResultChatRoomUsers(info->proto, node);
			}
		}
	} else if (!strcmp(type, "5")) { // room name & group & flags info - sent on joining the room
		char *from;
		if ((from=JabberXmlGetAttrValue(node, "from")) != NULL) {
			if (strstr(from, "@c") != NULL) {
				TlenIqResultRoomInfo(info->proto, node);
			}
		}
	} else if (!strcmp(type, "6")) { // new nick registered
		char *from;
		if ((from=JabberXmlGetAttrValue(node, "from")) != NULL) {
			if (strcmp(from, "c") == 0) {
				TlenIqResultUserNicks(info->proto, node);
			}
		}
	} else if (!strcmp(type, "7")) { // user nicknames list
		char *from;
		if ((from=JabberXmlGetAttrValue(node, "from")) != NULL) {
			if (strcmp(from, "c") == 0) {
				TlenIqResultUserNicks(info->proto, node);
			}
		}
	} else if (!strcmp(type, "8")) { // user chat rooms list
		char *from;
		if ((from=JabberXmlGetAttrValue(node, "from")) != NULL) {
			if (strcmp(from, "c") == 0) {
				TlenIqResultUserRooms(info->proto, node);
			}
		}
	}
}

/*
 * Web messages
 */
static void TlenProcessW(XmlNode *node, ThreadData *info)
{
	HANDLE hContact;
	PROTORECVEVENT recv;
	char *f, *e, *s, *body;
	char *str, *localMessage;
	int strSize;

	if (!node->name || strcmp(node->name, "w")) return;
	if ((body=node->text) == NULL) return;

	if ((f=JabberXmlGetAttrValue(node, "f")) != NULL) {

		char webContactName[128];
		mir_snprintf(webContactName, SIZEOF(webContactName), Translate("%s Web Messages"), info->proto->m_szModuleName);
		if ((hContact=JabberHContactFromJID(info->proto, webContactName)) == NULL) {
			hContact = JabberDBCreateContact(info->proto, webContactName, webContactName, TRUE);
		}

		s = JabberXmlGetAttrValue(node, "s");
		e = JabberXmlGetAttrValue(node, "e");

		str = NULL;
		strSize = 0;
		JabberStringAppend(&str, &strSize, "%s\r\n%s: ", Translate("Web message"), Translate("From"));

		if (f != NULL)
			JabberStringAppend(&str, &strSize, "%s", f);
		JabberStringAppend(&str, &strSize, "\r\n%s: ", Translate("E-mail"));
		if (e != NULL)
			JabberStringAppend(&str, &strSize, "%s", e);
		JabberStringAppend(&str, &strSize, "\r\n\r\n%s", body);

		localMessage = JabberTextDecode(str);

		recv.flags = 0;
		recv.timestamp = (DWORD) time(NULL);
		recv.szMessage = localMessage;
		recv.lParam = 0;
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
	HANDLE hContact;
	PROTORECVEVENT recv;
	char *f;//, *from;//username
	char *tp;//typing start/stop
	char *p, *n, *r, *s, *str, *localMessage;
	int i;
	XmlNode *xNode, *invNode, *bNode, *subjectNode;

	if (!node->name || strcmp(node->name, "m")) return;

	if ((f=JabberXmlGetAttrValue(node, "f")) != NULL) {
		char *fLogin = JabberLoginFromJID(f);
		if ((hContact=JabberHContactFromJID(info->proto, fLogin)) != NULL) {
			if ((tp=JabberXmlGetAttrValue(node, "tp")) != NULL) {
				JABBER_LIST_ITEM *item = JabberListGetItemPtr(info->proto, LIST_ROSTER, fLogin);
				if (!strcmp(tp, "t")) { //contact is writing
					if (item != NULL ) {
						item->isTyping = TRUE;
						CallService(MS_PROTO_CONTACTISTYPING, (WPARAM)hContact, (LPARAM)PROTOTYPE_CONTACTTYPING_INFINITE);
					}
				}
				else if (!strcmp(tp, "u")) {//contact stopped writing
					if (item != NULL) {
						item->isTyping = FALSE;
						CallService(MS_PROTO_CONTACTISTYPING, (WPARAM)hContact, (LPARAM)PROTOTYPE_CONTACTTYPING_OFF);
					}
				}
				else if (!strcmp(tp, "a")) {//alert was received
					int bAlert = TRUE;
					if (info->proto->tlenOptions.alertPolicy == TLEN_ALERTS_IGNORE_ALL) {
						bAlert = FALSE;
					} else if (info->proto->tlenOptions.alertPolicy == TLEN_ALERTS_IGNORE_NIR) {
						bAlert = IsAuthorized(info->proto, fLogin);
					}
					if (bAlert) {
						if (info->proto->tlenOptions.useNudge) {
							NotifyEventHooks(info->proto->hTlenNudge,(WPARAM) hContact,0);
						} else {
							if (info->proto->tlenOptions.logAlerts) {
								TlenLogMessage(info->proto, hContact, 0, Translate("An alert has been received."));
							}
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
				s = JabberXmlGetAttrValue(node, "s");
				if (s != NULL) {
					timestamp = TlenTimeToUTC(atol(s));
					if (timestamp > time(NULL)) {
						timestamp = time(NULL);
					}
				} else {
					timestamp = time(NULL);
				}
				tp=JabberXmlGetAttrValue(node, "tp");
				bNode = JabberXmlGetChild(node, "b");
				f = JabberTextDecode(f);
				if (bNode != NULL && bNode->text != NULL) {
					if (tp != NULL && !strcmp(tp, "p")) {
						/* MUC private message */
						str = JabberResourceFromJID(f);
						hContact = JabberDBCreateContact(info->proto, f, str, TRUE);
						db_set_b(hContact, info->proto->m_szModuleName, "bChat", TRUE);
						mir_free(str);
						localMessage = JabberTextDecode(bNode->text);
						recv.flags = 0;
						recv.timestamp = (DWORD) timestamp;
						recv.szMessage = localMessage;
						recv.lParam = 0;
						ProtoChainRecvMsg(hContact, &recv);
						mir_free(localMessage);
					} else {
						/* MUC message */
						TlenMUCRecvMessage(info->proto, f, timestamp, bNode);
					}
				}
				mir_free(f);
			} else { // message from chat room (system)
				subjectNode = JabberXmlGetChild(node, "subject");
				if (subjectNode != NULL) {
					f = JabberTextDecode(f);
					localMessage = "";
					if (subjectNode->text != NULL)  {
						localMessage = subjectNode->text;
					}
					localMessage = JabberTextDecode(localMessage);
					TlenMUCRecvTopic(info->proto, f, localMessage);
					mir_free(localMessage);
					mir_free(f);
				}
			}
		}
		i=1;
		while ((xNode=JabberXmlGetNthChild(node, "x", i)) != NULL) {
			invNode=JabberXmlGetChild(xNode, "inv");
			if (invNode != NULL) {
				r = JabberTextDecode(f);
				f = JabberXmlGetAttrValue(invNode, "f");
				f = JabberTextDecode(f);
				n = JabberXmlGetAttrValue(invNode, "n");
				if (n != NULL && strstr(r, n) != r) {
					n = JabberTextDecode(n);
				} else {
					n = mir_strdup(Translate("Private conference"));
					//n = JabberNickFromJID(r);
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
	if ( !ServiceExists(MS_POPUP_ADDPOPUP))
		return;
	if (!db_get_b(NULL, proto->m_szModuleName, "MailPopupEnabled", TRUE))
		return;

	POPUPDATA ppd = { 0 };
	ppd.lchIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_MAIL));
	strcpy(ppd.lpzContactName, title);
	strcpy(ppd.lpzText, emailInfo);
	ppd.colorBack = db_get_dw(NULL, proto->m_szModuleName, "MailPopupBack", 0);
	ppd.colorText = db_get_dw(NULL, proto->m_szModuleName, "MailPopupText", 0);
	BYTE delayMode = db_get_b(NULL, proto->m_szModuleName, "MailPopupDelayMode", 0);
	if (delayMode == 1)
		ppd.iSeconds = db_get_dw(NULL, proto->m_szModuleName, "MailPopupDelay", 4);
	else if (delayMode == 2)
		ppd.iSeconds = -1;
	PUAddPopup(&ppd);
}
/*
 * Incoming e-mail notification
 */
static void TlenProcessN(XmlNode *node, ThreadData *info)
{
	char *f, *s;
	char *str, *popupTitle, *popupText;
	int strSize;

	if (!node->name || strcmp(node->name, "n")) return;

	s = JabberXmlGetAttrValue(node, "s");
	f = JabberXmlGetAttrValue(node, "f");
	if (s != NULL && f != NULL) {
		str = NULL;
		strSize = 0;

		JabberStringAppend(&str, &strSize, Translate("%s mail"), info->proto->m_szModuleName);
		popupTitle = JabberTextDecode(str);
		mir_free(str);

		str = NULL;
		strSize = 0;

		JabberStringAppend(&str, &strSize, "%s: %s\n", Translate("From"), f);
		JabberStringAppend(&str, &strSize, "%s: %s", Translate("Subject"), s);
		popupText = JabberTextDecode(str);
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
	char jid[512];
	char *f, *id, *tp, *a, *n, *k;
	XmlNode *sNode, *xNode, *iNode, *kNode;
	int status, flags;

	if (!node->name || strcmp(node->name, "p")) return;

// presence from users in chat room
	flags = 0;
	status = ID_STATUS_ONLINE;
	f = JabberXmlGetAttrValue(node, "f");
	xNode = JabberXmlGetChild(node, "x");
	if (xNode != NULL) { // x subtag present (message from chat room) - change user rights only
		char *temp, *iStr;
		iNode = JabberXmlGetChild(xNode, "i");
		if (iNode != NULL) {
			iStr = JabberXmlGetAttrValue(iNode, "i");
			temp = (char*)mir_alloc(strlen(f)+strlen(iStr)+2);
			strcpy(temp, f);
			strcat(temp, "/");
			strcat(temp, iStr);
			f = JabberTextDecode(temp);
			mir_free(temp);
			node = iNode;
			status = 0;
		} else {
			f = JabberTextDecode(f);
		}
	} else {
		f = JabberTextDecode(f);
	}
	a = JabberXmlGetAttrValue(node, "z");
	if (a != NULL) {
		if (atoi(a) &1 ) {
			flags |= USER_FLAGS_REGISTERED;
		}
	}
	a = JabberXmlGetAttrValue(node, "a");
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
	sNode = JabberXmlGetChild(node, "s");
	if (sNode != NULL) {
		if (!strcmp(sNode->text, "unavailable")) {
			status = ID_STATUS_OFFLINE;
		}
	}
	kNode = JabberXmlGetChild(node, "kick");
	k = NULL;
	if (kNode != NULL) {
		k = JabberXmlGetAttrValue(kNode, "r");
		if (k == NULL) {
			k = "";
		}
		k = JabberTextDecode(k);
	}
	tp = JabberXmlGetAttrValue(node, "tp");
	if (tp != NULL && !strcmp(tp, "c")) { // new chat room has just been created
		id = JabberXmlGetAttrValue(node, "id");
		if (id != NULL) {
			n = JabberXmlGetAttrValue(node, "n");
			if (n != NULL) {
				n = JabberTextDecode(n);
			} else {
				n = mir_strdup(Translate("Private conference"));// JabberNickFromJID(f);
			}
			mir_snprintf(jid, SIZEOF(jid), "%s/%s", f, info->username);
//			if (!db_get(NULL, info->proto->m_szModuleName, "LoginName", &dbv)) {
				// always real username
//				sprintf(jid, "%s/%s", f, dbv.pszVal);
			TlenMUCCreateWindow(info->proto, f, n, 0, NULL, id);
			TlenMUCRecvPresence(info->proto, jid, ID_STATUS_ONLINE, flags, k);
//				db_free(&dbv);
//			}
			mir_free(n);
		}
	} else {
		TlenMUCRecvPresence(info->proto, f, status, flags, k); // user presence
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
	JABBER_LIST_ITEM *item;
	char *from, *id, *e, *p;
//	if (!node->name || strcmp(node->name, "v")) return;

	if ((from=JabberXmlGetAttrValue(node, "f")) != NULL) {
		if (strchr(from, '@') == NULL) {
			mir_snprintf(jid, sizeof(jid), "%s@%s", from, info->server);
		} else {
			mir_snprintf(jid, sizeof(jid), "%s", from);
		}
		if ((e=JabberXmlGetAttrValue(node, "e")) != NULL) {
			if (!strcmp(e, "1")) {
				if ((id=JabberXmlGetAttrValue(node, "i")) != NULL) {
					SkinPlaySound("TlenVoiceNotify");
					TlenVoiceAccept(info->proto, id, from);
				}
			} else if (!strcmp(e, "3")) {
				// FILE_RECV : e='3' : invalid transfer error
				if ((p=JabberXmlGetAttrValue(node, "i")) != NULL) {
					if ((item=JabberListGetItemPtr(info->proto, LIST_VOICE, p)) != NULL) {
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
							JabberListRemove(info->proto, LIST_VOICE, p);
						}
					}
				}
			} else if (!strcmp(e, "4")) {
				// FILE_SEND : e='4' : File sending request was denied by the remote client
				if ((p=JabberXmlGetAttrValue(node, "i")) != NULL) {
					if ((item=JabberListGetItemPtr(info->proto, LIST_VOICE, p)) != NULL) {
						if (!strcmp(item->ft->jid, jid)) {
							TlenVoiceCancelAll(info->proto);
							//JabberListRemove(info->proto, LIST_VOICE, p);
						}
					}
				}
			} else if (!strcmp(e, "5")) {
			// FILE_SEND : e='5' : Voice request was accepted
				if ((p=JabberXmlGetAttrValue(node, "i")) != NULL) {
					if ((item=JabberListGetItemPtr(info->proto, LIST_VOICE, p)) != NULL) {
						JabberLog(info->proto, "should start voice 1 ? %s ?? %s", jid, item->ft->jid);
						if (!strcmp(item->ft->jid, jid)) {
							JabberLog(info->proto, "starting voice 1");
							TlenVoiceStart(item->ft, 1);
						}
					}
				}
			} else if (!strcmp(e, "6")) {
				// FILE_RECV : e='6' : IP and port information to connect to get file
				if ((p=JabberXmlGetAttrValue(node, "i")) != NULL) {
					if ((item=JabberListGetItemPtr(info->proto, LIST_VOICE, p)) != NULL) {
						if ((p=JabberXmlGetAttrValue(node, "a")) != NULL) {
							item->ft->hostName = mir_strdup(p);
							if ((p=JabberXmlGetAttrValue(node, "p")) != NULL) {
								item->ft->wPort = atoi(p);
								TlenVoiceStart(item->ft, 0);
								//JabberForkThread((void (__cdecl *)(void*))TlenVoiceReceiveThread, 0, item->ft);
							}
						}
					}
				}
			}
			else if (!strcmp(e, "7")) {
				// FILE_RECV : e='7' : IP and port information to connect to send file
				// in case the conection to the given server was not successful
				if ((p=JabberXmlGetAttrValue(node, "i")) != NULL) {
					if ((item=JabberListGetItemPtr(info->proto, LIST_VOICE, p)) != NULL) {
						if ((p=JabberXmlGetAttrValue(node, "a")) != NULL) {
							if (item->ft->hostName != NULL) mir_free(item->ft->hostName);
							item->ft->hostName = mir_strdup(p);
							if ((p=JabberXmlGetAttrValue(node, "p")) != NULL) {
								item->ft->wPort = atoi(p);
								item->ft->state = FT_SWITCH;
								SetEvent(item->ft->hFileEvent);
							}
						}
					}
				}
			}
			else if (!strcmp(e, "8")) {
				// FILE_RECV : e='8' : transfer error
				if ((p=JabberXmlGetAttrValue(node, "i")) != NULL) {
					if ((item=JabberListGetItemPtr(info->proto, LIST_VOICE, p)) != NULL) {
						item->ft->state = FT_ERROR;
						SetEvent(item->ft->hFileEvent);
					}
				}
			}

		}
	}
}

static void __cdecl JabberKeepAliveThread(void *ptr)
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
			JabberSend(proto, " \t ");
	}
	JabberLog(proto, "Exiting KeepAliveThread");
}

