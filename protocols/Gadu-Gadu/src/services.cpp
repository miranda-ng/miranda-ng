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
	switch(status) {
		case ID_STATUS_AWAY:       prefix = "Away"; break;
		case ID_STATUS_NA:         prefix = "Na"; break;
		case ID_STATUS_DND:        prefix = "Dnd"; break;
		case ID_STATUS_OCCUPIED:   prefix = "Occupied"; break;
		case ID_STATUS_FREECHAT:   prefix = "FreeChat"; break;
		case ID_STATUS_ONLINE:     prefix = "On"; break;
		case ID_STATUS_OFFLINE:    prefix = "Off"; break;
		case ID_STATUS_INVISIBLE:  prefix = "Inv"; break;
		case ID_STATUS_ONTHEPHONE: prefix = "Otp"; break;
		case ID_STATUS_OUTTOLUNCH: prefix = "Otl"; break;
		default: return NULL;
	}

	static char str[64];
	mir_snprintf(str, _countof(str), "%s%s", prefix, suffix);
	return str;
}

//////////////////////////////////////////////////////////
// gets protocol status

TCHAR* GGPROTO::getstatusmsg(int status)
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
		debugLogA("refreshstatus(): Waiting pth_sess thread. Going to connect...");
#endif
		threadwait(&pth_sess);
#ifdef DEBUGMODE
		debugLogA("refreshstatus(): Waiting pth_sess thread - OK");
		debugLogA("refreshstatus(): ForkThreadEx 21 GGPROTO::mainthread");
#endif
		pth_sess.hThread = ForkThreadEx(&GGPROTO::mainthread, NULL, &pth_sess.dwThreadId);
	}
	else
	{
		TCHAR *szMsg = NULL;
		// Select proper msg
		gg_EnterCriticalSection(&modemsg_mutex, "refreshstatus", 69, "modemsg_mutex", 1);
		szMsg = getstatusmsg(status);
		gg_LeaveCriticalSection(&modemsg_mutex, "refreshstatus", 69, 1, "modemsg_mutex", 1);
		T2Utf szMsg_utf8(szMsg);
		if (szMsg_utf8) {
			debugLogA("refreshstatus(): Setting status and away message.");
			gg_EnterCriticalSection(&sess_mutex, "refreshstatus", 70, "sess_mutex", 1);
			gg_change_status_descr(sess, status_m2gg(status, szMsg_utf8 != NULL), szMsg_utf8);
			gg_LeaveCriticalSection(&sess_mutex, "refreshstatus", 70, 1, "sess_mutex", 1);
		}
		else {
			debugLogA("refreshstatus(): Setting just status.");
			gg_EnterCriticalSection(&sess_mutex, "refreshstatus", 71, "sess_mutex", 1);
			gg_change_status(sess, status_m2gg(status, 0));
			gg_LeaveCriticalSection(&sess_mutex, "refreshstatus", 71, 1, "sess_mutex", 1);
		}
		// Change status of the contact with our own UIN (if got yourself added to the contact list)
		changecontactstatus( getDword(GG_KEY_UIN, 0), status_m2gg(status, szMsg != NULL), szMsg, 0, 0, 0, 0);
		broadcastnewstatus(status);
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
// registered as ProtoService PS_GETAVATARCAPS

INT_PTR GGPROTO::getavatarcaps(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case AF_MAXSIZE:
		((POINT *)lParam)->x = ((POINT *)lParam)->y = 200;
		return 0;
	case AF_FORMATSUPPORTED:
		return (lParam == PA_FORMAT_JPEG || lParam == PA_FORMAT_GIF || lParam == PA_FORMAT_PNG);
	case AF_ENABLED:
		return getByte(GG_KEY_ENABLEAVATARS, GG_KEYDEF_ENABLEAVATARS);
	case AF_DONTNEEDDELAYS:
		return 1;
	case AF_MAXFILESIZE:
		return 307200;
	case AF_FETCHIFPROTONOTVISIBLE:
	case AF_FETCHIFCONTACTOFFLINE:
		return 1;
	}
	return 0;
}

//////////////////////////////////////////////////////////
// gets avatar information
// registered as ProtoService PS_GETAVATARINFO

INT_PTR GGPROTO::getavatarinfo(WPARAM wParam, LPARAM lParam)
{
	PROTO_AVATAR_INFORMATION *pai = (PROTO_AVATAR_INFORMATION *)lParam;
	pai->filename[0] = 0;
	pai->format = PA_FORMAT_UNKNOWN;

	uin_t uin = (uin_t)getDword(pai->hContact, GG_KEY_UIN, 0);
	if (!uin) {
		debugLogA("getavatarinfo(): Incoming request for avatar information. No uin found. return GAIR_NOAVATAR");
		return GAIR_NOAVATAR;
	}

	if (!getByte(GG_KEY_ENABLEAVATARS, GG_KEYDEF_ENABLEAVATARS)) {
		debugLogA("getavatarinfo(): Incoming request for avatar information. GG_KEY_ENABLEAVATARS == 0. return GAIR_NOAVATAR");
		return GAIR_NOAVATAR;
	}

	//directly check if contact has protected user avatar set by AVS, and if yes return it as protocol avatar
	DBVARIANT dbv;
	if (!db_get_ts(pai->hContact, "ContactPhoto", "Backup", &dbv)) {
		if ((mir_tstrlen(dbv.ptszVal)>0) && db_get_b(pai->hContact, "ContactPhoto", "Locked", 0)){
			debugLogA("getavatarinfo(): Incoming request for avatar information. Contact has assigned Locked ContactPhoto. return GAIR_SUCCESS");
			_tcscpy_s(pai->filename, SIZEOF(pai->filename) ,dbv.ptszVal);
			pai->format = ProtoGetAvatarFormat(pai->filename);
			db_free(&dbv);
			return GAIR_SUCCESS;
		}
		db_free(&dbv);
	}

	if (!getByte(pai->hContact, GG_KEY_AVATARREQUESTED, GG_KEYDEF_AVATARREQUESTED)) {
		requestAvatarInfo(pai->hContact, 1);
		if ((wParam & GAIF_FORCE) != 0) {
			debugLogA("getavatarinfo(): Incoming request for avatar information. uin=%d. requestAvatarInfo() fired. return GAIR_WAITFOR", uin);
			return GAIR_WAITFOR;
		} else {
			debugLogA("getavatarinfo(): Incoming request for avatar information. uin=%d. requestAvatarInfo() fired. return GAIR_NOAVATAR", uin);
			return GAIR_NOAVATAR;
		}
	}

	pai->format = getByte(pai->hContact, GG_KEY_AVATARTYPE, GG_KEYDEF_AVATARTYPE);

	ptrA AvatarHash(NULL);
	ptrA AvatarURL( getStringA(pai->hContact, GG_KEY_AVATARURL));
	ptrA AvatarTs( getStringA(pai->hContact, GG_KEY_AVATARTS));
	if (AvatarURL != NULL && AvatarTs != NULL) {
		char *AvatarName = strrchr(AvatarURL, '/');
		AvatarName++;
		char AvatarNameWithTS[128];
		mir_snprintf(AvatarNameWithTS, SIZEOF(AvatarNameWithTS), "%s%s", AvatarName, AvatarTs);
		AvatarHash = gg_avatarhash(AvatarNameWithTS);
	}

	ptrA AvatarSavedHash( getStringA(pai->hContact, GG_KEY_AVATARHASH));
	if (AvatarHash != NULL && AvatarSavedHash != NULL) {
		getAvatarFilename(pai->hContact, pai->filename, SIZEOF(pai->filename));
		if (!mir_strcmp(AvatarHash, AvatarSavedHash)) {
			if (_taccess(pai->filename, 0) == 0){
				debugLogA("getavatarinfo(): Incoming request for avatar information. uin=%d. Avatar hash unchanged. return GAIR_SUCCESS", uin);
				return GAIR_SUCCESS;
			}

			requestAvatarTransfer(pai->hContact, AvatarURL);
			debugLog(_T("getavatarinfo(): Incoming request for avatar information. uin=%d. Avatar hash unchanged but file %s does not exist. errno=%d: %s. requestAvatarTransfer() fired. return GAIR_WAITFOR"), uin, pai->filename, errno, strerror(errno));
			return GAIR_WAITFOR;
		}
		if ((wParam & GAIF_FORCE) != 0) {
			if (_tremove(pai->filename) != 0){
				debugLog(_T("getavatarinfo(): refresh. _tremove 1 file %s error. errno=%d: %s"), pai->filename, errno, strerror(errno));
				TCHAR error[512];
				mir_sntprintf(error, SIZEOF(error), TranslateT("Cannot remove old avatar file before refresh. ERROR: %d: %s\n%s"), errno, _tcserror(errno), pai->filename);
				showpopup(m_tszUserName, error, GG_POPUP_ERROR);
			}
			setString(pai->hContact, GG_KEY_AVATARHASH, AvatarHash);
			requestAvatarTransfer(pai->hContact, AvatarURL);
			debugLogA("getavatarinfo(): Incoming request for avatar information. uin=%d. Avatar hash changed, requestAvatarTransfer() fired. return GAIR_WAITFOR", uin);
			return GAIR_WAITFOR;
		}
	}
	else if ((wParam & GAIF_FORCE) != 0) {
		if (AvatarHash == NULL && AvatarSavedHash != NULL) {
			getAvatarFilename(pai->hContact, pai->filename, sizeof(pai->filename));
			if (_tremove(pai->filename) != 0){
				debugLog(_T("getavatarinfo(): delete. _tremove file %s error. errno=%d: %s"), pai->filename, errno, strerror(errno));
				TCHAR error[512];
				mir_sntprintf(error, SIZEOF(error), TranslateT("Cannot remove old avatar file. ERROR: %d: %s\n%s"), errno, _tcserror(errno), pai->filename);
				showpopup(m_tszUserName, error, GG_POPUP_ERROR);
			}
			delSetting(pai->hContact, GG_KEY_AVATARHASH);
			delSetting(pai->hContact, GG_KEY_AVATARURL);
			delSetting(pai->hContact, GG_KEY_AVATARTYPE);
			debugLogA("getavatarinfo(): Incoming request for avatar information. Contact %d deleted avatar. return GAIR_NOAVATAR", uin);
		}
		else if (AvatarHash != NULL && AvatarSavedHash == NULL) {
			setString(pai->hContact, GG_KEY_AVATARHASH, AvatarHash);
			requestAvatarTransfer(pai->hContact, AvatarURL);
			debugLogA("getavatarinfo(): Incoming request for avatar information. Contact %d set avatar. requestAvatarTransfer() fired. return GAIR_WAITFOR", uin);
			return GAIR_WAITFOR;
		}
		else debugLogA("getavatarinfo(): Incoming request for avatar information. uin=%d. AvatarHash==AvatarSavedHash==NULL, with GAIF_FORCE param. return GAIR_NOAVATAR", uin);
	}
	else debugLogA("getavatarinfo(): Incoming request for avatar information. uin=%d. AvatarHash==null or AvatarSavedHash==null, but no GAIF_FORCE param. return GAIR_NOAVATAR", uin);

	return GAIR_NOAVATAR;
}

//////////////////////////////////////////////////////////
// gets avatar
// registered as ProtoService PS_GETMYAVATAR

INT_PTR GGPROTO::getmyavatar(WPARAM wParam, LPARAM lParam)
{
	TCHAR *szFilename = (TCHAR*)wParam;
	int len = (int)lParam;

	if (szFilename == NULL || len <= 0) {
		debugLogA("getmyavatar(): Incoming request for self avatar information. szFilename == NULL. return -1 (error)");
		return -1;
	}

	if (!getByte(GG_KEY_ENABLEAVATARS, GG_KEYDEF_ENABLEAVATARS)) {
		debugLogA("getmyavatar(): Incoming request for self avatar information. GG_KEY_ENABLEAVATARS==0. return -2 (error)");
		return -2;
	}

	getAvatarFilename(NULL, szFilename, len);
	if (_taccess(szFilename, 0) == 0){
		debugLogA("getmyavatar(): Incoming request for self avatar information. returned ok.");
		return 0;
	} else {
		debugLog(_T("getmyavatar(): Incoming request for self avatar information. saved avatar file %s does not exist. return -1 (error)"), szFilename);
		return -1;
	}

}

//////////////////////////////////////////////////////////
// sets avatar
// registered as ProtoService PS_SETMYAVATAR

INT_PTR GGPROTO::setmyavatar(WPARAM wParam, LPARAM lParam)
{
	TCHAR *szFilename = (TCHAR*)lParam;

	if (!getByte(GG_KEY_ENABLEAVATARS, GG_KEYDEF_ENABLEAVATARS))
		return -2;

	if (szFilename == NULL) {
		MessageBox(NULL, 
			TranslateT("To remove your Gadu-Gadu avatar, you must use the gg.pl website."),
			m_tszUserName, MB_OK | MB_ICONINFORMATION);
		return -1;
	}

	int iAvType = ProtoGetAvatarFormat(szFilename);
	if ( iAvType == PA_FORMAT_UNKNOWN) {
		debugLogA("setmyavatar(): Failed to set user avatar. File %s has incompatible extension.", szFilename);
		return -1;
	}

	setByte(GG_KEY_AVATARTYPEPREV, getByte(GG_KEY_AVATARTYPE, -1));
	setByte(GG_KEY_AVATARTYPE, (BYTE)iAvType);

	TCHAR szMyFilename[MAX_PATH];
	getAvatarFilename(NULL, szMyFilename, SIZEOF(szMyFilename));
	if ( mir_tstrcmp(szFilename, szMyFilename) && !CopyFile(szFilename, szMyFilename, FALSE)) {
		debugLogA("setmyavatar(): Failed to set user avatar. File with type %d could not be created/overwritten.", iAvType);
		return -1;
	}

	setAvatar(szMyFilename);
	return 0;
}

//////////////////////////////////////////////////////////
// gets protocol status message
// registered as ProtoService PS_GETMYAWAYMSG

INT_PTR GGPROTO::getmyawaymsg(WPARAM wParam, LPARAM lParam)
{
	INT_PTR res = 0;
	TCHAR *szMsg;

	gg_EnterCriticalSection(&modemsg_mutex, "refreshstatus", 72, "modemsg_mutex", 1);
	szMsg = getstatusmsg(wParam ? gg_normalizestatus(wParam) : m_iStatus);
	if (isonline() && szMsg)
		res = (lParam & SGMA_UNICODE) ? (INT_PTR)mir_t2u(szMsg) : (INT_PTR)mir_t2a(szMsg);
	gg_LeaveCriticalSection(&modemsg_mutex, "refreshstatus", 72, 1, "modemsg_mutex", 1);
	return res;
}

//////////////////////////////////////////////////////////
// gets account manager GUI
// registered as ProtoService PS_CREATEACCMGRUI

extern INT_PTR CALLBACK gg_acc_mgr_guidlgproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

INT_PTR GGPROTO::get_acc_mgr_gui(WPARAM wParam, LPARAM lParam)
{
	return (INT_PTR) CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_ACCMGRUI), (HWND)lParam, gg_acc_mgr_guidlgproc, (LPARAM)this);
}

//////////////////////////////////////////////////////////
// leaves (terminates) conference
// registered as ProtoService PS_LEAVECHAT

INT_PTR GGPROTO::leavechat(WPARAM hContact, LPARAM lParam)
{
	if (hContact)
		CallService(MS_DB_CONTACT_DELETE, hContact, 0);

	return 0;
}
