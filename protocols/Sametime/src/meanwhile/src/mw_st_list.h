
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

#ifndef _MW_ST_LIST_H
#define _MW_ST_LIST_H


/** @file mw_st_list.h

    Parse and compose buddy lists in the format commonly used by Sametime
    Connect clients.
*/


#include <glib.h>
#include <glib/glist.h>
#include "mw_common.h"


#ifdef __cplusplus
extern "C" {
#endif


#define ST_LIST_MAJOR  3
#define ST_LIST_MINOR  1
#define ST_LIST_MICRO  3


enum mwSametimeGroupType {
  mwSametimeGroup_NORMAL  = 1,  /**< a normal group of users */
  mwSametimeGroup_DYNAMIC = 2,  /**< a server-side group */
  mwSametimeGroup_UNKNOWN = 0,  /**< error determining group type */
};


enum mwSametimeUserType {
  mwSametimeUser_NORMAL   = 1,  /**< user on same community */
  mwSametimeUser_EXTERNAL = 2,  /**< external user */
  mwSametimeUser_UNKNOWN  = 0,  /**< error determining user type */
};


/** @struct mwSametimeList

    Represents a group-based buddy list. */
struct mwSametimeList;


/** @struct mwSametimeGroup

    Represents a group in a buddy list */
struct mwSametimeGroup;


/** @struct mwSametimeUser

    Represents a user in a group in a buddy list */
struct mwSametimeUser;


/** Create a new list */
struct mwSametimeList *mwSametimeList_new();


/** Free the list, all of its groups, and all of the groups' members */
void mwSametimeList_free(struct mwSametimeList *l);


/** Load a sametime list from a buffer. The list must be encapsulated
    as a string (eg, the first two bytes in the buffer should be the
    length of the string) */
void mwSametimeList_get(struct mwGetBuffer *b, struct mwSametimeList *l);


/** Write a sametime list onto a buffer. The list will be encapsulated
    in a string (the first two bytes written will be the length of the
    rest of the written list data) */
void mwSametimeList_put(struct mwPutBuffer *b, struct mwSametimeList *l);


/** convert a plain string into a sametime list */
struct mwSametimeList *mwSametimeList_load(const char *str);


/** convert a sametime list into a string */
char *mwSametimeList_store(struct mwSametimeList *l);


void mwSametimeList_setMajor(struct mwSametimeList *l, guint v);


guint mwSametimeList_getMajor(struct mwSametimeList *l);


void mwSametimeList_setMinor(struct mwSametimeList *l, guint v);


guint mwSametimeList_getMinor(struct mwSametimeList *l);


void mwSametimeList_setMicro(struct mwSametimeList *l, guint v);


guint mwSametimeList_getMicro(struct mwSametimeList *l);


/** Get a GList snapshot of the groups in a list */
GList *mwSametimeList_getGroups(struct mwSametimeList *l);


struct mwSametimeGroup *
mwSametimeList_findGroup(struct mwSametimeList *l,
			 const char *name);


/** Create a new group in a list */
struct mwSametimeGroup *
mwSametimeGroup_new(struct mwSametimeList *l,
		    enum mwSametimeGroupType type,
		    const char *name);


/** Remove a group from its list, and free it. Also frees all users
    contained in the group */
void mwSametimeGroup_free(struct mwSametimeGroup *g);


enum mwSametimeGroupType mwSametimeGroup_getType(struct mwSametimeGroup *g);


const char *mwSametimeGroup_getName(struct mwSametimeGroup *g);


void mwSametimeGroup_setAlias(struct mwSametimeGroup *g,
			      const char *alias);


const char *mwSametimeGroup_getAlias(struct mwSametimeGroup *g);


void mwSametimeGroup_setOpen(struct mwSametimeGroup *g, gboolean open);


gboolean mwSametimeGroup_isOpen(struct mwSametimeGroup *g);


struct mwSametimeList *mwSametimeGroup_getList(struct mwSametimeGroup *g);


/** Get a GList snapshot of the users in a list */
GList *mwSametimeGroup_getUsers(struct mwSametimeGroup *g);


struct mwSametimeUser *
mwSametimeGroup_findUser(struct mwSametimeGroup *g,
			 struct mwIdBlock *user);


/** Create a user in a group */
struct mwSametimeUser *
mwSametimeUser_new(struct mwSametimeGroup *g,
		   enum mwSametimeUserType type,
		   struct mwIdBlock *user);


/** Remove user from its group, and free it */
void mwSametimeUser_free(struct mwSametimeUser *u);


struct mwSametimeGroup *mwSametimeUser_getGroup(struct mwSametimeUser *u);


enum mwSametimeUserType mwSametimeUser_getType(struct mwSametimeUser *u);


const char *mwSametimeUser_getUser(struct mwSametimeUser *u);


const char *mwSametimeUser_getCommunity(struct mwSametimeUser *u);


void mwSametimeUser_setShortName(struct mwSametimeUser *u, const char *name);


const char *mwSametimeUser_getShortName(struct mwSametimeUser *u);


void mwSametimeUser_setAlias(struct mwSametimeUser *u, const char *alias);


const char *mwSametimeUser_getAlias(struct mwSametimeUser *u);



#ifdef __cplusplus
}
#endif


#endif /* _MW_ST_LIST_H */
