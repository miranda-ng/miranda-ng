
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

#ifndef _MW_MESSAGE_H
#define _MW_MESSAGE_H


#include <glib.h>
#include "mw_common.h"


#ifdef __cplusplus
extern "C" {
#endif


/** Cast a pointer to a message subtype (eg, mwMsgHandshake,
    mwMsgAdmin) into a pointer to a mwMessage */
#define MW_MESSAGE(msg) (&msg->head)


/** Indicates the type of a message. */
enum mwMessageType {
  mwMessage_HANDSHAKE         = 0x0000,  /**< mwMsgHandshake */
  mwMessage_HANDSHAKE_ACK     = 0x8000,  /**< mwMsgHandshakeAck */
  mwMessage_LOGIN             = 0x0001,  /**< mwMsgLogin */
  mwMessage_LOGIN_ACK         = 0x8001,  /**< mwMsgLoginAck */
  mwMessage_LOGIN_REDIRECT    = 0x0018,  /**< mwMsgLoginRedirect */
  mwMessage_LOGIN_CONTINUE    = 0x0016,  /**< mwMsgLoginContinue */

  mwMessage_CHANNEL_CREATE    = 0x0002,  /**< mwMsgChannelCreate */
  mwMessage_CHANNEL_DESTROY   = 0x0003,  /**< mwMsgChannelDestroy */
  mwMessage_CHANNEL_SEND      = 0x0004,  /**< mwMsgChannelSend */
  mwMessage_CHANNEL_ACCEPT    = 0x0006,  /**< mwMsgChannelAccept */

  mwMessage_SET_USER_STATUS   = 0x0009,  /**< mwMsgSetUserStatus */
  mwMessage_SET_PRIVACY_LIST  = 0x000b,  /**< mwMsgSetPrivacyList */
  mwMessage_SENSE_SERVICE     = 0x0011,  /**< mwMsgSenseService */
  mwMessage_ADMIN             = 0x0019,  /**< mwMsgAdmin */
  mwMessage_ANNOUNCE          = 0x0022,  /**< mwMsgAnnounce */
};


enum mwMessageOption {
  mwMessageOption_ENCRYPT      = 0x4000,  /**< message data is encrypted */
  mwMessageOption_HAS_ATTRIBS  = 0x8000,  /**< message has attributes */
};


/** @see mwMessageOption */
#define MW_MESSAGE_HAS_OPTION(msg, opt) \
  ((msg)->options & (opt))


struct mwMessage {
  guint16 type;     /**< @see mwMessageType */
  guint16 options;  /**< @see mwMessageOption */
  guint32 channel;  /**< ID of channel message is intended for */
  struct mwOpaque attribs;  /**< optional message attributes */
};



/** Allocate and initialize a new message of the specified type */
struct mwMessage *mwMessage_new(enum mwMessageType type);


/** build a message from its representation */
struct mwMessage *mwMessage_get(struct mwGetBuffer *b);


void mwMessage_put(struct mwPutBuffer *b, struct mwMessage *msg);


void mwMessage_free(struct mwMessage *msg);


/* 8.4 Messages */
/* 8.4.1 Basic Community Messages */
/* 8.4.1.1 Handshake */

struct mwMsgHandshake {
  struct mwMessage head;
  guint16 major;          /**< client's major version number */
  guint16 minor;          /**< client's minor version number */
  guint32 srvrcalc_addr;  /**< 0.0.0.0 */
  guint16 login_type;     /**< @see mwLoginType */
  guint32 loclcalc_addr;  /**< local public IP */
  guint16 unknown_a;      /**< normally 0x0100 */
  guint32 unknown_b;      /**< normally 0x00000000 */
  char *local_host;       /**< name of client host */
};


/* 8.4.1.2 HandshakeAck */

struct mwMsgHandshakeAck {
  struct mwMessage head;
  guint16 major;          /**< server's major version number */
  guint16 minor;          /**< server's minor version number */
  guint32 srvrcalc_addr;  /**< server-calculated address */
  guint32 magic;          /**< four bytes of something */
  struct mwOpaque data;   /**< server's DH public key for auth */
};


/* 8.3.7 Authentication Types */

enum mwAuthType {
  mwAuthType_PLAIN    = 0x0000,
  mwAuthType_TOKEN    = 0x0001,
  mwAuthType_ENCRYPT  = 0x0002, /**< @todo remove for 1.0 */
  mwAuthType_RC2_40   = 0x0002,
  mwAuthType_RC2_128  = 0x0004,
};


/* 8.4.1.3 Login */

struct mwMsgLogin {
  struct mwMessage head;
  guint16 login_type;         /**< @see mwLoginType */
  char *name;                 /**< user identification */
  guint16 auth_type;          /**< @see mwAuthType */
  struct mwOpaque auth_data;  /**< authentication data */
};


/* 8.4.1.4 LoginAck */

struct mwMsgLoginAck {
  struct mwMessage head;
  struct mwLoginInfo login;
  struct mwPrivacyInfo privacy;
  struct mwUserStatus status;
};


/* 8.4.1.5 LoginCont */

struct mwMsgLoginContinue {
  struct mwMessage head;
};


/* 8.4.1.6 AuthPassed */

struct mwMsgLoginRedirect {
  struct mwMessage head;
  char *host;
  char *server_id;
};


/* 8.4.1.7 CreateCnl */

/** an offer of encryption items */
struct mwEncryptOffer {
  guint16 mode;   /**< encryption mode */
  GList *items;   /**< list of mwEncryptItem offered */
  guint16 extra;  /**< encryption mode again? */
  gboolean flag;  /**< unknown flag */
};


struct mwMsgChannelCreate {
  struct mwMessage head;
  guint32 reserved;         /**< unknown reserved data */
  guint32 channel;          /**< intended ID for new channel */
  struct mwIdBlock target;  /**< User ID. for service use */
  guint32 service;          /**< ID for the target service */
  guint32 proto_type;       /**< protocol type for the service */
  guint32 proto_ver;        /**< protocol version for the service */
  guint32 options;          /**< options */
  struct mwOpaque addtl;    /**< service-specific additional data */
  gboolean creator_flag;    /**< indicate presence of creator information */
  struct mwLoginInfo creator;
  struct mwEncryptOffer encrypt;
};


/* 8.4.1.8 AcceptCnl */

/** a selected encryption item from those offered */
struct mwEncryptAccept {
  guint16 mode;                /**< encryption mode */
  struct mwEncryptItem *item;  /**< chosen mwEncryptItem (optional) */
  guint16 extra;               /**< encryption mode again? */
  gboolean flag;               /**< unknown flag */
};


struct mwMsgChannelAccept {
  struct mwMessage head;
  guint32 service;         /**< ID for the channel's service */
  guint32 proto_type;      /**< protocol type for the service */
  guint32 proto_ver;       /**< protocol version for the service */
  struct mwOpaque addtl;   /**< service-specific additional data */
  gboolean acceptor_flag;  /**< indicate presence of acceptor information */
  struct mwLoginInfo acceptor;
  struct mwEncryptAccept encrypt;
};


/* 8.4.1.9 SendOnCnl */

struct mwMsgChannelSend {
  struct mwMessage head;

  /** message type. each service defines its own send types. Type IDs
      are only necessarily unique within a given service. */
  guint16 type;

  /** protocol data to be interpreted by the handling service */
  struct mwOpaque data;
};


/* 8.4.1.10 DestroyCnl */

struct mwMsgChannelDestroy {
  struct mwMessage head;
  guint32 reason;        /**< reason for closing the channel. */
  struct mwOpaque data;  /**< additional information */
};


/* 8.4.1.11 SetUserStatus */

struct mwMsgSetUserStatus {
  struct mwMessage head;
  struct mwUserStatus status;
};


/* 8.4.1.12 SetPrivacyList */

struct mwMsgSetPrivacyList {
  struct mwMessage head;
  struct mwPrivacyInfo privacy;
};


/* Sense Service */

/** Sent to the server to request the presense of a service by its
    ID. Sent to the client to indicate the presense of such a
    service */
struct mwMsgSenseService {
  struct mwMessage head;
  guint32 service;
};


/* Admin */

/** An administrative broadcast message */
struct mwMsgAdmin {
  struct mwMessage head;
  char *text;
};


/* Announce */

/** An announcement between users */
struct mwMsgAnnounce {
  struct mwMessage head;
  gboolean sender_present;    /**< indicates presence of sender data */
  struct mwLoginInfo sender;  /**< who sent the announcement */
  guint16 unknown_a;          /**< unknown A. Usually 0x00 */
  gboolean may_reply;         /**< replies allowed */
  char *text;                 /**< text of message */

  /** list of (char *) indicating recipients. Recipient users are in
      the format "@U username" and recipient NAB groups are in the
      format "@G groupname" */
  GList *recipients;
};


#ifdef __cplusplus
}
#endif


#endif /* _MW_MESSAGE_H */

