
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

#include "mw_common.h"


/** @todo the *_get functions should make sure to clear their
    structures in the event of failure, to prevent memory leaks */


#define MW16_PUT(b, val) \
  *(b)++ = ((val) >> 0x08) & 0xff; \
  *(b)++ = (val) & 0xff;


#define MW16_GET(b, val) \
  val = (*(b)++ & 0xff) << 8; \
  val = val | (*(b)++ & 0xff);


#define MW32_PUT(b, val) \
  *(b)++ = ((val) >> 0x18) & 0xff; \
  *(b)++ = ((val) >> 0x10) & 0xff; \
  *(b)++ = ((val) >> 0x08) & 0xff; \
  *(b)++ = (val) & 0xff;


#define MW32_GET(b, val) \
  val = (*(b)++ & 0xff) << 0x18; \
  val = val | (*(b)++ & 0xff) << 0x10; \
  val = val | (*(b)++ & 0xff) << 0x08; \
  val = val | (*(b)++ & 0xff);


struct mwPutBuffer {
  guchar *buf;  /**< head of buffer */
  gsize len;    /**< length of buffer */

  guchar *ptr;  /**< offset to first unused byte */
  gsize rem;    /**< count of unused bytes remaining */
};


struct mwGetBuffer {
  guchar *buf;  /**< head of buffer */
  gsize len;    /**< length of buffer */

  guchar *ptr;  /**< offset to first unused byte */
  gsize rem;    /**< count of unused bytes remaining */

  gboolean wrap;   /**< TRUE to indicate buf shouldn't be freed */
  gboolean error;  /**< TRUE to indicate an error */
};


#define BUFFER_USED(buffer) \
  ((buffer)->len - (buffer)->rem)


/** ensure that there's at least enough space remaining in the put
    buffer to fit needed. */
static void ensure_buffer(struct mwPutBuffer *b, gsize needed) {
  if(b->rem < needed) {
    gsize len = b->len, use = BUFFER_USED(b);
    guchar *buf;

    /* newly created buffers are empty until written to, and then they
       have 1024 available */
    if(! len) len = 1024;

    /* double len until it's large enough to fit needed */
    while( (len - use) < needed ) len = len << 1;

    /* create the new buffer. if there was anything in the old buffer,
       copy it into the new buffer and free the old copy */
    buf = g_malloc(len);
    if(b->buf) {
      memcpy(buf, b->buf, use);
      g_free(b->buf);
    }

    /* put the new buffer into b */
    b->buf = buf;
    b->len = len;
    b->ptr = buf + use;
    b->rem = len - use;
  }
}


/** determine if there are at least needed bytes available in the
    buffer. sets the error flag if there's not at least needed bytes
    left in the buffer

    @returns true if there's enough data, false if not */
static gboolean check_buffer(struct mwGetBuffer *b, gsize needed) {
  if(! b->error)  b->error = (b->rem < needed);
  return ! b->error;
}


struct mwPutBuffer *mwPutBuffer_new() {
  return g_new0(struct mwPutBuffer, 1);
}


void mwPutBuffer_write(struct mwPutBuffer *b, gpointer data, gsize len) {
  g_return_if_fail(b != NULL);
  g_return_if_fail(data != NULL);

  if(! len) return;

  ensure_buffer(b, len);
  memcpy(b->ptr, data, len);
  b->ptr += len;
  b->rem -= len;
}


void mwPutBuffer_free(struct mwPutBuffer *b) {
  if(! b) return;
  g_free(b->buf);
  g_free(b);
}


void mwPutBuffer_finalize(struct mwOpaque *to, struct mwPutBuffer *from) {
  g_return_if_fail(to != NULL);
  g_return_if_fail(from != NULL);

  to->len = BUFFER_USED(from);
  to->data = from->buf;

  g_free(from);
}


struct mwGetBuffer *mwGetBuffer_new(struct mwOpaque *o) {
  struct mwGetBuffer *b = g_new0(struct mwGetBuffer, 1);

  if(o && o->len) {
    b->buf = b->ptr = g_memdup(o->data, o->len);
    b->len = b->rem = o->len;
  }

  return b;
}


struct mwGetBuffer *mwGetBuffer_wrap(const struct mwOpaque *o) {
  struct mwGetBuffer *b = g_new0(struct mwGetBuffer, 1);

  if(o && o->len) {
    b->buf = b->ptr = o->data;
    b->len = b->rem = o->len;
  }
  b->wrap = TRUE;

  return b;
}


gsize mwGetBuffer_read(struct mwGetBuffer *b, gpointer data, gsize len) {
  g_return_val_if_fail(b != NULL, 0);
  g_return_val_if_fail(data != NULL, 0);

  if(b->error) return 0;
  if(! len) return 0;

  if(b->rem < len)
    len = b->rem;

  memcpy(data, b->ptr, len);
  b->ptr += len;
  b->rem -= len;

  return len;
}


gsize mwGetBuffer_advance(struct mwGetBuffer *b, gsize len) {
  g_return_val_if_fail(b != NULL, 0);

  if(b->error) return 0;
  if(! len) return 0;

  if(b->rem < len)
    len = b->rem;

  b->ptr += len;
  b->rem -= len;

  return len;
}


void mwGetBuffer_reset(struct mwGetBuffer *b) {
  g_return_if_fail(b != NULL);

  b->rem = b->len;
  b->ptr = b->buf;
  b->error = FALSE;
}


gsize mwGetBuffer_remaining(struct mwGetBuffer *b) {
  g_return_val_if_fail(b != NULL, 0);
  return b->rem;
}


gboolean mwGetBuffer_error(struct mwGetBuffer *b) {
  g_return_val_if_fail(b != NULL, TRUE);
  return b->error;
}


void mwGetBuffer_free(struct mwGetBuffer *b) {
  if(! b) return;
  if(! b->wrap) g_free(b->buf);
  g_free(b);
}


#define guint16_buflen()  2


void guint16_put(struct mwPutBuffer *b, guint16 val) {
  g_return_if_fail(b != NULL);

  ensure_buffer(b, guint16_buflen());
  MW16_PUT(b->ptr, val);
  b->rem -= guint16_buflen();
}


void guint16_get(struct mwGetBuffer *b, guint16 *val) {
  g_return_if_fail(b != NULL);

  if(b->error) return;
  g_return_if_fail(check_buffer(b, guint16_buflen()));

  MW16_GET(b->ptr, *val);
  b->rem -= guint16_buflen();
}


guint16 guint16_peek(struct mwGetBuffer *b) {
  guchar *buf = b->buf;
  guint16 r = 0;
  
  if(b->rem >= guint16_buflen())
    MW16_GET(buf, r);

  return r;
}


#define guint32_buflen()  4


void guint32_put(struct mwPutBuffer *b, guint32 val) {
  g_return_if_fail(b != NULL);

  ensure_buffer(b, guint32_buflen());
  MW32_PUT(b->ptr, val);
  b->rem -= guint32_buflen();
}


void guint32_get(struct mwGetBuffer *b, guint32 *val) {
  g_return_if_fail(b != NULL);

  if(b->error) return;
  g_return_if_fail(check_buffer(b, guint32_buflen()));

  MW32_GET(b->ptr, *val);
  b->rem -= guint32_buflen();
}


guint32 guint32_peek(struct mwGetBuffer *b) {
  guchar *buf = b->buf;
  guint32 r = 0;

  if(b->rem >= guint32_buflen())
    MW32_GET(buf, r);

  return r;
}


#define gboolean_buflen()  1


void gboolean_put(struct mwPutBuffer *b, gboolean val) {
  g_return_if_fail(b != NULL);

  ensure_buffer(b, gboolean_buflen());
  *(b->ptr) = !! val;
  b->ptr++;
  b->rem--;
}


void gboolean_get(struct mwGetBuffer *b, gboolean *val) {
  g_return_if_fail(b != NULL);

  if(b->error) return;
  g_return_if_fail(check_buffer(b, gboolean_buflen()));

  *val = !! *(b->ptr);
  b->ptr++;
  b->rem--;
}


gboolean gboolean_peek(struct mwGetBuffer *b) {
  gboolean v = FALSE;

  if(b->rem >= gboolean_buflen())
    v = !! *(b->ptr);

  return v;
}


static gboolean mw_streq(const char *a, const char *b) {
  return (a == b) || (a && b && !strcmp(a, b));
}


void mwString_put(struct mwPutBuffer *b, const char *val) {
  gsize len = 0;

  g_return_if_fail(b != NULL);

  if(val) len = strlen(val);

  guint16_put(b, (guint16) len);

  if(len) {
    ensure_buffer(b, len);
    memcpy(b->ptr, val, len);
    b->ptr += len;
    b->rem -= len;
  }
}


void mwString_get(struct mwGetBuffer *b, char **val) {
  guint16 len = 0;

  g_return_if_fail(b != NULL);
  g_return_if_fail(val != NULL);

  *val = NULL;

  if(b->error) return;
  guint16_get(b, &len);

  g_return_if_fail(check_buffer(b, (gsize) len));

  if(len) {
    *val = g_malloc0(len + 1);
    memcpy(*val, b->ptr, len);
    b->ptr += len;
    b->rem -= len;
  }
}


void mwOpaque_put(struct mwPutBuffer *b, const struct mwOpaque *o) {
  gsize len;

  g_return_if_fail(b != NULL);

  if(! o) {
    guint32_put(b, 0x00);
    return;
  }

  len = o->len;
  if(len)
    g_return_if_fail(o->data != NULL);
  
  guint32_put(b, (guint32) len);

  if(len) {
    ensure_buffer(b, len);
    memcpy(b->ptr, o->data, len);
    b->ptr += len;
    b->rem -= len;
  }
}


void mwOpaque_get(struct mwGetBuffer *b, struct mwOpaque *o) {
  guint32 tmp = 0;

  g_return_if_fail(b != NULL);
  g_return_if_fail(o != NULL);

  o->len = 0;
  o->data = NULL;
  
  if(b->error) return;
  guint32_get(b, &tmp);

  g_return_if_fail(check_buffer(b, (gsize) tmp));

  o->len = (gsize) tmp;
  if(tmp > 0) {
    o->data = g_memdup(b->ptr, tmp);
    b->ptr += tmp;
    b->rem -= tmp;
  }
}


void mwOpaque_clear(struct mwOpaque *o) {
  if(! o) return;
  g_free(o->data);
  o->data = NULL;
  o->len = 0;
}


void mwOpaque_free(struct mwOpaque *o) {
  if(! o) return;
  g_free(o->data);
  g_free(o);
}


void mwOpaque_clone(struct mwOpaque *to, const struct mwOpaque *from) {
  g_return_if_fail(to != NULL);

  to->len = 0;
  to->data = NULL;

  if(from) {
    to->len = from->len;
    if(to->len)
      to->data = g_memdup(from->data, to->len);
  }
}


/* 8.2 Common Structures */
/* 8.2.1 Login Info block */


void mwLoginInfo_put(struct mwPutBuffer *b, const struct mwLoginInfo *login) {
  g_return_if_fail(b != NULL);
  g_return_if_fail(login != NULL);

  mwString_put(b, login->login_id);
  guint16_put(b, login->type);
  mwString_put(b, login->user_id);
  mwString_put(b, login->user_name);
  mwString_put(b, login->community);
  gboolean_put(b, login->full);

  if(login->full) {
    mwString_put(b, login->desc);
    guint32_put(b, login->ip_addr);
    mwString_put(b, login->server_id);
  }
}


void mwLoginInfo_get(struct mwGetBuffer *b, struct mwLoginInfo *login) {
  g_return_if_fail(b != NULL);
  g_return_if_fail(login != NULL);

  if(b->error) return;

  mwString_get(b, &login->login_id);
  guint16_get(b, &login->type);
  mwString_get(b, &login->user_id);
  mwString_get(b, &login->user_name);
  mwString_get(b, &login->community);
  gboolean_get(b, &login->full);
  
  if(login->full) {
    mwString_get(b, &login->desc);
    guint32_get(b, &login->ip_addr);
    mwString_get(b, &login->server_id);
  }
}


void mwLoginInfo_clear(struct mwLoginInfo *login) {
  if(! login) return;

  g_free(login->login_id);
  g_free(login->user_id);
  g_free(login->user_name);
  g_free(login->community);
  g_free(login->desc);
  g_free(login->server_id);

  memset(login, 0x00, sizeof(struct mwLoginInfo));
}


void mwLoginInfo_clone(struct mwLoginInfo *to,
		       const struct mwLoginInfo *from) {

  g_return_if_fail(to != NULL);
  g_return_if_fail(from != NULL);

  to->login_id= g_strdup(from->login_id);
  to->type = from->type;
  to->user_id = g_strdup(from->user_id);
  to->user_name = g_strdup(from->user_name);
  to->community = g_strdup(from->community);

  if( (to->full = from->full) ) {
    to->desc = g_strdup(from->desc);
    to->ip_addr = from->ip_addr;
    to->server_id = g_strdup(from->server_id);
  }
}


/* 8.2.2 Private Info Block */


void mwUserItem_put(struct mwPutBuffer *b, const struct mwUserItem *user) {
  g_return_if_fail(b != NULL);
  g_return_if_fail(user != NULL);

  gboolean_put(b, user->full);
  mwString_put(b, user->id);
  mwString_put(b, user->community);
  
  if(user->full)
    mwString_put(b, user->name);
}


void mwUserItem_get(struct mwGetBuffer *b, struct mwUserItem *user) {
  g_return_if_fail(b != NULL);
  g_return_if_fail(user != NULL);

  if(b->error) return;

  gboolean_get(b, &user->full);
  mwString_get(b, &user->id);
  mwString_get(b, &user->community);

  if(user->full)
    mwString_get(b, &user->name);
}


void mwUserItem_clear(struct mwUserItem *user) {
  if(! user) return;

  g_free(user->id);
  g_free(user->community);
  g_free(user->name);

  memset(user, 0x00, sizeof(struct mwUserItem));
}


void mwUserItem_clone(struct mwUserItem *to,
		      const struct mwUserItem *from) {

  g_return_if_fail(to != NULL);
  g_return_if_fail(from != NULL);

  to->full = from->full;
  to->id = g_strdup(from->id);
  to->community = g_strdup(from->community);
  to->name = (to->full)? g_strdup(from->name): NULL;
}


void mwPrivacyInfo_put(struct mwPutBuffer *b,
		       const struct mwPrivacyInfo *info) {
  guint32 c;

  g_return_if_fail(b != NULL);
  g_return_if_fail(info != NULL);

  gboolean_put(b, info->deny);
  guint32_put(b, info->count);

  for(c = info->count; c--; ) mwUserItem_put(b, info->users + c);
}


void mwPrivacyInfo_get(struct mwGetBuffer *b, struct mwPrivacyInfo *info) {
  g_return_if_fail(b != NULL);
  g_return_if_fail(info != NULL);

  if(b->error) return;

  gboolean_get(b, &info->deny);
  guint32_get(b, &info->count);

  if(info->count) {
    guint32 c = info->count;
    info->users = g_new0(struct mwUserItem, c);
    while(c--) mwUserItem_get(b, info->users + c);
  }
}


void mwPrivacyInfo_clone(struct mwPrivacyInfo *to,
			 const struct mwPrivacyInfo *from) {

  guint32 c;

  g_return_if_fail(to != NULL);
  g_return_if_fail(from != NULL);

  to->deny = from->deny;
  c = to->count = from->count;

  to->users = g_new0(struct mwUserItem, c);
  while(c--) mwUserItem_clone(to->users+c, from->users+c);
}


void mwPrivacyInfo_clear(struct mwPrivacyInfo *info) {
  struct mwUserItem *u;
  guint32 c;

  g_return_if_fail(info != NULL);

  u = info->users;
  c = info->count;

  while(c--) mwUserItem_clear(u + c);
  g_free(u);

  info->count = 0;
  info->users = NULL;
}


/* 8.2.3 User Status Block */


void mwUserStatus_put(struct mwPutBuffer *b,
		      const struct mwUserStatus *stat) {

  g_return_if_fail(b != NULL);
  g_return_if_fail(stat != NULL);

  guint16_put(b, stat->status);
  guint32_put(b, stat->time);
  mwString_put(b, stat->desc);
}


void mwUserStatus_get(struct mwGetBuffer *b, struct mwUserStatus *stat) {
  g_return_if_fail(b != NULL);
  g_return_if_fail(stat != NULL);

  if(b->error) return;

  guint16_get(b, &stat->status);
  guint32_get(b, &stat->time);
  mwString_get(b, &stat->desc);

  /// Miranda NG adaptation - start - http://www.lilotux.net/~mikael/pub/meanwhile/status_timestamp_workaround.diff
  // Quick'n ugly hack for recent Sametime clients
  stat->time = 0;
  /// Miranda NG adaptation - end
}


void mwUserStatus_clear(struct mwUserStatus *stat) {
  if(! stat) return;
  g_free(stat->desc);
  memset(stat, 0x00, sizeof(struct mwUserStatus));
}


void mwUserStatus_clone(struct mwUserStatus *to,
			const struct mwUserStatus *from) {

  g_return_if_fail(to != NULL);
  g_return_if_fail(from != NULL);

  to->status = from->status;
  to->time = from->time;
  to->desc = g_strdup(from->desc);
}


/* 8.2.4 ID Block */


void mwIdBlock_put(struct mwPutBuffer *b, const struct mwIdBlock *id) {
  g_return_if_fail(b != NULL);
  g_return_if_fail(id != NULL);

  mwString_put(b, id->user);
  mwString_put(b, id->community);
}


void mwIdBlock_get(struct mwGetBuffer *b, struct mwIdBlock *id) {
  g_return_if_fail(b != NULL);
  g_return_if_fail(id != NULL);

  if(b->error) return;

  mwString_get(b, &id->user);
  mwString_get(b, &id->community);
}


void mwIdBlock_clear(struct mwIdBlock *id) {
  if(! id) return;

  g_free(id->user);
  id->user = NULL;

  g_free(id->community);
  id->community = NULL;
}


void mwIdBlock_clone(struct mwIdBlock *to, const struct mwIdBlock *from) {
  g_return_if_fail(to != NULL);
  g_return_if_fail(from != NULL);

  to->user = g_strdup(from->user);
  to->community = g_strdup(from->community);
}


guint mwIdBlock_hash(const struct mwIdBlock *idb) {
  return (idb)? g_str_hash(idb->user): 0;
}


gboolean mwIdBlock_equal(const struct mwIdBlock *a,
			 const struct mwIdBlock *b) {

  g_return_val_if_fail(a != NULL, FALSE);
  g_return_val_if_fail(b != NULL, FALSE);

  return ( mw_streq(a->user, b->user) &&
	   mw_streq(a->community, b->community) );
}


/* 8.2.5 Encryption Block */

/** @todo I think this can be put into cipher */

void mwEncryptItem_put(struct mwPutBuffer *b,
		       const struct mwEncryptItem *ei) {

  g_return_if_fail(b != NULL);
  g_return_if_fail(ei != NULL);
  
  guint16_put(b, ei->id);
  mwOpaque_put(b, &ei->info);

}


void mwEncryptItem_get(struct mwGetBuffer *b, struct mwEncryptItem *ei) {
  g_return_if_fail(b != NULL);
  g_return_if_fail(ei != NULL);

  if(b->error) return;

  guint16_get(b, &ei->id);
  mwOpaque_get(b, &ei->info);
}


void mwEncryptItem_clear(struct mwEncryptItem *ei) {
  if(! ei) return;
  ei->id = 0x0000;
  mwOpaque_clear(&ei->info);
}


void mwEncryptItem_free(struct mwEncryptItem *ei) {
  mwEncryptItem_clear(ei);
  g_free(ei);
}


/* 8.4.2.1 Awareness ID Block */


/** @todo move this into srvc_aware */

void mwAwareIdBlock_put(struct mwPutBuffer *b,
			const struct mwAwareIdBlock *idb) {

  g_return_if_fail(b != NULL);
  g_return_if_fail(idb != NULL);

  guint16_put(b, idb->type);
  mwString_put(b, idb->user);
  mwString_put(b, idb->community);
}


void mwAwareIdBlock_get(struct mwGetBuffer *b, struct mwAwareIdBlock *idb) {
  g_return_if_fail(b != NULL);
  g_return_if_fail(idb != NULL);

  if(b->error) return;

  guint16_get(b, &idb->type);
  mwString_get(b, &idb->user);
  mwString_get(b, &idb->community);
}


void mwAwareIdBlock_clone(struct mwAwareIdBlock *to,
			  const struct mwAwareIdBlock *from) {

  g_return_if_fail(to != NULL);
  g_return_if_fail(from != NULL);

  to->type = from->type;
  to->user = g_strdup(from->user);
  to->community = g_strdup(from->community);
}


void mwAwareIdBlock_clear(struct mwAwareIdBlock *idb) {
  if(! idb) return;
  g_free(idb->user);
  g_free(idb->community);
  memset(idb, 0x00, sizeof(struct mwAwareIdBlock));
}


guint mwAwareIdBlock_hash(const struct mwAwareIdBlock *a) {
  return (a)? g_str_hash(a->user): 0;
}


gboolean mwAwareIdBlock_equal(const struct mwAwareIdBlock *a,
			      const struct mwAwareIdBlock *b) {

  g_return_val_if_fail(a != NULL, FALSE);
  g_return_val_if_fail(b != NULL, FALSE);
  
  return ( (a->type == b->type) &&
	   mw_streq(a->user, b->user) &&
	   mw_streq(a->community, b->community) );
}


/* 8.4.2.4 Snapshot */

void mwAwareSnapshot_get(struct mwGetBuffer *b, struct mwAwareSnapshot *idb) {

  /// Miranda NG adaptation - start - http://www.lilotux.net/~mikael/pub/meanwhile/presence_fix_v2.diff
  //guint32 junk;
  //char *empty = NULL;
  guint32 end_of_block;
  /// Miranda NG adaptation - end

  g_return_if_fail(b != NULL);
  g_return_if_fail(idb != NULL);

  /// Miranda NG adaptation - start - http://www.lilotux.net/~mikael/pub/meanwhile/presence_fix_v2.diff
  //guint32_get(b, &junk);
  guint32_get(b, &end_of_block);
  /// Miranda NG adaptation - end
  mwAwareIdBlock_get(b, &idb->id);
  mwString_get(b, &idb->group);
  gboolean_get(b, &idb->online);

  /// Miranda NG adaptation - start - http://www.lilotux.net/~mikael/pub/meanwhile/presence_fix_v2.diff
  //g_free(empty);
  /// Miranda NG adaptation - end

  if(idb->online) {
    mwString_get(b, &idb->alt_id);
    mwUserStatus_get(b, &idb->status);
    mwString_get(b, &idb->name);
  }

  /// Miranda NG adaptation - start - http://www.lilotux.net/~mikael/pub/meanwhile/presence_fix_v2.diff
  if(b->ptr < b->buf + end_of_block) {
    mwGetBuffer_advance(b, b->buf + end_of_block - b->ptr);
  }
  /// Miranda NG adaptation - end
}


void mwAwareSnapshot_clone(struct mwAwareSnapshot *to,
			   const struct mwAwareSnapshot *from) {

  g_return_if_fail(to != NULL);
  g_return_if_fail(from != NULL);

  mwAwareIdBlock_clone(&to->id, &from->id);
  if( (to->online = from->online) ) {
    to->alt_id = g_strdup(from->alt_id);
    mwUserStatus_clone(&to->status, &from->status);
    to->name = g_strdup(from->name);
    to->group = g_strdup(from->group);
  }
}


void mwAwareSnapshot_clear(struct mwAwareSnapshot *idb) {
  if(! idb) return;
  mwAwareIdBlock_clear(&idb->id);
  mwUserStatus_clear(&idb->status);
  g_free(idb->alt_id);
  g_free(idb->name);
  g_free(idb->group);
  memset(idb, 0x00, sizeof(struct mwAwareSnapshot));
}

