/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	$Id: pgpGroups.h,v 1.12 2003/08/08 04:40:39 ajivsov Exp $
____________________________________________________________________________*/

#ifndef Included_pgpGroups_h	/* [ */
#define Included_pgpGroups_h

/*____________________________________________________________________________
	Note: All functions in this file have been deprecated and will be
	replaced in the future with equivalent functionality.
____________________________________________________________________________*/

#include "pgpPubTypes.h"
#include "pgpEncode.h"

#if PGP_DEPRECATED	/* [ */

#if PGP_MACINTOSH
#include <Files.h>
#endif

#if PGP_MACINTOSH
#pragma options align=mac68k
#endif

#define kPGPMaxGroupNameLength				63
#define kPGPMaxGroupDescriptionLength		63

typedef PGPChar8	PGPGroupName[ kPGPMaxGroupNameLength + 1 ];
typedef PGPChar8	PGPGroupDescription[ kPGPMaxGroupDescriptionLength + 1 ];


typedef struct PGPGroupSet *		PGPGroupSetRef;
typedef struct PGPGroupIter *		PGPGroupItemIterRef;

#define	kInvalidPGPGroupSetRef			((PGPGroupSetRef) NULL)
#define	kInvalidPGPGroupItemIterRef		((PGPGroupItemIterRef) NULL)

#define PGPGroupSetRefIsValid(ref)		((ref) != kInvalidPGPGroupSetRef)
#define PGPGroupItemIterRefIsValid(ref)	((ref) != kInvalidPGPGroupItemIterRef)

/* any type will do that is distinct */
typedef PGPUInt32				PGPGroupID;
#define kPGPInvalidGroupID		( (PGPGroupID)0 )

enum PGPGroupItemType_
{
	kPGPGroupItem_KeyID = 1,
	kPGPGroupItem_Group,
	
	PGP_ENUM_FORCE( PGPGroupItemType_)
};
PGPENUM_TYPEDEF( PGPGroupItemType_, PGPGroupItemType );

/*____________________________________________________________________________
	A run-time group item, used when iterating through a group.
	For client use; not necessarily the internal storage format.
	
	'userValue' is *not* saved to disk.
____________________________________________________________________________*/

typedef struct PGPGroupItem
{
	PGPGroupItemType	type;
	PGPUserValue		userValue;
	
	union
	{
		PGPGroupID	groupID;
		PGPKeyID	keyID;
	} u;
} PGPGroupItem;


typedef PGPInt32	(*PGPGroupItemCompareProc)( PGPGroupItem *,
							PGPGroupItem *, PGPUserValue userValue );

/*____________________________________________________________________________
	Info obtained via PGPGetGroupInfo.
____________________________________________________________________________*/

typedef struct PGPGroupInfo
{
	PGPGroupID		id;
	PGPGroupName	name;
	PGPGroupName	description;
	PGPUserValue	userValue;
	
} PGPGroupInfo;


typedef PGPFlags	PGPGroupItemIterFlags;
/* flag (1UL << 0 ) is reserved */
#define kPGPGroupIterFlags_Recursive	(PGPFlags)(1UL << 1 )
#define kPGPGroupIterFlags_Keys			(PGPFlags)(1UL << 2 )
#define kPGPGroupIterFlags_Groups		(PGPFlags)(1UL << 3 )

#define kPGPGroupIterFlags_AllKeysRecursive			\
	( kPGPGroupIterFlags_Recursive | kPGPGroupIterFlags_Keys )
	
#define kPGPGroupIterFlags_AllGroupsRecursive			\
	( kPGPGroupIterFlags_Recursive | kPGPGroupIterFlags_Groups )
	
#define kPGPGroupIterFlags_AllItems			\
	( kPGPGroupIterFlags_Keys | kPGPGroupIterFlags_Groups )
	
#define kPGPGroupIterFlags_AllRecursive	\
	( kPGPGroupIterFlags_Recursive | kPGPGroupIterFlags_AllItems )
	
	

#if PGP_MACINTOSH
#pragma options align=reset
#endif

PGP_BEGIN_C_DECLARATIONS

/*____________________________________________________________________________
	Manipulating pgp group sets (PGPGroupSetRef)
____________________________________________________________________________*/
/* create a new, empty groups collection */
PGPError	PGPNewGroupSet( PGPContextRef context, PGPGroupSetRef *outRef );

/* file is *not* left open; all data is loaded into memory */
PGPError	PGPNewGroupSetFromFile( PGPContextRef context,
				PGPFileSpecRef file,
				PGPGroupSetRef *outRef );

#if PGP_MACINTOSH || PGP_OSX
PGPError	PGPNewGroupSetFromFSSpec( PGPContextRef context,
				const struct FSSpec *spec, PGPGroupSetRef *outRef );
#endif

/* overwrites existing.  Don't bother unless PGPGroupSetNeedsCommit() */
PGPError	PGPSaveGroupSetToFile( PGPGroupSetRef set, PGPFileSpecRef file );

/* free all data structures; be sure to save first if you want */
PGPError	PGPFreeGroupSet( PGPGroupSetRef set );


/* has the group changed? */
PGPBoolean		PGPGroupSetNeedsCommit( PGPGroupSetRef set );

PGPContextRef	PGPGetGroupSetContext( PGPGroupSetRef set );
		
/* export the groupset to a buffer. Use PGPFreeData to free the buffer */
PGPError	PGPExportGroupSetToBuffer( PGPGroupSetRef set, void **buffer,
				PGPSize *bufferSize );

/* import a groupset from a buffer */
PGPError	PGPImportGroupSetFromBuffer(PGPContextRef context, void *buffer,
				PGPSize bufSize, PGPGroupSetRef *outSet );
		
/*____________________________________________________________________________
	Manipulating groups
	
	Groups are always referred to by IDs which remain valid until the set
	is disposed.
____________________________________________________________________________*/

/* initial parent ID is kPGPInvalidGroupID */
PGPError	PGPNewGroup( PGPGroupSetRef set,
				const PGPChar8 * name, const PGPChar8 *description,
				PGPGroupID *id );

PGPError	PGPCountGroupsInSet( PGPGroupSetRef set,
				PGPUInt32 *numGroups);
PGPError	PGPGetIndGroupID( PGPGroupSetRef set,
				PGPUInt32 groupIndex, PGPGroupID *id );

/* delete this group from the set */
/* All references to it are removed in all sets */
PGPError	PGPDeleteGroup( PGPGroupSetRef set, PGPGroupID id );

/* delete the indexed item from the group */
/* the item may be a group or a key */
PGPError	PGPDeleteIndItemFromGroup( PGPGroupSetRef set,
				PGPGroupID	id, PGPUInt32 item );

/* same as PGPDeleteIndItemFromGroup, but accepts an item */
PGPError	PGPDeleteItemFromGroup( PGPGroupSetRef set,
				PGPGroupID id, PGPGroupItem const *item );


PGPError	PGPGetGroupInfo( PGPGroupSetRef set,
				PGPGroupID id, PGPGroupInfo *info );
				
PGPError	PGPSetGroupName( PGPGroupSetRef set,
				PGPGroupID id, const PGPChar8 * name );
PGPError	PGPSetGroupUserValue( PGPGroupSetRef set,
				PGPGroupID id, PGPUserValue userValue );
PGPError	PGPSetGroupDescription( PGPGroupSetRef set,
				PGPGroupID id, const PGPChar8 * name );

/* 'item' specifies a group or a key id */
/* you must fill the item in completely */
PGPError	PGPAddItemToGroup( PGPGroupSetRef set,
				PGPGroupItem const *item, PGPGroupID group );


PGPError	PGPMergeGroupIntoDifferentSet( PGPGroupSetRef fromSet,
				PGPGroupID fromID, PGPGroupSetRef toSet );

PGPError	PGPMergeGroupSets( PGPGroupSetRef fromSet,
				PGPGroupSetRef intoSet );

PGPError	PGPCopyGroupSet(PGPGroupSetRef sourceSet,
				PGPGroupSetRef *destSet);
	
/*____________________________________________________________________________
	Manipulating group items
____________________________________________________________________________*/

/* count how many items there are in a group */
/* totalItems includes keys and groups */
PGPError	PGPCountGroupItems( PGPGroupSetRef set,
					PGPGroupID id, PGPBoolean recursive,
					PGPUInt32 * numKeys,
					PGPUInt32 * totalItems );

/* non-recursive call; index only applies to group itself */
PGPError	PGPGetIndGroupItem( PGPGroupSetRef set,
				PGPGroupID id, PGPUInt32 groupIndex, PGPGroupItem * item );

/* use PGPGetIndGroupItem() if you want to get the user value */
PGPError	PGPSetIndGroupItemUserValue( PGPGroupSetRef set,
				PGPGroupID id, PGPUInt32 groupIndex, PGPUserValue userValue );

PGPError	PGPSortGroupItems( PGPGroupSetRef set, PGPGroupID id,
				PGPGroupItemCompareProc, PGPUserValue userValue );
				
PGPError	PGPSortGroupSet( PGPGroupSetRef set,
				PGPGroupItemCompareProc, PGPUserValue userValue );

/*____________________________________________________________________________
	PGPGroupItemIterRef--iterator through group items.
	
	Special note: this is not a full-fledged iterator.  You may *not* add
	or delete items while iterating and you may only move forward.  However,
	you may change the values of items.
____________________________________________________________________________*/

PGPError	PGPNewGroupItemIter( PGPGroupSetRef set, PGPGroupID id,
				PGPGroupItemIterFlags flags, PGPGroupItemIterRef *iter );
				
PGPError	PGPFreeGroupItemIter( PGPGroupItemIterRef iter );

/* returns kPGPError_EndOfIteration when done */
PGPError	PGPGroupItemIterNext( PGPGroupItemIterRef iter,
				PGPGroupItem * item );

/*____________________________________________________________________________
	Group utilities
____________________________________________________________________________*/

/*____________________________________________________________________________
	Return the lowest validity of any item in the group
	keyset should contain all keys available
	It is not an error if keys can't be found; you may want to check
	the not found count.
	
	The lowest validity is kPGPValidity_Invalid and kPGPValidity_Unknown
	is never returned.
____________________________________________________________________________*/
PGPError	PGPGetGroupLowestValidity( PGPGroupSetRef set, PGPGroupID id,
				PGPKeyDBRef keyDB, PGPValidity * lowestValidity,
				PGPUInt32 * numKeysNotFound);

/*____________________________________________________________________________
	All all the keys in the group (and its subgroups) to the keyset
____________________________________________________________________________*/
PGPError	PGPNewKeySetFromGroup( PGPGroupSetRef set, PGPGroupID id,
				PGPKeyDBRef keyDB, PGPKeySetRef * resultSet,
				PGPUInt32 * numKeysNotFound);

/*____________________________________________________________________________
	Create a simple, flattened group of unique key IDs from the source group.
	Note that sourceSet and destSet must be different.
____________________________________________________________________________*/
PGPError	PGPNewFlattenedGroupFromGroup(PGPGroupSetRef sourceSet,
				PGPGroupID sourceID, PGPGroupSetRef destSet,
				PGPGroupID *destID);

/*____________________________________________________________________________
	Perform a "standard" sort on a group
____________________________________________________________________________*/
PGPError	PGPSortGroupSetStd( PGPGroupSetRef set, PGPKeyDBRef keydb );
				

PGP_END_C_DECLARATIONS

#endif	/* ] PGP_DEPRECATED */

#endif /* ] Included_pgpGroups_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/








