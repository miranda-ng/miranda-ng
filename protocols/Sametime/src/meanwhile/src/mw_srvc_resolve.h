
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

#ifndef _MW_SRVC_RESOLVE_H
#define _MW_SRVC_RESOLVE_H


#include <glib.h>
#include <glib/glist.h>


#ifdef __cplusplus
extern "C" {
#endif


/** Type identifier for the conference service */
#define mwService_RESOLVE  0x00000015


/** Return value of mwServiceResolve_search indicating an error */
#define SEARCH_ERROR  0x00


/** @struct mwServiceResolve
    User lookup service */
struct mwServiceResolve;


enum mwResolveFlag {
  /** return unique results or none at all */
  mwResolveFlag_UNIQUE    = 0x00000001,

  /** return only the first result */
  mwResolveFlag_FIRST     = 0x00000002,

  /** search all directories, not just the first with a match */
  mwResolveFlag_ALL_DIRS  = 0x00000004,

  /** search for users */
  mwResolveFlag_USERS     = 0x00000008,

  /** search for groups */
  mwResolveFlag_GROUPS    = 0x00000010,
};


/** @see mwResolveResult */
enum mwResolveCode {
  /** successful search */
  mwResolveCode_SUCCESS     = 0x00000000,

  /** only some of the nested searches were successful */
  mwResolveCode_PARTIAL     = 0x00010000,

  /** more than one result (occurs when mwResolveFlag_UNIQUE is used
      and more than one result would have been otherwise returned) */
  mwResolveCode_MULTIPLE    = 0x80020000,

  /** the name is not resolvable due to its format */
  mwResolveCode_BAD_FORMAT  = 0x80030000,
};


enum mwResolveMatchType {
  mwResolveMatch_USER   = 0x00000001,
  mwResolveMatch_GROUP  = 0x00000002,
};


struct mwResolveMatch {
  char *id;      /**< user id */
  char *name;    /**< user name */
  char *desc;    /**< description */
  guint32 type;  /**< @see mwResolveMatchType */
};


struct mwResolveResult {
  guint32 code;    /**< @see mwResolveCode */
  char *name;      /**< name of the result */
  GList *matches;  /**< list of mwResolveMatch */
};


/** Handle the results of a resolve request. If there was a cleanup
    function specified to mwServiceResolve_search, it will be called
    upon the user data after this callback returns.

    @param srvc     the resolve service
    @param id       the resolve request ID
    @param code     return code
    @param results  list of mwResolveResult
    @param data     optional user data attached to the request
*/
typedef void (*mwResolveHandler)
     (struct mwServiceResolve *srvc,
      guint32 id, guint32 code, GList *results,
      gpointer data);


/** Allocate a new resolve service */
struct mwServiceResolve *mwServiceResolve_new(struct mwSession *);


/** Inisitate a resolve request.

    @param srvc     the resolve service
    @param queries  list query strings
    @param flags    search flags
    @param handler  result handling function
    @param data     optional user data attached to the request
    @param cleanup  optional function to clean up user data
    @return         generated ID for the search request, or SEARCH_ERROR
*/
guint32 mwServiceResolve_resolve(struct mwServiceResolve *srvc,
				 GList *queries, enum mwResolveFlag flags,
				 mwResolveHandler handler,
				 gpointer data, GDestroyNotify cleanup);


/** Cancel a resolve request by its generated ID. The handler function
    will not be called, and the optional cleanup function will be
    called upon the optional user data for the request */
void mwServiceResolve_cancelResolve(struct mwServiceResolve *, guint32);


/// Miranda NG adaptation start - new method
struct mwService *mwServiceResolve_getService(struct mwServiceResolve *srvc);
/// Miranda NG adaptation end


#ifdef __cplusplus
}
#endif


#endif /* _MW_SRVC_RESOLVE_H */
