
#if defined(OS400) && (__OS400_TGTVRM__ >= 510)
#pragma datamodel(P128)
#endif

/*********************************************************************/
/*                                                                   */
/* Licensed Materials - Property of IBM                              */
/*                                                                   */
/* L-GHUS-5VMPGW, L-GHUS-5S3PEE                                      */
/* (C) Copyright IBM Corp. 1989, 2005  All Rights Reserved           */
/*                                                                   */
/* US Government Users Restricted Rights - Use, duplication or       */
/* disclosure restricted by GSA ADP Schedule Contract with           */
/* IBM Corp.                                                         */
/*                                                                   */
/*********************************************************************/



/*	NIF manipulation routines & basic datatypes */

#ifndef NIF_DEFS
#define NIF_DEFS

#ifndef NSF_DATA_DEFS
#include "nsfdata.h"					/* We need NOTEID */
#endif

#ifndef NSF_NOTE_DEFS
#include "nsfnote.h"					/* We need NOTE_CLASS_VIEW */
#endif

#ifndef POOL_DEFS
#include "pool.h"
#endif

#ifndef STD_NAME_DEFS
#include "stdnames.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define DB2NIF_VERSION 11

/*	Collection handle */

typedef WORD HCOLLECTION;				/* Handle to NIF collection */
#define	NULLHCOLLECTION ( (HCOLLECTION) 0 )


/*	NIFOpenCollection "open" flags */

#define OPEN_REBUILD_INDEX		0x0001	/* Throw away existing index and */
										/* rebuild it from scratch */
#define OPEN_NOUPDATE			0x0002	/* Do not update index or unread */
										/* list as part of open (usually */
										/* set by server when it does it */
										/* incrementally instead). */
#define OPEN_DO_NOT_CREATE		0x0004	/* If collection object has not yet */
										/* been created, do NOT create it */
										/* automatically, but instead return */
										/* a special internal error called */
										/* ERR_COLLECTION_NOT_CREATED */
#define OPEN_SHARED_VIEW_NOTE	0x0010	/* Tells NIF to "own" the view note */
										/* (which gets read while opening the */
										/* collection) in memory, rather than */
										/* the caller "owning" the view note */
										/* by default.  If this flag is specified */
										/* on subsequent opens, and NIF currently */
										/* owns a copy of the view note, it */
										/* will just pass back the view note */
										/* handle  rather than re-reading it */
										/* from disk/network.  If specified, */
										/* the the caller does NOT have to */
										/* close the handle.  If not specified, */
										/* the caller gets a separate copy, */
										/* and has to NSFNoteClose the */
										/* handle when its done with it. */
#define OPEN_REOPEN_COLLECTION	0x0020	/* Force re-open of collection and */
										/* thus, re-read of view note. */
										/* Also implicitly prevents sharing */
										/* of collection handle, and thus */
										/* prevents any sharing of associated */
										/* structures such as unread lists, etc */


/*	Collection navigation directives */

#define NAVIGATE_CURRENT			0	/* Remain at current position */
										/* (reset position & return data) */
#define NAVIGATE_PARENT				3	/* Up 1 level */
#define NAVIGATE_CHILD				4	/* Down 1 level to first child */
#define NAVIGATE_NEXT_PEER			5	/* Next node at our level */
#define NAVIGATE_PREV_PEER			6	/* Prev node at our level */
#define NAVIGATE_FIRST_PEER			7	/* First node at our level */
#define NAVIGATE_LAST_PEER			8	/* Last node at our level */
#define NAVIGATE_CURRENT_MAIN		11	/* Highest level non-category entry */
#define NAVIGATE_NEXT_MAIN			12	/* CURRENT_MAIN, then NEXT_PEER */
#define NAVIGATE_PREV_MAIN			13	/* CURRENT_MAIN, then PREV_PEER only if already there */
#define NAVIGATE_NEXT_PARENT		19	/* PARENT, then NEXT_PEER */
#define NAVIGATE_PREV_PARENT		20	/* PARENT, then PREV_PEER */

#define NAVIGATE_NEXT				1	/* Next entry over entire tree */
										/* (parent first, then children,...) */
#define NAVIGATE_PREV				9	/* Previous entry over entire tree */
										/* (opposite order of PREORDER) */
#define NAVIGATE_ALL_DESCENDANTS	17	/* NEXT, but only descendants */
										/* below NIFReadEntries StartPos */
#define NAVIGATE_NEXT_UNREAD		10	/* NEXT, but only "unread" entries */
#define NAVIGATE_NEXT_UNREAD_MAIN	18	/* NEXT_UNREAD, but stop at main note also */
#define NAVIGATE_PREV_UNREAD_MAIN	34	/* Previous unread main. */
#define NAVIGATE_PREV_UNREAD		21	/* PREV, but only "unread" entries */
#define NAVIGATE_NEXT_SELECTED		14	/* NEXT, but only "selected" entries */
#define NAVIGATE_PREV_SELECTED		22	/* PREV, but only "selected" entries */
#define NAVIGATE_NEXT_SELECTED_MAIN	32	/* Next selected main. (Next unread */
										/* main can be found above.) */
#define NAVIGATE_PREV_SELECTED_MAIN	33	/* Previous selected main. */
#define NAVIGATE_NEXT_EXPANDED		15	/* NEXT, but only "expanded" entries */
#define NAVIGATE_PREV_EXPANDED		16	/* PREV, but only "expanded" entries */
#define NAVIGATE_NEXT_EXPANDED_UNREAD 23 /* NEXT, but only "expanded" AND "unread" entries */
#define NAVIGATE_PREV_EXPANDED_UNREAD 24 /* PREV, but only "expanded" AND "unread" entries */
#define NAVIGATE_NEXT_EXPANDED_SELECTED 25 /* NEXT, but only "expanded" AND "selected" entries */
#define NAVIGATE_PREV_EXPANDED_SELECTED 26 /* PREV, but only "expanded" AND "selected" entries */
#define NAVIGATE_NEXT_EXPANDED_CATEGORY	27 /* NEXT, but only "expanded" AND "category" entries */
#define NAVIGATE_PREV_EXPANDED_CATEGORY	28 /* PREV, but only "expanded" AND "category" entries */
#define NAVIGATE_NEXT_EXP_NONCATEGORY	39	/* NEXT, but only "expanded" "non-category" entries */
#define NAVIGATE_PREV_EXP_NONCATEGORY	40	/* PREV, but only "expanded" "non-category" entries */
#define NAVIGATE_NEXT_HIT			29	/* NEXT, but only FTSearch "hit" entries */
										/* (in the SAME ORDER as the hit's relevance ranking) */
#define NAVIGATE_PREV_HIT			30	/* PREV, but only FTSearch "hit" entries */
										/* (in the SAME ORDER as the hit's relevance ranking) */
#define NAVIGATE_CURRENT_HIT		31	/* Remain at current position in hit's relevance rank array */
										/* (in the order of the hit's relevance ranking) */
#define NAVIGATE_NEXT_SELECTED_HIT	35	/* NEXT, but only "selected" and FTSearch "hit" entries */
										/* (in the SAME ORDER as the hit's relevance ranking) */
#define NAVIGATE_PREV_SELECTED_HIT	36	/* PREV, but only "selected" and FTSearch "hit" entries */
										/* (in the SAME ORDER as the hit's relevance ranking) */
#define NAVIGATE_NEXT_UNREAD_HIT	37	/* NEXT, but only "unread" and FTSearch "hit" entries */
										/* (in the SAME ORDER as the hit's relevance ranking) */
#define NAVIGATE_PREV_UNREAD_HIT	38	/* PREV, but only "unread" and FTSearch "hit" entries */
										/* (in the SAME ORDER as the hit's relevance ranking) */
#define NAVIGATE_NEXT_CATEGORY		41	/* NEXT, but only "category" entries */
#define NAVIGATE_PREV_CATEGORY		42	/* PREV, but only "category" entries */
#define NAVIGATE_NEXT_NONCATEGORY	43	/* NEXT, but only "non-category" entries */
#define NAVIGATE_PREV_NONCATEGORY	44	/* PREV, but only "non-category" entries */

#define NAVIGATE_MASK			0x007F	/* Navigator code (see above) */


/*	Flag which can be used with ALL navigators which causes the navigation
	to be limited to entries at a specific level (specified by the
	field "MinLevel" in the collection position) or any higher levels
	but never a level lower than the "MinLevel" level.  Note that level 0
	means the top level of the index, so the term "minimum level" really
	means the "highest level" the navigation can move to.
	This can be used to find all entries below a specific position
	in the index, limiting yourself only to that subindex, and yet be
	able to use any of the navigators to move around within that subindex.
	This feature was added in Version 4 of Notes, so it cannot be used
	with earlier Notes Servers. */

#define NAVIGATE_MINLEVEL		0x0100	/* Honor "Minlevel" field in position */
#define NAVIGATE_MAXLEVEL		0x0200	/* Honor "Maxlevel" field in position */

/*  This flag can be combined with any navigation directive to
	prevent having a navigation (Skip) failure abort the (ReadEntries) operation.
	For example, this is used by the Notes user interface when
	getting the entries to display in the view, so that if an attempt is made to
	skip past either end of the index (e.g. using PageUp/PageDown),
	the skip will be left at the end of the index, and the return will return
	whatever can be returned using the separate return navigator.

	This flag is also used to get the "last" N entries of a view by setting the
	Skip Navigator to NAVIGATE_NEXT | NAVIGATE_CONTINUE, setting the SkipCount to MAXDWORD,
	setting the ReturnNavigator to NAVIGATE_PREV_EXPANDED, and setting the ReturnCount
	to N (N must be greater than 0). */

#define NAVIGATE_CONTINUE		0x8000	/* "Return" even if "Skip" error */

/*	Structure which describes statistics about the overall collection,
	and can be requested using the READ_MASK_COLLECTIONSTATS flag.  If
	requested, this structure is returned at the beginning of the returned
	ReadEntries buffer. */

typedef struct
	{
	WORD	TopLevelEntries;			/* # top level entries (level 0) */
	WORD	spare[3];					/* 0 */
	} COLLECTIONSTATS16;

typedef struct
	{
	DWORD	TopLevelEntries;			/* # top level entries (level 0) */
	DWORD	spare;						/* 0 */
	} COLLECTIONSTATS;

/*	Structure which specifies collection index position. */

#define MAXTUMBLERLEVELS_V2 8			/* Max. levels in hierarchy tree in V2 */
#define MAXTUMBLERLEVELS 32				/* Max. levels in hierarchy tree */

typedef struct
	{
	WORD	Level;						/* # levels -1 in tumbler */
										/* (top level = 0) */
	WORD	Tumbler[MAXTUMBLERLEVELS_V2]; /* Current tumbler (1.2.3, etc) */
										/* (an array of ordinal ranks) */
										/* (0th entry = top level) */
	} COLLECTIONPOSITION16;

typedef struct
	{
	WORD	Level;						/* # levels -1 in tumbler */
										/* (top level = 0) */
	BYTE	MinLevel;					/* MINIMUM level that this position */
										/* is allowed to be nagivated to. */
										/* This is useful to navigate a */
										/* subtree using all navigator codes. */
										/* This field is IGNORED unless */
										/* the NAVIGATE_MINLEVEL flag is */
										/* enabled (for backward compat) */
	BYTE	MaxLevel;					/* MAXIMUM level that this position */
										/* is allowed to be nagivated to. */
										/* This is useful to navigate a */
										/* subtree using all navigator codes. */
										/* This field is IGNORED unless */
										/* the NAVIGATE_MAXLEVEL flag is */
										/* enabled (for backward compat) */
	DWORD	Tumbler[MAXTUMBLERLEVELS];	/* Current tumbler (1.2.3, etc) */
										/* (an array of ordinal ranks) */
										/* (0th entry = top level) */
										/* Actual number of array entries */
										/* is Level+1 */
	} COLLECTIONPOSITION;


/*	Macro which computes size of portion of COLLECTIONPOSITION structure
	which is actually used.  This is the size which is returned by
	NIFReadEntries when READ_MASK_INDEXPOSITION is specified. */

#define COLLECTIONPOSITIONSIZE16(p) (sizeof(WORD) * ((p)->Level+2))
#define COLLECTIONPOSITIONSIZE(p) (sizeof(DWORD) * ((p)->Level+2))
/*	NIFReadEntries return mask flags

	These flags specified what information is returned in the return
	buffer.  With the exception of READ_MASK_COLLECTIONSTATS, the
	information which corresponds to each of the flags in this mask
	are returned in the buffer, repeated for each index entry, in the
	order in which the bits are listed here.

	The return buffer consists of:

	1) COLLECTIONSTATS structure, if requested (READ_MASK_COLLECTIONSTATS).
		This structure is returned only once at the beginning of the
		buffer, and is not repeated for each index entry.

	2) Information about each index entry.  Each flag requested a different
		bit of information about the index entry.  If more than one flag
		is defined, the values follow each other, in the order in which
		the bits are listed here.  This portion repeats for as many
		index entries as are requested.
*/

	/*	Fixed length stuff */
#define READ_MASK_NOTEID			0x00000001L	/* NOTEID of entry */
#define READ_MASK_NOTEUNID			0x00000002L	/* UNID of entry */
#define READ_MASK_NOTECLASS			0x00000004L	/* WORD of "note class" */
#define READ_MASK_INDEXSIBLINGS		0x00000008L	/* DWORD/WORD of # siblings of entry */
#define READ_MASK_INDEXCHILDREN		0x00000010L	/* DWORD/WORD of # direct children of entry */
#define READ_MASK_INDEXDESCENDANTS	0x00000020L	/* DWORD/WORD of # descendants below entry */
#define READ_MASK_INDEXANYUNREAD	0x00000040L	/* WORD of TRUE if "unread" or */
											/* "unread" descendants; else FALSE */
#define READ_MASK_INDENTLEVELS		0x00000080L	/* WORD of # levels that this */
											/* entry should be indented in */
											/* a formatted view. */
	 										/* For category entries: */
											/*  # sub-levels that this */
											/*  category entry is within its */
											/*  Collation Descriptor.  Used */
											/*  for multiple-level category */
											/*  columns (backslash-delimited). */
											/*  "0" for 1st level in this column, etc. */
											/* For response entries: */
											/*  # levels that this response */
											/*  is below the "main note" level. */
											/* For normal entries: 0 */
#define READ_MASK_SCORE				0x00000200L	/* Relavence "score" of entry */
											/*  (only used with FTSearch). */
#define READ_MASK_INDEXUNREAD		0x00000400L /* WORD of TRUE if this entry (only) "unread" */


	/*	Stuff returned only once at beginning of return buffer */
#define READ_MASK_COLLECTIONSTATS	0x00000100L	/* Collection statistics (COLLECTIONSTATS/COLLECTIONSTATS16) */


	/*	Variable length stuff */
#define READ_MASK_INDEXPOSITION		0x00004000L	/* Truncated COLLECTIONPOSITION/COLLECTIONPOSITION16 */
#define READ_MASK_SUMMARYVALUES		0x00002000L	/* Summary buffer w/o item names */
#define READ_MASK_SUMMARY			0x00008000L	/* Summary buffer with item names */
/*	Structures which are used by NIFGetCollectionData to return data
	about the collection. NOTE: If the COLLECTIONDATA structure changes,
	nifods.c must change as well. */

/* Definitions which are used by NIFGetCollectionData to return data about the collection.  */

#define PERCENTILE_COUNT	11

#define PERCENTILE_0		0
#define PERCENTILE_10		1
#define PERCENTILE_20		2
#define PERCENTILE_30		3
#define PERCENTILE_40		4
#define PERCENTILE_50		5
#define PERCENTILE_60		6
#define PERCENTILE_70		7
#define PERCENTILE_80		8
#define PERCENTILE_90		9
#define PERCENTILE_100		10

typedef struct
	{
	DWORD		DocCount;	 	/* Total number of documents in the collection */
	DWORD		DocTotalSize;	/* Total number of bytes occupied by the documents in the collection */
	DWORD		BTreeLeafNodes;	/* Number of B-Tree leaf nodes for this index. */
	WORD		BTreeDepth;		/* Number of B-tree levels for this index. */
	WORD		Spare;			/* Unused */
	DWORD		KeyOffset[PERCENTILE_COUNT];  /* Offset of ITEM_VALUE_TABLE for each 10th-percentile key value */
				/* A series of ITEM_VALUE_TABLEs follows this structure. */
	} COLLECTIONDATA;



/*	Flag in index entry's NOTEID to indicate (ghost) "category entry" */
/*	Note: this relies upon the fact that NOTEID_RESERVED is high bit! */

#define NOTEID_CATEGORY			0x80000000L /* Bit 31 -> (ghost) "category entry" */
#define NOTEID_CATEGORY_TOTAL	0xC0000000L /* Bit 31+30 -> (ghost) "grand total entry" */
#define NOTEID_CATEGORY_INDENT	0x3F000000L	/* Bits 24-29 -> category indent level within this column */
#define NOTEID_CATEGORY_ID		0x00FFFFFFL	/* Low 24 bits are unique category # */


/*	SignalFlags word returned by NIFReadEntries and V4+ NIFFindByKey */

#define SIGNAL_DEFN_ITEM_MODIFIED 0x0001
									/* At least one of the "definition" */
									/* view items ($FORMULA, $COLLATION, */
									/* or $FORMULACLASS) has been modified */
									/* by another user since last ReadEntries. */
									/* Upon receipt, you may wish to */
									/* re-read the view note if up-to-date */
									/* copies of these items are needed. */
									/* Upon receipt, you may also wish to */
									/* re-synchronize your index position */
									/* and re-read the rebuilt index. */
									/* Signal returned only ONCE per detection */
#define SIGNAL_VIEW_ITEM_MODIFIED 0x0002
									/* At least one of the non-"definition" */
									/* view items ($TITLE,etc) has been */
									/* modified since last ReadEntries. */
									/* Upon receipt, you may wish to */
									/* re-read the view note if up-to-date */
									/* copies of these items are needed. */
									/* Signal returned only ONCE per detection */
#define SIGNAL_INDEX_MODIFIED	  0x0004
									/* Collection index has been modified */
									/* by another user since last ReadEntries. */
									/* Upon receipt, you may wish to */
									/* re-synchronize your index position */
									/* and re-read the modified index. */
									/* Signal returned only ONCE per detection */
#define SIGNAL_UNREADLIST_MODIFIED 0x0008
									/* Unread list has been modified */
									/* by another window using the same */
									/* hCollection context */
									/* Upon receipt, you may wish to */
									/* repaint the window if the window */
									/* contains the state of unread flags */
									/* (This signal is never generated */
									/*  by NIF - only unread list users) */
#define SIGNAL_DATABASE_MODIFIED  0x0010
									/* Collection is not up to date */
#define SIGNAL_MORE_TO_DO		  0x0020
									/* End of collection has not been reached */
									/* due to buffer being too full. */
									/* The ReadEntries should be repeated */
									/* to continue reading the desired entries. */
#define SIGNAL_VIEW_TIME_RELATIVE 0x0040
									/* The view contains a time-relative formula */
									/* (e.g., @Now).  Use this flag to tell if the */
									/* collection will EVER be up-to-date since */
									/* time-relative views, by definition, are NEVER */
									/* up-to-date. */
#define SIGNAL_NOT_SUPPORTED      0x0080
									/* Returned if signal flags are not supported */
									/* This is used by NIFFindByKeyExtended when it */
									/* is talking to a pre-V4 server that does not */
									/* support signal flags for FindByKey */

/*	Mask that defines all "sharing conflicts", which are cases when
	the database or collection has changed out from under the user. */

#define SIGNAL_ANY_CONFLICT	(SIGNAL_DEFN_ITEM_MODIFIED | \
							SIGNAL_VIEW_ITEM_MODIFIED | \
							SIGNAL_INDEX_MODIFIED | \
							SIGNAL_UNREADLIST_MODIFIED | \
							SIGNAL_DATABASE_MODIFIED)

/*	Mask that defines all "sharing conflicts" except for SIGNAL_DATABASE_MODIFIED.
	This can be used in combination with SIGNAL_VIEW_TIME_RELATIVE to tell if
	the database or collection has truly changed out from under the user or if the
	view is a time-relative view which will NEVER be up-to-date.  SIGNAL_DATABASE_MODIFIED
	is always returned for a time-relative view to indicate that it is never up-to-date. */

#define SIGNAL_ANY_NONDATA_CONFLICT	(SIGNAL_DEFN_ITEM_MODIFIED | \
									SIGNAL_VIEW_ITEM_MODIFIED | \
									SIGNAL_INDEX_MODIFIED | \
									SIGNAL_UNREADLIST_MODIFIED)

/*	NIFFindByKey "find" flags */

#define FIND_PARTIAL			0x0001	/* Match only initial characters */
										/* ("T" matches "Tim") */
#define FIND_CASE_INSENSITIVE	0x0002	/* Case insensitive */
										/* ("tim" matches "Tim") */
#define	FIND_RETURN_DWORD		0x0004	/* Input/Output is DWORD COLLECTIONPOSITION */
#define FIND_ACCENT_INSENSITIVE	0x0008	/* Accent insensitive (ignore diacritical marks */
#define	FIND_UPDATE_IF_NOT_FOUND	0x0020 /* If key is not found, update collection */
										   /* and search again */

/* At most one of the following four flags should be specified */
#define	FIND_LESS_THAN			0x0040	/* Find last entry less than the key value */
#define	FIND_FIRST_EQUAL		0x0000	/* Find first entry equal to the key value (if more than one) */
#define	FIND_LAST_EQUAL			0x0080	/* Find last entry equal to the key value (if more than one) */
#define	FIND_GREATER_THAN 		0x00C0	/* Find first entry greater than the key value */
#define	FIND_EQUAL		 		0x0800	/* Qualifies LESS_THAN and GREATER_THAN to mean */
										/* LESS_THAN_OR_EQUAL and GREATER_THAN_OR_EQUAL */
#define	FIND_COMPARE_MASK		0x08C0	/* Bitmask of the comparison flags defined above */

#define FIND_RANGE_OVERLAP		0x0100	/* Overlapping ranges match, and values within a range match */
#define	FIND_RETURN_ANY_NON_CATEGORY_MATCH	0x0200
											/* Return First Match at bottom level of
											Categorized view (Doesn't have
											to be first of duplicates */
#define FIND_NONCATEGORY_ONLY	0x0400	/* Only match non-category entries */

/*	NIF public entry points */

STATUS LNPUBLIC		NIFOpenCollection (DBHANDLE hViewDB, DBHANDLE hDataDB,
									NOTEID ViewNoteID, WORD OpenFlags,
									HANDLE hUnreadList,
									HCOLLECTION far *rethCollection,
									NOTEHANDLE far *rethViewNote, UNID far *retViewUNID,
									HANDLE far *rethCollapsedList,
									HANDLE far *rethSelectedList);
STATUS LNPUBLIC		NIFCloseCollection (HCOLLECTION hCollection);
STATUS LNPUBLIC		NIFUpdateCollection (HCOLLECTION hCollection);

STATUS LNPUBLIC		NIFOpenCollectionWithUserNameList (DBHANDLE hViewDB, DBHANDLE hDataDB,
									NOTEID ViewNoteID, WORD OpenFlags,
									HANDLE hUnreadList,
									HCOLLECTION far *rethCollection,
									NOTEHANDLE far *rethViewNote, UNID far *retViewUNID,
									HANDLE far *rethCollapsedList,
									HANDLE far *rethSelectedList,
									HANDLE nameList);
STATUS LNPUBLIC		NIFReadEntries (HCOLLECTION hCollection, COLLECTIONPOSITION far *IndexPos,
									WORD SkipNavigator, DWORD SkipCount,
									WORD ReturnNavigator, DWORD ReturnCount,
									DWORD ReturnMask,
									HANDLE far *rethBuffer, WORD far *retBufferLength,
									DWORD far *retNumEntriesSkipped,
									DWORD far *retNumEntriesReturned,
									WORD far *retSignalFlags);

STATUS LNPUBLIC  NIFSetCollation(HCOLLECTION hCollection, WORD CollationNum);
STATUS LNPUBLIC  NIFGetCollation(HCOLLECTION hCollection, WORD *retCollationNum);


STATUS LNPUBLIC  NIFLocateNote (HCOLLECTION hCollection, COLLECTIONPOSITION far *IndexPos, NOTEID NoteID);
STATUS LNPUBLIC		NIFFindByKey (HCOLLECTION hCollection, void far *KeyBuffer,
									WORD FindFlags,
									COLLECTIONPOSITION far *retIndexPos,
									DWORD far *retNumMatches);
STATUS LNPUBLIC		NIFFindByName (HCOLLECTION hCollection, const char far *Name, WORD FindFlags,
									COLLECTIONPOSITION far *retIndexPos,
									DWORD far *retNumMatches);

STATUS LNPUBLIC		NIFFindDesignNote (DBHANDLE hFile, const char far *Name, WORD Class, NOTEID far *retNoteID);
#define NIFFindView(hFile,Name,retNoteID) 			  NIFFindDesignNoteExt(hFile,Name,NOTE_CLASS_VIEW, DFLAGPAT_VIEWS_AND_FOLDERS, retNoteID, 0)
#define NIFFindDesignNoteByName(hFile,Name,retNoteID) NIFFindDesignNote(hFile,Name,NOTE_CLASS_ALL,retNoteID) /* Only for V2 backward compatibility */

STATUS LNPUBLIC		NIFFindPrivateDesignNote (DBHANDLE hFile, const char far *Name, WORD Class, NOTEID far *retNoteID);
#define NIFFindPrivateView(hFile,Name,retNoteID) NIFFindPrivateDesignNote(hFile,Name,NOTE_CLASS_VIEW,retNoteID)
STATUS LNPUBLIC		NIFGetCollectionData (HCOLLECTION hCollection,
										  HANDLE far *rethCollData);

#define	FIND_DESIGN_NOTE_PARTIAL	1   /* Find design note with a partial match.  This means that
										when you are looking for "Inbox" it will also
										match on "($Inbox)".  And when you are looking
										for "All Escalated Bugs" it will match
										"QA\All Escalated Bugs"  */

STATUS LNPUBLIC		NIFFindDesignNoteExt (DBHANDLE hFile, const char far *Name, WORD Class, const char *pszFlagsPattern, NOTEID far *retNoteID, DWORD Options);
void LNPUBLIC	 NIFGetLastModifiedTime (HCOLLECTION hCollection,
										TIMEDATE far *retLastModifiedTime);

#ifdef __cplusplus
}
#endif

#endif


#if defined(OS400) && (__OS400_TGTVRM__ >= 510)
#pragma datamodel(pop)
#endif

