
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

#include "mw_channel.h"
#include "mw_common.h"
#include "mw_debug.h"
#include "mw_error.h"
#include "mw_message.h"
#include "mw_service.h"
#include "mw_session.h"
#include "mw_srvc_dir.h"
#include "mw_util.h"


#define PROTOCOL_TYPE  0x0000001c
#define PROTOCOL_VER   0x00000005


enum dir_action {
  action_list    = 0x0000,  /**< list address books */
  action_open    = 0x0001,  /**< open an addressbook as a directory */
  action_close   = 0x0002,  /**< close a directory */
  action_search  = 0x0003,  /**< search an open directory */
};


struct mwServiceDirectory {
  struct mwService service;

  struct mwDirectoryHandler *handler;

  struct mwChannel *channel;
 
  guint32 counter;       /**< counter of request IDs */
  GHashTable *requests;  /**< map of request ID:directory */
  GHashTable *books;     /**< book->name:mwAddressBook */
};


struct mwAddressBook {
  struct mwServiceDirectory *service;

  guint32 id;        /**< id or type or something */
  char *name;        /**< name of address book */
  GHashTable *dirs;  /**< dir->id:mwDirectory */
};


struct mwDirectory {
  struct mwServiceDirectory *service;
  struct mwAddressBook *book;

  enum mwDirectoryState state;

  guint32 id;         /**< id of directory, assigned by server */
  guint32 search_id;  /**< id of current search, from srvc->counter++ */

  mwSearchHandler handler;
  struct mw_datum client_data;
};


#define next_request_id(srvc) ( ++((srvc)->counter) )


static guint32 map_request(struct mwDirectory *dir) {
  struct mwServiceDirectory *srvc = dir->service;
  guint32 id = next_request_id(srvc);

  dir->search_id = id;
  map_guint_insert(srvc->requests, id, dir);

  return id;
}


/** called when directory is removed from the service directory map */
static void dir_free(struct mwDirectory *dir) {
  map_guint_remove(dir->service->requests, dir->search_id);
  g_free(dir);
}


/** remove the directory from the service list and its owning address
    book, then frees the directory */
static void dir_remove(struct mwDirectory *dir) {
  struct mwAddressBook *book = dir->book;
  map_guint_remove(book->dirs, dir->id);
}


/// __attribute__((used)) ///Miranda NG adaptation, MSVC
static struct mwDirectory *dir_new(struct mwAddressBook *book, guint32 id) {
  struct mwDirectory *dir = g_new0(struct mwDirectory, 1);
  dir->service = book->service;
  dir->book = book;
  dir->id = id;
  map_guint_insert(book->dirs, id, dir);
  return dir;
}


/** called when book is removed from the service book map. Removed all
    directories as well */
static void book_free(struct mwAddressBook *book) {
  g_hash_table_destroy(book->dirs);
  g_free(book->name);
}


/// __attribute__((used)) ///Miranda NG adaptation, MSVC
static void book_remove(struct mwAddressBook *book) {
  struct mwServiceDirectory *srvc = book->service;
  g_hash_table_remove(srvc->books, book->name);
}


static struct mwAddressBook *book_new(struct mwServiceDirectory *srvc,
				      const char *name, guint32 id) {
  struct mwAddressBook *book = g_new0(struct mwAddressBook, 1);
  book->service = srvc;
  book->id = id;
  book->name = g_strdup(name);
  book->dirs = map_guint_new_full((GDestroyNotify) dir_free);
  g_hash_table_insert(srvc->books, book->name, book);
  return book;
}


static const char *getName(struct mwService *srvc) {
  return "Address Book and Directory";
}


static const char *getDesc(struct mwService *srvc) {
  return "Address book directory service for user and group lookups";
}


static struct mwChannel *make_channel(struct mwServiceDirectory *srvc) {
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


static void start(struct mwServiceDirectory *srvc) {
  struct mwChannel *chan;

  chan = make_channel(srvc);
  if(chan) {
    srvc->channel = chan;
  } else {
    mwService_stopped(MW_SERVICE(srvc));
    return;
  }
}


static void stop(struct mwServiceDirectory *srvc) {
  /* XXX */

  if(srvc->channel) {
    mwChannel_destroy(srvc->channel, ERR_SUCCESS, NULL);
    srvc->channel = NULL;
  }
}


static void clear(struct mwServiceDirectory *srvc) {
  struct mwDirectoryHandler *handler;

  if(srvc->books) {
    g_hash_table_destroy(srvc->books);
    srvc->books = NULL;
  }
  
  /* clear the handler */
  handler = srvc->handler;
  if(handler && handler->clear)
    handler->clear(srvc);
  srvc->handler = NULL;
}


static void recv_create(struct mwServiceDirectory *srvc,
			struct mwChannel *chan,
			struct mwMsgChannelCreate *msg) {

  /* no way man, we call the shots around here */
  mwChannel_destroy(chan, ERR_FAILURE, NULL);
}


static void recv_accept(struct mwServiceDirectory *srvc,
			struct mwChannel *chan,
			struct mwMsgChannelAccept *msg) {

  g_return_if_fail(srvc->channel != NULL);
  g_return_if_fail(srvc->channel == chan);

  if(MW_SERVICE_IS_STARTING(srvc)) {
    mwService_started(MW_SERVICE(srvc));
      
  } else {
    mwChannel_destroy(chan, ERR_FAILURE, NULL);
  }
}


static void recv_destroy(struct mwServiceDirectory *srvc,
			 struct mwChannel *chan,
			 struct mwMsgChannelDestroy *msg) {

  srvc->channel = NULL;
  mwService_stop(MW_SERVICE(srvc));
  /** @todo session sense service */
}


static void recv_list(struct mwServiceDirectory *srvc,
		      struct mwOpaque *data) {

  struct mwGetBuffer *b;
  guint32 request, code, count;
  gboolean foo_1;
  guint16 foo_2;
  
  b = mwGetBuffer_wrap(data);
  
  guint32_get(b, &request);
  guint32_get(b, &code);
  guint32_get(b, &count);

  gboolean_get(b, &foo_1);
  guint16_get(b, &foo_2);

  if(foo_1 || foo_2) {
    mw_mailme_opaque(data, "received strange address book list");
    mwGetBuffer_free(b);
    return;
  }

  while(!mwGetBuffer_error(b) && count--) {
    guint32 id;
    char *name = NULL;

    guint32_get(b, &id);
    mwString_get(b, &name);

    book_new(srvc, name, id);
    g_free(name);
  }
}


static void recv_open(struct mwServiceDirectory *srvc,
		      struct mwOpaque *data) {

  /* look up the directory associated with this request id, 
     mark it as open, and trigger the event */
}


static void recv_search(struct mwServiceDirectory *srvc,
			struct mwOpaque *data) {

  /* look up the directory associated with this request id,
     trigger the event */
}


static void recv(struct mwServiceDirectory *srvc,
		 struct mwChannel *chan,
		 guint16 msg_type, struct mwOpaque *data) {
  
  g_return_if_fail(srvc != NULL);
  g_return_if_fail(chan != NULL);
  g_return_if_fail(chan == srvc->channel);
  g_return_if_fail(data != NULL);

  switch(msg_type) {
  case action_list:
    recv_list(srvc, data);
    break;

  case action_open:
    recv_open(srvc, data);
    break;

  case action_close:
    ; /* I don't think we should receive these */
    break;

  case action_search:
    recv_search(srvc, data);
    break;

  default:
    mw_mailme_opaque(data, "msg type 0x%04x in directory service", msg_type);
  }
}


struct mwServiceDirectory *
mwServiceDirectory_new(struct mwSession *session,
		       struct mwDirectoryHandler *handler) {

  struct mwServiceDirectory *srvc;
  struct mwService *service;

  g_return_val_if_fail(session != NULL, NULL);
  g_return_val_if_fail(handler != NULL, NULL);

  srvc = g_new0(struct mwServiceDirectory, 1);
  service = MW_SERVICE(srvc);

  mwService_init(service, session, SERVICE_DIRECTORY);
  service->get_name = getName;
  service->get_desc = getDesc;
  service->start = (mwService_funcStart) start;
  service->stop = (mwService_funcStop) stop;
  service->clear = (mwService_funcClear) clear;
  service->recv_create = (mwService_funcRecvCreate) recv_create;
  service->recv_accept = (mwService_funcRecvAccept) recv_accept;
  service->recv_destroy = (mwService_funcRecvDestroy) recv_destroy;
  service->recv = (mwService_funcRecv) recv;

  srvc->handler = handler;
  srvc->requests = map_guint_new();
  srvc->books = g_hash_table_new_full(g_str_hash, g_str_equal,
				      NULL, (GDestroyNotify) book_free);
  return srvc;
}


struct mwDirectoryHandler *
mwServiceDirectory_getHandler(struct mwServiceDirectory *srvc) {
  g_return_val_if_fail(srvc != NULL, NULL);
  return srvc->handler;
}


int mwServiceDirectory_refreshAddressBooks(struct mwServiceDirectory *srvc) {
  struct mwChannel *chan;
  struct mwPutBuffer *b;
  struct mwOpaque o;
  int ret;

  g_return_val_if_fail(srvc != NULL, -1);

  chan = srvc->channel;
  g_return_val_if_fail(chan != NULL, -1);

  b = mwPutBuffer_new();
  guint32_put(b, next_request_id(srvc));

  mwPutBuffer_finalize(&o, b);
  ret = mwChannel_send(chan, action_list, &o);
  mwOpaque_clear(&o);

  return ret;
}


GList *mwServiceDirectory_getAddressBooks(struct mwServiceDirectory *srvc) {
  g_return_val_if_fail(srvc != NULL, NULL);
  g_return_val_if_fail(srvc->books != NULL, NULL);

  return map_collect_values(srvc->books);
}


GList *mwServiceDirectory_getDirectories(struct mwServiceDirectory *srvc) {
  GList *bl, *ret = NULL;
  
  g_return_val_if_fail(srvc != NULL, NULL);
  g_return_val_if_fail(srvc->books != NULL, NULL);

  bl = map_collect_values(srvc->books);
  for( ; bl; bl = g_list_delete_link(bl, bl)) {
    struct mwAddressBook *book = bl->data;
    ret = g_list_concat(ret, map_collect_values(book->dirs));
  }

  return ret;
}


GList *mwAddressBook_getDirectories(struct mwAddressBook *book) {
  g_return_val_if_fail(book != NULL, NULL);
  g_return_val_if_fail(book->dirs != NULL, NULL);

  return map_collect_values(book->dirs);
}


const char *mwAddressBook_getName(struct mwAddressBook *book) {
  g_return_val_if_fail(book != NULL, NULL);
  return book->name;
}


struct mwDirectory *mwDirectory_new(struct mwAddressBook *book) {
  struct mwDirectory *dir;

  g_return_val_if_fail(book != NULL, NULL);
  g_return_val_if_fail(book->service != NULL, NULL);

  dir = g_new0(struct mwDirectory, 1);
  dir->service = book->service;
  dir->book = book;
  dir->state = mwDirectory_NEW;

  return dir;
}


enum mwDirectoryState mwDirectory_getState(struct mwDirectory *dir) {
  g_return_val_if_fail(dir != NULL, mwDirectory_UNKNOWN);
  return dir->state;
}


void mwDirectory_setClientData(struct mwDirectory *dir,
			       gpointer data, GDestroyNotify clear) {

  g_return_if_fail(dir != NULL);
  mw_datum_set(&dir->client_data, data, clear);
}


gpointer mwDirectory_getClientData(struct mwDirectory *dir) {
  g_return_val_if_fail(dir != NULL, NULL);
  return mw_datum_get(&dir->client_data);
}


void mwDirectory_removeClientData(struct mwDirectory *dir) {
  g_return_if_fail(dir != NULL);
  mw_datum_clear(&dir->client_data);
}


struct mwServiceDirectory *mwDirectory_getService(struct mwDirectory *dir) {
  g_return_val_if_fail(dir != NULL, NULL);
  g_return_val_if_fail(dir->book != NULL, NULL);
  return dir->book->service;
}


struct mwAddressBook *mwDirectory_getAddressBook(struct mwDirectory *dir) {
  g_return_val_if_fail(dir != NULL, NULL);
  return dir->book;
}


static int dir_open(struct mwDirectory *dir) {
  struct mwServiceDirectory *srvc;
  struct mwChannel *chan;
  struct mwPutBuffer *b;
  struct mwOpaque o;
  int ret;

  g_return_val_if_fail(dir != NULL, -1);

  srvc = dir->service;
  g_return_val_if_fail(srvc != NULL, -1);

  chan = srvc->channel;
  g_return_val_if_fail(chan != NULL, -1);

  b = mwPutBuffer_new();
  guint32_put(b, map_request(dir));

  /* unsure about these three bytes */
  gboolean_put(b, FALSE);
  guint16_put(b, 0x0000);

  guint32_put(b, dir->book->id);
  mwString_put(b, dir->book->name);

  mwPutBuffer_finalize(&o, b);
  ret = mwChannel_send(chan, action_open, &o);
  mwOpaque_clear(&o);

  return ret;
}


int mwDirectory_open(struct mwDirectory *dir, mwSearchHandler cb) {
  g_return_val_if_fail(dir != NULL, -1);
  g_return_val_if_fail(cb != NULL, -1);
  g_return_val_if_fail(MW_DIRECTORY_IS_NEW(dir), -1);

  dir->state = mwDirectory_PENDING;
  dir->handler = cb;

  return dir_open(dir);
}


int mwDirectory_next(struct mwDirectory *dir) {  
  struct mwServiceDirectory *srvc;
  struct mwChannel *chan;
  struct mwPutBuffer *b;
  struct mwOpaque o;
  int ret;

  g_return_val_if_fail(dir != NULL, -1);
  g_return_val_if_fail(MW_DIRECTORY_IS_OPEN(dir), -1);

  srvc = dir->service;
  g_return_val_if_fail(srvc != NULL, -1);

  chan = srvc->channel;
  g_return_val_if_fail(chan != NULL, -1);

  b = mwPutBuffer_new();
  guint32_put(b, map_request(dir));
  guint32_put(b, dir->id);
  guint16_put(b, 0xffff);      /* some magic? */
  guint32_put(b, 0x00000000);  /* next results */

  mwPutBuffer_finalize(&o, b);
  ret = mwChannel_send(chan, action_search, &o);
  mwOpaque_clear(&o);

  return ret;
}


int mwDirectory_previous(struct mwDirectory *dir) {
  struct mwServiceDirectory *srvc;
  struct mwChannel *chan;
  struct mwPutBuffer *b;
  struct mwOpaque o;
  int ret;

  g_return_val_if_fail(dir != NULL, -1);
  g_return_val_if_fail(MW_DIRECTORY_IS_OPEN(dir), -1);

  srvc = dir->service;
  g_return_val_if_fail(srvc != NULL, -1);

  chan = srvc->channel;
  g_return_val_if_fail(chan != NULL, -1);

  b = mwPutBuffer_new();
  guint32_put(b, map_request(dir));
  guint32_put(b, dir->id);
  guint16_put(b, 0x0061);      /* some magic? */
  guint32_put(b, 0x00000001);  /* prev results */

  mwPutBuffer_finalize(&o, b);
  ret = mwChannel_send(chan, action_search, &o);
  mwOpaque_clear(&o);

  return ret;
}


int mwDirectory_search(struct mwDirectory *dir, const char *query) {
  struct mwServiceDirectory *srvc;
  struct mwChannel *chan;
  struct mwPutBuffer *b;
  struct mwOpaque o;
  int ret;

  g_return_val_if_fail(dir != NULL, -1);
  g_return_val_if_fail(MW_DIRECTORY_IS_OPEN(dir), -1);
  g_return_val_if_fail(query != NULL, -1);
  g_return_val_if_fail(*query != '\0', -1);

  srvc = dir->service;
  g_return_val_if_fail(srvc != NULL, -1);

  chan = srvc->channel;
  g_return_val_if_fail(chan != NULL, -1);

  b = mwPutBuffer_new();
  guint32_put(b, map_request(dir));
  guint32_put(b, dir->id);
  guint16_put(b, 0x0061);      /* some magic? */
  guint32_put(b, 0x00000008);  /* seek results */
  mwString_put(b, query);

  mwPutBuffer_finalize(&o, b);
  ret = mwChannel_send(chan, action_search, &o);
  mwOpaque_clear(&o);

  return ret;
}


static int dir_close(struct mwDirectory *dir) {
  struct mwServiceDirectory *srvc;
  struct mwChannel *chan;
  struct mwPutBuffer *b;
  struct mwOpaque o;
  int ret;

  g_return_val_if_fail(dir != NULL, -1);

  srvc = dir->service;
  g_return_val_if_fail(srvc != NULL, -1);

  chan = srvc->channel;
  g_return_val_if_fail(chan != NULL, -1);

  b = mwPutBuffer_new();
  guint32_put(b, next_request_id(dir->service));
  guint32_put(b, dir->id);

  mwPutBuffer_finalize(&o, b);
  ret = mwChannel_send(chan, action_close, &o);
  mwOpaque_clear(&o);

  return ret;
}


int mwDirectory_destroy(struct mwDirectory *dir) {
  int ret = 0;

  g_return_val_if_fail(dir != NULL, -1);

  if(MW_DIRECTORY_IS_OPEN(dir) || MW_DIRECTORY_IS_PENDING(dir)) {
    ret = dir_close(dir);
  }
  dir_remove(dir);

  return ret;
}

