
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

#ifndef _MW_SRVC_PLACE_H
#define _MW_SRVC_PLACE_H


#include <glib/glist.h>
#include "mw_common.h"


#ifdef __cplusplus
extern "C" {
#endif


/** Type identifier for the place service */
#define mwService_PLACE  0x80000022


/** @struct mwServicePlace */
struct mwServicePlace;


/** @struct mwPlace */
struct mwPlace;


struct mwPlaceHandler {
  void (*opened)(struct mwPlace *place);
  void (*closed)(struct mwPlace *place, guint32 code);

  void (*peerJoined)(struct mwPlace *place,
		     const struct mwIdBlock *peer);

  void (*peerParted)(struct mwPlace *place,
		     const struct mwIdBlock *peer);

  void (*peerSetAttribute)(struct mwPlace *place,
			   const struct mwIdBlock *peer,
			   guint32 attr, struct mwOpaque *o);

  void (*peerUnsetAttribute)(struct mwPlace *place,
			     const struct mwIdBlock *peer,
			     guint32 attr);

  void (*message)(struct mwPlace *place,
		  const struct mwIdBlock *who,
		  const char *msg);

  void (*clear)(struct mwServicePlace *srvc);
};


enum mwPlacePeerAttribute {
  mwPlacePeer_TYPING = 0x00000008,
};


struct mwServicePlace *
mwServicePlace_new(struct mwSession *session,
		   struct mwPlaceHandler *handler);


struct mwPlaceHandler *
mwServicePlace_getHandler(struct mwServicePlace *srvc);


const GList *mwServicePlace_getPlaces(struct mwServicePlace *srvc);


struct mwPlace *mwPlace_new(struct mwServicePlace *srvc,
			    const char *name, const char *title);


struct mwServicePlace *mwPlace_getService(struct mwPlace *place);


const char *mwPlace_getName(struct mwPlace *place);


const char *mwPlace_getTitle(struct mwPlace *place);


int mwPlace_open(struct mwPlace *place);


int mwPlace_destroy(struct mwPlace *place, guint32 code);


/** returns a GList* of struct mwIdBlock*. The GList will need to be
    freed after use, the mwIdBlock structures should not be modified
    or freed */
GList *mwPlace_getMembers(struct mwPlace *place);


int mwPlace_sendText(struct mwPlace *place, const char *msg);


/** send a legacy invitation for this place to a user. The user will
    receive an apparent invitation from a Conference (rather than a
    Place) */
int mwPlace_legacyInvite(struct mwPlace *place,
			 struct mwIdBlock *idb,
			 const char *message);


int mwPlace_setAttribute(struct mwPlace *place, guint32 attrib,
			 struct mwOpaque *data);


int mwPlace_unsetAttribute(struct mwPlace *place, guint32 attrib);


void mwPlace_setClientData(struct mwPlace *place,
			   gpointer data, GDestroyNotify clean);


gpointer mwPlace_getClientData(struct mwPlace *place);


void mwPlace_removeClientData(struct mwPlace *place);


#ifdef __cplusplus
}
#endif


#endif /* _MW_SRVC_PLACE_H */

