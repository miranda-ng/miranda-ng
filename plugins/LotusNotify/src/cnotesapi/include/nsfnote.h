
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



#ifndef NSF_NOTE_DEFS
#define NSF_NOTE_DEFS


#ifndef NULLKFHANDLE
typedef void far * KFHANDLE;
#define NULLKFHANDLE ((KFHANDLE)0)
#endif

#ifndef NSF_DEFS
#include "nsfdata.h"
#endif

#ifndef POOL_DEFS
#include "pool.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Note Storage File Note Definitions */

/*	Item Flags */

#define	ITEM_SIGN						0x0001		/* This field will be signed if requested */
#define	ITEM_SEAL						0x0002		/* This field will be encrypted if requested */
#define	ITEM_SUMMARY					0x0004		/* This field can be referenced in a formula */
#define	ITEM_READWRITERS				0x0020		/* This field identifies subset of users that have read/write access */
#define	ITEM_NAMES						0x0040		/* This field contains user/group names */
#define	ITEM_PLACEHOLDER				0x0100		/* Simply add this item to "item name table", but do not store */
#define	ITEM_PROTECTED					0x0200		/* This field cannot be modified except by "owner" */
#define	ITEM_READERS					0x0400		/* This field identifies subset of users that have read access */
#define ITEM_UNCHANGED					0x1000		/* Item is same as on-disk  */

/*	If the following is ORed in with a note class, the resultant note ID
	may be passed into NSFNoteOpen and may be treated as though you first
	did an NSFGetSpecialNoteID followed by an NSFNoteOpen, all in a single
	transaction. */

#define	NOTE_ID_SPECIAL				0xFFFF0000L

/*	Note Classifications */
/*	If NOTE_CLASS_DEFAULT is ORed with another note class, it is in
	essence specifying that this is the default item in this class.  There
	should only be one DEFAULT note of each class that is ever updated,
	although nothing in the NSF machinery prevents the caller from adding
	more than one.  The file header contains a table of the note IDs of
	the default notes (for efficient access to them).  Whenever a note
	is updated that has the default bit set, the reference in the file
	header is updated to reflect that fact.
	WARNING: NOTE_CLASS_DOCUMENT CANNOT have a "default".  This is precluded
	by code in NSFNoteOpen to make it fast for data notes. 
*/

#define	NOTE_CLASS_DOCUMENT			0x0001		/* document note */
#define	NOTE_CLASS_DATA		NOTE_CLASS_DOCUMENT	/* old name for document note */
#define	NOTE_CLASS_INFO				0x0002		/* notefile info (help-about) note */
#define	NOTE_CLASS_FORM				0x0004		/* form note */
#define	NOTE_CLASS_VIEW				0x0008		/* view note */
#define NOTE_CLASS_ICON				0x0010		/* icon note */
#define NOTE_CLASS_DESIGN			0x0020		/* design note collection */
#define NOTE_CLASS_ACL				0x0040		/* acl note */
#define NOTE_CLASS_HELP_INDEX		0x0080		/* Notes product help index note */
#define	NOTE_CLASS_HELP				0x0100		/* designer's help note */
#define	NOTE_CLASS_FILTER			0x0200		/* filter note */
#define	NOTE_CLASS_FIELD			0x0400		/* field note */
#define NOTE_CLASS_REPLFORMULA		0x0800		/* replication formula */
#define NOTE_CLASS_PRIVATE			0x1000		/* Private design note, use $PrivateDesign view to locate/classify */


#define	NOTE_CLASS_DEFAULT			0x8000		/* MODIFIER - default version of each */

#define	NOTE_CLASS_NOTIFYDELETION	NOTE_CLASS_DEFAULT	/* see SEARCH_NOTIFYDELETIONS */
#define NOTE_CLASS_ALL				0x7fff		/* all note types */
#define NOTE_CLASS_ALLNONDATA		0x7ffe		/* all non-data notes */
#define	NOTE_CLASS_NONE				0x0000		/* no notes */

#define NC_MASK_DB2_VISIBLE			0x0FFF		/* all classes up to NOTE_CLASS_REPLFORMULA*/



/* Define symbol for those note classes that allow only one such in a file */

#define NOTE_CLASS_SINGLE_INSTANCE	( \
									NOTE_CLASS_DESIGN | \
									NOTE_CLASS_ACL | \
									NOTE_CLASS_INFO | \
									NOTE_CLASS_ICON | \
									NOTE_CLASS_HELP_INDEX | \
									0)

/*	Note flag definitions */

#define	NOTE_SIGNED					0x0001		/* signed */
#define	NOTE_ENCRYPTED				0x0002		/* encrypted */

/*	Open Flag Definitions.  These flags are passed to NSFNoteOpen. */

#define	OPEN_SUMMARY			0x0001	/* open only summary info */
#define	OPEN_NOVERIFYDEFAULT	0x0002	/* don't bother verifying default bit */
#define	OPEN_EXPAND				0x0004	/* expand data while opening */
#define	OPEN_NOOBJECTS			0x0008	/* don't include any objects */
#define OPEN_SHARE				0x0020	/* open in a "shared" memory mode */
#define OPEN_CANONICAL			0x0040	/* Return ALL item values in canonical form */
#define	OPEN_MARK_READ			0x0100	/* Mark unread if unread list is currently associated */
#define	OPEN_ABSTRACT			0x0200	/* Only open an abstract of large documents */
#define OPEN_RESPONSE_ID_TABLE	0x1000	/* Return response ID table */
#define OPEN_WITH_FOLDERS		0x00020000	/* Include folder objects - default is not to */
#define OPEN_RAW_RFC822_TEXT	0x01000000	/* If set, leave TYPE_RFC822_TEXT items in native
												format.  Otherwise, convert to TYPE_TEXT/TYPE_TIME. */
#define OPEN_RAW_MIME_PART		0x02000000	/* If set, leave TYPE_MIME_PART items in native
												format.  Otherwise, convert to TYPE_COMPOSITE. */
#define OPEN_RAW_MIME	(OPEN_RAW_RFC822_TEXT | OPEN_RAW_MIME_PART)

/*	Update Flag Definitions.  These flags are passed to NSFNoteUpdate and
	NSFNoteDelete. See also NOTEID_xxx special definitions in nsfdata.h. */

#define UPDATE_FORCE			0x0001	/* update even if ERR_CONFLICT */
#define	UPDATE_NAME_KEY_WARNING	0x0002	/* give error if new field name defined */
#define	UPDATE_NOCOMMIT			0x0004	/* do NOT do a database commit after update */
#define UPDATE_NOREVISION		0x0100	/* do NOT maintain revision history */
#define	UPDATE_NOSTUB			0x0200	/* update body but leave no trace of note in file if deleted */
#define UPDATE_INCREMENTAL		0x4000	/* Compute incremental note info */
#define	UPDATE_DELETED			0x8000	/* update body DELETED */

#define UPDATE_DUPLICATES		0		/* Obsolete; but in SDK */

/* Conflict Handler defines */
#define CONFLICT_ACTION_MERGE 1
#define CONFLICT_ACTION_HANDLED 2

#define	UPDATE_SHARE_SECOND			0x00200000L	/*	Split the second update of this note with the object store */
#define	UPDATE_SHARE_OBJECTS		0x00400000L	/*	Share objects only, not non-summary items, with the object store */
#define NOTE_LOCK_STATUS 			0x00000008		/* Return status of lock */
#define NOTE_LOCK_HARD  			0x00000010		/* Take out a hard note lock */
#define NOTE_LOCK_PROVISIONAL  		0x00000020		/* Take out a provisional hard note lock */
/*	Structure returned from NSFNoteDecrypt which can be used to decrypt
	file attachment objects, which are not decrypted until necessary. */

typedef struct
	{
	BYTE	Byte1;
	WORD	Word1;
	BYTE	Text[16];
	}
	ENCRYPTION_KEY;


/*	Flags returned (beginning in V3) in the _NOTE_FLAGS */

#define	NOTE_FLAG_READONLY		0x0001	/* TRUE if document cannot be updated */
#define	NOTE_FLAG_ABSTRACTED	0x0002	/* missing some data */
#define NOTE_FLAG_INCREMENTAL	0x0004	/* Incremental note (place holders) */
#define NOTE_FLAG_LINKED		0x0020	/* Note contains linked items or linked objects */
#define NOTE_FLAG_INCREMENTAL_FULL	0x0040	/* Incremental type note Fully opened (NO place holders)
											This type of note is meant to retain the 
											Item sequence numbers */
#define NOTE_FLAG_CANONICAL		0x4000	/* Note is (opened) in canonical form */

/* 	Note structure member IDs for NSFNoteGet&SetInfo. */

#define	_NOTE_DB		0		/* IDs for NSFNoteGet&SetInfo */
#define	_NOTE_ID		1		/* (When adding new values, see the */ 
#define	_NOTE_OID		2		/*  table in NTINFO.C */
#define	_NOTE_CLASS		3
#define	_NOTE_MODIFIED	4
#define	_NOTE_PRIVILEGES	5	/* For pre-V3 compatibility. Should use $Readers item */
#define	_NOTE_FLAGS		7
#define	_NOTE_ACCESSED	8
#define _NOTE_PARENT_NOTEID 10	/* For response hierarchy */
#define _NOTE_RESPONSE_COUNT 11	/* For response hierarchy */
#define _NOTE_RESPONSES 12		/* For response hierarchy */
#define _NOTE_ADDED_TO_FILE 13	/* For AddedToFile time */
#define _NOTE_OBJSTORE_DB 14	/* DBHANDLE of object store used by linked items */


/*	EncryptFlags used in NSFNoteCopyAndEncrypt */

#define	ENCRYPT_WITH_USER_PUBLIC_KEY	0x0001
#define	ENCRYPT_SMIME_IF_MIME_PRESENT	0x0002
#define ENCRYPT_SMIME_NO_SENDER			0x0004
#define	ENCRYPT_SMIME_TRUST_ALL_CERTS	0x0008

#define	SIGN_NOTES_IF_MIME_PRESENT		0x00000001

/*	DecryptFlags used in NSFNoteDecrypt */

#define DECRYPT_ATTACHMENTS_IN_PLACE	0x0001

/*	Flags used for NSFNoteExtractFileExt */

#define NTEXT_RESONLY		0x0001	/*	If a Mac attachment, extract resource fork only. */
#define NTEXT_FTYPE_MASK	0x0070	/*	File type mask */
#define NTEXT_FTYPE_FLAT	0x0000	/*	Normal one fork file */
#define NTEXT_FTYPE_MACBIN	0x0010	/*	MacBinaryII file */
#define NTEXT_RAWMIME		0x0080	/*	Do not decode MIME content transfer encoding */
#define NTEXT_IGNORE_HUFF2	0x0100	/*	Ignore checksum mismatch and save data anyway */

/* 	Possible return values from the callback routine specified in
	NSFNoteComputeWithForm() */
	
#define CWF_ABORT					1
#define CWF_NEXT_FIELD				2
#define CWF_RECHECK_FIELD			3

/* 	Possible validation phases for NSFNoteComputeWithForm()  */
	
#define CWF_DV_FORMULA				1 
#define CWF_IT_FORMULA				2
#define CWF_IV_FORMULA				3
#define CWF_COMPUTED_FORMULA		4 
#define CWF_DATATYPE_CONVERSION		5	
#define CWF_COMPUTED_FORMULA_LOAD	CWF_COMPUTED_FORMULA
#define CWF_COMPUTED_FORMULA_SAVE	6

/*	Function pointer type for NSFNoteComputeWithForm() callback */

typedef	WORD (LNCALLBACKPTR CWF_ERROR_PROC) (const void far *pCDField, 
					 					   WORD phase, STATUS error, 
										   HANDLE ErrorText,
							 			   WORD wErrorTextSize,
							 			   void far *ctx);

/*	Options to NSFNoteComputeWithForm() */

#define	CWF_CONTINUE_ON_ERROR			0x0001		/*	Ignore compute errors */


/* function templates */

STATUS LNPUBLIC NSFItemAppend (NOTEHANDLE hNote, WORD ItemFlags,
								const char far *Name, WORD NameLength,
								WORD DataType,
								const void far *Value, DWORD ValueLength);
STATUS LNPUBLIC NSFItemAppendByBLOCKID (NOTEHANDLE hNote, WORD ItemFlags,
								const char far *Name, WORD NameLength,
								BLOCKID bhValue, DWORD ValueLength,
								BLOCKID far *retbhItem);


STATUS LNPUBLIC NSFItemAppendObject (NOTEHANDLE hNote, WORD ItemFlags,
								const char far *Name, WORD NameLength,
								BLOCKID bhValue, DWORD ValueLength, BOOL fDealloc);

STATUS LNPUBLIC NSFItemDelete (NOTEHANDLE hNote, const char far *Name, WORD NameLength);
STATUS LNPUBLIC NSFItemDeleteByBLOCKID (NOTEHANDLE hNote, BLOCKID bhItem);

STATUS LNPUBLIC NSFItemRealloc (BLOCKID bhItem, BLOCKID far *bhValue, DWORD ValueLength);

STATUS LNPUBLIC NSFItemCopy (NOTEHANDLE hNote, BLOCKID bhItem);
STATUS LNPUBLIC NSFItemInfo (NOTEHANDLE hNote,
								const char far *Name, WORD NameLength,
								BLOCKID far *retbhItem,
								WORD far *retDataType,
								BLOCKID far *retbhValue,
								DWORD far *retValueLength);

#define NSFItemIsPresent(hNote, Name, NameLength) \
		((BOOL) (NSFItemInfo(hNote, Name, NameLength, NULL, NULL, NULL, NULL) == NOERROR))

STATUS LNPUBLIC NSFItemInfoNext (NOTEHANDLE hNote, BLOCKID NextItem,
								const char far *Name, WORD NameLength,
								BLOCKID far *retbhItem,
								WORD far *retDataType,
								BLOCKID far *retbhValue,
								DWORD far *retValueLength);

STATUS LNPUBLIC NSFItemInfoPrev (NOTEHANDLE hNote, BLOCKID CurrItem,
								const char far *Name, WORD NameLength,
								BLOCKID far *retbhItem,
								WORD far *retDataType,
								BLOCKID far *retbhValue,
								DWORD far *retValueLength);

void LNPUBLIC NSFItemQuery (NOTEHANDLE hNote, BLOCKID bhItem,
								char far *retItemName, WORD ItemNameBufferLength,
								WORD far *retItemNameLength,
								WORD far *retItemFlags,
								WORD far *retDataType,
								BLOCKID far *retbhValue, DWORD far *retValueLength);

void LNPUBLIC NSFItemQueryEx (NOTEHANDLE hNote, BLOCKID bhItem,
								char *retItemName, WORD ItemNameBufferLength,
								WORD *retItemNameLength,
								WORD *retItemFlags,
								WORD *retDataType,
								BLOCKID *retbhValue, DWORD *retValueLength, 
								BYTE *retSeqByte, BYTE *retDupItemID);

WORD LNPUBLIC NSFItemGetText (NOTEHANDLE hNote, const char far *ItemName, char far *retBuffer, WORD BufferLength);
BOOL LNPUBLIC NSFItemGetTime (NOTEHANDLE hNote, const char far *ItemName, TIMEDATE far *retTime);
BOOL LNPUBLIC NSFItemGetNumber (NOTEHANDLE hNote, const char far *ItemName, NUMBER far *retNumber);
LONG LNPUBLIC NSFItemGetLong (NOTEHANDLE hNote, const char far *ItemName, LONG DefaultNumber);

STATUS LNPUBLIC NSFItemSetText (NOTEHANDLE hNote, const char far *ItemName, const char far *Text, WORD TextLength);


STATUS LNPUBLIC NSFItemSetTextSummary (NOTEHANDLE hNote, const char far *ItemName, const char far *Text, WORD TextLength, BOOL Summary);
STATUS LNPUBLIC NSFItemSetTime (NOTEHANDLE hNote, const char far *ItemName, const TIMEDATE far *Time);
STATUS LNPUBLIC NSFItemSetNumber (NOTEHANDLE hNote, const char far *ItemName, const NUMBER far *Number);

WORD LNPUBLIC NSFItemGetTextListEntries (NOTEHANDLE hNote, const char far *ItemName);
WORD LNPUBLIC NSFItemGetTextListEntry (NOTEHANDLE hNote, const char far *ItemName, WORD EntryPos, char far *retBuffer, WORD BufferLength);
STATUS LNPUBLIC NSFItemCreateTextList (NOTEHANDLE hNote, const char far *ItemName, const char far *Text, WORD TextLength);
STATUS LNPUBLIC NSFItemAppendTextList (NOTEHANDLE hNote, const char far *ItemName, const char far *Text, WORD TextLength, BOOL fAllowDuplicates);

STATUS LNPUBLIC NSFItemGetModifiedTime(HANDLE hNote, const char *ItemName, WORD ItemNameLength, DWORD Flags, TIMEDATE *retTime);
STATUS LNPUBLIC NSFItemGetModifiedTimeByBLOCKID (HANDLE hNote, BLOCKID bhItem, DWORD Flags, TIMEDATE *retTime);



BOOL LNPUBLIC NSFItemTextEqual(NOTEHANDLE hNote, const char far *ItemName, const char far *Text, WORD TextLength, BOOL fCaseSensitive);
BOOL LNPUBLIC NSFItemTimeCompare(NOTEHANDLE hNote, const char far *ItemName, const TIMEDATE far *Time, int far *retVal);
BOOL LNPUBLIC NSFItemLongCompare(NOTEHANDLE hNote, const char far *ItemName, long Value, int far *retVal);

WORD LNPUBLIC NSFItemConvertValueToText (WORD DataType, BLOCKID bhValue, DWORD ValueLength, char far *retBuffer, WORD BufferLength, char SepChar);
WORD LNPUBLIC NSFItemConvertToText (NOTEHANDLE hNote, const char far *ItemName, char far *retBuffer, WORD BufferLength, char SepChar);

BOOL LNPUBLIC NSFGetSummaryValue (const void far *SummaryBuffer, const char far *Name, char far *retValue, WORD ValueBufferLength);
BOOL LNPUBLIC NSFLocateSummaryValue (const void far *SummaryBuffer, const char far *Name,
								void far *retValuePointer,
								WORD far *retValueLength,
								WORD far *retDataType);
typedef STATUS (LNCALLBACKPTR NSFITEMSCANPROC)(WORD Spare, WORD ItemFlags,
											char far *Name, WORD NameLength,
											void far *Value, DWORD ValueLength,
											void far *RoutineParameter);
STATUS LNPUBLIC NSFItemScan (NOTEHANDLE hNote,
								NSFITEMSCANPROC ActionRoutine,
								void far *RoutineParameter);
void LNPUBLIC NSFNoteGetInfo (NOTEHANDLE hNote, WORD Type, void far *Value);
void LNPUBLIC NSFNoteSetInfo (NOTEHANDLE hNote, WORD Type, void far *Value);
STATUS LNPUBLIC NSFNoteContract (NOTEHANDLE hNote);
STATUS LNPUBLIC NSFNoteClose (NOTEHANDLE hNote);
STATUS LNPUBLIC NSFNoteCreate (DBHANDLE hDB, NOTEHANDLE far *rethNote);
STATUS LNPUBLIC NSFNoteDelete (DBHANDLE hDB, NOTEID NoteID, WORD UpdateFlags);
STATUS LNPUBLIC NSFNoteDeleteExtended(DBHANDLE hDB, NOTEID NoteID, DWORD UpdateFlags);
STATUS LNPUBLIC NSFDbNoteLock (DBHANDLE hDB, NOTEID NoteID, DWORD Flags, char *pLockers, HANDLE *rethLockers, DWORD *retLength);
STATUS LNPUBLIC NSFDbNoteUnlock (DBHANDLE hDB, NOTEID NoteID, DWORD Flags);
STATUS LNPUBLIC NSFNoteOpenWithLock (DBHANDLE hDB, NOTEID NoteID, DWORD LockFlags, DWORD OpenFlags, char *pLockers, HANDLE *rethLockers, DWORD *retLength, NOTEHANDLE far *rethNote);
STATUS LNPUBLIC NSFNoteOpen (DBHANDLE hDB, NOTEID NoteID, WORD OpenFlags, NOTEHANDLE far *rethNote);
STATUS LNPUBLIC NSFNoteOpenExt(DBHANDLE hDB, NOTEID NoteID, DWORD flags, NOTEHANDLE *rethNote);
STATUS LNPUBLIC NSFNoteHardDelete(DBHANDLE hDB, NOTEID NoteID, DWORD Reserved);
STATUS LNPUBLIC NSFNoteOpenSoftDelete(DBHANDLE hDB, NOTEID NoteID, DWORD Reserved, NOTEHANDLE *rethNote);
#define GETNOTES_PRESERVE_ORDER		0x00000001	/* Preserve order of notes in NoteID list */
#define GETNOTES_SEND_OBJECTS   	0x00000002	/* Send (copiable) objects along with note */
#define GETNOTES_ORDER_BY_SIZE		0x00000004	/* Order returned notes by (approximate) ascending size */
#define GETNOTES_CONTINUE_ON_ERROR	0x00000008	/* Continue to next on list if error encountered */
#define GETNOTES_GET_FOLDER_ADDS	0x00000010	/* Enable folder-add callback function after the note-level callback */
#define GETNOTES_APPLY_FOLDER_ADDS	0x00000020	/* Apply folder ops directly - don't bother using callback */
#define	GETNOTES_NO_STREAMING		0x00000040	/* Don't stream - used primarily for testing purposes */
typedef STATUS (LNCALLBACKPTR NSFGETNOTESCALLBACK) (void *Param, DWORD TotalSizeLow, DWORD TotalSizeHigh);
typedef STATUS (LNCALLBACKPTR NSFNOTEOPENCALLBACK) (void *Param, NOTEHANDLE hNote, DWORD NoteID, STATUS status);
typedef STATUS (LNCALLBACKPTR NSFOBJECTALLOCCALLBACK) (void *Param, NOTEHANDLE hNote, NOTEID OldRRV, STATUS status, DWORD ObjectSize);
typedef STATUS (LNCALLBACKPTR NSFOBJECTWRITECALLBACK) (void *Param, NOTEHANDLE hNote, NOTEID OldRRV, STATUS status, BYTE *Buffer, DWORD BufferSize);
typedef STATUS (LNCALLBACKPTR NSFFOLDERADDCALLBACK) (void *Param, UNID *NoteUNID, HANDLE OpBlock, DWORD OpBlockSize);
STATUS LNPUBLIC NSFDbGetNotes(DBHANDLE hDB, DWORD NumNotes, NOTEID *NoteID, DWORD *NoteOpenFlags, DWORD *SinceSeqNum,
								DWORD ControlFlags, DBHANDLE hObjectDB, void *CallbackParam,
								NSFGETNOTESCALLBACK	GetNotesCallback,
								NSFNOTEOPENCALLBACK NoteOpenCallback,
								NSFOBJECTALLOCCALLBACK ObjectAllocCallback,
								NSFOBJECTWRITECALLBACK ObjectWriteCallback,
								TIMEDATE *FolderSinceTime,
								NSFFOLDERADDCALLBACK FolderAddCallback);
STATUS LNPUBLIC NSFNoteOpenByUNID(DBHANDLE hDB, UNID far *pUNID, WORD flags, NOTEHANDLE far *rethNote);
STATUS LNPUBLIC NSFNoteUpdate (NOTEHANDLE hNote, WORD UpdateFlags);
STATUS LNPUBLIC NSFNoteUpdateExtended (NOTEHANDLE hNote, DWORD UpdateFlags);
STATUS LNPUBLIC NSFNoteComputeWithForm (NOTEHANDLE hNote, NOTEHANDLE hFormNote,
								DWORD dwFlags, CWF_ERROR_PROC ErrorRoutine,  
								void far *CallersContext);

STATUS LNPUBLIC NSFNoteAttachFile (NOTEHANDLE hNOTE,
								const char far *ItemName, WORD ItemNameLength,
								const char far *PathName,
								const char far *OriginalPathName,
								WORD Encoding);
STATUS LNPUBLIC NSFNoteExtractFile (NOTEHANDLE hNote, BLOCKID bhItem,
								const char far *FileName,
								ENCRYPTION_KEY far *DecryptionKey);
STATUS LNPUBLIC NSFNoteExtractFileExt (NOTEHANDLE hNote, BLOCKID bhItem,
								const char far *FileName,
								ENCRYPTION_KEY far *DecryptionKey,
								WORD wFlags);
typedef STATUS (LNCALLBACKPTR NOTEEXTRACTCALLBACK)(const BYTE *bytes, DWORD length, void far *pParam);
STATUS LNPUBLIC NSFNoteExtractWithCallback(NOTEHANDLE hNote, BLOCKID bhItem, ENCRYPTION_KEY far *DecryptionKey,
										   WORD wFlags, NOTEEXTRACTCALLBACK pNoteExtractCallback,
										   void far *pParam);

STATUS LNPUBLIC NSFNoteDetachFile (NOTEHANDLE hNote, BLOCKID bhItem);
BOOL LNPUBLIC	  NSFNoteHasObjects (NOTEHANDLE hNote, BLOCKID far *bhFirstObjectItem);
STATUS LNPUBLIC NSFNoteGetAuthor (NOTEHANDLE hNote, char far *retName, WORD far *retNameLength, 
									BOOL far *retIsItMe);
STATUS LNPUBLIC NSFNoteCopy (NOTEHANDLE hSrcNote, NOTEHANDLE far *rethDstNote);
STATUS LNPUBLIC NSFNoteSignExt(NOTEHANDLE hNote, 
								const char far *SignatureItemName,
								WORD ItemCount, HANDLE hItemIDs);
STATUS LNPUBLIC NSFNoteSign (NOTEHANDLE hNote);
STATUS LNPUBLIC NSFComputeObjectDigest(HANDLE hNote, BLOCKID bItem);
STATUS LNPUBLIC NSFNoteSignExt3(NOTEHANDLE hNote, 
								KFHANDLE	hKFC,
								const char far *SignatureItemName,
								WORD ItemCount, HANDLE hItemIDs, 
								DWORD Flags, DWORD Reserved,
								void	*pReserved);
STATUS LNPUBLIC NSFNoteInspectSignatureExt2 (NOTEHANDLE hNote, 
								KFHANDLE	hKFC,
								char far *pzSigItemName,
								TIMEDATE far *retWhenSigned, 
								char far *retSigner, char far *retCertifier,
								WORD	*retItemCount,
								HANDLE	*rethItemIDs,
								DWORD	Reserved,
								void 	*pReserved);

STATUS LNPUBLIC NSFHotSpotSign(BYTE *pSource, DWORD dwSourceLength, BYTE *pObject, DWORD dwObjectLength, HANDLE *hSigData, DWORD *dwSigLength); 
STATUS LNPUBLIC NSFNoteSignHotspots(NOTEHANDLE hNote, DWORD dwFlags, BOOL *retfSigned);
STATUS LNPUBLIC NSFNoteVerifySignature (NOTEHANDLE hNote, 
								char far *Reserved,
								TIMEDATE far *retWhenSigned, 
								char far *retSigner, char far *retCertifier);
STATUS LNPUBLIC NSFVerifyFileObjSignature (DBHANDLE hDB, BLOCKID bhItem);
STATUS LNPUBLIC NSFNoteUnsign (NOTEHANDLE hNote);
STATUS LNPUBLIC NSFNoteCopyAndEncrypt (NOTEHANDLE hSrcNote, WORD EncryptFlags, NOTEHANDLE far *rethDstNote);
STATUS LNPUBLIC NSFNoteCopyAndEncryptExt2 (NOTEHANDLE hSrcNote, KFHANDLE hKFC,  WORD EncryptFlags, NOTEHANDLE far *rethDstNote, DWORD Reserved, void *pReserved);
STATUS LNPUBLIC NSFNoteDecrypt (NOTEHANDLE hNote, WORD DecryptFlags, ENCRYPTION_KEY far *retKeyForAttachments);
STATUS LNPUBLIC NSFNoteDecryptExt2 (NOTEHANDLE hNote, KFHANDLE hKFC, WORD DecryptFlags, ENCRYPTION_KEY far *retKeyForAttachments, DWORD Reserved, void *pReserved);

/*	Profile note routines. */

STATUS LNPUBLIC NSFProfileOpen(
		DBHANDLE hDB, const char *ProfileName, WORD ProfileNameLength,
		const char *UserName, WORD UserNameLength, BOOL CopyProfile,
		NOTEHANDLE *rethProfileNote);
STATUS LNPUBLIC NSFProfileDelete(
		DBHANDLE hDB, const char *ProfileName, WORD ProfileNameLength,
		const char *UserName, WORD UserNameLength);
STATUS LNPUBLIC NSFProfileUpdate(
		NOTEHANDLE hProfile,
		const char *ProfileName, WORD ProfileNameLength,
		const char *UserName, WORD UserNameLength);
typedef STATUS (LNCALLBACKPTR NSFPROFILEENUMPROC)(
					DBHANDLE hDB, 
					void far *Ctx,
					char *ProfileName,
					WORD ProfileNameLength,
					char *UserName,
					WORD UserNameLength,
					NOTEID ProfileNoteID);
STATUS LNPUBLIC NSFProfileEnum(
		DBHANDLE hDB, const char *ProfileName, WORD ProfileNameLength,
		NSFPROFILEENUMPROC Callback,
		void *CallbackCtx, DWORD Flags);
STATUS LNPUBLIC NSFProfileGetField(
		DBHANDLE hDB, const char *ProfileName, WORD ProfileNameLength,
		const char *UserName, WORD UserNameLength,
		const char *FieldName, WORD FieldNameLength,
		WORD *retDatatype, BLOCKID *retbhValue, DWORD *retValueLength);
STATUS LNPUBLIC NSFProfileSetField(
		DBHANDLE hDB, const char *ProfileName, WORD ProfileNameLength,
		const char *UserName, WORD UserNameLength,
		const char *FieldName, WORD FieldNameLength,
		WORD Datatype, void *Value, DWORD ValueLength);


BOOL LNPUBLIC NSFNoteIsSignedOrSealed (NOTEHANDLE hNote, BOOL far *retfSigned, BOOL far *retfSealed);
STATUS LNPUBLIC NSFNoteCheck (HANDLE hNote);

STATUS LNPUBLIC NSFNoteFindDivergenceTime(NOTEHANDLE hNote1, NOTEHANDLE hNote2,	DWORD dwFlags, TIMEDATE *tdLastSyncTime);
STATUS LNPUBLIC NSFNoteFindMatchingItem (NOTEHANDLE hNote1, BLOCKID bhItem1, NOTEHANDLE hNote2, DWORD dwFlags, BLOCKID *retbhItem2);


/*	External (text) link routines */

#define LINKFLAG_ADD_TEMPORARY		0x00000002L
#define LINKFLAG_NO_REPL_SEARCH		0x00000004L

STATUS LNPUBLIC NSFNoteLinkFromText(HANDLE hLinkText, WORD LinkTextLength,
									NOTELINK far *NoteLink,
									char far *ServerHint,
									char far *LinkText, WORD MaxLinkText,
									DWORD far *retFlags);
STATUS LNPUBLIC NSFNoteLinkToText(char far *Title,
									NOTELINK far *NoteLink,
									char far *ServerHint,
									char far *LinkText,
									HANDLE far *phLinkText,
									WORD far *pLinkTextLength,
									DWORD Flags);

/* End of Note Storage File Note Definitions */

STATUS LNPUBLIC NSFNoteLSCompile ( DBHANDLE hDB, 
					NOTEHANDLE hNote, 
					DWORD dwFlags );

/*	Extended version with callback for compile errors.  Note that the callback
	function only gets called for the first error in each LotusScript module.
	There may be more than one module in a note. */

typedef struct
{
	WORD		Version;		/* allows for future expansion - currently always 1 */
	WORD		Line;			/* source line number of error, relative to LotusScript
									module containing the error, if applicable */
	const char*	pErrText;		/* error text */
	const char* pErrFile;		/* file name, if applicable */
} LSCOMPILE_ERR_INFO;

typedef STATUS (LNCALLBACKPTR LSCOMPILEERRPROC)(
	const LSCOMPILE_ERR_INFO* pInfo,	/* error info - see above */
	void* pCtx);						/* caller's pCtx argument from NSFNoteLSCompileExt */

STATUS LNPUBLIC NSFNoteLSCompileExt ( DBHANDLE hDB, 
					NOTEHANDLE hNote, 
					DWORD dwFlags,
					LSCOMPILEERRPROC pfnErrProc,	/* callback function for compile errors */
					void* pCtx );					/* caller's context argument, passed to callback */

#ifdef __cplusplus
}
#endif

#endif



#if defined(OS400) && (__OS400_TGTVRM__ >= 510)
#pragma datamodel(pop)
#endif

