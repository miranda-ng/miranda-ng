
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

#ifndef _MW_SRVC_STORE_H
#define _MW_SRVC_STORE_H


#include <glib.h>
#include "mw_common.h"


#ifdef __cplusplus
extern "C" {
#endif


/** Type identifier for the storage service */
#define mwService_STORAGE  0x00000018


/** @struct mwServiceStorage
    @see mwServiceStorage_new

    Instance of the storage service */
struct mwServiceStorage;


/** @struct mwStorage

    Unit Represents information intended for loading from or saving to
    the storage service */
struct mwStorageUnit;


/** The upper limit of reserved Lotus keys */
#define LOTUS_RESERVED_LIMIT  0x186a0


/** Check if a key is in the range of Lotus reserved keys */
#define KEY_IS_LOTUS_RESERVED(key) \
  (((guint32) key) <= (LOTUS_RESERVED_LIMIT))


/** Some common keys storage keys. Anything in the range 0x00 to
    0x186a0 (100000) is reserved for use by the Lotus
    clients. */
enum mwStorageKey {

  /** The buddy list, in the Sametime .dat file format. String */
  mwStore_AWARE_LIST      = 0x00000000,

  /** Default text for chat invitations. String */
  mwStore_INVITE_CHAT     = 0x00000006,

  /** Default text for meeting invitations. String */
  mwStore_INVITE_MEETING  = 0x0000000e,

  /** Last five Away messages, separated by semicolon. String */
  mwStore_AWAY_MESSAGES   = 0x00000050,

  /** Last five Busy (DND) messages, separated by semicolon. String */
  mwStore_BUSY_MESSAGES   = 0x0000005a,

  /** Last five Active messages, separated by semicolon. String */
  mwStore_ACTIVE_MESSAGES = 0x00000064,
};


/** Appropriate function type for load and store callbacks.
    @param srvc       the storage service
    @param result     the result value of the load or store call
    @param item       the storage unit loaded or saved
    @param data       optional user data
*/
typedef void (*mwStorageCallback)
     (struct mwServiceStorage *srvc,
      guint32 result, struct mwStorageUnit *item,
      gpointer data);


/** Allocates and initializes a storage service instance for use on
    the passed session. */
struct mwServiceStorage *mwServiceStorage_new(struct mwSession *);


/** create an empty storage unit */
struct mwStorageUnit *mwStorageUnit_new(guint32 key);


/** creates a storage unit with the passed key, and a copy of data. */
struct mwStorageUnit *mwStorageUnit_newOpaque(guint32 key,
					      struct mwOpaque *data);


/** creates a storage unit with the passed key, and an encapsulated
    boolean value */
struct mwStorageUnit *mwStorageUnit_newBoolean(guint32 key,
					       gboolean val);


struct mwStorageUnit *mwStorageUnit_newInteger(guint32 key,
					       guint32 val);


/** creates a storage unit with the passed key, and an encapsulated
    string value. */
struct mwStorageUnit *mwStorageUnit_newString(guint32 key,
					      const char *str);


/** get the key for the given storage unit */
guint32 mwStorageUnit_getKey(struct mwStorageUnit *);


/** attempts to obtain a boolean value from a storage unit. If the
    unit is empty, or does not contain the type in a recongnizable
    format, val is returned instead */
gboolean mwStorageUnit_asBoolean(struct mwStorageUnit *, gboolean val);


/** attempts to obtain a guint32 value from a storage unit. If the
    unit is empty, or does not contain the type in a recognizable
    format, val is returned instead */
guint32 mwStorageUnit_asInteger(struct mwStorageUnit *, guint32 val);


/** attempts to obtain a string value from a storage unit. If the unit
    is empty, or does not contain the type in a recognizable format,
    NULL is returned instead. Note that the string returned is a copy,
    and will need to be deallocated at some point. */
char *mwStorageUnit_asString(struct mwStorageUnit *);


/** direct access to the opaque data backing the storage unit */
struct mwOpaque *mwStorageUnit_asOpaque(struct mwStorageUnit *);


/** clears and frees a storage unit */
void mwStorageUnit_free(struct mwStorageUnit *);

      
/** Initiates a load call to the storage service. If the service is
    not currently available, the call will be cached and processed
    when the service is started.

    @param srvc       the storage service
    @param item       storage unit to load
    @param cb         callback function when the load call completes
    @param data       user data for callback
    @param data_free  optional cleanup function for user data
*/
void mwServiceStorage_load(struct mwServiceStorage *srvc,
			   struct mwStorageUnit *item,
			   mwStorageCallback cb,
			   gpointer data, GDestroyNotify data_free);


/** Initiates a store call to the storage service. If the service is
    not currently available, the call will be cached and processed
    when the service is started.

    @param srvc       the storage service
    @param item       storage unit to save
    @param cb         callback function when the load call completes
    @param data       optional user data for callback
    @param data_free  optional cleanup function for user data
 */
void mwServiceStorage_save(struct mwServiceStorage *srvc,
			   struct mwStorageUnit *item,
			   mwStorageCallback cb,
			   gpointer data, GDestroyNotify data_free);


/// Miranda NG adaptation start - new method
struct mwService *mwServiceStorage_getService(struct mwServiceStorage *srvc);
/// Miranda NG adaptation end


#ifdef __cplusplus
}
#endif


#endif /* _MW_SRVC_STORE_H */
