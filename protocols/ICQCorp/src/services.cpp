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

#include "corp.h"

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqGetCaps(WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
    case PFLAGNUM_1:
        return PF1_IM|PF1_URL|PF1_FILE|PF1_MODEMSG|PF1_AUTHREQ|PF1_ADDED|PF1_PEER2PEER|PF1_BASICSEARCH|PF1_EXTSEARCH|PF1_CANRENAMEFILE|PF1_FILERESUME|PF1_ADDSEARCHRES|PF1_SEARCHBYEMAIL|PF1_SEARCHBYNAME|PF1_NUMERICUSERID;

    case PFLAGNUM_2:
        return PF2_ONLINE|PF2_INVISIBLE|PF2_SHORTAWAY|PF2_LONGAWAY|PF2_LIGHTDND|PF2_HEAVYDND|PF2_FREECHAT;

    case PFLAGNUM_3:
        return PF2_SHORTAWAY|PF2_LONGAWAY|PF2_LIGHTDND|PF2_HEAVYDND|PF2_FREECHAT;

    case PFLAG_UNIQUEIDTEXT:
        return (int)Translate("ICQ number:");

	case PFLAG_UNIQUEIDSETTING:
		return (int)"UIN";
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqGetName(WPARAM wParam, LPARAM lParam)
{
    strncpy((char*)lParam, Translate(protoName), wParam);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqLoadIcon(WPARAM wParam, LPARAM lParam)
{
    unsigned int id;

    switch (wParam & 0xFFFF) 
    {
        case PLI_PROTOCOL: id = IDI_ICQCORP; break;
        default: return (int)(HICON)NULL;
    }
    return (int)LoadImage(hInstance, MAKEINTRESOURCE(id), IMAGE_ICON, GetSystemMetrics(wParam & PLIF_SMALL ? SM_CXSMICON : SM_CXICON), GetSystemMetrics(wParam & PLIF_SMALL ? SM_CYSMICON : SM_CYICON), 0);
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqSetStatus(WPARAM wParam, LPARAM lParam)
{
    unsigned short desiredStatus = (unsigned short)wParam;

    T("[   ] set status\n");

    // on change status to online set away msg not calling
    if (desiredStatus == ID_STATUS_ONLINE) icq.awayMessage[0] = 0;

    if (icq.desiredStatus == desiredStatus) return 0;
    if (desiredStatus == ID_STATUS_OFFLINE)
    {
        icq.desiredStatus = desiredStatus;
        icq.logoff(false);
    }
    else
    {
        if (icq.statusVal == ID_STATUS_OFFLINE) icq.logon(desiredStatus);
        else icq.setStatus(desiredStatus);
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqGetStatus(WPARAM wParam, LPARAM lParam)
{
    return icq.statusVal;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqAuthAllow(WPARAM wParam, LPARAM lParam)
{
//    DBEVENTINFO dbei;
    //DWORD uin;
/*
    if (!icqOnline) return 1;
    ZeroMemory(&dbei, sizeof(dbei));
    dbei.cbSize = sizeof(dbei);
    dbei.cbBlob = sizeof(DWORD);
    dbei.pBlob = (PBYTE)&uin;
    if (CallService(MS_DB_EVENT_GET, wParam, (LPARAM)&dbei)) return 1;
    if (dbei.eventType != EVENTTYPE_AUTHREQUEST) return 1;
    if (strcmp(dbei.szModule, ICQOSCPROTONAME)) return 1;
    if (uin <= 1) return 1;
    icq_sendAuthGrantedServ(uin);
*/    
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqAuthDeny(WPARAM wParam, LPARAM lParam)
{
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqBasicSearch(WPARAM wParam, LPARAM lParam)
{
    T("[   ] basic search\n");
    icq.startSearch(0, 0, (char*)lParam, 0);
    return 1;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqSearchByEmail(WPARAM wParam, LPARAM lParam)
{
    T("[   ] search by e-mail\n");
    icq.startSearch(4, 0, (char*)lParam, 0);
    return 1;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqSearchByName(WPARAM wParam, LPARAM lParam)
{
    PROTOSEARCHBYNAME *psbn = (PROTOSEARCHBYNAME*)lParam;

    T("[   ] search by name\n");
    icq.startSearch(1, 0, psbn->pszNick, 0);
    return 1;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqAddToList(WPARAM wParam, LPARAM lParam)
{
    ICQSEARCHRESULT *isr = (ICQSEARCHRESULT *)lParam;
    bool persistent = (wParam & PALF_TEMPORARY) == 0;

    T("[   ] add user to list\n");
    if (isr->hdr.cbSize != sizeof(ICQSEARCHRESULT) || isr->uin == icq.uin) return NULL;
    return (int)icq.addUser(isr->uin, persistent)->hContact;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqGetInfo(WPARAM wParam, LPARAM lParam)
{
    CCSDATA *ccs = (CCSDATA *)lParam;
    ICQUser *u;

    T("[   ] get user info\n");

    u = icq.getUserByContact(ccs->hContact);
    if (u == NULL || icq.statusVal <= ID_STATUS_OFFLINE) return 1;

    icq.getUserInfo(u, ccs->wParam & SGIF_MINIMAL);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqSendMessage(WPARAM wParam, LPARAM lParam)
{
    CCSDATA *ccs = (CCSDATA *)lParam;
    ICQUser *u;
    ICQEvent *icqEvent;

    T("[   ] send message\n");

    u = icq.getUserByContact(ccs->hContact);
//  uin = db_get_dw(ccs->hContact, ICQCORP_PROTONAME, "UIN", 0);
    if (u == NULL || icq.statusVal <= ID_STATUS_OFFLINE) return 0;

    icqEvent = icq.sendMessage(u, (char*)ccs->lParam);
    return icqEvent ? icqEvent->sequence : 0;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqRecvMessage(WPARAM wParam, LPARAM lParam)
{
    DBEVENTINFO dbei;
    CCSDATA *ccs = (CCSDATA*)lParam;
    PROTORECVEVENT *pre = (PROTORECVEVENT*)ccs->lParam;

    T("[   ] recieve message\n");

    db_unset(ccs->hContact, "CList", "Hidden");
    ZeroMemory(&dbei, sizeof(dbei));
    dbei.cbSize = sizeof(dbei);
    dbei.szModule = protoName;
    dbei.timestamp = pre->timestamp;
    dbei.flags = pre->flags & (PREF_CREATEREAD ? DBEF_READ : 0);
    dbei.eventType = EVENTTYPE_MESSAGE;
    dbei.cbBlob = lstrlen(pre->szMessage) + 1;
    dbei.pBlob = (PBYTE)pre->szMessage;
	db_event_add(ccs->hContact, &dbei);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqSendUrl(WPARAM wParam, LPARAM lParam)
{
    CCSDATA *ccs = (CCSDATA *)lParam;
    ICQUser *u;
    ICQEvent *icqEvent;

    T("[   ] send url\n");

    u = icq.getUserByContact(ccs->hContact);
//  uin = db_get_dw(ccs->hContact, ICQCORP_PROTONAME, "UIN", 0);
    if (u == NULL || icq.statusVal <= ID_STATUS_OFFLINE) return 0;

    icqEvent = icq.sendUrl(u, (char*)ccs->lParam);
    return icqEvent ? icqEvent->sequence : 0;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqRecvUrl(WPARAM wParam, LPARAM lParam)
{
    DBEVENTINFO dbei;
    CCSDATA *ccs = (CCSDATA*)lParam;
    PROTORECVEVENT *pre = (PROTORECVEVENT*)ccs->lParam;
    char *pszDescr;

    T("[   ] recieve url\n");

    db_unset(ccs->hContact, "CList", "Hidden");
    pszDescr = pre->szMessage + lstrlen(pre->szMessage) + 1;
    ZeroMemory(&dbei, sizeof(dbei));
    dbei.cbSize = sizeof(dbei);
    dbei.szModule = protoName;
    dbei.timestamp = pre->timestamp;
    dbei.flags = pre->flags & (PREF_CREATEREAD ? DBEF_READ : 0);
    dbei.eventType = EVENTTYPE_URL;
    dbei.cbBlob = lstrlen(pre->szMessage) + lstrlen(pszDescr) + 2;
    dbei.pBlob = (PBYTE)pre->szMessage;
	db_event_add(ccs->hContact, &dbei);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqSetAwayMsg(WPARAM wParam, LPARAM lParam)
{
    T("[   ] set away msg\n");

    if (lParam == NULL) return 0;

    if (icq.awayMessage) delete [] icq.awayMessage;
    icq.awayMessage = new char[strlen((char*)lParam) + 1];
    strcpy(icq.awayMessage, (char*)lParam);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqGetAwayMsg(WPARAM wParam, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA *)lParam;
    ICQUser *u;
    ICQEvent *icqEvent;

    T("[   ] send get away msg\n");

    u = icq.getUserByContact(ccs->hContact);
    if (u == NULL || u->statusVal <= ID_STATUS_ONLINE) return 0;

    icqEvent = icq.sendReadAwayMsg(u);
    return icqEvent ? icqEvent->sequence : 0;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqRecvAwayMsg(WPARAM wParam, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA *)lParam;
	PROTORECVEVENT *pre = (PROTORECVEVENT *)ccs->lParam;

    T("[   ] recieve away message\n");

	ProtoBroadcastAck(protoName, ccs->hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)pre->lParam, (LPARAM)pre->szMessage);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqSendFile(WPARAM wParam, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA *)lParam;
	char **files = (char **)ccs->lParam;
    //HANDLE hFile;
    ICQUser *u;

    T("[   ] send file\n");

    u = icq.getUserByContact(ccs->hContact);
    if (u == NULL || u->statusVal == ID_STATUS_OFFLINE || icq.statusVal <= ID_STATUS_OFFLINE) return 0;

    unsigned long filesCount, directoriesCount, filesSize = 0;
    char filename[MAX_PATH], format[32];
    WIN32_FIND_DATA findData;

	for (filesCount=0,directoriesCount=0; files[filesCount]; filesCount++)
    {
        FindClose(FindFirstFile(files[filesCount], &findData));
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) directoriesCount++;
        else filesSize += findData.nFileSizeLow;
    }
    filesCount -= directoriesCount;

    if (directoriesCount)
    {
        sprintf(format, "%s, %s", filesCount == 1 ? Translate("%d file") : Translate("%d files"), directoriesCount == 1 ? Translate("%d directory") : Translate("%d directories"));
        sprintf(filename, format, filesCount, directoriesCount);
    }
    else
    {
        if (filesCount == 1)
        {
            char *p = strrchr(files[0], '\\');
            strcpy(filename, p ? p+1 : files[0]);
        }
        else sprintf(filename, filesCount == 1 ? Translate("%d file") : Translate("%d files"), filesCount);
    }

    return (int)icq.sendFile(u, (char*)ccs->wParam, filename, filesSize, files);
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqFileAllow(WPARAM wParam, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA *)lParam;
    ICQTransfer *t = (ICQTransfer *)ccs->wParam;
    ICQUser *u;

    T("[   ] send accept file request\n");

    u = icq.getUserByContact(ccs->hContact);
    if (u == NULL || u->statusVal == ID_STATUS_OFFLINE) return 0;

    t->path = _strdup((char*)ccs->lParam);

	icq.acceptFile(u, t->sequence, (char*)ccs->lParam);
    return (int)t;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqFileDeny(WPARAM wParam, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA *)lParam;
    ICQTransfer *t = (ICQTransfer *)ccs->wParam;
    ICQUser *u;

    T("[   ] send refuse file request\n");

    u = icq.getUserByContact(ccs->hContact);
    if (u == NULL || u->statusVal == ID_STATUS_OFFLINE) return 0;

	icq.refuseFile(u, t->sequence, (char*)ccs->lParam);

    unsigned int i;
    for (i=0; i<icqTransfers.size(); i++)
    {
        if (icqTransfers[i] == t)
        {
            delete icqTransfers[i];
            icqTransfers[i] = icqTransfers[icqTransfers.size() - 1];
            icqTransfers.pop_back();
            break;
        }
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqFileCancel(WPARAM wParam, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA *)lParam;
    ICQTransfer *t = (ICQTransfer *)ccs->wParam;
    ICQUser *u;

    T("[   ] file cancel\n");

    u = icq.getUserByContact(ccs->hContact);
    if (u == NULL || u->statusVal == ID_STATUS_OFFLINE) return 0;

//	icq.refuseFile(u, t->sequence, (char*)ccs->lParam);

    unsigned int i;
    for (i=0; i<icqTransfers.size(); i++)
    {
        if (icqTransfers[i] == t)
        {
            delete icqTransfers[i];
            icqTransfers[i] = icqTransfers[icqTransfers.size() - 1];
            icqTransfers.pop_back();
            break;
        }
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqRecvFile(WPARAM wParam, LPARAM lParam)
{
	DBEVENTINFO dbei;
	CCSDATA *ccs = (CCSDATA *)lParam;
	PROTORECVEVENT *pre = (PROTORECVEVENT *)ccs->lParam;
	char *szDesc, *szFile;

    T("[   ] recieve file\n");

	db_unset(ccs->hContact, "CList", "Hidden");

	szFile = pre->szMessage + sizeof(DWORD);
	szDesc = szFile + strlen(szFile) + 1;

	ZeroMemory(&dbei, sizeof(dbei));
	dbei.cbSize = sizeof(dbei);
	dbei.szModule = protoName;
	dbei.timestamp = pre->timestamp;
	dbei.flags = pre->flags & (PREF_CREATEREAD ? DBEF_READ : 0);
	dbei.eventType = EVENTTYPE_FILE;
	dbei.cbBlob = sizeof(DWORD)+(DWORD)strlen(szFile) + (DWORD)strlen(szDesc) + 2;
	dbei.pBlob = (PBYTE)pre->szMessage;
	db_event_add(ccs->hContact, &dbei);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqFileResume(WPARAM wParam, LPARAM lParam)
{
	PROTOFILERESUME *pfr = (PROTOFILERESUME*)lParam;
    ICQTransfer *t = (ICQTransfer *)wParam;

    T("[   ] send file resume\n");
    t->resume(pfr->action, pfr->szFilename);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR icqSetApparentMode(WPARAM wParam, LPARAM lParam)
{
/*
	CCSDATA *ccs = (CCSDATA *)lParam;
	int oldMode, newMode = ccs->wParam;
    ICQUser *u;

    u = icq.getUserByContact(ccs->hContact);
    if (u == NULL) return 1;

    oldMode = db_get_w(u->hContact, ICQCORP_PROTONAME, "ApparentMode", 0);
    if (newMode == oldMode) return 1;

    T("[   ] set apparent mode\n");

    if (newMode == ID_STATUS_ONLINE || newMode == ID_STATUS_OFFLINE) db_set_w(u->hContact, ICQCORP_PROTONAME, "ApparentMode", (WORD)newMode);
    else db_unset(u->hContact, ICQCORP_PROTONAME, "ApparentMode");

	if (icq.statusVal <= ID_STATUS_OFFLINE) return 0;

    if (oldMode != 0) icq.updateUserList(u, oldMode == ID_STATUS_OFFLINE ? 1 : 2, 0);
    if (newMode != 0) icq.updateUserList(u, newMode == ID_STATUS_OFFLINE ? 1 : 2, 1);
*/    
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

static int icqContactDeleted(WPARAM wParam, LPARAM lParam)
{
    ICQUser *u;

    T("[   ] contact deleted\n");

	if ((u = icq.getUserByContact((MCONTACT)wParam)) != NULL) icq.removeUser(u);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static int icqModulesLoaded(WPARAM wParam, LPARAM lParam)
{
    HookEvent(ME_USERINFO_INITIALISE, icqUserInfoInitialise);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

char *CreateServiceName(char *serviceName, char *functionName)
{
    strcpy(serviceName, protoName);
    strcat(serviceName, functionName);

    return serviceName;
}

///////////////////////////////////////////////////////////////////////////////

int LoadServices()
{
    char serviceName[MAX_PATH + 30];

    if (!icq.load()) return 1;

	HookEvent(ME_DB_CONTACT_DELETED, icqContactDeleted);
   	HookEvent(ME_OPT_INITIALISE, icqOptionsInitialise);
    HookEvent(ME_SYSTEM_MODULESLOADED, icqModulesLoaded);

    CreateServiceFunction(CreateServiceName(serviceName, PS_GETCAPS), icqGetCaps);
    CreateServiceFunction(CreateServiceName(serviceName, PS_GETNAME), icqGetName);
    CreateServiceFunction(CreateServiceName(serviceName, PS_LOADICON), icqLoadIcon);
    CreateServiceFunction(CreateServiceName(serviceName, PS_SETSTATUS), icqSetStatus);
    CreateServiceFunction(CreateServiceName(serviceName, PS_GETSTATUS), icqGetStatus);
    CreateServiceFunction(CreateServiceName(serviceName, PS_SETAWAYMSG), icqSetAwayMsg);
    CreateServiceFunction(CreateServiceName(serviceName, PS_AUTHALLOW), icqAuthAllow);
    CreateServiceFunction(CreateServiceName(serviceName, PS_AUTHDENY), icqAuthDeny);
    CreateServiceFunction(CreateServiceName(serviceName, PS_BASICSEARCH), icqBasicSearch);
    CreateServiceFunction(CreateServiceName(serviceName, PS_SEARCHBYEMAIL), icqSearchByEmail);
    CreateServiceFunction(CreateServiceName(serviceName, PS_SEARCHBYNAME), icqSearchByName);
//    CreateServiceFunction(MS_ICQ_SEARCHBYDETAILS, icqSearchByDetails);
    CreateServiceFunction(CreateServiceName(serviceName, PS_ADDTOLIST), icqAddToList);
//    CreateServiceFunction(CreateServiceName(serviceName, PS_ADDTOLISTBYEVENT), icqAddToListByEvent);
    CreateServiceFunction(CreateServiceName(serviceName, PS_FILERESUME), icqFileResume);

    CreateServiceFunction(CreateServiceName(serviceName, PSS_GETINFO), icqGetInfo);
    CreateServiceFunction(CreateServiceName(serviceName, PSS_MESSAGE), icqSendMessage);
    CreateServiceFunction(CreateServiceName(serviceName, PSS_URL), icqSendUrl);
    CreateServiceFunction(CreateServiceName(serviceName, PSS_GETAWAYMSG), icqGetAwayMsg);
    CreateServiceFunction(CreateServiceName(serviceName, PSS_FILE), icqSendFile);
    CreateServiceFunction(CreateServiceName(serviceName, PSS_FILEALLOW), icqFileAllow);
    CreateServiceFunction(CreateServiceName(serviceName, PSS_FILEDENY), icqFileDeny);
    CreateServiceFunction(CreateServiceName(serviceName, PSS_FILECANCEL), icqFileCancel);
    CreateServiceFunction(CreateServiceName(serviceName, PSS_SETAPPARENTMODE), icqSetApparentMode);

    CreateServiceFunction(CreateServiceName(serviceName, PSR_MESSAGE), icqRecvMessage);
    CreateServiceFunction(CreateServiceName(serviceName, PSR_URL), icqRecvUrl);
	CreateServiceFunction(CreateServiceName(serviceName, PSR_AWAYMSG), icqRecvAwayMsg);
	CreateServiceFunction(CreateServiceName(serviceName, PSR_FILE), icqRecvFile);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int UnloadServices()
{
    icq.unload();
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
