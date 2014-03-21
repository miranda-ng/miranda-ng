
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


/** the hash table key for a service, for mwSession::services */
#define SERVICE_KEY(srvc) mwService_getType(srvc)

/** the hash table key for a cipher, for mwSession::ciphers */
#define CIPHER_KEY(ciph)  mwCipher_getType(ciph)


#define GPOINTER(val)  (GUINT_TO_POINTER((guint) (val)))
#define GUINT(val)     (GPOINTER_TO_UINT((val)))


struct mwSession {

  /** provides I/O and callback functions */
  struct mwSessionHandler *handler;

  enum mwSessionState state;  /**< session state */
  gpointer state_info;        /**< additional state info */

  /* input buffering for an incoming message */
  guchar *buf;  /**< buffer for incoming message data */
  gsize buf_len;       /**< length of buf */
  gsize buf_used;      /**< offset to last-used byte of buf */
  
  struct mwLoginInfo login;      /**< login information */
  struct mwUserStatus status;    /**< user status */
  struct mwPrivacyInfo privacy;  /**< privacy list */

  /** the collection of channels */
  struct mwChannelSet *channels;

  /** the collection of services, keyed to guint32 service id */
  GHashTable *services;

  /** the collection of ciphers, keyed to guint16 cipher type */
  GHashTable *ciphers;

  /** arbitrary key:value pairs */
  GHashTable *attributes;

  /** optional user data */
  struct mw_datum client_data;
};


static void property_set(struct mwSession *s, const char *key,
			 gpointer val, GDestroyNotify clean) {

  g_hash_table_insert(s->attributes, g_strdup(key),
		      mw_datum_new(val, clean));
}


static gpointer property_get(struct mwSession *s, const char *key) {
  struct mw_datum *p = g_hash_table_lookup(s->attributes, key);
  return p? p->data: NULL;
}


static void property_del(struct mwSession *s, const char *key) {
  g_hash_table_remove(s->attributes, key);
}


/**
   set up the default properties for a newly created session
*/
static void session_defaults(struct mwSession *s) {
  property_set(s, mwSession_CLIENT_VER_MAJOR,
	       GPOINTER(MW_PROTOCOL_VERSION_MAJOR), NULL);

  property_set(s, mwSession_CLIENT_VER_MINOR, 
	       GPOINTER(MW_PROTOCOL_VERSION_MINOR), NULL);

  property_set(s, mwSession_CLIENT_TYPE_ID,
	       GPOINTER(mwLogin_MEANWHILE), NULL);
}


struct mwSession *mwSession_new(struct mwSessionHandler *handler) {
  struct mwSession *s;

  g_return_val_if_fail(handler != NULL, NULL);

  /* consider io_write and io_close to be absolute necessities */
  g_return_val_if_fail(handler->io_write != NULL, NULL);
  g_return_val_if_fail(handler->io_close != NULL, NULL);

  s = g_new0(struct mwSession, 1);

  s->state = mwSession_STOPPED;

  s->handler = handler;

  s->channels = mwChannelSet_new(s);
  s->services = map_guint_new();
  s->ciphers = map_guint_new();

  s->attributes = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
					(GDestroyNotify) mw_datum_free);

  session_defaults(s);

  return s;
}


/** free and reset the session buffer */
static void session_buf_free(struct mwSession *s) {
  g_return_if_fail(s != NULL);

  g_free(s->buf);
  s->buf = NULL;
  s->buf_len = 0;
  s->buf_used = 0;
}


/** a polite string version of the session state enum */
static const char *state_str(enum mwSessionState state) {
  switch(state) {
  case mwSession_STARTING:      return "starting";
  case mwSession_HANDSHAKE:     return "handshake sent";
  case mwSession_HANDSHAKE_ACK: return "handshake acknowledged";
  case mwSession_LOGIN:         return "login sent";
  case mwSession_LOGIN_REDIR:   return "login redirected";
  case mwSession_LOGIN_CONT:    return "forcing login";
  case mwSession_LOGIN_ACK:     return "login acknowledged";
  case mwSession_STARTED:       return "started";
  case mwSession_STOPPING:      return "stopping";
  case mwSession_STOPPED:       return "stopped";

  case mwSession_UNKNOWN:       /* fall-through */
  default:                      return "UNKNOWN";
  }
}


void mwSession_free(struct mwSession *s) {
  struct mwSessionHandler *h;

  g_return_if_fail(s != NULL);

  if(! mwSession_isStopped(s)) {
    g_debug("session is not stopped (state: %s), proceeding with free",
	    state_str(s->state));
  }

  h = s->handler;
  if(h && h->clear) h->clear(s);
  s->handler = NULL;

  session_buf_free(s);

  mwChannelSet_free(s->channels);
  g_hash_table_destroy(s->services);
  g_hash_table_destroy(s->ciphers);
  g_hash_table_destroy(s->attributes);

  mwLoginInfo_clear(&s->login);
  mwUserStatus_clear(&s->status);
  mwPrivacyInfo_clear(&s->privacy);

  g_free(s);
}


/** write data to the session handler */
static int io_write(struct mwSession *s, const guchar *buf, gsize len) {
  g_return_val_if_fail(s != NULL, -1);
  g_return_val_if_fail(s->handler != NULL, -1);
  g_return_val_if_fail(s->handler->io_write != NULL, -1);

  return s->handler->io_write(s, buf, len);
}


/** close the session handler */
static void io_close(struct mwSession *s) {
  g_return_if_fail(s != NULL);
  g_return_if_fail(s->handler != NULL);
  g_return_if_fail(s->handler->io_close != NULL);

  s->handler->io_close(s);
}


static void state(struct mwSession *s, enum mwSessionState state,
		  gpointer info) {

  struct mwSessionHandler *sh;

  g_return_if_fail(s != NULL);
  g_return_if_fail(s->handler != NULL);

  if(mwSession_isState(s, state)) return;

  s->state = state;
  s->state_info = info;

  switch(state) {
  case mwSession_STOPPING:
  case mwSession_STOPPED:
    g_message("session state: %s (0x%08x)", state_str(state),
	      GPOINTER_TO_UINT(info));
    break;

  case mwSession_LOGIN_REDIR:
	/// Miranda NG adaptation - start - https://developer.pidgin.im/ticket/7563#comment:4
    //g_message("session state: %s (%s)", state_str(state), (char *)info);
    g_message("session state: %s (%s)", state_str(state), NSTR((char *)info));
	/// Miranda NG adaptation - end
    break;

  default:
    g_message("session state: %s", state_str(state));
  }

  sh = s->handler;
  if(sh && sh->on_stateChange)
    sh->on_stateChange(s, state, info);
}


void mwSession_start(struct mwSession *s) {
  struct mwMsgHandshake *msg;
  int ret;

  g_return_if_fail(s != NULL);
  g_return_if_fail(mwSession_isStopped(s));

  if(mwSession_isStarted(s) || mwSession_isStarting(s)) {
    g_debug("attempted to start session that is already started/starting");
    return;
  }
  
  state(s, mwSession_STARTING, 0);

  msg = (struct mwMsgHandshake *) mwMessage_new(mwMessage_HANDSHAKE);
  msg->major = GUINT(property_get(s, mwSession_CLIENT_VER_MAJOR));
  msg->minor = GUINT(property_get(s, mwSession_CLIENT_VER_MINOR));
  msg->login_type = GUINT(property_get(s, mwSession_CLIENT_TYPE_ID));

  msg->loclcalc_addr = GUINT(property_get(s, mwSession_CLIENT_IP));

  if(msg->major >= 0x001e && msg->minor >= 0x001d) {
    msg->unknown_a = 0x0100;
    msg->local_host = property_get(s, mwSession_CLIENT_HOST);
  }

  ret = mwSession_send(s, MW_MESSAGE(msg));
  mwMessage_free(MW_MESSAGE(msg));

  if(ret) {
    mwSession_stop(s, CONNECTION_BROKEN);
  } else {
    state(s, mwSession_HANDSHAKE, 0);
  }
}


void mwSession_stop(struct mwSession *s, guint32 reason) {
  GList *list, *l = NULL;
  struct mwMsgChannelDestroy *msg;

  g_return_if_fail(s != NULL);
  
  if(mwSession_isStopped(s) || mwSession_isStopping(s)) {
    g_debug("attempted to stop session that is already stopped/stopping");
    return;
  }

  state(s, mwSession_STOPPING, GUINT_TO_POINTER(reason));

  for(list = l = mwSession_getServices(s); l; l = l->next)
    mwService_stop(MW_SERVICE(l->data));
  g_list_free(list);

  msg = (struct mwMsgChannelDestroy *)
    mwMessage_new(mwMessage_CHANNEL_DESTROY);

  msg->head.channel = MW_MASTER_CHANNEL_ID;
  msg->reason = reason;

  /* don't care if this fails, we're closing the connection anyway */
  mwSession_send(s, MW_MESSAGE(msg));
  mwMessage_free(MW_MESSAGE(msg));

  session_buf_free(s);

  /* close the connection */
  io_close(s);

  state(s, mwSession_STOPPED, GUINT_TO_POINTER(reason));
}


/** compose authentication information into an opaque based on the
    password, encrypted via RC2/40 */
static void compose_auth_rc2_40(struct mwOpaque *auth, const char *pass) {
  guchar iv[8], key[5];
  struct mwOpaque a, b, z;
  struct mwPutBuffer *p;

  /* get an IV and a random five-byte key */
  mwIV_init(iv);
  mwKeyRandom(key, 5);

  /* the opaque with the key */
  a.len = 5;
  a.data = key;

  /* the opaque to receive the encrypted pass */
  b.len = 0;
  b.data = NULL;

  /* the plain-text pass dressed up as an opaque */
  z.len = strlen(pass);
  z.data = (guchar *) pass;

  /* the opaque with the encrypted pass */
  mwEncrypt(a.data, a.len, iv, &z, &b);

  /* an opaque containing the other two opaques */
  p = mwPutBuffer_new();
  mwOpaque_put(p, &a);
  mwOpaque_put(p, &b);
  mwPutBuffer_finalize(auth, p);

  /* this is the only one to clear, as the other uses a static buffer */
  mwOpaque_clear(&b);
}


static void compose_auth_rc2_128(struct mwOpaque *auth, const char *pass,
				 guint32 magic, struct mwOpaque *rkey) {

  guchar iv[8];
  struct mwOpaque a, b, c;
  struct mwPutBuffer *p;

  struct mwMpi *private, *public;
  struct mwMpi *remote;
  struct mwMpi *shared;

  private = mwMpi_new();
  public = mwMpi_new();
  remote = mwMpi_new();
  shared = mwMpi_new();

  mwIV_init(iv);

  mwMpi_randDHKeypair(private, public);
  mwMpi_import(remote, rkey);
  mwMpi_calculateDHShared(shared, remote, private);

  /* put the password in opaque a */
  p = mwPutBuffer_new();
  guint32_put(p, magic);
  mwString_put(p, pass);
  mwPutBuffer_finalize(&a, p);

  /* put the shared key in opaque b */
  mwMpi_export(shared, &b);

  /* encrypt the password (a) using the shared key (b), put the result
     in opaque c */
  mwEncrypt(b.data+(b.len-16), 16, iv, &a, &c);

  /* don't need the shared key anymore, re-use opaque (b) as the
     export of the public key */
  mwOpaque_clear(&b);
  mwMpi_export(public, &b);

  p = mwPutBuffer_new();
  guint16_put(p, 0x0001);  /* XXX: unknown */
  mwOpaque_put(p, &b);
  mwOpaque_put(p, &c);
  mwPutBuffer_finalize(auth, p);

  mwOpaque_clear(&a);
  mwOpaque_clear(&b);
  mwOpaque_clear(&c);

  mwMpi_free(private);
  mwMpi_free(public);
  mwMpi_free(remote);
  mwMpi_free(shared);
}


/** handle the receipt of a handshake_ack message by sending the login
    message */
static void HANDSHAKE_ACK_recv(struct mwSession *s,
			       struct mwMsgHandshakeAck *msg) {
  struct mwMsgLogin *log;
  int ret;
			       
  g_return_if_fail(s != NULL);
  g_return_if_fail(msg != NULL);
  g_return_if_fail(mwSession_isState(s, mwSession_HANDSHAKE) ||
		   mwSession_isState(s, mwSession_LOGIN_CONT));

  if(mwSession_isState(s, mwSession_LOGIN_CONT)) {
    /* this is a login continuation, don't re-send the login. We
       should receive a login ack in a moment */

    state(s, mwSession_HANDSHAKE_ACK, 0);
    state(s, mwSession_LOGIN, 0);
    return;

  } else {
    state(s, mwSession_HANDSHAKE_ACK, 0);
  }

  /* record the major/minor versions from the server */
  property_set(s, mwSession_SERVER_VER_MAJOR, GPOINTER(msg->major), NULL);
  property_set(s, mwSession_SERVER_VER_MINOR, GPOINTER(msg->minor), NULL);

  /* compose the login message */
  log = (struct mwMsgLogin *) mwMessage_new(mwMessage_LOGIN);
  log->login_type = GUINT(property_get(s, mwSession_CLIENT_TYPE_ID));
  log->name = g_strdup(property_get(s, mwSession_AUTH_USER_ID));

  /** @todo default to password for now. later use token optionally */
  {
    const char *pw;
    pw = property_get(s, mwSession_AUTH_PASSWORD);
   
    if(msg->data.len >= 64) {
      /* good login encryption */
      log->auth_type = mwAuthType_RC2_128;
      compose_auth_rc2_128(&log->auth_data, pw, msg->magic, &msg->data);

    } else {
      /* BAD login encryption */
      log->auth_type = mwAuthType_RC2_40;
      compose_auth_rc2_40(&log->auth_data, pw);
    }
  }
  
  /* send the login message */
  ret = mwSession_send(s, MW_MESSAGE(log));
  mwMessage_free(MW_MESSAGE(log));

  if(! ret) {
    /* sent login OK, set state appropriately */
    state(s, mwSession_LOGIN, 0);
  }
}


/** handle the receipt of a login_ack message. This completes the
    startup sequence for the session */
static void LOGIN_ACK_recv(struct mwSession *s,
			   struct mwMsgLoginAck *msg) {
  GList *ll, *l;

  g_return_if_fail(s != NULL);
  g_return_if_fail(msg != NULL);
  g_return_if_fail(mwSession_isState(s, mwSession_LOGIN));

  /* store the login information in the session */
  mwLoginInfo_clear(&s->login);
  mwLoginInfo_clone(&s->login, &msg->login);

  state(s, mwSession_LOGIN_ACK, 0);

  /* start up our services */
  for(ll = l = mwSession_getServices(s); l; l = l->next) {
    mwService_start(l->data);
  }
  g_list_free(ll);

  /* @todo any further startup stuff? */

  state(s, mwSession_STARTED, 0);
}


static void CHANNEL_CREATE_recv(struct mwSession *s,
				struct mwMsgChannelCreate *msg) {
  struct mwChannel *chan;
  chan = mwChannel_newIncoming(s->channels, msg->channel);

  /* hand off to channel */
  mwChannel_recvCreate(chan, msg);
}


static void CHANNEL_ACCEPT_recv(struct mwSession *s,
				struct mwMsgChannelAccept *msg) {
  struct mwChannel *chan;
  chan = mwChannel_find(s->channels, msg->head.channel);

  g_return_if_fail(chan != NULL);

  /* hand off to channel */
  mwChannel_recvAccept(chan, msg);
}


static void CHANNEL_DESTROY_recv(struct mwSession *s,
				 struct mwMsgChannelDestroy *msg) {

  /* the server can indicate that we should close the session by
     destroying the zero channel */
  if(msg->head.channel == MW_MASTER_CHANNEL_ID) {
    mwSession_stop(s, msg->reason);

  } else {
    struct mwChannel *chan;
    chan = mwChannel_find(s->channels, msg->head.channel);

    /* we don't have any such channel... so I guess we destroyed it.
       This is to remove a warning from timing errors when two clients
       both try to close a channel at about the same time. */
    if(! chan) return;
    
    /* hand off to channel */
    mwChannel_recvDestroy(chan, msg);
  }
}


static void CHANNEL_SEND_recv(struct mwSession *s,
			      struct mwMsgChannelSend *msg) {
  struct mwChannel *chan;
  chan = mwChannel_find(s->channels, msg->head.channel);

  /* if we don't have any such channel, we're certainly not going to
     accept data from it */
  if(! chan) return;

  /* hand off to channel */
  mwChannel_recv(chan, msg);
}


static void SET_PRIVACY_LIST_recv(struct mwSession *s,
				  struct mwMsgSetPrivacyList *msg) {
  struct mwSessionHandler *sh = s->handler;

  /// Miranda NG adaptation start - MSVC
  ///g_info("SET_PRIVACY_LIST");
  g_message("SET_PRIVACY_LIST");
  /// Miranda NG adaptation end

  mwPrivacyInfo_clear(&s->privacy);
  mwPrivacyInfo_clone(&s->privacy, &msg->privacy);

  if(sh && sh->on_setPrivacyInfo)
    sh->on_setPrivacyInfo(s);
}


static void SET_USER_STATUS_recv(struct mwSession *s,
				 struct mwMsgSetUserStatus *msg) {
  struct mwSessionHandler *sh = s->handler;

  mwUserStatus_clear(&s->status);
  mwUserStatus_clone(&s->status, &msg->status);

  if(sh && sh->on_setUserStatus)
    sh->on_setUserStatus(s);
}


static void SENSE_SERVICE_recv(struct mwSession *s,
			       struct mwMsgSenseService *msg) {
  struct mwService *srvc;

  srvc = mwSession_getService(s, msg->service);
  if(srvc) mwService_start(srvc);
}


static void ADMIN_recv(struct mwSession *s, struct mwMsgAdmin *msg) {
  struct mwSessionHandler *sh = s->handler;

  if(sh && sh->on_admin)
    sh->on_admin(s, msg->text);
}


static void ANNOUNCE_recv(struct mwSession *s, struct mwMsgAnnounce *msg) {
  struct mwSessionHandler *sh = s->handler;

  if(sh && sh->on_announce)
    sh->on_announce(s, &msg->sender, msg->may_reply, msg->text);
}


static void LOGIN_REDIRECT_recv(struct mwSession *s,
				struct mwMsgLoginRedirect *msg) {

  state(s, mwSession_LOGIN_REDIR, msg->host);
}


#define CASE(var, type) \
case mwMessage_ ## var: \
  var ## _recv(s, (struct type *) msg); \
  break;


static void session_process(struct mwSession *s,
			    const guchar *buf, gsize len) {

  /// Miranda NG adaptation start - MSVC
  /// struct mwOpaque o = { .len = len, .data = (guchar *) buf };
  struct mwOpaque o;
  /// Miranda NG adaptation end

  struct mwGetBuffer *b;
  struct mwMessage *msg;

  /// Miranda NG adaptation start - MSVC
  o.len = len;
  o.data = (guchar*) buf;
  /// Miranda NG adaptation end

  g_return_if_fail(s != NULL);
  g_return_if_fail(buf != NULL);

  /* ignore zero-length messages */
  if(len == 0) return;

  /* wrap up buf */
  b = mwGetBuffer_wrap(&o);

  /* attempt to parse the message. */
  msg = mwMessage_get(b);

  if(mwGetBuffer_error(b)) {
    mw_mailme_opaque(&o, "parsing of message failed");
  }

  mwGetBuffer_free(b);

  g_return_if_fail(msg != NULL);

  /* handle each of the appropriate incoming types of mwMessage */
  switch(msg->type) {
    CASE(HANDSHAKE_ACK, mwMsgHandshakeAck);
    CASE(LOGIN_REDIRECT, mwMsgLoginRedirect);
    CASE(LOGIN_ACK, mwMsgLoginAck);
    CASE(CHANNEL_CREATE, mwMsgChannelCreate);
    CASE(CHANNEL_DESTROY, mwMsgChannelDestroy);
    CASE(CHANNEL_SEND, mwMsgChannelSend);
    CASE(CHANNEL_ACCEPT, mwMsgChannelAccept);
    CASE(SET_PRIVACY_LIST, mwMsgSetPrivacyList);
    CASE(SET_USER_STATUS, mwMsgSetUserStatus);
    CASE(SENSE_SERVICE, mwMsgSenseService);
    CASE(ADMIN, mwMsgAdmin);
    CASE(ANNOUNCE, mwMsgAnnounce);
    
  default:
    g_warning("unknown message type 0x%04x, no handler", msg->type);
  }

  mwMessage_free(msg);
}


#undef CASE


#define ADVANCE(b, n, count) { b += count; n -= count; }


/* handle input to complete an existing buffer */
static gsize session_recv_cont(struct mwSession *s,
			       const guchar *b, gsize n) {

  /* determine how many bytes still required */
  gsize x = s->buf_len - s->buf_used;

  /* g_message(" session_recv_cont: session = %p, b = %p, n = %u",
	    s, b, n); */
  
  if(n < x) {
    /* not quite enough; still need some more */
    memcpy(s->buf+s->buf_used, b, n);
    s->buf_used += n;
    return 0;
    
  } else {
    /* enough to finish the buffer, at least */
    memcpy(s->buf+s->buf_used, b, x);
    ADVANCE(b, n, x);
    
    if(s->buf_len == 4) {
      /* if only the length bytes were being buffered, we'll now try
       to complete an actual message */

      struct mwOpaque o = { 4, s->buf };
      struct mwGetBuffer *gb = mwGetBuffer_wrap(&o);
      x = guint32_peek(gb);
      mwGetBuffer_free(gb);

      if(n < x) {
	/* there isn't enough to meet the demands of the length, so
	   we'll buffer it for next time */

	guchar *t;
	x += 4;
	t = (guchar *) g_malloc(x);
	memcpy(t, s->buf, 4);
	memcpy(t+4, b, n);
	
	session_buf_free(s);
	
	s->buf = t;
	s->buf_len = x;
	s->buf_used = n + 4;
	return 0;
	
      } else {
	/* there's enough (maybe more) for a full message. don't need
	   the old session buffer (which recall, was only the length
	   bytes) any more */
	
	session_buf_free(s);
	session_process(s, b, x);
	ADVANCE(b, n, x);
      }
      
    } else {
      /* process the now-complete buffer. remember to skip the first
	 four bytes, since they're just the size count */
      session_process(s, s->buf+4, s->buf_len-4);
      session_buf_free(s);
    }
  }

  return n;
}


/* handle input when there's nothing previously buffered */
static gsize session_recv_empty(struct mwSession *s,
				const guchar *b, gsize n) {

  struct mwOpaque o = { n, (guchar *) b };
  struct mwGetBuffer *gb;
  gsize x;

  if(n < 4) {
    /* uh oh. less than four bytes means we've got an incomplete
       length indicator. Have to buffer to get the rest of it. */
    s->buf = (guchar *) g_malloc0(4);
    memcpy(s->buf, b, n);
    s->buf_len = 4;
    s->buf_used = n;
    return 0;
  }
  
  /* peek at the length indicator. if it's a zero length message,
     don't process, just skip it */
  gb = mwGetBuffer_wrap(&o);
  x = guint32_peek(gb);
  mwGetBuffer_free(gb);
  if(! x) return n - 4;

  if(n < (x + 4)) {
    /* if the total amount of data isn't enough to cover the length
       bytes and the length indicated by those bytes, then we'll need
       to buffer. This is where the DOS mentioned below in
       session_recv takes place */

    x += 4;
    s->buf = (guchar *) g_malloc(x);
    memcpy(s->buf, b, n);
    s->buf_len = x;
    s->buf_used = n;
    return 0;
    
  } else {
    /* advance past length bytes */
    ADVANCE(b, n, 4);
    
    /* process and advance */
    session_process(s, b, x);
    ADVANCE(b, n, x);

    /* return left-over count */
    return n;
  }
}


static gsize session_recv(struct mwSession *s,
			  const guchar *b, gsize n) {

  /* This is messy and kind of confusing. I'd like to simplify it at
     some point, but the constraints are as follows:

      - buffer up to a single full message on the session buffer
      - buffer must contain the four length bytes
      - the four length bytes indicate how much we'll need to buffer
      - the four length bytes might not arrive all at once, so it's
        possible that we'll need to buffer to get them.
      - since our buffering includes the length bytes, we know we
        still have an incomplete length if the buffer length is only
        four. */
  
  /** @todo we should allow a compiled-in upper limit to message
     sizes, and just drop messages over that size. However, to do that
     we'd need to keep track of the size of a message and keep
     dropping bytes until we'd fulfilled the entire length. eg: if we
     receive a message size of 10MB, we need to pass up exactly 10MB
     before it's safe to start processing the rest as a new
     message. As it stands, a malicious packet from the server can run
     us out of memory by indicating it's going to send us some
     obscenely long message (even if it never actually sends it) */
  
  /* g_message(" session_recv: session = %p, b = %p, n = %u",
	    s, b, n); */
  
  if(s->buf_len == 0) {
    while(n && (*b & 0x80)) {
      /* keep-alive and series bytes are ignored */
      ADVANCE(b, n, 1);
    }
  }

  if(n == 0) {
    return 0;

  } else if(s->buf_len > 0) {
    return session_recv_cont(s, b, n);

  } else {
    return session_recv_empty(s, b, n);
  }
}


#undef ADVANCE


void mwSession_recv(struct mwSession *s, const guchar *buf, gsize n) {
  guchar *b = (guchar *) buf;
  gsize remain = 0;

  g_return_if_fail(s != NULL);

  while(n > 0) {
    remain = session_recv(s, b, n);
    b += (n - remain);
    n = remain;
  }
}


int mwSession_send(struct mwSession *s, struct mwMessage *msg) {
  struct mwPutBuffer *b;
  struct mwOpaque o;
  int ret = 0;

  g_return_val_if_fail(s != NULL, -1);

  /* writing nothing is easy */
  if(! msg) return 0;

  /* first we render the message into an opaque */
  b = mwPutBuffer_new();
  mwMessage_put(b, msg);
  mwPutBuffer_finalize(&o, b);

  /* then we render the opaque into... another opaque! */
  b = mwPutBuffer_new();
  mwOpaque_put(b, &o);
  mwOpaque_clear(&o);
  mwPutBuffer_finalize(&o, b);

  /* then we use that opaque's data and length to write to the socket */
  ret = io_write(s, o.data, o.len);
  mwOpaque_clear(&o);

  /* ensure we could actually write the message */
  if(! ret) {

    /* special case, as the server doesn't always respond to user
       status messages. Thus, we trigger the event when we send the
       messages as well as when we receive them */
    if(msg->type == mwMessage_SET_USER_STATUS) {
      SET_USER_STATUS_recv(s, (struct mwMsgSetUserStatus *) msg);
    }
  }

  return ret;
}


int mwSession_sendKeepalive(struct mwSession *s) {
  const guchar b = 0x80;

  g_return_val_if_fail(s != NULL, -1);
  return io_write(s, &b, 1);
}


int mwSession_forceLogin(struct mwSession *s) {
  struct mwMsgLoginContinue *msg;
  int ret;

  g_return_val_if_fail(s != NULL, -1);
  g_return_val_if_fail(mwSession_isState(s, mwSession_LOGIN_REDIR), -1);
  
  state(s, mwSession_LOGIN_CONT, 0x00);

  msg = (struct mwMsgLoginContinue *)
    mwMessage_new(mwMessage_LOGIN_CONTINUE);

  ret = mwSession_send(s, MW_MESSAGE(msg));
  mwMessage_free(MW_MESSAGE(msg));
  
  return ret;
}


int mwSession_sendAnnounce(struct mwSession *s, gboolean may_reply,
			   const char *text, const GList *recipients) {

  struct mwMsgAnnounce *msg;
  int ret;

  g_return_val_if_fail(s != NULL, -1);
  g_return_val_if_fail(mwSession_isStarted(s), -1);
  
  msg = (struct mwMsgAnnounce *) mwMessage_new(mwMessage_ANNOUNCE);

  msg->recipients = (GList *) recipients;
  msg->may_reply = may_reply;
  msg->text = g_strdup(text);

  ret = mwSession_send(s, MW_MESSAGE(msg));

  msg->recipients = NULL;  /* don't kill our recipients param */
  mwMessage_free(MW_MESSAGE(msg));

  return ret;
}


struct mwSessionHandler *mwSession_getHandler(struct mwSession *s) {
  g_return_val_if_fail(s != NULL, NULL);
  return s->handler;
}


struct mwLoginInfo *mwSession_getLoginInfo(struct mwSession *s) {
  g_return_val_if_fail(s != NULL, NULL);
  return &s->login;
}


int mwSession_setPrivacyInfo(struct mwSession *s,
			     struct mwPrivacyInfo *privacy) {

  struct mwMsgSetPrivacyList *msg;
  int ret;

  g_return_val_if_fail(s != NULL, -1);
  g_return_val_if_fail(privacy != NULL, -1);

  msg = (struct mwMsgSetPrivacyList *)
    mwMessage_new(mwMessage_SET_PRIVACY_LIST);

  mwPrivacyInfo_clone(&msg->privacy, privacy);

  ret = mwSession_send(s, MW_MESSAGE(msg));
  mwMessage_free(MW_MESSAGE(msg));

  return ret;
}


struct mwPrivacyInfo *mwSession_getPrivacyInfo(struct mwSession *s) {
  g_return_val_if_fail(s != NULL, NULL);
  return &s->privacy;
}


int mwSession_setUserStatus(struct mwSession *s,
			    struct mwUserStatus *stat) {

  struct mwMsgSetUserStatus *msg;
  int ret;

  g_return_val_if_fail(s != NULL, -1);
  g_return_val_if_fail(stat != NULL, -1);

  msg = (struct mwMsgSetUserStatus *)
    mwMessage_new(mwMessage_SET_USER_STATUS);

  mwUserStatus_clone(&msg->status, stat);

  ret = mwSession_send(s, MW_MESSAGE(msg));
  mwMessage_free(MW_MESSAGE(msg));

  return ret;
}


struct mwUserStatus *mwSession_getUserStatus(struct mwSession *s) {
  g_return_val_if_fail(s != NULL, NULL);
  return &s->status;
}


enum mwSessionState mwSession_getState(struct mwSession *s) {
  g_return_val_if_fail(s != NULL, mwSession_UNKNOWN);
  return s->state;
}


gpointer mwSession_getStateInfo(struct mwSession *s) {
  g_return_val_if_fail(s != NULL, 0);
  return s->state_info;
}


struct mwChannelSet *mwSession_getChannels(struct mwSession *session) {
  g_return_val_if_fail(session != NULL, NULL);
  return session->channels;
}


gboolean mwSession_addService(struct mwSession *s, struct mwService *srv) {
  g_return_val_if_fail(s != NULL, FALSE);
  g_return_val_if_fail(srv != NULL, FALSE);
  g_return_val_if_fail(s->services != NULL, FALSE);

  if(map_guint_lookup(s->services, SERVICE_KEY(srv))) {
    return FALSE;

  } else {
    map_guint_insert(s->services, SERVICE_KEY(srv), srv);
    if(mwSession_isState(s, mwSession_STARTED))
      mwSession_senseService(s, mwService_getType(srv));
    return TRUE;
  }
}


struct mwService *mwSession_getService(struct mwSession *s, guint32 srv) {
  g_return_val_if_fail(s != NULL, NULL);
  g_return_val_if_fail(s->services != NULL, NULL);

  return map_guint_lookup(s->services, srv);
}


struct mwService *mwSession_removeService(struct mwSession *s, guint32 srv) {
  struct mwService *svc;

  g_return_val_if_fail(s != NULL, NULL);
  g_return_val_if_fail(s->services != NULL, NULL);

  svc = map_guint_lookup(s->services, srv);
  if(svc) map_guint_remove(s->services, srv);
  return svc;
}


GList *mwSession_getServices(struct mwSession *s) {
  g_return_val_if_fail(s != NULL, NULL);
  g_return_val_if_fail(s->services != NULL, NULL);

  return map_collect_values(s->services);
}


void mwSession_senseService(struct mwSession *s, guint32 srvc) {
  struct mwMsgSenseService *msg;

  g_return_if_fail(s != NULL);
  g_return_if_fail(srvc != 0x00);
  g_return_if_fail(mwSession_isStarted(s));

  msg = (struct mwMsgSenseService *)
    mwMessage_new(mwMessage_SENSE_SERVICE);
  msg->service = srvc;

  mwSession_send(s, MW_MESSAGE(msg));
  mwMessage_free(MW_MESSAGE(msg));
}


gboolean mwSession_addCipher(struct mwSession *s, struct mwCipher *c) {
  g_return_val_if_fail(s != NULL, FALSE);
  g_return_val_if_fail(c != NULL, FALSE);
  g_return_val_if_fail(s->ciphers != NULL, FALSE);

  if(map_guint_lookup(s->ciphers, mwCipher_getType(c))) {
    g_message("cipher %s is already added, apparently",
	      NSTR(mwCipher_getName(c)));
    return FALSE;

  } else {
    g_message("adding cipher %s", NSTR(mwCipher_getName(c)));
    map_guint_insert(s->ciphers, mwCipher_getType(c), c);
    return TRUE;
  }
}


struct mwCipher *mwSession_getCipher(struct mwSession *s, guint16 c) {
  g_return_val_if_fail(s != NULL, NULL);
  g_return_val_if_fail(s->ciphers != NULL, NULL);

  return map_guint_lookup(s->ciphers, c);
}


struct mwCipher *mwSession_removeCipher(struct mwSession *s, guint16 c) {
  struct mwCipher *ciph;

  g_return_val_if_fail(s != NULL, NULL);
  g_return_val_if_fail(s->ciphers != NULL, NULL);

  ciph = map_guint_lookup(s->ciphers, c);
  if(ciph) map_guint_remove(s->ciphers, c);
  return ciph;
}


GList *mwSession_getCiphers(struct mwSession *s) {
  g_return_val_if_fail(s != NULL, NULL);
  g_return_val_if_fail(s->ciphers != NULL, NULL);

  return map_collect_values(s->ciphers);
}


void mwSession_setProperty(struct mwSession *s, const char *key,
			   gpointer val, GDestroyNotify clean) {

  g_return_if_fail(s != NULL);
  g_return_if_fail(s->attributes != NULL);
  g_return_if_fail(key != NULL);

  property_set(s, key, val, clean);
}


gpointer mwSession_getProperty(struct mwSession *s, const char *key) {
 
  g_return_val_if_fail(s != NULL, NULL);
  g_return_val_if_fail(s->attributes != NULL, NULL);
  g_return_val_if_fail(key != NULL, NULL);

  return property_get(s, key);
}


void mwSession_removeProperty(struct mwSession *s, const char *key) {
  g_return_if_fail(s != NULL);
  g_return_if_fail(s->attributes != NULL);
  g_return_if_fail(key != NULL);

  property_del(s, key);
}


void mwSession_setClientData(struct mwSession *session,
			     gpointer data, GDestroyNotify clear) {

  g_return_if_fail(session != NULL);
  mw_datum_set(&session->client_data, data, clear);
}


gpointer mwSession_getClientData(struct mwSession *session) {
  g_return_val_if_fail(session != NULL, NULL);
  return mw_datum_get(&session->client_data);
}


void mwSession_removeClientData(struct mwSession *session) {
  g_return_if_fail(session != NULL);
  mw_datum_clear(&session->client_data);
}

