
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


#ifndef NSF_DATA_DEFS
#define NSF_DATA_DEFS

#include "pool.h"

#ifdef __cplusplus
extern "C" {
#endif
/* Note Storage File Data Definitions */

/*	Define some synonyms for self-documentation of templates */

#define DBHANDLE HANDLE
#define NOTEHANDLE HANDLE
#define ITEMDEFTABLEHANDLE HANDLE
#define ITEMDEFTABLEEXTHANDLE HANDLE
#define FORMULAHANDLE HANDLE
typedef void far * HCOMPUTE;

/*	Reserved NoteIDs */

#define NOTEID_RESERVED		0x80000000L		/*	Reserved Note ID, used for
												categories in NIFReadEntries
												and for deleted notes in a
												lot of interfaces. */
#define NOTEID_ADD			0x00000000L		/*	Reserved NoteID used as input
												to NoteUpdate, to add a new
												note (gets error if UNID assigned
												to new note already exists). */
#define NOTEID_ADD_OR_REPLACE 0x80000000L	/*	Reserved NoteID used as input
												to NoteUpdate, to update if
												note UNID already exists, or
												add note if doesn't exist. */
#define NOTEID_ADD_UNID		0x80000001L		/*	Reserved NoteID used as input
												to NoteUpdate.  Try to preserve
												the specified note UNID, but if
												it already exists, assign a new
												one.  (Note that the UNID in the
												hNote IS updated.) */
#define NOTEID_NULL_FOLDER	0x00000000L		/*	Used for null folder ids. */

/*	An RRV "file position" is defined to be a DWORD, 4 bytes long. */

#define RRV_ALIGNMENT 4L			/* most typical RRV alignment (DBTABLE.C) */
#define	RRV_DELETED NOTEID_RESERVED	/* indicates a deleted note (DBTABLE.C) */

#define	NOTEID_NO_PARENT	0x00000000L		/*	Reserved Note ID, used to indicate
												that this note has no parent in the
												response hierarchy. */

/*	This is the structure that identifies a database.  It is used for both
	the creation date/time and the originator date/time. */

typedef TIMEDATE DBID;

/*	This is the structure that identifies a note within a database.  It is
	simply a file position (RRV) that is guaranteed never to change WITHIN
	this file.  A replicated note, however, may have a different note id
	in two separate files. */

typedef DWORD NOTEID;

/*	This is the structure that identifies ALL replicas of the same note.  The
	"File" member contains a totally unique (random) number, generated at
	the time the note is created.  The "Note" member contains the date/time
	when the very first copy of the note was stored into the first NSF.  The
	"Sequence" member is a sequence number used to keep track of the most
	recent version of the note for replicated data purposes.  The
	"SequenceTime" member is a sequence number qualifier, that allows the
	replicator to determine which note is later given identical Sequence's.
	Both are required for the following reason.  The sequence number is needed
	to prevent someone from locking out future edits by setting the time/date
	to the future.  The sequence time qualifies the sequence number for two
	reasons:  1)  It prevents two concurrent updates from looking like
	no update at all and 2) it forces all systems to reach the same decision
	as to which update is the "latest" version.

	Time/dates associated with notes:

		OID.Note				Can be Timedate when the note was created
								(but not guaranteed to be - look for $CREATED
								item first for note creation time)
								Obtained by NSFNoteGetInfo(_NOTE_OID) or
								OID in SEARCH_MATCH.
		OID.SequenceTime		Timedate of last revision
								Obtained by NSFNoteGetInfo(_NOTE_OID) or
								OID in SEARCH_MATCH.
		NOTE.EditModified		Timedate when added to (or last updated in)
								this database.
								(Obtained by NSFNoteGetInfo(_NOTE_MODIFIED) or
								ID.Note in SEARCH_MATCH.


*/

#define MAXSEQNO			0x00FFFFFFL	/* Maximum sequence number for a note */
#define	OID_SEQNO_MASK		0x00FFFFFFL	/* Mask used to extract sequence # */
#define	OID_NO_REPLICATE	0x80000000L	/* Never replicate outward, currently used ONLY for deleted stubs */

typedef struct ORIGINATORID_tag {
	DBID File;					/* Unique (random) number */
								/* (Even though this field is called "File", */
								/*  it doesn't have anything to do with the file!) */
	TIMEDATE Note;				/* Can be Original Note Creation time/date */
								/*	(see OID.Note comment above) */
								/* (THE ABOVE 2 FIELDS MUST BE FIRST - UNID */
								/*  COPIED FROM HERE ASSUMED AT OFFSET 0) */
	DWORD Sequence;				/* LOW ORDER: sequence number, 1 for first version */
								/* HIGH ORDER WORD: flags, as above */
	TIMEDATE SequenceTime;		/* time/date when sequence number was bumped */
} ORIGINATORID;					/* 28 bytes */

#define OID ORIGINATORID


/*  Replication flags

	NOTE:  Please note the distinction between REPLFLG_DISABLE and
	REPLFLG_NEVER_REPLICATE.  The former is used to temporarily disable
	replication.  The latter is used to indicate that this database should
	NEVER be replicated.  The former may be set and cleared by the Notes
	user interface.  The latter is intended to be set programmatically
	and SHOULD NEVER be able to be cleared by the user interface.

	The latter was invented to avoid having to set the replica ID to
	the known value of REPLICA_ID_NEVERREPLICATE.  This latter method has
	the failing that DBs that use it cannot have DocLinks to them.  */

/*								0x0001	spare was COPY_ACL */
/*								0x0002	spare */
#define REPLFLG_DISABLE			0x0004	/* Disable replication */
#define REPLFLG_UNREADIFFNEW	0x0008	/* Mark unread only if newer note */
#define REPLFLG_IGNORE_DELETES	0x0010	/* Don't propagate deleted notes when
											replicating from this database */
#define REPLFLG_HIDDEN_DESIGN	0x0020	/* UI does not allow perusal of Design */
#define REPLFLG_DO_NOT_CATALOG	0x0040	/* Do not list in catalog */
#define	REPLFLG_CUTOFF_DELETE	0x0080	/* Auto-Delete documents prior to cutoff date */
#define REPLFLG_NEVER_REPLICATE	0x0100	/* DB is not to be replicated at all */
#define REPLFLG_ABSTRACT		0x0200	/* Abstract during replication */
#define REPLFLG_DO_NOT_BROWSE	0x0400	/* Do not list in database add */
#define REPLFLG_NO_CHRONOS		0x0800	/* Do not run chronos on database */
#define REPLFLG_IGNORE_DEST_DELETES	0x1000	/* Don't replicate deleted notes
											 into destination database */
#define REPLFLG_MULTIDB_INDEX	0x2000	/* Include in Multi Database indexing */
#define REPLFLG_PRIORITY_LOW	0xC000	/* Low priority */
#define REPLFLG_PRIORITY_MED	0x0000	/* Medium priority */
#define REPLFLG_PRIORITY_HI		0x4000	/* High priority */
#define REPLFLG_PRIORITY_SHIFT	14		/* Shift count for priority field */
#define REPLFLG_PRIORITY_MASK	0x0003	/* Mask for priority field after shifting*/
#define REPLFLG_PRIORITY_INVMASK 0x3fff	/* Mask for clearing the field */
#define REPLFLG_USED_MASK		(0x4|0x8|0x10|0x40|0x80|0x100|0x200|0xC000|0x1000|0x2000|0x4000)


/*  Replication priority values are stored in the high bits of the
	replication flags.  The stored value is biased by -1 so that
	an encoding of 0 represents medium priority (-1 is low and +1 is high).
	The following macros make getting and setting the priority easy.
	They return and accept normalized values of 0 - 2. */

#define REPL_GET_PRIORITY(Flags) \
	(((Flags >> REPLFLG_PRIORITY_SHIFT)+1) & REPLFLG_PRIORITY_MASK)
#define REPL_SET_PRIORITY(Pri) \
	(((Pri - 1) & REPLFLG_PRIORITY_MASK) << REPLFLG_PRIORITY_SHIFT)

/*  Reserved ReplicaID.Date.  Used in ID.Date field in ReplicaID to escape
	to reserved REPLICA_ID_xxx
*/
#define REPLICA_DATE_RESERVED		0		/* If used, see REPLICA_ID_xxx */

/*  Known Replica IDs.  Used in ID.Time field in ReplicaID.  Date
	subfield must be REPLICA_DATE_RESERVED).  NOTE:  If you add to this list,
	you should check the code in \catalog\search.c to see if the new one(s)
	need to be added to that code (probably not - but worth checking).

	The format is as follows.  Least sig. byte is version number.  2nd
	byte represents package code but is hard-coded to protect against
	changes in the package code.  Most sig. 2 bytes are reserved for future
	use.
*/
#define REPLICA_ID_UNINITIALIZED	0x00000000 /*  Uninitialized ID */
#define REPLICA_ID_CATALOG			0x00003301 /* Database Catalog (Version 2) */
#define REPLICA_ID_EVENT			0x00003302 /* Stats & Events Config DB */


/*  The following known replica ID is now obsolete.  Although the replicator
	still supports it, the problem is that DBs that use it cannot have
	DocLinks to them.  Instead use the replica flag REPLFLG_NEVER_REPLICATE. */
#define REPLICA_ID_NEVERREPLICATE	0x00001601 /* Do not allow replicas */

/*	Number of times within cutoff interval that we purge deleted stubs.
	For example, if the cutoff interval is 90 days, we purge every 30
	days. */

#define	CUTOFF_CHANGES_DURING_INTERVAL 3

/*	This is the structure that identifies a replica database. */

typedef struct {
	TIMEDATE ID;				/* ID that is same for all replica files */
	WORD Flags; 				/* Replication flags */
	WORD CutoffInterval;		/* Automatic Replication Cutoff Interval (Days) */
	TIMEDATE Cutoff;			/* Replication cutoff date */
} DBREPLICAINFO;


typedef struct {
	DWORD WarningThreshold; /* Database size warning threshold in kbyte units */
	DWORD SizeLimit;		/* Database size limit in kbyte units */
	DWORD CurrentDbSize;	/* Current size of database (in kbyte units) */
	DWORD MaxDbSize;		/* Max database file size possible (in kbyte units) */
} DBQUOTAINFO;

typedef struct {
	DWORD WarningThreshold; /* Database size warning threshold in kbyte units */
	DWORD SizeLimit;		/* Database size limit in kbyte units */
	DWORD CurrentDbSize;	/* Current size of database (in kbyte units) */
	DWORD MaxDbSize;		/* Max database file size possible (in kbyte units) */
	WORD QuotaMethod;		/* Enforcement method - filesize or usage */
	DWORD CurrentUsage;		/* Current amount of space used in the database (in kbyte units) */
	DWORD CurrentSizeUsed;	/* Either CurrentDbSize, or CurrentUsage, depending on method in use */
	DWORD Unused1;			/* Reserved.  Unused */
	DWORD Unused2;			/* Reserved.  Unused */
} DBQUOTAINFOEXT;


/*	This is the structure that globally identifies an INSTANCE of a note,
	that is, if we are doing a SEARCH_ALL_VERSIONS, the one with the
	latest modification date is the one that is the "most recent" instance. */

typedef struct {
	DBID File;					/* database Creation time/date */
	TIMEDATE Note;				/* note Modification time/date */
	NOTEID NoteID;				/* note ID within database */
} GLOBALINSTANCEID;

/*	This is the structure that universally (across all servers) describes
	a note (ALL INSTANCES of the same note), but without the information
	necessary to directly access the note in a given database.  It is used
	for referencing a specific note from another note (response notes and
	hot buttons are examples of its use) by storing this structure in the
	referencing note itself.  It is intended to work properly on any server,
	and even if the note being referenced is updated.  Matching of notes
	to other notes is done via the NIF machinery. */

typedef struct UNIVERSALNOTEID_tag {
	DBID File;					/* Unique (random) number */
								/* (Even though this field is called "File", */
								/*  it doesn't have anything to do with the file!) */
	TIMEDATE Note;				/* Can be Original Note Creation time/date */
								/*	(see OID.Note comment above) */
} UNIVERSALNOTEID;

#define UNID UNIVERSALNOTEID

/*	This is the structure that universally (across all servers) describes
	a note LINK. */

typedef struct {
	TIMEDATE File;					/* File's replica ID */
	UNID View;						/* View's Note Creation TIMEDATE */
	UNID Note;						/* Note's Creation TIMEDATE */
} NOTELINK;


/*	Data Type Definitions. */


/*	Class definitions.  Classes are defined to be the
	"generic" classes of data type that the internal formula computation
	mechanism recognizes when doing recalcs. */

#define	CLASS_NOCOMPUTE				(0 << 8)
#define	CLASS_ERROR					(1 << 8)
#define	CLASS_UNAVAILABLE			(2 << 8)
#define CLASS_NUMBER				(3 << 8)
#define CLASS_TIME					(4 << 8)
#define CLASS_TEXT					(5 << 8)
#define	CLASS_FORMULA				(6 << 8)
#define	CLASS_USERID				(7 << 8)

#define CLASS_MASK					0xff00

/*  All datatypes below are passed to NSF in either host (machine-specific
	byte ordering and padding) or canonical form (Intel 86 packed form).
	The format of each datatype, as it is passed to and from NSF functions,
	is listed below in the comment field next to each of the data types.
	(This host/canonical issue is NOT applicable to Intel86 machines,
	because on that machine, they are the same and no conversion is required).
	On all other machines, use the ODS subroutine package to perform
	conversions of those datatypes in canonical format before they can
	be interpreted. */

/*	"Computable" Data Types */

#define	TYPE_ERROR					0 + CLASS_ERROR			/* Host form */
#define	TYPE_UNAVAILABLE			0 + CLASS_UNAVAILABLE	/* Host form */
#define TYPE_TEXT					0 + CLASS_TEXT			/* Host form */
#define TYPE_TEXT_LIST				1 + CLASS_TEXT			/* Host form */
#define TYPE_NUMBER					0 + CLASS_NUMBER		/* Host form */
#define TYPE_NUMBER_RANGE			1 + CLASS_NUMBER		/* Host form */
#define TYPE_TIME					0 + CLASS_TIME			/* Host form */
#define TYPE_TIME_RANGE				1 + CLASS_TIME			/* Host form */
#define TYPE_FORMULA				0 + CLASS_FORMULA		/* Canonical form */
#define	TYPE_USERID					0 + CLASS_USERID		/* Host form */

/*	"Non-Computable" Data Types */

#define	TYPE_INVALID_OR_UNKNOWN		0 + CLASS_NOCOMPUTE		/* Host form */
#define	TYPE_COMPOSITE				1 + CLASS_NOCOMPUTE		/* Canonical form, >64K handled by more than one item of same name concatenated */
#define TYPE_COLLATION				2 + CLASS_NOCOMPUTE		/* Canonical form */
#define TYPE_OBJECT					3 + CLASS_NOCOMPUTE		/* Canonical form */
#define TYPE_NOTEREF_LIST			4 + CLASS_NOCOMPUTE		/* Host form */
#define TYPE_VIEW_FORMAT			5 + CLASS_NOCOMPUTE		/* Canonical form */
#define TYPE_ICON					6 + CLASS_NOCOMPUTE		/* Canonical form */
#define TYPE_NOTELINK_LIST			7 + CLASS_NOCOMPUTE		/* Host form */
#define TYPE_SIGNATURE				8 + CLASS_NOCOMPUTE		/* Canonical form */
#define TYPE_SEAL					9 + CLASS_NOCOMPUTE		/* Canonical form */
#define TYPE_SEALDATA				10 + CLASS_NOCOMPUTE	/* Canonical form */
#define TYPE_SEAL_LIST				11 + CLASS_NOCOMPUTE	/* Canonical form */
#define TYPE_HIGHLIGHTS				12 + CLASS_NOCOMPUTE	/* Host form */
#define TYPE_WORKSHEET_DATA			13 + CLASS_NOCOMPUTE	/* Used ONLY by Chronicle product */
															/* Canonical form */
#define TYPE_USERDATA				14 + CLASS_NOCOMPUTE	/* Arbitrary format data - see format below */
															/* Canonical form */
#define TYPE_QUERY					15 + CLASS_NOCOMPUTE	/* Saved query CD records; Canonical form */
#define TYPE_ACTION					16 + CLASS_NOCOMPUTE	/* Saved action CD records; Canonical form */
#define TYPE_ASSISTANT_INFO			17 + CLASS_NOCOMPUTE	/* Saved assistant info */
#define TYPE_VIEWMAP_DATASET		18 + CLASS_NOCOMPUTE	/* Saved ViewMap dataset; Canonical form */
#define TYPE_VIEWMAP_LAYOUT			19 + CLASS_NOCOMPUTE	/* Saved ViewMap layout; Canonical form */
#define TYPE_LSOBJECT				20 + CLASS_NOCOMPUTE	/* Saved LS Object code for an agent.	*/
#define	TYPE_HTML					21 + CLASS_NOCOMPUTE	/* LMBCS-encoded HTML, >64K handled by more than one item of same name concatenated */
#define TYPE_SCHED_LIST				22 + CLASS_NOCOMPUTE	/* Busy time schedule entries list; Host form */
#define TYPE_CALENDAR_FORMAT		24 + CLASS_NOCOMPUTE	/* Canonical form */
#define TYPE_MIME_PART				25 + CLASS_NOCOMPUTE	/* MIME body part; Canonical form */
#define TYPE_RFC822_TEXT			 2 + CLASS_TEXT			/* RFC822( RFC2047) message header; Canonical form */

/*	This is the structure used for summary buffers */

typedef struct {
	USHORT Length;				/* total length of this buffer */
	USHORT Items;				/* number of items in the table */
								/* now come the ITEMs */
								/* now comes the packed text */
} ITEM_TABLE;

typedef struct {				/* used for item names and values */
	USHORT NameLength;			/* length of the item's name */
	USHORT ValueLength;			/* length of the value field */
} ITEM;

typedef struct {
	USHORT Length;				/* total length of this buffer */
	USHORT Items;				/* number of items in the table */
								/* now comes an array of WORDS representing
									the lengths of the item names. */
								/* now comes the item names as packed text */
} ITEM_NAME_TABLE;

typedef struct {
	USHORT Length;				/* total length of this buffer */
	USHORT Items;				/* number of items in the table */
								/* now comes an array of WORDS representing
									the lengths of the item values. */
								/* now comes the item values as packed bytes */
} ITEM_VALUE_TABLE;

/*	Here is the format of the various LIST data types:

	TYPE_TEXT_LIST:

		LIST				* list header *
		USHORT ...			* array of text string lengths following *
		text				* now comes the packed text for all strings *

	TYPE_NUMBER_RANGE:

		RANGE				* range header *
		NUMBER ...			* array of NUMBERs *
		NUMBER_PAIR ...		* array of NUMBER_PAIRs *

	TYPE_TIME_RANGE:

		RANGE				* range header *
		TIMEDATE ...		* array of time/date's *
		TIMEDATE_PAIR ...	* array of time/date pairs *

	TYPE_NOTEREF_LIST:

		LIST				* list header *
		UNIVERSALNOTEID		* array of UNIVERSALNOTEIDs *


	TYPE_NOTELINK_LIST:

		LIST				* list header *
		NOTELINK			* array of NOTELINKs *

	TYPE_USERDATA:

		BYTE Length			* length of LMBCS "format-name" string *
		char[Length];		* LMBCS "format-name" string used to distinguish *
							* various formats of user data that follows. *
							* ("format-name" string is NOT NULL-TERMINATED!) *
		data				* next is variable-length data that corresponds *
							* to the format specified by the string *

*/

/*	This is the data structure used for TYPE_OBJECT values */

typedef struct {
	WORD ObjectType;			/* Type of object (OBJECT_xxx) */
	DWORD RRV;					/* Object ID of the object in THIS FILE */
} OBJECT_DESCRIPTOR;

/*	These must be OR-ed into the ObjectType below in order to get the
	desired behavior.  Note that OBJECT_COLLECTION implicitly has
	both of these bits implied, because that was the desired behavior
	before these bits were invented. */

#define	OBJECT_NO_COPY	0x8000	/* do not copy object when updating to new note or database */
#define	OBJECT_PRESERVE	0x4000	/* keep object around even if hNote doesn't have it when NoteUpdating */
#define	OBJECT_PUBLIC	0x2000	/* Public access object being allocated. */

/*	Object Types, a sub-category of TYPE_OBJECT */

#define	OBJECT_FILE					0		/* File Attachment */
#define OBJECT_FILTER_LEFTTODO 		3		/* IDTable of "done" docs attached to filter */
#define OBJECT_ASSIST_RUNDATA		8		/* Assistant run data object */
#define OBJECT_UNKNOWN	0xffff		/* Used as input to NSFDbGetObjectSize */

/*	File Attachment definitions */

#define HOST_MASK		0x0f00	/* used for NSFNoteAttachFile Encoding arg */
#define	HOST_MSDOS		(0 << 8)/* CRNL at EOL, optional ^Z at EOF */
#define	HOST_OLE		(1 << 8)/* unknown internal representation, up to app */
#define	HOST_MAC		(2 << 8)/* potentially has resource forks, etc. */
#define	HOST_UNKNOWN	(3 << 8)/* came inbound thru a gateway */
#define HOST_HPFS		(4 << 8)/* HPFS. Contains EAs and long filenames */
#define	HOST_OLELIB		(5 << 8)/* OLE 1 Library encapsulation */
#define HOST_BYTEARRAY_EXT	(6 << 8)/* OLE 2 ILockBytes byte array extent table */
#define HOST_BYTEARRAY_PAGE	(7 << 8)/* OLE 2 ILockBytes byte array page */
#define HOST_CDSTORAGE	(8 << 8)/* externally stored CD records */
#define HOST_STREAM		(9 << 8)/* Binary private stream */
#define HOST_LINK		(10 << 8)/* contains a RESOURCELINK to a named element */

#define	HOST_LOCAL		0x0f00	/* ONLY used as argument to NSFNoteAttachFile */
								/* means "use MY os's HOST_ type */

#define EFLAGS_MASK		0xf000	/* used for NSFNoteAttachFile encoding arg */
#define EFLAGS_INDOC	0x1000	/* used to pass FILEFLAG_INDOC flag to NSFNoteAttachFile */
#define EFLAGS_KEEPPATH 0x2000  /* don't strip off path in the filename */

/* changed below from 0x00ff to 0x000f to make room for flags defined below */
#define	COMPRESS_MASK	0x000f	/* used for NSFNoteAttachFile Encoding arg */
#define	COMPRESS_NONE	0		/* no compression */
#define	COMPRESS_HUFF	1		/* huffman encoding for compression */
#define	COMPRESS_LZ1	2		/* LZ1 compression */
#define	RECOMPRESS_HUFF	3		/* Huffman compression even if server supports LZ1 */

#define NTATT_FTYPE_MASK	0x0070	/*	File type mask */
#define NTATT_FTYPE_FLAT	0x0000	/*	Normal one fork file */
#define NTATT_FTYPE_MACBIN	0x0010	/*	MacBinaryII file */
#define NTATT_FTYPE_EBCDIC	0x0020	/*	EBCDIC flat file */
#define NTATT_NODEALLOC		0x0080	/*	Don't deallocate object when item is deleted */

#define	ATTRIB_READONLY	0x0001	/* file was read-only */
#define	ATTRIB_PRIVATE	0x0002	/* file was private or public */

#define	FILEFLAG_SIGN		0x0001	/* file object has object digest appended */
#define FILEFLAG_INDOC		0x0002	/* file is represented by an editor run in the document */
#define FILEFLAG_MIME		0x0004  /* file object has mime data appended */
									/* and NSFDbGetObjectInfo, NSFDbCopyObject. */
#define ENCODE_MASK			0x0038  /* file object has mime content transfer encoding */
#define ENCODE_NONE			(0 << 3)/* no encoding */
#define ENCODE_BASE64		(1 << 3)/* base64 encoding */
#define ENCODE_QP			(2 << 3)/* quoted-printable encoding */
#define ENCODE_UUENCODE		(3 << 3)/* x-uuencode encoding */
#define ENCODE_EXTENSION	(4 << 3)/* unknown extension encoding */

typedef struct {
	OBJECT_DESCRIPTOR Header;	/* object header */
	WORD FileNameLength;		/* length of file name */
	WORD HostType;		 		/* identifies type of text file delimeters (HOST_) */
	WORD CompressionType;		/* compression technique used (COMPRESS_) */
	WORD FileAttributes;		/* original file attributes (ATTRIB_) */
	WORD Flags;					/* miscellaneous flags (FILEFLAG_, ENCODE_) */
	DWORD FileSize;				/* original file size */
	TIMEDATE FileCreated;		/* original file date/time of creation, 0 if unknown */
	TIMEDATE FileModified;		/* original file date/time of modification */
								/* Now comes the file name... It is the original */
								/* RELATIVE file path with no device specifiers */
} FILEOBJECT;

typedef struct {
	char FileCreator[4];		/* application that created the file */
	char FileType[4];			/* type of file */
	DWORD ResourcesStart;		/* offset into the object at which resources begin */
	DWORD ResourcesLen;			/* length of the resources section in bytes */
	WORD CompressionType;		/* compression used for Mac resources */
	DWORD Spare;				/* 0 */
} FILEOBJECT_MACEXT;

typedef struct {
	DWORD EAStart;				/* offset into the object at which EAs begin */
	DWORD EALen;				/* length of EA section */
	DWORD Spare;				/* 0 */
} FILEOBJECT_HPFSEXT;


/* @SPECIAL Escape Codes */

#define	ESCBEGIN 0x7f
#define	ESCEND	 0xff

/*	Index information structure passed into NSFTranslateSpecial to provide
	index-related information for certain @INDEX functions, if specified. */


typedef struct
	{
	DWORD	IndexSiblings;		/* # siblings of entry */
	DWORD	IndexChildren;		/* # direct children of entry */
	DWORD	IndexDescendants;	/* # descendants of entry */
	WORD	IndexAnyUnread;		/* TRUE if entry "unread, or any descendants "unread" */
	} INDEXSPECIALINFO;

/*	Calendar busy time schedule list structure */

/* These are application ID's for the SCHED_LIST wApplicationID field.
** This is used to interpret the application specific UserAttr field.
** Notes ignores the UserAttr field, however application specific
** information can be returned by application specific gateways.
** If you need an ID, please register it with Lotus.
*/
#define SCHEDAPPLID_ORGANIZER2X	0x0001
#define SCHEDAPPLID_ORGANIZER4X	0x0002
#define SCHEDAPPLID_OV			0x0003


/* Define the maximum number of schedule entries allowed per note item.
** Calculated from maximum size of a note item (MAXONESEGSIZE) minus the size
** of a sched list,  divided by the size of a sched entry.  Subtract 100 of
** the number for some breathing room.
**
** The SCHED_LIST and SCHED_ENTRY are the preR6 data types.  For R6 we used
** the SCHED_LIST.Spare value to convey the length of the SCHED_ENTRY_EXT
** data that follows.  If .Spare is 0, the data that follows is SCHED_ENTRY,
** NOT SCHED_ENTRY_EXT.
** Now that we convey the length of each SCHED_ENTRY_EXT that follows (R6 and
** later), we can easily extend SCHED_ENTRY_EXT w/o adversely affecting
** previous clients since from R6 on the clients should be using
** SCHED_LIST.Spare to do pointer shifts.  For example, say in R7 we add
** even more info to SCHED_ENTRY_EXT, we simply append it to the R6 definition
** and adjust SCHED_LIST.Spare to reflect the new size.  That way an R6 client
** can get the info it needs/wants and can skip the 'newer' data that it
** has no way of properly using/processing.
**
** Note: If the SCHED_LIST.Spare is 0 then a SCHED_ENTRY MUST follow.
** If the SCHED_LIST.Spare is non-0 then a SCHED_ENTRY_EXT MUST follow.
**
** We are NOT allowing mixing of data types since it would only cause confusion
** for older users!  The system will return SCHED_LIST/SCHED_ENTRY data if it
** detects that the caller is using the preR6 API calls (or is using the R6
** API calls with a "preR6" flag to indicate old API call in use).
*/

#define MAXSCHEDLISTSIZE        (MAXONESEGSIZE - 100 * sizeof(SCHED_ENTRY))
#define MAXENTRIESPERSCHEDLIST  ((MAXSCHEDLISTSIZE - sizeof (SCHED_LIST)) / sizeof(SCHED_ENTRY))

typedef struct {
    ALIGNED_NUMBER  nLongitude;         /* Longitude coordinate value */
    ALIGNED_NUMBER  nLatitude;          /* Latitude coordinate value */
} GEO_INFO;

/* Scheduling list header */

typedef struct {
    DWORD   NumEntries;         /* Total number of schedule entries follow */
    WORD    wApplicationID;     /* application id for UserAttr interpretation */
    WORD    Spare;              /* PreR6: spare
                                ** R6: This now conveys the length of a single
                                ** SCHED_ENTRY_xxx that follows.  Use this value
                                ** to skip entries that MAY be larger (ie: R7
                                ** extends SCHED_ENTRY_EXT by appending values
                                ** that R6 does not know about so SCHED_ENTRY_xxx
                                ** would actually be larger than the R6
                                ** SCHED_ENTRY_EXT
                                */
                                /* Now come the schedule entries...
                                ** IFF Spare==0 then SCHED_ENTRYs follow
                                ** Otherwise Spare==the length of the
                                ** SCHED_ENTRY_EXTs that follow
                                */
} SCHED_LIST;

/* Scheduling busy time entry */

typedef struct {
    UNID            Unid;       /* UNID of the entry */
    TIMEDATE_PAIR   Interval;   /* Interval of the entry */
    BYTE            Attr;       /* SCHED_ATTR_xxx attributes defined by Notes */
    BYTE            UserAttr;   /* Application specific attributes */
    BYTE            spare[2];
} SCHED_ENTRY;

/* R6 scheduling busy time entry */

typedef struct {
    UNID            Unid;       /* UNID of the entry */
    TIMEDATE_PAIR   Interval;   /* Interval of the entry */
    BYTE            Attr;       /* SCHED_ATTR_xxx attributes defined by Notes */
    BYTE            UserAttr;   /* Application specific attributes */
    BYTE            spare[2];

    /* Everything above this point is the same as SCHED_ENTRY for preR6 clients!
    ** Everything from here on down is R6 (or later) only!
    */

    UNID            ApptUnid;   /* ApptUNID of the entry */
    DWORD           dwEntrySize;/* Size of this entry (for future ease of expansion) */
    GEO_INFO        GEOInfo;    /* Geographical coordinates of the entry */
} SCHED_ENTRY_EXT;

/* R6 Detailed busytime information is harvested and stored in a fairly
** compact manner thanks to length encoding of everything.  The format
** is basically:
**
**  SCHED_DETAIL_LIST (Detail list prefix header)
**  TEXT_LIST (of items that harvested and whose info follows)
**  SCHED_DETAIL_ENTRY (Actual harvested info per UNID)
**
** Since it may be desirable in the future to extend the detail format
** the SCHED_DETAIL_xxx structures are length encoded.  Any additional
** SCHED_DETAIL_LIST (or 'overhead') extensions MUST be inserted between
** the TEXT_LIST and the SCHED_DETAIL_ENTRY.  Any additional
** SCHED_DETAIL_ENTRY extensions MUST be inserted at the end of the
** structure so that they can easily be trimmed off by older clients.
**
** Minor clarification for those nitpickers out there, the SCHED_DETAIL_ENTRY
** is NOT just a single struct but is really composed of 2 parts, a
** 'prefix' that contains all info common to that detail (ie: entry
** UNID or flags) and then the actual data in an encoded form.  That
** form is described in greater detail below.  A hierarchical view
** of how the data is organzied (where indenting is used to
** semi-convey 'containment') when 3 items are harvested for 2 UNIDs
** would be:
**
**  SCHED_DETAIL_LIST (Detail list header)
**      TEXT_LIST (of 3 item names that were harvested)
**      SCHED_DETAIL_ENTRY (Actual harvested info for UNID1)
**          SCHED_DETAIL_DATA (Actual item data)
**          SCHED_DETAIL_DATA (Actual item data)
**          SCHED_DETAIL_DATA (Actual item data)
**      SCHED_DETAIL_ENTRY (Actual harvested info for UNID2)
**          SCHED_DETAIL_DATA (Actual item data)
**          SCHED_DETAIL_DATA (Actual item data)
**          SCHED_DETAIL_DATA (Actual item data)
**
*/

/* Flags used on SCHED_DETAIL_LIST */

#define SCHED_DETAIL_LIST_ATTR_NODATA           0x01 /* List has no data */
#define SCHED_DETAIL_LIST_ATTR_RESERVED2        0x02
#define SCHED_DETAIL_LIST_ATTR_RESERVED3        0x04
#define SCHED_DETAIL_LIST_ATTR_RESERVED4        0x08
#define SCHED_DETAIL_LIST_ATTR_RESERVED5        0x10
#define SCHED_DETAIL_LIST_ATTR_RESERVED6        0x20
#define SCHED_DETAIL_LIST_ATTR_RESERVED7        0x40
#define SCHED_DETAIL_LIST_ATTR_RESERVED8        0x80

/* R6 Schedule detail list header */

typedef struct {
    WORD        wHeaderLen;     /* Length of THIS header, in case it
                                ** ever grows, so that new items can be
                                ** easily skipped
                                */
    WORD        wEntryLen;      /* Length of THIS entire list and ALL of
                                ** its related data.
                                */
    WORD        wNumEntries;    /* Number of entries that follow */
    WORD        wOffsetItems;   /* Offset from list start to TEXT_LIST */
    WORD        wOffsetDetails; /* Offset from list start to SCHED_DETAIL_ENTRY */
    BYTE        Attr;           /* SCHED_DETAIL_LIST_ATTR_xxx attributes */
    BYTE        bReserved;      /* Reserved space/padding for ODS */

    /* Now comes the TEXT_LIST that corresponds to the item names
    ** and then comes the SCHED_DETAIL_ENTRY for each UNID
    */
} SCHED_DETAIL_LIST;

/* Flags used on SCHED_DETAIL_ENTRY */

#define SCHED_DETAIL_ENTRY_ATTR_PRIVATE         0x01    /* Entry is private */
#define SCHED_DETAIL_ENTRY_ATTR_RESERVED2       0x02
#define SCHED_DETAIL_ENTRY_ATTR_RESERVED3       0x04
#define SCHED_DETAIL_ENTRY_ATTR_RESERVED4       0x08
#define SCHED_DETAIL_ENTRY_ATTR_RESERVED5       0x10
#define SCHED_DETAIL_ENTRY_ATTR_RESERVED6       0x20
#define SCHED_DETAIL_ENTRY_ATTR_RESERVED7       0x40
#define SCHED_DETAIL_ENTRY_ATTR_RESERVED8       0x80

/* R6 Schedule detail list entry */

typedef struct {
    WORD        wPrefixLen;     /* Length of THIS prefix entry, in case it
                                ** ever grows, so that new items can be
                                ** easily skipped
                                */
    WORD        wEntryLen;      /* Length of THIS entire entry and ALL of
                                ** its related data.
                                */
    UNID        Unid;           /* UNID of the entry this is details of */
    WORD        wOffsetDetails; /* Offset from entry start to actual data */
    BYTE        Attr;           /* SCHED_DETAIL_ENTRY_ATTR_xxx attributes (TBD) */
    BYTE        bReserved;      /* Reserved space/padding for ODS */

    /* Now comes the data that corresponds to the item values (1 per item name)
    ** UNLESS dwEntryLen == wPrefixLen (which means NO details available
    ** for this UNID)
    */
} SCHED_DETAIL_ENTRY;

/* Flags used on SCHED_DETAIL_DATA */

#define SCHED_DETAIL_DATA_ATTR_TRUNCATED        0x01 /* Used to indicate value is trunated */
#define SCHED_DETAIL_DATA_ATTR_NOTFOUND         0x02 /* Item/value not found */
#define SCHED_DETAIL_DATA_ATTR_RESERVED3        0x04
#define SCHED_DETAIL_DATA_ATTR_RESERVED4        0x08
#define SCHED_DETAIL_DATA_ATTR_RESERVED5        0x10
#define SCHED_DETAIL_DATA_ATTR_RESERVED6        0x20
#define SCHED_DETAIL_DATA_ATTR_RESERVED7        0x40
#define SCHED_DETAIL_DATA_ATTR_RESERVED8        0x80

/* R6 Schedule detail list actual data */

typedef struct {
    WORD        wType;          /* Notes data type for the data */
    WORD        wDataLen;       /* Length of the data that immediately follows */
    BYTE        Attr;           /* SCHED_DETAIL_DATA_ATTR_xxx attributes */
    BYTE        bReserved;      /* Reserved space/padding for ODS */

    /* Now comes the actual data that corresponds to the item values */
} SCHED_DETAIL_DATA;

/* Since we have new structs for R6, we have to have new MAXSCHEDxxx values */

#define MAXSCHEDLISTEXTSIZE         ( MAXONESEGSIZE - 100 * sizeof(SCHED_ENTRY_EXT) )
#define MAXENTRIESPERSCHEDLISTEXT   ( ( MAXSCHEDLISTEXTSIZE - sizeof(SCHED_LIST) ) / sizeof(SCHED_ENTRY_EXT) )

/* The Attr field contains these bits.  Note that if bit3 is set
** then the entry will take up busy time.
** The lower nibble of the attributes defines the the entry type.
** NOTE: We are reserving the upper 4 bits of the Attr field for future use.

*/
#define SCHED_ATTR_FOREIGN_UNID 	0x10 /* Used by gateways to return foreign UNIDs */
#define SCHED_ATTR_REPEAT_EVENT 	0x20 /* Used by V5 C&S to identify new repeating meetings */
#define SCHED_ATTR_RESERVED4		0x40
#define SCHED_ATTR_RESERVED8    	0x80

/* these are the entry type bits */
#define SCHED_ATTR_TYPE_BITS    0x0F
#define SCHED_ATTR_FREE_BASE    0x00
#define SCHED_ATTR_BUSY_BASE    0x08

/* Entry types that don't block off busy time */
#define SCHED_ATTR_NULL             (SCHED_ATTR_FREE_BASE + 0x00)
#define SCHED_ATTR_PENCILED         (SCHED_ATTR_FREE_BASE + 0x01)
#define SCHED_ATTR_FREE_RESERVED2   (SCHED_ATTR_FREE_BASE + 0x02)
#define SCHED_ATTR_FREE_RESERVED3   (SCHED_ATTR_FREE_BASE + 0x03)
#define SCHED_ATTR_FREE_RESERVED4   (SCHED_ATTR_FREE_BASE + 0x04)
#define SCHED_ATTR_FREE_RESERVED5   (SCHED_ATTR_FREE_BASE + 0x05)
#define SCHED_ATTR_FREE_RESERVED6   (SCHED_ATTR_FREE_BASE + 0x06)
#define SCHED_ATTR_FREE_RESERVED7   (SCHED_ATTR_FREE_BASE + 0x07)

/* Entry types that block off busy time */
#define SCHED_ATTR_APPT             (SCHED_ATTR_BUSY_BASE + 0x00)
#define SCHED_ATTR_NONWORK          (SCHED_ATTR_BUSY_BASE + 0x01)
#define SCHED_ATTR_BUSY_RESERVED2   (SCHED_ATTR_BUSY_BASE + 0x02)
#define SCHED_ATTR_BUSY_RESERVED3   (SCHED_ATTR_BUSY_BASE + 0x03)
#define SCHED_ATTR_BUSY_RESERVED4   (SCHED_ATTR_BUSY_BASE + 0x04)
#define SCHED_ATTR_BUSY_RESERVED5   (SCHED_ATTR_BUSY_BASE + 0x05)
#define SCHED_ATTR_BUSY_RESERVED6   (SCHED_ATTR_BUSY_BASE + 0x06)
#define SCHED_ATTR_BUSY_RESERVED7   (SCHED_ATTR_BUSY_BASE + 0x07)


#define SCHED_ATTR_TYPE(attr) ((attr) & SCHED_ATTR_TYPE_BITS)
#define SCHED_ATTR_AVAILABLE(attr) (!((attr) & SCHED_ATTR_BUSY_BASE))


/* Function templates */


STATUS LNPUBLIC   NSFTranslateSpecial(void far *InputString,
							WORD InputStringLength,
							void far *OutputString,
							WORD OutputStringBufferLength,
							NOTEID NoteID,
							void far *IndexPosition,
							INDEXSPECIALINFO far *IndexInfo,
							HANDLE hUnreadList,
							HANDLE hCollapsedList,
							char far *FileTitle,
							char far *ViewTitle,
							WORD far *retLength);

/* End of Note Storage File Data Definitions */

#ifdef __cplusplus
}
#endif


#endif

#if defined(OS400) && (__OS400_TGTVRM__ >= 510)
#pragma datamodel(pop)
#endif

