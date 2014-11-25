/*
 * libyahoo2: libyahoo2.c
 *
 * Some code copyright (C) 2002-2004, Philip S Tellis <philip.tellis AT gmx.net>
 *
 * Yahoo Search copyright (C) 2003, Konstantin Klyagin <konst AT konst.org.ua>
 *
 * Much of this code was taken and adapted from the yahoo module for
 * gaim released under the GNU GPL.  This code is also released under the 
 * GNU GPL.
 *
 * This code is derivitive of Gaim <http://gaim.sourceforge.net>
 * copyright (C) 1998-1999, Mark Spencer <markster@marko.net>
 *	       1998-1999, Adam Fritzler <afritz@marko.net>
 *	       1998-2002, Rob Flynn <rob@marko.net>
 *	       2000-2002, Eric Warmenhoven <eric@warmenhoven.org>
 *	       2001-2002, Brian Macke <macke@strangelove.net>
 *		    2001, Anand Biligiri S <abiligiri@users.sf.net>
 *		    2001, Valdis Kletnieks
 *		    2002, Sean Egan <bj91704@binghamton.edu>
 *		    2002, Toby Gray <toby.gray@ntlworld.com>
 *
 * This library also uses code from other libraries, namely:
 *     Portions from libfaim copyright 1998, 1999 Adam Fritzler
 *     <afritz@auk.cx>
 *     Portions of Sylpheed copyright 2000-2002 Hiroyuki Yamamoto
 *     <hiro-y@kcn.ne.jp>
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#ifndef _WIN32
# include <unistd.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <stdarg.h>

#if STDC_HEADERS
# include <string.h>
#else
# if !HAVE_STRCHR
#  define strchr index
#  define strrchr rindex
# endif
char *strchr (), *strrchr ();
# if !HAVE_MEMCPY
#  define memcpy(d, s, n) bcopy ((s), (d), (n))
#  define memmove(d, s, n) bcopy ((s), (d), (n))
# endif
#endif

#include <sys/types.h>

#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <malloc.h>

#include "yahoo2.h"
#include "yahoo_httplib.h"
#include "yahoo_util.h"

#include "yahoo2_callbacks.h"
#include "yahoo_debug.h"

#ifdef USE_STRUCT_CALLBACKS
struct yahoo_callbacks *yc=NULL;

void yahoo_register_callbacks(struct yahoo_callbacks * tyc)
{
	yc = tyc;
}

#define YAHOO_CALLBACK(x)	yc->x
#else
#define YAHOO_CALLBACK(x)	x
#endif

struct yahoo_pair {
	int key;
	char *value;
};

struct yahoo_packet {
	unsigned short int service;
	int status;
	unsigned int id;
	YList *hash;
};

struct yahoo_search_state {
	int   lsearch_type;
	char  *lsearch_text;
	int   lsearch_gender;
	int   lsearch_agerange;
	int   lsearch_photo;
	int   lsearch_yahoo_only;
	int   lsearch_nstart;
	int   lsearch_nfound;
	int   lsearch_ntotal;
};

struct data_queue {
	unsigned char *queue;
	int len;
};

struct yahoo_input_data {
	struct yahoo_data *yd;
	struct yahoo_webcam *wcm;
	struct yahoo_webcam_data *wcd;
	struct yahoo_search_state *ys;

	int   fd;
	enum yahoo_connection_type type;
	
	unsigned char	*rxqueue;
	int   rxlen;
	int   read_tag;

	YList *txqueues;
	int   write_tag;
};

/* default values for servers */
static const char pager_host[] = "scs.msg.yahoo.com";
static const int pager_port = 5050;
static const int fallback_ports[]={80, 23, 25, 20, 119, 8001, 8002, 5050, 0};
static const char filetransfer_host[]="filetransfer.msg.yahoo.com";
static const int filetransfer_port=80;
static const char webcam_host[]="webcam.yahoo.com";
static const int webcam_port=5100;
static const char webcam_description[]="";
static char local_host[]="";
static int conn_type=Y_WCM_DSL;
static const char login_host[]="login.yahoo.com";
static char profile_url[] = "http://profiles.yahoo.com/";

typedef struct {
	int key;
	char *name;
}value_string;

static int yahoo_send_data(int fd, const char *data, int len);

int yahoo_log_message(char * fmt, ...)
{
	char out[1024];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(out, sizeof(out), fmt, ap);
	va_end(ap);
	return YAHOO_CALLBACK(ext_yahoo_log)("%s", out);
}

int yahoo_connect(char * host, int port, int type)
{
	return YAHOO_CALLBACK(ext_yahoo_connect)(host, port, type);
}

static enum yahoo_log_level log_level = YAHOO_LOG_NONE;

enum yahoo_log_level yahoo_get_log_level()
{
	return log_level;
}

int yahoo_set_log_level(enum yahoo_log_level level)
{
	enum yahoo_log_level l = log_level;
	log_level = level;
	return l;
}

static const value_string ymsg_service_vals[] = {
	{YAHOO_SERVICE_LOGON, "Pager Logon"},
	{YAHOO_SERVICE_LOGOFF, "Pager Logoff"},
	{YAHOO_SERVICE_ISAWAY, "Is Away"},
	{YAHOO_SERVICE_ISBACK, "Is Back"},
	{YAHOO_SERVICE_IDLE, "Idle"},
	{YAHOO_SERVICE_MESSAGE, "Message"},
	{YAHOO_SERVICE_IDACT, "Activate Identity"},
	{YAHOO_SERVICE_IDDEACT, "Deactivate Identity"},
	{YAHOO_SERVICE_MAILSTAT, "Mail Status"},
	{YAHOO_SERVICE_USERSTAT, "User Status"},
	{YAHOO_SERVICE_NEWMAIL, "New Mail"},
	{YAHOO_SERVICE_CHATINVITE, "Chat Invitation"},
	{YAHOO_SERVICE_CALENDAR, "Calendar Reminder"},
	{YAHOO_SERVICE_NEWPERSONALMAIL, "New Personals Mail"},
	{YAHOO_SERVICE_NEWCONTACT, "New Friend"},
	{YAHOO_SERVICE_ADDIDENT, "Add Identity"},
	{YAHOO_SERVICE_ADDIGNORE, "Add Ignore"},
	{YAHOO_SERVICE_PING, "Ping"},
	{YAHOO_SERVICE_GOTGROUPRENAME, "Got Group Rename"},
	{YAHOO_SERVICE_SYSMESSAGE, "System Message"},
	{YAHOO_SERVICE_SKINNAME, "YAHOO_SERVICE_SKINNAME"},
	{YAHOO_SERVICE_PASSTHROUGH2, "Passthrough 2"},
	{YAHOO_SERVICE_CONFINVITE, "Conference Invitation"},
	{YAHOO_SERVICE_CONFLOGON, "Conference Logon"},
	{YAHOO_SERVICE_CONFDECLINE, "Conference Decline"},
	{YAHOO_SERVICE_CONFLOGOFF, "Conference Logoff"},
	{YAHOO_SERVICE_CONFADDINVITE, "Conference Additional Invitation"},
	{YAHOO_SERVICE_CONFMSG, "Conference Message"},
	{YAHOO_SERVICE_CHATLOGON, "Chat Logon"},
	{YAHOO_SERVICE_CHATLOGOFF, "Chat Logoff"},
	{YAHOO_SERVICE_CHATMSG, "Chat Message"},
	{YAHOO_SERVICE_GAMELOGON, "Game Logon"},
	{YAHOO_SERVICE_GAMELOGOFF, "Game Logoff"},
	{YAHOO_SERVICE_GAMEMSG, "Game Message"},
	{YAHOO_SERVICE_FILETRANSFER, "File Transfer"},
	{YAHOO_SERVICE_VOICECHAT, "Voice Chat"},
	{YAHOO_SERVICE_NOTIFY, "Notify"},
	{YAHOO_SERVICE_VERIFY, "Verify"},
	{YAHOO_SERVICE_P2PFILEXFER, "P2P File Transfer"}, 
	{YAHOO_SERVICE_PEERTOPEER, "Peer To Peer"},
	{YAHOO_SERVICE_WEBCAM, "WebCam"},
	{YAHOO_SERVICE_AUTHRESP, "Authentication Response"},
	{YAHOO_SERVICE_LIST, "List"},
	{YAHOO_SERVICE_AUTH, "Authentication"},
	{YAHOO_SERVICE_ADDBUDDY, "Add Buddy"},
	{YAHOO_SERVICE_REMBUDDY, "Remove Buddy"},
	{YAHOO_SERVICE_IGNORECONTACT, "Ignore Contact"},
	{YAHOO_SERVICE_REJECTCONTACT, "Reject Contact"},
	{YAHOO_SERVICE_GROUPRENAME, "Group Rename"},
	{YAHOO_SERVICE_KEEPALIVE, "Keep Alive"},
	{YAHOO_SERVICE_CHATONLINE, "Chat Online"},
	{YAHOO_SERVICE_CHATGOTO, "Chat Goto"},
	{YAHOO_SERVICE_CHATJOIN, "Chat Join"},
	{YAHOO_SERVICE_CHATLEAVE, "Chat Leave"},
	{YAHOO_SERVICE_CHATEXIT, "Chat Exit"},
	{YAHOO_SERVICE_CHATADDINVITE, "Chat Invite"},
	{YAHOO_SERVICE_CHATLOGOUT, "Chat Logout"},
	{YAHOO_SERVICE_CHATPING, "Chat Ping"},
	{YAHOO_SERVICE_COMMENT, "Comment"},
	{YAHOO_SERVICE_GAME_INVITE,"Game Invite"},
	{YAHOO_SERVICE_STEALTH_PERM, "Stealth Permanent"},
	{YAHOO_SERVICE_STEALTH_SESSION, "Stealth Session"},
	{YAHOO_SERVICE_AVATAR,"Avatar"},
	{YAHOO_SERVICE_PICTURE_CHECKSUM,"Picture Checksum"},
	{YAHOO_SERVICE_PICTURE,"Picture"},
	{YAHOO_SERVICE_PICTURE_UPDATE,"Picture Update"},
	{YAHOO_SERVICE_PICTURE_UPLOAD,"Picture Upload"},
	{YAHOO_SERVICE_YAB_UPDATE,"Yahoo Address Book Update"},
	{YAHOO_SERVICE_Y6_VISIBLE_TOGGLE, "Y6 Visibility Toggle"},
	{YAHOO_SERVICE_Y6_STATUS_UPDATE, "Y6 Status Update"},
	{YAHOO_SERVICE_PICTURE_SHARING, "Picture Sharing Status"},
	{YAHOO_SERVICE_VERIFY_ID_EXISTS, "Verify ID Exists"},
	{YAHOO_SERVICE_AUDIBLE, "Audible"},
	{YAHOO_SERVICE_Y7_CONTACT_DETAILS,"Y7 Contact Details"},
	{YAHOO_SERVICE_Y7_CHAT_SESSION,	"Y7 Chat Session"},
	{YAHOO_SERVICE_Y7_AUTHORIZATION,"Y7 Buddy Authorization"},
	{YAHOO_SERVICE_Y7_FILETRANSFER,"Y7 File Transfer"},
	{YAHOO_SERVICE_Y7_FILETRANSFERINFO,"Y7 File Transfer Information"},
	{YAHOO_SERVICE_Y7_FILETRANSFERACCEPT,"Y7 File Transfer Accept"},
	{YAHOO_SERVICE_Y7_MINGLE, "Y7 360 Mingle"},
	{YAHOO_SERVICE_Y7_CHANGE_GROUP, "Y7 Change Group"},
	{YAHOO_SERVICE_Y8_STATUS_UPDATE, "Y8 Buddy Status Update"},
	{YAHOO_SERVICE_Y8_LIST, "Y8 Buddy List"},
	{YAHOO_SERVICE_Y9_MESSAGE_ACK, "Y9 Message Ack"},
	{YAHOO_SERVICE_Y9_PINGBOX_LIST, "Y9 Pingbox List"},
	{YAHOO_SERVICE_Y9_PINGBOX_GUEST_STATUS, "Y9 Pingbox Guest Status"},
	{YAHOO_SERVICE_Y9_PINGBOX_NA, "Y9 Pingbox ???"},
	{YAHOO_SERVICE_WEBLOGIN, "Web Login"},
	{YAHOO_SERVICE_SMS_MSG, "SMS Message"},
	{YAHOO_SERVICE_Y7_DISCONNECTED, "Y7 Disconnected"},
	{0, NULL}
};

static const value_string ymsg_status_vals[] = {
	{YPACKET_STATUS_DISCONNECTED,"Disconnected"},
	{YPACKET_STATUS_DEFAULT,""},
	{YPACKET_STATUS_SERVERACK,"Server Ack"},
	{YPACKET_STATUS_GAME,"Playing Game"},
	{YPACKET_STATUS_AWAY, "Away"},
	{YPACKET_STATUS_CONTINUED,"More Packets??"},
	{YPACKET_STATUS_NOTIFY, "Notify"},
	{YPACKET_STATUS_WEBLOGIN,"Web Login"},
	{YPACKET_STATUS_OFFLINE,"Offline"},
	{0, NULL}
};

static const value_string packet_keys[]={
	{  0, "identity" },
	{  1, "ID" },
	{  2, "id?" },
	{  3, "my id"},
	{  4, "ID/Nick"},
	{  5, "To"},
	{  6, "auth token 1"},
	{  7, "Buddy" },
	{  8, "# buddies"}, 
	{  9, "# mails"},
	{ 10, "state"},
	{ 11, "session"},
	{ 12, "reverse ip? [gaim]"},
	{ 13, "stat/location"}, // bitnask: 0 = pager, 1 = chat, 2 = game
	{ 14, "ind/msg"},
	{ 15, "time"},
	{ 16, "Error msg"},
	{ 17, "chat"},
	{ 18, "subject/topic?"},
	{ 19, "custom msg"},
	{ 20, "url"},
	{ 24, "session timestamp"},
	{ 27, "filename"},
	{ 28, "filesize"},
	{ 31, "visibility?"},
	{ 38, "expires"},
	{ 42, "email"},
	{ 43, "email who"},
	{ 47, "away"},
	{ 49, "service"},
	{ 50, "conf host"},
	{ 52, "conf invite"},
	{ 53, "conf logon"},
	{ 54, "conf decline"},
	{ 55, "conf unavail"},
	{ 56, "conf logoff"},
	{ 57, "conf room"},
	{ 58, "conf joinmsg"},
	{ 59, "cookies"},
	{ 60, "SMS/Mobile"},
	{ 61, "Cookie?"},
	{ 63, "imvironment name;num"},
	{ 64, "imvironment enabled/avail"},
	{ 65, "group"},
	{ 66, "login status"},
	{ 73, "user name"},
	{ 87, "buds/groups"},
	{ 88, "ignore list"},
	{ 89, "identities"},
	{ 91, "pingbox nick"},
	{ 92, "pingbox id"},
	{ 94, "auth seed"},
	{ 96, "auth token 2"},
	{ 97, "utf8"},
	{104, "room name"},
	{105, "chat topic"},
	{108, "chat nbuddies"},
	{109, "chat from"},
	{110, "chat age"},
	{113, "chat attrs"},
	{117, "chat msg"},
	{124, "chat msg type"},
	{128, "chat room category?"},
	{129, "chat room serial 2"},
	{130, "first join/chat room cookie"},
	{135, "YIM version"},
	{137, "idle time"},
	{138, "idle?"},
	{142, "chat location"},
	{143, "ping interval (mins)"},
	{144, "keep-alive interval (mins)"},
	{185, "stealth/hide?"},
	{192, "Pictures/Buddy Icons"},
	{197, "Avatars"},
	{203, "YAB data?"},
	{206, "display image type"},
	{213, "share avatar type"},
	{216, "first name"},
	{219, "cookie separator?"},
	{222, "FT7 Service"},
	{223, "authorized?"},
	{230, "the audible, in foo.bar.baz format"},
	{231, "audible text"},
	{232, "weird number (md5 hash?) [audible]"},
	{241, "protocol"},
	{244, "client version"},
	{249, "FT7 Op"},
	{250, "FT7 Relay Host"},
	{251, "File Preview?"},
	{254, "last name"},
	{265, "FT7 Token"},
	{266, "FT7 # Files"},
	{267, "FT7 Preview"},
	{317, "Stealth"},
	{430, "Seq #"},
	{450, "Retry"},
	{1002, "YIM6+"},
	{10093, "YIM7 (sets it to 4)"},
	{10097, "Region (SMS?)"},
	{ -1, "" }
};

const char *dbg_key(int key) 
{
	int i=0;
	
	while ((packet_keys[i].key >=0) && (packet_keys[i].key != key))
		i++;
	
	if (packet_keys[i].key != key)
			return NULL;
	else
			return packet_keys[i].name;
}

const char *dbg_service(int key) 
{
	int i=0;
	
	while ((ymsg_service_vals[i].key > 0) && (ymsg_service_vals[i].key != key)) 
		i++;
	
	if (ymsg_service_vals[i].key != key)
			return NULL;
	else
			return ymsg_service_vals[i].name;
}

const char *dbg_status(int key) 
{
	int i;
	
	for (i = 0; ymsg_status_vals[i].name != NULL; i++ ) {
		if (ymsg_status_vals[i].key == key) 
			return ymsg_status_vals[i].name;
	}
	
	return NULL;
}

static struct yahoo_server_settings* _yahoo_default_server_settings()
{
	struct yahoo_server_settings *yss = y_new0(struct yahoo_server_settings, 1);

	yss->pager_host = strdup(pager_host);
	yss->pager_port = pager_port;
	yss->filetransfer_host = strdup(filetransfer_host);
	yss->filetransfer_port = filetransfer_port;
	yss->webcam_host = strdup(webcam_host);
	yss->webcam_port = webcam_port;
	yss->webcam_description = strdup(webcam_description);
	yss->local_host = strdup(local_host);
	yss->conn_type = conn_type;
	yss->pic_cksum = -1;
	yss->login_host = strdup(login_host);
	
	return yss;
}

static struct yahoo_server_settings * _yahoo_assign_server_settings(va_list ap)
{
	struct yahoo_server_settings *yss = _yahoo_default_server_settings();
	char *key;
	char *svalue;
	int   nvalue;

	while(1) {
		key = va_arg(ap, char *);
		if (key == NULL)
			break;

		if (!strcmp(key, "pager_host")) {
			svalue = va_arg(ap, char *);
			free(yss->pager_host);
			yss->pager_host = strdup(svalue);
		} else if (!strcmp(key, "pager_port")) {
			nvalue = va_arg(ap, int);
			yss->pager_port = nvalue;
		} else if (!strcmp(key, "filetransfer_host")) {
			svalue = va_arg(ap, char *);
			free(yss->filetransfer_host);
			yss->filetransfer_host = strdup(svalue);
		} else if (!strcmp(key, "filetransfer_port")) {
			nvalue = va_arg(ap, int);
			yss->filetransfer_port = nvalue;
		} else if (!strcmp(key, "webcam_host")) {
			svalue = va_arg(ap, char *);
			free(yss->webcam_host);
			yss->webcam_host = strdup(svalue);
		} else if (!strcmp(key, "webcam_port")) {
			nvalue = va_arg(ap, int);
			yss->webcam_port = nvalue;
		} else if (!strcmp(key, "webcam_description")) {
			svalue = va_arg(ap, char *);
			free(yss->webcam_description);
			yss->webcam_description = strdup(svalue);
		} else if (!strcmp(key, "local_host")) {
			svalue = va_arg(ap, char *);
			free(yss->local_host);
			yss->local_host = strdup(svalue);
		} else if (!strcmp(key, "conn_type")) {
			nvalue = va_arg(ap, int);
			yss->conn_type = nvalue;
		} else if (!strcmp(key, "picture_checksum")) {
			nvalue = va_arg(ap, int);
			yss->pic_cksum = nvalue;
		} else if (!strcmp(key, "web_messenger")) {
			nvalue = va_arg(ap, int);
			yss->web_messenger = nvalue;
		} else if (!strcmp(key, "login_host")) {
			svalue = va_arg(ap, char *);
			free(yss->login_host);
			yss->login_host = strdup(svalue);
		} else {
			WARNING(("Unknown key passed to yahoo_init, "
				"perhaps you didn't terminate the list "
				"with NULL"));
		}
	}

	return yss;
}

static void yahoo_free_server_settings(struct yahoo_server_settings *yss)
{
	if (!yss)
		return;

	free(yss->pager_host);
	free(yss->filetransfer_host);
	free(yss->webcam_host);
	free(yss->webcam_description);
	free(yss->local_host);
	free(yss->login_host);
	free(yss);
}

static YList *conns=NULL;
static YList *inputs=NULL;
static int last_id=0;

static void add_to_list(struct yahoo_data *yd)
{
	conns = y_list_prepend(conns, yd);
}
static struct yahoo_data * find_conn_by_id(int id)
{
	YList *l;
	for (l = conns; l; l = y_list_next(l)) {
		struct yahoo_data *yd = (struct yahoo_data *) l->data;
		if (yd->client_id == id)
			return yd;
	}
	return NULL;
}
static void del_from_list(struct yahoo_data *yd)
{
	conns = y_list_remove(conns, yd);
}

/* call repeatedly to get the next one */
/*
static struct yahoo_input_data * find_input_by_id(int id)
{
	YList *l;
	for (l = inputs; l; l = y_list_next(l)) {
		struct yahoo_input_data *yid = l->data;
		if (yid->yd->client_id == id)
			return yid;
	}
	return NULL;
}
*/

static struct yahoo_input_data * find_input_by_id_and_webcam_user(int id, const char * who)
{
	YList *l;
	LOG(("find_input_by_id_and_webcam_user"));
	for (l = inputs; l; l = y_list_next(l)) {
		struct yahoo_input_data *yid = (struct yahoo_input_data *) l->data;
		if (yid->type == YAHOO_CONNECTION_WEBCAM && yid->yd->client_id == id 
				&& yid->wcm && 
				((who && yid->wcm->user && !strcmp(who, yid->wcm->user)) ||
				 !(yid->wcm->user && !who)))
			return yid;
	}
	return NULL;
}

static struct yahoo_input_data * find_input_by_id_and_type(int id, enum yahoo_connection_type type)
{
	YList *l;
	
	//LOG(("[find_input_by_id_and_type] id: %d, type: %d", id, type));
	for (l = inputs; l; l = y_list_next(l)) {
		struct yahoo_input_data *yid = (struct yahoo_input_data *)l->data;
		if (yid->type == type && yid->yd->client_id == id) {
			//LOG(("[find_input_by_id_and_type] Got it!!!"));
			return yid;
		}
	}
	return NULL;
}

static struct yahoo_input_data * find_input_by_id_and_fd(int id, int fd)
{
	YList *l;
	LOG(("find_input_by_id_and_fd"));
	for (l = inputs; l; l = y_list_next(l)) {
		struct yahoo_input_data *yid = (struct yahoo_input_data *) l->data;
		
		if (yid->fd == fd && yid->yd->client_id == id)
			return yid;
	}
	return NULL;
}

static int count_inputs_with_id(int id)
{
	int c=0;
	YList *l;
	LOG(("counting %d", id));
	for (l = inputs; l; l = y_list_next(l)) {
		struct yahoo_input_data *yid = (struct yahoo_input_data *) l->data;
		if (yid->yd->client_id == id)
			c++;
	}
	LOG(("%d", c));
	return c;
}


extern char *yahoo_crypt(char *, char *);

/* Free a buddy list */
static void yahoo_free_buddies(YList * list)
{
	YList *l;

	for (l = list; l; l = l->next)
	{
		struct yahoo_buddy *bud = (struct yahoo_buddy *) l->data;
		if (!bud)
			continue;

		FREE(bud->group);
		FREE(bud->id);
		FREE(bud->real_name);
		if (bud->yab_entry) {
			FREE(bud->yab_entry->fname);
			FREE(bud->yab_entry->lname);
			FREE(bud->yab_entry->nname);
			FREE(bud->yab_entry->id);
			FREE(bud->yab_entry->email);
			FREE(bud->yab_entry->hphone);
			FREE(bud->yab_entry->wphone);
			FREE(bud->yab_entry->mphone);
			FREE(bud->yab_entry);
		}
		FREE(bud);
		l->data = bud = NULL;
	}

	y_list_free(list);
}

/* Free an identities list */
static void yahoo_free_identities(YList * list)
{
	while (list) {
		YList *n = list;
		FREE(list->data);
		list = y_list_remove_link(list, list);
		y_list_free_1(n);
	}
}

/* Free webcam data */
static void yahoo_free_webcam(struct yahoo_webcam *wcm)
{
	if (wcm) {
		FREE(wcm->user);
		FREE(wcm->server);
		FREE(wcm->key);
		FREE(wcm->description);
		FREE(wcm->my_ip);
	}
	FREE(wcm);
}

static void yahoo_free_data(struct yahoo_data *yd)
{
	FREE(yd->user);
	FREE(yd->password);
	FREE(yd->pw_token);
	FREE(yd->cookie_y);
	FREE(yd->cookie_t);
	FREE(yd->cookie_c);
	FREE(yd->cookie_b);
	FREE(yd->login_cookie);
	FREE(yd->login_id);
	FREE(yd->rawstealthlist);
	FREE(yd->ygrp);

	yahoo_free_buddies(yd->buddies);
	yahoo_free_buddies(yd->ignore);
	yahoo_free_identities(yd->identities);

	yahoo_free_server_settings(yd->server_settings);

	FREE(yd);
}

#define YAHOO_PACKET_HDRLEN (4 + 2 + 2 + 2 + 2 + 4 + 4)

static struct yahoo_packet *yahoo_packet_new(enum yahoo_service service, 
		enum ypacket_status status, int id)
{
	struct yahoo_packet *pkt = y_new0(struct yahoo_packet, 1);

	pkt->service = service;
	pkt->status = status;
	pkt->id = id;

	return pkt;
}

static void yahoo_packet_hash(struct yahoo_packet *pkt, int key, const char *value)
{
	struct yahoo_pair *pair = y_new0(struct yahoo_pair, 1);
	pair->key = key;
	pair->value = strdup(value);
	pkt->hash = y_list_append(pkt->hash, pair);
}

static void yahoo_packet_hash_int(struct yahoo_packet *pkt, int key, int value)
{
	char 	c[128];
	
	snprintf(c, 128, "%d", value);
	yahoo_packet_hash(pkt, key, c);
}



static int yahoo_packet_length(struct yahoo_packet *pkt)
{
	YList *l;

	int len = 0;

	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;
		int tmp = pair->key;
		do {
			tmp /= 10;
			len++;
		} while (tmp);
		len += 2;
		len += strlen(pair->value);
		len += 2;
	}

	return len;
}

#define yahoo_put16(buf, data) ( \
		(*(buf) = (unsigned char)((data)>>8)&0xff), \
		(*((buf)+1) = (unsigned char)(data)&0xff),  \
		2)
#define yahoo_get16(buf) ((((*(buf))&0xff)<<8) + ((*((buf)+1)) & 0xff))
#define yahoo_put32(buf, data) ( \
		(*((buf)) = (unsigned char)((data)>>24)&0xff), \
		(*((buf)+1) = (unsigned char)((data)>>16)&0xff), \
		(*((buf)+2) = (unsigned char)((data)>>8)&0xff), \
		(*((buf)+3) = (unsigned char)(data)&0xff), \
		4)
#define yahoo_get32(buf) ((((*(buf)   )&0xff)<<24) + \
			 (((*((buf)+1))&0xff)<<16) + \
			 (((*((buf)+2))&0xff)<< 8) + \
			 (((*((buf)+3))&0xff)))

static void yahoo_packet_read(struct yahoo_packet *pkt, unsigned char *data, int len)
{
	int pos = 0, zl;
	char z[100];
	
	snprintf(z, sizeof(z), "-=[ %s (0x%02x) ", dbg_service(pkt->service), pkt->service);
	
	if (pkt->status != 0)
		snprintf(z, sizeof(z), "%s, %s (%d)", z, dbg_status(pkt->status),pkt->status);

	if (len != 0)
		snprintf(z, sizeof(z), "%s Length: %d", z, len);
	
	snprintf(z, sizeof(z), "%s ]=-", z);
	
	zl = strlen(z);
	DEBUG_MSG1((z));

	while (pos + 1 < len) {
		char *key, *value = NULL;
		int accept;
		int x;

		struct yahoo_pair *pair = y_new0(struct yahoo_pair, 1);

		key = (char *) malloc(len + 1);
		x = 0;
		while (pos + 1 < len) {
			if (data[pos] == 0xc0 && data[pos + 1] == 0x80)
				break;
			key[x++] = data[pos++];
		}
		key[x] = 0;
		pos += 2;
		pair->key = strtol(key, NULL, 10);
		free(key);

		accept = x; 

		if (pos + 1 > len) {
			/* Malformed packet! (Truncated--garbage or something) */
			accept = 0;
		}
		
		/* if x is 0 there was no key, so don't accept it */
		if (accept)
			value = (char *) malloc(len - pos + 1);
		x = 0;
		while (pos + 1 < len) {
			if (data[pos] == 0xc0 && data[pos + 1] == 0x80)
				break;
			if (accept)
				value[x++] = data[pos++];
		}
		if (accept)
			value[x] = 0;
		pos += 2;
		if (accept) {
			pair->value = strdup(value);
			FREE(value);
			pkt->hash = y_list_append(pkt->hash, pair);
			
			DEBUG_MSG1(("Key: (%5d) %-25s Value: '%s'", pair->key, dbg_key(pair->key), pair->value));
		} else {
			FREE(pair);
		}
	}
	
	for (pos = 0; pos < zl; pos++) z[pos] = '-';
	z[pos] = '\0';
	DEBUG_MSG1((z));
}

static void yahoo_packet_write(struct yahoo_packet *pkt, unsigned char *data)
{
	YList *l;
	int pos = 0;

	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;
		unsigned char buf[100];

		snprintf((char *)buf, sizeof(buf), "%d", pair->key);
		strcpy((char *)data + pos, (char *)buf);
		pos += strlen((char *)buf);
		data[pos++] = 0xc0;
		data[pos++] = 0x80;

		strcpy((char *)data + pos, pair->value);
		pos += strlen(pair->value);
		data[pos++] = 0xc0;
		data[pos++] = 0x80;
	}
}

static void yahoo_dump_unhandled(struct yahoo_packet *pkt)
{
	YList *l;

	NOTICE(("Service: %s (0x%02x)\tStatus: %s (%d)", dbg_service(pkt->service),pkt->service, dbg_status(pkt->status), pkt->status));
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;
		
		NOTICE(("\t%d => %s", pair->key, pair->value));
	}
}


static void yahoo_packet_dump(unsigned char *data, int len)
{
	if (yahoo_get_log_level() >= YAHOO_LOG_DEBUG) {
		char z[4096], t[10];
		int i;
		
		z[0]='\0';
		
		for (i = 0; i < len; i++) {
			if ((i % 8 == 0) && i)
				//YAHOO_CALLBACK(ext_yahoo_log)(" ");
				lstrcatA(z, " ");
			if ((i % 16 == 0) && i)
				lstrcatA(z, "\n");
			
			wsprintfA(t, "%02x ", data[i]);
			lstrcatA(z, t);
		}
		lstrcatA(z, "\n");
		YAHOO_CALLBACK(ext_yahoo_log)(z);
		
		z[0]='\0';
		for (i = 0; i < len; i++) {
			if ((i % 8 == 0) && i)
				//YAHOO_CALLBACK(ext_yahoo_log)(" ");
				lstrcatA(z, " ");
			if ((i % 16 == 0) && i)
				//YAHOO_CALLBACK(ext_yahoo_log)("\n");
				lstrcatA(z, "\n");
			if (isprint(data[i])) {
				//YAHOO_CALLBACK(ext_yahoo_log)(" %c ", data[i]);
				wsprintfA(t, " %c ", data[i]);
				lstrcatA(z, t);
			} else
				//YAHOO_CALLBACK(ext_yahoo_log)(" . ");
				lstrcatA(z, " . ");
		}
		//YAHOO_CALLBACK(ext_yahoo_log)("\n");
		lstrcatA(z, "\n");
		YAHOO_CALLBACK(ext_yahoo_log)(z);
	}
}

static const char base64digits[] = 	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				"abcdefghijklmnopqrstuvwxyz"
				"0123456789._";

static void to_y64(unsigned char *out, const unsigned char *in, int inlen)
/* raw bytes in quasi-big-endian order to base 64 string (NUL-terminated) */
{
	for (; inlen >= 3; inlen -= 3)
		{
			*out++ = base64digits[in[0] >> 2];
			*out++ = base64digits[((in[0]<<4) & 0x30) | (in[1]>>4)];
			*out++ = base64digits[((in[1]<<2) & 0x3c) | (in[2]>>6)];
			*out++ = base64digits[in[2] & 0x3f];
			in += 3;
		}
	if (inlen > 0)
		{
			unsigned char fragment;

			*out++ = base64digits[in[0] >> 2];
			fragment = (in[0] << 4) & 0x30;
			if (inlen > 1)
				fragment |= in[1] >> 4;
			*out++ = base64digits[fragment];
			*out++ = (inlen < 2) ? '-' 
					: base64digits[(in[1] << 2) & 0x3c];
			*out++ = '-';
		}
	*out = '\0';
}

static void yahoo_add_to_send_queue(struct yahoo_input_data *yid, void *data, int length)
{
	struct data_queue *tx = y_new0(struct data_queue, 1);
	tx->queue = y_new0(unsigned char, length);
	tx->len = length;
	memcpy(tx->queue, data, length);

	yid->txqueues = y_list_append(yid->txqueues, tx);

	if (!yid->write_tag)
		yid->write_tag=YAHOO_CALLBACK(ext_yahoo_add_handler)(yid->yd->client_id, yid->fd, YAHOO_INPUT_WRITE, yid);
}

static void yahoo_send_packet(struct yahoo_input_data *yid, struct yahoo_packet *pkt, int extra_pad)
{
	int pktlen = yahoo_packet_length(pkt);
	int len = YAHOO_PACKET_HDRLEN + pktlen;
	unsigned char *data;
	int pos = 0;

	if (yid->fd < 0)
		return;

	data = y_new0(unsigned char, len + 1);

	memcpy(data + pos, "YMSG", 4); pos += 4;
	pos += yahoo_put16(data + pos, YAHOO_PROTO_VER); /* version [latest 12 0x000c] */
	pos += yahoo_put16(data + pos, 0x0000); /* HIWORD pkt length??? */
	pos += yahoo_put16(data + pos, pktlen + extra_pad); /* LOWORD pkt length? */
	pos += yahoo_put16(data + pos, pkt->service); /* service */
	pos += yahoo_put32(data + pos, pkt->status); /* status [4bytes] */
	pos += yahoo_put32(data + pos, pkt->id); /* session [4bytes] */

	yahoo_packet_write(pkt, data + pos);

	//yahoo_packet_dump(data, len);
	DEBUG_MSG1(("Sending Packet:"));

	yahoo_packet_read(pkt, data + pos, len - pos);	
	
	if ( yid->type == YAHOO_CONNECTION_FT || 
		( yid->type == YAHOO_CONNECTION_PAGER && 
			( pkt->service == YAHOO_SERVICE_KEEPALIVE || 
			  pkt->service == YAHOO_SERVICE_PING ||
			  pkt->service == YAHOO_SERVICE_LOGOFF))
		) {
		yahoo_send_data(yid->fd, (const char *)data, len);
	} else {
		yahoo_add_to_send_queue(yid, data, len);
	}
	
	FREE(data);
}

static void yahoo_packet_free(struct yahoo_packet *pkt)
{
	while (pkt->hash) {
		struct yahoo_pair *pair = (struct yahoo_pair *)pkt->hash->data;
		YList *tmp;
		FREE(pair->value);
		FREE(pair);
		tmp = pkt->hash;
		pkt->hash = y_list_remove_link(pkt->hash, pkt->hash);
		y_list_free_1(tmp);
	}
	FREE(pkt);
}

static int yahoo_send_data(int fd, const char *data, int len)
{
	int ret;
	int e;

	if (fd < 0)
		return -1;

	//yahoo_packet_dump(data, len);

	do {
		ret = write(fd, data, len);
	} while(ret == -1 && errno==EINTR);
	e=errno;

	if (ret == -1)  {
		LOG(("wrote data: ERR %s", strerror(errno)));
	} /*else {
		LOG(("wrote data: OK"));
	}*/

	errno=e;
	return ret;
}

void yahoo_close(int id) 
{
	struct yahoo_data *yd = find_conn_by_id(id);
	if (!yd)
		return;

	del_from_list(yd);

	yahoo_free_data(yd);
	if (id == last_id)
		last_id--;
}

static void yahoo_input_close(struct yahoo_input_data *yid) 
{
	inputs = y_list_remove(inputs, yid);

	LOG(("yahoo_input_close(read)")); 
	YAHOO_CALLBACK(ext_yahoo_remove_handler)(yid->yd->client_id, yid->read_tag);
	LOG(("yahoo_input_close(write)")); 
	YAHOO_CALLBACK(ext_yahoo_remove_handler)(yid->yd->client_id, yid->write_tag);
	yid->read_tag = yid->write_tag = 0;
	if (yid->fd)
		close(yid->fd);
	yid->fd = 0;
	FREE(yid->rxqueue);
	if (count_inputs_with_id(yid->yd->client_id) == 0) {
		LOG(("closing %d", yid->yd->client_id));
		yahoo_close(yid->yd->client_id);
	}
	yahoo_free_webcam(yid->wcm);
	if (yid->wcd)
		FREE(yid->wcd);
	if (yid->ys) {
		FREE(yid->ys->lsearch_text);
		FREE(yid->ys);
	}
	FREE(yid);
}

static int is_same_bud(const void * a, const void * b) 
{
	const struct yahoo_buddy *subject = (struct yahoo_buddy *) a;
	const struct yahoo_buddy *object  = (struct yahoo_buddy *) b;

	return strcmp(subject->id, object->id) && ( subject->protocol == object->protocol );
}

char * getcookie(char *rawcookie)
{
	char * cookie=NULL;
	char * tmpcookie; 
	char * cookieend;

	if (strlen(rawcookie) < 2) 
		return NULL;

	tmpcookie = strdup(rawcookie+2);
	cookieend = strchr(tmpcookie, ';');

	if (cookieend)
		*cookieend = '\0';

	cookie = strdup(tmpcookie);
	FREE(tmpcookie);
	/* cookieend=NULL;  not sure why this was there since the value is not preserved in the stack -dd */

	return cookie;
}

static char * getlcookie(char *cookie)
{
	char *tmp;
	char *tmpend;
	char *login_cookie = NULL;

	tmpend = strstr(cookie, "n=");
	if (tmpend) {
		tmp = strdup(tmpend+2);
		tmpend = strchr(tmp, '&');
		if (tmpend)
			*tmpend='\0';
		login_cookie = strdup(tmp);
		FREE(tmp);
	}

	return login_cookie;
}

static void yahoo_process_notify(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	struct yahoo_data *yd = yid->yd;
	char *msg = NULL;
	char *from = NULL;
	char *to = NULL;
	int stat = 0;
	int accept = 0;
	int protocol = 0;
	char *ind = NULL;
	YList *l;
	
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;
		if (pair->key == 4)
			from = pair->value;
		if (pair->key == 5)
			to = pair->value;
		if (pair->key == 49)
			msg = pair->value;
		if (pair->key == 13)
			stat = atoi(pair->value);
		if (pair->key == 14)
			ind = pair->value;
		if (pair->key == 16) {	/* status == -1 */
			NOTICE((pair->value));
			return;
		}
		if (pair->key == 241)
			protocol = atoi(pair->value);
	}

	if (!msg)
		return;
	
	if (!strncasecmp(msg, "TYPING", strlen("TYPING"))) 
		YAHOO_CALLBACK(ext_yahoo_typing_notify)(yd->client_id, to, from, protocol, stat);
	else if (!strncasecmp(msg, "GAME", strlen("GAME"))) 
		YAHOO_CALLBACK(ext_yahoo_game_notify)(yd->client_id, to, from, stat, ind);
	else if (!strncasecmp(msg, "WEBCAMINVITE", strlen("WEBCAMINVITE"))) 
	{
		if (!strcmp(ind, " ")) {
			YAHOO_CALLBACK(ext_yahoo_webcam_invite)(yd->client_id, to, from);
		} else {
			accept = atoi(ind);
			/* accept the invitation (-1 = deny 1 = accept) */
			if (accept < 0)
				accept = 0;
			YAHOO_CALLBACK(ext_yahoo_webcam_invite_reply)(yd->client_id, to, from, accept);
		}
	}
	else
		LOG(("Got unknown notification: %s", msg));
}

static void yahoo_process_filetransfer(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	struct yahoo_data *yd = yid->yd;
	char *from=NULL;
	char *to=NULL;
	char *msg=NULL;
	char *url=NULL;
	long expires=0;

	char *service=NULL;
	char *ft_token=NULL;
	char *filename=NULL;
	unsigned long filesize=0L;

	YList *l;
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;
		if (pair->key == 4)
			from = pair->value;
		if (pair->key == 5)
			to = pair->value;
		if (pair->key == 14)
			msg = pair->value;
		if (pair->key == 20)
			url = pair->value;
		if (pair->key == 38)
			expires = atol(pair->value);

		if (pair->key == 27)
			filename = pair->value;
		
		if (pair->key == 28)
			filesize = atol(pair->value);

		if (pair->key == 49)
			service = pair->value;
		
		if (pair->key == 53)
			ft_token = pair->value;
	}

	if (pkt->service == YAHOO_SERVICE_P2PFILEXFER) {
		if (strcmp("FILEXFER", service) != 0) {
			WARNING(("unhandled service 0x%02x", pkt->service));
			yahoo_dump_unhandled(pkt);
			return;
		}
	}

	if (msg) {
		char *tmp;
		tmp = strchr(msg, '\006');
		if (tmp)
			*tmp = '\0';
	}
	if (url && from)
		YAHOO_CALLBACK(ext_yahoo_got_file)(yd->client_id, to, from, url, expires, msg, filename, filesize, ft_token, 0);
	else if (strcmp(from, "FILE_TRANSFER_SYSTEM") == 0 && msg != NULL)
		YAHOO_CALLBACK(ext_yahoo_system_message)(yd->client_id, to, from, msg);
}

static void yahoo_process_filetransfer7(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	struct yahoo_data *yd = yid->yd;
	char *from=NULL;
	char *to=NULL;

	int service=0;
	char *ft_token=NULL;
	char *filename=NULL;
	unsigned long filesize=0L;
	
	struct yahoo_file_info *fi;
	YList *l, *files=NULL;
	
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;
		switch (pair->key) {
		case 4: /* from */
			from = pair->value;
			break;
			
		case 5: /* to */
			to = pair->value;
			break;
			
		case 222: /* Services: 
					1   - dl
					2	- cancel
					3	- send
					*/
			service = atol(pair->value);
			break;
			
		case 265: /* this is the FT token for this batch/session */
			ft_token = pair->value;
			break;
			
			
		case 27: /* filename */
			filename = pair->value;
			break;
		
		case 28: /* file size */
			filesize = atol(pair->value);
			break;
			
		case 301:  /* file terminator token usually set to 268 */
			fi = y_new0(struct yahoo_file_info, 1);
			fi->filename = strdup(filename);
			fi->filesize = filesize;
			
			files = y_list_append(files, fi);
			break;
		}
	}

	switch (service) {
	case 1: // FT7 
		YAHOO_CALLBACK(ext_yahoo_got_files)(yd->client_id, to, from, ft_token, service, files);
		break;
	case 2: // FT7 Cancelled
		break;
	case 3: // FT7 Send Files
		YAHOO_CALLBACK(ext_yahoo_send_file7info)(yd->client_id, to, from, ft_token);
		break;
	case 4: // FT7 Declined
		
		break;
	}
}

char *yahoo_decode(const char *t)
{
	/*
	 * Need to process URL ??? we get sent \002 style thingies.. which need to be decoded
	 * and then urlencoded?
	 *
	 * Thanks GAIM for the code...
	 */
	char y[1024];
	char *n;
	const char *end, *p;
	int i, k;

	n = y;
	end = t + lstrlenA(t);
	
	for (p = t; p < end; p++, n++) {
		if (*p == '\\') {
			if (p[1] >= '0' && p[1] <= '7') {
				p += 1;
				for (i = 0, k = 0; k < 3; k += 1) {
					char c = p[k];
					if (c < '0' || c > '7') break;
					i *= 8;
					i += c - '0';
				}
				*n = i;
				p += k - 1;
			} else { /* bug 959248 */
				/* If we see a \ not followed by an octal number,
				 * it means that it is actually a \\ with one \
				 * already eaten by some unknown function.
				 * This is arguably broken.
				 *
				 * I think wing is wrong here, there is no function
				 * called that I see that could have done it. I guess
				 * it is just really sending single \'s. That's yahoo
				 * for you.
				 */
				*n = *p;
			}
		}
		else
			*n = *p;
	}

	*n = '\0';

	return yahoo_urlencode(y);
}

static void yahoo_process_filetransfer7info(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	struct yahoo_data *yd = yid->yd;
	char *from=NULL;
	char *to=NULL;
	int service=0;
	char *ft_token=NULL;
	char *filename=NULL;
	char *host = NULL;
	char *token = NULL;
	unsigned long filesize=0L;

	/**
		TODO: Need to process FileTransfer7Info Disconnected Status.
	
		It doesn't send service but sends Status (66) = -1
	[10:56:02 YAHOO] Key: (    1) ID                        Value: 'xxx'
	[10:56:02 YAHOO] Key: (    4) ID/Nick                   Value: 'xxx'
	[10:56:02 YAHOO] Key: (    5) To                        Value: 'xxxxxxx'
	[10:56:02 YAHOO] Key: (   66) login status              Value: '-1'
	[10:56:02 YAHOO] Key: (  251) (null)                    Value: 'likQolabUXpDajoIdTZKPw--AsM.A7RnMpJwfZjQmIm.SZea2CCIGPAjF0DTHjizENuccwdZueaEuA13irqIIdAJcPOT24yWnwwvIHYqcMg4foLt0LA-'
	[10:56:02 YAHOO] Key: (  265) FT7 Token                 Value: '$t$1vTZy4AzepDkGzJoMBg$$'

	*/
	
	YList *l;
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;
		switch (pair->key) {
		case 4:
			from = pair->value;
			break;
		case 5:
			to = pair->value;
			break;
		case 27:
			filename = pair->value;
			break;
		
		case 28:
			filesize = atol(pair->value);
			break;
			
		case 249:
			service = atol(pair->value);
			break;
		case 250:
			host = pair->value;
			break;
		case 251:
			token = pair->value;
			break;
		case 265:
			ft_token = pair->value;
			break;
		}
	}

	switch (service) {
	case 1: // P2P
		//YAHOO_CALLBACK(ext_yahoo_got_file)(yd->client_id, to, from, url, expires, msg, filename, filesize, ft_token, 1);
		{
			/*
			 * From Kopete: deny P2P
			 */
			struct yahoo_packet *pkt1 = NULL;

			LOG(("[yahoo_process_filetransfer7info] Got File info, Denying P2P."));
			
			pkt1 = yahoo_packet_new(YAHOO_SERVICE_Y7_FILETRANSFERACCEPT, YPACKET_STATUS_DEFAULT, yd->session_id);
			yahoo_packet_hash(pkt1, 1, yd->user);
			yahoo_packet_hash(pkt1, 5, from);
			yahoo_packet_hash(pkt1,265, ft_token);
			yahoo_packet_hash(pkt1,66, "-3");
			
			yahoo_send_packet(yid, pkt1, 0);
			yahoo_packet_free(pkt1);

		}
		break;
	case 3: // Relay
		{
			char url[1024];
			char *t;
			
			/*
			 * From Kopete: accept the info?
			 */
			struct yahoo_packet *pkt1 = NULL;

			LOG(("[yahoo_process_filetransfer7info] Got File info, Relaying FT."));
			
			pkt1 = yahoo_packet_new(YAHOO_SERVICE_Y7_FILETRANSFERACCEPT, YPACKET_STATUS_DEFAULT, yd->session_id);
			yahoo_packet_hash(pkt1, 1, yd->user);
			yahoo_packet_hash(pkt1, 5, from);
			yahoo_packet_hash(pkt1,265, ft_token);
			yahoo_packet_hash(pkt1,27, filename);
			yahoo_packet_hash(pkt1,249, "3"); // use reflection server
			yahoo_packet_hash(pkt1,251, token);
			
			yahoo_send_packet(yid, pkt1, 0);
			yahoo_packet_free(pkt1);
			
			t = yahoo_decode(token);
			sprintf(url,"http://%s/relay?token=%s&sender=%s&recver=%s", host, t, from, to);
			
			YAHOO_CALLBACK(ext_yahoo_got_file7info)(yd->client_id, to, from, url, filename, ft_token);
			
			FREE(t);
		}
		break;
	}
}
static void yahoo_process_filetransfer7accept(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	struct yahoo_data *yd = yid->yd;
	char *from=NULL;
	char *to=NULL;
	int service=0;
	char *ft_token=NULL;
	char *filename=NULL;
	char *token=NULL;

	/**
		TODO: Need to process FileTransfer7Info Disconnected Status.
	
		It doesn't send service but sends Status (66) = -1
	[10:56:02 YAHOO] Key: (    1) ID                        Value: 'xxx'
	[10:56:02 YAHOO] Key: (    4) ID/Nick                   Value: 'xxx'
	[10:56:02 YAHOO] Key: (    5) To                        Value: 'xxxxxxx'
	[10:56:02 YAHOO] Key: (   66) login status              Value: '-1'
	[10:56:02 YAHOO] Key: (  251) (null)                    Value: 'likQolabUXpDajoIdTZKPw--AsM.A7RnMpJwfZjQmIm.SZea2CCIGPAjF0DTHjizENuccwdZueaEuA13irqIIdAJcPOT24yWnwwvIHYqcMg4foLt0LA-'
	[10:56:02 YAHOO] Key: (  265) FT7 Token                 Value: '$t$1vTZy4AzepDkGzJoMBg$$'

	*/
	
	YList *l;
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;
		
		switch (pair->key) {
		case 4:
			from = pair->value;
			break;
		case 5:
			to = pair->value;
			break;
		case 27:
			filename = pair->value;
			break;
		
		case 249:
			service = atol(pair->value);
			break;
			
		case 251:
			token = pair->value;
			break;
			
		case 265:
			ft_token = pair->value;
			break;
			
		case 66: // login status = -1  Disconnected/Failed Transfer.
			break;
			
		case 271: // 271 = 1 "Next File"
			YAHOO_CALLBACK(ext_yahoo_send_file7info)(yd->client_id, to, from, ft_token);
			break;
		}
	}

	switch (service) {
	case 1: // P2P
		
		break;
		
	case 3: // Relay
		YAHOO_CALLBACK(ext_yahoo_ft7_send_file)(yd->client_id, to, from, filename, token, ft_token);
		break;
	}
}


static void yahoo_process_conference(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	struct yahoo_data *yd = yid->yd;
	char *msg = NULL;
	char *host = NULL;
	char *who = NULL;
	char *room = NULL;
	char *id = NULL;
	int  utf8 = 0;
	YList *members = NULL;
	YList *l;
	
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;
		
		if (pair->key == 50)
			host = pair->value;
		
		if (pair->key == 52) {		/* invite */
			members = y_list_append(members, strdup(pair->value));
		}
		if (pair->key == 53)		/* logon */
			who = pair->value;
		if (pair->key == 54)		/* decline */
			who = pair->value;
		if (pair->key == 55)		/* unavailable (status == 2) */
			who = pair->value;
		if (pair->key == 56)		/* logoff */
			who = pair->value;

		if (pair->key == 57)
			room = pair->value;

		if (pair->key == 58)		/* join message */
			msg = pair->value;
		if (pair->key == 14)		/* decline/conf message */
			msg = pair->value;

		if (pair->key == 13)
			;
		if (pair->key == 16)		/* error */
			msg = pair->value;

		if (pair->key == 1)		/* my id */
			id = pair->value;
		if (pair->key == 3)		/* message sender */
			who = pair->value;

		if (pair->key == 97)
			utf8 = atoi(pair->value);
	}

	if (!room)
		return;

	if (host) {
		for (l = members; l; l = l->next) {
			char * w = (char *) l->data;
			if (!strcmp(w, host))
				break;
		}
		if (!l)
			members = y_list_append(members, strdup(host));
	}
	/* invite, decline, join, left, message -> status == 1 */

	switch(pkt->service) {
	case YAHOO_SERVICE_CONFINVITE:
		if (pkt->status == 2)
			;
		else if (members)
			YAHOO_CALLBACK(ext_yahoo_got_conf_invite)(yd->client_id, id, host, room, msg, members);
		else if (msg)
			YAHOO_CALLBACK(ext_yahoo_error)(yd->client_id, msg, 0, E_CONFNOTAVAIL);
		break;
	case YAHOO_SERVICE_CONFADDINVITE:
		if (pkt->status == 2)
			;
		else
			YAHOO_CALLBACK(ext_yahoo_got_conf_invite)(yd->client_id, id, host, room, msg, members);
		break;
	case YAHOO_SERVICE_CONFDECLINE:
		if (who)
			YAHOO_CALLBACK(ext_yahoo_conf_userdecline)(yd->client_id, id, who, room, msg);
		break;
	case YAHOO_SERVICE_CONFLOGON:
		if (who)
			YAHOO_CALLBACK(ext_yahoo_conf_userjoin)(yd->client_id, id, who, room);
		break;
	case YAHOO_SERVICE_CONFLOGOFF:
		if (who)
			YAHOO_CALLBACK(ext_yahoo_conf_userleave)(yd->client_id, id, who, room);
		break;
	case YAHOO_SERVICE_CONFMSG:
		if (who)
			YAHOO_CALLBACK(ext_yahoo_conf_message)(yd->client_id, id, who, room, msg, utf8);
		break;
	}
}

static void yahoo_process_chat(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	char *msg = NULL;
	char *id = NULL;
	char *who = NULL;
	char *room = NULL;
	char *topic = NULL;
	YList *members = NULL;
	struct yahoo_chat_member *currentmember = NULL;
	int  msgtype = 1;
	int  utf8 = 0;
	int  firstjoin = 0;
	int  membercount = 0;
	int  chaterr=0;
	YList *l;
	
	yahoo_dump_unhandled(pkt);
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;

		if (pair->key == 1) {
			/* My identity */
			id = pair->value;
		}

		if (pair->key == 104) {
			/* Room name */
			room = pair->value;
		}

		if (pair->key == 105) {
			/* Room topic */
			topic = pair->value;
		}

		if (pair->key == 108) {
			/* Number of members in this packet */
			membercount = atoi(pair->value);
		}

		if (pair->key == 109) {
			/* message sender */
			who = pair->value;

			if (pkt->service == YAHOO_SERVICE_CHATJOIN) {
				currentmember = y_new0(struct yahoo_chat_member, 1);
				currentmember->id = strdup(pair->value);
				members = y_list_append(members, currentmember);
			}
		}

		if (pair->key == 110) {
			/* age */
			if (pkt->service == YAHOO_SERVICE_CHATJOIN)
				currentmember->age = atoi(pair->value);
		}

		if (pair->key == 113) {
			/* attribs */
			if (pkt->service == YAHOO_SERVICE_CHATJOIN)
				currentmember->attribs = atoi(pair->value);
		}

		if (pair->key == 141) {
			/* alias */
			if (pkt->service == YAHOO_SERVICE_CHATJOIN)
				currentmember->alias = strdup(pair->value);
		}

		if (pair->key == 142) {
			/* location */
			if (pkt->service == YAHOO_SERVICE_CHATJOIN)
				currentmember->location = strdup(pair->value);
		}


		if (pair->key == 130) {
			/* first join */
			firstjoin = 1;
		}

		if (pair->key == 117) {
			/* message */
			msg = pair->value;
		}

		if (pair->key == 124) {
			/* Message type */
			msgtype = atoi(pair->value);
		}
		if (pair->key == 114) {
			/* message error not sure what all the pair values mean */
			/* but -1 means no session in room */
			chaterr= atoi(pair->value);
		}
	}

	if (!room) {
		if (pkt->service == YAHOO_SERVICE_CHATLOGOUT) { /* yahoo originated chat logout */
			YAHOO_CALLBACK(ext_yahoo_chat_yahoologout)(yid->yd->client_id, id);
			return ;
		}
		if (pkt->service == YAHOO_SERVICE_COMMENT && chaterr)  {
			YAHOO_CALLBACK(ext_yahoo_chat_yahooerror)(yid->yd->client_id, id);
			return;
		}

		WARNING(("We didn't get a room name, ignoring packet"));
		return;
	}

	switch(pkt->service) {
	case YAHOO_SERVICE_CHATJOIN:
		if (y_list_length(members) != membercount) {
			WARNING(("Count of members doesn't match No. of members we got"));
		}
		if (firstjoin && members) {
			YAHOO_CALLBACK(ext_yahoo_chat_join)(yid->yd->client_id, id, room, topic, members, yid->fd);
		} else if (who) {
			if (y_list_length(members) != 1) {
				WARNING(("Got more than 1 member on a normal join"));
			}
			/* this should only ever have one, but just in case */
			while(members) {
				YList *n = members->next;
				currentmember = (struct yahoo_chat_member *) members->data;
				YAHOO_CALLBACK(ext_yahoo_chat_userjoin)(yid->yd->client_id, id, room, currentmember);
				y_list_free_1(members);
				members=n;
			}
		}
		break;
	case YAHOO_SERVICE_CHATEXIT:
		if (who) {
			YAHOO_CALLBACK(ext_yahoo_chat_userleave)(yid->yd->client_id, id, room, who);
		}
		break;
	case YAHOO_SERVICE_COMMENT:
		if (who) {
			YAHOO_CALLBACK(ext_yahoo_chat_message)(yid->yd->client_id, id, who, room, msg, msgtype, utf8);
		}
		break;
	}
}

static void yahoo_process_message(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	struct yahoo_data *yd = yid->yd;
	YList *l;
	YList * messages = NULL;

	struct m {
		int  i_31;
		int  i_32;
		char *to;
		char *from;
		long tm;
		char *msg;
		int  utf8;
		int  buddy_icon;
		int  protocol;
		char *seqn;
		int  sendn;
	} *message = y_new0(struct m, 1);

	message->buddy_icon = -1; // no info
	message->utf8		= 1 ; // default value for utf-8. (Seems a ton of clients/pingbox/etc.. don't send this)
	
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;
		
		/* so it seems that key == 1 is not used when receiving messages and causes
		  problems for mobile IMs? This has been reported in the forum and this patch
		 was provided by Bryan Aldrich */
		switch (pair->key) {
			/*case 1: */
			case 4:
				if (!message->from)
					message->from = pair->value;
				
				break;
		
			case 5:
				message->to = pair->value;
				break;
				
			case 15:
				message->tm = strtol(pair->value, NULL, 10);
				break;
				
			case 206:
				message->buddy_icon = atoi(pair->value);
				break;
				
			case 97:
				message->utf8 = atoi(pair->value);
				break;
				
			/* user message */  /* sys message */
			case 14:
			case 16:
				message->msg = pair->value;
				break;
				
			case 31:
				if (message->i_31) {
					messages = y_list_append(messages, message);
					message = y_new0(struct m, 1);
				}
				message->i_31 = atoi(pair->value);
				break;
				
			case 32:
				message->i_32 = atoi(pair->value);
				break;
				
			case 241:
				message->protocol = strtol(pair->value, NULL, 10);
				break;
			
			case 429: /* message sequence # */
				message->seqn = pair->value;
				break;
				
			case 450: /* attempt # */
				message->sendn = atoi(pair->value);
				break;
				
				
			/*default:
				LOG(("yahoo_process_message: status: %d, key: %d, value: %s",
					pkt->status, pair->key, pair->value));
			*/
		}
	}

	messages = y_list_append(messages, message);

	for (l = messages; l; l=l->next) {
		message = (struct m*) l->data;
		if (pkt->service == YAHOO_SERVICE_SYSMESSAGE) {
			YAHOO_CALLBACK(ext_yahoo_system_message)(yd->client_id, message->to, 
													message->from, message->msg);
		} else if (pkt->status <= 2 || pkt->status == 5) {
			YAHOO_CALLBACK(ext_yahoo_got_im)(yd->client_id, message->to, message->from, 
											message->protocol, message->msg, message->tm, 
											pkt->status, message->utf8, message->buddy_icon, 
											message->seqn, message->sendn);
		} else if (pkt->status == YPACKET_STATUS_DISCONNECTED) {
			YAHOO_CALLBACK(ext_yahoo_error)(yd->client_id, message->msg, 0, E_SYSTEM);
		}
		free(message);
	}

	y_list_free(messages);
}

static void yahoo_process_logon(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	YList *l;
	struct yahoo_data *yd = yid->yd;
	char *name = NULL;
	int state = 0, away = 0, idle = 0, mobile = 0, cksum = 0, buddy_icon = -1, protocol = 0,
		client_version = 0, utf8 = 1;
	char *msg = NULL;
	
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;

		switch (pair->key) {
		case 0: /* we won't actually do anything with this */
			//NOTICE(("key %d:%s", pair->key, pair->value));
			break;
		case 1: /* we don't get the full buddy list here. */
			if (!yd->logged_in) {
				yd->logged_in = TRUE;
				
				if (yd->current_status == YAHOO_STATUS_OFFLINE)
					yd->current_status = yd->initial_status;
				
				YAHOO_CALLBACK(ext_yahoo_login_response)(yd->client_id, YAHOO_LOGIN_OK, NULL);
			}
			break;
		case 7: /* the current buddy */
			if (name != NULL) {
				YAHOO_CALLBACK(ext_yahoo_status_logon)(yd->client_id, name, protocol, state, msg, away, idle, mobile, cksum, buddy_icon, client_version, utf8);
				msg = NULL;
				utf8 = 1;
				protocol = client_version = cksum = state = away = idle = mobile = 0;
				buddy_icon = -1;
			}
			name = pair->value;
			break;
		case 8: /* how many online buddies we have */
			//NOTICE(("key %d:%s", pair->key, pair->value));
			break;
		case 10: /* state */
			state = strtol(pair->value, NULL, 10);
			break;
		case 11: /* this is the buddy's session id */
			//NOTICE(("key %d:%s", pair->key, pair->value));
			break;
		case 13: /* bitmask, bit 0 = pager, bit 1 = chat, bit 2 = game */
			if (strtol(pair->value, NULL, 10) == 0) {
				//YAHOO_CALLBACK(ext_yahoo_status_changed)(yd->client_id, name, 0, YAHOO_STATUS_OFFLINE, NULL, 1, 0, 0);
				//name = NULL;
				state = YAHOO_STATUS_OFFLINE;
				//break;
			}
			break;
		case 16: /* Custom error message */
			YAHOO_CALLBACK(ext_yahoo_error)(yd->client_id, pair->value, 0, E_CUSTOM);
			break;
		case 17: /* in chat? */
			break;
		case 19: /* custom status message */
			msg = pair->value;
			break;
		case 24:  /* session timestamp */
			yd->session_timestamp = atol(pair->value);
			break;
		case 47: /* is it an away message or not */
			away = atoi(pair->value);
			break;
		case 60: /* SMS -> 1 MOBILE USER */
			/* sometimes going offline makes this 2, but invisible never sends it */
			//NOTICE(("key %d:%s", pair->key, pair->value));
			if (atoi(pair->value) > 0)
				mobile = 1;
			break;
			
		case 97: /* utf8 */
			utf8 = atoi(pair->value);
			break;
			
		case 137: /* Idle: seconds */
			idle = atoi(pair->value);
			break;
		case 138: /* Idle: Flag 
				   *	0: Use the 137 key to see how long
				   *    1: not-idle
				   */
			
			idle = 0;
			break;

		case 192: /* Pictures aka BuddyIcon  checksum*/
			cksum = strtol(pair->value, NULL, 10);
			break;

		case 197: /* avatar base64 encodded [Ignored by Gaim?] */
			/*avatar = pair->value;*/
			break;

		case 213: /* Pictures aka BuddyIcon  type 0-none, 1-avatar, 2-picture*/
			buddy_icon = strtol(pair->value, NULL, 10);
			break;
	
		case 244: /* client version number. Yahoo Client Detection */
			client_version = strtol(pair->value, NULL, 10);
			break;
			
		case 241: /* protocol */
			protocol = strtol(pair->value, NULL, 10);
			break;
		
		default:
			//WARNING(("unknown status key %d:%s", pair->key, pair->value));
			break;
		}
	}
	
	if (name != NULL) 
		YAHOO_CALLBACK(ext_yahoo_status_logon)(yd->client_id, name, protocol, state, msg, away, idle, mobile, cksum, buddy_icon, client_version, utf8);
	
}

static void yahoo_process_status(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	YList *l;
	struct yahoo_data *yd = yid->yd;
	char *name = NULL;
	int state = YAHOO_STATUS_AVAILABLE;
	int away = 0, idle = 0, mobile = 0, protocol = 0, utf8 = 1;
	int login_status=YAHOO_LOGIN_LOGOFF;
	char *msg = NULL;
	char *errmsg = NULL;
	
	/*if (pkt->service == YAHOO_SERVICE_LOGOFF && pkt->status == YAHOO_STATUS_DISCONNECTED) {
		YAHOO_CALLBACK(ext_yahoo_login_response)(yd->client_id, YAHOO_LOGIN_DUPL, NULL);
		return;
	}*/

	if (pkt->service == YAHOO_SERVICE_LOGOFF) 
		state = YAHOO_STATUS_OFFLINE;
	
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;

		switch (pair->key) {
		case 0: /* we won't actually do anything with this */
			NOTICE(("key %d:%s", pair->key, pair->value));
			break;
		case 1: /* we don't get the full buddy list here. */
			if (!yd->logged_in) {
				yd->logged_in = TRUE;
				if (yd->current_status == YAHOO_STATUS_OFFLINE)
					yd->current_status = yd->initial_status;
				
				YAHOO_CALLBACK(ext_yahoo_login_response)(yd->client_id, YAHOO_LOGIN_OK, NULL);
			}
			break;
		case 8: /* how many online buddies we have */
			NOTICE(("key %d:%s", pair->key, pair->value));
			break;
		case 7: /* the current buddy */
			if (name != NULL) {
				YAHOO_CALLBACK(ext_yahoo_status_changed)(yd->client_id, name, protocol, state, msg, away, idle, mobile, utf8);
				msg = NULL;
				utf8 = 1;
				protocol = away = idle = mobile = 0;
				state = (pkt->service == YAHOO_SERVICE_LOGOFF) ? YAHOO_STATUS_OFFLINE : YAHOO_STATUS_AVAILABLE;
			}
			name = pair->value;
			
			/*if (pkt->service == YAHOO_SERVICE_LOGOFF) {
				YAHOO_CALLBACK(ext_yahoo_status_changed)(yd->client_id, name, protocol, YAHOO_STATUS_OFFLINE, NULL, 0, 0, 0);
				name = NULL;
			}*/
			break;
		case 10: /* state */
			state = strtol(pair->value, NULL, 10);
			break;
		case 19: /* custom status message */
			msg = pair->value;
			break;
		case 47: /* is it an away message or not */
			away = atoi(pair->value);
			break;
		case 137: /* seconds idle */
			idle = atoi(pair->value);
			break;
		case 138: /* either we're not idle, or we are but won't say how long */
			/* thanx Gaim.. I am seeing 138 -> 1. so don't do idle at all for miranda
				since we don't have idle w/o time :( */
			idle = 0;
			break;
		case 11: /* this is the buddy's session id */
			NOTICE(("key %d:%s", pair->key, pair->value));
			break;
		case 17: /* in chat? */
			break;
		case 13: /* bitmask, bit 0 = pager, bit 1 = chat, bit 2 = game */
			/*if (pkt->service == YAHOO_SERVICE_LOGOFF || strtol(pair->value, NULL, 10) == 0) {
				YAHOO_CALLBACK(ext_yahoo_status_changed)(yd->client_id, name, protocol, YAHOO_STATUS_OFFLINE, NULL, 0, 0, 0);
				name = NULL;
				break;
			}*/
			if (strtol(pair->value, NULL, 10) == 0) 
				state = YAHOO_STATUS_OFFLINE;
			break;
		case 60: /* SMS -> 1 MOBILE USER */
			/* sometimes going offline makes this 2, but invisible never sends it */
			NOTICE(("key %d:%s", pair->key, pair->value));
			if (atoi(pair->value) > 0)
				mobile = 1;
			break;
		case 16: /* Custom error message */
			errmsg = pair->value;
			break;
		case 241: /* protocol */
			protocol = strtol(pair->value, NULL, 10);
			break;
		case 66: /* login status */
			{
				int i = atoi(pair->value);
				
				switch(i) {
				case 42: /* duplicate login */
					login_status = YAHOO_LOGIN_DUPL;
					break;
				case 28: /* session expired */
					
					break;
				}
			}
			break;
		case 97:  /* utf-8 ? */
			utf8 = strtol(pair->value, NULL, 10);
			break;
			
		default:
			//WARNING(("unknown status key %d:%s", pair->key, pair->value));
			break;
		}
	}
	
	if (name != NULL) 
		YAHOO_CALLBACK(ext_yahoo_status_changed)(yd->client_id, name, protocol, state, msg, away, idle, mobile, utf8);
	else if (pkt->service == YAHOO_SERVICE_LOGOFF && pkt->status == YPACKET_STATUS_DISCONNECTED) 
		//
		//Key: Error msg (16)  	Value: 'Session expired. Please relogin'
		//Key: login status (66)  	Value: '28'
		//
		YAHOO_CALLBACK(ext_yahoo_login_response)(yd->client_id, login_status, NULL);
	else if (errmsg != NULL)
		YAHOO_CALLBACK(ext_yahoo_error)(yd->client_id, errmsg, 0, E_CUSTOM);
	else if (pkt->service == YAHOO_SERVICE_LOGOFF && pkt->status == YAHOO_STATUS_AVAILABLE && pkt->hash == NULL) 
		// Server Acking our Logoff (close connection)
		yahoo_input_close(yid);
}

static void yahoo_process_list(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	struct yahoo_data *yd = yid->yd;
	YList *l;
	char *fname = NULL, *lname = NULL, *nick = NULL;

	/* we could be getting multiple packets here */
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;

		switch(pair->key) {

		case 89: /* identities */
			{
			char **identities = y_strsplit(pair->value, ",", -1);
			int i;
			for (i=0; identities[i]; i++)
				yd->identities = y_list_append(yd->identities, 
						strdup(identities[i]));
			y_strfreev(identities);
			}
			
			break;
		case 59: /* cookies */
			if (pair->value[0]=='Y') {
				FREE(yd->cookie_y);
				FREE(yd->login_cookie);

				yd->cookie_y = getcookie(pair->value);
				yd->login_cookie = getlcookie(yd->cookie_y);

			} else if (pair->value[0]=='T') {
				FREE(yd->cookie_t);
				yd->cookie_t = getcookie(pair->value);

			} else if (pair->value[0]=='C') {
				FREE(yd->cookie_c);
				yd->cookie_c = getcookie(pair->value);
			} 

			break;
		case 3: /* my id */
			nick = pair->value;
			break;
		case 90: /* 1 */
		case 100: /* 0 */
		case 101: /* NULL */
		case 102: /* NULL */
		case 93: /* 86400/1440 */
			break;
		case 213: /* my current avatar setting */
			{
				int buddy_icon = strtol(pair->value, NULL, 10);
				
				YAHOO_CALLBACK(ext_yahoo_got_avatar_share)(yd->client_id, buddy_icon);
			}
			break;
		case 217: /*??? Seems like last key */
					
			break;
		
		case 216: /* Firat Name */
			fname = pair->value;
			break;
			
		case 254: /* Last Name */
			lname = pair->value;
			break;
		}
	}

	YAHOO_CALLBACK(ext_yahoo_got_identities)(yd->client_id, nick, fname, lname, yd->identities);
	
	/* we could be getting multiple packets here */
	if (pkt->status != 0) /* Thanks for the fix GAIM */
		return;
	
	if (yd->cookie_y && yd->cookie_t && yd->cookie_c)
				YAHOO_CALLBACK(ext_yahoo_got_cookies)(yd->client_id);
	
	/*** We login at the very end of the packet communication */
	if (!yd->logged_in) {
		yd->logged_in = TRUE;
		
		if (yd->current_status == YAHOO_STATUS_OFFLINE)
			yd->current_status = yd->initial_status;
		
		YAHOO_CALLBACK(ext_yahoo_login_response)(yd->client_id, YAHOO_LOGIN_OK, NULL);
	}
}

static void yahoo_process_y8_list(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	struct yahoo_data 	*yd = yid->yd;
	YList 				*l;
	struct yahoo_buddy 	*bud=NULL;
	
	/* we could be getting multiple packets here */
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;

		switch(pair->key) {
		case 302:
			/* This is always 318 before a group, 319 before the first s/n in a group, 320 before any ignored s/n.
			 * It is not sent for s/n's in a group after the first.
			 * All ignored s/n's are listed last, so when we see a 320 we clear the group and begin marking the
			 * s/n's as ignored.  It is always followed by an identical 300 key.
			 */
			if (pair->value && !strcmp(pair->value, "320")) {
				/* No longer in any group; this indicates the start of the ignore list. */
				FREE(yd->ygrp);
			}

			break;
		case 301: /* This is 319 before all s/n's in a group after the first. It is followed by an identical 300. */
			break;
		case 300: /* This is 318 before a group, 319 before any s/n in a group, and 320 before any ignored s/n. */
			break;
		case 65: /* This is the group */
			FREE(yd->ygrp);
			yd->ygrp = strdup(pair->value);
			break;
		case 7: /* buddy's s/n */
			/**
			 * Need to add the buddy to one of several lists
			 */
			bud = y_new0(struct yahoo_buddy, 1);
			bud->id = strdup(pair->value);
			
			if (yd->ygrp) {
				bud->group = strdup(yd->ygrp);
				
				yd->buddies = y_list_append(yd->buddies, bud);
			} else {
				yd->ignore = y_list_append(yd->ignore, bud);
			}
			
			break;
		case 241: /* another protocol user */
			if (bud) {
				bud->protocol = strtol(pair->value, NULL, 10);
			}
			break;
			
		case 223:  /* Auth request pending */
			if (bud) {
				bud->auth = strtol(pair->value, NULL, 10);
			}
			break;
			
		case 59: /* somebody told cookies come here too, but im not sure */
			break;
			
		case 317: /* Stealth Setting */
			if (bud && (strtol(pair->value, NULL, 10) == 2)) {
				//f->presence = YAHOO_PRESENCE_PERM_OFFLINE;
				bud->stealth = 2;
			}
			
			break;
		}
	}
	
	/* we could be getting multiple packets here */
	if (pkt->status != 0) 
		return;
	
	if (yd->buddies) {
		YAHOO_CALLBACK(ext_yahoo_got_buddies)(yd->client_id, yd->buddies);
	}

	if (yd->ignore) {
		YAHOO_CALLBACK(ext_yahoo_got_ignore)(yd->client_id, yd->ignore);
	}
	

}

static void yahoo_process_verify(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	struct yahoo_data *yd = yid->yd;
	struct yahoo_server_settings *yss = yd->server_settings;
	
	if (pkt->status != 0x01) {
		DEBUG_MSG(("expected status: 0x01, got: %d", pkt->status));
		YAHOO_CALLBACK(ext_yahoo_login_response)(yd->client_id, YAHOO_LOGIN_LOCK, "");
		return;
	}

	pkt = yahoo_packet_new(YAHOO_SERVICE_AUTH, YPACKET_STATUS_DEFAULT, 0);
	yahoo_packet_hash(pkt, 1, yd->user);
	//NOTICE(("web messenger: %d", yss->web_messenger));
	if (yss->web_messenger) {
		yahoo_packet_hash(pkt, 0, yd->user);
		yahoo_packet_hash(pkt, 24, "0");
	}
	//NOTICE(("Sending initial packet"));

	yahoo_send_packet(yid, pkt, 0);

	yahoo_packet_free(pkt);

}

/*
 * New Yahoo 9.x auth protocol 
 */
static void yahoo_process_auth_0x0f(struct yahoo_input_data *yid, const char *seed, const char *sn)
{
	struct yahoo_packet 			*pack = NULL;
	struct yahoo_data 				*yd = yid->yd;
	struct yahoo_server_settings 	*yss;

	char *crumb=NULL;
	char *response = NULL;
	char url[1024];
	char *c, *t;
	BYTE result[16];
	mir_md5_state_t ctx;
	unsigned char *magic_hash = (unsigned char*) malloc(50); /* this one is like 26 bytes? */
	int i;
	
	/**
		 case 2: Totally Cracked... Yay.. no more crypt tables.. just need some SSL magic.
			
			Thanks to: http://shinkaiho.com/?p=32
			
			login.yahoo.com:443

*chal is returned from ymsg connection
GET /config/pwtoken_get?src=ymsgr&ts=1195577375&login=user&passwd=pass&chal=chal HTTP/1.1

*token is the ymsgr value returned from the above request
GET /config/pwtoken_login?src=ymsgr&ts=1195577376&token=token HTTP/1.1

*crumb is returned from the above request along with ymsg cookie
307 field is crumb + chal md5ed (16 bytes dont convert to hex) then base64ed
			
		 **/
	yss = yd->server_settings;
	
	if (yd->pw_token == NULL) {

		c = yahoo_urlencode(yd->password);
		
		_snprintf(url, sizeof(url), "/config/pwtoken_get?src=ymsgr&login=%s&passwd=%s", sn, c);
		response = YAHOO_CALLBACK(ext_yahoo_send_https_request)(yd, yss->login_host, url);
		
		FREE(c);
		
		if (response == NULL) {
			YAHOO_CALLBACK(ext_yahoo_login_response)(yd->client_id, YAHOO_LOGIN_SOCK, NULL);
			return; // fail for now
		}
		
		LOG(("Got response:\n%s", response));
		
		if (!isdigit(response[0])) {
			LOG(("Non numeric status code received."));
			
			YAHOO_CALLBACK(ext_yahoo_login_response)(yd->client_id, YAHOO_LOGIN_SOCK, NULL);
			return; // fail for now
		}
		
		i = atoi(response);
		
		if (i != 0) {
			/**
			 * Some Error Code, we need to process it here
			 */
			
			switch (i) {
				case 1212: /* Invalid ID or password. Please try again. */
						YAHOO_CALLBACK(ext_yahoo_login_response)(yd->client_id, YAHOO_LOGIN_PASSWD, NULL);
						break;
						
				case 1213: 
						/* security lock from too many failed login attempts */
						YAHOO_CALLBACK(ext_yahoo_login_response)(yd->client_id, YAHOO_LOGIN_LOCK, "Yahoo! website");
						break;

				case 1235: /* This ID is not yet taken */
						YAHOO_CALLBACK(ext_yahoo_login_response)(yd->client_id, YAHOO_LOGIN_UNAME, NULL);
						break;
				
				case 1221:
					c = strstr(response,"url=");
		
					if (c != NULL) {
						t = c + 6;
			
						while ( (*c) != '\0' && (*c) != '\r' && (*c) != '\n') c++;
				
						i = c - t;
						
						if (i > 1000) 
							i = 1000;
						
						strncpy(url, t, i);
						url[i] = '\0';
					} else {
						url[0] = '\0';
					}
				
					YAHOO_CALLBACK(ext_yahoo_login_response)(yd->client_id, YAHOO_LOGIN_LOCK, url);
					break;
						
				case 1214:
				case 1236: /* indicates a lock of some description */
						YAHOO_CALLBACK(ext_yahoo_login_response)(yd->client_id, YAHOO_LOGIN_LOCK, "Yahoo! website");
						break;


				case 100: /* Required field missing */
						YAHOO_CALLBACK(ext_yahoo_login_response)(yd->client_id, YAHOO_LOGIN_PASSWD, NULL);
						break;

				default:
						YAHOO_CALLBACK(ext_yahoo_login_response)(yd->client_id, YAHOO_LOGIN_SOCK, NULL);
						break;
			}
			
			FREE(response);
			return;
		}
		/*
			0  - status code. See: http://www.carbonize.co.uk/ymsg16.html
			ymsgr=<YToken>
			partnerid=<???>
		 */
		c = strstr(response,"ymsgr=");
		
		if (c != NULL) {
				t = c + 6;
			
				while ( (*c) != '\0' && (*c) != '\r' && (*c) != '\n') c++;
				
				yd->pw_token = (char *) malloc(c - t + 1);
				
				memcpy(yd->pw_token, t, c - t);
				yd->pw_token[c - t] = '\0';
				
				LOG(("Got Token: %s", yd->pw_token));
		}
		
		FREE(response);
		
		if (yd->pw_token == NULL) {
			YAHOO_CALLBACK(ext_yahoo_login_response)(yd->client_id, YAHOO_LOGIN_PASSWD, NULL);
			return; // fail for now
		}
	}
	
	//_snprintf(url, sizeof(url), "/config/pwtoken_login?src=ymsgr&token=%s&ext_err=1",token);
	_snprintf(url, sizeof(url), "/config/pwtoken_login?src=ymsgr&token=%s",yd->pw_token);
				
	/*
				0
			crumb=hN3LKzv4Ho.
			Y=v=1&n=11nh9j9k4vpm8&l=64d0xxtsqqt/o&p=m270ar7013000000&jb=33|47|&r=bt&lg=us&intl=us&np=1; path=/; domain=.yahoo.com
			T=z=xUvdFBxaEeFBfOaVlmk3RSXNDMxBjU2MjQyNjFPNTE-&a=QAE&sk=DAAWDRZBoXexNr&d=c2wBTXpRMkFUSXhOVE0xTVRZNE1qWS0BYQFRQUUBenoBeFV2ZEZCZ1dBAXRpcAFNSVlVN0Q-; path=/; domain=.yahoo.com
			cookievalidfor=86400

	 */
	response = YAHOO_CALLBACK(ext_yahoo_send_https_request)(yd, yss->login_host, url);

	if (response == NULL) {
			YAHOO_CALLBACK(ext_yahoo_login_response)(yd->client_id, YAHOO_LOGIN_SOCK, NULL);
			return; // fail for now
	}
	
	LOG(("Got response:\n%s", response));
	
	if (!isdigit(response[0])) {
		LOG(("Non numeric status code received."));
		
		YAHOO_CALLBACK(ext_yahoo_login_response)(yd->client_id, YAHOO_LOGIN_SOCK, NULL);
		return; // fail for now
	}
		
	i = atoi(response);
	
	if (i != 0) {
		/**
		 * Some Error Code, we need to process it here
		 */
		
		switch (i) {

			case 100: /* Required field missing???? */
					YAHOO_CALLBACK(ext_yahoo_login_response)(yd->client_id, YAHOO_LOGIN_SOCK, NULL);
					break;

			default:
					YAHOO_CALLBACK(ext_yahoo_login_response)(yd->client_id, YAHOO_LOGIN_SOCK, NULL);
					break;
		}
		
		FREE(response);
		return;
	}

	c = strstr(response,"crumb=");
	if (c != NULL) {
		t = c + 6;
	
		while ( (*c) != '\0' && (*c) != '\r' && (*c) != '\n') c++;
		
		crumb = (char *) _alloca(c - t + 1);
		
		memcpy(crumb, t, c - t);
		crumb[c - t] = '\0';
		
		LOG(("Got crumb: %s", crumb));
	} else 
		goto LBL_FAILED;
	
	c = strstr(response,"Y=");
	if (c != NULL) {
		t = c + 2;
	
		while ( (*c) != '\0' && (*c) != '\r' && (*c) != '\n') c++;
		
		FREE(yd->cookie_y);
		yd->cookie_y = (char *) malloc(c - t + 1);
		
		memcpy(yd->cookie_y, t, c - t);
		yd->cookie_y[c - t] = '\0';
		
		LOG(("Got Y Cookie: %s", yd->cookie_y));
	} else 
		goto LBL_FAILED;
	
	c = strstr(response,"T=");
	if (c != NULL) {
		t = c + 2;

		while ( (*c) != '\0' && (*c) != '\r' && (*c) != '\n') c++;
		
		yd->cookie_t = (char *) malloc(c - t + 1);
		
		memcpy(yd->cookie_t, t, c - t);
		yd->cookie_t[c - t] = '\0';
		
		LOG(("Got T Cookie: %s", yd->cookie_t));
	} else {
LBL_FAILED:
		FREE(response);
		
		YAHOO_CALLBACK(ext_yahoo_login_response)(yd->client_id, YAHOO_LOGIN_PASSWD, "At stage 2");
		return;
	}

	FREE(response);
		
	pack = yahoo_packet_new(YAHOO_SERVICE_AUTHRESP, (yd->initial_status == YAHOO_STATUS_INVISIBLE) ?YPACKET_STATUS_INVISIBLE:YPACKET_STATUS_WEBLOGIN, 0);
/*
		AuthResp, WebLogin
		0: id
		1: id
		277: v=....
		278: z=...
		307: <from above>
		244: 4194239
		2: id
		2: 1
		59: B\tvalue
		98: us
		135: 9.0.0.1912
*/	
	/*
	 277:v=1&n=11nh9j9k4vpm8&l=64d0xxtsqqt/o&p=m270ar7013000000&jb=33|47|&r=bt&lg=us&intl=us&np=1; path=/; domain=.yahoo.com
	 278:z=xUvdFBxaEeFBfOaVlmk3RSXNDMxBjU2MjQyNjFPNTE-&a=QAE&sk=DAAWDRZBoXexNr&d=c2wBTXpRMkFUSXhOVE0xTVRZNE1qWS0BYQFRQUUBenoBeFV2ZEZCZ1dBAXRpcAFNSVlVN0Q-; path=/; domain=.yahoo.com
	 307:VATg29jzHSXlp_2LL7J4Fw--
	*/
	mir_md5_init(&ctx);

	mir_md5_append(&ctx, (BYTE *)crumb,  strlen(crumb));
	mir_md5_append(&ctx, (BYTE *)seed,  strlen(seed));
	mir_md5_finish(&ctx, result);
	
	to_y64(magic_hash, result, 16);
	LOG(("Y64 Hash: %s", magic_hash));
	
	yahoo_packet_hash(pack, 1, sn);
	yahoo_packet_hash(pack, 0, sn);
	
	yahoo_packet_hash(pack, 277, yd->cookie_y);
	yahoo_packet_hash(pack, 278, yd->cookie_t);
	yahoo_packet_hash(pack, 307, (const char *)magic_hash);
	free(magic_hash);

	yahoo_packet_hash(pack, 244, "4194239");  // Yahoo 9.0
	
	yahoo_packet_hash(pack, 2, sn);
	yahoo_packet_hash(pack, 2, "1");
	
	yahoo_packet_hash(pack, 135, "9.0.0.2162"); 

	yahoo_send_packet(yid, pack, 0);
	yahoo_packet_free(pack);
}

static void yahoo_process_auth(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	char *seed = NULL;
	char *sn   = NULL;
	YList *l = pkt->hash;
	int m = 0;

	while (l) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;
		
		switch (pair->key) {
		case 94:
			seed = pair->value;
			break;
		case 1:
			sn = pair->value;
			break;
		case 13:
			m = atoi(pair->value);
			break;
		}
		l = l->next;
	}

	if (!seed || !sn) {
		YAHOO_CALLBACK(ext_yahoo_login_response)(yid->yd->client_id, YAHOO_LOGIN_SOCK, NULL);
		return;
	}

	switch (m) {
		case 2:
			yahoo_process_auth_0x0f(yid, seed, sn);
			break;
		default:
			/* call error */
			WARNING(("unknown auth type %d", m));
			//yahoo_process_auth_0x0b(yid, seed, sn);
			YAHOO_CALLBACK(ext_yahoo_login_response)(yid->yd->client_id, YAHOO_LOGIN_SOCK, NULL);
			break;
	}
}

static void yahoo_process_auth_resp(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	struct yahoo_data *yd = yid->yd;
	char *login_id;
	char *handle;
	char *url=NULL;
	int  login_status=-1;

	YList *l;

	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;
		if (pair->key == 0)
			login_id = pair->value;
		else if (pair->key == 1)
			handle = pair->value;
		else if (pair->key == 20)
			url = pair->value;
		else if (pair->key == 66)
			login_status = atoi(pair->value);
	}

	if (pkt->status == YPACKET_STATUS_DISCONNECTED) {
		YAHOO_CALLBACK(ext_yahoo_login_response)(yd->client_id, login_status, url);
	/*	yahoo_logoff(yd->client_id);*/
	}
}

static void yahoo_process_mail(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	struct yahoo_data *yd = yid->yd;
	char *who = NULL;
	char *email = NULL;
	char *subj = NULL;
	int count = 0;
	YList *l;

	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;
		if (pair->key == 9)
			count = strtol(pair->value, NULL, 10);
		else if (pair->key == 43)
			who = pair->value;
		else if (pair->key == 42)
			email = pair->value;
		else if (pair->key == 18)
			subj = pair->value;
		else
			LOG(("key: %d => value: '%s'", pair->key, pair->value));
	}

	if (email && subj) {
		char from[1024];
		
		if (who) {
			snprintf(from, sizeof(from), "\"%s\" <%s>", who, email);
		} else {
			snprintf(from, sizeof(from), "%s", email);
		}
		
		YAHOO_CALLBACK(ext_yahoo_mail_notify)(yd->client_id, from, subj, count);
	} else {
		YAHOO_CALLBACK(ext_yahoo_mail_notify)(yd->client_id, NULL, NULL, count);
	}
}

static void yahoo_buddy_added_us(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	struct yahoo_data *yd = yid->yd;
	char *id = NULL;
	char *who = NULL;
	char *msg = NULL;
	long tm = 0L;
	YList *l;
	int protocol = 0;

	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;
		switch (pair->key) {
		case 1:
			id = pair->value;
			break;
		case 3:
			who = pair->value;
			break;
		case 14:
			msg = pair->value;
			break;
		case 15:
			tm = strtol(pair->value, NULL, 10);
			break;

		case 241:
			protocol = strtol(pair->value, NULL, 10);
			break;

		default:
			LOG(("key: %d => value: '%s'", pair->key, pair->value));
		}
	}

	YAHOO_CALLBACK(ext_yahoo_contact_added)(yd->client_id, id, who, NULL, NULL, msg, protocol);
}

static void yahoo_buddy_denied_our_add(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	struct yahoo_data *yd = yid->yd;
	char *who = NULL;
	char *msg = NULL;
	YList *l;
	
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;
		switch (pair->key) {
		case 3:
			who = pair->value;
			break;
		case 14:
			msg = pair->value;
			break;
		default:
			LOG(("key: %d => value: '%s'", pair->key, pair->value));
		}
	}

	YAHOO_CALLBACK(ext_yahoo_rejected)(yd->client_id, who, msg);
}

static void yahoo_process_contact(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	switch (pkt->status) {
	case 1:
		yahoo_process_status(yid, pkt);
		return;
	case 3:
		yahoo_buddy_added_us(yid, pkt);
		break;
	case 7:
		yahoo_buddy_denied_our_add(yid, pkt);
		break;
	default:
		LOG(("Unknown status value: '%d'", pkt->status));
	}

}

static void yahoo_process_authorization(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	struct yahoo_data *yd = yid->yd;
	char *who = NULL,
		 *msg = NULL,
		 *fname = NULL,
		 *lname = NULL,
		 *id  = NULL;
	int state = 0, utf8 = 0, protocol = 0;
	YList *l;
	
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;
		switch (pair->key) {
		case 4: /* who added us */
			who = pair->value;
			break;

		case 5: /* our identity */
			id = pair->value;
			break;
			
		case 13: /* which type of request this is */
			state = strtol(pair->value, NULL, 10);
			break;
			
		case 14: /* was there a message ? */
			msg = pair->value;
			break;
			
		case 97: /* Unicode flag? */
			utf8 = strtol(pair->value, NULL, 10);
			break;
			
		case 216: /* first name */
			fname = pair->value;
			break;

		case 241:
			protocol = strtol(pair->value, NULL, 10);
			break;
			
		case 254: /* last name */
			lname = pair->value;
			break;
			
		default:
			LOG(("key: %d => value: '%s'", pair->key, pair->value));
		}
	}

	switch (state) {
		case 1: /* Authorization Accepted */
				
				break;
		case 2: /* Authorization Denied */
				YAHOO_CALLBACK(ext_yahoo_rejected)(yd->client_id, who, msg);
				break;
		default: /* Authorization Request? */
				YAHOO_CALLBACK(ext_yahoo_contact_added)(yd->client_id, id, who, fname, lname, msg, protocol);
			
	}

}

static void yahoo_process_buddyadd(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	struct yahoo_data *yd = yid->yd;
	char *who = NULL;
	char *where = NULL;
	int status = 0, auth = 0, protocol = 0;
	char *me = NULL;

	struct yahoo_buddy *bud=NULL;

	YList *l;
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;
		
		switch (pair->key) { 
		case 1:
			me = pair->value;
			break;
		case 7:
			who = pair->value;
			break;
		case 65:
			where = pair->value;
			break;
		case 66:
			status = strtol(pair->value, NULL, 10);
			break;
			
		case 223:
			auth = strtol(pair->value, NULL, 10);
			break;
		
		case 241:
			protocol = strtol(pair->value, NULL, 10);
			break;
		
		default:
			DEBUG_MSG(("unknown key: %d = %s", pair->key, pair->value));
		}
	}

	//yahoo_dump_unhandled(pkt);

	if (!who)
		return;
	if (!where)
		where = "Unknown";

	bud = y_new0(struct yahoo_buddy, 1);
	bud->id = strdup(who);
	bud->group = strdup(where);
	bud->real_name = NULL;
	bud->protocol = protocol;

	yd->buddies = y_list_append(yd->buddies, bud);

	YAHOO_CALLBACK(ext_yahoo_buddy_added)(yd->client_id, me, who, where, status, auth); 
/*	YAHOO_CALLBACK(ext_yahoo_status_changed)(yd->client_id, who, status, NULL, (status==YAHOO_STATUS_AVAILABLE?0:1)); */
}

static void yahoo_process_buddydel(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	struct yahoo_data *yd = yid->yd;
	char *who = NULL;
	char *where = NULL;
	int unk_66 = 0, protocol = 0;
	char *me = NULL;
	struct yahoo_buddy *bud;

	YList *buddy;

	YList *l;
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;
		
		switch (pair->key) {
			case 1:
					me = pair->value;
					break;
					
			case 7:
					who = pair->value;
					break;
					
			case 65:
					where = pair->value;
					break;
					
			case 66:
					unk_66 = strtol(pair->value, NULL, 10);
					break;
					
			case 241:
					protocol = strtol(pair->value, NULL, 10);
					break;
					
			default:
					DEBUG_MSG(("unknown key: %d = %s", pair->key, pair->value));
		}
	}

	if (!who || !where)
		return;
	
	bud 			= y_new0(struct yahoo_buddy, 1);
	bud->id 		= strdup(who);
	bud->group 		= strdup(where);
	bud->protocol 	= protocol;
	
	buddy = y_list_find_custom(yd->buddies, bud, is_same_bud);

	FREE(bud->id);
	FREE(bud->group);
	FREE(bud);

	if (buddy) {
		bud = (struct yahoo_buddy *) buddy->data;
		yd->buddies = y_list_remove_link(yd->buddies, buddy);
		y_list_free_1(buddy);

		FREE(bud->id);
		FREE(bud->group);
		FREE(bud->real_name);
		FREE(bud);

		bud=NULL;
	}
}
static void yahoo_process_yahoo7_change_group(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	struct yahoo_data *yd = yid->yd;
	char *who = NULL;
	char *me = NULL;
	char *old_group = NULL;
	char *new_group = NULL;

	YList *l;
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;
		
		switch (pair->key) { 
		case 1:
			me = pair->value;
			break;
		case 7:
			who = pair->value;
			break;
		case 224:
			old_group = pair->value;
			break;
		case 264:
			new_group = pair->value;
			break;
		}
	}

	YAHOO_CALLBACK(ext_yahoo_buddy_group_changed)(yd->client_id, me, who, old_group, new_group); 
}

static void yahoo_process_ignore(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	struct yahoo_data *yd = yid->yd;
	char *who = NULL;
	int  status = 0;
	char *me = NULL;
	int  un_ignore = 0;

	YList *l;
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;
		if (pair->key == 0)
			who = pair->value;
		if (pair->key == 1)
			me = pair->value;
		if (pair->key == 13) /* 1 == ignore, 2 == unignore */ 
			un_ignore = strtol(pair->value, NULL, 10);
		if (pair->key == 66) 
			status = strtol(pair->value, NULL, 10);
	}


	/*
	 * status
	 * 	0  - ok
	 * 	2  - already in ignore list, could not add
	 * 	3  - not in ignore list, could not delete
	 * 	12 - is a buddy, could not add
	 */

	if (status) {
		YAHOO_CALLBACK(ext_yahoo_error)(yd->client_id, who, 0, status);
	} else {
		/* we adding or removing to the ignore list */
		if (un_ignore == 1) { /* ignore */
				struct yahoo_buddy *bud = y_new0(struct yahoo_buddy, 1);
				
				bud->id = strdup(who);
				bud->group = NULL;
				bud->real_name = NULL;

				yd->ignore = y_list_append(yd->ignore, bud);
				
		} else { /* unignore */
			YList *buddy;
			
			buddy = yd->ignore;
			
			while (buddy) {
				struct yahoo_buddy *b = (struct yahoo_buddy *) buddy->data;
				
				if (lstrcmpiA(b->id, who) == 0) 
					break;
				
				buddy = buddy->next;
			}
				
			if (buddy) {
				struct yahoo_buddy *bud = (struct yahoo_buddy *) buddy->data;
				
				yd->ignore = y_list_remove_link(yd->ignore, buddy);
				y_list_free_1(buddy);
		
				FREE(bud->id);
				FREE(bud->group);
				FREE(bud->real_name);
				FREE(bud);
		
				bud=NULL;
			}	
		}
	}
}

static void yahoo_process_stealth(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	//struct yahoo_data *yd = yid->yd;
	char *who = NULL;
	int  status = 0;
	
	YList *l;
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;
		
		if (pair->key == 7)
			who = pair->value;
		
		if (pair->key == 31) 
			status = strtol(pair->value, NULL, 10);
	}

	NOTICE(("got %s stealth info for %s with value: %d", pkt->service == YAHOO_SERVICE_STEALTH_PERM ? "permanent": "session" , who, status == 1));
}

static void yahoo_process_voicechat(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	char *who = NULL;
	char *me = NULL;
	char *room = NULL;
	char *voice_room = NULL;

	YList *l;
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;
		
		if (pair->key == 4)
			who = pair->value;
		if (pair->key == 5)
			me = pair->value;
		if (pair->key == 13)
			voice_room=pair->value;
		if (pair->key == 57) 
			room=pair->value;
	}

	NOTICE(("got voice chat invite from %s in %s to identity %s", who, room, me));
	/* 
	 * send: s:0 1:me 5:who 57:room 13:1
	 * ????  s:4 5:who 10:99 19:-1615114531
	 * gotr: s:4 5:who 10:99 19:-1615114615
	 * ????  s:1 5:me 4:who 57:room 13:3room
	 * got:  s:1 5:me 4:who 57:room 13:1room
	 * rej:  s:0 1:me 5:who 57:room 13:3
	 * rejr: s:4 5:who 10:99 19:-1617114599
	 */
}

static void yahoo_process_picture(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	char *who = NULL;
	char *me = NULL;
	char *pic_url = NULL;
	int cksum = 0;
	int	type = 0;
	
	YList *l;
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;
		
		/* based on GAIM code */
		switch (pair->key) {
		case 1:
		case 4:
			who = pair->value;
			break;
		case 5:
			me = pair->value;
			break;
		case 13: 
			type = strtol(pair->value, NULL, 10);
			break;
		case 20:
			pic_url=pair->value;
			break;
		case 192:
			cksum = strtol(pair->value, NULL, 10);
			break;
		} /*switch */
		
	}
	NOTICE(("got picture packet"));
	YAHOO_CALLBACK(ext_yahoo_got_picture)(yid->yd->client_id, me, who, pic_url, cksum, type);
}

void yahoo_send_picture_info(int id, const char *who, int type, const char *pic_url, int cksum)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt = NULL;
	struct yahoo_server_settings *yss;
	
	if (!yid)
		return;

	yd = yid->yd;
	yss = yd->server_settings;
	pkt = yahoo_packet_new(YAHOO_SERVICE_PICTURE, YPACKET_STATUS_DEFAULT, yd->session_id);

	yahoo_packet_hash(pkt, 1, yd->user);
	//yahoo_packet_hash(pkt, 4, yd->user);
	yahoo_packet_hash(pkt, 5, who);
	
	yahoo_packet_hash_int(pkt, 13, type);
	
	yahoo_packet_hash(pkt, 20, pic_url);
	
	yahoo_packet_hash_int(pkt, 192, cksum);
	yahoo_send_packet(yid, pkt, 0);

	if (yss->web_messenger) {
		yahoo_packet_hash(pkt, 0, yd->user); 
		yahoo_packet_hash_int(pkt, 24, yd->session_timestamp);
	}

	yahoo_packet_free(pkt);
}

void yahoo_send_picture_update(int id, const char *who, int type)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt = NULL;
	struct yahoo_server_settings *yss;
		
	if (!yid)
		return;

	yd = yid->yd;
	yss = yd->server_settings;
	pkt = yahoo_packet_new(YAHOO_SERVICE_PICTURE_UPDATE, YPACKET_STATUS_DEFAULT, yd->session_id);
	yahoo_packet_hash(pkt, 1, yd->user);
	yahoo_packet_hash(pkt, 5, who);
		
	yahoo_packet_hash_int(pkt, 206, type);
	yahoo_send_packet(yid, pkt, 0);

	if (yss->web_messenger) {
		yahoo_packet_hash(pkt, 0, yd->user); 
		yahoo_packet_hash_int(pkt, 24, yd->session_timestamp);
	}

	yahoo_packet_free(pkt);
}

void yahoo_send_picture_checksum(int id, const char *who, int cksum)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt = NULL;
	struct yahoo_server_settings *yss;	
		
	if (!yid)
		return;

	yd = yid->yd;
	yss = yd->server_settings;
	pkt = yahoo_packet_new(YAHOO_SERVICE_PICTURE_CHECKSUM, YPACKET_STATUS_DEFAULT, yd->session_id);
	yahoo_packet_hash(pkt, 1, yd->user);
		
	if (who)
		yahoo_packet_hash(pkt, 5, who);	// ?

	yahoo_packet_hash(pkt, 212, "1");	// ?
	
	yahoo_packet_hash_int(pkt, 192, cksum);	 // checksum
	yahoo_send_packet(yid, pkt, 0);

	if (yss->web_messenger) {
		yahoo_packet_hash(pkt, 0, yd->user); 
		yahoo_packet_hash_int(pkt, 24, yd->session_timestamp);
	}

	yahoo_packet_free(pkt);
	
	/* weird YIM7 sends another packet! See picture_status below*/
}

void yahoo_send_picture_status(int id, int buddy_icon)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt = NULL;
	struct yahoo_server_settings *yss;	
		
	if (!yid)
		return;

	yd = yid->yd;
	yss = yd->server_settings;
	pkt = yahoo_packet_new(YAHOO_SERVICE_PICTURE_SHARING, YPACKET_STATUS_DEFAULT, yd->session_id);
	yahoo_packet_hash(pkt, 3, yd->user);

	yahoo_packet_hash_int(pkt, 213, buddy_icon);

	if (yss->web_messenger) {
		yahoo_packet_hash(pkt, 0, yd->user); 
		yahoo_packet_hash_int(pkt, 24, yd->session_timestamp);
	}
	
	yahoo_send_packet(yid, pkt, 0);

	yahoo_packet_free(pkt);
}

static void yahoo_process_picture_checksum(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	char *who = NULL;
	char *me = NULL;
	int cksum = 0;
	
	YList *l;
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;
		if (pair->key == 4)
			who = pair->value;
		if (pair->key == 5)
			me = pair->value;
		if (pair->key == 192) 
			cksum = strtol(pair->value, NULL, 10);
		
	}
	NOTICE(("got picture_checksum packet"));
	YAHOO_CALLBACK(ext_yahoo_got_picture_checksum)(yid->yd->client_id, me, who, cksum);
}

static void yahoo_process_picture_update(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	char *who = NULL;
	char *me = NULL;
	int buddy_icon = -1;
	
	YList *l;
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *)l->data;
		if (pair->key == 4)
			who = pair->value;
		if (pair->key == 5)
			me = pair->value;
		if (pair->key == 206) 
			buddy_icon = strtol(pair->value, NULL, 10);
		
	}
	NOTICE(("got picture_update packet"));
	YAHOO_CALLBACK(ext_yahoo_got_picture_update)(yid->yd->client_id, me, who, buddy_icon);
}

static void yahoo_process_picture_upload(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	char *url = NULL;
	char *me = NULL;
	unsigned int ts = 0;
	
	YList *l;
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *)l->data;
		switch (pair->key) {
		case 5: /* our id */
				me = pair->value;
				break;
		case 27: /* filename on our computer */
				break;
		case 20: /* url at yahoo */
				url = pair->value;
				break;
		case 38: /* timestamp */
				ts = strtol(pair->value, NULL, 10);
				break;
		}
	}
	NOTICE(("[yahoo_process_picture_upload]"));
	
	YAHOO_CALLBACK(ext_yahoo_got_picture_upload)(yid->yd->client_id, me, url, ts);
}

static void yahoo_process_picture_status(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	char *who = NULL;
	char *me = NULL;
	int buddy_icon = -1;
	
	YList *l;
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;
		switch (pair->key) {
		case 5: /* our id */
				me = pair->value;
				break;
		case 4: /* who is notifying all */
				who = pair->value;
				break;
		case 213: /* picture = 0-none, 1-?, 2=picture */
				buddy_icon = strtol(pair->value, NULL, 10);
				break;
		}
	}
	NOTICE(("[yahoo_process_picture_status]"));
	if (who) // sometimes we just get a confirmation without the WHO.(ack on our avt update)
		YAHOO_CALLBACK(ext_yahoo_got_picture_status)(yid->yd->client_id, me, who, buddy_icon);
}

static void yahoo_process_audible(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	char *who = NULL;
	char *me = NULL;
	char *aud_hash=NULL;
	char *msg = NULL;
	char *aud = NULL;
	
	YList *l;
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;
		switch (pair->key) {
		case 5: /* our id */
				me = pair->value;
				break;
		case 4: /* who is notifying all */
				who = pair->value;
				break;
		case 230: /* file class name 
					GAIM: the audible, in foo.bar.baz format
			
					Actually this is the filename.
					Full URL:
				
					http://us.dl1.yimg.com/download.yahoo.com/dl/aud/us/aud.swf 
					
					where aud in foo.bar.baz format
					*/
				aud = pair->value;
				break;
		case 231: /*audible text*/
				msg = pair->value;
				break;
		case 232: /*  weird number (md5 hash?) */
				aud_hash = pair->value;
				break;
		}
	}
	NOTICE(("[yahoo_process_audible]"));
	if (who) // sometimes we just get a confirmation without the WHO.(ack on our send/update)
		YAHOO_CALLBACK(ext_yahoo_got_audible)(yid->yd->client_id, me, who, aud, msg, aud_hash);
}

static void yahoo_process_calendar(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	char *msg = NULL;
	char *url = NULL;
	int  svc = -1, type = -1;
	
	YList *l;
	
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;
		switch (pair->key) {
		case 20: /* url to calendar reminder/event */
				if (pair->value[0] != '\0')
					url = pair->value;
				break;
		case 21: /* type? number seems to be 0? */
				type = atol(pair->value);
				break;
		case 14: /* index msg/title ? */
				if (pair->value[0] != '\0')
					msg = pair->value;
				break;
		case 13: /* service # ? */
				svc = atol(pair->value);
				break;
		}
	}

	if (url) // sometimes we just get a reminder w/o the URL
		YAHOO_CALLBACK(ext_yahoo_got_calendar)(yid->yd->client_id, url, type, msg, svc);
}


static void yahoo_process_ping(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	char *errormsg = NULL;
	
	YList *l;
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;
		if (pair->key == 16)
			errormsg = pair->value;
	}
	
	NOTICE(("got ping packet"));
	YAHOO_CALLBACK(ext_yahoo_got_ping)(yid->yd->client_id, errormsg);
}

static void yahoo_process_yab_update(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	char *who=NULL,*yentry=NULL;
	int  svc=0;
	YList *l;
	
	/*
	[15:42:00 YAHOO] Yahoo Service: (null) (0xc4) Status: YAHOO_STATUS_AVAILABLE (0)
[15:42:00 YAHOO]  
[15:42:00 YAHOO] libyahoo2/libyahoo2.c:900: debug: 
[15:42:00 YAHOO] [Reading packet] len: 309
[15:42:00 YAHOO]  
[15:42:00 YAHOO] Key: To (5)  	Value: 'xxxxxxx'
[15:42:00 YAHOO]  
[15:42:00 YAHOO] Key: (null) (203)  	Value: '<?xml version="1.0" encoding="ISO-8859-1"?>
	<ab k="aaaaaaa" cc="1" ec="1" rs="OK"><ct e="1" id="1" mt="1147894756" cr="1090811437" fn="ZZZ" ln="XXX" 
	e0="aaaa@yahoo.com" nn="AAAA" ca="Unfiled" yi="xxxxxxx" pr="0" cm="Some personal notes here." 
	imm="xxxxxx@hotmail.com"/></ab>'
[15:42:00 YAHOO]  
[15:42:00 YAHOO] Key: stat/location (13)  	Value: '1'

	*/
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;
		switch (pair->key) {
		case 5: /* who */
			who = pair->value;
			break;
		case 203:	/* yab entry */
			yentry = pair->value;
			break;
		case 13: /* type of update */
			svc = atoi(pair->value);
		}
	}
	
	NOTICE(("got YAB Update packet"));
	//YAHOO_CALLBACK(ext_yahoo_got_ping)(yid->yd->client_id, errormsg);
}

static void _yahoo_webcam_get_server_connected(int fd, int error, void *d)
{
	struct yahoo_input_data *yid = (struct yahoo_input_data *) d;
	char *who = yid->wcm->user;
	char *data = NULL;
	char *packet = NULL;
	unsigned char magic_nr[] = {0, 1, 0};
	unsigned char header_len = 8;
	unsigned int len = 0;
	unsigned int pos = 0;

	if (error || fd <= 0) {
		FREE(who);
		FREE(yid);
		return;
	}

	yid->fd = fd;
	inputs = y_list_prepend(inputs, yid);
	
	/* send initial packet */
	if (who)
		data = strdup("<RVWCFG>");
	else
		data = strdup("<RUPCFG>");
	yahoo_add_to_send_queue(yid, data, strlen(data));
	FREE(data);

	/* send data */
	if (who)
	{
		data = strdup("g=");
		data = y_string_append(data, who);
		data = y_string_append(data, "\r\n");
	} else {
		data = strdup("f=1\r\n");
	}
	len = strlen(data);
	packet = y_new0(char, header_len + len);
	packet[pos++] = header_len;
	memcpy(packet + pos, magic_nr, sizeof(magic_nr));
	pos += sizeof(magic_nr);
	pos += yahoo_put32(packet + pos, len);
	memcpy(packet + pos, data, len);
	pos += len;
	yahoo_add_to_send_queue(yid, packet, pos);
	FREE(packet);
	FREE(data);

	yid->read_tag=YAHOO_CALLBACK(ext_yahoo_add_handler)(yid->yd->client_id, fd, YAHOO_INPUT_READ, yid);
}

static void yahoo_webcam_get_server(struct yahoo_input_data *y, char *who, char *key)
{
	struct yahoo_input_data *yid = y_new0(struct yahoo_input_data, 1);
	struct yahoo_server_settings *yss = y->yd->server_settings;

	yid->type = YAHOO_CONNECTION_WEBCAM_MASTER;
	yid->yd = y->yd;
	yid->wcm = y_new0(struct yahoo_webcam, 1);
	yid->wcm->user = who?strdup(who):NULL;
	yid->wcm->direction = who?YAHOO_WEBCAM_DOWNLOAD:YAHOO_WEBCAM_UPLOAD;
	yid->wcm->key = strdup(key);

	YAHOO_CALLBACK(ext_yahoo_connect_async)(yid->yd->client_id, yss->webcam_host, yss->webcam_port, yid->type,
			_yahoo_webcam_get_server_connected, yid);

}

static YList *webcam_queue=NULL;
static void yahoo_process_webcam_key(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	char *me = NULL;
	char *key = NULL;
	char *who = NULL;

	YList *l;
	yahoo_dump_unhandled(pkt);
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = (struct yahoo_pair *) l->data;
		if (pair->key == 5)
			me = pair->value;
		if (pair->key == 61) 
			key=pair->value;
	}

	l = webcam_queue;
	if (!l)
		return;
	
	who = (char *) l->data;
	webcam_queue = y_list_remove_link(webcam_queue, webcam_queue);
	y_list_free_1(l);
	yahoo_webcam_get_server(yid, who, key);
	FREE(who);
}

static void yahoo_packet_process(struct yahoo_input_data *yid, struct yahoo_packet *pkt)
{
	//DEBUG_MSG(("yahoo_packet_process: 0x%02x", pkt->service));
	switch (pkt->service)
	{
	case YAHOO_SERVICE_LOGON:
	case YAHOO_SERVICE_Y8_STATUS_UPDATE:
		yahoo_process_logon(yid, pkt);
		break;
	case YAHOO_SERVICE_USERSTAT:
	case YAHOO_SERVICE_LOGOFF:
	case YAHOO_SERVICE_ISAWAY:
	case YAHOO_SERVICE_ISBACK:
	case YAHOO_SERVICE_GAMELOGON:
	case YAHOO_SERVICE_GAMELOGOFF:
	case YAHOO_SERVICE_IDACT:
	case YAHOO_SERVICE_IDDEACT:
	case YAHOO_SERVICE_Y6_STATUS_UPDATE:
		yahoo_process_status(yid, pkt);
		break;
	case YAHOO_SERVICE_NOTIFY:
		yahoo_process_notify(yid, pkt);
		break;
	case YAHOO_SERVICE_MESSAGE:
	case YAHOO_SERVICE_GAMEMSG:
	case YAHOO_SERVICE_SYSMESSAGE:
		yahoo_process_message(yid, pkt);
		break;
	case YAHOO_SERVICE_NEWMAIL:
		yahoo_process_mail(yid, pkt);
		break;
	case YAHOO_SERVICE_NEWCONTACT:
		yahoo_process_contact(yid, pkt);
		break;
	case YAHOO_SERVICE_LIST:
		yahoo_process_list(yid, pkt);
		break;		
	case YAHOO_SERVICE_VERIFY:
		yahoo_process_verify(yid, pkt);
		break;
	case YAHOO_SERVICE_AUTH:
		yahoo_process_auth(yid, pkt);
		break;
	case YAHOO_SERVICE_AUTHRESP:
		yahoo_process_auth_resp(yid, pkt);
		break;
	case YAHOO_SERVICE_CONFINVITE:
	case YAHOO_SERVICE_CONFADDINVITE:
	case YAHOO_SERVICE_CONFDECLINE:
	case YAHOO_SERVICE_CONFLOGON:
	case YAHOO_SERVICE_CONFLOGOFF:
	case YAHOO_SERVICE_CONFMSG:
		yahoo_process_conference(yid, pkt);
		break;
	case YAHOO_SERVICE_CHATONLINE:
	case YAHOO_SERVICE_CHATGOTO:
	case YAHOO_SERVICE_CHATJOIN:
	case YAHOO_SERVICE_CHATLEAVE:
	case YAHOO_SERVICE_CHATEXIT:
	case YAHOO_SERVICE_CHATLOGOUT:
	case YAHOO_SERVICE_CHATPING:
	case YAHOO_SERVICE_COMMENT:
		yahoo_process_chat(yid, pkt);
		break;
	case YAHOO_SERVICE_P2PFILEXFER:
	case YAHOO_SERVICE_FILETRANSFER:
		yahoo_process_filetransfer(yid, pkt);
		break;
	case YAHOO_SERVICE_ADDBUDDY:
		yahoo_process_buddyadd(yid, pkt);
		break;
	case YAHOO_SERVICE_REMBUDDY:
		yahoo_process_buddydel(yid, pkt);
		break;
	case YAHOO_SERVICE_IGNORECONTACT:
		yahoo_process_ignore(yid, pkt);
		break;
	case YAHOO_SERVICE_STEALTH_PERM:
	case YAHOO_SERVICE_STEALTH_SESSION:
		yahoo_process_stealth(yid, pkt);
		break;		
	case YAHOO_SERVICE_VOICECHAT:
		yahoo_process_voicechat(yid, pkt);
		break;
	case YAHOO_SERVICE_WEBCAM:
		yahoo_process_webcam_key(yid, pkt);
		break;
	case YAHOO_SERVICE_PING:
		yahoo_process_ping(yid, pkt);
		break;
	case YAHOO_SERVICE_PICTURE:
		yahoo_process_picture(yid, pkt);
		break;
	case YAHOO_SERVICE_PICTURE_CHECKSUM:
		yahoo_process_picture_checksum(yid, pkt);
		break;
	case YAHOO_SERVICE_PICTURE_UPDATE:
		yahoo_process_picture_update(yid, pkt);
		break;
	case YAHOO_SERVICE_PICTURE_UPLOAD:
		yahoo_process_picture_upload(yid, pkt);
		break;
	case YAHOO_SERVICE_YAB_UPDATE:
		yahoo_process_yab_update(yid, pkt);
		break;
	case YAHOO_SERVICE_PICTURE_SHARING:
		yahoo_process_picture_status(yid, pkt);
		break;
	case YAHOO_SERVICE_AUDIBLE:
		yahoo_process_audible(yid, pkt);
		break;
	case YAHOO_SERVICE_CALENDAR:
		yahoo_process_calendar(yid, pkt);
		break;
	case YAHOO_SERVICE_Y7_AUTHORIZATION:
		yahoo_process_authorization(yid, pkt);
		break;
	case YAHOO_SERVICE_Y7_FILETRANSFER:
		yahoo_process_filetransfer7(yid, pkt);
		break;
	case YAHOO_SERVICE_Y7_FILETRANSFERINFO:
		yahoo_process_filetransfer7info(yid, pkt);
		break;
	case YAHOO_SERVICE_Y7_FILETRANSFERACCEPT:
		/*
		 * We need to parse this packet
		 *
		 *  Abort is signalled via status = -1 and  66 login status = -1 with FT_TOKEN
		 */
		yahoo_process_filetransfer7accept(yid, pkt);
		break;
	case YAHOO_SERVICE_Y7_CHANGE_GROUP:
		yahoo_process_yahoo7_change_group(yid, pkt);
		break;
	case YAHOO_SERVICE_Y8_LIST:
		yahoo_process_y8_list(yid, pkt);
		break;
	case YAHOO_SERVICE_IDLE:
	case YAHOO_SERVICE_MAILSTAT:
	case YAHOO_SERVICE_CHATINVITE:
	case YAHOO_SERVICE_NEWPERSONALMAIL:
	case YAHOO_SERVICE_ADDIDENT:
	case YAHOO_SERVICE_ADDIGNORE:
	case YAHOO_SERVICE_GOTGROUPRENAME:
	case YAHOO_SERVICE_GROUPRENAME:
	case YAHOO_SERVICE_PASSTHROUGH2:
	case YAHOO_SERVICE_CHATLOGON:
	case YAHOO_SERVICE_CHATLOGOFF:
	case YAHOO_SERVICE_CHATMSG:
	case YAHOO_SERVICE_REJECTCONTACT:
	case YAHOO_SERVICE_PEERTOPEER:
		WARNING(("unhandled service 0x%02x", pkt->service));
		//yahoo_dump_unhandled(pkt);
		break;
	default:
		WARNING(("unknown service 0x%02x", pkt->service));
		//yahoo_dump_unhandled(pkt);
		break;
	}
}

static struct yahoo_packet * yahoo_getdata(struct yahoo_input_data * yid)
{
	struct yahoo_packet *pkt;
	struct yahoo_data *yd = yid->yd;
	int pos = 0;
	int pktlen;

	if (!yd)
		return NULL;

	DEBUG_MSG(("rxlen is %d", yid->rxlen));
	if (yid->rxlen < YAHOO_PACKET_HDRLEN) {
		DEBUG_MSG(("len < YAHOO_PACKET_HDRLEN"));
		return NULL;
	}

	/*DEBUG_MSG(("Dumping Packet Header:"));
	yahoo_packet_dump(yid->rxqueue + pos, YAHOO_PACKET_HDRLEN);
	DEBUG_MSG(("--- Done Dumping Packet Header ---"));*/
	{
		char *buf = (char *)(yid->rxqueue + pos);
		
		if	(buf[0] != 'Y' || buf[1] != 'M' || buf[2] != 'S' || buf[3] != 'G') {
			DEBUG_MSG(("Not a YMSG packet?"));
			return NULL;
		}
	}
	pos += 4; /* YMSG */
	pos += 2;
	pos += 2;

	pktlen = yahoo_get16(yid->rxqueue + pos); pos += 2;
	DEBUG_MSG(("%d bytes to read, rxlen is %d", pktlen, yid->rxlen));

	if (yid->rxlen < (YAHOO_PACKET_HDRLEN + pktlen)) {
		DEBUG_MSG(("len < YAHOO_PACKET_HDRLEN + pktlen"));
		return NULL;
	}

	//LOG(("reading packet"));
	//yahoo_packet_dump(yid->rxqueue, YAHOO_PACKET_HDRLEN + pktlen);

	pkt = yahoo_packet_new(YAHOO_SERVICE_LOGON, YPACKET_STATUS_DEFAULT, 0);

	pkt->service = yahoo_get16(yid->rxqueue + pos); pos += 2;
	pkt->status = yahoo_get32(yid->rxqueue + pos); pos += 4;
	pkt->id = yahoo_get32(yid->rxqueue + pos); pos += 4;

	yd->session_id = pkt->id;

	yahoo_packet_read(pkt, yid->rxqueue + pos, pktlen);

	yid->rxlen -= YAHOO_PACKET_HDRLEN + pktlen;
	//DEBUG_MSG(("rxlen == %d, rxqueue == %p", yid->rxlen, yid->rxqueue));
	if (yid->rxlen>0) {
		unsigned char *tmp = (unsigned char *) y_memdup(yid->rxqueue + YAHOO_PACKET_HDRLEN 
				+ pktlen, yid->rxlen);
		FREE(yid->rxqueue);
		yid->rxqueue = tmp;
		//DEBUG_MSG(("new rxlen == %d, rxqueue == %p", yid->rxlen, yid->rxqueue));
	} else {
		//DEBUG_MSG(("freed rxqueue == %p", yid->rxqueue));
		FREE(yid->rxqueue);
	}

	return pkt;
}

static void yahoo_yab_read(struct yab *yab, unsigned char *d, int len)
{
	char *st, *en;
	char *data = (char *)d;
	data[len]='\0';

	DEBUG_MSG(("Got yab: %s", data));
	st = en = strstr(data, "userid=\"");
	if (st) {
		st += strlen("userid=\"");
		en = strchr(st, '"'); *en++ = '\0';
		yab->id = yahoo_xmldecode(st);
	}

	st = strstr(en, "fname=\"");
	if (st) {
		st += strlen("fname=\"");
		en = strchr(st, '"'); *en++ = '\0';
		yab->fname = yahoo_xmldecode(st);
	}

	st = strstr(en, "lname=\"");
	if (st) {
		st += strlen("lname=\"");
		en = strchr(st, '"'); *en++ = '\0';
		yab->lname = yahoo_xmldecode(st);
	}

	st = strstr(en, "nname=\"");
	if (st) {
		st += strlen("nname=\"");
		en = strchr(st, '"'); *en++ = '\0';
		yab->nname = yahoo_xmldecode(st);
	}

	st = strstr(en, "email=\"");
	if (st) {
		st += strlen("email=\"");
		en = strchr(st, '"'); *en++ = '\0';
		yab->email = yahoo_xmldecode(st);
	}

	st = strstr(en, "hphone=\"");
	if (st) {
		st += strlen("hphone=\"");
		en = strchr(st, '"'); *en++ = '\0';
		yab->hphone = yahoo_xmldecode(st);
	}

	st = strstr(en, "wphone=\"");
	if (st) {
		st += strlen("wphone=\"");
		en = strchr(st, '"'); *en++ = '\0';
		yab->wphone = yahoo_xmldecode(st);
	}

	st = strstr(en, "mphone=\"");
	if (st) {
		st += strlen("mphone=\"");
		en = strchr(st, '"'); *en++ = '\0';
		yab->mphone = yahoo_xmldecode(st);
	}

	st = strstr(en, "dbid=\"");
	if (st) {
		st += strlen("dbid=\"");
		en = strchr(st, '"'); *en++ = '\0';
		yab->dbid = atoi(st);
	}
}

static struct yab * yahoo_getyab(struct yahoo_input_data *yid)
{
	struct yab *yab = NULL;
	int pos = 0, end=0;
	struct yahoo_data *yd = yid->yd;

	if (!yd)
		return NULL;

	//DEBUG_MSG(("rxlen is %d", yid->rxlen));

	if (yid->rxlen <= strlen("<record"))
		return NULL;

	/* start with <record */
	while(pos < yid->rxlen-strlen("<record")+1 
			&& memcmp(yid->rxqueue + pos, "<record", strlen("<record")))
		pos++;

	if (pos >= yid->rxlen-1)
		return NULL;

	end = pos+2;
	/* end with /> */
	while(end < yid->rxlen-strlen("/>")+1 && memcmp(yid->rxqueue + end, "/>", strlen("/>")))
		end++;

	if (end >= yid->rxlen-1)
		return NULL;

	yab = y_new0(struct yab, 1);
	yahoo_yab_read(yab, yid->rxqueue + pos, end+2-pos);
	

	yid->rxlen -= end+1;
	//DEBUG_MSG(("rxlen == %d, rxqueue == %p", yid->rxlen, yid->rxqueue));
	if (yid->rxlen>0) {
		unsigned char *tmp = (unsigned char *) y_memdup(yid->rxqueue + end + 1, yid->rxlen);
		FREE(yid->rxqueue);
		yid->rxqueue = tmp;
		//DEBUG_MSG(("new rxlen == %d, rxqueue == %p", yid->rxlen, yid->rxqueue));
	} else {
		//DEBUG_MSG(("freed rxqueue == %p", yid->rxqueue));
		FREE(yid->rxqueue);
	}


	return yab;
}

static char * yahoo_getwebcam_master(struct yahoo_input_data *yid)
{
	unsigned int pos=0;
	int len=0;
	unsigned int status=0;
	char *server=NULL;
	struct yahoo_data *yd;

	if (!yid || !yid->yd)
		return NULL;
	yd = yid->yd;

	DEBUG_MSG(("rxlen is %d", yid->rxlen));

	len = yid->rxqueue[pos++];
	if (yid->rxlen < len)
		return NULL;

	/* extract status (0 = ok, 6 = webcam not online) */
	status = yid->rxqueue[pos++];

	if (status == 0)
	{
		pos += 2; /* skip next 2 bytes */
		server =  (char *) y_memdup(yid->rxqueue+pos, 16);
		pos += 16;
	}
	else if (status == 6)
	{
		YAHOO_CALLBACK(ext_yahoo_webcam_closed)
			(yd->client_id, yid->wcm->user, 4);
	}

	/* skip rest of the data */

	yid->rxlen -= len;
	DEBUG_MSG(("rxlen == %d, rxqueue == %p", yid->rxlen, yid->rxqueue));
	if (yid->rxlen>0) {
		unsigned char *tmp = (unsigned char *) y_memdup(yid->rxqueue + pos, yid->rxlen);
		FREE(yid->rxqueue);
		yid->rxqueue = tmp;
		DEBUG_MSG(("new rxlen == %d, rxqueue == %p", yid->rxlen, yid->rxqueue));
	} else {
		DEBUG_MSG(("freed rxqueue == %p", yid->rxqueue));
		FREE(yid->rxqueue);
	}

	return server;
}

static int yahoo_get_webcam_data(struct yahoo_input_data *yid)
{
	unsigned char reason=0;
	int pos=0;
	int begin=0;
	int end=0;
	unsigned int closed=0;
	unsigned char header_len=0;
	char *who;
	int connect=0;
	struct yahoo_data *yd = yid->yd;

	if (!yd)
		return -1;

	if (!yid->wcm || !yid->wcd || !yid->rxlen)
		return -1;

	DEBUG_MSG(("rxlen is %d", yid->rxlen));

	/* if we are not reading part of image then read header */
	if (!yid->wcd->to_read)
	{
		header_len=yid->rxqueue[pos++];
		yid->wcd->packet_type=0;

		if (yid->rxlen < header_len)
			return 0;

		if (header_len >= 8)
		{
			reason = yid->rxqueue[pos++];
			/* next 2 bytes should always be 05 00 */
			pos += 2;
			yid->wcd->data_size = yahoo_get32(yid->rxqueue + pos);
			pos += 4;
			yid->wcd->to_read = yid->wcd->data_size;
		}
		if (header_len >= 13)
		{
			yid->wcd->packet_type = yid->rxqueue[pos++];
			yid->wcd->timestamp = yahoo_get32(yid->rxqueue + pos);
			pos += 4;
		}

		/* skip rest of header */
		pos = header_len;
	}

	begin = pos;
	pos += yid->wcd->to_read;
	if (pos > yid->rxlen) pos = yid->rxlen;

	/* if it is not an image then make sure we have the whole packet */
	if (yid->wcd->packet_type != 0x02) {
		if ((pos - begin) != yid->wcd->data_size) {
			yid->wcd->to_read = 0;
			return 0;
		} else {
			yahoo_packet_dump(yid->rxqueue + begin, pos - begin);
		}
	}

	DEBUG_MSG(("packet type %.2X, data length %d", yid->wcd->packet_type,
		yid->wcd->data_size));

	/* find out what kind of packet we got */
	switch (yid->wcd->packet_type)
	{
		case 0x00:
			/* user requests to view webcam (uploading) */
			if (yid->wcd->data_size &&
				yid->wcm->direction == YAHOO_WEBCAM_UPLOAD) {
				end = begin;
				while (end <= yid->rxlen &&
					yid->rxqueue[end++] != 13);
				if (end > begin)
				{
					who = (char *) y_memdup(yid->rxqueue + begin, end - begin);
					who[end - begin - 1] = 0;
					YAHOO_CALLBACK(ext_yahoo_webcam_viewer)(yd->client_id, who + 2, 2);
					FREE(who);
				}
			}

			if (yid->wcm->direction == YAHOO_WEBCAM_DOWNLOAD) {
				/* timestamp/status field */
				/* 0 = declined viewing permission */
				/* 1 = accepted viewing permission */
				if (yid->wcd->timestamp == 0) {
					YAHOO_CALLBACK(ext_yahoo_webcam_closed)(yd->client_id, yid->wcm->user, 3);
				}
			}
			break;
		case 0x01: /* status packets?? */
			/* timestamp contains status info */
			/* 00 00 00 01 = we have data?? */
			break;
		case 0x02: /* image data */
			YAHOO_CALLBACK(ext_yahoo_got_webcam_image)(yd->client_id, 
					yid->wcm->user, yid->rxqueue + begin,
					yid->wcd->data_size, pos - begin,
					yid->wcd->timestamp);
			break;
		case 0x05: /* response packets when uploading */
			if (!yid->wcd->data_size) {
				YAHOO_CALLBACK(ext_yahoo_webcam_data_request)(yd->client_id, yid->wcd->timestamp);
			}
			break;
		case 0x07: /* connection is closing */
			switch(reason)
			{
				case 0x01: /* user closed connection */
					closed = 1;
					break;
				case 0x0F: /* user cancelled permission */
					closed = 2;
					break;
			}
			YAHOO_CALLBACK(ext_yahoo_webcam_closed)(yd->client_id, yid->wcm->user, closed);
			break;
		case 0x0C: /* user connected */
		case 0x0D: /* user disconnected */
			if (yid->wcd->data_size) {
				who = (char *) y_memdup(yid->rxqueue + begin, pos - begin + 1);
				who[pos - begin] = 0;
				if (yid->wcd->packet_type == 0x0C)
					connect=1;
				else
					connect=0;
				YAHOO_CALLBACK(ext_yahoo_webcam_viewer)(yd->client_id, who, connect);
				FREE(who);
			}
			break;
		case 0x13: /* user data */
			/* i=user_ip (ip of the user we are viewing) */
			/* j=user_ext_ip (external ip of the user we */
 			/*                are viewing) */
			break;
		case 0x17: /* ?? */
			break;
	}
	yid->wcd->to_read -= pos - begin;

	yid->rxlen -= pos;
	DEBUG_MSG(("rxlen == %d, rxqueue == %p", yid->rxlen, yid->rxqueue));
	if (yid->rxlen>0) {
		unsigned char *tmp = (unsigned char *) y_memdup(yid->rxqueue + pos, yid->rxlen);
		FREE(yid->rxqueue);
		yid->rxqueue = tmp;
		DEBUG_MSG(("new rxlen == %d, rxqueue == %p", yid->rxlen, yid->rxqueue));
	} else {
		DEBUG_MSG(("freed rxqueue == %p", yid->rxqueue));
		FREE(yid->rxqueue);
	}

	/* If we read a complete packet return success */
	if (!yid->wcd->to_read)
		return 1;

	return 0;
}

int yahoo_write_ready(int id, int fd, void *data)
{
	struct yahoo_input_data *yid = (struct yahoo_input_data *) data;
	int len;
	struct data_queue *tx;

	LOG(("write callback: id=%d fd=%d data=%p", id, fd, data));
	if (!yid || !yid->txqueues)
		return -2;
	
	tx = (struct data_queue *) yid->txqueues->data;
	LOG(("writing %d bytes", tx->len));
	len = yahoo_send_data(fd, (const char *)tx->queue, MIN(1024, tx->len));

	if (len == -1 && errno == EAGAIN)
		return 1;

	if (len <= 0) {
		int e = errno;
		DEBUG_MSG(("len == %d (<= 0)", len));
		while(yid->txqueues) {
			YList *l=yid->txqueues;
			tx = (struct data_queue *) l->data;
			free(tx->queue);
			free(tx);
			yid->txqueues = y_list_remove_link(yid->txqueues, yid->txqueues);
			y_list_free_1(l);
		}
		LOG(("yahoo_write_ready(%d, %d) len < 0", id, fd));
		YAHOO_CALLBACK(ext_yahoo_remove_handler)(id, yid->write_tag);
		yid->write_tag = 0;
		errno=e;
		return 0;
	}


	tx->len -= len;
	//LOG(("yahoo_write_ready(%d, %d) tx->len: %d, len: %d", id, fd, tx->len, len));
	if (tx->len > 0) {
		unsigned char *tmp = (unsigned char *) y_memdup(tx->queue + len, tx->len);
		FREE(tx->queue);
		tx->queue = tmp;
	} else {
		YList *l=yid->txqueues;
		free(tx->queue);
		free(tx);
		yid->txqueues = y_list_remove_link(yid->txqueues, yid->txqueues);
		y_list_free_1(l);
		if (!yid->txqueues) {
			//LOG(("yahoo_write_ready(%d, %d) !txqueues", id, fd));
			YAHOO_CALLBACK(ext_yahoo_remove_handler)(id, yid->write_tag);
			yid->write_tag = 0;
		}
	}

	return 1;
}

static void yahoo_process_pager_connection(struct yahoo_input_data *yid, int over)
{
	struct yahoo_packet *pkt;
	struct yahoo_data *yd = yid->yd;
	int id = yd->client_id;

	if (over)
		return;

	while (find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER) 
			&& (pkt = yahoo_getdata(yid)) != NULL) {

		yahoo_packet_process(yid, pkt);

		yahoo_packet_free(pkt);
	}
}

static void yahoo_process_ft_connection(struct yahoo_input_data *yid, int over)
{
}

static void yahoo_process_chatcat_connection(struct yahoo_input_data *yid, int over)
{
	if (over)
		return;

	if (strstr((char*)yid->rxqueue+(yid->rxlen-20), "</content>")) {
		YAHOO_CALLBACK(ext_yahoo_chat_cat_xml)(yid->yd->client_id, (char*)yid->rxqueue);
	}
}

static void yahoo_process_yab_connection(struct yahoo_input_data *yid, int over)
{
	struct yahoo_data *yd = yid->yd;
	struct yab *yab;
	YList *buds;
	//int changed=0;
	int id = yd->client_id;
	BOOL yab_used = FALSE;

	LOG(("yahoo_process_yab_connection(over = %d) ", over));
	if (over) {
		YAHOO_CALLBACK(ext_yahoo_got_buddies)(yd->client_id, yd->buddies);
		return;
	}

	while(find_input_by_id_and_type(id, YAHOO_CONNECTION_YAB) 
			&& (yab = yahoo_getyab(yid)) != NULL) {
		if (!yab->id)
			continue;
		
		//changed=1;
		yab_used = FALSE;
		for (buds = yd->buddies; buds; buds=buds->next) {
			struct yahoo_buddy * bud = (struct yahoo_buddy *) buds->data;
			
			if (!strcmp(bud->id, yab->id)) {
				yab_used = TRUE;
				bud->yab_entry = yab;
				if (yab->nname) {
					bud->real_name = strdup(yab->nname);
				} else if (yab->fname && yab->lname) {
					bud->real_name = y_new0(char, 
							strlen(yab->fname)+
							strlen(yab->lname)+2
							);
					sprintf(bud->real_name, "%s %s",
							yab->fname, yab->lname);
				} else if (yab->fname) {
					bud->real_name = strdup(yab->fname);
				}
				break; /* for */
			}
		}
		
		if (!yab_used)
		{
			//need to free the yab entry
			FREE(yab->fname);
			FREE(yab->lname);
			FREE(yab->nname);
			FREE(yab->id);
			FREE(yab->email);
			FREE(yab->hphone);
			FREE(yab->wphone);
			FREE(yab->mphone);
			FREE(yab);
		}

	}

	//if (changed)
	//	YAHOO_CALLBACK(ext_yahoo_got_buddies)(yd->client_id, yd->buddies);
}

static void yahoo_process_search_connection(struct yahoo_input_data *yid, int over)
{
	struct yahoo_found_contact *yct=NULL;
	char *p = (char *)yid->rxqueue, *np, *cp;
	int k, n;
	int start=0, found=0, total=0;
	YList *contacts=NULL;
    struct yahoo_input_data *pyid;
    
	LOG(("[yahoo_process_search_connection] over:%d", over));
	
	pyid = find_input_by_id_and_type(yid->yd->client_id, YAHOO_CONNECTION_PAGER);

	if (!over || !pyid) {
		LOG(("yahoo_process_search_connection] ?? Not Done yet? Waiting for more packets!"));
		return;
	}

	if (p && (p=strstr(p, "\r\n\r\n"))) {
		p += 4;

		for (k = 0; (p = strchr(p, 4)) && (k < 4); k++) {
			p++;
			n = atoi(p);
			switch(k) {
				case 0: found = pyid->ys->lsearch_nfound = n; break;
				case 2: start = pyid->ys->lsearch_nstart = n; break;
				case 3: total = pyid->ys->lsearch_ntotal = n; break;
			}
		}

		if (p)
			p++;

		k=0;
		while(p && *p) {
			cp = p;
			np = strchr(p, 4);

			if (!np)
				break;
			*np = 0;
			p = np+1;

			switch(k++) {
				case 1:
					if (strlen(cp) > 2 && y_list_length(contacts) < total) {
						yct = y_new0(struct yahoo_found_contact, 1);
						contacts = y_list_append(contacts, yct);
						yct->id = cp+2;
					} else {
						*p = 0;
					}
					break;
				case 2: 
					yct->online = !strcmp(cp, "2") ? 1 : 0;
					break;
				case 3: 
					yct->gender = cp;
					break;
				case 4: 
					yct->age = atoi(cp);
					break;
				case 5: 
					if (cp != "\005")
						yct->location = cp;
					k = 0;
					break;
			}
		}
	}

	YAHOO_CALLBACK(ext_yahoo_got_search_result)(yid->yd->client_id, found, start, total, contacts);

	while(contacts) {
		YList *node = contacts;
		contacts = y_list_remove_link(contacts, node);
		free(node->data);
		y_list_free_1(node);
	}
}

static void _yahoo_webcam_connected(int fd, int error, void *d)
{
	struct yahoo_input_data *yid 	= (struct yahoo_input_data *) d;
	struct yahoo_webcam * wcm 		= yid->wcm;
	struct yahoo_data * yd 			= yid->yd;
	char conn_type[100];
	char *data=NULL;
	char *packet=NULL;
	unsigned char magic_nr[] = {1, 0, 0, 0, 1};
	unsigned header_len=0;
	unsigned int len=0;
	unsigned int pos=0;

	if (error || fd <= 0) {
		FREE(yid);
		return;
	}

	yid->fd = fd;
	inputs = y_list_prepend(inputs, yid);

	LOG(("Connected"));
	/* send initial packet */
	switch (wcm->direction)
	{
		case YAHOO_WEBCAM_DOWNLOAD:
			data = strdup("<REQIMG>");
			break;
		case YAHOO_WEBCAM_UPLOAD:	
			data = strdup("<SNDIMG>");
			break;
		default:
			return;
	}
	yahoo_add_to_send_queue(yid, data, strlen(data));
	FREE(data);

	/* send data */
	switch (wcm->direction)
	{
		case YAHOO_WEBCAM_DOWNLOAD:
			header_len = 8;
			data = strdup("a=2\r\nc=us\r\ne=21\r\nu=");
			data = y_string_append(data, yd->user);
			data = y_string_append(data, "\r\nt=");
			data = y_string_append(data, wcm->key);
			data = y_string_append(data, "\r\ni=");
			data = y_string_append(data, wcm->my_ip);
			data = y_string_append(data, "\r\ng=");
			data = y_string_append(data, wcm->user);
			data = y_string_append(data, "\r\no=w-2-5-1\r\np=");
			snprintf(conn_type, sizeof(conn_type), "%d", wcm->conn_type);
			data = y_string_append(data, conn_type);
			data = y_string_append(data, "\r\n");
			break;
		case YAHOO_WEBCAM_UPLOAD:
			header_len = 13;
			data = strdup("a=2\r\nc=us\r\nu=");
			data = y_string_append(data, yd->user);
			data = y_string_append(data, "\r\nt=");
			data = y_string_append(data, wcm->key);
			data = y_string_append(data, "\r\ni=");
			data = y_string_append(data, wcm->my_ip);
			data = y_string_append(data, "\r\no=w-2-5-1\r\np=");
			snprintf(conn_type, sizeof(conn_type), "%d", wcm->conn_type);
			data = y_string_append(data, conn_type);
			data = y_string_append(data, "\r\nb=");
			data = y_string_append(data, wcm->description);
			data = y_string_append(data, "\r\n");
			break;
	}

	len = strlen(data);
	packet = y_new0(char, header_len + len);
	packet[pos++] = header_len;
	packet[pos++] = 0;
	switch (wcm->direction)
	{
		case YAHOO_WEBCAM_DOWNLOAD:
			packet[pos++] = 1;
			packet[pos++] = 0;
			break;
		case YAHOO_WEBCAM_UPLOAD:
			packet[pos++] = 5;
			packet[pos++] = 0;
			break;
	}

	pos += yahoo_put32(packet + pos, len);
	if (wcm->direction == YAHOO_WEBCAM_UPLOAD)
	{
		memcpy(packet + pos, magic_nr, sizeof(magic_nr));
		pos += sizeof(magic_nr);
	}
	memcpy(packet + pos, data, len);
	yahoo_add_to_send_queue(yid, packet, header_len + len);
	FREE(packet);
	FREE(data);

	yid->read_tag=YAHOO_CALLBACK(ext_yahoo_add_handler)(yid->yd->client_id, yid->fd, YAHOO_INPUT_READ, yid);
}

static void yahoo_webcam_connect(struct yahoo_input_data *y)
{
	struct yahoo_webcam *wcm = y->wcm;
	struct yahoo_input_data *yid;
	struct yahoo_server_settings *yss;

	if (!wcm || !wcm->server || !wcm->key)
		return;

	yid = y_new0(struct yahoo_input_data, 1);
	yid->type = YAHOO_CONNECTION_WEBCAM;
	yid->yd = y->yd;

	/* copy webcam data to new connection */
	yid->wcm = y->wcm;
	y->wcm = NULL;

	yss = y->yd->server_settings;

	yid->wcd = y_new0(struct yahoo_webcam_data, 1);

	LOG(("Connecting to: %s:%d", wcm->server, wcm->port));
	YAHOO_CALLBACK(ext_yahoo_connect_async)(y->yd->client_id, wcm->server, wcm->port, yid->type,
			_yahoo_webcam_connected, yid);

}

static void yahoo_process_webcam_master_connection(struct yahoo_input_data *yid, int over)
{
	char* server;
	struct yahoo_server_settings *yss;

	if (over)
		return;

	server = yahoo_getwebcam_master(yid);

	if (server)
	{
		yss = yid->yd->server_settings;
		yid->wcm->server = strdup(server);
		yid->wcm->port = yss->webcam_port;
		yid->wcm->conn_type = yss->conn_type;
		yid->wcm->my_ip = strdup(yss->local_host);
		if (yid->wcm->direction == YAHOO_WEBCAM_UPLOAD)
			yid->wcm->description = strdup(yss->webcam_description);
		yahoo_webcam_connect(yid);
		FREE(server);
	}
}

static void yahoo_process_webcam_connection(struct yahoo_input_data *yid, int over)
{
	int id = yid->yd->client_id;
	int fd = yid->fd;

	if (over)
		return;

	/* as long as we still have packets available keep processing them */
	while (find_input_by_id_and_fd(id, fd) 
			&& yahoo_get_webcam_data(yid) == 1);
}

static void (*yahoo_process_connection[])(struct yahoo_input_data *, int over) = {
	yahoo_process_pager_connection,
	yahoo_process_ft_connection,
	yahoo_process_yab_connection,
	yahoo_process_webcam_master_connection,
	yahoo_process_webcam_connection,
	yahoo_process_chatcat_connection,
	yahoo_process_search_connection
};

int yahoo_read_ready(int id, int fd, void *data)
{
	struct yahoo_input_data *yid = (struct yahoo_input_data *) data;
	struct yahoo_server_settings *yss;
	char buf[4096];
	int len;

	//LOG(("read callback: id=%d fd=%d data=%p", id, fd, data));
	if (!yid)
		return -2;

	
	do {
		len = read(fd, buf, sizeof(buf));
			
		//LOG(("read callback: id=%d fd=%d len=%d", id, fd, len));
		
	} while(len == -1 && errno == EINTR);

	if (len == -1 && errno == EAGAIN)	/* we'll try again later */
		return 1;

	if (len <= 0) {
		int e = errno;
		DEBUG_MSG(("len == %d (<= 0)", len));

		if (yid->type == YAHOO_CONNECTION_PAGER) {
			
			if (yid->yd) {
				// need this to handle live connection with web_messenger set
				yss = yid->yd->server_settings;
				
				if (yss && yss->web_messenger && len == 0)
					return 1; // try again later.. just nothing here yet
			}
			
			YAHOO_CALLBACK(ext_yahoo_error)(yid->yd->client_id, "Connection closed by server", 1, E_CONNECTION);
		}

		yahoo_process_connection[yid->type](yid, 1);
		yahoo_input_close(yid);

		/* no need to return an error, because we've already fixed it */
		if (len == 0)
			return 1;

		errno=e;
		LOG(("read error: %s", strerror(errno)));
		return -1;
	}

	yid->rxqueue = y_renew(unsigned char, yid->rxqueue, len + yid->rxlen + 1);
	memcpy(yid->rxqueue + yid->rxlen, buf, len);
	yid->rxlen += len;
	yid->rxqueue[yid->rxlen] = 0; // zero terminate

	yahoo_process_connection[yid->type](yid, 0);

	return len;
}

int yahoo_init_with_attributes(const char *username, const char *password, const char *pw_token, ...)
{
	va_list ap;
	struct yahoo_data *yd;
	char *c;
	
	yd = y_new0(struct yahoo_data, 1);

	if (!yd)
		return 0;

	yd->user = strdup(username);

	/* we need to strip out @yahoo.com in case a user enters full e-mail address. 
	  NOTE: Not sure what other domains to strip out as well
	 */
	c = strstr(yd->user, "@yahoo.com");
	
	if (c != NULL) 
		(*c) = '\0';
	
	/**
	 * Lower case it in case a user uses different/mixed case
	 */
	strlwr(yd->user);
	
	yd->password = strdup(password);
	yd->pw_token = (pw_token != NULL && pw_token[0] != '\0') ? strdup(pw_token) : NULL;
	
	yd->initial_status = YAHOO_STATUS_OFFLINE;
	yd->current_status = YAHOO_STATUS_OFFLINE;

	yd->client_id = ++last_id;

	add_to_list(yd);

	va_start(ap, pw_token);
	yd->server_settings = _yahoo_assign_server_settings(ap);
	va_end(ap);

	yd->ignore = yd->buddies = NULL;
	yd->ygrp = NULL;
	
	return yd->client_id;
}

int yahoo_init(const char *username, const char *password, const char *pw_token)
{
	return yahoo_init_with_attributes(username, password, pw_token, NULL);
}

struct connect_callback_data {
	struct yahoo_data *yd;
	int tag;
	int i;
	int type;
};

static void yahoo_connected(int fd, int error, void *data)
{
	struct connect_callback_data *ccd = (struct connect_callback_data *) data;
	struct yahoo_data *yd = ccd->yd;
	struct yahoo_packet *pkt;
	struct yahoo_input_data *yid;
	struct yahoo_server_settings *yss = yd->server_settings;

	if (error) {
		if (ccd->type == YAHOO_CONNECTION_PAGER && fallback_ports[ccd->i]) {
			int tag;
			yss->pager_port = fallback_ports[ccd->i++];
			
			LOG(("[yahoo_connected] Trying port %d", yss->pager_port));
			
			tag = YAHOO_CALLBACK(ext_yahoo_connect_async)(yd->client_id, yss->pager_host, yss->pager_port, 
				ccd->type, yahoo_connected, ccd);

			if (tag > 0)
				ccd->tag=tag;
		} else {
			LOG(("[yahoo_connected] No More ports or wrong type?"));
			
			FREE(ccd);
			YAHOO_CALLBACK(ext_yahoo_login_response)(yd->client_id, YAHOO_LOGIN_SOCK, NULL);
		}
		return;
	}

	FREE(ccd);

	/* fd < 0 && error == 0 means connect was cancelled */
	if (fd < 0)
		return;

	pkt = yahoo_packet_new(YAHOO_SERVICE_VERIFY, YPACKET_STATUS_DEFAULT, 0);

	yid = y_new0(struct yahoo_input_data, 1);
	yid->yd = yd;
	yid->fd = fd;
	inputs = y_list_prepend(inputs, yid);

	yahoo_send_packet(yid, pkt, 0);

	yahoo_packet_free(pkt);

	yid->read_tag=YAHOO_CALLBACK(ext_yahoo_add_handler)(yid->yd->client_id, yid->fd, YAHOO_INPUT_READ, yid);
}

void yahoo_login(int id, enum yahoo_status initial)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	struct connect_callback_data *ccd;
	struct yahoo_server_settings *yss;

	LOG(("[yahoo_login] id: %d, initial status: %d", id, initial));
	
	if (!yd)
		return;

	yss = yd->server_settings;

	yd->initial_status = initial;

	ccd = y_new0(struct connect_callback_data, 1);
	ccd->yd = yd;
	ccd->type = YAHOO_CONNECTION_PAGER;
	YAHOO_CALLBACK(ext_yahoo_connect_async)(yd->client_id, yss->pager_host, yss->pager_port, YAHOO_CONNECTION_PAGER,
			yahoo_connected, ccd);
}


int yahoo_get_fd(int id)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	if (!yid)
		return 0;
	else
		return yid->fd;
}

void yahoo_send_im(int id, const char *from, const char *who, int protocol, const char *msg, int utf8, int buddy_icon)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_packet *pkt = NULL;
	struct yahoo_data *yd;
	struct yahoo_server_settings *yss;
	
	if (!yid)
		return;

	yd = yid->yd;
	yss = yd->server_settings;
	pkt = yahoo_packet_new(YAHOO_SERVICE_MESSAGE, YPACKET_STATUS_OFFLINE, yd->session_id);

	if (from && strcmp(from, yd->user))
		yahoo_packet_hash(pkt, 0, yd->user);
	yahoo_packet_hash(pkt, 1, from?from:yd->user);
	yahoo_packet_hash(pkt, 5, who);

	if (utf8)
		yahoo_packet_hash(pkt, 97, "1");

	yahoo_packet_hash(pkt, 14, msg);


	/* GAIM does doodle so they allow/enable imvironments (that get rejected?)
	 63 - imvironment  string;11
	 64 - imvironment enabled/allowed
			0 - enabled imwironment ;0 - no imvironment
			2 - disabled		    '' - empty cause we don;t do these
	 */
	yahoo_packet_hash(pkt, 63, "");	/* imvironment name; or ;0 (doodle;11)*/
	yahoo_packet_hash(pkt, 64, "2"); 
	
	//if (!yss->web_messenger) {
		//yahoo_packet_hash(pkt, 1002, "1"); /* YIM6+ */
		/*
		 * So yahoo swallows the packet if I sent this now?? WTF?? Taking it out
		 */
		//yahoo_packet_hash(pkt, 10093, "4"); /* YIM7? */
	//}

	yahoo_packet_hash_int(pkt, 206, buddy_icon); /* buddy_icon, 0 = none, 1=avatar?, 2=picture */
	
	if (protocol != 0) 
		yahoo_packet_hash_int(pkt, 241, protocol); 
	
	if (yss->web_messenger) {
			yahoo_packet_hash(pkt, 0, yd->user); 
			yahoo_packet_hash_int(pkt, 24, yd->session_timestamp);
	}
	
	yahoo_send_packet(yid, pkt, 0);

	yahoo_packet_free(pkt);
}

void yahoo_send_typing(int id, const char *from, const char *who, int protocol, int typ)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt = NULL;
	struct yahoo_server_settings *yss;
	
	if (!yid)
		return;

	yd = yid->yd;
	yss = yd->server_settings;
	
	pkt = yahoo_packet_new(YAHOO_SERVICE_NOTIFY, YPACKET_STATUS_NOTIFY, yd->session_id);

	yahoo_packet_hash(pkt, 49, "TYPING");
	yahoo_packet_hash(pkt, 1, from?from:yd->user);
	yahoo_packet_hash(pkt, 14, " ");
	yahoo_packet_hash(pkt, 13, typ ? "1" : "0");
	yahoo_packet_hash(pkt, 5, who);
	
	if (protocol != 0) 
		yahoo_packet_hash_int(pkt, 241, protocol);
	
	if (yss->web_messenger) {
			yahoo_packet_hash(pkt, 0, yd->user); 
			yahoo_packet_hash_int(pkt, 24, yd->session_timestamp);
	//} else {
		//yahoo_packet_hash(pkt, 1002, "1"); /* YIM6+ */
		//yahoo_packet_hash(pkt, 10093, "4"); /* YIM7+ */
	}
	
	yahoo_send_packet(yid, pkt, 0);

	yahoo_packet_free(pkt);
}

void yahoo_set_away(int id, enum yahoo_status state, const char *msg, int away)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt = NULL;
	struct yahoo_server_settings *yss;
	//int service;
	enum yahoo_status cs;

	if (!yid)
		return;

	yd = yid->yd;

	//if (yd->current_status == state && state != YAHOO_STATUS_CUSTOM)
	//	return;
	
	cs = yd->current_status;
	yss = yd->server_settings;
	
	if (state == YAHOO_STATUS_INVISIBLE) {
		pkt = yahoo_packet_new(YAHOO_SERVICE_Y6_VISIBLE_TOGGLE, YPACKET_STATUS_DEFAULT, yd->session_id);
		yahoo_packet_hash(pkt, 13, "2");
		yd->current_status = state;
	} else {
		LOG(("yahoo_set_away: state: %d, msg: %s, away: %d", state, msg, away));
		
		if (msg) {
			yd->current_status = YAHOO_STATUS_CUSTOM;
		} else {
			yd->current_status = state;
		}
	
		//if (yd->current_status == YAHOO_STATUS_AVAILABLE)
		//	service = YAHOO_SERVICE_ISBACK;
		//else
		//	service = YAHOO_SERVICE_ISAWAY;
		 
		pkt = yahoo_packet_new(YAHOO_SERVICE_Y6_STATUS_UPDATE, YPACKET_STATUS_DEFAULT, yd->session_id);
		if ((away == 2) && (yd->current_status == YAHOO_STATUS_AVAILABLE)) {
			//pkt = yahoo_packet_new(YAHOO_SERVICE_ISAWAY, YAHOO_STATUS_BRB, yd->session_id);
			yahoo_packet_hash(pkt, 10, "999");
			yahoo_packet_hash(pkt, 47, "2");
		}else {
			//pkt = yahoo_packet_new(YAHOO_SERVICE_YAHOO6_STATUS_UPDATE, YAHOO_STATUS_AVAILABLE, yd->session_id);
			yahoo_packet_hash_int(pkt, 10, yd->current_status);
			
			if (yd->current_status == YAHOO_STATUS_CUSTOM) {
				yahoo_packet_hash(pkt, 19, msg);
				yahoo_packet_hash(pkt, 97, "1");
				yahoo_packet_hash(pkt, 47, (away == 2)? "2": (away) ?"1":"0");
				yahoo_packet_hash(pkt, 187, "0"); // ???
			} else {
				yahoo_packet_hash(pkt, 19, "");
				yahoo_packet_hash(pkt, 97, "1");
				//yahoo_packet_hash(pkt, 47, (away == 2)? "2": (away) ?"1":"0");
			}
			
			
			
		}
	}
		if (yss->web_messenger) {
			yahoo_packet_hash(pkt, 0, yd->user); 
			yahoo_packet_hash_int(pkt, 24, yd->session_timestamp);
		}
	
	yahoo_send_packet(yid, pkt, 0);
	yahoo_packet_free(pkt);
	
	if (cs == YAHOO_STATUS_INVISIBLE && state != YAHOO_STATUS_INVISIBLE) {
		pkt = yahoo_packet_new(YAHOO_SERVICE_Y6_VISIBLE_TOGGLE, YPACKET_STATUS_DEFAULT, yd->session_id);
		yahoo_packet_hash(pkt, 13, "1");
		yd->current_status = state;

		yahoo_send_packet(yid, pkt, 0);
		yahoo_packet_free(pkt);
	} 
}

void yahoo_set_stealth(int id, const char *buddy, int protocol, int add)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt = NULL;
	//int service;
	//char s[4];

	if (!yid)
		return;

	yd = yid->yd;

	pkt = yahoo_packet_new(YAHOO_SERVICE_STEALTH_PERM, YPACKET_STATUS_DEFAULT, yd->session_id);
	yahoo_packet_hash(pkt, 1, yd->user); 
	yahoo_packet_hash(pkt, 31, add ? "1" : "2"); /*visibility? */
	yahoo_packet_hash(pkt, 13, "2");	// function/service
	
	yahoo_packet_hash(pkt, 302, "319");
	yahoo_packet_hash(pkt, 300, "319");
	
	yahoo_packet_hash(pkt, 7, buddy);
	
	if (protocol != 0)
		yahoo_packet_hash_int(pkt, 241, protocol);
	
	yahoo_packet_hash(pkt, 301, "319");
	yahoo_packet_hash(pkt, 303, "319");
	
	yahoo_send_packet(yid, pkt, 0);
	yahoo_packet_free(pkt);
}

void yahoo_logoff(int id)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt = NULL;

	if (!yid)
		return;
	yd = yid->yd;

	LOG(("yahoo_logoff: current status: %d", yd->current_status));

	if (yd->current_status != YAHOO_STATUS_OFFLINE) {
		struct yahoo_server_settings *yss = yd->server_settings;
		
		pkt = yahoo_packet_new(YAHOO_SERVICE_LOGOFF, YPACKET_STATUS_DEFAULT, yd->session_id);
		
		if (yss->web_messenger) {
			yahoo_packet_hash(pkt, 0, yd->user); 
			yahoo_packet_hash_int(pkt, 24, yd->session_timestamp);
		}
		
		yd->current_status = YAHOO_STATUS_OFFLINE;

		if (pkt) {
			yahoo_send_packet(yid, pkt, 0);
			yahoo_packet_free(pkt);
		}
	}

	
/*	do {
		yahoo_input_close(yid);
	} while((yid = find_input_by_id(id)));*/
	
}

void yahoo_get_list(int id)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt = NULL;

	if (!yid)
		return;
	yd = yid->yd;

	pkt = yahoo_packet_new(YAHOO_SERVICE_LIST, YPACKET_STATUS_DEFAULT, yd->session_id);
	yahoo_packet_hash(pkt, 1, yd->user);
	if (pkt) {
		yahoo_send_packet(yid, pkt, 0);
		yahoo_packet_free(pkt);
	}
}

static void _yahoo_http_connected(int id, int fd, int error, void *data)
{
	struct yahoo_input_data *yid = (struct yahoo_input_data *) data;
	if (fd <= 0) {
		inputs = y_list_remove(inputs, yid);
		FREE(yid);
		return;
	}

	yid->fd = fd;
	yid->read_tag=YAHOO_CALLBACK(ext_yahoo_add_handler)(yid->yd->client_id, fd, YAHOO_INPUT_READ, yid);
}

void yahoo_get_yab(int id)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	struct yahoo_input_data *yid;
	char url[1024];
	char buff[1024];

	if (!yd)
		return;

	yid = y_new0(struct yahoo_input_data, 1);
	yid->yd = yd;
	yid->type = YAHOO_CONNECTION_YAB;

	snprintf(url, 1024, "http://insider.msg.yahoo.com/ycontent/?ab2=0");

	snprintf(buff, sizeof(buff), "Y=%s; T=%s",
			yd->cookie_y, yd->cookie_t);

	inputs = y_list_prepend(inputs, yid);

	//yahoo_http_get(yid->yd->client_id, url, buff, 
	//		_yahoo_http_connected, yid);
	YAHOO_CALLBACK(ext_yahoo_send_http_request)(yid->yd->client_id, YAHOO_CONNECTION_YAB, "GET", url, buff, 0, 
			_yahoo_http_connected, yid);
}

void yahoo_set_yab(int id, struct yab * yab)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	struct yahoo_input_data *yid;
	char url[1024];
	char buff[1024];
	char *temp;
	int size = sizeof(url)-1;

	if (!yd)
		return;

	yid = y_new0(struct yahoo_input_data, 1);
	yid->type = YAHOO_CONNECTION_YAB;
	yid->yd = yd;

	strncpy(url, "http://insider.msg.yahoo.com/ycontent/?addab2=0", size);

	if (yab->dbid) {
		/* change existing yab */
		char tmp[32];
		strncat(url, "&ee=1&ow=1&id=", size - strlen(url));
		snprintf(tmp, sizeof(tmp), "%d", yab->dbid);
		strncat(url, tmp, size - strlen(url));
	}

	if (yab->fname) {
		strncat(url, "&fn=", size - strlen(url));
		temp = yahoo_urlencode(yab->fname);
		strncat(url, temp, size - strlen(url));
		free(temp);
	}
	if (yab->lname) {
		strncat(url, "&ln=", size - strlen(url));
		temp = yahoo_urlencode(yab->lname);
		strncat(url, temp, size - strlen(url));
		free(temp);
	}
	strncat(url, "&yid=", size - strlen(url));
	temp = yahoo_urlencode(yab->id);
	strncat(url, temp, size - strlen(url));
	free(temp);
	if (yab->nname) {
		strncat(url, "&nn=", size - strlen(url));
		temp = yahoo_urlencode(yab->nname);
		strncat(url, temp, size - strlen(url));
		free(temp);
	}
	if (yab->email) {
		strncat(url, "&e=", size - strlen(url));
		temp = yahoo_urlencode(yab->email);
		strncat(url, temp, size - strlen(url));
		free(temp);
	}
	if (yab->hphone) {
		strncat(url, "&hp=", size - strlen(url));
		temp = yahoo_urlencode(yab->hphone);
		strncat(url, temp, size - strlen(url));
		free(temp);
	}
	if (yab->wphone) {
		strncat(url, "&wp=", size - strlen(url));
		temp = yahoo_urlencode(yab->wphone);
		strncat(url, temp, size - strlen(url));
		free(temp);
	}
	if (yab->mphone) {
		strncat(url, "&mp=", size - strlen(url));
		temp = yahoo_urlencode(yab->mphone);
		strncat(url, temp, size - strlen(url));
		free(temp);
	}
	strncat(url, "&pp=0", size - strlen(url));

	snprintf(buff, sizeof(buff), "Y=%s; T=%s",
			yd->cookie_y, yd->cookie_t);

	inputs = y_list_prepend(inputs, yid);

//	yahoo_http_get(yid->yd->client_id, url, buff, 
//			_yahoo_http_connected, yid);

	YAHOO_CALLBACK(ext_yahoo_send_http_request)(yid->yd->client_id, YAHOO_CONNECTION_YAB, "GET", url, buff, 0, 
			_yahoo_http_connected, yid);
}

void yahoo_set_identity_status(int id, const char * identity, int active)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt = NULL;

	if (!yid)
		return;
	yd = yid->yd;

	pkt = yahoo_packet_new(active?YAHOO_SERVICE_IDACT:YAHOO_SERVICE_IDDEACT,
			YPACKET_STATUS_DEFAULT, yd->session_id);
	yahoo_packet_hash(pkt, 3, identity);
	if (pkt) {
		yahoo_send_packet(yid, pkt, 0);
		yahoo_packet_free(pkt);
	}
}

void yahoo_refresh(int id)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt = NULL;

	if (!yid)
		return;
	yd = yid->yd;

	pkt = yahoo_packet_new(YAHOO_SERVICE_USERSTAT, YPACKET_STATUS_DEFAULT, yd->session_id);
	if (pkt) {
		yahoo_send_packet(yid, pkt, 0);
		yahoo_packet_free(pkt);
	}
}

void yahoo_send_ping(int id)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt=NULL;
	if (!yid)
		return;
	yd = yid->yd;

	pkt = yahoo_packet_new(YAHOO_SERVICE_PING, YPACKET_STATUS_DEFAULT, yd->session_id);
	yahoo_send_packet(yid, pkt, 0);
	yahoo_packet_free(pkt);
}

void yahoo_keepalive(int id)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt=NULL;
	if (!yid)
		return;
	yd = yid->yd;

	pkt = yahoo_packet_new(YAHOO_SERVICE_KEEPALIVE, YPACKET_STATUS_DEFAULT, yd->session_id);
	yahoo_packet_hash(pkt, 0, yd->user);
	yahoo_send_packet(yid, pkt, 0);
	yahoo_packet_free(pkt);
}

void yahoo_chat_keepalive (int id)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type (id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt = NULL;

	if (!yid)
	    return;

	yd = yid->yd;

	pkt = yahoo_packet_new (YAHOO_SERVICE_CHATPING, YPACKET_STATUS_DEFAULT, yd->session_id);
	yahoo_send_packet (yid, pkt, 0);
	yahoo_packet_free (pkt);
}

void yahoo_add_buddy(int id, const char *myid, const char *fname, const char *lname, const char *who, int protocol, const char *group, const char *msg)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt;

	if (!yid)
		return;
	yd = yid->yd;

	if (!yd->logged_in)
		return;

	pkt = yahoo_packet_new(YAHOO_SERVICE_ADDBUDDY, YPACKET_STATUS_DEFAULT, yd->session_id);
	yahoo_packet_hash(pkt, 14, (msg != NULL) ? msg : "");
	yahoo_packet_hash(pkt, 65, group);
	yahoo_packet_hash(pkt, 97, "1");
	
	if (fname != NULL)
		yahoo_packet_hash(pkt, 216, fname); 
	
	if (lname != NULL)
		yahoo_packet_hash(pkt, 254, lname); 
		
	yahoo_packet_hash(pkt, 1, myid ? myid : yd->user); // identity with which we are adding the user.
	yahoo_packet_hash(pkt, 302, "319");
	yahoo_packet_hash(pkt, 300, "319");
	yahoo_packet_hash(pkt, 7, who);
	//yahoo_packet_hash(pkt, 334, "0");
	
	if (protocol != 0) {
		yahoo_packet_hash_int(pkt, 241, protocol);
	}
	
	yahoo_packet_hash(pkt, 301, "319");
	yahoo_packet_hash(pkt, 303, "319");
	
	
	/* YIM7 does something weird here:
		yahoo_packet_hash(pkt, 1, yd->user);	
		yahoo_packet_hash(pkt, 14, msg != NULL ? msg : "");
		yahoo_packet_hash(pkt, 65, group);
		yahoo_packet_hash(pkt, 97, 1); ?????
		yahoo_packet_hash(pkt, 216, "First Name");???
		yahoo_packet_hash(pkt, 254, "Last Name");???
		yahoo_packet_hash(pkt, 7, who);
	
		Server Replies with:
		1: ID
		66: 0
		 7: who
		65: group
		223: 1     ??
	*/
	yahoo_send_packet(yid, pkt, 0);
	yahoo_packet_free(pkt);
}

void yahoo_remove_buddy(int id, const char *who, int protocol, const char *group)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt = NULL;
	
	if (!yid)
		return;
	yd = yid->yd;

	pkt = yahoo_packet_new(YAHOO_SERVICE_REMBUDDY, YPACKET_STATUS_DEFAULT, yd->session_id);

	yahoo_packet_hash(pkt, 1, yd->user);
	yahoo_packet_hash(pkt, 7, who);
	yahoo_packet_hash(pkt, 65, group);
	//yahoo_packet_hash(pkt, 66, "0"); // Yahoo 9.0 does login status 0?? What for?
	
	if (protocol != 0)
		yahoo_packet_hash_int(pkt, 241, protocol);
	
	yahoo_send_packet(yid, pkt, 0);
	yahoo_packet_free(pkt);
}

void yahoo_accept_buddy(int id, const char *myid, const char *who, int protocol)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt;
	
	if (!yid)
		return;
	yd = yid->yd;

	if (!yd->logged_in)
		return;

	pkt = yahoo_packet_new(YAHOO_SERVICE_Y7_AUTHORIZATION, YPACKET_STATUS_DEFAULT, yd->session_id);
	yahoo_packet_hash(pkt, 1, myid ? myid : yd->user);
	yahoo_packet_hash(pkt, 5, who);
	
	if (protocol != 0)
		yahoo_packet_hash_int(pkt, 241, protocol); 
	
	yahoo_packet_hash(pkt, 13, "1"); // Accept Authorization
	
	// Y8 also send 334: 0 - I guess that's the protocol stuff
	yahoo_send_packet(yid, pkt, 0);
	yahoo_packet_free(pkt);
}

void yahoo_reject_buddy(int id, const char *myid, const char *who, int protocol, const char *msg)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt;
	
	if (!yid)
		return;
	yd = yid->yd;

	if (!yd->logged_in)
		return;

	pkt = yahoo_packet_new(YAHOO_SERVICE_Y7_AUTHORIZATION, YPACKET_STATUS_DEFAULT, yd->session_id);
	yahoo_packet_hash(pkt, 1, myid ? myid : yd->user);
	yahoo_packet_hash(pkt, 5, who);
	yahoo_packet_hash(pkt, 13, "2"); // Reject Authorization
	
	if (msg != NULL)
		yahoo_packet_hash(pkt, 14, msg);
	
	if (protocol != 0)
		yahoo_packet_hash_int(pkt, 241, protocol); 
	
	yahoo_send_packet(yid, pkt, 0);
	yahoo_packet_free(pkt);
}

void yahoo_ignore_buddy(int id, const char *who, int unignore)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt;

	if (!yid)
		return;
	yd = yid->yd;

	if (!yd->logged_in)
		return;

	pkt = yahoo_packet_new(YAHOO_SERVICE_IGNORECONTACT, YPACKET_STATUS_DEFAULT, yd->session_id);
	yahoo_packet_hash(pkt, 1, yd->user);
	yahoo_packet_hash(pkt, 7, who);
	yahoo_packet_hash(pkt, 13, unignore?"2":"1");
	yahoo_send_packet(yid, pkt, 0);
	yahoo_packet_free(pkt);
}

void yahoo_change_buddy_group(int id, const char *who, const char *old_group, const char *new_group)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt = NULL;

	if (!yid)
		return;
	yd = yid->yd;

	/*pkt = yahoo_packet_new(YAHOO_SERVICE_ADDBUDDY, YPACKET_STATUS_DEFAULT, yd->session_id);
	yahoo_packet_hash(pkt, 1, yd->user);
	yahoo_packet_hash(pkt, 7, who);
	yahoo_packet_hash(pkt, 14, "");
	yahoo_packet_hash(pkt, 65, new_group);
	yahoo_packet_hash(pkt, 97, "1");
	yahoo_send_packet(yid, pkt, 0);
	yahoo_packet_free(pkt);

	pkt = yahoo_packet_new(YAHOO_SERVICE_REMBUDDY, YPACKET_STATUS_DEFAULT, yd->session_id);
	yahoo_packet_hash(pkt, 1, yd->user);
	yahoo_packet_hash(pkt, 7, who);
	yahoo_packet_hash(pkt, 65, old_group);
	yahoo_send_packet(yid, pkt, 0);
	yahoo_packet_free(pkt);*/
	
	pkt = yahoo_packet_new(YAHOO_SERVICE_Y7_CHANGE_GROUP, YPACKET_STATUS_DEFAULT, yd->session_id);
	yahoo_packet_hash(pkt, 1, yd->user);
	yahoo_packet_hash(pkt, 302, "240"); //???
	yahoo_packet_hash(pkt, 300, "240"); //???
	yahoo_packet_hash(pkt, 7, who);
	yahoo_packet_hash(pkt, 224, old_group);
	yahoo_packet_hash(pkt, 264, new_group);
	yahoo_packet_hash(pkt, 301, "240"); //???
	yahoo_packet_hash(pkt, 303, "240"); //???
	yahoo_send_packet(yid, pkt, 0);
	yahoo_packet_free(pkt);
}

void yahoo_group_rename(int id, const char *old_group, const char *new_group)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt = NULL;

	if (!yid)
		return;
	yd = yid->yd;

	pkt = yahoo_packet_new(YAHOO_SERVICE_GROUPRENAME, YPACKET_STATUS_DEFAULT, yd->session_id);
	yahoo_packet_hash(pkt, 1, yd->user);
	yahoo_packet_hash(pkt, 65, old_group);
	yahoo_packet_hash(pkt, 67, new_group);

	yahoo_send_packet(yid, pkt, 0);
	yahoo_packet_free(pkt);
}

void yahoo_conference_addinvite(int id, const char * from, const char *who, const char *room, const YList * members, const char *msg)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt;
		
	if (!yid)
		return;
	yd = yid->yd;

	pkt = yahoo_packet_new(YAHOO_SERVICE_CONFADDINVITE, YPACKET_STATUS_DEFAULT, yd->session_id);

	yahoo_packet_hash(pkt, 1, (from?from:yd->user));
	yahoo_packet_hash(pkt, 51, who);
	yahoo_packet_hash(pkt, 57, room);
	yahoo_packet_hash(pkt, 58, msg);
	yahoo_packet_hash(pkt, 13, "0");
	for (; members; members = members->next) {
		yahoo_packet_hash(pkt, 52, (char *)members->data);
		yahoo_packet_hash(pkt, 53, (char *)members->data);
	}
	/* 52, 53 -> other members? */

	yahoo_send_packet(yid, pkt, 0);

	yahoo_packet_free(pkt);
}

void yahoo_conference_invite(int id, const char * from, YList *who, const char *room, const char *msg)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt;
		
	if (!yid)
		return;
	yd = yid->yd;

	pkt = yahoo_packet_new(YAHOO_SERVICE_CONFINVITE, YPACKET_STATUS_DEFAULT, yd->session_id);

	yahoo_packet_hash(pkt, 1, (from?from:yd->user));
	yahoo_packet_hash(pkt, 50, yd->user);
	for (; who; who = who->next) {
		yahoo_packet_hash(pkt, 52, (char *)who->data);
	}
	yahoo_packet_hash(pkt, 57, room);
	yahoo_packet_hash(pkt, 58, msg);
	yahoo_packet_hash(pkt, 13, "0");

	yahoo_send_packet(yid, pkt, 0);

	yahoo_packet_free(pkt);
}

void yahoo_conference_logon(int id, const char *from, YList *who, const char *room)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt;
		
	if (!yid)
		return;
	
	yd = yid->yd;

	pkt = yahoo_packet_new(YAHOO_SERVICE_CONFLOGON, YPACKET_STATUS_DEFAULT, yd->session_id);

	yahoo_packet_hash(pkt, 1, (from?from:yd->user));
	
	yahoo_packet_hash(pkt, 57, room);

	for (; who; who = who->next) {
		yahoo_packet_hash(pkt, 3, (char *)who->data);
	}
	
	yahoo_send_packet(yid, pkt, 0);

	yahoo_packet_free(pkt);
}

void yahoo_conference_decline(int id, const char * from, YList *who, const char *room, const char *msg)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt;
		
	if (!yid)
		return;
	yd = yid->yd;

	pkt = yahoo_packet_new(YAHOO_SERVICE_CONFDECLINE, YPACKET_STATUS_DEFAULT, yd->session_id);

	yahoo_packet_hash(pkt, 1, (from?from:yd->user));
	for (; who; who = who->next) {
		yahoo_packet_hash(pkt, 3, (char *)who->data);
	}
	yahoo_packet_hash(pkt, 57, room);
	yahoo_packet_hash(pkt, 14, msg);

	yahoo_send_packet(yid, pkt, 0);

	yahoo_packet_free(pkt);
}

void yahoo_conference_logoff(int id, const char * from, YList *who, const char *room)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt;
		
	if (!yid)
		return;
	yd = yid->yd;

	pkt = yahoo_packet_new(YAHOO_SERVICE_CONFLOGOFF, YPACKET_STATUS_DEFAULT, yd->session_id);

	yahoo_packet_hash(pkt, 1, (from?from:yd->user));
	for (; who; who = who->next) {
		yahoo_packet_hash(pkt, 3, (char *)who->data);
	}
	yahoo_packet_hash(pkt, 57, room);

	yahoo_send_packet(yid, pkt, 0);

	yahoo_packet_free(pkt);
}

void yahoo_conference_message(int id, const char * from, YList *who, const char *room, const char *msg, int utf8)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt;
		
	if (!yid)
		return;
	yd = yid->yd;

	pkt = yahoo_packet_new(YAHOO_SERVICE_CONFMSG, YPACKET_STATUS_DEFAULT, yd->session_id);

	yahoo_packet_hash(pkt, 1, (from?from:yd->user));
	for (; who; who = who->next) {
		yahoo_packet_hash(pkt, 53, (char *)who->data);
	}
	yahoo_packet_hash(pkt, 57, room);
	yahoo_packet_hash(pkt, 14, msg);

	if (utf8)
		yahoo_packet_hash(pkt, 97, "1");

	yahoo_send_packet(yid, pkt, 0);

	yahoo_packet_free(pkt);
}

void yahoo_get_chatrooms(int id, int chatroomid)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	struct yahoo_input_data *yid;
	char url[1024];
	char buff[1024];

	if (!yd)
		return;

	yid = y_new0(struct yahoo_input_data, 1);
	yid->yd = yd;
	yid->type = YAHOO_CONNECTION_CHATCAT;

	if (chatroomid == 0) {
		snprintf(url, 1024, "http://insider.msg.yahoo.com/ycontent/?chatcat=0");
	} else {
		snprintf(url, 1024, "http://insider.msg.yahoo.com/ycontent/?chatroom_%d=0",chatroomid);
	}

	snprintf(buff, sizeof(buff), "Y=%s; T=%s", yd->cookie_y, yd->cookie_t);

	inputs = y_list_prepend(inputs, yid);

	//yahoo_http_get(yid->yd->client_id, url, buff, _yahoo_http_connected, yid);
	YAHOO_CALLBACK(ext_yahoo_send_http_request)(yid->yd->client_id, YAHOO_CONNECTION_CHATCAT, "GET", url, buff, 0, 
			_yahoo_http_connected, yid);

}

void yahoo_chat_logon(int id, const char *from, const char *room, const char *roomid)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt;
		
	if (!yid)
		return;

	yd = yid->yd;

	pkt = yahoo_packet_new(YAHOO_SERVICE_CHATONLINE, YPACKET_STATUS_DEFAULT, yd->session_id);

	yahoo_packet_hash(pkt, 1, (from?from:yd->user));
	yahoo_packet_hash(pkt, 109, yd->user);
	yahoo_packet_hash(pkt, 6, "abcde");

	yahoo_send_packet(yid, pkt, 0);

	yahoo_packet_free(pkt);

	pkt = yahoo_packet_new(YAHOO_SERVICE_CHATJOIN, YPACKET_STATUS_DEFAULT, yd->session_id);

	yahoo_packet_hash(pkt, 1, (from?from:yd->user));
	yahoo_packet_hash(pkt, 104, room);
	yahoo_packet_hash(pkt, 129, roomid);
	yahoo_packet_hash(pkt, 62, "2"); /* ??? */

	yahoo_send_packet(yid, pkt, 0);

	yahoo_packet_free(pkt);
}


void  yahoo_chat_message(int id, const char *from, const char *room, const char *msg, const int msgtype, const int utf8)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt;
		
	if (!yid)
		return;

	yd = yid->yd;

	pkt = yahoo_packet_new(YAHOO_SERVICE_COMMENT, YPACKET_STATUS_DEFAULT, yd->session_id);

	yahoo_packet_hash(pkt, 1, (from?from:yd->user));
	yahoo_packet_hash(pkt, 104, room);
	yahoo_packet_hash(pkt, 117, msg);
	
	yahoo_packet_hash_int(pkt, 124, msgtype);

	if (utf8)
		yahoo_packet_hash(pkt, 97, "1");

	yahoo_send_packet(yid, pkt, 0);

	yahoo_packet_free(pkt);
}


void yahoo_chat_logoff(int id, const char *from)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt;
		
	if (!yid)
		return;

	yd = yid->yd;

	pkt = yahoo_packet_new(YAHOO_SERVICE_CHATLOGOUT, YPACKET_STATUS_DEFAULT, yd->session_id);

	yahoo_packet_hash(pkt, 1, (from?from:yd->user));

	yahoo_send_packet(yid, pkt, 0);

	yahoo_packet_free(pkt);
}

void yahoo_webcam_close_feed(int id, const char *who)
{
	struct yahoo_input_data *yid = find_input_by_id_and_webcam_user(id, who);

	if (yid)
		yahoo_input_close(yid);
}

void yahoo_webcam_get_feed(int id, const char *who)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt;
		
	if (!yid)
		return;

	/* 
	 * add the user to the queue.  this is a dirty hack, since
	 * the yahoo server doesn't tell us who's key it's returning,
	 * we have to just hope that it sends back keys in the same 
	 * order that we request them.
	 * The queue is popped in yahoo_process_webcam_key
	 */
	webcam_queue = y_list_append(webcam_queue, who?strdup(who):NULL);

	yd = yid->yd;

	pkt = yahoo_packet_new(YAHOO_SERVICE_WEBCAM, YPACKET_STATUS_DEFAULT, yd->session_id);

	yahoo_packet_hash(pkt, 1, yd->user);
	if (who != NULL)
		yahoo_packet_hash(pkt, 5, who);
	yahoo_send_packet(yid, pkt, 0);

	yahoo_packet_free(pkt);
}

void yahoo_webcam_send_image(int id, unsigned char *image, unsigned int length, unsigned int timestamp)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_WEBCAM);
	unsigned char *packet;
	unsigned char header_len = 13;
	unsigned int pos = 0;

	if (!yid)
		return;

	packet = y_new0(unsigned char, header_len);

	packet[pos++] = header_len;
	packet[pos++] = 0;
	packet[pos++] = 5; /* version byte?? */
	packet[pos++] = 0;
	pos += yahoo_put32(packet + pos, length);
	packet[pos++] = 2; /* packet type, image */
	pos += yahoo_put32(packet + pos, timestamp);
	yahoo_add_to_send_queue(yid, packet, header_len);
	FREE(packet);

	if (length)
		yahoo_add_to_send_queue(yid, image, length);
}

void yahoo_webcam_accept_viewer(int id, const char* who, int accept)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_WEBCAM);
	char *packet = NULL;
	char *data = NULL;
	unsigned char header_len = 13;
	unsigned int pos = 0;
	unsigned int len = 0;

	if (!yid)
		return;

	data = strdup("u=");
	data = y_string_append(data, (char*)who);
	data = y_string_append(data, "\r\n");
	len = strlen(data);

	packet = y_new0(char, header_len + len);
	packet[pos++] = header_len;
	packet[pos++] = 0;
	packet[pos++] = 5; /* version byte?? */
	packet[pos++] = 0;
	pos += yahoo_put32(packet + pos, len);
	packet[pos++] = 0; /* packet type */
	pos += yahoo_put32(packet + pos, accept);
	memcpy(packet + pos, data, len);
	FREE(data);
	yahoo_add_to_send_queue(yid, packet, header_len + len);
	FREE(packet);
}

void yahoo_webcam_invite(int id, const char *who)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_packet *pkt;
		
	if (!yid)
		return;

	pkt = yahoo_packet_new(YAHOO_SERVICE_NOTIFY, YPACKET_STATUS_NOTIFY, yid->yd->session_id);

	yahoo_packet_hash(pkt, 49, "WEBCAMINVITE");
	yahoo_packet_hash(pkt, 14, " ");
	yahoo_packet_hash(pkt, 13, "0");
	yahoo_packet_hash(pkt, 1, yid->yd->user);
	yahoo_packet_hash(pkt, 5, who);
	yahoo_send_packet(yid, pkt, 0);

	yahoo_packet_free(pkt);
}

static void yahoo_search_internal(int id, int t, const char *text, int g, int ar, int photo, int yahoo_only, int startpos, int total)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	struct yahoo_input_data *yid;
	char url[1024];
	char buff[1024];
	char *ctext, *p;

	if (!yd)
		return;

	yid = y_new0(struct yahoo_input_data, 1);
	yid->yd = yd;
	yid->type = YAHOO_CONNECTION_SEARCH;

	/*
	age range
	.ar=1 - 13-18, 2 - 18-25, 3 - 25-35, 4 - 35-50, 5 - 50-70, 6 - 70+
	*/

	snprintf(buff, sizeof(buff), "&.sq=%%20&.tt=%d&.ss=%d", total, startpos);

	ctext = strdup(text);
	while((p = strchr(ctext, ' ')))
		*p = '+';

	snprintf(url, 1024, "http://profiles.yahoo.com/?.oc=m&.kw=%s&.sb=%d&.g=%d&.ar=0%s%s%s",
			ctext, t, g, photo ? "&.p=y" : "", yahoo_only ? "&.pg=y" : "",
			startpos ? buff : "");

	FREE(ctext);

	snprintf(buff, sizeof(buff), "Y=%s; T=%s", yd->cookie_y, yd->cookie_t);
	//snprintf(buff, sizeof(buff), "Y=%s; T=%s; C=%s", yd->cookie_y, yd->cookie_t, yd->cookie_c);

	inputs = y_list_prepend(inputs, yid);
//	yahoo_http_get(yid->yd->client_id, url, buff, _yahoo_http_connected, yid);
	YAHOO_CALLBACK(ext_yahoo_send_http_request)(yid->yd->client_id, YAHOO_CONNECTION_SEARCH, "GET", url, buff, 0, 
			_yahoo_http_connected, yid);

}

void yahoo_search(int id, enum yahoo_search_type t, const char *text, enum yahoo_search_gender g, enum yahoo_search_agerange ar, 
		int photo, int yahoo_only)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_search_state *yss;

	if (!yid)
		return;

	if (!yid->ys)
		yid->ys = y_new0(struct yahoo_search_state, 1);

	yss = yid->ys;

	FREE(yss->lsearch_text);
	yss->lsearch_type = t;
	yss->lsearch_text = strdup(text);
	yss->lsearch_gender = g;
	yss->lsearch_agerange = ar;
	yss->lsearch_photo = photo;
	yss->lsearch_yahoo_only = yahoo_only;

	yahoo_search_internal(id, t, text, g, ar, photo, yahoo_only, 0, 0);
}

void yahoo_search_again(int id, int start)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_search_state *yss;

	if (!yid || !yid->ys)
		return;

	yss = yid->ys;

	if (start == -1)
		start = yss->lsearch_nstart + yss->lsearch_nfound;

	yahoo_search_internal(id, yss->lsearch_type, yss->lsearch_text, 
			yss->lsearch_gender, yss->lsearch_agerange, 
			yss->lsearch_photo, yss->lsearch_yahoo_only, 
			start, yss->lsearch_ntotal);
}

struct send_file_data {
	struct yahoo_packet *pkt;
	yahoo_get_fd_callback callback;
	void *user_data;
};

static void _yahoo_send_file_connected(int id, int fd, int error, void *data)
{
	struct yahoo_input_data *yid 	= find_input_by_id_and_type(id, YAHOO_CONNECTION_FT);
	struct send_file_data *sfd 		= (struct send_file_data *) data;
	struct yahoo_packet *pkt 		= sfd->pkt;
	char buff[1024];

	if (fd <= 0) {
		sfd->callback(id, fd, error, sfd->user_data);
		FREE(sfd);
		yahoo_packet_free(pkt);
		inputs = y_list_remove(inputs, yid);
		FREE(yid);
		return;
	}

	yid->fd = fd;
	yahoo_send_packet(yid, pkt, 4); /* we pad with 4 chars that follow bellow */
	yahoo_packet_free(pkt);

	/* 4 magic padding chars that we need to send */
	buff[0] = 0x32;
	buff[1] = 0x39;
	buff[2] = 0xc0;
	buff[3] = 0x80;
	
	write(yid->fd, buff, 4);

	/*	YAHOO_CALLBACK(ext_yahoo_add_handler)(nyd->fd, YAHOO_INPUT_READ); */

	sfd->callback(id, fd, error, sfd->user_data);
	FREE(sfd);
	inputs = y_list_remove(inputs, yid);
	/*
	while(yahoo_tcp_readline(buff, sizeof(buff), nyd->fd) > 0) {
		if (!strcmp(buff, ""))
			break;
	}

	*/
	yahoo_input_close(yid);
}

void yahoo_send_file(int id, const char *who, const char *msg, 
		const char *name, unsigned long size, 
		yahoo_get_fd_callback callback, void *data)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	struct yahoo_input_data *yid;
	struct yahoo_server_settings *yss;
	struct yahoo_packet *pkt = NULL;
	char size_str[10];
	long content_length=0;
	char buff[1024];
	char url[255];
	struct send_file_data *sfd;
	const char *s;
		
	if (!yd)
		return;

	yss = yd->server_settings;

	yid = y_new0(struct yahoo_input_data, 1);
	yid->yd = yd;
	yid->type = YAHOO_CONNECTION_FT;

	pkt = yahoo_packet_new(YAHOO_SERVICE_FILETRANSFER, YPACKET_STATUS_DEFAULT, yd->session_id);

	snprintf(size_str, sizeof(size_str), "%lu", size);

	yahoo_packet_hash(pkt, 0, yd->user);
	yahoo_packet_hash(pkt, 5, who);
	yahoo_packet_hash(pkt, 14, msg);
	
	s = strrchr(name, '\\');
	if (s == NULL)
		s = name;
	else
		s++;
	
	yahoo_packet_hash(pkt, 27, s);
	yahoo_packet_hash(pkt, 28, size_str);

	content_length = YAHOO_PACKET_HDRLEN + yahoo_packet_length(pkt);

	snprintf(url, sizeof(url), "http://%s:%d/notifyft", 
			yss->filetransfer_host, yss->filetransfer_port);
	snprintf((char *)buff, sizeof(buff), "Y=%s; T=%s; B=%s;",
			yd->cookie_y, yd->cookie_t, yd->cookie_b);
	inputs = y_list_prepend(inputs, yid);

	sfd = y_new0(struct send_file_data, 1);
	sfd->pkt = pkt;
	sfd->callback = callback;
	sfd->user_data = data;
//	yahoo_http_post(yid->yd->client_id, url, (char *)buff, content_length+4+size,
			//_yahoo_send_file_connected, sfd);
	YAHOO_CALLBACK(ext_yahoo_send_http_request)(yid->yd->client_id, YAHOO_CONNECTION_FT, "POST", url, buff, content_length+4+size,
			_yahoo_send_file_connected, sfd);
}

void yahoo_send_file_y7(int id, const char *from, const char *to, const char *relay_ip, 
				unsigned long size, const char* token, yahoo_get_fd_callback callback, void *data)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	struct yahoo_input_data *yid;
	struct yahoo_server_settings *yss;
	char buff[1024];
	char url[255];
	char *s;
		
	if (!yd)
		return;

	yss = yd->server_settings;

	yid = y_new0(struct yahoo_input_data, 1);
	yid->yd = yd;
	yid->type = YAHOO_CONNECTION_FT;

	s = yahoo_decode(token);
	snprintf(url, sizeof(url), "http://%s/relay?token=%s&sender=%s&recver=%s", relay_ip, s, from, to);
	
	FREE(s);
	
	snprintf((char *)buff, sizeof(buff), "Y=%s; T=%s; B=%s;",
			yd->cookie_y, yd->cookie_t, yd->cookie_b);
	inputs = y_list_prepend(inputs, yid);

	YAHOO_CALLBACK(ext_yahoo_send_http_request)(yid->yd->client_id, YAHOO_CONNECTION_FT, "POST", url, buff, size, callback, data);
}


void yahoo_send_avatar(int id, const char *name, unsigned long size, 
		yahoo_get_fd_callback callback, void *data)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	struct yahoo_input_data *yid;
	struct yahoo_server_settings *yss;
	struct yahoo_packet *pkt = NULL;
	char size_str[10];
	long content_length=0;
	char buff[1024];
	char url[255];
	struct send_file_data *sfd;
	const char *s;
		
	if (!yd)
		return;

	yss = yd->server_settings;

	yid = y_new0(struct yahoo_input_data, 1);
	yid->yd = yd;
	yid->type = YAHOO_CONNECTION_FT;

	pkt = yahoo_packet_new(YAHOO_SERVICE_PICTURE_UPLOAD, YPACKET_STATUS_DEFAULT, yd->session_id);
    /* 1 = me, 38 = expire time(?), 0 = me, 28 = size, 27 = filename, 14 = NULL, 29 = data */
	snprintf(size_str, sizeof(size_str), "%lu", size);

	yahoo_packet_hash(pkt, 1, yd->user);
	yahoo_packet_hash(pkt, 38, "604800"); /* time to expire */
	yahoo_packet_hash(pkt, 0, yd->user);
	
	s = strrchr(name, '\\');
	if (s == NULL)
		s = name;
	else
		s++;
	yahoo_packet_hash(pkt, 28, size_str);	
	yahoo_packet_hash(pkt, 27, s);
	yahoo_packet_hash(pkt, 14, "");

	content_length = YAHOO_PACKET_HDRLEN + yahoo_packet_length(pkt);

	//snprintf(url, sizeof(url), "http://%s:%d/notifyft", yss->filetransfer_host, yss->filetransfer_port);
	if (yss->filetransfer_port != 80) {
		snprintf(url, sizeof(url), "http://%s:%d/notifyft", yss->filetransfer_host, yss->filetransfer_port);
	} else {
		snprintf(url, sizeof(url), "http://%s/notifyft", yss->filetransfer_host);
	}
	
	//snprintf((char *)buff, sizeof(buff), "Y=%s; T=%s; B=%s;", yd->cookie_y, yd->cookie_t, yd->cookie_b);
	snprintf((char *)buff, sizeof(buff), "T=%s; Y=%s", yd->cookie_t, yd->cookie_y);
			
	inputs = y_list_prepend(inputs, yid);

	sfd = y_new0(struct send_file_data, 1);
	sfd->pkt = pkt;
	sfd->callback = callback;
	sfd->user_data = data;
//	yahoo_http_post(yid->yd->client_id, url, (char *)buff, content_length+4+size,
//			_yahoo_send_file_connected, sfd);
	YAHOO_CALLBACK(ext_yahoo_send_http_request)(yid->yd->client_id, YAHOO_CONNECTION_FT, "POST", url, buff, content_length+4+size,
			_yahoo_send_file_connected, sfd);
}

enum yahoo_status yahoo_current_status(int id)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	
	if (!yd)
		return YAHOO_STATUS_OFFLINE;
	
	return yd->current_status;
}

const YList * yahoo_get_buddylist(int id)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	if (!yd)
		return NULL;
	return yd->buddies;
}

const YList * yahoo_get_ignorelist(int id)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	if (!yd)
		return NULL;
	return yd->ignore;
}

const YList * yahoo_get_identities(int id)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	if (!yd)
		return NULL;
	return yd->identities;
}

const char * yahoo_get_cookie(int id, const char *which)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	if (!yd)
		return NULL;
	if (!strncasecmp(which, "y", 1))
		return yd->cookie_y;
	if (!strncasecmp(which, "t", 1))
		return yd->cookie_t;
	if (!strncasecmp(which, "c", 1))
		return yd->cookie_c;
	if (!strncasecmp(which, "login", 5))
		return yd->login_cookie;
	if (!strncasecmp(which, "b", 1))
		return yd->cookie_b;
	return NULL;
}

const char * yahoo_get_pw_token(int id)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	if (!yd)
		return NULL;
	
	return yd->pw_token;
}

void yahoo_get_url_handle(int id, const char *url, 
		yahoo_get_url_handle_callback callback, void *data)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	if (!yd)
		return;

	yahoo_get_url_fd(id, url, yd, callback, data);
}

const char * yahoo_get_profile_url( void )
{
	return profile_url;
}

void yahoo_request_buddy_avatar(int id, const char *buddy)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt = NULL;
	struct yahoo_server_settings *yss;

	if (!yid)
		return;

	yd = yid->yd;
	yss = yd->server_settings;
	
	pkt = yahoo_packet_new(YAHOO_SERVICE_PICTURE, YPACKET_STATUS_DEFAULT, yd->session_id);
	yahoo_packet_hash(pkt, 1, yd->user);
	yahoo_packet_hash(pkt, 5, buddy);
	yahoo_packet_hash(pkt, 13, "1");

	if (yss->web_messenger) {
		yahoo_packet_hash(pkt, 0, yd->user); 
		yahoo_packet_hash_int(pkt, 24, yd->session_timestamp);
	}
	
	yahoo_send_packet(yid, pkt, 0);
	yahoo_packet_free(pkt);
}

void yahoo_ftdc_deny(int id, const char *buddy, const char *filename, const char *ft_token, int command)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt = NULL;

	if (!yid)
		return;

	yd = yid->yd;

	pkt = yahoo_packet_new(YAHOO_SERVICE_P2PFILEXFER, YPACKET_STATUS_DEFAULT, yd->session_id);
	yahoo_packet_hash(pkt, 5, buddy);
	yahoo_packet_hash(pkt, 49, "FILEXFER");
	yahoo_packet_hash(pkt, 1, yd->user);
	yahoo_packet_hash(pkt, 13, (command == 2) ? "2" : "3");
	yahoo_packet_hash(pkt, 27, filename);
	yahoo_packet_hash(pkt, 53, ft_token);
	
	yahoo_send_packet(yid, pkt, 0);
	yahoo_packet_free(pkt);

}

void yahoo_ft7dc_accept(int id, const char *buddy, const char *ft_token)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt = NULL;

	if (!yid)
		return;

	yd = yid->yd;

	pkt = yahoo_packet_new(YAHOO_SERVICE_Y7_FILETRANSFER, YPACKET_STATUS_DEFAULT, yd->session_id);
	yahoo_packet_hash(pkt, 1, yd->user);
	yahoo_packet_hash(pkt, 5, buddy);
	yahoo_packet_hash(pkt,265, ft_token);
	yahoo_packet_hash(pkt,222, "3");
	
	yahoo_send_packet(yid, pkt, 0);
	yahoo_packet_free(pkt);

}

void yahoo_ft7dc_deny(int id, const char *buddy, const char *ft_token)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt = NULL;

	if (!yid)
		return;

	yd = yid->yd;

	pkt = yahoo_packet_new(YAHOO_SERVICE_Y7_FILETRANSFER, YPACKET_STATUS_DEFAULT, yd->session_id);
	yahoo_packet_hash(pkt, 1, yd->user);
	yahoo_packet_hash(pkt, 5, buddy);
	yahoo_packet_hash(pkt,265, ft_token);
	yahoo_packet_hash(pkt,222, "4");

	yahoo_send_packet(yid, pkt, 0);
	yahoo_packet_free(pkt);

}

void yahoo_ft7dc_abort(int id, const char *buddy, const char *ft_token)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt = NULL;

	if (!yid)
		return;

	yd = yid->yd;

	pkt = yahoo_packet_new(YAHOO_SERVICE_Y7_FILETRANSFERACCEPT, YPACKET_STATUS_DISCONNECTED, yd->session_id);
	yahoo_packet_hash(pkt, 1, yd->user);
	yahoo_packet_hash(pkt, 5, buddy);
	yahoo_packet_hash(pkt,265, ft_token);
	yahoo_packet_hash(pkt,66, "-1");

	yahoo_send_packet(yid, pkt, 0);
	yahoo_packet_free(pkt);

}

void yahoo_ft7dc_relay(int id, const char *buddy, const char *ft_token)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt = NULL;

	if (!yid)
		return;

	yd = yid->yd;

	pkt = yahoo_packet_new(YAHOO_SERVICE_Y7_FILETRANSFERACCEPT, YPACKET_STATUS_DEFAULT, yd->session_id);
	yahoo_packet_hash(pkt, 1, yd->user);
	yahoo_packet_hash(pkt, 5, buddy);
	yahoo_packet_hash(pkt,265, ft_token);
	yahoo_packet_hash(pkt,66, "-3");

	yahoo_send_packet(yid, pkt, 0);
	yahoo_packet_free(pkt);

}

void yahoo_ft7dc_nextfile(int id, const char *buddy, const char *ft_token)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt = NULL;

	if (!yid)
		return;

	yd = yid->yd;

	pkt = yahoo_packet_new(YAHOO_SERVICE_Y7_FILETRANSFERACCEPT, YPACKET_STATUS_DEFAULT, yd->session_id);
	yahoo_packet_hash(pkt, 1, yd->user);
	yahoo_packet_hash(pkt, 5, buddy);
	yahoo_packet_hash(pkt,265, ft_token);
	yahoo_packet_hash(pkt,271, "1");
	
	yahoo_send_packet(yid, pkt, 0);
	yahoo_packet_free(pkt);

}

char *yahoo_ft7dc_send(int id, const char *buddy, YList *files)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt = NULL;
	char ft_token[32]; // we only need 23 chars actually
	YList *l=files;
	BYTE result[16];
	mir_md5_state_t ctx;

	if (!yid)
		return NULL;

	mir_md5_init(&ctx);
	mir_md5_append(&ctx, (BYTE *)buddy, strlen(buddy));
	
	snprintf(ft_token, 32, "%lu", time(NULL));
	mir_md5_append(&ctx, (BYTE *)ft_token, strlen(ft_token));
	mir_md5_finish(&ctx, result);
	to_y64((unsigned char *)ft_token, result, 16);
	
	yd = yid->yd;

	pkt = yahoo_packet_new(YAHOO_SERVICE_Y7_FILETRANSFER, YPACKET_STATUS_DEFAULT, yd->session_id);
	yahoo_packet_hash(pkt, 1, yd->user);
	yahoo_packet_hash(pkt, 5, buddy);
	yahoo_packet_hash(pkt,222, "1");
	yahoo_packet_hash(pkt,265, ft_token);
		
	yahoo_packet_hash_int(pkt,266, y_list_length(files)); // files
	
	yahoo_packet_hash(pkt,302, "268");
	yahoo_packet_hash(pkt,300, "268");
	
	while (l) {
		struct yahoo_file_info * fi = (struct yahoo_file_info *) l->data;
		char *c = strrchr(fi->filename, '\\');
		
		if (c != NULL) {
			c++;
		} else {
			c = fi->filename;
		}
		
		yahoo_packet_hash(pkt, 27, c);
		yahoo_packet_hash_int(pkt, 28, fi->filesize);
		
		if (l->next) {
			yahoo_packet_hash(pkt,301, "268");
			yahoo_packet_hash(pkt,300, "268");
		}
		
		l = l->next;
	}
	
	yahoo_packet_hash(pkt, 301, "268");
	yahoo_packet_hash(pkt, 303, "268");
	
	yahoo_send_packet(yid, pkt, 0);
	yahoo_packet_free(pkt);

	return strdup(ft_token);
}

void yahoo_send_file7info(int id, const char *me, const char *who, const char *ft_token, const char* filename,
							const char *relay_ip)
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt = NULL;

	if (!yid)
		return;

	yd = yid->yd;

	pkt = yahoo_packet_new(YAHOO_SERVICE_Y7_FILETRANSFERINFO, YPACKET_STATUS_DEFAULT, yd->session_id);
	yahoo_packet_hash(pkt, 1, me);
	yahoo_packet_hash(pkt, 5, who);
	yahoo_packet_hash(pkt,265, ft_token);
	yahoo_packet_hash(pkt,27, filename);
	yahoo_packet_hash(pkt,249, "3");
	yahoo_packet_hash(pkt,250, relay_ip);
	
	yahoo_send_packet(yid, pkt, 0);
	yahoo_packet_free(pkt);

}

unsigned char *yahoo_webmessenger_idle_packet(int id, int *len) 
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt = NULL;
	int pktlen;
	unsigned char *data;
	int pos = 0;
	int web_messenger = 1;
	
	if (!yid) {
		DEBUG_MSG(("NO Yahoo Input Data???"));
		return NULL;
	}

	yd = yid->yd;

	DEBUG_MSG(("[yahoo_webmessenger_idle_packet] Session: %ld", yd->session_timestamp));
	
	pkt = yahoo_packet_new(YAHOO_SERVICE_IDLE, YPACKET_STATUS_DEFAULT, yd->session_id);
	yahoo_packet_hash(pkt, 0, yd->user);
	
	yahoo_packet_hash_int(pkt, 24, yd->session_timestamp);

	pktlen = yahoo_packet_length(pkt);
	(*len) = YAHOO_PACKET_HDRLEN + pktlen;
	data = y_new0(unsigned char, (*len) + 1);

	memcpy(data + pos, "YMSG", 4); pos += 4;
	pos += yahoo_put16(data + pos, web_messenger ? YAHOO_WEBMESSENGER_PROTO_VER : YAHOO_PROTO_VER); /* version [latest 12 0x000c */
	pos += yahoo_put16(data + pos, 0x0000); /* HIWORD pkt length??? */
	pos += yahoo_put16(data + pos, pktlen); /* LOWORD pkt length? */
	pos += yahoo_put16(data + pos, pkt->service); /* service */
	pos += yahoo_put32(data + pos, pkt->status); /* status [4bytes] */
	pos += yahoo_put32(data + pos, pkt->id); /* session [4bytes] */

	yahoo_packet_write(pkt, data + pos);

	//yahoo_packet_dump(data, len);
	DEBUG_MSG(("Sending Idle Packet:"));

	yahoo_packet_read(pkt, data + pos, (*len) - pos);	
	
	
	return data;
}

void yahoo_send_idle_packet(int id) 
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt = NULL;
	
	if (!yid) {
		DEBUG_MSG(("NO Yahoo Input Data???"));
		return;
	}

	yd = yid->yd;

	DEBUG_MSG(("[yahoo_send_idle_packet] Session: %ld", yd->session_timestamp));
	
	pkt = yahoo_packet_new(YAHOO_SERVICE_IDLE, YPACKET_STATUS_DEFAULT, yd->session_id);
	yahoo_packet_hash(pkt, 0, yd->user);
	
	yahoo_packet_hash_int(pkt, 24, yd->session_timestamp);

	yahoo_send_packet(yid, pkt, 0);
	yahoo_packet_free(pkt);
}

void yahoo_send_im_ack(int id, const char *me, const char *buddy, const char *seqn, int sendn) 
{
	struct yahoo_input_data *yid = find_input_by_id_and_type(id, YAHOO_CONNECTION_PAGER);
	struct yahoo_data *yd;
	struct yahoo_packet *pkt = NULL;

	DEBUG_MSG(("[yahoo_send_im_ack] My Id: %s, Buddy: %s, Seq #: %s, Retry: %d", me, buddy, seqn, sendn));
	
	if (!yid) {
		DEBUG_MSG(("NO Yahoo Input Data???"));
		return;
	}

	yd = yid->yd;

	pkt = yahoo_packet_new(YAHOO_SERVICE_Y9_MESSAGE_ACK, YPACKET_STATUS_DEFAULT, yd->session_id);
	yahoo_packet_hash(pkt, 1, (me != NULL) ? me : yd->user);
	yahoo_packet_hash(pkt, 5, buddy);
	
	yahoo_packet_hash(pkt, 302, "430");
	yahoo_packet_hash(pkt, 430, seqn);
	yahoo_packet_hash(pkt, 303, "430");
	yahoo_packet_hash_int(pkt, 450, sendn);
	//yahoo_packet_hash_int(pkt, 24, yd->session_timestamp);

	yahoo_send_packet(yid, pkt, 0);
	yahoo_packet_free(pkt);
}


