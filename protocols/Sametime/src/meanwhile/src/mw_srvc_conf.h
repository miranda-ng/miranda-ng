
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

#ifndef _MW_SRVC_CONF_H
#define _MW_SRVC_CONF_H


#include <glib.h>
#include "mw_common.h"


#ifdef __cplusplus
extern "C" {
#endif


/** Type identifier for the conference service */
#define mwService_CONFERENCE  0x80000010


enum mwConferenceState {
  mwConference_NEW,      /**< new outgoing conference */
  mwConference_PENDING,  /**< outgoing conference pending creation */
  mwConference_INVITED,  /**< invited to incoming conference */
  mwConference_OPEN,     /**< conference open and active */
  mwConference_CLOSING,  /**< conference is closing */
  mwConference_ERROR,    /**< conference is closing due to error */
  mwConference_UNKNOWN,  /**< unable to determine conference state */
};


/** @struct mwServiceConference
    Instance of the multi-user conference service */
struct mwServiceConference;


/** @struct mwConference
    A multi-user chat */
struct mwConference;


/** Handler structure used to provide callbacks for an instance of the
    conferencing service */
struct mwConferenceHandler {

  /** triggered when we receive a conference invitation. Call
      mwConference_accept to accept the invitation and join the
      conference, or mwConference_close to reject the invitation.

      @param conf     the newly created conference
      @param inviter  the indentity of the user who sent the invitation
      @param invite   the invitation text
   */
  void (*on_invited)(struct mwConference *conf,
		     struct mwLoginInfo *inviter, const char *invite);

  /** triggered when we enter the conference. Provides the initial
      conference membership list as a GList of mwLoginInfo structures

      @param conf     the conference just joined
      @param members  mwLoginInfo list of existing conference members
  */
  void (*conf_opened)(struct mwConference *conf, GList *members);

  /** triggered when a conference is closed. This is typically when
      we've left it */
  void (*conf_closed)(struct mwConference *, guint32 reason);

  /** triggered when someone joins the conference */
  void (*on_peer_joined)(struct mwConference *, struct mwLoginInfo *);

  /** triggered when someone leaves the conference */
  void (*on_peer_parted)(struct mwConference *, struct mwLoginInfo *);

  /** triggered when someone says something */
  void (*on_text)(struct mwConference *conf,
		  struct mwLoginInfo *who, const char *what);

  /** typing notification */
  void (*on_typing)(struct mwConference *conf,
		    struct mwLoginInfo *who, gboolean typing);

  /** optional. called from mwService_free */
  void (*clear)(struct mwServiceConference *srvc);
};


/** Allocate a new conferencing service, attaching the given handler
    @param sess     owning session
    @param handler  handler providing call-back functions for the service
 */
struct mwServiceConference *
mwServiceConference_new(struct mwSession *sess,
			struct mwConferenceHandler *handler);


/** @returns the conference handler for the service */
struct mwConferenceHandler *
mwServiceConference_getHandler(struct mwServiceConference *srvc);


/** a mwConference list of the conferences in this service. The GList
    will need to be destroyed with g_list_free after use */
GList *mwServiceConference_getConferences(struct mwServiceConference *srvc);


/** Allocate a new conference, in state NEW with the given title.
    @see mwConference_create */
struct mwConference *mwConference_new(struct mwServiceConference *srvc,
				      const char *title);


/** @returns the owning service of a conference */
/// Miranda NG adaptation start - renamed method
struct mwServiceConference *mwConference_getServiceConference(struct mwConference *conf);
/// Miranda NG adaptation end


/// Miranda NG adaptation start - new method
struct mwService *mwServiceConference_getService(struct mwServiceConference *srvc);
/// Miranda NG adaptation end


/** @returns unique conference name */
const char *mwConference_getName(struct mwConference *conf);


/** @returns conference title */
const char *mwConference_getTitle(struct mwConference *conf);


/** a mwIdBlock list of the members of the conference. The GList will
    need to be free'd after use */
GList *mwConference_getMembers(struct mwConference *conf);


/** Initiate a conference. Conference must be in state NEW. If no name
    or title for the conference has been set, they will be
    generated. Conference will be placed into state PENDING. */
int mwConference_open(struct mwConference *conf);


/** Leave and close an existing conference, or reject an invitation.
    Triggers mwServiceConfHandler::conf_closed and free's the
    conference.
 */
int mwConference_destroy(struct mwConference *conf,
			 guint32 reason, const char *text);


#define mwConference_reject(c,r,t) \
  mwConference_destroy((c),(r),(t))


/** accept a conference invitation. Conference must be in the state
    INVITED. */
int mwConference_accept(struct mwConference *conf);


/** invite another user to an ACTIVE conference
    @param conf  conference
    @param who   user to invite
    @param text  invitation message
 */
int mwConference_invite(struct mwConference *conf,
			struct mwIdBlock *who, const char *text);


/** send a text message over an open conference */
int mwConference_sendText(struct mwConference *conf, const char *text);


/** send typing notification over an open conference */
int mwConference_sendTyping(struct mwConference *conf, gboolean typing);


/** associate arbitrary client data and an optional cleanup function
    with a conference. If there is already client data with a clear
    function, it will not be called. */
void mwConference_setClientData(struct mwConference *conf,
				gpointer data, GDestroyNotify clear);


/** reference associated client data */
gpointer mwConference_getClientData(struct mwConference *conf);


/** remove associated client data if any, and call the cleanup
    function on the data as necessary */
void mwConference_removeClientData(struct mwConference *conf);


#ifdef __cplusplus
}
#endif
				    

#endif /* _MW_SRVC_CONF_H */

