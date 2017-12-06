
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
#include <string.h>

#include "mw_channel.h"
#include "mw_cipher.h"
#include "mw_debug.h"
#include "mw_error.h"
#include "mw_message.h"
#include "mw_service.h"
#include "mw_session.h"
#include "mw_util.h"


/** @todo reorganize this file, stuff is just strewn about */


struct mwChannel {

  /** session this channel belongs to */
  struct mwSession *session;

  enum mwChannelState state;

  /** creator for incoming channel, target for outgoing channel */
  struct mwLoginInfo user;

  /* similar to data from the CreateCnl message in 8.4.1.7 */
  guint32 reserved;    /**< special, unknown meaning */
  guint32 id;          /**< channel ID */
  guint32 service;     /**< service ID */
  guint32 proto_type;  /**< service protocol type */
  guint32 proto_ver;   /**< service protocol version */
  guint32 options;     /**< channel options */

  struct mwOpaque addtl_create;
  struct mwOpaque addtl_accept;

  /** all those supported ciphers */
  GHashTable *supported;
  guint16 offered_policy;  /**< @see enum mwEncryptPolicy */
  guint16 policy;          /**< @see enum mwEncryptPolicy */

  /** cipher information determined at channel acceptance */
  struct mwCipherInstance *cipher;

  /** statistics table */
  GHashTable *stats;

  GSList *outgoing_queue;     /**< queued outgoing messages */
  GSList *incoming_queue;     /**< queued incoming messages */

  struct mw_datum srvc_data;  /**< service-specific data */
};


struct mwChannelSet {
  struct mwSession *session;  /**< owning session */
  GHashTable *map;            /**< map of all channels, by ID */
  guint32 counter;            /**< counter for outgoing ID */
};


static void flush_channel(struct mwChannel *);


static const char *state_str(enum mwChannelState state) {
  switch(state) {
  case mwChannel_NEW:      return "new";
  case mwChannel_INIT:     return "initializing";
  case mwChannel_WAIT:     return "waiting";
  case mwChannel_OPEN:     return "open";
  case mwChannel_DESTROY:  return "closing";
  case mwChannel_ERROR:    return "error";

  case mwChannel_UNKNOWN:  /* fall through */
  default:                 return "UNKNOWN";
  }
}


static void state(struct mwChannel *chan, enum mwChannelState state,
		  guint32 err_code) {

  g_return_if_fail(chan != NULL);

  if(chan->state == state) return;

  chan->state = state;

  if(err_code) {
    g_message("channel 0x%08x state: %s (0x%08x)",
	      chan->id, state_str(state), err_code);
  } else {
    g_message("channel 0x%08x state: %s", chan->id, state_str(state));
  }
}


static gpointer get_stat(struct mwChannel *chan,
			 enum mwChannelStatField field) {

  return g_hash_table_lookup(chan->stats, (gpointer) field);
}


static void set_stat(struct mwChannel *chan, enum mwChannelStatField field,
		     gpointer val) {

  g_hash_table_insert(chan->stats, (gpointer) field, val);
}

/// Miranda NG adaptation start - MSVC
//#define incr_stat(chan, field, incr) \
//  set_stat(chan, field, get_stat(chan, field) + incr)
#define incr_stat(chan, field, incr) \
  set_stat(chan, field, (char*)get_stat(chan, field) + incr)
/// Miranda NG adaptation end

#define timestamp_stat(chan, field) \
  set_stat(chan, field, (gpointer) time(NULL))


static void sup_free(gpointer a) {
  mwCipherInstance_free(a);
}


static struct mwCipherInstance *
get_supported(struct mwChannel *chan, guint16 id) {

  guint32 cid = (guint32) id;
  return g_hash_table_lookup(chan->supported, GUINT_TO_POINTER(cid));
}


static void put_supported(struct mwChannel *chan,
			  struct mwCipherInstance *ci) {

  struct mwCipher *cipher = mwCipherInstance_getCipher(ci);
  guint32 cid = (guint32) mwCipher_getType(cipher);
  g_hash_table_insert(chan->supported, GUINT_TO_POINTER(cid), ci);
}


struct mwChannel *mwChannel_newIncoming(struct mwChannelSet *cs, guint32 id) {
  struct mwChannel *chan;

  g_return_val_if_fail(cs != NULL, NULL);
  g_return_val_if_fail(cs->session != NULL, NULL);

  chan = g_new0(struct mwChannel, 1);
  chan->state = mwChannel_NEW;
  chan->session = cs->session;
  chan->id = id;

  chan->stats = g_hash_table_new(g_direct_hash, g_direct_equal);

  chan->supported = g_hash_table_new_full(g_direct_hash, g_direct_equal,
					  NULL, sup_free);

  g_hash_table_insert(cs->map, GUINT_TO_POINTER(id), chan);

  state(chan, mwChannel_WAIT, 0);

  return chan;
}


struct mwChannel *mwChannel_newOutgoing(struct mwChannelSet *cs) {
  guint32 id;
  struct mwChannel *chan;

  g_return_val_if_fail(cs != NULL, NULL);
  g_return_val_if_fail(cs->map != NULL, NULL);

  /* grab the next id, and try to make sure there isn't already a
     channel using it */
  do {
    id = ++cs->counter;
  } while(g_hash_table_lookup(cs->map, GUINT_TO_POINTER(id)));
  
  chan = mwChannel_newIncoming(cs, id);
  state(chan, mwChannel_INIT, 0);

  return chan;
}


guint32 mwChannel_getId(struct mwChannel *chan) {
  g_return_val_if_fail(chan != NULL, 0);
  return chan->id;
}


struct mwSession *mwChannel_getSession(struct mwChannel *chan) {
  g_return_val_if_fail(chan != NULL, NULL);
  return chan->session;
}


guint32 mwChannel_getServiceId(struct mwChannel *chan) {
  g_return_val_if_fail(chan != NULL, 0);
  return chan->service;
}


struct mwService *mwChannel_getService(struct mwChannel *chan) {
  g_return_val_if_fail(chan != NULL, NULL);
  return mwSession_getService(chan->session, chan->service);
}


void mwChannel_setService(struct mwChannel *chan, struct mwService *srvc) {
  g_return_if_fail(chan != NULL);
  g_return_if_fail(srvc != NULL);
  g_return_if_fail(chan->state == mwChannel_INIT);
  chan->service = mwService_getType(srvc);
}


gpointer mwChannel_getServiceData(struct mwChannel *chan) {
  g_return_val_if_fail(chan != NULL, NULL);
  return mw_datum_get(&chan->srvc_data);
}


void mwChannel_setServiceData(struct mwChannel *chan,
			      gpointer data, GDestroyNotify clean) {

  g_return_if_fail(chan != NULL);
  mw_datum_set(&chan->srvc_data, data, clean);
}


void mwChannel_removeServiceData(struct mwChannel *chan) {
  g_return_if_fail(chan != NULL);
  mw_datum_clear(&chan->srvc_data);
}


guint32 mwChannel_getProtoType(struct mwChannel *chan) {
  g_return_val_if_fail(chan != NULL, 0x00);
  return chan->proto_type;
}


void mwChannel_setProtoType(struct mwChannel *chan, guint32 proto_type) {
  g_return_if_fail(chan != NULL);
  g_return_if_fail(chan->state == mwChannel_INIT);
  chan->proto_type = proto_type;
}


guint32 mwChannel_getProtoVer(struct mwChannel *chan) {
  g_return_val_if_fail(chan != NULL, 0x00);
  return chan->proto_ver;
}


void mwChannel_setProtoVer(struct mwChannel *chan, guint32 proto_ver) {
  g_return_if_fail(chan != NULL);
  g_return_if_fail(chan->state == mwChannel_INIT);
  chan->proto_ver = proto_ver;
}


guint16 mwChannel_getEncryptPolicy(struct mwChannel *chan) {
  g_return_val_if_fail(chan != NULL, 0x00);
  return chan->policy;
}


guint32 mwChannel_getOptions(struct mwChannel *chan) {
  g_return_val_if_fail(chan != NULL, 0x00);
  return chan->options;
}


void mwChannel_setOptions(struct mwChannel *chan, guint32 options) {
  g_return_if_fail(chan != NULL);
  g_return_if_fail(chan->state == mwChannel_INIT);
  chan->options = options;
}


struct mwLoginInfo *mwChannel_getUser(struct mwChannel *chan) {
  g_return_val_if_fail(chan != NULL, NULL);
  return &chan->user;
}


struct mwOpaque *mwChannel_getAddtlCreate(struct mwChannel *chan) {
  g_return_val_if_fail(chan != NULL, NULL);
  return &chan->addtl_create;
}


struct mwOpaque *mwChannel_getAddtlAccept(struct mwChannel *chan) {
  g_return_val_if_fail(chan != NULL, NULL);
  return &chan->addtl_accept;
}


struct mwCipherInstance *
mwChannel_getCipherInstance(struct mwChannel *chan) {

  g_return_val_if_fail(chan != NULL, NULL);
  return chan->cipher;
}


enum mwChannelState mwChannel_getState(struct mwChannel *chan) {
  g_return_val_if_fail(chan != NULL, mwChannel_UNKNOWN);
  return chan->state;
}


gpointer mwChannel_getStatistic(struct mwChannel *chan,
				enum mwChannelStatField stat) {
  
  g_return_val_if_fail(chan != NULL, 0);
  g_return_val_if_fail(chan->stats != NULL, 0);

  return get_stat(chan, stat);
}


/* send a channel create message */
int mwChannel_create(struct mwChannel *chan) {
  struct mwMsgChannelCreate *msg;
  GList *list, *l;
  int ret;

  g_return_val_if_fail(chan != NULL, -1);
  g_return_val_if_fail(chan->state == mwChannel_INIT, -1);
  g_return_val_if_fail(mwChannel_isOutgoing(chan), -1);

  msg = (struct mwMsgChannelCreate *)
    mwMessage_new(mwMessage_CHANNEL_CREATE);

  msg->channel = chan->id;
  msg->target.user = g_strdup(chan->user.user_id);
  msg->target.community = g_strdup(chan->user.community);
  msg->service = chan->service;
  msg->proto_type = chan->proto_type;
  msg->proto_ver = chan->proto_ver;
  msg->options = chan->options;
  mwOpaque_clone(&msg->addtl, &chan->addtl_create);

  list = mwChannel_getSupportedCipherInstances(chan);
  if(list) {
    /* offer what we have */
    for(l = list; l; l = l->next) {
      struct mwEncryptItem *ei = mwCipherInstance_offer(l->data);
      msg->encrypt.items = g_list_append(msg->encrypt.items, ei);
    }

    /* we're easy to get along with */
    chan->offered_policy = mwEncrypt_WHATEVER;
    g_list_free(list);

  } else {
    /* we apparently don't support anything */
    chan->offered_policy = mwEncrypt_NONE;
  }

  msg->encrypt.mode = chan->offered_policy;
  msg->encrypt.extra = chan->offered_policy;
  
  ret = mwSession_send(chan->session, MW_MESSAGE(msg));
  mwMessage_free(MW_MESSAGE(msg));

  state(chan, (ret)? mwChannel_ERROR: mwChannel_WAIT, ret);

  return ret;
}


static void channel_open(struct mwChannel *chan) {
  state(chan, mwChannel_OPEN, 0);
  timestamp_stat(chan, mwChannelStat_OPENED_AT);
  flush_channel(chan);
}


int mwChannel_accept(struct mwChannel *chan) {
  struct mwSession *session;
  struct mwMsgChannelAccept *msg;
  struct mwCipherInstance *ci;

  int ret;

  g_return_val_if_fail(chan != NULL, -1);
  g_return_val_if_fail(mwChannel_isIncoming(chan), -1);
  g_return_val_if_fail(chan->state == mwChannel_WAIT, -1);

  session = chan->session;
  g_return_val_if_fail(session != NULL, -1);

  msg = (struct mwMsgChannelAccept *)
    mwMessage_new(mwMessage_CHANNEL_ACCEPT);

  msg->head.channel = chan->id;
  msg->service = chan->service;
  msg->proto_type = chan->proto_type;
  msg->proto_ver = chan->proto_ver;
  mwOpaque_clone(&msg->addtl, &chan->addtl_accept);

  ci = chan->cipher;

  if(! ci) {
    /* automatically select a cipher if one hasn't been already */

    switch(chan->offered_policy) {
    case mwEncrypt_NONE:
      mwChannel_selectCipherInstance(chan, NULL);
      break;
      
    case mwEncrypt_RC2_40:
      ci = get_supported(chan, mwCipher_RC2_40);
      mwChannel_selectCipherInstance(chan, ci);
      break;

    case mwEncrypt_RC2_128:
      ci = get_supported(chan, mwCipher_RC2_128);
      mwChannel_selectCipherInstance(chan, ci);
      break;
      
    case mwEncrypt_WHATEVER:
    case mwEncrypt_ALL:
    default:
      {
	GList *l, *ll;

	l = mwChannel_getSupportedCipherInstances(chan);
	if(l) {
	  /* nobody selected a cipher, so we'll just pick the last in
	     the list of available ones */
	  for(ll = l; ll->next; ll = ll->next);
	  ci = ll->data;
	  g_list_free(l);
	  
	  mwChannel_selectCipherInstance(chan, ci);
	  
	} else {
	  /* this may cause breakage, but there's really nothing else
	     we can do. They want something we can't provide. If they
	     don't like it, then they'll error the channel out */
	  mwChannel_selectCipherInstance(chan, NULL);
	}
      }
    }
  }

  msg->encrypt.mode = chan->policy; /* set in selectCipherInstance */
  msg->encrypt.extra = chan->offered_policy;

  if(chan->cipher) {
    msg->encrypt.item = mwCipherInstance_accept(chan->cipher);
  }

  ret = mwSession_send(session, MW_MESSAGE(msg));
  mwMessage_free(MW_MESSAGE(msg));

  if(ret) {
    state(chan, mwChannel_ERROR, ret);
  } else {
    channel_open(chan);
  }

  return ret;
}


static void channel_free(struct mwChannel *chan) {
  struct mwSession *s;
  struct mwMessage *msg;
  GSList *l;

  /* maybe no warning in the future */
  g_return_if_fail(chan != NULL);

  s = chan->session;

  mwLoginInfo_clear(&chan->user);
  mwOpaque_clear(&chan->addtl_create);
  mwOpaque_clear(&chan->addtl_accept);

  if(chan->supported) {
    g_hash_table_destroy(chan->supported);
    chan->supported = NULL;
  }

  if(chan->stats) {
    g_hash_table_destroy(chan->stats);
    chan->stats = NULL;
  }
  
  mwCipherInstance_free(chan->cipher);

  /* clean up the outgoing queue */
  for(l = chan->outgoing_queue; l; l = l->next) {
    msg = (struct mwMessage *) l->data;
    l->data = NULL;
    mwMessage_free(msg);
  }
  g_slist_free(chan->outgoing_queue);

  /* clean up the incoming queue */
  for(l = chan->incoming_queue; l; l = l->next) {
    msg = (struct mwMessage *) l->data;
    l->data = NULL;
    mwMessage_free(msg);
  }
  g_slist_free(chan->incoming_queue);

  g_free(chan);
}


int mwChannel_destroy(struct mwChannel *chan,
		      guint32 reason, struct mwOpaque *info) {

  struct mwMsgChannelDestroy *msg;
  struct mwSession *session;
  struct mwChannelSet *cs;
  int ret;

  /* may make this not a warning in the future */
  g_return_val_if_fail(chan != NULL, 0);

  state(chan, reason? mwChannel_ERROR: mwChannel_DESTROY, reason);

  session = chan->session;
  g_return_val_if_fail(session != NULL, -1);

  cs = mwSession_getChannels(session);
  g_return_val_if_fail(cs != NULL, -1);

  /* compose the message */
  msg = (struct mwMsgChannelDestroy *)
    mwMessage_new(mwMessage_CHANNEL_DESTROY);
  msg->head.channel = chan->id;
  msg->reason = reason;
  if(info) mwOpaque_clone(&msg->data, info);

  /* remove the channel from the channel set */
  g_hash_table_remove(cs->map, GUINT_TO_POINTER(chan->id));
  
  /* send the message */
  ret = mwSession_send(session, (struct mwMessage *) msg);
  mwMessage_free(MW_MESSAGE(msg));

  return ret;
}


static void queue_outgoing(struct mwChannel *chan,
			   struct mwMsgChannelSend *msg) {
  // Miranda NG adaptation
  //g_info("queue_outgoing, channel 0x%08x", chan->id);
  g_message("queue_outgoing, channel 0x%08x", chan->id);
  chan->outgoing_queue = g_slist_append(chan->outgoing_queue, msg);
}


static int channel_send(struct mwChannel *chan,
			struct mwMsgChannelSend *msg) {

  int ret = 0;

  /* if the channel is open, send and free the message. Otherwise,
     queue the message to be sent once the channel is finally
     opened */

  if(chan->state == mwChannel_OPEN) {
    ret = mwSession_send(chan->session, (struct mwMessage *) msg);
    mwMessage_free(MW_MESSAGE(msg));

  } else {
    queue_outgoing(chan, msg);
  }

  return ret;
}


int mwChannel_sendEncrypted(struct mwChannel *chan,
			    guint32 type, struct mwOpaque *data,
			    gboolean encrypt) {

  struct mwMsgChannelSend *msg;

  g_return_val_if_fail(chan != NULL, -1);

  msg = (struct mwMsgChannelSend *) mwMessage_new(mwMessage_CHANNEL_SEND);
  msg->head.channel = chan->id;
  msg->type = type;

  mwOpaque_clone(&msg->data, data);

  if(encrypt && chan->cipher) {
    msg->head.options = mwMessageOption_ENCRYPT;
    mwCipherInstance_encrypt(chan->cipher, &msg->data);
  }

  return channel_send(chan, msg);  
}


int mwChannel_send(struct mwChannel *chan, guint32 type,
		   struct mwOpaque *data) {

  return mwChannel_sendEncrypted(chan, type, data, TRUE);
}


static void queue_incoming(struct mwChannel *chan,
			   struct mwMsgChannelSend *msg) {

  /* we clone the message, because session_process will clear it once
     we return */

  struct mwMsgChannelSend *m = g_new0(struct mwMsgChannelSend, 1);
  m->head.type = msg->head.type;
  m->head.options = msg->head.options;
  m->head.channel = msg->head.channel;
  mwOpaque_clone(&m->head.attribs, &msg->head.attribs);

  m->type = msg->type;
  mwOpaque_clone(&m->data, &msg->data);

  // Miranda NG adaptation
  //g_info("queue_incoming, channel 0x%08x", chan->id);
  g_message("queue_incoming, channel 0x%08x", chan->id);
  chan->incoming_queue = g_slist_append(chan->incoming_queue, m);
}


static void channel_recv(struct mwChannel *chan,
			 struct mwMsgChannelSend *msg) {

  struct mwService *srvc;

  srvc = mwChannel_getService(chan);
  incr_stat(chan, mwChannelStat_MSG_RECV, 1);

  if(msg->head.options & mwMessageOption_ENCRYPT) {
    struct mwOpaque data = { 0, 0 };
    mwOpaque_clone(&data, &msg->data);

    mwCipherInstance_decrypt(chan->cipher, &data);
    mwService_recv(srvc, chan, msg->type, &data);
    mwOpaque_clear(&data);
    
  } else {
    mwService_recv(srvc, chan, msg->type, &msg->data);
  }
}


static void flush_channel(struct mwChannel *chan) {
  GSList *l;

  for(l = chan->incoming_queue; l; l = l->next) {
    struct mwMsgChannelSend *msg = (struct mwMsgChannelSend *) l->data;
    l->data = NULL;

    channel_recv(chan, msg);
    mwMessage_free(MW_MESSAGE(msg));
  }
  g_slist_free(chan->incoming_queue);
  chan->incoming_queue = NULL;

  for(l = chan->outgoing_queue; l; l = l->next) {
    struct mwMessage *msg = (struct mwMessage *) l->data;
    l->data = NULL;

    mwSession_send(chan->session, msg);
    mwMessage_free(msg);
  }
  g_slist_free(chan->outgoing_queue);
  chan->outgoing_queue = NULL;
}


void mwChannel_recv(struct mwChannel *chan, struct mwMsgChannelSend *msg) {
  if(chan->state == mwChannel_OPEN) {
    channel_recv(chan, msg);

  } else {
    queue_incoming(chan, msg);
  }
}


struct mwChannel *mwChannel_find(struct mwChannelSet *cs, guint32 chan) {
  g_return_val_if_fail(cs != NULL, NULL);
  g_return_val_if_fail(cs->map != NULL, NULL);
  return g_hash_table_lookup(cs->map, GUINT_TO_POINTER(chan));
}


void mwChannelSet_free(struct mwChannelSet *cs) {
  if(! cs) return;
  if(cs->map) g_hash_table_destroy(cs->map);
  g_free(cs);
}


struct mwChannelSet *mwChannelSet_new(struct mwSession *s) {
  struct mwChannelSet *cs = g_new0(struct mwChannelSet, 1);
  cs->session = s;

  /* for some reason, g_int_hash/g_int_equal cause a SIGSEGV */
  cs->map = g_hash_table_new_full(g_direct_hash, g_direct_equal,
				  NULL, (GDestroyNotify) channel_free);
  return cs;
}


void mwChannel_recvCreate(struct mwChannel *chan,
			  struct mwMsgChannelCreate *msg) {

  struct mwSession *session;
  GList *list;
  struct mwService *srvc;
  
  g_return_if_fail(chan != NULL);
  g_return_if_fail(msg != NULL);
  g_return_if_fail(chan->id == msg->channel);

  session = chan->session;
  g_return_if_fail(session != NULL);

  if(mwChannel_isOutgoing(chan)) {
    g_warning("channel 0x%08x not an incoming channel", chan->id);
    mwChannel_destroy(chan, ERR_REQUEST_INVALID, NULL);
    return;
  }

  chan->offered_policy = msg->encrypt.mode;
  g_message("channel offered with encrypt policy 0x%04x", chan->policy);

  for(list = msg->encrypt.items; list; list = list->next) {
    struct mwEncryptItem *ei = list->data;
    struct mwCipher *cipher;
    struct mwCipherInstance *ci;

    g_message("channel offered cipher id 0x%04x", ei->id);
    cipher = mwSession_getCipher(session, ei->id);
    if(! cipher) {
      g_message("no such cipher found in session");
      continue;
    }

    ci = mwCipher_newInstance(cipher, chan);
    mwCipherInstance_offered(ci, ei);
    mwChannel_addSupportedCipherInstance(chan, ci);
  }

  mwLoginInfo_clone(&chan->user, &msg->creator);
  chan->service = msg->service;
  chan->proto_type = msg->proto_type;
  chan->proto_ver = msg->proto_ver;
  
  srvc = mwSession_getService(session, msg->service);
  if(srvc) {
    mwService_recvCreate(srvc, chan, msg);

  } else {
    mwChannel_destroy(chan, ERR_SERVICE_NO_SUPPORT, NULL);
  }  
}


void mwChannel_recvAccept(struct mwChannel *chan,
			  struct mwMsgChannelAccept *msg) {

  struct mwService *srvc;

  g_return_if_fail(chan != NULL);
  g_return_if_fail(msg != NULL);
  g_return_if_fail(chan->id == msg->head.channel);

  if(mwChannel_isIncoming(chan)) {
    g_warning("channel 0x%08x not an outgoing channel", chan->id);
    mwChannel_destroy(chan, ERR_REQUEST_INVALID, NULL);
    return;
  }

  if(chan->state != mwChannel_WAIT) {
    g_warning("channel 0x%08x state not WAIT: %s",
	      chan->id, state_str(chan->state));
    mwChannel_destroy(chan, ERR_REQUEST_INVALID, NULL);
    return;
  }

  mwLoginInfo_clone(&chan->user, &msg->acceptor);

  srvc = mwSession_getService(chan->session, chan->service);
  if(! srvc) {
    g_warning("no service: 0x%08x", chan->service);
    mwChannel_destroy(chan, ERR_SERVICE_NO_SUPPORT, NULL);
    return;
  }

  chan->policy = msg->encrypt.mode;
  g_message("channel accepted with encrypt policy 0x%04x", chan->policy);

  if(! msg->encrypt.mode || ! msg->encrypt.item) {
    /* no mode or no item means no encryption */
    mwChannel_selectCipherInstance(chan, NULL);

  } else {
    guint16 cid = msg->encrypt.item->id;
    struct mwCipherInstance *ci = get_supported(chan, cid);

    if(! ci) {
      g_warning("not an offered cipher: 0x%04x", cid);
      mwChannel_destroy(chan, ERR_REQUEST_INVALID, NULL);
      return;
    }

    mwCipherInstance_accepted(ci, msg->encrypt.item);
    mwChannel_selectCipherInstance(chan, ci);
  }

  /* mark it as open for the service */
  state(chan, mwChannel_OPEN, 0);

  /* let the service know */
  mwService_recvAccept(srvc, chan, msg);

  /* flush it if the service didn't just immediately close it */
  if(mwChannel_isState(chan, mwChannel_OPEN)) {
    channel_open(chan);
  }
}


void mwChannel_recvDestroy(struct mwChannel *chan,
			   struct mwMsgChannelDestroy *msg) {

  struct mwChannelSet *cs;
  struct mwService *srvc;

  g_return_if_fail(chan != NULL);
  g_return_if_fail(msg != NULL);
  g_return_if_fail(chan->id == msg->head.channel);

  state(chan, msg->reason? mwChannel_ERROR: mwChannel_DESTROY, msg->reason);

  srvc = mwChannel_getService(chan);
  if(srvc) mwService_recvDestroy(srvc, chan, msg);

  cs = mwSession_getChannels(chan->session);
  g_return_if_fail(cs != NULL);
  g_return_if_fail(cs->map != NULL);

  g_hash_table_remove(cs->map, GUINT_TO_POINTER(chan->id));
}


void mwChannel_populateSupportedCipherInstances(struct mwChannel *chan) {
  struct mwSession *session;
  GList *list;

  g_return_if_fail(chan != NULL);

  session = chan->session;
  g_return_if_fail(session != NULL);

  for(list = mwSession_getCiphers(session); list; list = list->next) {
    struct mwCipherInstance *ci = mwCipher_newInstance(list->data, chan);
    if(! ci) continue;
    put_supported(chan, ci);
  }
}


void mwChannel_addSupportedCipherInstance(struct mwChannel *chan,
					  struct mwCipherInstance *ci) {
  g_return_if_fail(chan != NULL);
  g_message("channel 0x%08x added cipher %s", chan->id,
	    NSTR(mwCipher_getName(mwCipherInstance_getCipher(ci))));
  put_supported(chan, ci);
}


static void collect(gpointer a, gpointer b, gpointer c) {
  GList **list = c;
  *list = g_list_append(*list, b);
}


GList *mwChannel_getSupportedCipherInstances(struct mwChannel *chan) {
  GList *list = NULL;

  g_return_val_if_fail(chan != NULL, NULL);
  g_hash_table_foreach(chan->supported, collect, &list);

  return list;
}


void mwChannel_selectCipherInstance(struct mwChannel *chan,
				    struct mwCipherInstance *ci) {
  struct mwCipher *c;

  g_return_if_fail(chan != NULL);
  g_return_if_fail(chan->supported != NULL);

  chan->cipher = ci;
  if(ci) {
    guint cid;

    c = mwCipherInstance_getCipher(ci);
    cid = mwCipher_getType(c);

    g_hash_table_steal(chan->supported, GUINT_TO_POINTER(cid));

    switch(mwCipher_getType(c)) {
    case mwCipher_RC2_40:
      chan->policy = mwEncrypt_RC2_40;
      break;

    case mwCipher_RC2_128:
      chan->policy = mwEncrypt_RC2_128;
      break;

    default:
      /* unsure if this is bad */
      chan->policy = mwEncrypt_WHATEVER;
    }

    g_message("channel 0x%08x selected cipher %s",
	      chan->id, NSTR(mwCipher_getName(c)));

  } else {

    chan->policy = mwEncrypt_NONE;
    g_message("channel 0x%08x selected no cipher", chan->id);
  }

  g_hash_table_destroy(chan->supported);
  chan->supported = NULL;
}


