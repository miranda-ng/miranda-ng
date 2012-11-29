////////////////////////////////////////////////////////////////////////////////
// Gadu-Gadu Plugin for Miranda IM
//
// Copyright (c) 2003-2009 Adam Strzelecki <ono+miranda@java.pl>
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

//////////////////////////////////////////////////////////
// Status mode -> DB
char *gg_status2db(int status, const char *suffix)
{
	char *prefix;
	static char str[64];

	switch(status) {
		case ID_STATUS_AWAY:		prefix = "Away"; break;
		case ID_STATUS_NA:			prefix = "Na"; break;
		case ID_STATUS_DND:			prefix = "Dnd"; break;
		case ID_STATUS_OCCUPIED:	prefix = "Occupied"; break;
		case ID_STATUS_FREECHAT:	prefix = "FreeChat"; break;
		case ID_STATUS_ONLINE:		prefix = "On"; break;
		case ID_STATUS_OFFLINE:		prefix = "Off"; break;
		case ID_STATUS_INVISIBLE:	prefix = "Inv"; break;
		case ID_STATUS_ONTHEPHONE:	prefix = "Otp"; break;
		case ID_STATUS_OUTTOLUNCH:	prefix = "Otl"; break;
		default: return NULL;
	}
	strncpy(str, prefix, sizeof(str));
	strncat(str, suffix, sizeof(str) - strlen(str));
	return str;
}

//////////////////////////////////////////////////////////
// gets protocol status

char* GGPROTO::getstatusmsg(int status)
{
	switch(status) {
	case ID_STATUS_ONLINE:
		return modemsg.online;
		break;
	case ID_STATUS_DND:
		return modemsg.dnd;
		break;
	case ID_STATUS_FREECHAT:
		return modemsg.freechat;
		break;
	case ID_STATUS_INVISIBLE:
		return modemsg.invisible;
		break;
	case ID_STATUS_AWAY:
	default:
		return modemsg.away;
	}
}

//////////////////////////////////////////////////////////
// sets specified protocol status

int GGPROTO::refreshstatus(int status)
{
	if (status == ID_STATUS_OFFLINE)
	{
		disconnect();
		return TRUE;
	}

	if (!isonline())
	{
		DWORD exitCode = 0;
		GetExitCodeThread(pth_sess.hThread, &exitCode);
		if (exitCode == STILL_ACTIVE)
			return TRUE;
#ifdef DEBUGMODE
		netlog("refreshstatus(): Waiting pth_sess thread. Going to connect...");
#endif
		threadwait(&pth_sess);
#ifdef DEBUGMODE
		netlog("refreshstatus(): Waiting pth_sess thread - OK");
		netlog("refreshstatus(): forkthreadex 21 GGPROTO::mainthread");
#endif
		pth_sess.hThread = forkthreadex(&GGPROTO::mainthread, NULL, &pth_sess.dwThreadId);
	}
	else
	{
		char *szMsg = NULL;
		// Select proper msg
		gg_EnterCriticalSection(&modemsg_mutex, "refreshstatus", 69, "modemsg_mutex", 1);
		szMsg = mir_strdup(getstatusmsg(status));
		gg_LeaveCriticalSection(&modemsg_mutex, "refreshstatus", 69, 1, "modemsg_mutex", 1);
		if (szMsg)
		{
			netlog("refreshstatus(): Setting status and away message.");
			gg_EnterCriticalSection(&sess_mutex, "refreshstatus", 70, "sess_mutex", 1);
			gg_change_status_descr(sess, status_m2gg(status, szMsg != NULL), szMsg);
			gg_LeaveCriticalSection(&sess_mutex, "refreshstatus", 70, 1, "sess_mutex", 1);
		}
		else
		{
			netlog("refreshstatus(): Setting just status.");
			gg_EnterCriticalSection(&sess_mutex, "refreshstatus", 71, "sess_mutex", 1);
			gg_change_status(sess, status_m2gg(status, 0));
			gg_LeaveCriticalSection(&sess_mutex, "refreshstatus", 71, 1, "sess_mutex", 1);
		}
		// Change status of the contact with our own UIN (if got yourself added to the contact list)
		changecontactstatus( db_get_dw(NULL, m_szModuleName, GG_KEY_UIN, 0), status_m2gg(status, szMsg != NULL), szMsg, 0, 0, 0, 0);
		broadcastnewstatus(status);
		mir_free(szMsg);
	}

	return TRUE;
}

//////////////////////////////////////////////////////////
// normalize gg status

int gg_normalizestatus(int status)
{
	switch(status) {
		case ID_STATUS_ONLINE:    return ID_STATUS_ONLINE;
		case ID_STATUS_DND:       return ID_STATUS_DND;
		case ID_STATUS_FREECHAT:  return ID_STATUS_FREECHAT;
		case ID_STATUS_OFFLINE:   return ID_STATUS_OFFLINE;
		case ID_STATUS_INVISIBLE: return ID_STATUS_INVISIBLE;
	}
	return ID_STATUS_AWAY;
}

//////////////////////////////////////////////////////////
// gets avatar capabilities

INT_PTR GGPROTO::getavatarcaps(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case AF_MAXSIZE:
		((POINT *)lParam)->x = ((POINT *)lParam)->y = 200;
		return 0;
	case AF_FORMATSUPPORTED:
		return (lParam == PA_FORMAT_JPEG || lParam == PA_FORMAT_GIF || lParam == PA_FORMAT_PNG);
	case AF_ENABLED:
		return db_get_b(NULL, m_szModuleName, GG_KEY_ENABLEAVATARS, GG_KEYDEF_ENABLEAVATARS);
	case AF_DONTNEEDDELAYS:
		return 1;
	case AF_MAXFILESIZE:
		return 307200;
	case AF_FETCHALWAYS:
		return 1;
	}
	return 0;
}

//////////////////////////////////////////////////////////
// gets avatar information

INT_PTR GGPROTO::getavatarinfo(WPARAM wParam, LPARAM lParam)
{
	PROTO_AVATAR_INFORMATIONT *pai = (PROTO_AVATAR_INFORMATIONT *)lParam;
	char *AvatarHash = NULL, *AvatarSavedHash = NULL;
	char *AvatarURL = NULL;
	char *AvatarTs = NULL;
	INT_PTR result = GAIR_NOAVATAR;
	DBVARIANT dbv;
	uin_t uin = (uin_t)db_get_dw(pai->hContact, m_szModuleName, GG_KEY_UIN, 0);

	netlog("getavatarinfo(): Requesting avatar information for %d.", uin);

	pai->filename[0] = 0;
	pai->format = PA_FORMAT_UNKNOWN;

	if (!uin || !db_get_b(NULL, m_szModuleName, GG_KEY_ENABLEAVATARS, GG_KEYDEF_ENABLEAVATARS))
		return GAIR_NOAVATAR;

	if (!db_get_b(pai->hContact, m_szModuleName, GG_KEY_AVATARREQUESTED, GG_KEYDEF_AVATARREQUESTED)) {
		requestAvatar(pai->hContact, 1);
		return (wParam & GAIF_FORCE) != 0 ? GAIR_WAITFOR : GAIR_NOAVATAR;
	}
	db_unset(pai->hContact, m_szModuleName, GG_KEY_AVATARREQUESTED);

	pai->format = db_get_b(pai->hContact, m_szModuleName, GG_KEY_AVATARTYPE, GG_KEYDEF_AVATARTYPE);

	if (!db_get_s(pai->hContact, m_szModuleName, GG_KEY_AVATARURL, &dbv, DBVT_ASCIIZ)) {
		AvatarURL = mir_strdup(dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	if (!db_get_s(pai->hContact, m_szModuleName, GG_KEY_AVATARTS, &dbv, DBVT_ASCIIZ)) {
		AvatarTs = mir_strdup(dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	if (AvatarURL != NULL && strlen(AvatarURL) > 0 && AvatarTs != NULL && strlen(AvatarTs) > 0) {
		char *AvatarName = strrchr(AvatarURL, '/');
		AvatarName++;
		char AvatarNameWithTS[128];
		sprintf(AvatarNameWithTS, "%s%s", AvatarName, AvatarTs);
		AvatarHash = gg_avatarhash(AvatarNameWithTS);
	}

	if (!db_get_s(pai->hContact, m_szModuleName, GG_KEY_AVATARHASH, &dbv, DBVT_ASCIIZ)) {
		AvatarSavedHash = mir_strdup(dbv.pszVal);
		DBFreeVariant(&dbv);
	}

	if (AvatarHash != NULL && AvatarSavedHash != NULL) {
		getAvatarFilename(pai->hContact, pai->filename, SIZEOF(pai->filename));
		if (!strcmp(AvatarHash, AvatarSavedHash) && !_taccess(pai->filename, 0)) {
			result = GAIR_SUCCESS;
		}
		else if ((wParam & GAIF_FORCE) != 0) {
			netlog("getavatarinfo(): Contact %d changed avatar.", uin);
			_tremove(pai->filename);
			db_set_s(pai->hContact, m_szModuleName, GG_KEY_AVATARHASH, AvatarHash);
			getAvatar(pai->hContact, AvatarURL);
			result = GAIR_WAITFOR;
		}
	}
	else if ((wParam & GAIF_FORCE) != 0) {
		if (AvatarHash == NULL && AvatarSavedHash != NULL) {
			netlog("getavatarinfo(): Contact %d deleted avatar.", uin);
			getAvatarFilename(pai->hContact, pai->filename, sizeof(pai->filename));
			_tremove(pai->filename);
			db_unset(pai->hContact, m_szModuleName, GG_KEY_AVATARHASH);
			db_unset(pai->hContact, m_szModuleName, GG_KEY_AVATARURL);
			db_unset(pai->hContact, m_szModuleName, GG_KEY_AVATARTYPE);
		}
		else if (AvatarHash != NULL && AvatarSavedHash == NULL) {
			netlog("getavatarinfo(): Contact %d set avatar.", uin);
			db_set_s(pai->hContact, m_szModuleName, GG_KEY_AVATARHASH, AvatarHash);
			getAvatar(pai->hContact, AvatarURL);
			result = GAIR_WAITFOR;
		}
	}

	mir_free(AvatarHash);
	mir_free(AvatarSavedHash);
	mir_free(AvatarURL);

	return result;
}

//////////////////////////////////////////////////////////
// gets avatar

INT_PTR GGPROTO::getmyavatar(WPARAM wParam, LPARAM lParam)
{
	TCHAR *szFilename = (TCHAR*)wParam;
	int len = (int)lParam;

	netlog("getmyavatar(): Requesting user avatar.");

	if (szFilename == NULL || len <= 0)
		return -1;

	if (!db_get_b(NULL, m_szModuleName, GG_KEY_ENABLEAVATARS, GG_KEYDEF_ENABLEAVATARS))
		return -2;

	getAvatarFilename(NULL, szFilename, len);
	return _taccess(szFilename, 0);
}

//////////////////////////////////////////////////////////
// sets avatar

INT_PTR GGPROTO::setmyavatar(WPARAM wParam, LPARAM lParam)
{
	TCHAR *szFilename = (TCHAR*)lParam;

	if (!db_get_b(NULL, m_szModuleName, GG_KEY_ENABLEAVATARS, GG_KEYDEF_ENABLEAVATARS))
		return -2;

	if (szFilename == NULL) {
		MessageBox(NULL, 
			TranslateT("To remove your Gadu-Gadu avatar, you must use the gg.pl website."),
			m_tszUserName, MB_OK | MB_ICONINFORMATION);
		return -1;
	}
	

	TCHAR *szAvType = _tcsrchr(szFilename, '.');
	int iAvType = -1;
	szAvType++;
	if (!_tcsicmp(szAvType, _T("jpg")))
		iAvType = PA_FORMAT_JPEG;
	else if (!_tcsicmp(szAvType, _T("gif")))
		iAvType = PA_FORMAT_GIF;
	else if (!_tcsicmp(szAvType, _T("png")))
		iAvType = PA_FORMAT_PNG;

	if ( iAvType == -1) {
		netlog("setmyavatar(): Failed to set user avatar. File %S has incompatible extansion.", szAvType);
		return -1;
	}

	db_set_b(NULL, m_szModuleName, GG_KEY_AVATARTYPEPREV, db_get_b(NULL, m_szModuleName, GG_KEY_AVATARTYPE, -1));
	db_set_b(NULL, m_szModuleName, GG_KEY_AVATARTYPE, (BYTE)iAvType);

	TCHAR szMyFilename[MAX_PATH];
	getAvatarFilename(NULL, szMyFilename, SIZEOF(szMyFilename));
	if ( _tcscmp(szFilename, szMyFilename) && !CopyFile(szFilename, szMyFilename, FALSE)) {
		netlog("setmyavatar(): Failed to set user avatar. File with type %d could not be created/overwritten.", iAvType);
		return -1;
	}

	setAvatar(szMyFilename);
	return 0;
}

//////////////////////////////////////////////////////////
// gets protocol status message

INT_PTR GGPROTO::getmyawaymsg(WPARAM wParam, LPARAM lParam)
{
	INT_PTR res = 0;
	char *szMsg;

	gg_EnterCriticalSection(&modemsg_mutex, "refreshstatus", 72, "modemsg_mutex", 1);
	szMsg = getstatusmsg(wParam ? gg_normalizestatus(wParam) : m_iStatus);
	if (isonline() && szMsg)
		res = (lParam & SGMA_UNICODE) ? (INT_PTR)mir_a2u(szMsg) : (INT_PTR)mir_strdup(szMsg);
	gg_LeaveCriticalSection(&modemsg_mutex, "refreshstatus", 72, 1, "modemsg_mutex", 1);
	return res;
}

//////////////////////////////////////////////////////////
// gets account manager GUI

extern INT_PTR CALLBACK gg_acc_mgr_guidlgproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

INT_PTR GGPROTO::get_acc_mgr_gui(WPARAM wParam, LPARAM lParam)
{
	return (INT_PTR) CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_ACCMGRUI), (HWND)lParam, gg_acc_mgr_guidlgproc, (LPARAM)this);
}

//////////////////////////////////////////////////////////
// leaves (terminates) conference

INT_PTR GGPROTO::leavechat(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	if (hContact)
		CallService(MS_DB_CONTACT_DELETE, (WPARAM)hContact, 0);

	return 0;
}
