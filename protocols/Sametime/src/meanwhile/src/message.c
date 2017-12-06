
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

#include "mw_debug.h"
#include "mw_message.h"


/* 7.1 Layering and message encapsulation */
/* 7.1.1 The Sametime Message Header */


static void mwMessageHead_put(struct mwPutBuffer *b, struct mwMessage *msg) {
  guint16_put(b, msg->type);
  guint16_put(b, msg->options);
  guint32_put(b, msg->channel);
  
  if(msg->options & mwMessageOption_HAS_ATTRIBS)
    mwOpaque_put(b, &msg->attribs);
}


static void mwMessageHead_get(struct mwGetBuffer *b, struct mwMessage *msg) {

  if(mwGetBuffer_error(b)) return;

  guint16_get(b, &msg->type);
  guint16_get(b, &msg->options);
  guint32_get(b, &msg->channel);

  if(msg->options & mwMessageOption_HAS_ATTRIBS)
    mwOpaque_get(b, &msg->attribs);
}


static void mwMessageHead_clone(struct mwMessage *to,
				struct mwMessage *from) {

  to->type = from->type;
  to->options = from->options;
  to->channel = from->channel;
  mwOpaque_clone(&to->attribs, &from->attribs);
}


static void mwMessageHead_clear(struct mwMessage *msg) {
  mwOpaque_clear(&msg->attribs);
}


/* 8.4 Messages */
/* 8.4.1 Basic Community Messages */
/* 8.4.1.1 Handshake */


static void HANDSHAKE_put(struct mwPutBuffer *b, struct mwMsgHandshake *msg) {
  guint16_put(b, msg->major);
  guint16_put(b, msg->minor);
  guint32_put(b, msg->head.channel);
  guint32_put(b, msg->srvrcalc_addr);
  guint16_put(b, msg->login_type);
  guint32_put(b, msg->loclcalc_addr);
  
  if(msg->major >= 0x001e && msg->minor >= 0x001d) {
    guint16_put(b, msg->unknown_a);
    guint32_put(b, msg->unknown_b);
    mwString_put(b, msg->local_host);
  }
}


static void HANDSHAKE_get(struct mwGetBuffer *b, struct mwMsgHandshake *msg) {
  if(mwGetBuffer_error(b)) return;

  guint16_get(b, &msg->major);
  guint16_get(b, &msg->minor);
  guint32_get(b, &msg->head.channel);
  guint32_get(b, &msg->srvrcalc_addr);
  guint16_get(b, &msg->login_type);
  guint32_get(b, &msg->loclcalc_addr);

  if(msg->major >= 0x001e && msg->minor >= 0x001d) {
    guint16_get(b, &msg->unknown_a);
    guint32_get(b, &msg->unknown_b);
    mwString_get(b, &msg->local_host);
  }
}


static void HANDSHAKE_clear(struct mwMsgHandshake *msg) {
  ; /* nothing to clean up */
}


/* 8.4.1.2 HandshakeAck */


static void HANDSHAKE_ACK_get(struct mwGetBuffer *b,
			      struct mwMsgHandshakeAck *msg) {

  if(mwGetBuffer_error(b)) return;

  guint16_get(b, &msg->major);
  guint16_get(b, &msg->minor);
  guint32_get(b, &msg->srvrcalc_addr);

  /** @todo: get a better handle on what versions support what parts
      of this message. eg: minor version 0x0018 doesn't send the
      following */
  if(msg->major >= 0x1e && msg->minor > 0x18) {
    guint32_get(b, &msg->magic);
    mwOpaque_get(b, &msg->data);
  }
}


static void HANDSHAKE_ACK_put(struct mwPutBuffer *b,
			      struct mwMsgHandshakeAck *msg) {

  guint16_put(b, msg->major);
  guint16_put(b, msg->minor);
  guint32_put(b, msg->srvrcalc_addr);

  if(msg->major >= 0x1e && msg->minor > 0x18) {
    guint32_put(b, msg->magic);
    mwOpaque_put(b, &msg->data);
  }
}


static void HANDSHAKE_ACK_clear(struct mwMsgHandshakeAck *msg) {
  mwOpaque_clear(&msg->data);
}


/* 8.4.1.3 Login */


static void LOGIN_put(struct mwPutBuffer *b, struct mwMsgLogin *msg) {
  guint16_put(b, msg->login_type);
  mwString_put(b, msg->name);

  /* ordering reversed from houri draft?? */
  mwOpaque_put(b, &msg->auth_data);
  guint16_put(b, msg->auth_type);

  guint16_put(b, 0x0000); /* unknown */
}


static void LOGIN_get(struct mwGetBuffer *b, struct mwMsgLogin *msg) {
  if(mwGetBuffer_error(b)) return;

  guint16_get(b, &msg->login_type);
  mwString_get(b, &msg->name);
  mwOpaque_get(b, &msg->auth_data);
  guint16_get(b, &msg->auth_type);
}


static void LOGIN_clear(struct mwMsgLogin *msg) {
  g_free(msg->name);  msg->name = NULL;
  mwOpaque_clear(&msg->auth_data);
}


/* 8.4.1.4 LoginAck */


static void LOGIN_ACK_get(struct mwGetBuffer *b, struct mwMsgLoginAck *msg) {
  guint16 junk;

  if(mwGetBuffer_error(b)) return;

  mwLoginInfo_get(b, &msg->login);
  guint16_get(b, &junk);
  mwPrivacyInfo_get(b, &msg->privacy);
  mwUserStatus_get(b, &msg->status);
}


static void LOGIN_ACK_clear(struct mwMsgLoginAck *msg) {
  mwLoginInfo_clear(&msg->login);
  mwPrivacyInfo_clear(&msg->privacy);
  mwUserStatus_clear(&msg->status);
}


/* 8.4.1.5 LoginCont */


static void LOGIN_CONTINUE_put(struct mwPutBuffer *b,
			       struct mwMsgLoginContinue *msg) {

  ; /* nothing but a message header */
}


static void LOGIN_CONTINUE_get(struct mwGetBuffer *b,
			       struct mwMsgLoginContinue *msg) {

  ; /* nothing but a message header */
}


static void LOGIN_CONTINUE_clear(struct mwMsgLoginContinue *msg) {
  ; /* this is a very simple message */
}


/* 8.4.1.6 AuthPassed */


static void LOGIN_REDIRECT_get(struct mwGetBuffer *b,
			       struct mwMsgLoginRedirect *msg) {

  if(mwGetBuffer_error(b)) return;
  mwString_get(b, &msg->host);
  mwString_get(b, &msg->server_id);  
}


static void LOGIN_REDIRECT_put(struct mwPutBuffer *b,
			       struct mwMsgLoginRedirect *msg) {
  mwString_put(b, msg->host);
  mwString_put(b, msg->server_id);
}


static void LOGIN_REDIRECT_clear(struct mwMsgLoginRedirect *msg) {
  g_free(msg->host);
  msg->host = NULL;

  g_free(msg->server_id);
  msg->server_id = NULL;
}


/* 8.4.1.7 CreateCnl */


static void enc_offer_put(struct mwPutBuffer *b, struct mwEncryptOffer *enc) {
  guint16_put(b, enc->mode);

  if(enc->items) {
    guint32 count;
    struct mwPutBuffer *p;
    struct mwOpaque o;
    GList *list;

    /* write the count, items, extra, and flag into a tmp buffer,
       render that buffer into an opaque, and write it into b */

    count = g_list_length(enc->items);
    p = mwPutBuffer_new();

    guint32_put(p, count);
    for(list = enc->items; list; list = list->next) {
      mwEncryptItem_put(p, list->data);
    }

    guint16_put(p, enc->extra);
    gboolean_put(p, enc->flag);

    mwPutBuffer_finalize(&o, p);
    mwOpaque_put(b, &o);
    mwOpaque_clear(&o);
  }
}


static void CHANNEL_CREATE_put(struct mwPutBuffer *b,
			       struct mwMsgChannelCreate *msg) {

  guint32_put(b, msg->reserved);
  guint32_put(b, msg->channel);
  mwIdBlock_put(b, &msg->target);
  guint32_put(b, msg->service);
  guint32_put(b, msg->proto_type);
  guint32_put(b, msg->proto_ver);
  guint32_put(b, msg->options);
  mwOpaque_put(b, &msg->addtl);
  gboolean_put(b, msg->creator_flag);

  if(msg->creator_flag)
    mwLoginInfo_put(b, &msg->creator);

  enc_offer_put(b, &msg->encrypt);

  guint32_put(b, 0x00);
  guint32_put(b, 0x00);
  guint16_put(b, 0x07);
}


static void enc_offer_get(struct mwGetBuffer *b,
			  struct mwEncryptOffer *enc) {
  guint32 skip;

  if(mwGetBuffer_error(b)) return;

  guint16_get(b, &enc->mode);
  guint32_get(b, &skip);

  if(skip >= 7) {
    guint32 count;

    guint32_get(b, &count);

    while(count-- && (! mwGetBuffer_error(b))) {
      struct mwEncryptItem *ei = g_new0(struct mwEncryptItem, 1);
      mwEncryptItem_get(b, ei);
      enc->items = g_list_append(enc->items, ei);
    }

    guint16_get(b, &enc->extra);
    gboolean_get(b, &enc->flag);
  }
}


static void CHANNEL_CREATE_get(struct mwGetBuffer *b,
			       struct mwMsgChannelCreate *msg) {

  if(mwGetBuffer_error(b)) return;

  guint32_get(b, &msg->reserved);
  guint32_get(b, &msg->channel);
  mwIdBlock_get(b, &msg->target);
  guint32_get(b, &msg->service);
  guint32_get(b, &msg->proto_type);
  guint32_get(b, &msg->proto_ver);
  guint32_get(b, &msg->options);
  mwOpaque_get(b, &msg->addtl);
  gboolean_get(b, &msg->creator_flag);
  
  if(msg->creator_flag)
    mwLoginInfo_get(b, &msg->creator);
  
  enc_offer_get(b, &msg->encrypt);
}


static void CHANNEL_CREATE_clear(struct mwMsgChannelCreate *msg) {
  GList *list;

  mwIdBlock_clear(&msg->target);
  mwOpaque_clear(&msg->addtl);
  mwLoginInfo_clear(&msg->creator);
  
  for(list = msg->encrypt.items; list; list = list->next) {
    mwEncryptItem_clear(list->data);
    g_free(list->data);
  }
  g_list_free(msg->encrypt.items);
}


/* 8.4.1.8 AcceptCnl */


static void enc_accept_put(struct mwPutBuffer *b,
			   struct mwEncryptAccept *enc) {

  guint16_put(b, enc->mode);

  if(enc->item) {
    struct mwPutBuffer *p;
    struct mwOpaque o;

    p = mwPutBuffer_new();

    mwEncryptItem_put(p, enc->item);
    guint16_put(p, enc->extra);
    gboolean_put(p, enc->flag);

    mwPutBuffer_finalize(&o, p);
    mwOpaque_put(b, &o);
    mwOpaque_clear(&o);
  }
}


static void CHANNEL_ACCEPT_put(struct mwPutBuffer *b,
			       struct mwMsgChannelAccept *msg) {
  
  guint32_put(b, msg->service);
  guint32_put(b, msg->proto_type);
  guint32_put(b, msg->proto_ver);
  mwOpaque_put(b, &msg->addtl);
  gboolean_put(b, msg->acceptor_flag);
  
  if(msg->acceptor_flag)
    mwLoginInfo_put(b, &msg->acceptor);
  
  enc_accept_put(b, &msg->encrypt);

  guint32_put(b, 0x00);
  guint32_put(b, 0x00);
  guint16_put(b, 0x07);
}


static void enc_accept_get(struct mwGetBuffer *b,
			   struct mwEncryptAccept *enc) {
  guint32 skip;

  if(mwGetBuffer_error(b)) return;

  guint16_get(b, &enc->mode);
  guint32_get(b, &skip);

  if(skip >= 6) {
    enc->item = g_new0(struct mwEncryptItem, 1);
    mwEncryptItem_get(b, enc->item);
  }

  if(skip >= 9) {
    guint16_get(b, &enc->extra);
    gboolean_get(b, &enc->flag);
  }
}


static void CHANNEL_ACCEPT_get(struct mwGetBuffer *b,
			       struct mwMsgChannelAccept *msg) {

  if(mwGetBuffer_error(b)) return;

  guint32_get(b, &msg->service);
  guint32_get(b, &msg->proto_type);
  guint32_get(b, &msg->proto_ver);
  mwOpaque_get(b, &msg->addtl);
  gboolean_get(b, &msg->acceptor_flag);

  if(msg->acceptor_flag)
    mwLoginInfo_get(b, &msg->acceptor);

  enc_accept_get(b, &msg->encrypt);
}


static void CHANNEL_ACCEPT_clear(struct mwMsgChannelAccept *msg) {
  mwOpaque_clear(&msg->addtl);
  mwLoginInfo_clear(&msg->acceptor);

  if(msg->encrypt.item) {
    mwEncryptItem_clear(msg->encrypt.item);
    g_free(msg->encrypt.item);
  }
}


/* 8.4.1.9 SendOnCnl */


static void CHANNEL_SEND_put(struct mwPutBuffer *b,
			     struct mwMsgChannelSend *msg) {

  guint16_put(b, msg->type);
  mwOpaque_put(b, &msg->data);
}


static void CHANNEL_SEND_get(struct mwGetBuffer *b,
			     struct mwMsgChannelSend *msg) {

  if(mwGetBuffer_error(b)) return;

  guint16_get(b, &msg->type);
  mwOpaque_get(b, &msg->data);
}


static void CHANNEL_SEND_clear(struct mwMsgChannelSend *msg) {
  mwOpaque_clear(&msg->data);
}


/* 8.4.1.10 DestroyCnl */


static void CHANNEL_DESTROY_put(struct mwPutBuffer *b,
				struct mwMsgChannelDestroy *msg) {
  guint32_put(b, msg->reason);
  mwOpaque_put(b, &msg->data);
}


static void CHANNEL_DESTROY_get(struct mwGetBuffer *b,
				struct mwMsgChannelDestroy *msg) {

  if(mwGetBuffer_error(b)) return;

  guint32_get(b, &msg->reason);
  mwOpaque_get(b, &msg->data);
}


static void CHANNEL_DESTROY_clear(struct mwMsgChannelDestroy *msg) {
  mwOpaque_clear(&msg->data);
}


/* 8.4.1.11 SetUserStatus */


static void SET_USER_STATUS_put(struct mwPutBuffer *b,
				struct mwMsgSetUserStatus *msg) {
  mwUserStatus_put(b, &msg->status);
}


static void SET_USER_STATUS_get(struct mwGetBuffer *b,
				struct mwMsgSetUserStatus *msg) {

  if(mwGetBuffer_error(b)) return;
  mwUserStatus_get(b, &msg->status);
}


static void SET_USER_STATUS_clear(struct mwMsgSetUserStatus *msg) {
  mwUserStatus_clear(&msg->status);
}


/* 8.4.1.12 SetPrivacyList */


static void SET_PRIVACY_LIST_put(struct mwPutBuffer *b,
				 struct mwMsgSetPrivacyList *msg) {
  mwPrivacyInfo_put(b, &msg->privacy);
}


static void SET_PRIVACY_LIST_get(struct mwGetBuffer *b,
				 struct mwMsgSetPrivacyList *msg) {

  if(mwGetBuffer_error(b)) return;
  mwPrivacyInfo_get(b, &msg->privacy);
}


static void SET_PRIVACY_LIST_clear(struct mwMsgSetPrivacyList *msg) {
  mwPrivacyInfo_clear(&msg->privacy);
}


/* Sense Service messages */


static void SENSE_SERVICE_put(struct mwPutBuffer *b,
			      struct mwMsgSenseService *msg) {
  guint32_put(b, msg->service);
}


static void SENSE_SERVICE_get(struct mwGetBuffer *b,
			      struct mwMsgSenseService *msg) {

  if(mwGetBuffer_error(b)) return;
  guint32_get(b, &msg->service);
}


static void SENSE_SERVICE_clear(struct mwMsgSenseService *msg) {
  ;
}


/* Admin messages */


static void ADMIN_get(struct mwGetBuffer *b, struct mwMsgAdmin *msg) {
  mwString_get(b, &msg->text);
}


static void ADMIN_clear(struct mwMsgAdmin *msg) {
  g_free(msg->text);
  msg->text = NULL;
}


/* Announcement messages */


static void ANNOUNCE_get(struct mwGetBuffer *b, struct mwMsgAnnounce *msg) {
  struct mwOpaque o = { 0, 0 };
  struct mwGetBuffer *gb;
  guint32 count;

  gboolean_get(b, &msg->sender_present);
  if(msg->sender_present)
    mwLoginInfo_get(b, &msg->sender);
  guint16_get(b, &msg->unknown_a);
  
  mwOpaque_get(b, &o);
  gb = mwGetBuffer_wrap(&o);

  gboolean_get(gb, &msg->may_reply);
  mwString_get(gb, &msg->text);

  mwGetBuffer_free(gb);
  mwOpaque_clear(&o);

  guint32_get(b, &count);
  while(count--) {
    char *r = NULL;
    mwString_get(b, &r);
    msg->recipients = g_list_prepend(msg->recipients, r);
  }
}


static void ANNOUNCE_put(struct mwPutBuffer *b, struct mwMsgAnnounce *msg) {
  struct mwOpaque o = { 0, 0 };
  struct mwPutBuffer *pb;
  GList *l;
  
  gboolean_put(b, msg->sender_present);
  if(msg->sender_present)
    mwLoginInfo_put(b, &msg->sender);
  guint16_put(b, msg->unknown_a);

  pb = mwPutBuffer_new();
  
  gboolean_put(pb, msg->may_reply);
  mwString_put(pb, msg->text);

  mwPutBuffer_finalize(&o, pb);
  mwOpaque_put(b, &o);
  mwOpaque_clear(&o);

  guint32_put(b, g_list_length(msg->recipients));
  for(l = msg->recipients; l; l = l->next) {
    mwString_put(b, l->data);
  }
}


static void ANNOUNCE_clear(struct mwMsgAnnounce *msg) {
  mwLoginInfo_clear(&msg->sender);

  g_free(msg->text);
  msg->text = NULL;
  
  while(msg->recipients) {
    g_free(msg->recipients->data);
    msg->recipients = g_list_delete_link(msg->recipients, msg->recipients);
  }
}


/* general functions */


#define CASE(v, t) \
case mwMessage_ ## v: \
  msg = (struct mwMessage *) g_new0(struct t, 1); \
  msg->type = type; \
  break;


struct mwMessage *mwMessage_new(enum mwMessageType type) {
  struct mwMessage *msg = NULL;
  
  switch(type) {
    CASE(HANDSHAKE, mwMsgHandshake);
    CASE(HANDSHAKE_ACK, mwMsgHandshakeAck);
    CASE(LOGIN, mwMsgLogin);
    CASE(LOGIN_REDIRECT, mwMsgLoginRedirect);
    CASE(LOGIN_CONTINUE, mwMsgLoginContinue);
    CASE(LOGIN_ACK, mwMsgLoginAck);
    CASE(CHANNEL_CREATE, mwMsgChannelCreate);
    CASE(CHANNEL_DESTROY, mwMsgChannelDestroy);
    CASE(CHANNEL_SEND, mwMsgChannelSend);
    CASE(CHANNEL_ACCEPT, mwMsgChannelAccept);
    CASE(SET_USER_STATUS, mwMsgSetUserStatus);
    CASE(SET_PRIVACY_LIST, mwMsgSetPrivacyList);
    CASE(SENSE_SERVICE, mwMsgSenseService);
    CASE(ADMIN, mwMsgAdmin);
    CASE(ANNOUNCE, mwMsgAnnounce);
    
  default:
    g_warning("unknown message type 0x%02x\n", type);
  }
  
  return msg;
}


#undef CASE


/* each type needs to be passed to a specially named _get functions,
   and cast to a specific subclass of mwMessage. */
#define CASE(v, t) \
case mwMessage_ ## v: \
  msg = (struct mwMessage *) g_new0(struct t, 1); \
  mwMessageHead_clone(msg, &head); \
  v ## _get(b, (struct t *) msg); \
  break;


struct mwMessage *mwMessage_get(struct mwGetBuffer *b) {
  struct mwMessage *msg = NULL;
  struct mwMessage head;
  
  g_return_val_if_fail(b != NULL, NULL);

  head.attribs.len = 0;
  head.attribs.data = NULL;

  /* attempt to read the header first */
  mwMessageHead_get(b, &head);

  if(mwGetBuffer_error(b)) {
    mwMessageHead_clear(&head);
    g_warning("problem parsing message head from buffer");
    return NULL;
  }

  /* load the rest of the message depending on the header type */
  switch(head.type) {
    CASE(HANDSHAKE, mwMsgHandshake);
    CASE(HANDSHAKE_ACK, mwMsgHandshakeAck);
    CASE(LOGIN, mwMsgLogin);
    CASE(LOGIN_REDIRECT, mwMsgLoginRedirect);
    CASE(LOGIN_CONTINUE, mwMsgLoginContinue);
    CASE(LOGIN_ACK, mwMsgLoginAck);
    CASE(CHANNEL_CREATE, mwMsgChannelCreate);
    CASE(CHANNEL_DESTROY, mwMsgChannelDestroy);
    CASE(CHANNEL_SEND, mwMsgChannelSend);
    CASE(CHANNEL_ACCEPT, mwMsgChannelAccept);
    CASE(SET_USER_STATUS, mwMsgSetUserStatus);
    CASE(SET_PRIVACY_LIST, mwMsgSetPrivacyList);
    CASE(SENSE_SERVICE, mwMsgSenseService);
    CASE(ADMIN, mwMsgAdmin);
    CASE(ANNOUNCE, mwMsgAnnounce);

  default:
    g_warning("unknown message type 0x%02x, no parse handler", head.type);
  }

  if(mwGetBuffer_error(b)) {
    g_warning("problem parsing message type 0x%02x, not enough data",
	      head.type);
  }

  mwMessageHead_clear(&head);
  
  return msg;
}


#undef CASE


#define CASE(v, t) \
case mwMessage_ ## v: \
  v ## _put(b, (struct t *) msg); \
  break;


void mwMessage_put(struct mwPutBuffer *b, struct mwMessage *msg) {

  g_return_if_fail(b != NULL);
  g_return_if_fail(msg != NULL);

  mwMessageHead_put(b, msg);

  switch(msg->type) {
    CASE(HANDSHAKE, mwMsgHandshake);
    CASE(HANDSHAKE_ACK, mwMsgHandshakeAck);
    CASE(LOGIN, mwMsgLogin);
    CASE(LOGIN_REDIRECT, mwMsgLoginRedirect);
    CASE(LOGIN_CONTINUE, mwMsgLoginContinue);
    CASE(CHANNEL_CREATE, mwMsgChannelCreate);
    CASE(CHANNEL_DESTROY, mwMsgChannelDestroy);
    CASE(CHANNEL_SEND, mwMsgChannelSend);
    CASE(CHANNEL_ACCEPT, mwMsgChannelAccept);
    CASE(SET_USER_STATUS, mwMsgSetUserStatus);
    CASE(SET_PRIVACY_LIST, mwMsgSetPrivacyList);
    CASE(SENSE_SERVICE, mwMsgSenseService);
    CASE(ANNOUNCE, mwMsgAnnounce);
    
  default:
    ; /* hrm. */
  }
}


#undef CASE


#define CASE(v, t) \
case mwMessage_ ## v: \
  v ## _clear((struct t *) msg); \
  break;


void mwMessage_free(struct mwMessage *msg) {
  if(! msg) return;

  mwMessageHead_clear(msg);

  switch(msg->type) {
    CASE(HANDSHAKE, mwMsgHandshake);
    CASE(HANDSHAKE_ACK, mwMsgHandshakeAck);
    CASE(LOGIN, mwMsgLogin);
    CASE(LOGIN_REDIRECT, mwMsgLoginRedirect);
    CASE(LOGIN_CONTINUE, mwMsgLoginContinue);
    CASE(LOGIN_ACK, mwMsgLoginAck);
    CASE(CHANNEL_CREATE, mwMsgChannelCreate);
    CASE(CHANNEL_DESTROY, mwMsgChannelDestroy);
    CASE(CHANNEL_SEND, mwMsgChannelSend);
    CASE(CHANNEL_ACCEPT, mwMsgChannelAccept);
    CASE(SET_USER_STATUS, mwMsgSetUserStatus);
    CASE(SET_PRIVACY_LIST, mwMsgSetPrivacyList);
    CASE(SENSE_SERVICE, mwMsgSenseService);
    CASE(ADMIN, mwMsgAdmin);
    CASE(ANNOUNCE, mwMsgAnnounce);
    
  default:
    ; /* hrm. */
  }

  g_free(msg);
}


#undef CASE


