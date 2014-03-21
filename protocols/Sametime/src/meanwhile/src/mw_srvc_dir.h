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

#ifndef _MW_SRVC_DIR_H
#define _MW_SERV_DIR_H


#include <glib.h>
#include <glib/glist.h>


#ifdef __cplusplus
extern "C" {
#endif


struct mwSession;


#define SERVICE_DIRECTORY  0x0000001a


/** @struct mwServiceDirectory

    the directory service. */
struct mwServiceDirectory;


/** @struct mwAddressBook

    server-side collection of users and groups. Open a directory
    based on an address book to search or list its contents */
struct mwAddressBook;


/** @struct mwDirectory

    searchable directory, based off of an address book */
struct mwDirectory;


enum mwDirectoryState {
  mwDirectory_NEW,      /**< directory is created, but not open */
  mwDirectory_PENDING,  /**< directory has in the process of opening */
  mwDirectory_OPEN,     /**< directory is open */
  mwDirectory_ERROR,    /**< error opening or using directory */
  mwDirectory_UNKNOWN,  /**< error determining directory state */
};


/** return value of directory searches that fail */
#define DIR_SEARCH_ERROR  0x00000000


#define MW_DIRECTORY_IS_STATE(dir, state) \
  (mwDirectory_getState(dir) == (state))

#define MW_DIRECTORY_IS_NEW(dir) \
  MW_DIRECTORY_IS_STATE((dir), mwDirectory_NEW)

#define MW_DIRECTORY_IS_PENDING(dir) \
  MW_DIRECTORY_IS_STATE((dir), mwDirectory_PENDING)

#define MW_DIRECTORY_IS_OPEN(dir) \
  MW_DIRECTORY_IS_STATE((dir), mwDirectory_OPEN)


enum mwDirectoryMemberType {
  mwDirectoryMember_USER   = 0x0000,
  mwDirectoryMember_GROUP  = 0x0001,
};


struct mwDirectoryMember {
  guint16 type;      /**< @see mwDirectoryMemberType */
  char *id;          /**< proper ID for member */
  char *long_name;   /**< full name of member (USER type only) */
  char *short_name;  /**< short name of member */
  guint16 foo;       /**< unknown */
};


/** Appropriate function signature for handling directory search results */
typedef void (*mwSearchHandler)
     (struct mwDirectory *dir,
      guint32 code, guint32 offset, GList *members);


/** handles asynchronous events for a directory service instance */
struct mwDirectoryHandler {

  /** handle receipt of the address book list from the service.
      Initially triggered by mwServiceDirectory_refreshAddressBooks
      and at service startup */
  void (*on_book_list)(struct mwServiceDirectory *srvc, GList *books);

  /** triggered when a directory has been successfully opened */
  void (*dir_opened)(struct mwDirectory *dir);

  /** triggered when a directory has been closed */
  void (*dir_closed)(struct mwDirectory *dir, guint32 reason);

  /** optional. called from mwService_free */
  void (*clear)(struct mwServiceDirectory *srvc);
};


/** Allocate a new directory service instance for use with session */
struct mwServiceDirectory *
mwServiceDirectory_new(struct mwSession *session,
		       struct mwDirectoryHandler *handler);


/** the handler associated with the service at its creation */
struct mwDirectoryHandler *
mwServiceDirectory_getHandler(struct mwServiceDirectory *srvc);


/** most recent list of address books available in service */
GList *mwServiceDirectory_getAddressBooks(struct mwServiceDirectory *srvc);


/** submit a request to obtain an updated list of address books from
    service */
int mwServiceDirectory_refreshAddressBooks(struct mwServiceDirectory *srvc);


/** list of directories in the service */
GList *mwServiceDirectory_getDirectories(struct mwServiceDirectory *srvc);


/** list of directories associated with address book. Note that the
    returned GList will need to be free'd after use */
GList *mwAddressBook_getDirectories(struct mwAddressBook *book);


/** the name of the address book */
const char *mwAddressBook_getName(struct mwAddressBook *book);


/** allocate a new directory based off the given address book */
struct mwDirectory *mwDirectory_new(struct mwAddressBook *book);


enum mwDirectoryState mwDirectory_getState(struct mwDirectory *dir);


/** set client data. If there is an existing clear function, it will
    not be called */
void mwDirectory_setClientData(struct mwDirectory *dir,
			       gpointer data, GDestroyNotify clear);


/** reference associated client data */
gpointer mwDirectory_getClientData(struct mwDirectory *dir);


/** remove and cleanup user data */
void mwDirectory_removeClientData(struct mwDirectory *dir);


/** reference owning service */
struct mwServiceDirectory *mwDirectory_getService(struct mwDirectory *dir);


/** reference owning address book */
struct mwAddressBook *mwDirectory_getAddressBook(struct mwDirectory *dir);


/** initialize a directory. */
int mwDirectory_open(struct mwDirectory *dir, mwSearchHandler cb);


/** continue a search into its next results */
int mwDirectory_next(struct mwDirectory *dir);


/** continue a search into its previous results */
int mwDirectory_previous(struct mwDirectory *dir);


/** initiate a search on an open directory */
int mwDirectory_search(struct mwDirectory *dir, const char *query);


/** close and free the directory, and unassociate it with its owning
    address book and service */
int mwDirectory_destroy(struct mwDirectory *dir);


#ifdef __cplusplus
}
#endif


#endif /* _MW_SRVC_DIR_H */
