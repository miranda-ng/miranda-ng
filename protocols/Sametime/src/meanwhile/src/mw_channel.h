
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

#ifndef _MW_CHANNEL_H
#define _MW_CHANNEL_H


/** @file mw_channel.h
    
Life-cycle of an outgoing channel:

1: mwChannel_new is called. If there is a channel in the outgoing
collection in state NEW, then it is returned. Otherwise, a channel
is allocated, assigned a unique outgoing id, marked as NEW, and
returned.

2: channel is set to INIT status (effectively earmarking it as in-
use).  fields on the channel can then be set as necessary to
prepare it for creation.

3: mwChannel_create is called. The channel is marked to WAIT status
and a message is sent to the server. The channel is also marked as
inactive as of that moment.

4: the channel is accepted (step 5) or rejected (step 7)

5: an accept message is received from the server, and the channel
is marked as OPEN, and the inactive mark is removed. And messages
in the in or out queues for that channel are processed. The channel
is now ready to be used.

6: data is sent and received over the channel

7: the channel is closed either by receipt of a close message or by
local action. If by local action, then a close message is sent to
the server.  The channel is cleaned up, its queues dumped, and it
is set to NEW status to await re-use.

Life-cycle of an incoming channel:

1: a channel create message is received. A channel is allocated and
given an id matching the message. It is placed in status WAIT, and
marked as inactive as of that moment. The service matching that
channel is alerted of the incoming creation request.

2: the service can either accept (step 3) or reject (step 5) the
channel

3: mwChannel_accept is called. The channel is marked as OPEN, and
an accept message is sent to the server. And messages in the in or
out queues for that channel are processed. The channel is now ready
to be used.

4: data is sent and received over the channel

5: The channel is closed either by receipt of a close message or by
local action. If by local action, then a close message is sent to
the server.  The channel is cleaned up, its queues dumped, and it
is deallocated. */


#include <time.h>
#include "mw_common.h"


#ifdef __cplusplus
extern "C" {
#endif


/* place-holders */
struct mwCipherInstance;
struct mwMsgChannelAccept;
struct mwMsgChannelCreate;
struct mwMsgChannelDestroy;
struct mwMsgChannelSend;
struct mwService;
struct mwSession;



/** @struct mwChannel
    Represents a channel to a service */
struct mwChannel;


/** @struct mwChannelSet
    Collection of channels */
struct mwChannelSet;


/** special ID indicating the master channel */
#define MW_MASTER_CHANNEL_ID  0x00000000


/** non-zero if a channel id appears to be that of an outgoing channel */
#define mwChannel_idIsOutgoing(id) \
  (! (0x80000000 & (id)))

/** non-zero if a channel id appears to be that of an incoming channel */
#define mwChannel_idIsIncoming(id) \
  (! mwChannel_idIsOutgoing(id))

/** non-zero if a channel appears to be an outgoing channel */
#define mwChannel_isOutgoing(chan) \
  mwChannel_idIsOutgoing(mwChannel_getId(chan))

/** non-zero if a channel appears to be an incoming channel */
#define mwChannel_isIncoming(chan) \
  mwChannel_idIsIncoming(mwChannel_getId(chan))


/** channel status */
enum mwChannelState {
  mwChannel_NEW,      /**< channel is newly allocated, in the pool */
  mwChannel_INIT,     /**< channel is being prepared, out of the pool */
  mwChannel_WAIT,     /**< channel is waiting for accept */
  mwChannel_OPEN,     /**< channel is accepted and open */
  mwChannel_DESTROY,  /**< channel is being destroyed */
  mwChannel_ERROR,    /**< channel is being destroyed due to error */
  mwChannel_UNKNOWN,  /**< unknown state, or error determining state */
};


#define mwChannel_isState(chan, state) \
  (mwChannel_getState(chan) == (state))


/** channel statistic fields.
    @see mwChannel_getStatistic */
enum mwChannelStatField {
  mwChannelStat_MSG_SENT,      /**< total send-on-chan messages sent */
  mwChannelStat_MSG_RECV,      /**< total send-on-chan messages received */
  mwChannelStat_U_BYTES_SENT,  /**< total bytes sent, pre-encryption */
  mwChannelStat_U_BYTES_RECV,  /**< total bytes received, post-decryption */
  mwChannelStat_OPENED_AT,     /**< time when channel was opened */
  mwChannelStat_CLOSED_AT,     /**< time when channel was closed */
};


/** @enum mwEncryptPolicy

    Policy for a channel, dictating what sort of encryption should be
    used, if any, and when.
*/
enum mwEncryptPolicy {
  mwEncrypt_NONE      = 0x0000, /**< encrypt none */
  mwEncrypt_WHATEVER  = 0x0001, /**< encrypt whatever you want */
  mwEncrypt_ALL       = 0x0002, /**< encrypt all, any cipher */
  mwEncrypt_RC2_40    = 0x1000, /**< encrypt all, RC2/40 cipher */
  mwEncrypt_RC2_128   = 0x2000, /**< encrypt all, RC2/128 cipher */
};


/** Allocate and initialize a channel set for a session */
struct mwChannelSet *mwChannelSet_new(struct mwSession *);


/** Clear and deallocate a channel set. Closes, clears, and frees all
    contained channels. */
void mwChannelSet_free(struct mwChannelSet *);


/** Create an incoming channel with the given channel id. Channel's state
    will be set to WAIT. Primarily for use in mw_session */
struct mwChannel *mwChannel_newIncoming(struct mwChannelSet *, guint32 id);


/** Create an outgoing channel. Its channel ID will be generated by
    the owning channel set. Channel's state will be set to INIT */
struct mwChannel *mwChannel_newOutgoing(struct mwChannelSet *);


/** Obtain a reference to a channel by its id.
    @returns the channel matching chan, or NULL */
struct mwChannel *mwChannel_find(struct mwChannelSet *cs, guint32 chan);


/** get the ID for a channel. 0x00 indicates an error, as that is not
    a permissible value */
guint32 mwChannel_getId(struct mwChannel *);


/** get the session for a channel. */
struct mwSession *mwChannel_getSession(struct mwChannel *);


/** get the ID of the service for a channel. This may be 0x00 for NEW
    channels */
guint32 mwChannel_getServiceId(struct mwChannel *);


/** get the service for a channel. This may be NULL for NEW
    channels */
struct mwService *mwChannel_getService(struct mwChannel *);


/** associate a channel with an owning service */
void mwChannel_setService(struct mwChannel *chan, struct mwService *srvc);


/** get service-specific data. This is for use by service
    implementations to easily associate information with the
    channel */
gpointer mwChannel_getServiceData(struct mwChannel *chan);


/** set service-specific data. This is for use by service
    implementations to easily associate information with the
    channel */
void mwChannel_setServiceData(struct mwChannel *chan,
			      gpointer data, GDestroyNotify clean);


void mwChannel_removeServiceData(struct mwChannel *chan);


guint32 mwChannel_getProtoType(struct mwChannel *chan);


void mwChannel_setProtoType(struct mwChannel *chan, guint32 proto_type);


guint32 mwChannel_getProtoVer(struct mwChannel *chan);


void mwChannel_setProtoVer(struct mwChannel *chan, guint32 proto_ver);


/** Channel encryption policy.

    Cannot currently be set, used internally to automatically
    negotiate ciphers. Future revisions may allow this to be specified
    in a new channel to dictate channel encryption.

    @see enum mwEncryptPolicy
*/
guint16 mwChannel_getEncryptPolicy(struct mwChannel *chan);


guint32 mwChannel_getOptions(struct mwChannel *chan);


void mwChannel_setOptions(struct mwChannel *chan, guint32 options);


/** User at the other end of the channel. The target user for outgoing
    channels, the creator for incoming channels */
struct mwLoginInfo *mwChannel_getUser(struct mwChannel *chan);


/** direct reference to the create addtl information for a channel */
struct mwOpaque *mwChannel_getAddtlCreate(struct mwChannel *);


/** direct reference to the accept addtl information for a channel */
struct mwOpaque *mwChannel_getAddtlAccept(struct mwChannel *);


/** automatically adds instances of all ciphers in the session to the
    list of supported ciphers for a channel */
void mwChannel_populateSupportedCipherInstances(struct mwChannel *chan);


/** add a cipher instance to a channel's list of supported
    ciphers. Channel must be NEW. */
void mwChannel_addSupportedCipherInstance(struct mwChannel *chan,
					  struct mwCipherInstance *ci);


/** the list of supported ciphers for a channel. This list will be
    empty once a cipher has been selected for the channel */
GList *mwChannel_getSupportedCipherInstances(struct mwChannel *chan);


/** select a cipher instance for a channel. A NULL instance indicates
    that no encryption should be used. */
void mwChannel_selectCipherInstance(struct mwChannel *chan,
				    struct mwCipherInstance *ci);


struct mwCipherInstance *
mwChannel_getCipherInstance(struct mwChannel *chan);


/** get the state of a channel  */
enum mwChannelState mwChannel_getState(struct mwChannel *);


/** obtain the value for a statistic field as a gpointer */
gpointer mwChannel_getStatistic(struct mwChannel *chan,
				enum mwChannelStatField stat);


/** Formally open a channel.

    For outgoing channels: instruct the session to send a channel
    create message to the server, and to mark the channel (which must
    be in INIT status) as being in WAIT status.
   
    For incoming channels: configures the channel according to options
    in the channel create message. Marks the channel as being in WAIT
    status
*/
int mwChannel_create(struct mwChannel *chan);


/** Formally accept an incoming channel. Instructs the session to send
    a channel accept message to the server, and to mark the channel as
    being OPEN. */
int mwChannel_accept(struct mwChannel *chan);


/** Destroy a channel. Sends a channel-destroy message to the server,
    and perform cleanup to remove the channel.

    @param chan    the channel to destroy
    @param reason  the reason code for closing the channel
    @param data    optional additional information 
*/
int mwChannel_destroy(struct mwChannel *chan, guint32 reason,
		      struct mwOpaque *data);


/** Compose a send-on-channel message, encrypt it as per the channel's
    specification, and send it */
int mwChannel_send(struct mwChannel *chan, guint32 msg_type,
		   struct mwOpaque *msg);


/** Compose a send-on-channel message, and if encrypt is TRUE, encrypt
    it as per the channel's specification, and send it */
int mwChannel_sendEncrypted(struct mwChannel *chan,
			    guint32 msg_type, struct mwOpaque *msg,
			    gboolean encrypt);


/** pass a create message to a channel for handling */
void mwChannel_recvCreate(struct mwChannel *chan,
			  struct mwMsgChannelCreate *msg);


/** pass an accept message to a channel for handling */
void mwChannel_recvAccept(struct mwChannel *chan,
			  struct mwMsgChannelAccept *msg);


/** pass a destroy message to a channel for handling */
void mwChannel_recvDestroy(struct mwChannel *chan,
			   struct mwMsgChannelDestroy *msg);


/** Feed data into a channel. */
void mwChannel_recv(struct mwChannel *chan, struct mwMsgChannelSend *msg);


#ifdef __cplusplus
}
#endif


#endif /* _MW_CHANNEL_H */

