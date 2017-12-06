
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

#ifndef _MW_SESSION_H
#define _MW_SESSION_H


/** @file mw_session.h

    A client session with a Sametime server is encapsulated in the
    mwSession structure. The session controls channels, provides
    encryption ciphers, and manages services using messages over the
    Master channel.

    A session does not directly communicate with a socket or stream,
    instead the session is initialized from client code with an
    instance of a mwSessionHandler structure. This session handler
    provides functions as call-backs for common session events, and
    provides functions for writing-to and closing the connection to
    the server.

    A session does not perform reads on a socket directly. Instead, it
    must be fed from an outside source via the mwSession_recv
    function. The session will buffer and merge data passed to this
    function to build complete protocol messages, and will act upon
    each complete message accordingly.
*/


#include "mw_common.h"


#ifdef __cplusplus
extern "C" {
#endif


struct mwChannelSet;
struct mwCipher;
struct mwMessage;
struct mwService;


/** default protocol major version */
#define MW_PROTOCOL_VERSION_MAJOR  0x001e


/** default protocol minor version */
#define MW_PROTOCOL_VERSION_MINOR  0x001d


/** @section Session Properties
    for use with mwSession_setProperty, et al.
*/
/*@{*/

/** char *, session user ID */
#define mwSession_AUTH_USER_ID      "session.auth.user"

/** char *, plaintext password */
#define mwSession_AUTH_PASSWORD     "session.auth.password"

/** struct mwOpaque *, authentication token */
#define mwSession_AUTH_TOKEN        "session.auth.token"

/** char *, hostname of client */
#define mwSession_CLIENT_HOST       "client.host"

/** guint32, local IP of client */
#define mwSession_CLIENT_IP         "client.ip"

/** guint16, major version of client protocol */
#define mwSession_CLIENT_VER_MAJOR  "client.version.major"

/** guint16, minor version of client protocol */
#define mwSession_CLIENT_VER_MINOR  "client.version.minor"

/** guint16, client type identifier */
#define mwSession_CLIENT_TYPE_ID    "client.id"

/** guint16, major version of server protocol */
#define mwSession_SERVER_VER_MAJOR  "server.version.major"

/** guint16, minor version of server protocol */
#define mwSession_SERVER_VER_MINOR  "server.version.minor"

/*@}*/


enum mwSessionState {
  mwSession_STARTING,      /**< session is starting */
  mwSession_HANDSHAKE,     /**< session has sent handshake */
  mwSession_HANDSHAKE_ACK, /**< session has received handshake ack */
  mwSession_LOGIN,         /**< session has sent login */
  mwSession_LOGIN_REDIR,   /**< session has been redirected */
  mwSession_LOGIN_ACK,     /**< session has received login ack */
  mwSession_STARTED,       /**< session is active */
  mwSession_STOPPING,      /**< session is shutting down */
  mwSession_STOPPED,       /**< session is stopped */
  mwSession_UNKNOWN,       /**< indicates an error determining state */
  mwSession_LOGIN_CONT,    /**< session has sent a login continue */
};


#define mwSession_isState(session, state) \
  (mwSession_getState((session)) == (state))

#define mwSession_isStarting(s) \
  (mwSession_isState((s), mwSession_STARTING)  || \
   mwSession_isState((s), mwSession_HANDSHAKE) || \
   mwSession_isState((s), mwSession_HANDSHAKE_ACK) || \
   mwSession_isState((s), mwSession_LOGIN) || \
   mwSession_isState((s), mwSession_LOGIN_ACK) || \
   mwSession_isState((s), mwSession_LOGIN_REDIR) || \
   mwSession_isState((s), mwSession_LOGIN_CONT))

#define mwSession_isStarted(s) \
  (mwSession_isState((s), mwSession_STARTED))

#define mwSession_isStopping(s) \
  (mwSession_isState((s), mwSession_STOPPING))

#define mwSession_isStopped(s) \
  (mwSession_isState((s), mwSession_STOPPED))


/** @struct mwSession

    Represents a Sametime client session */
struct mwSession;


/** @struct mwSessionHandler

    session handler. Structure which interfaces a session with client
    code to provide I/O and event handling */
struct mwSessionHandler {
  
  /** write data to the server connection. Required. Should return
      zero for success, non-zero for error */
  int (*io_write)(struct mwSession *, const guchar *buf, gsize len);
  
  /** close the server connection. Required */
  void (*io_close)(struct mwSession *);

  /** triggered by mwSession_free. Optional. Put cleanup code here */
  void (*clear)(struct mwSession *);

  /** Called when the session has changed status.

      @see mwSession_getStateInfo for uses of info field

      @param s      the session
      @param state  the session's state
      @param info   additional state information */
  void (*on_stateChange)(struct mwSession *s,
			 enum mwSessionState state, gpointer info);

  /** called when privacy information has been sent or received

      @see mwSession_getPrivacyInfo
  */
  void (*on_setPrivacyInfo)(struct mwSession *);

  /** called when user status has changed

      @see mwSession_getUserStatus */
  void (*on_setUserStatus)(struct mwSession *);

  /** called when an admin messages has been received */
  void (*on_admin)(struct mwSession *, const char *text);

  /** called when an announcement arrives */
  void (*on_announce)(struct mwSession *, struct mwLoginInfo *from,
		      gboolean may_reply, const char *text);

};


/** allocate a new session */
struct mwSession *mwSession_new(struct mwSessionHandler *);


/** stop, clear, free a session. Does not free contained ciphers or
    services, these must be taken care of explicitly. */
void mwSession_free(struct mwSession *);


/** obtain a reference to the session's handler */
struct mwSessionHandler *mwSession_getHandler(struct mwSession *);


/** instruct the session to begin. This will result in the initial
    handshake message being sent. */
void mwSession_start(struct mwSession *);


/** instruct the session to shut down with the following reason
    code. */
void mwSession_stop(struct mwSession *, guint32 reason);


/** Data is buffered, unpacked, and parsed into a message, then
    processed accordingly. */
void mwSession_recv(struct mwSession *, const guchar *, gsize);


/** primarily used by services to have messages serialized and sent
    @param s    session to send message over
    @param msg  message to serialize and send
    @returns    0 for success */
int mwSession_send(struct mwSession *s, struct mwMessage *msg);


/** sends the keepalive byte */
int mwSession_sendKeepalive(struct mwSession *s);


/** respond to a login redirect message by forcing the login sequence
    to continue through the immediate server. */
int mwSession_forceLogin(struct mwSession *s);


/** send an announcement to a list of users/groups. Targets of
    announcement must be in the same community as the session.

    @param s          session to send announcement from
    @param may_reply  permit clients to reply. Not all clients honor this.
    @param text       text of announcement
    @param recipients list of recipients. Each recipient is specified
                      by a single string, prefix with "@U " for users
                      and "@G " for Notes Address Book groups.
*/
int mwSession_sendAnnounce(struct mwSession *s, gboolean may_reply,
			   const char *text, const GList *recipients);


/** set the internal privacy information, and inform the server as
    necessary. Triggers the on_setPrivacyInfo call-back. */
int mwSession_setPrivacyInfo(struct mwSession *, struct mwPrivacyInfo *);


/** direct reference to the session's internal privacy structure */
struct mwPrivacyInfo *mwSession_getPrivacyInfo(struct mwSession *);


/** reference the login information for the session */
struct mwLoginInfo *mwSession_getLoginInfo(struct mwSession *);


/** set the internal user status state, and inform the server as
    necessary. Triggers the on_setUserStatus call-back */
int mwSession_setUserStatus(struct mwSession *, struct mwUserStatus *);


struct mwUserStatus *mwSession_getUserStatus(struct mwSession *);


/** current status of the session */
enum mwSessionState mwSession_getState(struct mwSession *);


/** additional status-specific information. Depending on the state of
    the session, this value has different meaning.

    @li @c mwSession_STOPPING guint32 error code causing
    the session to shut down

    @li @c mwSession_STOPPED guint32 error code causing
    the session to shut down

    @li @c mwSession_LOGIN_REDIR (char *) host to redirect
    to
*/
gpointer mwSession_getStateInfo(struct mwSession *);


struct mwChannelSet *mwSession_getChannels(struct mwSession *);


/** adds a service to the session. If the session is started (or when
    the session is successfully started) and the service has a start
    function, the session will request service availability from the
    server. On receipt of the service availability notification, the
    session will call the service's start function.

    @return TRUE if the session was added correctly */
gboolean mwSession_addService(struct mwSession *, struct mwService *);


/** find a service by its type identifier */
struct mwService *mwSession_getService(struct mwSession *, guint32 type);


/** removes a service from the session. If the session is started and
    the service has a stop function, it will be called. Returns the
    removed service */
struct mwService *mwSession_removeService(struct mwSession *, guint32 type);


/** a GList of services in this session. The GList needs to be freed
    after use */
GList *mwSession_getServices(struct mwSession *);


/** instruct a STARTED session to check the server for the presense of
    a given service. The service will be automatically started upon
    receipt of an affirmative reply from the server. This function is
    automatically called upon all services in a session when the
    session is fully STARTED.

    Services which terminate due to an error may call this on
    themselves to re-initialize when their server-side counterpart is
    made available again.

    @param s     owning session
    @param type  service type ID */
void mwSession_senseService(struct mwSession *s, guint32 type);


/** adds a cipher to the session. */
gboolean mwSession_addCipher(struct mwSession *, struct mwCipher *);


/** find a cipher by its type identifier */
struct mwCipher *mwSession_getCipher(struct mwSession *, guint16 type);


/** remove a cipher from the session */
struct mwCipher *mwSession_removeCipher(struct mwSession *, guint16 type);


/** a GList of ciphers in this session. The GList needs to be freed
    after use */
GList *mwSession_getCiphers(struct mwSession *);


/** associate a key:value pair with the session. If an existing value is
    associated with the same key, it will have its clear function called
    and will be replaced with the new value */
void mwSession_setProperty(struct mwSession *, const char *key,
			   gpointer val, GDestroyNotify clear);


/** obtain the value of a previously set property, or NULL */
gpointer mwSession_getProperty(struct mwSession *, const char *key);


/** remove a property, calling the optional GDestroyNotify function
    indicated in mwSession_setProperty if applicable */
void mwSession_removeProperty(struct mwSession *, const char *key);


/** associate arbitrary data with the session for use by the client
    code. Only client applications should use this, never services.

    @param session  the session to associate the data with
    @param data     arbitrary client data
    @param clear    optional cleanup function called on data from
                    mwSession_removeClientData and mwSession_free
*/
void mwSession_setClientData(struct mwSession *session,
			     gpointer data, GDestroyNotify clear);


gpointer mwSession_getClientData(struct mwSession *session);


/** remove client data, calling the optional GDestroyNotify function
    indicated in mwSession_setClientData if applicable */
void mwSession_removeClientData(struct mwSession *session);


#ifdef __cplusplus
}
#endif


#endif /* _MW_SESSION_H */

