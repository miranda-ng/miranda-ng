/*
 * $Id: yahoo.cpp 13557 2011-04-09 02:26:58Z borkra $
 *
 * myYahoo Miranda Plugin 
 *
 * Authors: Gennady Feldman (aka Gena01) 
 *          Laurent Marechal (aka Peorth)
 *
 * This code is under GPL and is based on AIM, MSN and Miranda source code.
 * I want to thank Robert Rainwater and George Hazan for their code and support
 * and for answering some of my questions during development of this plugin.
 */

/*
 * Miranda headers
 */
#include "yahoo.h"
#include <m_protosvc.h>
#include <m_langpack.h>
#include <m_skin.h>
#include <m_popup.h>
#include <m_message.h>

#include "avatar.h"
#include "chat.h"
#include "webcam.h"
#include "file_transfer.h"
#include "im.h"
#include "search.h"
#include "ignore.h"

typedef struct {
	int id;
	char *label;
} yahoo_idlabel;

typedef struct {
	int id;
	char *who;
} yahoo_authorize_data;

yahoo_idlabel yahoo_status_codes[] = {
	{YAHOO_STATUS_AVAILABLE, ""},
	{YAHOO_STATUS_BRB, "BRB"},
	{YAHOO_STATUS_BUSY, "Busy"},
	{YAHOO_STATUS_NOTATHOME, "Not At Home"},
	{YAHOO_STATUS_NOTATDESK, "Not At my Desk"},
	{YAHOO_STATUS_NOTINOFFICE, "Not In The Office"},
	{YAHOO_STATUS_ONPHONE, "On The Phone"},
	{YAHOO_STATUS_ONVACATION, "On Vacation"},
	{YAHOO_STATUS_OUTTOLUNCH, "Out To Lunch"},
	{YAHOO_STATUS_STEPPEDOUT, "Stepped Out"},
	{YAHOO_STATUS_INVISIBLE, "Invisible"},
	{YAHOO_STATUS_IDLE, "Idle"},
	{YAHOO_STATUS_OFFLINE, "Offline"},
	{YAHOO_STATUS_CUSTOM, "[Custom]"},
//	{YAHOO_STATUS_NOTIFY, "Notify"},
	{0, NULL}
};

#define MAX_PREF_LEN 255

int do_yahoo_debug = 0;
#ifdef HTTP_GATEWAY
int iHTTPGateway = 0;
#endif
extern int poll_loop;

char * yahoo_status_code(enum yahoo_status s)
{
	int i;

	for(i=0; yahoo_status_codes[i].label; i++)
		if(yahoo_status_codes[i].id == s)
			return yahoo_status_codes[i].label;
		
	return "Unknown";
}

yahoo_status miranda_to_yahoo(int myyahooStatus)
{
	yahoo_status ret = YAHOO_STATUS_AVAILABLE;

	switch (myyahooStatus) {
	case ID_STATUS_OFFLINE:
		ret = YAHOO_STATUS_OFFLINE;
		break;

	case ID_STATUS_FREECHAT:
	case ID_STATUS_ONLINE: 
		ret = YAHOO_STATUS_AVAILABLE;
		break;

	case ID_STATUS_AWAY:
		ret = YAHOO_STATUS_STEPPEDOUT;
		break;

	case ID_STATUS_NA:
		ret = YAHOO_STATUS_BRB;
		break;

	case ID_STATUS_OCCUPIED:
		ret = YAHOO_STATUS_BUSY;
		break;

	case ID_STATUS_DND:
		ret = YAHOO_STATUS_BUSY;
		break;

	case ID_STATUS_ONTHEPHONE:
		ret = YAHOO_STATUS_ONPHONE;
		break;

	case ID_STATUS_OUTTOLUNCH:
		ret = YAHOO_STATUS_OUTTOLUNCH;                            
		break;

	case ID_STATUS_INVISIBLE:
		ret = YAHOO_STATUS_INVISIBLE;
		break;
	}                                                

    return ret;
}

int yahoo_to_miranda_status(int m_iStatus, int away)
{
	int ret = ID_STATUS_OFFLINE;

	switch (m_iStatus) {
	case YAHOO_STATUS_AVAILABLE: 
		ret = ID_STATUS_ONLINE;
		break;
	case YAHOO_STATUS_BRB:
		ret = ID_STATUS_NA;
		break;
	case YAHOO_STATUS_BUSY:
		ret = ID_STATUS_OCCUPIED;
		break;
	case YAHOO_STATUS_ONPHONE:
		ret = ID_STATUS_ONTHEPHONE;
		break;
	case YAHOO_STATUS_OUTTOLUNCH:
		ret = ID_STATUS_OUTTOLUNCH;                            
		break;
	case YAHOO_STATUS_INVISIBLE:
		ret = ID_STATUS_INVISIBLE;
		break;
	case YAHOO_STATUS_NOTATHOME:
	case YAHOO_STATUS_NOTATDESK:
	case YAHOO_STATUS_NOTINOFFICE:
	case YAHOO_STATUS_ONVACATION:
	case YAHOO_STATUS_STEPPEDOUT:
	case YAHOO_STATUS_IDLE:
		ret = ID_STATUS_AWAY;
		break;
	case YAHOO_STATUS_CUSTOM:
		ret = (away ? ID_STATUS_AWAY:ID_STATUS_ONLINE);
		break;
	}
	return ret;
}

void CYahooProto::set_status(int myyahooStatus, char *msg, int away)
{
	LOG(("[set_status] myyahooStatus: %d, msg: %s, away: %d", myyahooStatus, msg, away));

	/* Safety check, don't dereference Invalid pointers */
	if (m_id > 0)  {
			
		if (YAHOO_CUSTOM_STATUS != myyahooStatus)
			yahoo_set_away(m_id, miranda_to_yahoo(myyahooStatus), msg, away);
		else
			yahoo_set_away(m_id, ( yahoo_status )YAHOO_CUSTOM_STATUS, msg, away);
	}
}

void CYahooProto::stealth(const char *buddy, int add)
{
	LOG(("[stealth] buddy: %s, add: %d", buddy, add));

	/* Safety check, don't dereference Invalid pointers */
	if (m_id > 0)
		yahoo_set_stealth(m_id, buddy, GetWord(getbuddyH(buddy), "yprotoid", 0), add);
}

void CYahooProto::remove_buddy(const char *who, int protocol)
{
	LOG(("[remove_buddy] Buddy: '%s' protocol: %d", who, protocol));

	DBVARIANT dbv;
	if ( DBGetContactSettingString( getbuddyH(who), m_szModuleName, "YGroup", &dbv )) {
		LOG(("WARNING NO DATABASE GROUP  using 'miranda'!"));
		yahoo_remove_buddy(m_id, who, protocol, "miranda");
		return;
	}

	yahoo_remove_buddy(m_id, who, protocol, dbv.pszVal);
	DBFreeVariant( &dbv );
}

void CYahooProto::sendtyping(const char *who, int protocol, int stat)
{
	LOG(("[sendtyping] Sending Typing Notification to '%s' protocol: %d, state: %d", who, protocol, stat));
	yahoo_send_typing(m_id, NULL, who, protocol, stat);
}

void CYahooProto::accept(const char *who, int protocol)
{
	yahoo_accept_buddy(m_id, who, protocol);
}

void CYahooProto::reject(const char *who, int protocol, const char *msg)
{
	yahoo_reject_buddy(m_id, who, protocol, msg);
}

void CYahooProto::logout()
{
	LOG(("[yahoo_logout]"));

	if (m_bLoggedIn)
		yahoo_logoff(m_id);

	/* need to stop the server and close all the connections */
	poll_loop = 0;
}

void CYahooProto::AddBuddy( const char *who, int protocol, const char *group, const char *msg)
{
	DBVARIANT dbv;
	char *fname=NULL, *lname=NULL;
	
	/* We adding a buddy to our list.
	  2 Stages.
	1. We send add buddy packet.
	2. We get a packet back from the server confirming add.
	
	No refresh needed. */
	
	if ( !DBGetContactSettingString( NULL, m_szModuleName, "First Name", &dbv )){
		fname = strdup(dbv.pszVal);
		DBFreeVariant(&dbv);
	}

	if ( !DBGetContactSettingString( NULL, m_szModuleName, "Last Name", &dbv )){
		lname = strdup(dbv.pszVal);
		DBFreeVariant(&dbv);
	}

	yahoo_add_buddy(m_id, fname, lname, who, protocol, group, msg);

	FREE(fname);
	FREE(lname);
}

HANDLE CYahooProto::getbuddyH(const char *yahoo_id)
{
	char  *szProto;
	HANDLE hContact;

	for ( hContact = ( HANDLE )YAHOO_CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
		hContact != NULL;
		hContact = ( HANDLE )YAHOO_CallService( MS_DB_CONTACT_FINDNEXT, ( WPARAM )hContact, 0 ))
	{
		szProto = ( char* )YAHOO_CallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM )hContact, 0 );
		if ( szProto != NULL && !lstrcmpA( szProto, m_szModuleName ))
		{
			DBVARIANT dbv;
			if ( DBGetContactSettingString( hContact, m_szModuleName, YAHOO_LOGINID, &dbv ))
				continue;

			{	
				int tCompareResult = lstrcmpiA( dbv.pszVal, yahoo_id );
				DBFreeVariant( &dbv );
				if ( tCompareResult )
					continue;
			}

			return hContact;
		}	
	}

	return NULL;
}

HANDLE CYahooProto::add_buddy( const char *yahoo_id, const char *yahoo_name, int protocol, DWORD flags )
{
	HANDLE hContact;
	char *yid = strdup(yahoo_id);
	
	strlwr(yid);
	
	hContact = getbuddyH(yid);
	if (hContact != NULL) {
		LOG(("[add_buddy] Found buddy id: %s, handle: %p", yid, hContact));
		if ( !( flags & PALF_TEMPORARY ) && DBGetContactSettingByte( hContact, "CList", "NotOnList", 1 )) 
		{
			LOG(("[add_buddy] Making Perm id: %s, flags: %lu", yahoo_id, flags));
			DBDeleteContactSetting( hContact, "CList", "NotOnList" );
			DBDeleteContactSetting( hContact, "CList", "Hidden" );

		}
		
		FREE(yid);
		return hContact;
	}

	//not already there: add
	LOG(("[add_buddy] Adding buddy id: %s (Nick: %s), flags: %lu", yid, yahoo_name, flags));
	hContact = ( HANDLE )YAHOO_CallService( MS_DB_CONTACT_ADD, 0, 0 );
	YAHOO_CallService( MS_PROTO_ADDTOCONTACT, ( WPARAM )hContact,( LPARAM )m_szModuleName );
	SetString( hContact, YAHOO_LOGINID, yid );
	Set_Protocol( hContact, protocol );

	if (lstrlenA(yahoo_name) > 0)
		SetStringUtf( hContact, "Nick", yahoo_name );
	else
		SetStringUtf( hContact, "Nick", yahoo_id );

	if (flags & PALF_TEMPORARY ) {
		DBWriteContactSettingByte( hContact, "CList", "NotOnList", 1 );
		DBWriteContactSettingByte( hContact, "CList", "Hidden", 1 );
	}	
	
	FREE(yid);
	return hContact;
}

const char* CYahooProto::find_buddy( const char *yahoo_id)
{
	static char nick[128];
	HANDLE hContact;
	DBVARIANT dbv;

	hContact = getbuddyH(yahoo_id);
	if (hContact != NULL) {
		if ( DBGetContactSettingString( hContact, m_szModuleName, "Nick", &dbv ))
			return NULL;

		strncpy(nick, dbv.pszVal, 128);
		DBFreeVariant( &dbv );
		return nick;
	}

	return NULL;
}

/* Required handlers below */

/* Other handlers */
void CYahooProto::ext_status_changed(const char *who, int protocol, int stat, const char *msg, int away, int idle, int mobile, int utf8)
{
	HANDLE 	hContact = 0;
	time_t  idlets = 0;
	
	YAHOO_DEBUGLOG("[ext_status_changed] %s (prot: %d) with msg %s utf8: %d, stat: %s (%d), away: %d, idle: %d seconds", 
						who, 
						protocol, 
						msg, 
						utf8, 
						(stat == 0) ? "Online" : yahoo_status_code( (yahoo_status)stat ),
						stat, 
						away, 
						idle);
	
	hContact = getbuddyH(who);
	if (hContact == NULL) {
		YAHOO_DEBUGLOG("Buddy Not Found. Adding...");
		hContact = add_buddy(who, who, protocol, 0);
/*	} else {
		YAHOO_DEBUGLOG("Buddy Found On My List! Buddy %p", hContact);*/
	}
	
	if (!mobile)
		SetWord(hContact, "Status", yahoo_to_miranda_status(stat,away));
	else
		SetWord(hContact, "Status", ID_STATUS_ONTHEPHONE);
	
	SetWord(hContact, "YStatus", stat);
	SetWord(hContact, "YAway", away);
	SetWord(hContact, "Mobile", mobile);

	if(msg) {
		YAHOO_DEBUGLOG("[ext_status_changed] %s custom message '%s'", who, msg);

		if (utf8)
			DBWriteContactSettingStringUtf( hContact, "CList", "StatusMsg", msg);
		else
			DBWriteContactSettingString( hContact, "CList", "StatusMsg", msg);
	}
	else DBDeleteContactSetting(hContact, "CList", "StatusMsg" );

	if (stat == YAHOO_STATUS_OFFLINE) {
		/*
		 * Delete the IdleTS if the user went offline
		 */
		DBDeleteContactSetting(hContact, m_szModuleName, "IdleTS");
	} else {
		if ( (away == 2) || (stat == YAHOO_STATUS_IDLE) || (idle > 0)) {
			/* TODO: set Idle=-1, because of key 138=1 and don't set idlets then */
			if (stat > 0) {
				YAHOO_DEBUGLOG("[ext_status_changed] %s idle for %d:%02d:%02d", who, idle/3600, (idle/60)%60, idle%60);
				
				time(&idlets);
				idlets -= idle;
			}
		} 
			
		DBWriteContactSettingDword(hContact, m_szModuleName, "IdleTS", idlets);
	}

	YAHOO_DEBUGLOG("[ext_status_changed] exiting");
}

void CYahooProto::ext_status_logon(const char *who, int protocol, int stat, const char *msg, int away, int idle, int mobile, int cksum, int buddy_icon, long client_version, int utf8)
{
	YAHOO_DEBUGLOG("[ext_status_logon] %s (prot: %d) with msg %s utf8: %d, (stat: %d, away: %d, idle: %d seconds, checksum: %d buddy_icon: %d client_version: %ld)", who, protocol, msg, utf8, stat, away, idle, cksum, buddy_icon, client_version);
	
	ext_status_changed(who, protocol, stat, msg, away, idle, mobile, utf8);

	HANDLE hContact = getbuddyH(who);
	if (hContact == NULL) {
		YAHOO_DEBUGLOG("[ext_status_logon] Can't find handle for %s??? PANIC!!!", who);
		return;
	} 

	/**
	 * We only do real client detection when using Yahoo client. Other option is to fill in the protocol info
	 */
	if (protocol == 0) {
		char *s = NULL;
		
		switch (client_version & 0x7FFFBF) {
		case 2:
			s = "Yahoo Mobile";
			break;
		
		case 6:
			s = "Yahoo PingBox";
			break;
			
		case 3075:
			s = "Yahoo Web Messenger";
			break;
	
		case 35846:
			s = "iPhone Y! Messenger"; // iPhone Y! Messenger 1.5
			break;
			
		case 262651: 
			s = "libyahoo2"; 
			break;
			
		case 262655: 
			s = "< Yahoo 6.x (Yahoo 5.x?)"; 
			break;
			
		case 278527: 
			s = "Yahoo 6.x"; 
			break;
			
		case 524223: 
			//Yahoo 7.4
			//Yahoo 7.5
			s = "Yahoo 7.x"; 
			break;
			
		case 888327:
			s = "Yahoo 9.0 for Vista";
			break;
	
		case 822366: /* Yahoo! Messenger 2.1.37 by RIM */
			s = "Yahoo for Blackberry";
			break;
	
		case 822543:  /* ? "Yahoo Version 3.0 beta 1 (build 18274) OSX" */
		case 1572799: /* 8.0.x ??  */ 
		case 2097087: /* 8.1.0.195 */ 
			s = "Yahoo 8.x"; 
			break;
			
		case 2088895:
		case 4194239:
			s = "Yahoo 9.0";
			break;
			
		case 8388543:
			s = "Yahoo 10.0";
			break;
		}

		if (s != NULL) 
			SetString( hContact, "MirVer", s);
		else
			DBDeleteContactSetting( hContact, m_szModuleName, "MirVer");
	
	} else {
		Set_Protocol(hContact, protocol);
	}
	
	/* Add the client_Version # to the contact DB entry */
	DBWriteContactSettingDword( hContact, m_szModuleName, "ClientVersion", client_version);
	
	/* Last thing check the checksum and request new one if we need to */
	if (buddy_icon == -1) {
		YAHOO_DEBUGLOG("[ext_status_logon] No avatar information in this packet? Not touching stuff!");
	} else {
		// we got some avatartype info
		DBWriteContactSettingByte(hContact, m_szModuleName, "AvatarType", buddy_icon);
		
		if (cksum == 0 || cksum == -1){
			// no avatar
			DBWriteContactSettingDword(hContact, m_szModuleName, "PictCK", 0);
		} else if (DBGetContactSettingDword(hContact, m_szModuleName,"PictCK", 0) != cksum) {
			//char szFile[MAX_PATH];
			
			// Save new Checksum
			DBWriteContactSettingDword(hContact, m_szModuleName, "PictCK", cksum);
			
			// Need to delete the Avatar File!!
			//GetAvatarFileName(hContact, szFile, sizeof szFile, 0);
			//DeleteFile(szFile);
		}
	
		// Cleanup the type? and reset things...
		reset_avatar(hContact);
	}
	
	YAHOO_DEBUGLOG("[ext_status_logon] exiting");
}

void CYahooProto::ext_got_audible(const char *me, const char *who, const char *aud, const char *msg, const char *aud_hash)
{
	/* aud = file class name 
					GAIM: the audible, in foo.bar.baz format
			
					Actually this is the filename.
					Full URL:
				
					http://us.dl1.yimg.com/download.yahoo.com/dl/aud/us/aud.swf 
					
					where aud in foo.bar.baz format
	*/
	
	LOG(("ext_got_audible for %s aud: %s msg:'%s' hash: %s", who, aud, msg, aud_hash));

	HANDLE hContact = getbuddyH(who);
	if (hContact == NULL) {
		LOG(("Buddy Not Found. Skipping avatar update"));
		return;
	}
	
	char z[1028];
	mir_snprintf(z, sizeof(z), "[miranda-audible] %s", msg ?msg:"");
	ext_got_im((char*)me, (char*)who, 0, z, 0, 0, 1, -1, NULL, 0);
}

void CYahooProto::ext_got_calendar(const char *url, int type, const char *msg, int svc)
{
	LOG(("[ext_got_calendar] URL:%s type: %d msg: %s svc: %d", url, type, msg, svc));
	
	if (!ShowPopup( Translate("Calendar Reminder"), msg, url))
		ShowNotification(Translate("Calendar Reminder"), msg, NIIF_INFO);
}

void CYahooProto::ext_got_stealth(char *stealthlist)
{
	char **s;
	int found = 0;
	char **stealth = NULL;
	char  *szProto;
	HANDLE hContact;

	LOG(("[ext_got_stealth] list: %s", stealthlist));
	
	if (stealthlist)
		stealth = y_strsplit(stealthlist, ",", -1);

	for ( hContact = ( HANDLE )YAHOO_CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
		   hContact != NULL;
			hContact = ( HANDLE )YAHOO_CallService( MS_DB_CONTACT_FINDNEXT, ( WPARAM )hContact, 0 ))
	{
		szProto = ( char* )YAHOO_CallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM )hContact, 0 );
		if ( szProto != NULL && !lstrcmpA( szProto, m_szModuleName )) {
			DBVARIANT dbv;
			if ( DBGetContactSettingString( hContact, m_szModuleName, YAHOO_LOGINID, &dbv ))
				continue;

			found = 0;

			for(s = stealth; s && *s; s++) {

				if (lstrcmpiA(*s, dbv.pszVal) == 0) {
					YAHOO_DEBUGLOG("GOT id = %s", dbv.pszVal);
					found = 1;
					break;
				}
			}

			/* Check the stealth list */
			if (found) { /* we have him on our Stealth List */
				YAHOO_DEBUGLOG("Setting STEALTH for id = %s", dbv.pszVal);
				/* need to set the ApparentMode thingy */
				if (ID_STATUS_OFFLINE != DBGetContactSettingWord(hContact, m_szModuleName, "ApparentMode", 0))
					DBWriteContactSettingWord(hContact, m_szModuleName, "ApparentMode", (WORD) ID_STATUS_OFFLINE);

			} else { /* he is not on the Stealth List */
				//LOG(("Resetting STEALTH for id = %s", dbv.pszVal));
				/* need to delete the ApparentMode thingy */
				if (DBGetContactSettingWord(hContact, m_szModuleName, "ApparentMode", 0))
					DBDeleteContactSetting(hContact, m_szModuleName, "ApparentMode");
			}

			DBFreeVariant( &dbv );
		}		
	}
}

void CYahooProto::ext_got_buddies(YList * buds)
{
	LOG(("[ext_got_buddies] "));

	if (buds == NULL) {
		LOG(("No Buddies to work on!"));
		return;
	}

	YAHOO_DEBUGLOG(("[ext_got_buddies] Walking buddy list..."));
	for(; buds; buds = buds->next) {
		HANDLE hContact;

		yahoo_buddy *bud = ( yahoo_buddy* )buds->data;
		if (bud == NULL) {
			LOG(("[ext_got_buddies] EMPTY BUDDY LIST??"));
			continue;
		}

		YAHOO_DEBUGLOG("[ext_got_buddies] id = %s, protocol = %d, group = %s, auth = %d", bud->id, bud->protocol, bud->group, bud->auth);
		
		hContact = getbuddyH(bud->id);
		if (hContact == NULL)
			hContact = add_buddy(bud->id, (bud->real_name != NULL) ? bud->real_name : bud->id, bud->protocol, 0);

		if (bud->protocol != 0)
			Set_Protocol(hContact, bud->protocol);

		if (bud->group)
			SetString( hContact, "YGroup", bud->group);

		if (bud->stealth) { /* we have him on our Stealth List */
			YAHOO_DEBUGLOG("Setting STEALTH for id = %s", bud->id);
			/* need to set the ApparentMode thingy */
			if (ID_STATUS_OFFLINE != DBGetContactSettingWord(hContact, m_szModuleName, "ApparentMode", 0))
				DBWriteContactSettingWord(hContact, m_szModuleName, "ApparentMode", (WORD) ID_STATUS_OFFLINE);

		} else { /* he is not on the Stealth List */
			//LOG(("Resetting STEALTH for id = %s", dbv.pszVal));
			/* need to delete the ApparentMode thingy */
			if (DBGetContactSettingWord(hContact, m_szModuleName, "ApparentMode", 0))
				DBDeleteContactSetting(hContact, m_szModuleName, "ApparentMode");
		}

		//if (bud->auth)
		//	YAHOO_DEBUGLOG( "Auth request waiting for: %s", bud->id );
		SetByte(hContact, "YAuth", bud->auth);

		if (bud->real_name) {
			YAHOO_DEBUGLOG("id = %s name = %s", bud->id, bud->real_name);
			SetStringUtf( hContact, "Nick", bud->real_name);
		}

		if (bud->yab_entry) {
			//LOG(("YAB_ENTRY"));

			if (bud->yab_entry->fname) 
				SetStringUtf( hContact, "FirstName", bud->yab_entry->fname);

			if (bud->yab_entry->lname) 
				SetStringUtf( hContact, "LastName", bud->yab_entry->lname);

			if (bud->yab_entry->email) 
				SetString( hContact, "e-mail", bud->yab_entry->email);

			if (bud->yab_entry->mphone) 
				SetString( hContact, "Cellular", bud->yab_entry->mphone);

			if (bud->yab_entry->hphone) 
				SetString( hContact, "Phone", bud->yab_entry->hphone);

			if (bud->yab_entry->wphone) 
				SetString( hContact, "CompanyPhone", bud->yab_entry->wphone);

			SetWord( hContact, "YabID", bud->yab_entry->dbid);
		}
	}
	
	YAHOO_DEBUGLOG(("[ext_got_buddies] buddy list Finished."));
}

void CYahooProto::ext_rejected(const char *who, const char *msg)
{
	LOG(("[ext_rejected] who: %s  msg: %s", who, msg));

	HANDLE hContact = getbuddyH(who);
	if (hContact != NULL) {
		/*
		* Make sure the contact is temporary so we could delete it w/o extra traffic
		*/ 
		DBWriteContactSettingByte( hContact, "CList", "NotOnList", 1 );
		YAHOO_CallService( MS_DB_CONTACT_DELETE, (WPARAM) hContact, 0);	
	}
	else LOG(("[ext_rejected] Buddy not on our buddy list"));

	char buff[1024];
	mir_snprintf(buff, sizeof(buff), Translate("%s has rejected your request and sent the following message:"), who);    
	MessageBoxA( NULL, msg, buff, MB_OK | MB_ICONINFORMATION );
}

void CYahooProto::ext_buddy_added(char *myid, char *who, char *group, int status, int auth)
{
	HANDLE hContact=getbuddyH(who);;
	
	LOG(("[ext_buddy_added] %s authorized you as %s group: %s status: %d auth: %d", who, myid, group, status, auth));
	
	switch (status) {
	case 0: /* we are ok */
	case 2: /* seems that we ok, not sure what this means.. we already on buddy list? */
	case 40: /* When adding MSN Live contacts we get this one? */
		DBDeleteContactSetting( hContact, "CList", "NotOnList" );
		DBDeleteContactSetting( hContact, "CList", "Hidden" );
		break;

	case 1:  /* invalid ID? */
	case 3:  /* invalid ID  */
		if (hContact != NULL) {
			ShowPopup( "Invalid Contact", "The id you tried to add is invalid.", NULL);
			/* Make it TEMP first, we don't want to send any extra packets for FALSE ids */
			DBWriteContactSettingByte( hContact, "CList", "NotOnList", 1 );
			YAHOO_CallService( MS_DB_CONTACT_DELETE, (WPARAM) hContact, 0);
		}
		break;

	default:
		/* ??? */
		if (hContact != NULL) {
			ShowPopup( "Invalid Contact", "Unknown Error??.", NULL);
			/* Make it TEMP first, we don't want to send any extra packets for FALSE ids */
			DBWriteContactSettingByte( hContact, "CList", "NotOnList", 1 );
			YAHOO_CallService( MS_DB_CONTACT_DELETE, (WPARAM) hContact, 0);
		}

		break;
	}
}

void CYahooProto::ext_contact_added(const char *myid, const char *who, const char *fname, const char *lname, const char *msg, int protocol)
{
	char		nick[128];
	BYTE		*pCurBlob;
	HANDLE		hContact = NULL;
	CCSDATA 	ccs = { 0 };
	PROTORECVEVENT pre = { 0 };

	/* NOTE: Msg is actually in UTF8 unless stated otherwise!! */
    LOG(("[ext_contact_added] %s %s (%s:%d) added you as %s w/ msg '%s'", fname, lname, who, protocol, myid, msg));
    
	if ( BuddyIgnored( who )) {
		LOG(("User '%s' on our Ignore List. Dropping Authorization Request.", who));
		return;
	}
	
	nick[0] = '\0';
	
	if (lname != NULL) {
		if (fname != NULL) {
			mir_snprintf(nick, sizeof(nick), "%s %s", fname, lname);
		} else {
			mir_snprintf(nick, sizeof(nick), "%s", lname);
		}
	} else if (fname != NULL) {
		mir_snprintf(nick, sizeof(nick), "%s", fname);
	}
	
	if (nick[0] == '\0') 
		mir_snprintf(nick, sizeof(nick), "%s", who);
	
	hContact = add_buddy(who, nick, protocol, PALF_TEMPORARY);
	
	if (lstrcmpA(nick, who) != 0)
		SetStringUtf( hContact, "Nick", nick);
	
	//SetWord(hContact, "yprotoid", protocol);
	Set_Protocol(hContact, protocol);
	
	ccs.szProtoService	= PSR_AUTH;
	ccs.hContact		= hContact;
	ccs.wParam			= 0;
	ccs.lParam			= (LPARAM) &pre;
	pre.flags			= PREF_UTF;
	pre.timestamp		= time(NULL);
	
	pre.lParam = sizeof(DWORD)+sizeof(HANDLE)+lstrlenA(who)+lstrlenA(nick)+5;

	if (fname != NULL)
		pre.lParam += lstrlenA(fname);

	if (lname != NULL)
		pre.lParam += lstrlenA(lname);

	if (msg != NULL)
		pre.lParam += lstrlenA(msg);

	pCurBlob=(PBYTE)malloc(pre.lParam);
	pre.szMessage = (char *)pCurBlob;

	/*
	Auth blob is: uin(DWORD),hcontact(HANDLE),nick(ASCIIZ),first(ASCIIZ),
	last(ASCIIZ),email(ASCIIZ),reason(ASCIIZ)

	blob is: 0(DWORD), hContact(HANDLE), nick(ASCIIZ), fname (ASCIIZ), lname (ASCIIZ), email(ASCIIZ), msg(ASCIIZ)

	*/

	// UIN
	*( PDWORD )pCurBlob = 0;
	pCurBlob+=sizeof(DWORD);

	// hContact
	*( PHANDLE )pCurBlob = hContact; 
	pCurBlob+=sizeof(HANDLE);

	// NICK
	lstrcpyA((char *)pCurBlob, nick); 

	pCurBlob+=lstrlenA((char *)pCurBlob)+1;

	// FIRST
	lstrcpyA((char *)pCurBlob, (fname != NULL) ? fname : ""); 
	pCurBlob+=lstrlenA((char *)pCurBlob)+1;

	// LAST
	lstrcpyA((char *)pCurBlob, (lname != NULL) ? lname : ""); 
	pCurBlob+=lstrlenA((char *)pCurBlob)+1;

	// E-mail    
	lstrcpyA((char *)pCurBlob,who); 
	pCurBlob+=lstrlenA((char *)pCurBlob)+1;

	// Reason
	lstrcpyA((char *)pCurBlob, (msg != NULL) ? msg : "" ); 

	CallService(MS_PROTO_CHAINRECV,0,(LPARAM)&ccs);
}

void CYahooProto::ext_typing_notify(const char *me, const char *who, int protocol, int stat)
{
	LOG(("[ext_typing_notify] me: '%s' who: '%s' protocol: %d stat: %d ", me, who, protocol, stat));

	HANDLE hContact = getbuddyH(who);
	
	if (hContact) 
		CallService(MS_PROTO_CONTACTISTYPING, (WPARAM)hContact, (LPARAM)stat?10:0);
}

void CYahooProto::ext_game_notify(const char *me, const char *who, int stat, const char *msg)
{
	/* There's also game invite packet:
	[17:36:44 YAHOO] libyahoo2/libyahoo2.c:3093: debug: 
[17:36:44 YAHOO] Yahoo Service: (null) (0xb7) Status: YAHOO_STATUS_BRB (1)
[17:36:44 YAHOO]  
[17:36:44 YAHOO] libyahoo2/libyahoo2.c:863: debug: 
[17:36:44 YAHOO] [Reading packet] len: 88
[17:36:44 YAHOO]  
[17:36:44 YAHOO] Key: From (4)  	Value: 'xxxxx'
[17:36:44 YAHOO]  
[17:36:44 YAHOO] Key: To (5)  	Value: 'zxzxxx'
[17:36:44 YAHOO]  
[17:36:44 YAHOO] Key: (null) (180)  	Value: 'pl'
[17:36:44 YAHOO]  
[17:36:44 YAHOO] Key: (null) (183)  	Value: ''
[17:36:44 YAHOO]  
[17:36:44 YAHOO] Key: (null) (181)  	Value: ''
[17:36:44 YAHOO]  
[17:36:44 YAHOO] Key: session (11)  	Value: 'o8114ik_lixyxtdfrxbogw--'
[17:36:44 YAHOO]  
[17:36:44 YAHOO] Key: stat/location (13)  	Value: '1'
[17:36:44 YAHOO]  
[17:36:44 YAHOO] libyahoo2/libyahoo2.c:908: debug: 
[17:36:44 YAHOO] [Reading packet done]

	 */
    LOG(("[ext_game_notify] me: %s, who: %s, stat: %d, msg: %s", me, who, stat, msg));
    /* FIXME - Not Implemented - this informs you someone else is playing on Yahoo! Games */
    /* Also Stubbed in Sample Client */
	HANDLE hContact = getbuddyH(who);
	if (!hContact)
		return;
	
	if (stat == 2) 
		SetString(hContact, "YGMsg", "");
	else if (msg) {
		const char *l = msg, *u = NULL;
		char *z, *c;
		int i = 0;
		
		/* Parse and Set a custom Message 
		 *
		 * Format: 1 [09] ygamesp [09] 1 [09] 0 [09] ante?room=yahoo_1078798506&follow=rrrrrrr	
		 * [09] Yahoo! Poker\nRoom: Intermediate Lounge 2
		 *
		 * Sign-in:
		 * [17:13:42 YAHOO] [ext_yahoo_game_notify] id: 1, me: xxxxx, who: rrrrrrr, 
		 * stat: 1, msg: 1	ygamesa	1	0	ante?room=yahoo_1043183792&follow=lotesdelere	
		 * Yahoo! Backgammon Room: Social Lounge 12 
		 *
		 * Sign-out:
		 * [17:18:38 YAHOO] [ext_yahoo_game_notify] id: 1, me: xxxxx, who: rrrrr, 
		 *	stat: 2, msg: 1	ygamesa	2
		 */
		z = (char *) _alloca(lstrlenA(l) + 50);
		
		z[0]='\0';
		do{
			c = ( char* )strchr(l, 0x09);
			i++;
			if (c != NULL) {
				l = c;
				l++;
				if (i == 4)
					u = l;
			}
		} while (c != NULL && i < 5);
		
		if (c != NULL) {
			// insert \r before \n
			do{
				c = ( char* )strchr(l, '\n');
				
				if (c != NULL) {
					(*c) = '\0';
					lstrcatA(z, l);
					lstrcatA(z, "\r\n");
					l = c + 1;
				} else {
					lstrcatA(z, l);
				}
			} while (c != NULL);
			
			lstrcatA(z, "\r\n\r\nhttp://games.yahoo.com/games/");
			lstrcatA(z, u);
			c = strchr(z, 0x09);
			(*c) = '\0';
		}
		
		SetString(hContact, "YGMsg", z);
		
	} else {
		/* ? no information / reset custom message */
		SetString(hContact, "YGMsg", "");
	}
}

void CYahooProto::ext_mail_notify(const char *from, const char *subj, int cnt)
{
	LOG(("[ext_mail_notify] from: %s subject: %s count: %d", from, subj, cnt));
	
	if (cnt > 0) {
		SkinPlaySound("mail");
	
		if (!GetByte( "DisableYahoomail", 0)) {    
			char z[MAX_SECONDLINE], title[MAX_CONTACTNAME];
			
			if (from == NULL) {
				snprintf(title, sizeof(title), "%s: %s", m_szModuleName, Translate("New Mail"));
				snprintf(z, sizeof(z), Translate("You Have %i unread msgs"), cnt);
			} else {
				snprintf(title, sizeof(title), Translate("New Mail (%i msgs)"), cnt);
				snprintf(z, sizeof(z), Translate("From: %s\nSubject: %s"), from, subj);
			}
	
			if(!ShowPopup( title, z, "http://mail.yahoo.com" ))
				ShowNotification(title, z, NIIF_INFO);
		}
	}
	
	m_unreadMessages = cnt;
	SendBroadcast( NULL, ACKTYPE_EMAIL, ACKRESULT_STATUS, NULL, 0 );
}    
    
void CYahooProto::ext_system_message(const char *me, const char *who, const char *msg)
{
	LOG(("[ext_system_message] System Message to: %s from: %s msg: %s", me, who, msg));
	
	ShowPopup( (who != NULL) ? who : "Yahoo System Message", msg, NULL);
}

void CYahooProto::ext_got_identities(const char *nick, const char *fname, const char *lname, YList * ids)
{
    LOG(("[ext_got_identities] First Name: %s, Last Name: %s", fname, lname));
	
    /* FIXME - Not implemented - Got list of Yahoo! identities */
    /* We currently only use the default identity */
    /* Also Stubbed in Sample Client */
	SetString(NULL, "First Name", fname ? fname : "");
	SetString(NULL, "Last Name", lname ? lname : "");
}

void __cdecl yahoo_get_yab_thread(void *psf) 
{
	int id = (int)psf;
	
	yahoo_get_yab(id);
}


void ext_yahoo_got_cookies(int id)
{
//    char z[1024];

    LOG(("[ext_got_cookies] id: %d", id));
/*    LOG(("Y Cookie: '%s'", yahoo_get_cookie(id, "y")));
    LOG(("T Cookie: '%s'", yahoo_get_cookie(id, "t")));
    LOG(("C Cookie: '%s'", yahoo_get_cookie(id, "c")));
    LOG(("Login Cookie: '%s'", yahoo_get_cookie(id, "login")));
    
    //wsprintfA(z, "Cookie: %s; C=%s; Y=%s; T=%s", Bcookie, yahoo_get_cookie(id, "c"), yahoo_get_cookie(id, "y"), yahoo_get_cookie(id, "t"));
    //wsprintfA(z, "Cookie: %s; Y=%s", Bcookie, yahoo_get_cookie(id, "y"), yahoo_get_cookie(id, "t"));    
    wsprintfA(z, "Cookie: Y=%s; T=%s", yahoo_get_cookie(id, "y"), yahoo_get_cookie(id, "t"));    
    LOG(("Our Cookie: '%s'", z));
    YAHOO_CallService(MS_NETLIB_SETSTICKYHEADERS, (WPARAM)hnuMain, (LPARAM)z);*/

#ifdef HTTP_GATEWAY	
	if (iHTTPGateway) {
		char z[1024];
		
		// need to add Cookie header to our requests or we get booted w/ "Bad Cookie" message.
		mir_snprintf(z, sizeof(z), "Cookie: Y=%s; T=%s; C=%s", yahoo_get_cookie(id, "y"), 
				yahoo_get_cookie(id, "t"), yahoo_get_cookie(id, "c"));    
		LOG(("Our Cookie: '%s'", z));
		YAHOO_CallService(MS_NETLIB_SETSTICKYHEADERS, (WPARAM)hNetlibUser, (LPARAM)z);
	}
#endif
	
}

void CYahooProto::ext_got_ping(const char *errormsg)
{
	LOG(("[ext_got_ping]"));

	if (errormsg) {
		LOG(("[ext_got_ping] Error msg: %s", errormsg));
		ShowError(Translate("Yahoo Ping Error"), errormsg);
		return;
	}

	if (m_iStatus == ID_STATUS_CONNECTING) {
		LOG(("[ext_got_ping] We are connecting. Checking for different status. Start: %d, Current: %d", m_startStatus, m_iStatus));
		if (m_startStatus != m_iStatus) {
			LOG(("[COOKIES] Updating Status to %d ", m_startStatus));    

			if (m_startStatus != ID_STATUS_INVISIBLE) {// don't generate a bogus packet for Invisible state
				if (m_startMsg != NULL) {
					set_status(YAHOO_STATUS_CUSTOM, m_startMsg, (m_startStatus != ID_STATUS_ONLINE) ? 1 : 0);
				} else
					set_status(m_startStatus, NULL, (m_startStatus != ID_STATUS_ONLINE) ? 1 : 0);
			}

			BroadcastStatus(m_startStatus);
			m_bLoggedIn=TRUE;

			/**
			 * Now load the YAB.
			 */
			if (GetByte( "UseYAB", 1 )) {
				LOG(("[ext_got_ping] GET YAB"));
				if (m_iStatus != ID_STATUS_OFFLINE)
					mir_forkthread(yahoo_get_yab_thread, (void *)m_id);
			}

		}
	}
}

void CYahooProto::ext_login_response(int succ, const char *url)
{
	char buff[1024];

	LOG(("[ext_login_response] succ: %d, url: %s", succ, url));
	
	if(succ == YAHOO_LOGIN_OK) {
		const char *c;
		
		m_status = yahoo_current_status(m_id);
		LOG(("logged in status-> %d", m_status));
		
		c = yahoo_get_pw_token(m_id);
		
		SetString(YAHOO_PWTOKEN, c);
		
		LOG(("PW Token-> %s", c));
		return;
	}
	
	if(succ == YAHOO_LOGIN_UNAME) {
		snprintf(buff, sizeof(buff), Translate("Could not log into Yahoo service - username not recognised.  Please verify that your username is correctly typed."));
		ProtoBroadcastAck(m_szModuleName, NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_BADUSERID);
	}
	else if(succ == YAHOO_LOGIN_PASSWD) {
		snprintf(buff, sizeof(buff), Translate("Could not log into Yahoo service - password incorrect.  Please verify that your username and password are correctly typed."));
		ProtoBroadcastAck(m_szModuleName, NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_WRONGPASSWORD);
	}
	else if(succ == YAHOO_LOGIN_LOCK) {
		snprintf(buff, sizeof(buff), Translate("Could not log into Yahoo service.  Your account has been locked.\nVisit %s to reactivate it."), url);
	}
	else if(succ == YAHOO_LOGIN_DUPL) {
		snprintf(buff, sizeof(buff), Translate("You have been logged out of the yahoo service, possibly due to a duplicate login."));
		ProtoBroadcastAck(m_szModuleName, NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_OTHERLOCATION);
	}
	else if(succ == YAHOO_LOGIN_LOGOFF) {
		//snprintf(buff, sizeof(buff), Translate("You have been logged out of the yahoo service."));
		//ProtoBroadcastAck(m_szModuleName, NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_OTHERLOCATION);
		return; // we logged out.. so just sign-off..
	}
	else if(succ == -1) {
		/// Can't Connect or got disconnected.
		if (m_iStatus == ID_STATUS_CONNECTING)
			snprintf(buff, sizeof(buff), Translate("Could not connect to the Yahoo service. Check your server/port and proxy settings."));	
		else
			return;
	} 
	else snprintf(buff, sizeof(buff),Translate("Could not log in, unknown reason: %d."), succ);

	DBDeleteContactSetting(NULL, m_szModuleName, YAHOO_PWTOKEN);
	
	YAHOO_DEBUGLOG("ERROR: %s", buff);
	
	/*
	 * Show Error Message
	 */
	ShowError(Translate("Yahoo Login Error"), buff);
	
	/*
	 * Stop the server thread and let Server cleanup
	 */
	poll_loop = 0;
}

void CYahooProto::ext_error(const char *err, int fatal, int num)
{
	char buff[1024];
	
	LOG(("[ext_error] Error: fatal: %d, num: %d, err: %s", fatal, num, err));
        
	switch(num) {
	case E_UNKNOWN:
		snprintf(buff, sizeof(buff), Translate("Unknown error %s"), err);
		break;
	case E_CUSTOM:
		snprintf(buff, sizeof(buff), Translate("Custom error %s"), err);
		break;
	case E_CONFNOTAVAIL:
		snprintf(buff, sizeof(buff), Translate("%s is not available for the conference"), err);
		break;
	case E_IGNOREDUP:
		snprintf(buff, sizeof(buff), Translate("%s is already ignored"), err);
		break;
	case E_IGNORENONE:
		snprintf(buff, sizeof(buff), Translate("%s is not in the ignore list"), err);
		break;
	case E_IGNORECONF:
		snprintf(buff, sizeof(buff), Translate("%s is in buddy list - cannot ignore "), err);
		break;
	case E_SYSTEM:
		snprintf(buff, sizeof(buff), Translate("System Error: %s"), err);
		break;
	case E_CONNECTION:
		snprintf(buff, sizeof(buff), Translate("Server Connection Error: %s"), err);
		YAHOO_DEBUGLOG("Error: %s", buff);
		return;
	}
	
	YAHOO_DEBUGLOG("Error: %s", buff);
	
	/*
	 * Show Error Message
	 */
	ShowError(Translate("Yahoo Error"), buff);
}

extern HANDLE g_hNetlibUser;

int CYahooProto::ext_connect(const char *h, int p, int type)
{
	LOG(("[ext_connect] %s:%d type: %d", h, p, type));

	HANDLE hUser=m_hNetlibUser;
	NETLIBOPENCONNECTION ncon = {0};
	ncon.cbSize = sizeof(ncon); 
	ncon.flags = NLOCF_V2;
	ncon.szHost = h;
	ncon.wPort = p;
	ncon.timeout = 5;	

	if (type != YAHOO_CONNECTION_PAGER) {
		ncon.flags |= NLOCF_HTTP;
		hUser		= g_hNetlibUser;
	}

	HANDLE con = (HANDLE) CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)hUser, (LPARAM)&ncon);
	if (con == NULL)  {
		LOG(("ERROR: Connect Failed!"));
		return -1;
	}

	LOG(("[ext_connect] Got: %d", (int)con));
	
	return (int)con;
}

void CYahooProto::ext_send_http_request(enum yahoo_connection_type type, const char *method, const char *url, 
		const char *cookies, long content_length, yahoo_get_fd_callback callback, void *callback_data)
{
/*	if (lstrcmpiA(method, "GET") == 0) 
		yahoo_http_get(id, url, cookies, callback, callback_data);
	else if (lstrcmpiA(method, "POST") == 0) 
		yahoo_http_post(id, url, cookies, content_length, callback, callback_data);
	else 
		LOG(("ERROR: Unknown method: %s", method));
*/
    NETLIBHTTPREQUEST 	nlhr={0};
	NETLIBHTTPHEADER 	httpHeaders[5];
	int 				fd, error = 0;
	char 				host[255];
	int 				port = 80, i=0;
	char 				path[255];
	char 				z[1024];
	
	LOG(("[ext_send_http_request] type: %d, method: %s, url: %s, cookies: %s, content length: %ld",
		type, method, url, cookies, content_length));
	
	if(!url_to_host_port_path(url, host, &port, path))
		return;

	fd = ext_connect(host, port, type);
	
	if (fd < 0) {
		LOG(("[ext_send_http_request] Can't connect?? Exiting..."));
		//return;
	} else {
		nlhr.cbSize=sizeof(nlhr);
		nlhr.requestType=(lstrcmpiA(method, "GET") == 0) ? REQUEST_GET : REQUEST_POST;
		nlhr.flags=NLHRF_DUMPASTEXT|NLHRF_HTTP11;
		nlhr.szUrl=(char *)path;
		nlhr.headers = httpHeaders;
		nlhr.headersCount = 3;
		
		if (cookies != NULL && cookies[0] != '\0') {
			httpHeaders[i].szName = "Cookie";
			httpHeaders[i].szValue = (char *)cookies;
			nlhr.headersCount = 4;
			i++;
		}
		
		httpHeaders[i].szName = "User-Agent";
		httpHeaders[i].szValue = "Mozilla/4.0 (compatible; MSIE 5.5)";
		i++;

		httpHeaders[i].szName = "Host";
		httpHeaders[i].szValue = host;
		i++;

		if (nlhr.requestType == REQUEST_POST) {
			httpHeaders[nlhr.headersCount].szName="Content-Length";
			mir_snprintf(z, 1024, "%d", content_length);
			httpHeaders[nlhr.headersCount].szValue=z;
	
			nlhr.headersCount++;
		}
		
		httpHeaders[i].szName = "Cache-Control";
		httpHeaders[i].szValue = "no-cache";
		i++;
		
		error = CallService(MS_NETLIB_SENDHTTPREQUEST,(WPARAM)fd,(LPARAM)&nlhr);
	}
	
	callback(m_id, fd, error == SOCKET_ERROR, callback_data);
}

/*************************************
 * Callback handling code starts here
 */
unsigned int CYahooProto::ext_add_handler(int fd, yahoo_input_condition cond, void *data)
{
	struct _conn *c = y_new0(struct _conn, 1);
	
	c->tag = ++m_connection_tags;
	c->id = m_id;
	c->fd = fd;
	c->cond = cond;
	c->data = data;

	LOG(("[ext_add_handler] fd:%d, id:%d, cond: %d, tag %d", fd, m_id, cond, c->tag));
	
	m_connections = y_list_prepend(m_connections, c);

	return c->tag;
}

void CYahooProto::ext_remove_handler(unsigned int tag)
{
	YList *l;
	
	LOG(("[ext_remove_handler] id:%d tag:%d ", m_id, tag));
	
	for(l = m_connections; l; l = y_list_next(l)) {
		struct _conn *c = ( _conn* )l->data;
		if(c->tag == tag) {
			/* don't actually remove it, just mark it for removal */
			/* we'll remove when we start the next poll cycle */
			LOG(("Marking id:%d fd:%d tag:%d for removal", c->id, c->fd, c->tag));
			c->remove = 1;
			return;
		}
	}
}

struct connect_callback_data {
	yahoo_connect_callback callback;
	void * callback_data;
	int id;
	int tag;
};

void ext_yahoo_remove_handler(int id, unsigned int tag);

static void connect_complete(void *data, int source, yahoo_input_condition condition)
{
	struct connect_callback_data *ccd = ( connect_callback_data* )data;
	int error = 0;//, err_size = sizeof(error);
	NETLIBSELECT tSelect = {0};

	ext_yahoo_remove_handler(ccd->id, ccd->tag);
	
	// We Need to read the Socket error
	//getsockopt(source, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&err_size);
	
	tSelect.cbSize = sizeof( tSelect );
	//tSelect.dwTimeout = T->mGatewayTimeout * 1000;
	tSelect.dwTimeout = 1;
	tSelect.hReadConns[ 0 ] = ( HANDLE )source;
	error = YAHOO_CallService( MS_NETLIB_SELECT, 0, ( LPARAM )&tSelect );

	if(error) {
		//close(source);
		Netlib_CloseHandle((HANDLE)source);
		source = -1;
	}

	LOG(("Connected fd: %d, error: %d", source, error));

	ccd->callback(source, error, ccd->callback_data);
	FREE(ccd);
}

void yahoo_callback(struct _conn *c, yahoo_input_condition cond)
{
	int ret=1;

	//LOG(("[yahoo_callback] id: %d, fd: %d tag: %d", c->id, c->fd, c->tag));
	if(c->id < 0) {
		connect_complete(c->data, c->fd, cond);
	} else if (c->fd > 0) {
	
		if(cond & YAHOO_INPUT_READ)
			ret = yahoo_read_ready(c->id, c->fd, c->data);
		if(ret>0 && cond & YAHOO_INPUT_WRITE)
			ret = yahoo_write_ready(c->id, c->fd, c->data);

		if (ret == -1) {
			LOG(("Yahoo read error (%d): %s", errno, strerror(errno)));
		} else if(ret == 0)
			LOG(("Yahoo read error: Server closed socket"));
	}
	
	//LOG(("[yahoo_callback] id: %d exiting...", c->id));
}

int CYahooProto::ext_connect_async(const char *host, int port, int type, yahoo_connect_callback callback, void *data)
{
	int err = 0, res;
	
    LOG(("[ext_connect_async] %s:%d type: %d", host, port, type));
    
    res = ext_connect(host, port, type);

	LOG(("[ext_connect_async] %s:%d type: %d, result: %d", host, port, type, res));
	
	if (type == YAHOO_CONNECTION_PAGER && res < 1) {
		err = 1;
	} 
		
	/*
	 * need to call the callback so we could handle the failure condition!!!
	 * fd = -1 in case of an error
	 */
	callback(res, err, data);
	
	/*
	 * Return proper thing: 0 - ok, -1 - failed, >0 - pending connect
	 */
	return (res <= 0) ? -1 : 0;
}
/*
 * Callback handling code ends here
 ***********************************/
char * CYahooProto::ext_send_https_request(struct yahoo_data *yd, const char *host, const char *path)
{
	NETLIBHTTPREQUEST nlhr={0},*nlhrReply;
	char z[4096], *result=NULL;
	int i;
	
	wsprintfA(z, "https://%s%s", host, path);
	nlhr.cbSize		= sizeof(nlhr);
	nlhr.requestType= REQUEST_GET;
	nlhr.flags		= NLHRF_HTTP11 | NLHRF_NODUMPSEND | NLHRF_DUMPASTEXT; /* Use HTTP/1.1 and don't dump the requests to the log */
	nlhr.szUrl		= z;

	nlhr.headersCount = 3;
	nlhr.headers=(NETLIBHTTPHEADER*)mir_alloc(sizeof(NETLIBHTTPHEADER)*(nlhr.headersCount+5));
	nlhr.headers[0].szName   = "User-Agent";
	nlhr.headers[0].szValue = "Mozilla/4.0 (compatible; MSIE 5.5)";
	nlhr.headers[1].szName  = "Cache-Control";
	nlhr.headers[1].szValue = "no-cache";
	nlhr.headers[2].szName  = "Connection";
	nlhr.headers[2].szValue = "close"; /*"Keep-Alive";*/
	
	nlhrReply=(NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION,(WPARAM)g_hNetlibUser,(LPARAM)&nlhr);
	if(nlhrReply) {
		
		if (nlhrReply->resultCode == 200 && nlhrReply->pData != NULL) {
			result = strdup(nlhrReply->pData);
		} else {
			LOG(("[ext_send_https_request] Got result code: %d, content length: %d",  nlhrReply->resultCode, nlhrReply->dataLength));
		}
		
		LOG(("Got %d headers!", nlhrReply->headersCount));
		
		for (i=0; i < nlhrReply->headersCount; i++) {
			//LOG(("%s: %s", nlhrReply->headers[i].szName, nlhrReply->headers[i].szValue));
			
			if (lstrcmpiA(nlhrReply->headers[i].szName, "Set-Cookie") == 0) {
				//LOG(("Found Cookie... Yum yum..."));
				
				if (nlhrReply->headers[i].szValue[0] == 'B' && nlhrReply->headers[i].szValue[1] == '=') {
						
					FREE(yd->cookie_b);
					yd->cookie_b = getcookie(nlhrReply->headers[i].szValue);
					
					LOG(("Got B Cookie: %s", yd->cookie_b));
				}
			}
		}
		
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT,0,(LPARAM)nlhrReply);
	} else {
		LOG(("No Response???"));
	}
	
	mir_free(nlhr.headers);

	return result;
}

void CYahooProto::ext_login(enum yahoo_status login_mode)
{
	char host[128], fthost[128], login_host[128];
	int port=0;
	DBVARIANT dbv;
#ifdef HTTP_GATEWAY				
	NETLIBUSERSETTINGS nlus = { 0 };
#endif

	LOG(("[ext_login]"));

	host[0] = '\0';
	
	/**
	 * Implementing Yahoo 9 2 Stage Login using their VIP server/services
	 */
	NETLIBHTTPREQUEST nlhr={0},*nlhrReply;
	char z[4096];

	wsprintfA(z, "http://%s%s", GetByte("YahooJapan",0) != 0 ? "cs1.msg.vip.ogk.yahoo.co.jp" : "vcs.msg.yahoo.com", "/capacity");
	nlhr.cbSize		= sizeof(nlhr);
	nlhr.requestType= REQUEST_GET;
	nlhr.flags		= NLHRF_HTTP11;
	nlhr.szUrl		= z;

	nlhrReply=(NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION,(WPARAM)g_hNetlibUser,(LPARAM)&nlhr);
	if(nlhrReply) {
		if (nlhrReply->resultCode == 200 && nlhrReply->pData != NULL) {
			char *c = strstr(nlhrReply->pData,"CS_IP_ADDRESS=");
			
			if (c != NULL) {
					char *t = c;
				
					while ( (*t) != '=') t++; /* scan until = */
					t++;
					
					while ( (*c) != '\0' && (*c) != '\r' && (*c) != '\n') c++;
					
					memcpy(host, t, c - t);
					host[c - t] = '\0';
					
					LOG(("Got Host: %s", host));
			}
		} else {
			LOG(( "Problem retrieving a response from VIP server." ));
		}
		
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT,0,(LPARAM)nlhrReply);
	} 
	
	if 	(host[0] == '\0') {
		if (!GetString(YAHOO_LOGINSERVER, &dbv)) {
			mir_snprintf(host, sizeof(host), "%s", dbv.pszVal);
			DBFreeVariant(&dbv);
		}
		else {
			snprintf(host, sizeof(host), "%s", 
							GetByte("YahooJapan",0) != 0 ? YAHOO_DEFAULT_JAPAN_LOGIN_SERVER :
															YAHOO_DEFAULT_LOGIN_SERVER
					);
		}
	}
	
	lstrcpynA(fthost,GetByte("YahooJapan",0)?"filetransfer.msg.yahoo.co.jp":"filetransfer.msg.yahoo.com" , sizeof(fthost));
	lstrcpynA(login_host,GetByte("YahooJapan",0)?"login.yahoo.co.jp":"login.yahoo.com" , sizeof(login_host));	
	port = DBGetContactSettingWord(NULL, m_szModuleName, YAHOO_LOGINPORT, YAHOO_DEFAULT_PORT);
	
#ifdef HTTP_GATEWAY			
	nlus.cbSize = sizeof( nlus );
	if (CallService(MS_NETLIB_GETUSERSETTINGS, (WPARAM) hNetlibUser, (LPARAM) &nlus) == 0) {
		LOG(("ERROR: Problem retrieving miranda network settings!!!"));
	}
	
	iHTTPGateway = (nlus.useProxy && nlus.proxyType == PROXYTYPE_HTTP) ? 1:0;
	LOG(("Proxy Type: %d HTTP Gateway: %d", nlus.proxyType, iHTTPGateway));
#endif

	m_id = yahoo_init_with_attributes(m_yahoo_id, m_password, m_pw_token,
		"pager_host", host,
		"pager_port", port,
		"filetransfer_host", fthost,
		"picture_checksum", GetDword("AvatarHash", -1),
#ifdef HTTP_GATEWAY			
		"web_messenger", iHTTPGateway,
#endif
		"login_host", login_host,
		NULL);
	
	m_status = YAHOO_STATUS_OFFLINE;
	yahoo_login(m_id, login_mode);

	if (m_id <= 0) {
		LOG(("Could not connect to Yahoo server.  Please verify that you are connected to the net and the pager host and port are correctly entered."));
		ShowError(Translate("Yahoo Login Error"), Translate("Could not connect to Yahoo server.  Please verify that you are connected to the net and the pager host and port are correctly entered."));
		return;
	}

	//rearm(&pingTimer, 600);
}

/////////////////////////////////////////////////////////////////////////////////////////
// stubs

CYahooProto* __fastcall getProtoById( int id )
{
	for ( int i=0; i < g_instances.getCount(); i++ )
		if ( g_instances[i]->m_id == id )
			return g_instances[i];

	return NULL;
}

unsigned int ext_yahoo_add_handler(int id, int fd, yahoo_input_condition cond, void *data)
{	
	CYahooProto* ppro = getProtoById( id ); 
	if ( ppro ) 
		return ppro->ext_add_handler(fd, cond, data);
	
	return 0;
}

void ext_yahoo_remove_handler(int id, unsigned int tag)
{
	GETPROTOBYID( id )->ext_remove_handler(tag); 
}

void ext_yahoo_status_changed(int id, const char *who, int protocol, int stat, const char *msg, int away, int idle, int mobile, int utf8)
{	
	GETPROTOBYID( id )->ext_status_changed(who, protocol, stat, msg, away, idle, mobile, utf8); 
}

void ext_yahoo_status_logon(int id, const char *who, int protocol, int stat, const char *msg, int away, int idle, int mobile, int cksum, int buddy_icon, long client_version, int utf8)
{	
	GETPROTOBYID( id )->ext_status_logon(who, protocol, stat, msg, away, idle, mobile, cksum, buddy_icon, client_version, utf8); 
}

void ext_yahoo_got_audible(int id, const char *me, const char *who, const char *aud, const char *msg, const char *aud_hash)
{	
	GETPROTOBYID( id )->ext_got_audible(me, who, aud, msg, aud_hash); 
}

void ext_yahoo_got_calendar(int id, const char *url, int type, const char *msg, int svc)
{	
	GETPROTOBYID( id )->ext_got_calendar(url, type, msg, svc); 
}

void ext_yahoo_got_stealth(int id, char *stealthlist)
{	
	GETPROTOBYID( id )->ext_got_stealth(stealthlist); 
}

void ext_yahoo_got_buddies(int id, YList * buds)
{	
	GETPROTOBYID( id )->ext_got_buddies(buds); 
}

void ext_yahoo_rejected(int id, const char *who, const char *msg)
{	
	GETPROTOBYID( id )->ext_rejected(who, msg); 
}

void ext_yahoo_buddy_added(int id, char *myid, char *who, char *group, int status, int auth)
{	
	GETPROTOBYID( id )->ext_buddy_added(myid, who, group, status, auth); 
}

void ext_yahoo_buddy_group_changed(int id, char *myid, char *who, char *old_group, char *new_group)
{	
	LOG(("[ext_yahoo_buddy_group_changed] %s has been moved from group: %s to: %s", who, old_group, new_group));
}

void ext_yahoo_contact_added(int id, const char *myid, const char *who, const char *fname, const char *lname, const char *msg, int protocol)
{	
	GETPROTOBYID( id )->ext_contact_added(myid, who, fname, lname, msg, protocol); 
}

void ext_yahoo_typing_notify(int id, const char *me, const char *who, int protocol, int stat)
{	
	GETPROTOBYID( id )->ext_typing_notify(me, who, protocol, stat); 
}

void ext_yahoo_game_notify(int id, const char *me, const char *who, int stat, const char *msg)
{	
	GETPROTOBYID( id )->ext_game_notify(me, who, stat, msg); 
}

void ext_yahoo_mail_notify(int id, const char *from, const char *subj, int cnt)
{	
	GETPROTOBYID( id )->ext_mail_notify(from, subj, cnt); 
}

void ext_yahoo_system_message(int id, const char *me, const char *who, const char *msg)
{	
	GETPROTOBYID( id )->ext_system_message(me, who, msg); 
}

void ext_yahoo_got_identities(int id, const char *nick, const char *fname, const char *lname, YList * ids)
{	
	GETPROTOBYID( id )->ext_got_identities(nick, fname, lname, ids); 
}

void ext_yahoo_got_ping(int id, const char *errormsg)
{	
	GETPROTOBYID( id )->ext_got_ping(errormsg); 
}

void ext_yahoo_error(int id, const char *err, int fatal, int num)
{	
	GETPROTOBYID( id )->ext_error(err, fatal, num); 
}

void ext_yahoo_login_response(int id, int succ, const char *url)
{	
	GETPROTOBYID( id )->ext_login_response(succ, url); 
}

void ext_yahoo_got_im(int id, const char *me, const char *who, int protocol, const char *msg, long tm, int stat, int utf8, int buddy_icon, const char *seqn, int sendn)
{	
	GETPROTOBYID( id )->ext_got_im(me, who, protocol, msg, tm, stat, utf8, buddy_icon, seqn, sendn); 
}

void ext_yahoo_got_search_result(int id, int found, int start, int total, YList *contacts)
{	
	GETPROTOBYID( id )->ext_got_search_result(found, start, total, contacts); 
}

void ext_yahoo_got_picture(int id, const char *me, const char *who, const char *pic_url, int cksum, int type)
{	
	GETPROTOBYID( id )->ext_got_picture(me, who, pic_url, cksum, type); 
}

void ext_yahoo_got_picture_checksum(int id, const char *me, const char *who, int cksum)
{	
	GETPROTOBYID( id )->ext_got_picture_checksum(me, who, cksum); 
}

void ext_yahoo_got_picture_update(int id, const char *me, const char *who, int buddy_icon)
{	
	GETPROTOBYID( id )->ext_got_picture_update(me, who, buddy_icon); 
}

void ext_yahoo_got_picture_status(int id, const char *me, const char *who, int buddy_icon)
{	
	GETPROTOBYID( id )->ext_got_picture_status(me, who, buddy_icon); 
}

void ext_yahoo_got_picture_upload(int id, const char *me, const char *url,unsigned int ts)
{	
	GETPROTOBYID( id )->ext_got_picture_upload(me, url, ts); 
}

void ext_yahoo_got_avatar_share(int id, int buddy_icon)
{	
	GETPROTOBYID( id )->ext_got_avatar_share(buddy_icon); 
}

void ext_yahoo_got_file(int id, const char *me, const char *who, const char *url, long expires, const char *msg, const char *fname, unsigned long fesize, const char *ft_token, int y7)
{	
	GETPROTOBYID( id )->ext_got_file(me, who, url, expires, msg, fname, fesize, ft_token, y7); 
}

void ext_yahoo_got_files(int id, const char *me, const char *who, const char *ft_token, int y7, YList* files)
{	
	GETPROTOBYID( id )->ext_got_files(me, who, ft_token, y7, files); 
}

void ext_yahoo_got_file7info(int id, const char *me, const char *who, const char *url, const char *fname, const char *ft_token)
{	
	GETPROTOBYID( id )->ext_got_file7info(me, who, url, fname, ft_token); 
}

void ext_yahoo_ft7_send_file(int id, const char *me, const char *who, const char *filename, const char *token, const char *ft_token)
{	
	GETPROTOBYID( id )->ext_ft7_send_file(me, who, filename, token, ft_token); 
}

int ext_yahoo_connect_async(int id, const char *host, int port, int type, yahoo_connect_callback callback, void *data)
{	
	CYahooProto* ppro = getProtoById( id ); 
	if ( ppro ) 
		return ppro->ext_connect_async(host, port, type, callback, data);
	return SOCKET_ERROR;
}

void ext_yahoo_send_http_request(int id, enum yahoo_connection_type type, const char *method, const char *url, const char *cookies, long content_length, yahoo_get_fd_callback callback, void *callback_data)
{	
	GETPROTOBYID( id )->ext_send_http_request(type, method, url, cookies, content_length, callback, callback_data); 
}

char *ext_yahoo_send_https_request(struct yahoo_data *yd, const char *host, const char *path)
{
	CYahooProto* ppro = getProtoById( yd->client_id ); 
	if ( ppro ) 
		return ppro->ext_send_https_request(yd, host, path);
	return NULL;
}

void ext_yahoo_got_ignore(int id, YList * igns)
{	
	GETPROTOBYID( id )->ext_got_ignore( igns );
}

void register_callbacks()
{
	static struct yahoo_callbacks yc;

	yc.ext_yahoo_login_response = ext_yahoo_login_response;
	yc.ext_yahoo_got_buddies = ext_yahoo_got_buddies;
	yc.ext_yahoo_got_ignore = ext_yahoo_got_ignore;
	yc.ext_yahoo_got_identities = ext_yahoo_got_identities;
	yc.ext_yahoo_got_cookies = ext_yahoo_got_cookies;
	yc.ext_yahoo_status_changed = ext_yahoo_status_changed;
	yc.ext_yahoo_status_logon = ext_yahoo_status_logon;
	yc.ext_yahoo_got_im = ext_yahoo_got_im;
	yc.ext_yahoo_got_conf_invite = ext_yahoo_got_conf_invite;
	yc.ext_yahoo_conf_userdecline = ext_yahoo_conf_userdecline;
	yc.ext_yahoo_conf_userjoin = ext_yahoo_conf_userjoin;
	yc.ext_yahoo_conf_userleave = ext_yahoo_conf_userleave;
	yc.ext_yahoo_conf_message = ext_yahoo_conf_message;
	yc.ext_yahoo_chat_cat_xml = ext_yahoo_chat_cat_xml;
	yc.ext_yahoo_chat_join = ext_yahoo_chat_join;
	yc.ext_yahoo_chat_userjoin = ext_yahoo_chat_userjoin;
	yc.ext_yahoo_chat_userleave = ext_yahoo_chat_userleave;
	yc.ext_yahoo_chat_message = ext_yahoo_chat_message;
	yc.ext_yahoo_chat_yahoologout = ext_yahoo_chat_yahoologout;
	yc.ext_yahoo_chat_yahooerror = ext_yahoo_chat_yahooerror;
	yc.ext_yahoo_got_webcam_image = ext_yahoo_got_webcam_image;
	yc.ext_yahoo_webcam_invite = ext_yahoo_webcam_invite;
	yc.ext_yahoo_webcam_invite_reply = ext_yahoo_webcam_invite_reply;
	yc.ext_yahoo_webcam_closed = ext_yahoo_webcam_closed;
	yc.ext_yahoo_webcam_viewer = ext_yahoo_webcam_viewer;
	yc.ext_yahoo_webcam_data_request = ext_yahoo_webcam_data_request;
	yc.ext_yahoo_got_file = ext_yahoo_got_file;
	yc.ext_yahoo_got_files = ext_yahoo_got_files;
	yc.ext_yahoo_got_file7info = ext_yahoo_got_file7info;
	yc.ext_yahoo_send_file7info = ext_yahoo_send_file7info;
	yc.ext_yahoo_ft7_send_file = ext_yahoo_ft7_send_file;
	yc.ext_yahoo_contact_added = ext_yahoo_contact_added;
	yc.ext_yahoo_rejected = ext_yahoo_rejected;
	yc.ext_yahoo_typing_notify = ext_yahoo_typing_notify;
	yc.ext_yahoo_game_notify = ext_yahoo_game_notify;
	yc.ext_yahoo_mail_notify = ext_yahoo_mail_notify;
	yc.ext_yahoo_got_search_result = ext_yahoo_got_search_result;
	yc.ext_yahoo_system_message = ext_yahoo_system_message;
	yc.ext_yahoo_error = ext_yahoo_error;
	yc.ext_yahoo_log = YAHOO_DEBUGLOG;
	yc.ext_yahoo_add_handler = ext_yahoo_add_handler;
	yc.ext_yahoo_remove_handler = ext_yahoo_remove_handler;
	//yc.ext_yahoo_connect = ext_yahoo_connect; not needed in fact
	yc.ext_yahoo_connect_async = ext_yahoo_connect_async;
	yc.ext_yahoo_send_http_request = ext_yahoo_send_http_request;

	yc.ext_yahoo_got_stealthlist = ext_yahoo_got_stealth;
	yc.ext_yahoo_got_ping  = ext_yahoo_got_ping;
	yc.ext_yahoo_got_picture  = ext_yahoo_got_picture;
	yc.ext_yahoo_got_picture_checksum = ext_yahoo_got_picture_checksum;
	yc.ext_yahoo_got_picture_update = ext_yahoo_got_picture_update;
	yc.ext_yahoo_got_avatar_share = ext_yahoo_got_avatar_share;
	
	yc.ext_yahoo_buddy_added = ext_yahoo_buddy_added;
	yc.ext_yahoo_got_picture_upload = ext_yahoo_got_picture_upload;
	yc.ext_yahoo_got_picture_status = ext_yahoo_got_picture_status;
	yc.ext_yahoo_got_audible = ext_yahoo_got_audible;
	yc.ext_yahoo_got_calendar = ext_yahoo_got_calendar;
	yc.ext_yahoo_buddy_group_changed = ext_yahoo_buddy_group_changed;
	
	yc.ext_yahoo_send_https_request = ext_yahoo_send_https_request;
	
	yahoo_register_callbacks(&yc);
}
