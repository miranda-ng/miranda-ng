
/*
  Meanwhile - Unofficial Lotus Sametime Community Client Library
  Copyright (C) 2004  Christopher (siege) O'Brien
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.
  
  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <glib.h>
#include <glib/ghash.h>
#include <glib/glist.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "mw_channel.h"
#include "mw_debug.h"
#include "mw_error.h"
#include "mw_message.h"
#include "mw_service.h"
#include "mw_session.h"
#include "mw_srvc_conf.h"
#include "mw_util.h"


/* This thing needs a re-write. More than anything else, I need to
   re-examine the conferencing service protocol from more modern
   clients */


#define PROTOCOL_TYPE   0x00000010
#define PROTOCOL_VER    0x00000002


/** @see mwMsgChannelSend::type
    @see recv */
enum msg_type {
  msg_WELCOME  = 0x0000,  /**< welcome message */
  msg_INVITE   = 0x0001,  /**< outgoing invitation */
  msg_JOIN     = 0x0002,  /**< someone joined */
  msg_PART     = 0x0003,  /**< someone left */
  msg_MESSAGE  = 0x0004,  /**< conference message */
};


/** the conferencing service */
struct mwServiceConference {
  struct mwService service;

  /** call-back handler for this service */
  struct mwConferenceHandler *handler;

  /** collection of conferences in this service */
  GList *confs;
};


/** a conference and its members */
struct mwConference {
  enum mwConferenceState state;   /**< state of the conference */
  struct mwServiceConference *service;  /**< owning service */
  struct mwChannel *channel;      /**< conference's channel */

  char *name;   /**< server identifier for the conference */
  char *title;  /**< topic for the conference */

  struct mwLoginInfo owner;  /**< person who created this conference */
  GHashTable *members;       /**< mapping guint16:mwLoginInfo */
  struct mw_datum client_data;
};


#define MEMBER_FIND(conf, id) \
  g_hash_table_lookup(conf->members, GUINT_TO_POINTER((guint) id))


#define MEMBER_ADD(conf, id, member) \
  g_hash_table_insert(conf->members, GUINT_TO_POINTER((guint) id), member)


#define MEMBER_REM(conf, id) \
  g_hash_table_remove(conf->members, GUINT_TO_POINTER((guint) id));


/** clear and free a login info block */
static void login_free(struct mwLoginInfo *li) {
  mwLoginInfo_clear(li);
  g_free(li);
}


/** generates a random conference name built around a user name */
static char *conf_generate_name(const char *user) {
  /// Miranda NG adaptation start - MSVC
  ///guint a, b;
  guint a;
  guint64 b;
  /// Miranda NG adaptation end
  char *ret;
  
  user = user? user: "";

  srand(clock() + rand());
  a = ((rand() & 0xff) << 8) | (rand() & 0xff);
  b = time(NULL);

  /// Miranda NG adaptation start - MSVC
  ///ret = g_strdup_printf("%s(%08x,%04x)", user, b, a);
  ret = g_strdup_printf("%s(%I64u,%04x)", user, b, a);
  /// Miranda NG adaptation end
  g_debug("generated random conference name: '%s'", ret);
  return ret;
}





static struct mwConference *conf_new(struct mwServiceConference *srvc) {

  struct mwConference *conf;
  struct mwSession *session;
  const char *user;

  conf = g_new0(struct mwConference, 1);
  conf->state = mwConference_NEW;
  conf->service = srvc;
  conf->members = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL,
					(GDestroyNotify) login_free);

  session = mwService_getSession(MW_SERVICE(srvc));
  user = mwSession_getProperty(session, mwSession_AUTH_USER_ID);

  srvc->confs = g_list_prepend(srvc->confs, conf);

  return conf;
}


/** clean and free a conference structure */
static void conf_free(struct mwConference *conf) {
  struct mwServiceConference *srvc;

  /* this shouldn't ever happen, but just to be sure */
  g_return_if_fail(conf != NULL);
  
  srvc = conf->service;

  if(conf->members)
    g_hash_table_destroy(conf->members);

  g_list_remove_all(srvc->confs, conf);

  mw_datum_clear(&conf->client_data);
  
  g_free(conf->name);
  g_free(conf->title);
  g_free(conf);
}


static struct mwConference *conf_find(struct mwServiceConference *srvc,
				      struct mwChannel *chan) {
  GList *l;

  g_return_val_if_fail(srvc != NULL, NULL);
  g_return_val_if_fail(chan != NULL, NULL);
  
  for(l = srvc->confs; l; l = l->next) {
    struct mwConference *conf = l->data;
    if(conf->channel == chan) return conf;
  }

  return NULL;
}


static const char *conf_state_str(enum mwConferenceState state) {
  switch(state) {
  case mwConference_NEW:      return "new";
  case mwConference_PENDING:  return "pending";
  case mwConference_INVITED:  return "invited";
  case mwConference_OPEN:     return "open";
  case mwConference_CLOSING:  return "closing";
  case mwConference_ERROR:    return "error";

  case mwConference_UNKNOWN:  /* fall through */
  default:                    return "UNKNOWN";
  }
}


static void conf_state(struct mwConference *conf,
		       enum mwConferenceState state) {
  g_return_if_fail(conf != NULL);

  if(conf->state == state) return;

  conf->state = state;
  g_message("conference %s state: %s",
	    NSTR(conf->name), conf_state_str(state));
}


static void recv_channelCreate(struct mwService *srvc,
			       struct mwChannel *chan,
			       struct mwMsgChannelCreate *msg) {

  /* - this is how we really receive invitations
     - create a conference and associate it with the channel
     - obtain the invite data from the msg addtl info
     - mark the conference as INVITED
     - trigger the got_invite event
  */

  struct mwServiceConference *srvc_conf = (struct mwServiceConference *) srvc;
  struct mwConference *conf;

  struct mwGetBuffer *b;

  char *invite = NULL;
  guint tmp;

  conf = conf_new(srvc_conf);
  conf->channel = chan;

  b = mwGetBuffer_wrap(&msg->addtl);

  guint32_get(b, &tmp);
  mwString_get(b, &conf->name);
  mwString_get(b, &conf->title);
  guint32_get(b, &tmp);
  mwLoginInfo_get(b, &conf->owner);
  guint32_get(b, &tmp);
  mwString_get(b, &invite);

  if(mwGetBuffer_error(b)) {
    g_warning("failure parsing addtl for conference invite");
    mwConference_destroy(conf, ERR_FAILURE, NULL);

  } else {
    struct mwConferenceHandler *h = srvc_conf->handler;
    conf_state(conf, mwConference_INVITED);
    if(h->on_invited)
      h->on_invited(conf, &conf->owner, invite);
  }

  mwGetBuffer_free(b);
  g_free(invite);
}


static void recv_channelAccept(struct mwService *srvc,
			       struct mwChannel *chan,
			       struct mwMsgChannelAccept *msg) {

  ;
}


static void recv_channelDestroy(struct mwService *srvc,
				struct mwChannel *chan,
				struct mwMsgChannelDestroy *msg) {

  /* - find conference from channel
     - trigger got_closed
     - remove conference, dealloc
  */

  struct mwServiceConference *srvc_conf = (struct mwServiceConference *) srvc;
  struct mwConference *conf = conf_find(srvc_conf, chan);
  struct mwConferenceHandler *h = srvc_conf->handler;

  /* if there's no such conference, then I guess there's nothing to worry
     about. Except of course for the fact that we should never receive a
     channel destroy for a conference that doesn't exist. */
  if(! conf) return;

  conf->channel = NULL;

  conf_state(conf, msg->reason? mwConference_ERROR: mwConference_CLOSING);

  if(h->conf_closed)
    h->conf_closed(conf, msg->reason);

  mwConference_destroy(conf, ERR_SUCCESS, NULL);
}


static void WELCOME_recv(struct mwServiceConference *srvc,
			 struct mwConference *conf,
			 struct mwGetBuffer *b) {

  struct mwConferenceHandler *h;
  guint16 tmp16;
  guint32 tmp32;
  guint32 count;
  GList *l = NULL;

  /* re-read name and title */
  g_free(conf->name);
  g_free(conf->title);
  conf->name = NULL;
  conf->title = NULL;
  mwString_get(b, &conf->name);
  mwString_get(b, &conf->title);

  /* some numbers we don't care about, then a count of members */
  guint16_get(b, &tmp16);
  guint32_get(b, &tmp32);
  guint32_get(b, &count);

  if(mwGetBuffer_error(b)) {
    g_warning("error parsing welcome message for conference");
    mwConference_destroy(conf, ERR_FAILURE, NULL);
    return;
  }
  
  while(count--) {
    guint16 member_id;
    struct mwLoginInfo *member = g_new0(struct mwLoginInfo, 1);

    guint16_get(b, &member_id);
    mwLoginInfo_get(b, member);

    if(mwGetBuffer_error(b)) {
      login_free(member);
      break;
    }

    MEMBER_ADD(conf, member_id, member);
    l = g_list_append(l, member);
  }

  conf_state(conf, mwConference_OPEN);

  h = srvc->handler;
  if(h->conf_opened)
    h->conf_opened(conf, l);

  /* get rid of the GList, but not its contents */
  g_list_free(l);
}


static void JOIN_recv(struct mwServiceConference *srvc,
		      struct mwConference *conf,
		      struct mwGetBuffer *b) {

  struct mwConferenceHandler *h;
  guint16 m_id;
  struct mwLoginInfo *m;
  
  /* for some inane reason, conferences we create will send a join
     message for ourselves before the welcome message. Since the
     welcome message will list our ID among those in the channel,
     we're going to just pretend that these join messages don't
     exist */
  if(conf->state == mwConference_PENDING)
    return;

  m = g_new0(struct mwLoginInfo, 1);

  guint16_get(b, &m_id);
  mwLoginInfo_get(b, m);

  if(mwGetBuffer_error(b)) {
    g_warning("failed parsing JOIN message in conference");
    login_free(m);
    return;
  }

  MEMBER_ADD(conf, m_id, m);

  h = srvc->handler;
  if(h->on_peer_joined)
    h->on_peer_joined(conf, m);
}


static void PART_recv(struct mwServiceConference *srvc,
		      struct mwConference *conf,
		      struct mwGetBuffer *b) {

  /* - parse who left
     - look up their membership
     - remove them from the members list
     - trigger the event
  */

  struct mwConferenceHandler *h;
  guint16 id = 0;
  struct mwLoginInfo *m;

  guint16_get(b, &id);

  if(mwGetBuffer_error(b)) return;

  m = MEMBER_FIND(conf, id);
  if(! m) return;

  h = srvc->handler;
  if(h->on_peer_parted)
    h->on_peer_parted(conf, m);

  MEMBER_REM(conf, id);
}


static void text_recv(struct mwServiceConference *srvc,
		      struct mwConference *conf,
		      struct mwLoginInfo *m,
		      struct mwGetBuffer *b) {

  /* this function acts a lot like receiving an IM Text message. The text
     message contains only a string */

  char *text = NULL;
  struct mwConferenceHandler *h;
  
  mwString_get(b, &text);

  if(mwGetBuffer_error(b)) {
    g_warning("failed to parse text message in conference");
    g_free(text);
    return;
  }

  h = srvc->handler;
  if(text && h->on_text) {
    h->on_text(conf, m, text);
  }

  g_free(text);
}


static void data_recv(struct mwServiceConference *srvc,
		      struct mwConference *conf,
		      struct mwLoginInfo *m,
		      struct mwGetBuffer *b) {

  /* this function acts a lot like receiving an IM Data message. The
     data message has a type, a subtype, and an opaque. We only
     support typing notification though. */

  /** @todo it's possible that some clients send text in a data
      message, as we've seen rarely in the IM service. Have to add
      support for that here */

  guint32 type, subtype;
  struct mwConferenceHandler *h;

  guint32_get(b, &type);
  guint32_get(b, &subtype);

  if(mwGetBuffer_error(b)) return;

  /* don't know how to deal with any others yet */
  if(type != 0x01) {
    g_message("unknown data message type (0x%08x, 0x%08x)", type, subtype);
    return;
  }

  h = srvc->handler;
  if(h->on_typing) {
    h->on_typing(conf, m, !subtype);
  }
}


static void MESSAGE_recv(struct mwServiceConference *srvc,
			 struct mwConference *conf,
			 struct mwGetBuffer *b) {

  /* - look up who send the message by their id
     - trigger the event
  */

  guint16 id;
  guint32 type;
  struct mwLoginInfo *m;

  /* an empty buffer isn't an error, just ignored */
  if(! mwGetBuffer_remaining(b)) return;

  guint16_get(b, &id);
  guint32_get(b, &type); /* reuse type variable */
  guint32_get(b, &type);

  if(mwGetBuffer_error(b)) return;

  m = MEMBER_FIND(conf, id);
  if(! m) {
    g_warning("received message type 0x%04x from"
	      " unknown conference member %u", type, id);
    return;
  }
  
  switch(type) {
  case 0x01:  /* type is text */
    text_recv(srvc, conf, m, b);
    break;

  case 0x02:  /* type is data */
    data_recv(srvc, conf, m, b);
    break;

  default:
    g_warning("unknown message type 0x%4x received in conference", type);
  }
}


static void recv(struct mwService *srvc, struct mwChannel *chan,
		 guint16 type, struct mwOpaque *data) {

  struct mwServiceConference *srvc_conf = (struct mwServiceConference *) srvc;
  struct mwConference *conf = conf_find(srvc_conf, chan);
  struct mwGetBuffer *b;

  g_return_if_fail(conf != NULL);

  b = mwGetBuffer_wrap(data);

  switch(type) {
  case msg_WELCOME:
    WELCOME_recv(srvc_conf, conf, b);
    break;

  case msg_JOIN:
    JOIN_recv(srvc_conf, conf, b);
    break;

  case msg_PART:
    PART_recv(srvc_conf, conf, b);
    break;

  case msg_MESSAGE:
    MESSAGE_recv(srvc_conf, conf, b);
    break;

  default:
    ; /* hrm. should log this. TODO */
  }
}


static void clear(struct mwServiceConference *srvc) {
  struct mwConferenceHandler *h;

  while(srvc->confs)
    conf_free(srvc->confs->data);

  h = srvc->handler;
  if(h && h->clear)
    h->clear(srvc);
  srvc->handler = NULL;
}


static const char *name(struct mwService *srvc) {
  return "Basic Conferencing";
}


static const char *desc(struct mwService *srvc) {
  return "Multi-user plain-text conferencing";
}


static void start(struct mwService *srvc) {
  mwService_started(srvc);
}


static void stop(struct mwServiceConference *srvc) {
  while(srvc->confs)
    mwConference_destroy(srvc->confs->data, ERR_SUCCESS, NULL);

  mwService_stopped(MW_SERVICE(srvc));
}


struct mwServiceConference *
mwServiceConference_new(struct mwSession *session,
			struct mwConferenceHandler *handler) {

  struct mwServiceConference *srvc_conf;
  struct mwService *srvc;

  g_return_val_if_fail(session != NULL, NULL);
  g_return_val_if_fail(handler != NULL, NULL);

  srvc_conf = g_new0(struct mwServiceConference, 1);
  srvc = &srvc_conf->service;

  mwService_init(srvc, session, mwService_CONFERENCE);
  srvc->start = start;
  srvc->stop = (mwService_funcStop) stop;
  srvc->recv_create = recv_channelCreate;
  srvc->recv_accept = recv_channelAccept;
  srvc->recv_destroy = recv_channelDestroy;
  srvc->recv = recv;
  srvc->clear = (mwService_funcClear) clear;
  srvc->get_name = name;
  srvc->get_desc = desc;

  srvc_conf->handler = handler;

  return srvc_conf;
}


struct mwConference *mwConference_new(struct mwServiceConference *srvc,
				      const char *title) {
  struct mwConference *conf;

  g_return_val_if_fail(srvc != NULL, NULL);

  conf = conf_new(srvc);
  conf->title = g_strdup(title);

  return conf;
}


/// Miranda NG adaptation start - renamed method
struct mwServiceConference *
mwConference_getServiceConference(struct mwConference *conf) {
  g_return_val_if_fail(conf != NULL, NULL);
  return conf->service;
}
/// Miranda NG adaptation end


/// Miranda NG adaptation start - new method
struct mwService *mwServiceConference_getService(struct mwServiceConference *srvc) {
  g_return_val_if_fail(srvc != NULL, NULL);
  return &(srvc->service);
}
/// Miranda NG adaptation end


const char *mwConference_getName(struct mwConference *conf) {
  g_return_val_if_fail(conf != NULL, NULL);
  return conf->name;
}


const char *mwConference_getTitle(struct mwConference *conf) {
  g_return_val_if_fail(conf != NULL, NULL);
  return conf->title;
}


GList *mwConference_getMembers(struct mwConference *conf) {
  g_return_val_if_fail(conf != NULL, NULL);
  g_return_val_if_fail(conf->members != NULL, NULL);

  return map_collect_values(conf->members);
}


int mwConference_open(struct mwConference *conf) {
  struct mwSession *session;
  struct mwChannel *chan;
  struct mwPutBuffer *b;
  int ret;
  
  g_return_val_if_fail(conf != NULL, -1);
  g_return_val_if_fail(conf->service != NULL, -1);
  g_return_val_if_fail(conf->state == mwConference_NEW, -1);
  g_return_val_if_fail(conf->channel == NULL, -1);

  session = mwService_getSession(MW_SERVICE(conf->service));
  g_return_val_if_fail(session != NULL, -1);

  if(! conf->name) {
    char *user = mwSession_getProperty(session, mwSession_AUTH_USER_ID);
    conf->name = conf_generate_name(user? user: "meanwhile");
  }

  chan = mwChannel_newOutgoing(mwSession_getChannels(session));
  mwChannel_setService(chan, MW_SERVICE(conf->service));
  mwChannel_setProtoType(chan, PROTOCOL_TYPE);
  mwChannel_setProtoVer(chan, PROTOCOL_VER);
  
#if 0
  /* offer all known ciphers */
  mwChannel_populateSupportedCipherInstances(chan);
#endif

  b = mwPutBuffer_new();
  mwString_put(b, conf->name);
  mwString_put(b, conf->title);
  guint32_put(b, 0x00);
  mwPutBuffer_finalize(mwChannel_getAddtlCreate(chan), b);

  ret = mwChannel_create(chan);
  if(ret) {
    conf_state(conf, mwConference_ERROR);
  } else {
    conf_state(conf, mwConference_PENDING);
    conf->channel = chan;
  }

  return ret;
}


int mwConference_destroy(struct mwConference *conf,
			 guint32 reason, const char *text) {

  struct mwServiceConference *srvc;
  struct mwOpaque info = { 0, 0 };
  int ret = 0;

  g_return_val_if_fail(conf != NULL, -1);

  srvc = conf->service;
  g_return_val_if_fail(srvc != NULL, -1);

  /* remove conference from the service */
  srvc->confs = g_list_remove_all(srvc->confs, conf);

  /* close the channel if applicable */
  if(conf->channel) {
    if(text && *text) {
      info.len = strlen(text);
      info.data = (guchar *) text;
    }

    ret = mwChannel_destroy(conf->channel, reason, &info);
  }
  
  /* free the conference */
  conf_free(conf);

  return ret;
}


int mwConference_accept(struct mwConference *conf) {
  /* - if conference is not INVITED, return -1
     - accept the conference channel
     - send an empty JOIN message
  */

  struct mwChannel *chan;
  int ret;

  g_return_val_if_fail(conf != NULL, -1);
  g_return_val_if_fail(conf->state == mwConference_INVITED, -1);

  chan = conf->channel;
  ret = mwChannel_accept(chan);

  if(! ret)
    ret = mwChannel_sendEncrypted(chan, msg_JOIN, NULL, FALSE);

  return ret;
}


int mwConference_invite(struct mwConference *conf,
			struct mwIdBlock *who,
			const char *text) {

  struct mwPutBuffer *b;
  struct mwOpaque o;
  int ret;

  g_return_val_if_fail(conf != NULL, -1);
  g_return_val_if_fail(conf->channel != NULL, -1);
  g_return_val_if_fail(who != NULL, -1);

  b = mwPutBuffer_new();

  mwIdBlock_put(b, who);
  guint16_put(b, 0x00);
  guint32_put(b, 0x00);
  mwString_put(b, text);
  mwString_put(b, who->user);

  mwPutBuffer_finalize(&o, b);
  ret = mwChannel_sendEncrypted(conf->channel, msg_INVITE, &o, FALSE);
  mwOpaque_clear(&o);

  return ret;
}


int mwConference_sendText(struct mwConference *conf, const char *text) {
  struct mwPutBuffer *b;
  struct mwOpaque o;
  int ret;

  g_return_val_if_fail(conf != NULL, -1);
  g_return_val_if_fail(conf->channel != NULL, -1);

  b = mwPutBuffer_new();

  guint32_put(b, 0x01);
  mwString_put(b, text);

  mwPutBuffer_finalize(&o, b);
  ret = mwChannel_sendEncrypted(conf->channel, msg_MESSAGE, &o, FALSE);
  mwOpaque_clear(&o);

  return ret;
}


int mwConference_sendTyping(struct mwConference *conf, gboolean typing) {
  struct mwPutBuffer *b;
  struct mwOpaque o;
  int ret;

  g_return_val_if_fail(conf != NULL, -1);
  g_return_val_if_fail(conf->channel != NULL, -1);
  g_return_val_if_fail(conf->state == mwConference_OPEN, -1);

  b = mwPutBuffer_new();

  guint32_put(b, 0x02);
  guint32_put(b, 0x01);
  guint32_put(b, !typing);
  mwOpaque_put(b, NULL);

  mwPutBuffer_finalize(&o, b);
  ret = mwChannel_sendEncrypted(conf->channel, msg_MESSAGE, &o, FALSE);
  mwOpaque_clear(&o);

  return ret;
}


void mwConference_setClientData(struct mwConference *conference,
			     gpointer data, GDestroyNotify clear) {

  g_return_if_fail(conference != NULL);
  mw_datum_set(&conference->client_data, data, clear);
}


gpointer mwConference_getClientData(struct mwConference *conference) {
  g_return_val_if_fail(conference != NULL, NULL);
  return mw_datum_get(&conference->client_data);
}


void mwConference_removeClientData(struct mwConference *conference) {
  g_return_if_fail(conference != NULL);
  mw_datum_clear(&conference->client_data);
}


struct mwConferenceHandler *
mwServiceConference_getHandler(struct mwServiceConference *srvc) {
  g_return_val_if_fail(srvc != NULL, NULL);
  return srvc->handler;
}


GList *mwServiceConference_getConferences(struct mwServiceConference *srvc) {
  g_return_val_if_fail(srvc != NULL, NULL);
  return g_list_copy(srvc->confs);
}

