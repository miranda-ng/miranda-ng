/*
	 ICQ Corporate protocol plugin for Miranda IM.
	 Copyright (C) 2003-2005 Eugene Tarasenko <zlyden13@inbox.ru>

	 This program is free software; you can redistribute it and/or modify
	 it under the terms of the GNU General Public License as published by
	 the Free Software Foundation; either version 2 of the License, or
	 (at your option) any later version.

	 This program is distributed in the hope that it will be useful,
	 but WITHOUT ANY WARRANTY; without even the implied warranty of
	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	 GNU General Public License for more details.

	 You should have received a copy of the GNU General Public License
	 along with this program; if not, write to the Free Software
	 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqGetCaps(WPARAM wParam, LPARAM)
{
	switch (wParam) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_FILE | PF1_MODEMSG | PF1_AUTHREQ | PF1_PEER2PEER | PF1_BASICSEARCH | PF1_EXTSEARCH | PF1_CANRENAMEFILE | PF1_FILERESUME | PF1_ADDSEARCHRES | PF1_SEARCHBYEMAIL | PF1_SEARCHBYNAME | PF1_NUMERICUSERID;

	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND | PF2_FREECHAT;

	case PFLAGNUM_3:
		return PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND | PF2_FREECHAT;

	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)TranslateT("ICQ number");
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqLoadIcon(WPARAM wParam, LPARAM)
{
	int id;

	switch (wParam & 0xFFFF) {
	case PLI_PROTOCOL: id = IDI_ICQCORP; break;
	default: return NULL;
	}
	return (INT_PTR)LoadImage(g_plugin.getInst(), MAKEINTRESOURCE(id), IMAGE_ICON, GetSystemMetrics(wParam & PLIF_SMALL ? SM_CXSMICON : SM_CXICON), GetSystemMetrics(wParam & PLIF_SMALL ? SM_CYSMICON : SM_CYICON), 0);
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqSetStatus(WPARAM wParam, LPARAM)
{
	unsigned short desiredStatus = (unsigned short)wParam;

	Netlib_Logf(hNetlibUser, "[   ] set status\n");

	// on change status to online set away msg not calling
	if (desiredStatus == ID_STATUS_ONLINE)
		icq.awayMessage[0] = 0;

	if (icq.desiredStatus == desiredStatus)
		return 0;
	
	if (desiredStatus == ID_STATUS_OFFLINE) {
		icq.desiredStatus = desiredStatus;
		icq.logoff(false);
	}
	else {
		if (icq.statusVal == ID_STATUS_OFFLINE)
			icq.logon(desiredStatus);
		else
			icq.setStatus(desiredStatus);
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqGetStatus(WPARAM, LPARAM)
{
	return icq.statusVal;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqAuthAllow(WPARAM, LPARAM)
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqAuthDeny(WPARAM, LPARAM)
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqBasicSearch(WPARAM, LPARAM lParam)
{
	Netlib_Logf(hNetlibUser, "[   ] basic search\n");
	icq.startSearch(0, 0, (char*)lParam, 0);
	return 1;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqSearchByEmail(WPARAM, LPARAM lParam)
{
	Netlib_Logf(hNetlibUser, "[   ] search by e-mail\n");
	icq.startSearch(4, 0, (char*)lParam, 0);
	return 1;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqSearchByName(WPARAM, LPARAM lParam)
{
	Netlib_Logf(hNetlibUser, "[   ] search by name\n");

	PROTOSEARCHBYNAME *psbn = (PROTOSEARCHBYNAME*)lParam;
	icq.startSearch(1, 0, (char*)psbn->pszNick, 0);
	return 1;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqAddToList(WPARAM wParam, LPARAM lParam)
{
	Netlib_Logf(hNetlibUser, "[   ] add user to list\n");

	ICQSEARCHRESULT *isr = (ICQSEARCHRESULT *)lParam;
	if (isr->hdr.cbSize != sizeof(ICQSEARCHRESULT) || isr->uin == icq.dwUIN)
		return NULL;
	
	bool persistent = (wParam & PALF_TEMPORARY) == 0;
	return icq.addUser(isr->uin, persistent)->hContact;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqGetInfo(WPARAM, LPARAM lParam)
{
	Netlib_Logf(hNetlibUser, "[   ] get user info\n");

	CCSDATA *ccs = (CCSDATA *)lParam;
	ICQUser *u = icq.getUserByContact(ccs->hContact);
	if (u == nullptr || icq.statusVal <= ID_STATUS_OFFLINE)
		return 1;

	icq.getUserInfo(u, ccs->wParam & SGIF_MINIMAL);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqSendMessage(WPARAM, LPARAM lParam)
{
	Netlib_Logf(hNetlibUser, "[   ] send message\n");

	CCSDATA *ccs = (CCSDATA *)lParam;
	ICQUser *u = icq.getUserByContact(ccs->hContact);
	if (u == nullptr || icq.statusVal <= ID_STATUS_OFFLINE)
		return 0;

	ICQEvent *icqEvent = icq.sendMessage(u, ptrA(mir_utf8decodeA((char*)ccs->lParam)));
	return icqEvent ? icqEvent->sequence : 0;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqRecvMessage(WPARAM, LPARAM lParam)
{
	Netlib_Logf(hNetlibUser, "[   ] receive message\n");

	CCSDATA *ccs = (CCSDATA*)lParam;
	Contact::Hide(ccs->hContact, false);

	auto &dbei = *(DB::EventInfo*)ccs->lParam;
	ptrA szMsg(mir_utf8encode(dbei.pBlob));

	dbei.szModule = protoName;
	dbei.flags = DBEF_UTF;
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.cbBlob = (uint32_t)mir_strlen(szMsg) + 1;
	dbei.pBlob = szMsg;
	db_event_add(ccs->hContact, &dbei);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqSetAwayMsg(WPARAM, LPARAM lParam)
{
	Netlib_Logf(hNetlibUser, "[   ] set away msg\n");

	if (lParam == NULL) return 0;

	if (icq.awayMessage) delete[] icq.awayMessage;
	icq.awayMessage = new char[mir_strlen((char*)lParam) + 1];
	mir_strcpy(icq.awayMessage, (char*)lParam);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqGetAwayMsg(WPARAM, LPARAM lParam)
{
	Netlib_Logf(hNetlibUser, "[   ] send get away msg\n");

	CCSDATA *ccs = (CCSDATA *)lParam;
	ICQUser *u = icq.getUserByContact(ccs->hContact);
	if (u == nullptr || u->statusVal <= ID_STATUS_ONLINE) return 0;

	ICQEvent *icqEvent = icq.sendReadAwayMsg(u);
	return icqEvent ? icqEvent->sequence : 0;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqRecvAwayMsg(WPARAM, LPARAM lParam)
{
	Netlib_Logf(hNetlibUser, "[   ] receive away message\n");

	CCSDATA *ccs = (CCSDATA *)lParam;
	auto *dbei = (DB::EventInfo *)ccs->lParam;
	ProtoBroadcastAck(protoName, ccs->hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)dbei->cbBlob, _A2T(dbei->pBlob));
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqSendFile(WPARAM, LPARAM lParam)
{
	Netlib_Logf(hNetlibUser, "[   ] send file\n");

	CCSDATA *ccs = (CCSDATA *)lParam;
	ICQUser *u = icq.getUserByContact(ccs->hContact);
	if (u == nullptr || u->statusVal == ID_STATUS_OFFLINE || icq.statusVal <= ID_STATUS_OFFLINE)
		return 0;

	unsigned long filesCount, directoriesCount, filesSize = 0;
	char filename[MAX_PATH], format[32];
	WIN32_FIND_DATAW findData;

	wchar_t **files = (wchar_t**)ccs->lParam;
	for (filesCount = 0, directoriesCount = 0; files[filesCount]; filesCount++) {
		FindClose(FindFirstFileW(files[filesCount], &findData));
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) directoriesCount++;
		else filesSize += findData.nFileSizeLow;
	}
	filesCount -= directoriesCount;

	if (directoriesCount) {
		sprintf(format, "%s, %s", filesCount == 1 ? Translate("%d file") : Translate("%d files"), directoriesCount == 1 ? Translate("%d directory") : Translate("%d directories"));
		sprintf(filename, format, filesCount, directoriesCount);
	}
	else {
		if (filesCount == 1) {
			wchar_t *p = wcsrchr(files[0], '\\');
			mir_strcpy(filename, _T2A(p ? p + 1 : files[0]));
		}
		else sprintf(filename, filesCount == 1 ? Translate("%d file") : Translate("%d files"), filesCount);
	}

	return (INT_PTR)icq.sendFile(u, (char*)ccs->wParam, filename, filesSize, files);
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqFileAllow(WPARAM, LPARAM lParam)
{
	Netlib_Logf(hNetlibUser, "[   ] send accept file request\n");

	CCSDATA *ccs = (CCSDATA *)lParam;
	ICQUser *u = icq.getUserByContact(ccs->hContact);
	if (u == nullptr || u->statusVal == ID_STATUS_OFFLINE)
		return 0;

	ICQTransfer *t = (ICQTransfer *)ccs->wParam;
	t->path = _wcsdup((wchar_t*)ccs->lParam);

	icq.acceptFile(u, t->sequence, (char*)ccs->lParam);
	return (INT_PTR)t;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqFileDeny(WPARAM, LPARAM lParam)
{
	Netlib_Logf(hNetlibUser, "[   ] send refuse file request\n");

	CCSDATA *ccs = (CCSDATA *)lParam;
	ICQUser *u = icq.getUserByContact(ccs->hContact);
	if (u == nullptr || u->statusVal == ID_STATUS_OFFLINE)
		return 0;

	ICQTransfer *t = (ICQTransfer *)ccs->wParam;
	icq.refuseFile(u, t->sequence, (char*)ccs->lParam);

	unsigned int i;
	for (i = 0; i < icqTransfers.size(); i++) {
		if (icqTransfers[i] == t) {
			delete icqTransfers[i];
			icqTransfers[i] = icqTransfers[icqTransfers.size() - 1];
			icqTransfers.pop_back();
			break;
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqFileCancel(WPARAM, LPARAM lParam)
{
	Netlib_Logf(hNetlibUser, "[   ] file cancel\n");

	CCSDATA *ccs = (CCSDATA *)lParam;
	ICQUser *u = icq.getUserByContact(ccs->hContact);
	if (u == nullptr || u->statusVal == ID_STATUS_OFFLINE)
		return 0;

	ICQTransfer *t = (ICQTransfer *)ccs->wParam;

	unsigned int i;
	for (i = 0; i < icqTransfers.size(); i++) {
		if (icqTransfers[i] == t) {
			delete icqTransfers[i];
			icqTransfers[i] = icqTransfers[icqTransfers.size() - 1];
			icqTransfers.pop_back();
			break;
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqRecvFile(WPARAM, LPARAM lParam)
{
	Netlib_Logf(hNetlibUser, "[   ] receive file\n");

	CCSDATA *ccs = (CCSDATA *)lParam;
	Contact::Hide(ccs->hContact, false);

	auto &dbei = *(DB::EventInfo *)ccs->lParam;
	dbei.szModule = protoName;
	dbei.eventType = EVENTTYPE_FILE;
	db_event_add(ccs->hContact, &dbei);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqFileResume(WPARAM wParam, LPARAM lParam)
{
	Netlib_Logf(hNetlibUser, "[   ] send file resume\n");

	PROTOFILERESUME *pfr = (PROTOFILERESUME*)lParam;
	ICQTransfer *t = (ICQTransfer *)wParam;
	t->resume(pfr->action, pfr->szFilename);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

static int icqContactDeleted(WPARAM hContact, LPARAM)
{
	Netlib_Logf(hNetlibUser, "[   ] contact deleted\n");

	ICQUser *u = icq.getUserByContact(hContact);
	if (u != nullptr)
		icq.removeUser(u);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

static int icqModulesLoaded(WPARAM, LPARAM)
{
	HookEvent(ME_USERINFO_INITIALISE, icqUserInfoInitialise);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

int LoadServices()
{
	if (!icq.load()) return 1;

	HookEvent(ME_DB_CONTACT_DELETED, icqContactDeleted);
	HookEvent(ME_OPT_INITIALISE, icqOptionsInitialise);
	HookEvent(ME_SYSTEM_MODULESLOADED, icqModulesLoaded);

	CreateProtoServiceFunction(protoName, PS_GETCAPS, icqGetCaps);
	CreateProtoServiceFunction(protoName, PS_LOADICON, icqLoadIcon);
	CreateProtoServiceFunction(protoName, PS_SETSTATUS, icqSetStatus);
	CreateProtoServiceFunction(protoName, PS_GETSTATUS, icqGetStatus);
	CreateProtoServiceFunction(protoName, PS_SETAWAYMSG, icqSetAwayMsg);
	CreateProtoServiceFunction(protoName, PS_AUTHALLOW, icqAuthAllow);
	CreateProtoServiceFunction(protoName, PS_AUTHDENY, icqAuthDeny);
	CreateProtoServiceFunction(protoName, PS_BASICSEARCH, icqBasicSearch);
	CreateProtoServiceFunction(protoName, PS_SEARCHBYEMAIL, icqSearchByEmail);
	CreateProtoServiceFunction(protoName, PS_SEARCHBYNAME, icqSearchByName);
	CreateProtoServiceFunction(protoName, PS_ADDTOLIST, icqAddToList);
	CreateProtoServiceFunction(protoName, PS_FILERESUME, icqFileResume);

	CreateProtoServiceFunction(protoName, PS_GETINFO, icqGetInfo);
	CreateProtoServiceFunction(protoName, PSS_MESSAGE, icqSendMessage);
	CreateProtoServiceFunction(protoName, PS_GETAWAYMSG, icqGetAwayMsg);
	CreateProtoServiceFunction(protoName, PSS_FILE, icqSendFile);
	CreateProtoServiceFunction(protoName, PS_FILEALLOW, icqFileAllow);
	CreateProtoServiceFunction(protoName, PS_FILEDENY, icqFileDeny);
	CreateProtoServiceFunction(protoName, PS_FILECANCEL, icqFileCancel);

	CreateProtoServiceFunction(protoName, PSR_MESSAGE, icqRecvMessage);
	CreateProtoServiceFunction(protoName, PSR_AWAYMSG, icqRecvAwayMsg);
	CreateProtoServiceFunction(protoName, PSR_FILE, icqRecvFile);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

int UnloadServices()
{
	icq.unload();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
