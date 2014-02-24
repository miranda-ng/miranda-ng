/*

'File Association Manager'-Plugin for Miranda IM

Copyright (C) 2005-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (AssocMgr-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

// -- Includes
#include "common.h"

/*
#define AIM_SUPPORT_TEST
#define ICQ_SUPPORT_TEST
#define YAHOO_SUPPORT_TEST
#define MSN_SUPPORT_TEST
#define GG_SUPPORT_TEST
#define JABBER_SUPPORT_TEST
*/

// -----------------------------------------

extern HINSTANCE hInst;

/* a copy of those in m_assocmgr.h, as we did not #include those helpers */
__inline static int AssocMgr_AddNewFileTypeT(const char *ext,const char *mime,const TCHAR *desc,const TCHAR *verb,HINSTANCE hinst,UINT iconid,const char *service,DWORD flags)
{
	FILETYPEDESC ftd;
	ftd.cbSize=sizeof(FILETYPEDESC);
	ftd.pszFileExt=ext;
	ftd.pszMimeType=mime;
	ftd.ptszDescription=desc;
	ftd.hInstance=hinst;
	ftd.nIconResID=iconid;
	ftd.ptszVerbDesc=verb;
	ftd.pszService=service;
	ftd.flags=flags|FTDF_TCHAR;
	return CallService(MS_ASSOCMGR_ADDNEWFILETYPE,0,(LPARAM)&ftd);
}

static int __inline AssocMgr_AddNewUrlType(const char *prefix,const char *desc,HINSTANCE hinst,UINT iconid,const char *service,DWORD flags)
{
	URLTYPEDESC utd;
	utd.cbSize=sizeof(URLTYPEDESC);
	utd.pszProtoPrefix=prefix;
	utd.pszDescription=desc;
	utd.hInstance=hinst;
	utd.nIconResID=iconid;
	utd.pszService=service;
	utd.flags=flags&~UTDF_UNICODE;
	return CallService(MS_ASSOCMGR_ADDNEWURLTYPE,0,(LPARAM)&utd);
}

// -----------------------------------------

#ifdef AIM_SUPPORT_TEST

#define AIM_PROTOCOL_NAME  "AIM"
#define IDI_AOL  28
#define MOD_KEY_CL  "CList"
#define AIM_KEY_NL  "NotOnList"
struct oscar_data {	HINSTANCE hInstance; } static conn;
static __inline HANDLE find_contact(const char *nick) { nick; return NULL; }
static __inline HANDLE add_contact(const char *nick) { nick; return db_find_first(); }
static __inline void aim_gchat_joinrequest(const char *room, int exchange) { room; exchange; MessageBoxA(NULL,"Join group chat!",room,MB_OK); return; }
#include <m_protosvc.h>
#include <m_message.h>

#include "m_assocmgr.h"
static HANDLE hHookModulesLoaded;
static HANDLE hServiceParseLink;

static int ServiceParseAimLink(WPARAM,LPARAM lParam)
{
    char *arg=(char*)lParam;
    if(arg==NULL) return 1; /* sanity check */
	/* skip leading prefix */
	arg=strchr(arg,':');
	if(arg==NULL) return 1; /* parse failed */
	for (++arg;*arg=='/';++arg);
	/*
		add user:      aim:addbuddy?screenname=NICK&groupname=GROUP
		send message:  aim:goim?screenname=NICK&message=MSG
		open chatroom: aim:gochat?roomname=ROOM&exchange=NUM
	*/
    /* add a contact to the list */
    if (!_strnicmp(arg,"addbuddy?",9)) {
        char *tok,*sn=NULL,*group=NULL;
        ADDCONTACTSTRUCT acs;
        PROTOSEARCHRESULT psr;
		if (*(arg+=9)==0) return 1; /* parse failed */
        tok=strtok(arg,"&"); /* first token */
        while(tok!=NULL) {
            if (!_strnicmp(tok,"screenname=",11) && *(tok+11)!=0)
                sn=Netlib_UrlDecode(tok+11);
            if (!_strnicmp(tok,"groupname=",10) && *(tok+10)!=0)
                group=Netlib_UrlDecode(tok+10);  /* group is currently ignored */
            tok=strtok(NULL,"&"); /* next token */
        }
        if(sn==NULL) return 1; /* parse failed */
        if(find_contact(sn)==NULL) { /* does not yet check if sn is current user */
            acs.handleType=HANDLE_SEARCHRESULT;
            acs.szProto=AIM_PROTOCOL_NAME;
            acs.psr=&psr;
            ZeroMemory(&psr,sizeof(PROTOSEARCHRESULT));
            psr.cbSize=sizeof(PROTOSEARCHRESULT);
            psr.nick=sn;
            CallService(MS_ADDCONTACT_SHOW,0,(LPARAM)&acs);
        }
		return 0;
    }
    /* send a message to a contact */
    else if (!_strnicmp(arg,"goim?",5)) {
        char *tok,*sn=NULL,*msg=NULL;
        MCONTACT hContact;
        if (*(arg+=5)==0) return 1; /* parse failed */
        tok=strtok(arg,"&"); /* first token */
        while(tok!=NULL) {
            if (!_strnicmp(tok,"screenname=",11) && *(tok+11)!=0)
                sn=Netlib_UrlDecode(tok+11);
            if (!_strnicmp(tok,"message=",8) && *(tok+8)!=0)
                msg=Netlib_UrlDecode(tok+8);
            tok=strtok(NULL,"&"); /* next token */
        }
        if(sn==NULL) return 1; /* parse failed */
		if(ServiceExists(MS_MSG_SENDMESSAGE)) {
            hContact=find_contact(sn);
            if(hContact==NULL) {
                hContact=add_contact(sn); /* does not yet check if sn is current user */
				if(hContact!=NULL)
	                db_set_b(hContact,MOD_KEY_CL,AIM_KEY_NL,1);
            }
            if(hContact!=NULL)
                CallService(MS_MSG_SENDMESSAGE,hContact,(LPARAM)msg);
		}
		return 0;
    }
    /* open a chatroom */
    else if (!_strnicmp(arg,"gochat?",7)) {
        char *tok,*rm=NULL;
        int exchange=0;
        if (*(arg+=7)==0) return 1; /* parse failed */
        tok=strtok(arg,"&"); /* first token */
        while(tok!=NULL) {
            if (!_strnicmp(tok,"roomname=",9) && *(tok+9)!=0)
                rm=Netlib_UrlDecode(tok+9);
            if (!_strnicmp(tok,"exchange=",9))
                exchange=atoi(Netlib_UrlDecode(tok+9)); 
            tok=strtok(NULL,"&"); /* next token */
        }
        if(rm==NULL || exchange<=0) return 1; /* parse failed */
        aim_gchat_joinrequest(rm,exchange);
		return 0;
    }
    return 1; /* parse failed */
}

static int AimLinksModulesLoaded(WPARAM wParam,LPARAM lParam)
{
    char service_name[MAXMODULELABELLENGTH];
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
    mir_snprintf(service_name,sizeof(service_name),"%s%s",AIM_PROTOCOL_NAME,"ParseAimLink");
	/* or "AOL Instant Messenger Links" */
    AssocMgr_AddNewUrlType("aim:",Translate("AIM Link Protocol"),conn.hInstance,IDI_AOL,service_name,0);
    return 0;
}

void aim_links_init()
{
    char service_name[MAXMODULELABELLENGTH];
    //LOG(LOG_DEBUG,"Links: init");
    mir_snprintf(service_name,sizeof(service_name),"%s%s",AIM_PROTOCOL_NAME,"ParseAimLink");
    hServiceParseLink=CreateServiceFunction(service_name,ServiceParseAimLink);
    hHookModulesLoaded=HookEvent(ME_SYSTEM_MODULESLOADED,AimLinksModulesLoaded);
}

void aim_links_destroy()
{
    //LOG(LOG_DEBUG,"Links: destroy");
    UnhookEvent(hHookModulesLoaded);
    DestroyServiceFunction(hServiceParseLink);
}
#endif

// -----------------------------------------

#ifdef ICQ_SUPPORT_TEST

#define gpszICQProtoName  "ICQ"
#define IDI_ICQ  101
#define hInst  GetModuleHandleA("ICQ")
static __inline HANDLE HContactFromUIN(DWORD dwUIN,int *Added) { dwUIN; Added; return db_find_first(); }
static __inline HANDLE ICQFindFirstContact(void) { return NULL; }
static __inline HANDLE ICQFindNextContact(MCONTACT hContact) { hContact; return NULL; }
static __inline void AddToCache(MCONTACT hContact,DWORD dwUin) { hContact; dwUin; }
static __inline DWORD ICQGetContactSettingUIN(MCONTACT hContact) { hContact; return 0; }
static __inline HANDLE HandleFromCacheByUin(DWORD dwUin) { dwUin; return NULL; }
#include <m_protosvc.h>
#include <m_message.h>
typedef struct {
  PROTOSEARCHRESULT hdr;
  DWORD uin;
  BYTE auth;
  char* uid;
} ICQSEARCHRESULT;

#include "m_assocmgr.h"
static HANDLE hHookModulesLoaded;
static HANDLE hServiceOpenFile;

static void TrimString(char *str)
{
	int len,start;
	len=lstrlenA(str);
	while(str[0]!='\0' && (unsigned char)str[len-1]<=' ') str[--len]=0;
	for(start=0;str[start] && (unsigned char)str[start]<=' ';++start);
	MoveMemory(str,str+start,len-start+1);
}

static BOOL IsEmpty(const char *str)
{
	int i;
	for(i=0;str[i]!='\0';i++)
		if(str[i]!=' ' && str[i]!='\r' && str[i]!='\n')
			return FALSE;
	return TRUE;
}

#define ICQFILE_MESSAGEUSER  1
#define ICQFILE_ADDUSER      2
typedef struct {
	int type;
	char uin[64];
	char email[128];
	char nick[128];
	char firstName[64];
	char lastName[64];
} ICQFILEINFO;

static HANDLE IsHContactFromUIN(DWORD uin)
{
	MCONTACT hContact;
	DWORD dwUin;
	hContact=HandleFromCacheByUin(uin);
	if(hContact!=NULL) return hContact;
	hContact=ICQFindFirstContact();
	while(hContact!=NULL) {
		dwUin=ICQGetContactSettingUIN(hContact);
		if(dwUin==uin) {
			AddToCache(hContact,dwUin);
			return hContact;
		}
		hContact=ICQFindNextContact(hContact);
	}
	return NULL;
}

static void AddIcqUser(ICQFILEINFO *info)
{
	ADDCONTACTSTRUCT acs;
	ICQSEARCHRESULT psr;
	DWORD uin;
	/* check that uin does not belong to current user */
	uin=atoi(info->uin);
	if(uin!=0 && uin!=ICQGetContactSettingUIN(NULL)) {
		/* not yet present? */
		if(IsHContactFromUIN(uin)==NULL) {
			acs.handleType=HANDLE_SEARCHRESULT;
			acs.szProto=gpszICQProtoName;
			acs.psr=(PROTOSEARCHRESULT*)&psr;
			ZeroMemory(&psr,sizeof(ICQSEARCHRESULT));
			psr.hdr.cbSize=sizeof(ICQSEARCHRESULT);
			if(info->nick[0]) psr.hdr.nick=info->nick;
			else psr.hdr.nick=info->uin;
			if(info->email[0]) psr.hdr.email=info->email;
			if(info->firstName[0]) psr.hdr.firstName=info->firstName;
			if(info->lastName[0]) psr.hdr.lastName=info->lastName;
			psr.uin=uin;
			psr.auth=1; /* authentication needed flag */
			psr.uid=NULL; /* icq contact */
			CallService(MS_ADDCONTACT_SHOW,0,(LPARAM)&acs);
		}
	}
}

static void MessageIcqUser(ICQFILEINFO *info)
{
	MCONTACT hContact;
	if(ServiceExists(MS_MSG_SENDMESSAGE)) {
		hContact=HContactFromUIN(atoi(info->uin),NULL); /* adds the contact if needed */
		if(hContact!=NULL)
			CallService(MS_MSG_SENDMESSAGE,hContact,0);
	}
}

static int IcqOpenFile(WPARAM wParam,LPARAM lParam)
{
    TCHAR *pszFile=(TCHAR*)lParam; /* TCHAR is specified on AssocMgr_AddNewFileTypeT() */
	FILE *fp;
	ICQFILEINFO info;
	char line[4096],*sep;
	UNREFERENCED_PARAMETER(wParam);
	/*
		send message:
		[ICQ Message User]
		UIN=1234567

		add user:
		(all the .uin files I've seen only have UIN= in them)
		[ICQ User]
		UIN=1234567
		Email=
		NickName=
		FirstName=
		LastName=
	*/
	if(pszFile==NULL) return 1; /* sanity check */
	fp=_tfopen(pszFile,_T("rt"));
	if(fp==NULL) return 1; /* open failed */
	info.type=0;
	while(!feof(fp)) {
		if(fgets(line,sizeof(line),fp)==NULL) break;
		TrimString(line);
		if(IsEmpty(line)) continue;
		if(line[0]=='[') {
			ZeroMemory(&info,sizeof(info));
			if (!lstrcmpiA(line,"[ICQ Message User]"))
				info.type=ICQFILE_MESSAGEUSER;
			else if (!lstrcmpiA(line,"[ICQ User]"))
				info.type=ICQFILE_ADDUSER;
			continue;
		}
		if(info.type==0) continue;
		sep=strchr(line,'=');
		if(sep==NULL) { info.type=0; break; } /* format error */
		*(sep++)='\0';
		if (!lstrcmpA("UIN",line)) lstrcpynA(info.uin,sep,sizeof(info.uin)); /* buffer safe */
		else if (!lstrcmpA("Email",line)) lstrcpynA(info.email,sep,sizeof(info.email)); /* buffer safe */
		else if (!lstrcmpA("NickName",line)) lstrcpynA(info.nick,sep,sizeof(info.nick)); /* buffer safe */
		else if (!lstrcmpA("FirstName",line)) lstrcpynA(info.firstName,sep,sizeof(info.firstName)); /* buffer safe */
		else if (!lstrcmpA("LastName",line)) lstrcpynA(info.lastName,sep,sizeof(info.lastName)); /* buffer safe */
	}
	fclose(fp);
	switch(info.type) {
		case ICQFILE_MESSAGEUSER: MessageIcqUser(&info); return 0;
		case ICQFILE_ADDUSER:     AddIcqUser(&info); return 0;
		default: return 1; /* open failed */
	}
}

static int IcqFilesModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	char szService[MAX_PATH+32];
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	strcat(strcpy(szService,gpszICQProtoName),"OpenFile");
	/* .icq files are not used, just by the ProtoLink plugin */
    //AssocMgr_AddNewFileTypeT(".icq","application/x-icq",TranslateT("ICQ Link Shortcut"),TranslateT("&Add to contact list..."),hInst,IDI_ICQ,szService,FTDF_BROWSERAUTOOPEN|FTDF_ISTEXT|FTDF_ISSHORTCUT|FTDF_DEFAULTDISABLED);
    AssocMgr_AddNewFileTypeT(".uin","application/x-icq",TranslateT("ICQ Link Shortcut"),TranslateT("&Add to contact list..."),hInst,IDI_ICQ,szService,FTDF_BROWSERAUTOOPEN|FTDF_ISTEXT|FTDF_ISSHORTCUT);
    return 0;
}

void InitIcqFiles(void)
{
	char szService[MAX_PATH+32];
	strcat(strcpy(szService,gpszICQProtoName),"OpenFile");
    hServiceOpenFile=CreateServiceFunction(szService,IcqOpenFile);
    hHookModulesLoaded=HookEvent(ME_SYSTEM_MODULESLOADED,IcqFilesModulesLoaded);
}

void UninitIcqFiles(void)
{
    UnhookEvent(hHookModulesLoaded);
    DestroyServiceFunction(hServiceOpenFile);
}

#undef hInst
#endif

// -----------------------------------------

#ifdef YAHOO_SUPPORT_TEST

#define yahooProtocolName "YAHOO"
#define IDI_YAHOO  10101
#define hinstance  GetModuleHandleA("YAHOO")
static __inline HANDLE getbuddyH(const char *yahoo_id) { yahoo_id; return NULL; }
static __inline HANDLE add_buddy(const char *yahoo_id,const char *yahoo_name,DWORD flags) { yahoo_id; yahoo_name; flags; return db_find_first(); }
#include <m_protosvc.h>
#include <m_message.h>

#include "m_assocmgr.h"
static HANDLE hHookModulesLoaded;
static HANDLE hServiceParseYmsgrLink;

static int ServiceParseYmsgrLink(WPARAM wParam,LPARAM lParam)
{
    char *arg=(char*)lParam;
	UNREFERENCED_PARAMETER(wParam);
    if(arg==NULL) return 1; /* sanity check */
	/* 
		add user:       ymsgr:addfriend?ID
		send message:   ymsgr:sendim?ID&m=MESSAGE
		add chatroom:   ymsgr:chat?ROOM
	*/
	/* skip leading prefix */
	arg=strchr(arg,':');
	if(arg==NULL) return 1; /* parse failed */
	for (++arg;*arg=='/';++arg);
    /* add a contact to the list */
    if (!_strnicmp(arg,"addfriend?",10)) {
        char *tok,*id=NULL;
        ADDCONTACTSTRUCT acs;
        PROTOSEARCHRESULT psr;
        if (*(arg+=10)==0) return 1; /* parse failed */
        tok=strtok(arg,"&"); /* first token */
        if(tok!=NULL) id=Netlib_UrlDecode(tok);
        if(id==NULL || *id==0) return 1; /* parse failed */
        if(getbuddyH(id)==NULL) { /* does not yet check if id is current user */
            acs.handleType=HANDLE_SEARCHRESULT;
            acs.szProto=yahooProtocolName;
            acs.psr=&psr;
            ZeroMemory(&psr,sizeof(PROTOSEARCHRESULT));
            psr.cbSize=sizeof(PROTOSEARCHRESULT);
            psr.nick=id;
            CallService(MS_ADDCONTACT_SHOW,0,(LPARAM)&acs);
        }
		return 0;
    }
    /* send a message to a contact */
    else if (!_strnicmp(arg,"sendim?",7)) {
        char *tok,*id=NULL,*msg=NULL;
        MCONTACT hContact;
        if (*(arg+=7)==0) return 1; /* parse failed */
        tok=strtok(arg,"&"); /* first token */
        if(tok!=NULL) id=tok;
		while(tok!=NULL) {
            if (!_strnicmp(tok,"m=",2) && *(tok+2)!=0)
                msg=Netlib_UrlDecode(tok+2);
            tok=strtok(NULL,"&"); /* next token */
        }
        if(id==NULL || *id==0) return 1; /* parse failed */
		if(ServiceExists(MS_MSG_SENDMESSAGE)) { /* does not yet check if sn is current user */
	        hContact=add_buddy(id,id,PALF_TEMPORARY); /* ensure contact is on list */
		    if(hContact!=NULL)
			    CallService(MS_MSG_SENDMESSAGE,hContact,(LPARAM)msg);
		}
		return 0;
	}
    /* open a chatroom */
    else if (!_strnicmp(arg,"chat?",5)) {
        char *tok,*rm=NULL;
        if (*(arg+=5)==0) return 1; /* parse failed */
        tok=strtok(arg,"&"); /* first token */
        if(tok!=NULL) rm=Netlib_UrlDecode(tok);
        if(rm==NULL) return 1; /* parse failed */
        /* not yet implemented (rm contains name of chatroom)*/
		return 0;
    }
    return 1; /* parse failed */
}

static int YmsgrLinksModulesLoaded(WPARAM wParam,LPARAM lParam)
{
    char szService[MAXMODULELABELLENGTH];
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
    mir_snprintf(szService,sizeof(szService),"%s%s",yahooProtocolName,"ParseYmsgrLink");
    AssocMgr_AddNewUrlType("ymsgr:",Translate("Yahoo Link Protocol"),hinstance,IDI_YAHOO,szService,0);
    return 0;
}

void YmsgrLinksInit(void)
{
    char szService[MAXMODULELABELLENGTH];
    mir_snprintf(szService,sizeof(szService),"%s%s",yahooProtocolName,"ParseYmsgrLink");
	hServiceParseYmsgrLink=CreateServiceFunction(szService,ServiceParseYmsgrLink);
    hHookModulesLoaded=HookEvent(ME_SYSTEM_MODULESLOADED,YmsgrLinksModulesLoaded);
}

void YmsgrLinksUninit(void)
{
    UnhookEvent(hHookModulesLoaded);
    DestroyServiceFunction(hServiceParseYmsgrLink);
}

#undef hinstance
#endif

// -----------------------------------------

#ifdef MSN_SUPPORT_TEST

#define msnProtocolName "MSN"
#define IDI_MSN  101
#define hInst  GetModuleHandleA("MSN")
static __inline HANDLE MSN_HContactFromEmailT(const char *msnEmail) { msnEmail; return NULL; }
static __inline HANDLE MSN_HContactFromEmail(const char *msnEmail,const char *msnNick,int addIfNeeded,int temporary) { msnEmail; msnNick; addIfNeeded; temporary; return db_find_first(); }
#include <m_protosvc.h>
#include <m_message.h>

#include "m_assocmgr.h"
static HANDLE hHookModulesLoaded;
static HANDLE hServiceParseMsnimLink;

static int ServiceParseMsnimLink(WPARAM wParam,LPARAM lParam)
{
    char *arg=(char*)lParam;
	UNREFERENCED_PARAMETER(wParam);
    if(arg==NULL) return 1; /* sanity check */
	/* 
		add user:      msnim:add?contact=netpassport@emailaddress.com
		send message:  msnim:chat?contact=netpassport@emailaddress.com
		voice chat:    msnim:voice?contact=netpassport@emailaddress.com
		video chat:    msnim:video?contact=netpassport@emailaddress.com
	*/
	/* skip leading prefix */
	arg=strchr(arg,':');
	if(arg==NULL) return 1; /* parse failed */
	for (++arg;*arg=='/';++arg);
    /* add a contact to the list */
    if (!_strnicmp(arg,"add?",4)) {
        char *tok,*email=NULL;
        ADDCONTACTSTRUCT acs;
        PROTOSEARCHRESULT psr;
        if (*(arg+=4)==0) return 1; /* parse failed */
        tok=strtok(arg,"&"); /* first token */
        while(tok!=NULL) {
            if (!_strnicmp(tok,"contact=",8) && *(tok+11)!=0)
                email=Netlib_UrlDecode(tok+11);
            tok=strtok(NULL,"&"); /* next token */
        }
        if(email==NULL || *email==0) return 1; /* parse failed */
        if(MSN_HContactFromEmailT(email)==NULL) { /* does not yet check if email is current user */
            acs.handleType=HANDLE_SEARCHRESULT;
            acs.szProto=AIM_PROTOCOL_NAME;
            acs.psr=&psr;
            ZeroMemory(&psr,sizeof(PROTOSEARCHRESULT));
            psr.cbSize=sizeof(PROTOSEARCHRESULT);
            psr.nick=email;
			psr.email=email;
            CallService(MS_ADDCONTACT_SHOW,0,(LPARAM)&acs);
        }
		return 0;
    }
    /* send a message to a contact */
	/* "voice" and "video" not yet implemented, perform same action as "chat" */
    else if (!_strnicmp(arg,"chat?",5) || !_strnicmp(arg,"voice?",6) || !_strnicmp(arg,"video?",6)) {
        char *tok,*email=NULL;
        MCONTACT hContact;
        if (*(arg+=5)==0) return 1; /* parse failed */
		if (*arg=='?' && *(++arg)==0) return 1; /* for "voice?" and "video?" */
        tok=strtok(arg,"&"); /* first token */
        while(tok!=NULL) {
            if (!_strnicmp(tok,"contact=",8) && *(tok+11)!=0)
                email=Netlib_UrlDecode(tok+11);
            tok=strtok(NULL,"&"); /* next token */
        }
        if(email==NULL || *email==0) return 1; /* parse failed */
		if(ServiceExists(MS_MSG_SENDMESSAGE)) {
            hContact=MSN_HContactFromEmail(email,email,TRUE,TRUE); /* does not yet check if email is current user */
            if(hContact!=NULL)
                CallService(MS_MSG_SENDMESSAGE,hContact,0);
		}
		return 0;
    }
    return 1; /* parse failed */
}

static int MsnLinksModulesLoaded(WPARAM wParam,LPARAM lParam)
{
    char szService[MAXMODULELABELLENGTH];
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
    mir_snprintf(szService,sizeof(szService),"%s%s",msnProtocolName,"ParseMsnimLink");
    AssocMgr_AddNewUrlType("msnim:",Translate("MSN Link Protocol"),hInst,IDI_MSN,szService,0);
    return 0;
}

int LoadMsnLinks(void)
{
    char szService[MAXMODULELABELLENGTH];
    mir_snprintf(szService,sizeof(szService),"%s%s",msnProtocolName,"ParseMsnimLink");
	hServiceParseMsnimLink=CreateServiceFunction(szService,ServiceParseMsnimLink);
    hHookModulesLoaded=HookEvent(ME_SYSTEM_MODULESLOADED,MsnLinksModulesLoaded);
	return 0;
}

void UnloadMsnLinks(void)
{
    UnhookEvent(hHookModulesLoaded);
    DestroyServiceFunction(hServiceParseMsnimLink);
}

#undef hInst
#endif

// -----------------------------------------

#ifdef GG_SUPPORT_TEST

#define GG_PROTO  "GG"
#define IDI_GG  251
#define hInstance  GetModuleHandleA("GG")
typedef DWORD uin_t;
static __inline HANDLE gg_getcontact(uin_t uin,int create,int inlist,char *szNick) { uin; create; inlist; szNick; return db_find_first(); }
#include <m_protosvc.h>
#include <m_message.h>

#include "m_assocmgr.h"
static HANDLE hHookModulesLoaded;
static HANDLE hServiceParseLink;

static int ServiceParseLink(WPARAM wParam,LPARAM lParam)
{
    char *arg=(char*)lParam;
	UNREFERENCED_PARAMETER(wParam);
    if(arg==NULL) return 1; /* sanity check */
	/* send message:  gg:UID */
	/* skip leading prefix */
	arg=strchr(arg,':');
	if(arg==NULL) return 1; /* parse failed */
	for (++arg;*arg=='/';++arg);
    /* send a message to a contact */
	{	MCONTACT hContact;
		if(ServiceExists(MS_MSG_SENDMESSAGE)) {
            hContact=gg_getcontact(atoi(arg),TRUE,FALSE,arg);
            if(hContact!=NULL)
                CallService(MS_MSG_SENDMESSAGE,hContact,0);
		}
    }
    return 0;
}

static int LinksModulesLoaded(WPARAM wParam,LPARAM lParam)
{
    char szService[MAXMODULELABELLENGTH];
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
    mir_snprintf(szService,sizeof(szService),"%s%s",GG_PROTO,"ParseMsnimLink");
    AssocMgr_AddNewUrlType("gg:",Translate("Gadu-Gadu Link Protocol"),hInstance,IDI_GG,szService,0);
    return 0;
}

void gg_registerlinks(void)
{
    char szService[MAXMODULELABELLENGTH];
    mir_snprintf(szService,sizeof(szService),"%s%s",GG_PROTO,"ParseLink");
	hServiceParseLink=CreateServiceFunction(szService,ServiceParseLink);
    hHookModulesLoaded=HookEvent(ME_SYSTEM_MODULESLOADED,LinksModulesLoaded);
}

void gg_unregisterlinks(void)
{
    UnhookEvent(hHookModulesLoaded);
    DestroyServiceFunction(hServiceParseLink);
}

#undef hInstance
#endif

// -----------------------------------------

#ifdef JABBER_SUPPORT_TEST

#define jabberProtoName  "JABBER"
#define IDI_JABBER  102
#define hInst  GetModuleHandleA("JABBER")
static __inline HANDLE JabberHContactFromJID(const char *jid) { jid; return NULL; }
static __inline HANDLE JabberDBCreateContact(char *jid,char *nick,BOOL temporary,BOOL stripResource) { jid; nick; temporary; stripResource; return db_find_first(); }
#include <m_protosvc.h>
#include <m_message.h>

#include "m_assocmgr.h"
static HANDLE hHookModulesLoaded;
static HANDLE hServiceParseXmppURI;

static int ServiceParseXmppURI(WPARAM wParam,LPARAM lParam)
{
    char *arg=(char*)lParam;
	char *jid;
	UNREFERENCED_PARAMETER(wParam);
    if(arg==NULL) return 1; /* sanity check */
	/* skip leading prefix */
	arg=strchr(arg,':');
	if(arg==NULL) return 1; /* parse failed */
	for (++arg;*arg=='/';++arg);
	/*
		complete specification: http://www.xmpp.org/extensions/xep-0147.html
		send message:           xmpp:JID?message;subject=TEXT&body=TEXT
		add user:               xmpp:JID?roster
		remove user:            xmpp:JID?remove
	*/
	/* user id */
	arg=strchr(jid=arg,'?');
	if(arg==NULL) arg+=lstrlenA(arg); /* points to terminating nul */
	else *(arg++)=0;
	if (*jid==0) return 1; /* parse failed */
    /* send a message to a contact */
    else if (*arg==0 || (!_strnicmp(arg,"message",7) && (*(arg+7)==';' || *(arg+7)==0))) {
        char *tok,*subj=NULL,*body=NULL;
        MCONTACT hContact;
		char msg[1024];
		arg+=7;
		while(*arg==';') ++arg;
        tok=strtok(arg,"&"); /* first token */
        while(tok!=NULL) {
            if (!_strnicmp(tok,"subject=",8) && *(tok+8)!=0)
                subj=Netlib_UrlDecode(tok+8);
            if (!_strnicmp(tok,"body=",5) && *(tok+5)!=0)
                body=Netlib_UrlDecode(tok+5);
            tok=strtok(NULL,"&"); /* next token */
        }
		if(ServiceExists(MS_MSG_SENDMESSAGE)) {
            hContact=JabberDBCreateContact(jid,jid,TRUE,FALSE);
			if(subj!=NULL && body!=NULL) {
				mir_snprintf(msg,sizeof(msg),"%.128s %s",subj,body);
				body=msg;
			} else if(body==NULL) body=subj;
            if(hContact!=NULL)
                CallService(MS_MSG_SENDMESSAGE,hContact,(LPARAM)body);
		}
		return 0;
    }
    /* add user to contact list */
    else if (!_strnicmp(arg,"roster",6) && (*(arg+6)==';' || *(arg+6)==0)) {
        ADDCONTACTSTRUCT acs;
        PROTOSEARCHRESULT psr;
        if(JabberHContactFromJID(jid)==NULL) { /* does not yet check if jid belongs to current user */
            acs.handleType=HANDLE_SEARCHRESULT;
            acs.szProto=jabberProtoName;
            acs.psr=&psr;
            ZeroMemory(&psr,sizeof(PROTOSEARCHRESULT));
            psr.cbSize=sizeof(PROTOSEARCHRESULT);
            psr.nick=jid;
            CallService(MS_ADDCONTACT_SHOW,0,(LPARAM)&acs);
        }
		return 0;
    }
	/* remove user from contact list */
    else if (!_strnicmp(arg,"remove",6) && (*(arg+6)==';' || *(arg+6)==0)) {
		MCONTACT hContact;
		hContact=JabberHContactFromJID(jid);
        if(hContact==NULL) /* not yet implemented: show standard miranda dialog here */
			CallService(MS_DB_CONTACT_DELETE,hContact,0);
		return 0;
    }
    /* add user subscription */
    else if (!_strnicmp(arg,"subscribe",9) && (*(arg+9)==';' || *(arg+9)==0)) {
		/* not yet implemented */
		return 0;
    }
	/* remove user subscription */
    else if (!_strnicmp(arg,"unsubscribe",11) && (*(arg+11)==';' || *(arg+11)==0)) {
		/* not yet implemented */
		return 0;
    }
    return 1; /* parse failed */
}

static int JabberLinksModulesLoaded(WPARAM wParam,LPARAM lParam)
{
    char szService[MAXMODULELABELLENGTH];
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
    mir_snprintf(szService,sizeof(szService),"%s%s",jabberProtoName,"ParseXmppURI");
    AssocMgr_AddNewUrlType("xmpp:",Translate("Jabber Link Protocol"),hInst,IDI_JABBER,szService,0);
    return 0;
}

int JabberLinksInit()
{
    char szService[MAXMODULELABELLENGTH];
    mir_snprintf(szService,sizeof(szService),"%s%s",jabberProtoName,"ParseXmppURI");
	hServiceParseXmppURI=CreateServiceFunction(szService,ServiceParseXmppURI);
    hHookModulesLoaded=HookEvent(ME_SYSTEM_MODULESLOADED,JabberLinksModulesLoaded);
	return 0;
}

int JabberLinksUninit()
{
    UnhookEvent(hHookModulesLoaded);
    DestroyServiceFunction(hServiceParseXmppURI);
	return 0;
}

#undef hInst
#endif

// -----------------------------------------

/*
static HANDLE hServiceTest;
static int TestingService(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	MessageBoxEx(NULL, (TCHAR*)lParam, TranslateT("Testing Service"), MB_OK | MB_SETFOREGROUND | MB_TOPMOST | MB_TASKMODAL, LANGIDFROMLCID((LCID)CallService(MS_LANGPACK_GETLOCALE, 0, 0)));
	return 0;
}
*/

void InitTest(void)
{
	#ifdef AIM_SUPPORT_TEST
	conn.hInstance=GetModuleHandleA("AIM");
	aim_links_init();
	#endif
	#ifdef ICQ_SUPPORT_TEST
	InitIcqFiles();
	#endif
	#ifdef YAHOO_SUPPORT_TEST
	YmsgrLinksInit();
	#endif
	#ifdef MSN_SUPPORT_TEST
	LoadMsnLinks();
	#endif
	#ifdef GG_SUPPORT_TEST
	gg_registerlinks();
	#endif
	#ifdef JABBER_SUPPORT_TEST
	JabberLinksInit();
	#endif
	//hServiceTest=CreateServiceFunction("AssocMgr/TestingService",TestingService);
	//AssocMgr_AddNewFileTypeT(".mir",NULL,TranslateT("Miranda Installer Package (demo purpose)"),TranslateT("&Install"),hInst,IDI_MIRANDAFILE,"AssocMgr/TestingService",0);
}

void UninitTest(void)
{
	#ifdef AIM_SUPPORT_TEST
	aim_links_destroy();
	#endif
	#ifdef ICQ_SUPPORT_TEST
	UninitIcqFiles();
	#endif
	#ifdef YAHOO_SUPPORT_TEST
	YmsgrLinksUninit();
	#endif
	#ifdef MSN_SUPPORT_TEST
	UnloadMsnLinks();
	#endif
	#ifdef GG_SUPPORT_TEST
	gg_unregisterlinks();
	#endif
	#ifdef JABBER_SUPPORT_TEST
	JabberLinksUninit();
	#endif
	//DestroyServiceFunction(hServiceTest);
}
