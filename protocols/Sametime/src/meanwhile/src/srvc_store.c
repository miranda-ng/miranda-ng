
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

#include <glib/glist.h>

#include "mw_channel.h"
#include "mw_debug.h"
#include "mw_error.h"
#include "mw_message.h"
#include "mw_service.h"
#include "mw_session.h"
#include "mw_srvc_store.h"


#define PROTOCOL_TYPE  0x00000025
#define PROTOCOL_VER   0x00000001


enum storage_action {
  action_load    = 0x0004,
  action_loaded  = 0x0005,
  action_save    = 0x0006,
  action_saved   = 0x0007,
};


struct mwStorageUnit {
  /** key by which data is referenced in service
      @see mwStorageKey */
  guint32 key;

  /** Data associated with key in service */
  struct mwOpaque data;
};


struct mwStorageReq {
  guint32 id;                  /**< unique id for this request */
  guint32 result_code;         /**< result code for completed request */
  enum storage_action action;  /**< load or save */
  struct mwStorageUnit *item;  /**< the key/data pair */ 
  mwStorageCallback cb;        /**< callback to notify upon completion */
  gpointer data;               /**< user data to pass with callback */
  GDestroyNotify data_free;    /**< optionally frees user data */
};


struct mwServiceStorage {
  struct mwService service;

  /** collection of mwStorageReq */
  GList *pending;

  /** current service channel */
  struct mwChannel *channel;

  /** keep track of the counter */
  guint32 id_counter;
};


static void request_get(struct mwGetBuffer *b, struct mwStorageReq *req) {
  guint32 id, count, junk;

  if(mwGetBuffer_error(b)) return;

  guint32_get(b, &id);
  guint32_get(b, &req->result_code);

  if(req->action == action_loaded) {
    guint32_get(b, &count);

    if(count > 0) {
      guint32_get(b, &junk);
      guint32_get(b, &req->item->key);

      mwOpaque_clear(&req->item->data);
      mwOpaque_get(b, &req->item->data);
    }
  }
}


static void request_put(struct mwPutBuffer *b, struct mwStorageReq *req) {

  guint32_put(b, req->id);
  guint32_put(b, 1);

  if(req->action == action_save) {
    guint32_put(b, 20 + req->item->data.len); /* ugh, offset garbage */
    guint32_put(b, req->item->key);
    mwOpaque_put(b, &req->item->data);

  } else {
    guint32_put(b, req->item->key);
  }
}


static int request_send(struct mwChannel *chan, struct mwStorageReq *req) {
  struct mwPutBuffer *b;
  struct mwOpaque o = { 0, 0 };
  int ret;

  b = mwPutBuffer_new();
  request_put(b, req);

  mwPutBuffer_finalize(&o, b);
  ret = mwChannel_send(chan, req->action, &o);
  mwOpaque_clear(&o);

  if(! ret) {
    if(req->action == action_save) {
      req->action = action_saved;
    } else if(req->action == action_load) {
      req->action = action_loaded;
    }
  }

  return ret;
}


static struct mwStorageReq *request_find(struct mwServiceStorage *srvc,
					 guint32 id) {
  GList *l;

  for(l = srvc->pending; l; l = l->next) {
    struct mwStorageReq *r = l->data;
    if(r->id == id) return r;
  }

  return NULL;
}


static const char *action_str(enum storage_action act) {
  switch(act) {
  case action_load:    return "load";
  case action_loaded:  return "loaded";
  case action_save:    return "save";
  case action_saved:   return "saved";
  default:             return "UNKNOWN";
  }
}


static void request_trigger(struct mwServiceStorage *srvc,
			    struct mwStorageReq *req) {

  struct mwStorageUnit *item = req->item;

  g_message("storage request %s: key = 0x%x, result = 0x%x, length = %u",
	    action_str(req->action),
	    item->key, req->result_code, (guint) item->data.len);
  
  if(req->cb)
    req->cb(srvc, req->result_code, item, req->data);
}


static void request_free(struct mwStorageReq *req) {
  if(req->data_free) {
    req->data_free(req->data);
    req->data = NULL;
    req->data_free = NULL;
  }

  mwStorageUnit_free(req->item);
  g_free(req);
}


static void request_remove(struct mwServiceStorage *srvc,
			   struct mwStorageReq *req) {

  srvc->pending = g_list_remove_all(srvc->pending, req);
  request_free(req);
}


static const char *get_name(struct mwService *srvc) {
  return "User Storage";
}


static const char *get_desc(struct mwService *srvc) {
  return "Stores user data and settings on the server";
}


static struct mwChannel *make_channel(struct mwServiceStorage *srvc) {
  struct mwSession *session;
  struct mwChannelSet *cs;
  struct mwChannel *chan;

  session = mwService_getSession(MW_SERVICE(srvc));
  cs = mwSession_getChannels(session);
  chan = mwChannel_newOutgoing(cs);
 
  mwChannel_setService(chan, MW_SERVICE(srvc));
  mwChannel_setProtoType(chan, PROTOCOL_TYPE);
  mwChannel_setProtoVer(chan, PROTOCOL_VER);

  return mwChannel_create(chan)? NULL: chan;
}


static void start(struct mwService *srvc) {
  struct mwServiceStorage *srvc_store;
  struct mwChannel *chan;

  g_return_if_fail(srvc != NULL);
  srvc_store = (struct mwServiceStorage *) srvc;

  chan = make_channel(srvc_store);
  if(chan) {
    srvc_store->channel = chan;
  } else {
    mwService_stopped(srvc);
  }
}


static void stop(struct mwService *srvc) {

  struct mwServiceStorage *srvc_store;
  GList *l;

  g_return_if_fail(srvc != NULL);
  srvc_store = (struct mwServiceStorage *) srvc;

  if(srvc_store->channel) {
    mwChannel_destroy(srvc_store->channel, ERR_SUCCESS, NULL);
    srvc_store->channel = NULL;
  }

#if 1
  /* the new way */
  /* remove pending requests. Sometimes we can crash the storage
     service, and when that happens, we end up resending the killer
     request over and over again, and the service never stays up */
  for(l = srvc_store->pending; l; l = l->next)
    request_free(l->data);

  g_list_free(srvc_store->pending);
  srvc_store->pending = NULL;

  srvc_store->id_counter = 0;

#else
  /* the old way */
  /* reset all of the started requests to their unstarted states */
  for(l = srvc_store->pending; l; l = l->next) {
    struct mwStorageReq *req = l->data;

    if(req->action == action_loaded) {
      req->action = action_load;
    } else if(req->action == action_saved) {
      req->action = action_save;
    }
  }
#endif

  mwService_stopped(srvc);
}


static void recv_channelAccept(struct mwService *srvc,
			       struct mwChannel *chan,
			       struct mwMsgChannelAccept *msg) {
 
  struct mwServiceStorage *srvc_stor;
  GList *l;

  g_return_if_fail(srvc != NULL);
  srvc_stor = (struct mwServiceStorage *) srvc;

  g_return_if_fail(chan != NULL);
  g_return_if_fail(chan == srvc_stor->channel);

  /* send all pending requests */
  for(l = srvc_stor->pending; l; l = l->next) {
    struct mwStorageReq *req = l->data;

    if(req->action == action_save || req->action == action_load) {
      request_send(chan, req);
    }
  }

  mwService_started(srvc);
}


static void recv_channelDestroy(struct mwService *srvc,
				struct mwChannel *chan,
				struct mwMsgChannelDestroy *msg) {

  struct mwSession *session;
  struct mwServiceStorage *srvc_stor;

  g_return_if_fail(srvc != NULL);
  g_return_if_fail(chan != NULL);

  session = mwService_getSession(srvc);
  g_return_if_fail(session != NULL);

  srvc_stor = (struct mwServiceStorage *) srvc;
  srvc_stor->channel = NULL;

  mwService_stop(srvc);
  mwSession_senseService(session, mwService_getType(srvc));
}


static void recv(struct mwService *srvc, struct mwChannel *chan,
		 guint16 type, struct mwOpaque *data) {

  /* process into results, trigger callbacks */

  struct mwGetBuffer *b;
  struct mwServiceStorage *srvc_stor;
  struct mwStorageReq *req;
  guint32 id;

  g_return_if_fail(srvc != NULL);
  srvc_stor = (struct mwServiceStorage *) srvc;

  g_return_if_fail(chan != NULL);
  g_return_if_fail(chan == srvc_stor->channel);
  g_return_if_fail(data != NULL);

  b = mwGetBuffer_wrap(data);

  id = guint32_peek(b);
  req = request_find(srvc_stor, id);

  if(! req) {
    g_warning("couldn't find request 0x%x in storage service", id);
    mwGetBuffer_free(b);
    return;
  }

  g_return_if_fail(req->action == type);
  request_get(b, req);

  if(mwGetBuffer_error(b)) {
    mw_mailme_opaque(data, "storage request 0x%x, type: 0x%x", id, type);

  } else {
    request_trigger(srvc_stor, req);
  }

  mwGetBuffer_free(b);
  request_remove(srvc_stor, req);
}


static void clear(struct mwService *srvc) {
  struct mwServiceStorage *srvc_stor;
  GList *l;

  srvc_stor = (struct mwServiceStorage *) srvc;

  for(l = srvc_stor->pending; l; l = l->next)
    request_free(l->data);

  g_list_free(srvc_stor->pending);
  srvc_stor->pending = NULL;

  srvc_stor->id_counter = 0;
}


struct mwServiceStorage *mwServiceStorage_new(struct mwSession *session) {
  struct mwServiceStorage *srvc_store;
  struct mwService *srvc;

  srvc_store = g_new0(struct mwServiceStorage, 1);
  srvc = MW_SERVICE(srvc_store);

  mwService_init(srvc, session, mwService_STORAGE);
  srvc->get_name = get_name;
  srvc->get_desc = get_desc;
  srvc->recv_accept = recv_channelAccept;
  srvc->recv_destroy = recv_channelDestroy;
  srvc->recv = recv;
  srvc->start = start;
  srvc->stop = stop;
  srvc->clear = clear;

  return srvc_store;
}


struct mwStorageUnit *mwStorageUnit_new(guint32 key) {
  struct mwStorageUnit *u;

  u = g_new0(struct mwStorageUnit, 1);
  u->key = key;

  return u;
}


struct mwStorageUnit *mwStorageUnit_newOpaque(guint32 key,
					      struct mwOpaque *data) {
  struct mwStorageUnit *u;

  u = g_new0(struct mwStorageUnit, 1);
  u->key = key;

  if(data)
    mwOpaque_clone(&u->data, data);

  return u;
}


struct mwStorageUnit *mwStorageUnit_newBoolean(guint32 key,
					       gboolean val) {

  return mwStorageUnit_newInteger(key, (guint32) val);
}


struct mwStorageUnit *mwStorageUnit_newInteger(guint32 key,
					       guint32 val) {
  struct mwStorageUnit *u;
  struct mwPutBuffer *b;

  u = g_new0(struct mwStorageUnit, 1);
  u->key = key;
  
  b = mwPutBuffer_new();
  guint32_put(b, val);
  mwPutBuffer_finalize(&u->data, b);

  return u;
}


struct mwStorageUnit *mwStorageUnit_newString(guint32 key,
					      const char *str) {
  struct mwStorageUnit *u;
  struct mwPutBuffer *b;

  u = g_new0(struct mwStorageUnit, 1);
  u->key = key;

  b = mwPutBuffer_new();
  mwString_put(b, str);
  mwPutBuffer_finalize(&u->data, b);

  return u;
}


guint32 mwStorageUnit_getKey(struct mwStorageUnit *item) {
  g_return_val_if_fail(item != NULL, 0x00); /* feh, unsafe */
  return item->key;
}


gboolean mwStorageUnit_asBoolean(struct mwStorageUnit *item,
				 gboolean val) {

  return !! mwStorageUnit_asInteger(item, (guint32) val);
}


guint32 mwStorageUnit_asInteger(struct mwStorageUnit *item,
				guint32 val) {
  struct mwGetBuffer *b;
  guint32 v;

  g_return_val_if_fail(item != NULL, val);

  b = mwGetBuffer_wrap(&item->data);

  guint32_get(b, &v);
  if(! mwGetBuffer_error(b)) val = v;
  mwGetBuffer_free(b);

  return val;
}


char *mwStorageUnit_asString(struct mwStorageUnit *item) {
  struct mwGetBuffer *b;
  char *c = NULL;

  g_return_val_if_fail(item != NULL, NULL);

  b = mwGetBuffer_wrap(&item->data);

  mwString_get(b, &c);

  if(mwGetBuffer_error(b))
    g_debug("error obtaining string value from opaque");

  mwGetBuffer_free(b);

  return c;
}


struct mwOpaque *mwStorageUnit_asOpaque(struct mwStorageUnit *item) {
  g_return_val_if_fail(item != NULL, NULL);
  return &item->data;
}


void mwStorageUnit_free(struct mwStorageUnit *item) {
  if(! item) return;

  mwOpaque_clear(&item->data);
  g_free(item);
}


static struct mwStorageReq *request_new(struct mwServiceStorage *srvc,
					struct mwStorageUnit *item,
					mwStorageCallback cb,
					gpointer data, GDestroyNotify df) {

  struct mwStorageReq *req = g_new0(struct mwStorageReq, 1);

  req->id = ++srvc->id_counter;
  req->item = item;
  req->cb = cb;
  req->data = data;
  req->data_free = df;

  return req;
}


void mwServiceStorage_load(struct mwServiceStorage *srvc,
			   struct mwStorageUnit *item,
			   mwStorageCallback cb,
			   gpointer data, GDestroyNotify d_free) {

  /* - construct a request
     - put request at end of pending
     - if channel is open and connected
       - compose the load message
       - send message
       - set request to sent
     - else
       - start service
  */ 

  struct mwStorageReq *req;

  req = request_new(srvc, item, cb, data, d_free);
  req->action = action_load;

  srvc->pending = g_list_append(srvc->pending, req);

  if(MW_SERVICE_IS_STARTED(MW_SERVICE(srvc)))
    request_send(srvc->channel, req);
}


void mwServiceStorage_save(struct mwServiceStorage *srvc,
			   struct mwStorageUnit *item,
			   mwStorageCallback cb,
			   gpointer data, GDestroyNotify d_free) {

  /* - construct a request
     - put request at end of pending
     - if channel is open and connected
       - compose the save message
       - send message
       - set request to sent
     - else
       - start service
  */

  struct mwStorageReq *req;

  req = request_new(srvc, item, cb, data, d_free);
  req->action = action_save;

  srvc->pending = g_list_append(srvc->pending, req);

  if(MW_SERVICE_IS_STARTED(MW_SERVICE(srvc)))
    request_send(srvc->channel, req);
}


/// Miranda NG adaptation start - new method
struct mwService *mwServiceStorage_getService(struct mwServiceStorage *srvc) {
  g_return_val_if_fail(srvc != NULL, NULL);
  return &(srvc->service);
}
/// Miranda NG adaptation end
