
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

#ifndef _MW_SRVC_AWARE_H
#define _MW_SRVC_AWARE_H


/** @file mw_srvc_aware.h

    The aware service...

    @todo remove the whole idea of an instantiated mwAwareList and
    instead use arbitrary pointers (including NULL) as keys to
    internally stored lists. This removes the problem of the service
    free'ing its lists and invalidating mwAwareList references from
    client code.
*/


#include "mw_common.h"


#ifdef __cplusplus
extern "C" {
#endif


/** Type identifier for the aware service */
#define mwService_AWARE  0x00000011


/** @struct mwServiceAware

    Instance of an Aware Service. The members of this structure are
    not made available. Accessing the parts of an aware service should
    be performed through the appropriate functions. Note that
    instances of this structure can be safely cast to a mwService.
*/
struct mwServiceAware;


/** @struct mwAwareList

    Instance of an Aware List. The members of this structure are not
    made available. Access to the parts of an aware list should be
    handled through the appropriate functions.

    Any references to an aware list are rendered invalid when the
    parent service is free'd
*/
struct mwAwareList;


/** @struct mwAwareAttribute

    Key/Opaque pair indicating an identity's attribute.
 */
struct mwAwareAttribute;


/** Predefined keys appropriate for a mwAwareAttribute
 */
enum mwAwareAttributeKeys {
  mwAttribute_AV_PREFS_SET   = 0x01, /**< A/V prefs specified, gboolean */
  mwAttribute_MICROPHONE     = 0x02, /**< has a microphone, gboolean */
  mwAttribute_SPEAKERS       = 0x03, /**< has speakers, gboolean */
  mwAttribute_VIDEO_CAMERA   = 0x04, /**< has a video camera, gboolean */
  mwAttribute_FILE_TRANSFER  = 0x06, /**< supports file transfers, gboolean */
};


typedef void (*mwAwareAttributeHandler)
     (struct mwServiceAware *srvc,
      struct mwAwareAttribute *attrib);


struct mwAwareHandler {
  mwAwareAttributeHandler on_attrib;
  void (*clear)(struct mwServiceAware *srvc);
};


/** Appropriate function type for the on-aware signal

    @param list  mwAwareList emiting the signal
    @param id    awareness status information
    @param data  user-specified data
*/
typedef void (*mwAwareSnapshotHandler)
     (struct mwAwareList *list,
      struct mwAwareSnapshot *id);


/** Appropriate function type for the on-option signal. The option's
    value may need to be explicitly loaded in some instances,
    resulting in this handler being triggered again.

    @param list    mwAwareList emiting the signal
    @param id      awareness the attribute belongs to
    @param attrib  attribute
*/
typedef void (*mwAwareIdAttributeHandler)
     (struct mwAwareList *list,
      struct mwAwareIdBlock *id,
      struct mwAwareAttribute *attrib);


struct mwAwareListHandler {
  /** handle aware updates */
  mwAwareSnapshotHandler on_aware;

  /** handle attribute updates */
  mwAwareIdAttributeHandler on_attrib;

  /** optional. Called from mwAwareList_free */
  void (*clear)(struct mwAwareList *list);
};


struct mwServiceAware *
mwServiceAware_new(struct mwSession *session,
		   struct mwAwareHandler *handler);


/** Set an attribute value for this session */
int mwServiceAware_setAttribute(struct mwServiceAware *srvc,
				guint32 key, struct mwOpaque *opaque);


int mwServiceAware_setAttributeBoolean(struct mwServiceAware *srvc,
				       guint32 key, gboolean val);


int mwServiceAware_setAttributeInteger(struct mwServiceAware *srvc,
				       guint32 key, guint32 val);


int mwServiceAware_setAttributeString(struct mwServiceAware *srvc,
				      guint32 key, const char *str);


/** Unset an attribute for this session */
int mwServiceAware_unsetAttribute(struct mwServiceAware *srvc,
				  guint32 key);


guint32 mwAwareAttribute_getKey(const struct mwAwareAttribute *attrib);


gboolean mwAwareAttribute_asBoolean(const struct mwAwareAttribute *attrib);


guint32 mwAwareAttribute_asInteger(const struct mwAwareAttribute *attrib);


/** Copy of attribute string, must be g_free'd. If the attribute's
    content cannot be loaded as a string, returns NULL */
char *mwAwareAttribute_asString(const struct mwAwareAttribute *attrib);


/** Direct access to an attribute's underlying opaque */
const struct mwOpaque *
mwAwareAttribute_asOpaque(const struct mwAwareAttribute *attrib);


/** Allocate and initialize an aware list */
struct mwAwareList *
mwAwareList_new(struct mwServiceAware *srvc,
		struct mwAwareListHandler *handler);


/** Clean and free an aware list */
void mwAwareList_free(struct mwAwareList *list);


struct mwAwareListHandler *mwAwareList_getHandler(struct mwAwareList *list);


/// Miranda NG adaptation start - new method
struct mwServiceAware *mwAwareList_getServiceAware(struct mwAwareList *list);
/// Miranda NG adaptation end


/** Add a collection of user IDs to an aware list.
    @param list     mwAwareList to add user ID to
    @param id_list  mwAwareIdBlock list of user IDs to add
    @return         0 for success, non-zero to indicate an error.
*/
int mwAwareList_addAware(struct mwAwareList *list, GList *id_list);


/** Remove a collection of user IDs from an aware list.
    @param list     mwAwareList to remove user ID from
    @param id_list  mwAwareIdBlock list of user IDs to remove
    @return  0      for success, non-zero to indicate an error.
*/
int mwAwareList_removeAware(struct mwAwareList *list, GList *id_list);


int mwAwareList_removeAllAware(struct mwAwareList *list);


/** watch an NULL terminated array of keys */
int mwAwareList_watchAttributeArray(struct mwAwareList *list,
				    guint32 *keys);


/** watch a NULL terminated list of keys */
int mwAwareList_watchAttributes(struct mwAwareList *list,
				guint32 key, ...);


/** stop watching a NULL terminated array of keys */
int mwAwareList_unwatchAttributeArray(struct mwAwareList *list,
				      guint32 *keys);


/** stop watching a NULL terminated list of keys */
int mwAwareList_unwatchAttributes(struct mwAwareList *list,
				  guint32 key, ...);


/** remove all watched attributes */
int mwAwareList_unwatchAllAttributes(struct mwAwareList *list);


guint32 *mwAwareList_getWatchedAttributes(struct mwAwareList *list);


void mwAwareList_setClientData(struct mwAwareList *list,
			       gpointer data, GDestroyNotify cleanup);


void mwAwareList_removeClientData(struct mwAwareList *list);


gpointer mwAwareList_getClientData(struct mwAwareList *list);


/** trigger a got_aware event constructed from the passed user and
    status information. Useful for adding false users and having the
    getText function work for them */
void mwServiceAware_setStatus(struct mwServiceAware *srvc,
			      struct mwAwareIdBlock *user,
			      struct mwUserStatus *stat);


/** look up the status description for a user */
const char *mwServiceAware_getText(struct mwServiceAware *srvc,
				   struct mwAwareIdBlock *user);


/** look up the last known copy of an attribute for a user by the
    attribute's key */
const struct mwAwareAttribute *
mwServiceAware_getAttribute(struct mwServiceAware *srvc,
			    struct mwAwareIdBlock *user,
			    guint32 key);


/// Miranda NG adaptation start - new method
struct mwService *mwServiceAware_getService(struct mwServiceAware *srvc);
/// Miranda NG adaptation end


#ifdef __cplusplus
}
#endif


#endif /* _MW_SRVC_AWARE_H */

