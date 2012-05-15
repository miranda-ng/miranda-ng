/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.

	$Id: pgpLDAP.h,v 1.24 2004/01/23 08:12:49 dallen Exp $
____________________________________________________________________________*/
#ifndef Included_pgpLDAP_h	/* [ */
#define Included_pgpLDAP_h

#include "pgpBase.h"
#include "pgpPubTypes.h"
#include "pgpMemoryMgr.h"
#include "pgpSockets.h"
#include "pgpBER.h"

PGP_BEGIN_C_DECLARATIONS

/* LDAP parameters and constants */
#define kPGPldap_DefaultPort		389
#define kPGPldap_DefaultSecurePort  636
#define kPGPldap_DefaultVersion		2

#define kPGPldapScopeString_Base		PGPTXT_MACHINE8("base")
#define kPGPldapScopeString_OneLevel	PGPTXT_MACHINE8("one")
#define kPGPldapScopeString_Subtree		PGPTXT_MACHINE8("sub")

#define kPGPldap_DefaultFilterString	PGPTXT_MACHINE8("(objectclass=*)")



/* Session options */
#define kPGPldapOpt_Deref			1
#define kPGPldapOpt_Sizelimit		2
#define kPGPldapOpt_Timelimit		3
#define kPGPldapOpt_Desc			4

#define kPGPldapOpt_ON				((void *) 1)
#define kPGPldapOpt_OFF 			((void *) 0)

enum PGPldapResult_
{
	kPGPldapResult_Success						= 0x00,
	kPGPldapResult_OperationsError				= 0x01,
	kPGPldapResult_ProtocolError				= 0x02,
	kPGPldapResult_TimelimitExceeded			= 0x03,
	kPGPldapResult_SizelimitExceeded			= 0x04,
	kPGPldapResult_CompareFalse 				= 0x05,
	kPGPldapResult_CompareTrue					= 0x06,
	kPGPldapResult_StrongAuthNotSupported		= 0x07,
	kPGPldapResult_StrongAuthRequired			= 0x08,
	kPGPldapResult_PartialResults				= 0x09,
	kPGPldapResult_NoSuchAttribute				= 0x10,
	kPGPldapResult_UndefinedType				= 0x11,
	kPGPldapResult_InappropriateMatching		= 0x12,
	kPGPldapResult_ConstraintViolation			= 0x13,
	kPGPldapResult_TypeOrValueExists			= 0x14,
	kPGPldapResult_InvalidSyntax				= 0x15,
	kPGPldapResult_NoSuchObject 				= 0x20,
	kPGPldapResult_AliasProblem 				= 0x21,
	kPGPldapResult_InvalidDNSyntax				= 0x22,
	kPGPldapResult_IsLeaf						= 0x23,
	kPGPldapResult_AliasDerefProblem			= 0x24,
	kPGPldapResult_InappropriateAuth			= 0x30,
	kPGPldapResult_InvalidCredentials			= 0x31,
	kPGPldapResult_InsufficientAccess			= 0x32,
	kPGPldapResult_Busy 						= 0x33,
	kPGPldapResult_Unavailable					= 0x34,
	kPGPldapResult_UnwillingToPerform			= 0x35,
	kPGPldapResult_LoopDetect					= 0x36,
	kPGPldapResult_NamingViolation				= 0x40,
	kPGPldapResult_ObjectClassViolation 		= 0x41,
	kPGPldapResult_NotAllowedOnNonleaf			= 0x42,
	kPGPldapResult_NotAllowedOnRDN				= 0x43,
	kPGPldapResult_AlreadyExists				= 0x44,
	kPGPldapResult_NoObjectClassMods			= 0x45,
	kPGPldapResult_ResultsTooLarge				= 0x46,
	kPGPldapResult_Other						= 0x50,
	kPGPldapResult_ServerDown					= 0x51,
	kPGPldapResult_LocalError					= 0x52,
	kPGPldapResult_EncodingError				= 0x53,
	kPGPldapResult_DecodingError				= 0x54,
	kPGPldapResult_Timeout						= 0x55,
	kPGPldapResult_AuthUnknown					= 0x56,
	kPGPldapResult_FilterError					= 0x57,
	kPGPldapResult_UserCancelled				= 0x58,
	kPGPldapResult_ParamError					= 0x59,
	kPGPldapResult_NoMemory 					= 0x5a,
	kPGPldapResult_ConnectError 				= 0x5b,
	
	kPGPldapResult_None							= 0xFFFFFFFF
};
PGPENUM_TYPEDEF (PGPldapResult_, PGPldapResult);

enum PGPldapType_
{
	kPGPldapType_None				= 0,

	kPGPldapRequest_Bind			= 0x60, /* application + constructed */
	kPGPldapRequest_Unbind			= 0x42, /* application + primitive	 */
	kPGPldapRequest_Search			= 0x63, /* application + constructed */
	kPGPldapRequest_Modify			= 0x66, /* application + constructed */
	kPGPldapRequest_Add 			= 0x68, /* application + constructed */
	kPGPldapRequest_Delete			= 0x4A, /* application + primitive	 */
	kPGPldapRequest_ModifyRDN		= 0x6C, /* application + constructed */
	kPGPldapRequest_Compare 		= 0x6E, /* application + constructed */
	kPGPldapRequest_Abandon 		= 0x50, /* application + primitive	 */

	kPGPldapResponse_Bind			= 0x61, /* application + constructed */
	kPGPldapResponse_SearchEntry	= 0x64, /* application + constructed */
	kPGPldapResponse_SearchResult	= 0x65, /* application + constructed */
	kPGPldapResponse_Modify 		= 0x67, /* application + constructed */
	kPGPldapResponse_Add			= 0x69, /* application + constructed */
	kPGPldapResponse_Delete 		= 0x6B, /* application + constructed */
	kPGPldapResponse_ModifyRDN		= 0x6D, /* application + constructed */
	kPGPldapResponse_Compare		= 0x6F, /* application + constructed */

	kPGPldapResponse_Any			= 0xFFFFFFFF
};
PGPENUM_TYPEDEF (PGPldapType_, PGPldapType);

enum PGPldapScope_
{
	kPGPldapScope_Base			= 0,
	kPGPldapScope_OneLevel		= 1,
	kPGPldapScope_Subtree		= 2,

	kPGPldapScope_None			= -1
};
PGPENUM_TYPEDEF (PGPldapScope_, PGPldapScope);

enum PGPldapDeref_
{
	kPGPldapDeref_Never 		= 0,
	kPGPldapDeref_Searching 	= 1,
	kPGPldapDeref_Finding		= 2,
	kPGPldapDeref_Always		= 3
};
PGPENUM_TYPEDEF (PGPldapDeref_, PGPldapDeref);

enum PGPldapAuth_
{
	kPGPldapAuth_Simple 		= 0x80	/* context-specific + primitive, tag = 0 */
	/* Not implemented -- kPGPldapAuth_Kerb41		  = 0x81, */
	/* Not implemented -- kPGPldapAuth_Kerb42		  = 0x82  */
};
PGPENUM_TYPEDEF (PGPldapAuth_, PGPldapAuth);

enum PGPldapFilter_
{
	kPGPldapFilter_And			= 0xA0,	/* context specific + constructed, tag = 0 */
	kPGPldapFilter_Or			= 0xA1,	/* context specific + constructed, tag = 1 */
	kPGPldapFilter_Not			= 0xA2,	/* context specific + constructed, tag = 2 */
	kPGPldapFilter_Equal		= 0xA3,	/* context specific + constructed, tag = 3 */
	kPGPldapFilter_Substrings	= 0xA4,	/* context specific + constructed, tag = 4 */
	kPGPldapFilter_GE			= 0xA5,	/* context specific + constructed, tag = 5 */
	kPGPldapFilter_LE			= 0xA6,	/* context specific + constructed, tag = 6 */
	kPGPldapFilter_Present		= 0x87,	/* context specific + primitive,   tag = 7 */
	kPGPldapFilter_Approx		= 0xA8,	/* context specific + constructed, tag = 8 */

	kPGPldapFilter_None			= (-1L)
};
PGPENUM_TYPEDEF( PGPldapFilter_, PGPldapFilter );

enum PGPldapSubstring_
{
	kPGPldapSubstring_Initial	= 0x80,	/* context specific + primitive, tag = 0 */
	kPGPldapSubstring_Any		= 0x81,	/* context specific + primitive, tag = 1 */
	kPGPldapSubstring_Final		= 0x82,	/* context specific + primitive, tag = 2 */

	kPGPldapSubstring_None		= (-1L)
};
PGPENUM_TYPEDEF( PGPldapSubstring_, PGPldapSubstring );

enum PGPldapModOp_
{
	kPGPldapModOp_Add			= 0x00,
	kPGPldapModOp_Delete		= 0x01,
	kPGPldapModOp_Replace		= 0x02,

	kPGPldapModOp_None			= (-1L)
};
PGPENUM_TYPEDEF( PGPldapModOp_, PGPldapModOp );

#define kPGPldapModOpMask_UseBERValues	0x10


/* LDAP Data types: typedefs, structs, and whatnot */

typedef PGPUInt32 PGPldapMessageID;

#define kPGPldapMessageID_Any		( (PGPldapMessageID) -1 )
#define kInvalidPGPldapMessageID	( (PGPldapMessageID)  0 )
#define PGPldapMessageIDIsValid(messageID) \
			( (messageID) != kInvalidPGPldapMessageID )
#define PGPValidateLDAPMessageID(messageID) \
			PGPValidateParam( PGPldapMessageIDIsValid( messageID ) )

typedef struct PGPldapContext *	PGPldapContextRef;

#define kInvalidPGPldapContextRef		( (PGPldapContextRef) NULL )
#define PGPldapContextRefIsValid(pgpLDAP)	\
			( (pgpLDAP) != kInvalidPGPldapContextRef )
#define PGPValidateLDAPContextRef(context) \
			PGPValidateParam( PGPldapContextRefIsValid( context ) )

typedef struct PGPldapMessage *	PGPldapMessageRef;

#define kInvalidPGPldapMessageRef		( (PGPldapMessageRef) NULL)
#define PGPldapMessageRefIsValid(message)	\
			( (message) != kInvalidPGPldapMessageRef )
#define PGPValidateLDAPMessageRef(message) \
			PGPValidateParam( PGPldapMessageRefIsValid( message ) )


#define IsPGPldapError(err) \
			( ( (err) >= kPGPError_LDAPMIN ) && ( (err) <= kPGPError_LDAPMAX ) )


/* Callbacks for using LDAP functions in multithreaded apps */
typedef struct PGPldapThreadFns
{
	void *		(*pgpLDAPThread_MutexAlloc) 	(void);
	void		(*pgpLDAPThread_MutexFree)		(void *);
	PGPError	(*pgpLDAPThread_MutexLock)		(void *);
	PGPError	(*pgpLDAPThread_MutexUnlock)	(void *);
	void		(*pgpLDAPThread_SetErrno)		(int);
	PGPError	(*pgpLDAPThread_GetErrno)		(void);
	void		(*pgpLDAPThread_SetLDAPErrno)	(PGPInt32, PGPChar8 *, PGPChar8 *);
	PGPInt32	(*pgpLDAPThread_GetLDAPErrno)	(PGPChar8 **, PGPChar8 **);
} PGPldapThreadFns;

typedef struct PGPldapMod
{
	PGPldapModOp	op;
	PGPChar8 *		type;

    PGPChar8 **		value;
    PGPberValue **	bvalue;
} PGPldapMod;

typedef struct PGPldapURLDesc
{
	PGPChar8 *		host;
	PGPUInt16		port;

	PGPChar8 *		dn;
	PGPChar8 **		attrs;

	PGPldapScope	scope;
	PGPChar8 *		filter;

} PGPldapURLDesc;

typedef PGPError (* PGPldapRebindProcPtr )(
						PGPldapContextRef	pgpLDAP,
						PGPChar8 **			dnp,
						PGPChar8 **			pwp,
						PGPInt32 *			method,
						PGPBoolean			freeIt,
						PGPChar8 *			arg );


/* LDAP function prototypes */
/*
 * Above each function is the name of the analogous function
 * in the UMich or Netscape LDAP SDK.  While there are some subtle
 * differences, for the most part, each function works basically
 * the same as it's UMich/Netscape SDK counterpart.
 */

/* Memory management functions */
	PGPError
PGPNewLDAPContext(
	PGPContextRef		context,
	PGPldapContextRef	*pgpLDAP );

	PGPError
PGPNewLDAPMessage(
	PGPldapContextRef	pgpLDAP,
	PGPldapMessageRef *	message );

	PGPError
PGPFreeLDAPContext(
	PGPldapContextRef	pgpLDAP );

	PGPError
PGPFreeLDAPMessage(
	PGPldapMessageRef	message );

	/* ldap_value_free */
	PGPError
PGPFreeLDAPValues(
	PGPChar8 **			vals );

	/* ldap_value_free_len */
	PGPError
PGPFreeLDAPValuesLen( 
	PGPberValue **		bvals );

	PGPError
PGPFreeLDAPURLDesc(
	PGPldapURLDesc *	lud );


/* Result-handling functions */
	/* ldap_get_lderrno */
	PGPError 
PGPldapGetErrno(
	PGPldapContextRef	pgpLDAP,
	PGPChar8 **			matched,
	PGPChar8 **			message,
	PGPldapResult *		result );

	PGPError
PGPldapResultToError(
	PGPldapContextRef	pgpLDAP,
	PGPldapResult		result );

/* LDAP option functions */
	/* ldap_set_option */
	PGPError
PGPldapSetOption(
	PGPldapContextRef	pgpLDAP, 
	PGPUInt32			option,
	void *				value );

	/* ldap_get_option */
	PGPError
PGPldapGetOption(
	PGPldapContextRef	pgpLDAP,
	PGPUInt32			option,
	void *				value );


/* Connection starting and stopping functions */
	/* ldap_open */
	PGPError
PGPldapOpen(
	PGPldapContextRef		pgpLDAP,
	PGPChar8 *				host, 
	PGPUInt16				port );

	PGPError
PGPldapOpenTimeout(
	PGPldapContextRef	pgpLDAP,
	PGPChar8 *			host,
	PGPUInt16			port,
	PGPSocketsTimeValue	*tv );

	/* ldap_simple_bind_s */
	PGPError 
PGPldapSimpleBindSync(
	PGPldapContextRef	pgpLDAP,
	PGPChar8 *			dn,
	PGPChar8 *			password );

	/* ldap_simple_bind_st (extension) */
	PGPError
PGPldapSimpleBindSyncTimeout(
	PGPldapContextRef	pgpLDAP,
	PGPChar8 *			dn,
	PGPChar8 *			password,
	PGPSocketsTimeValue *tv);

	/* ldap_simple_bind */
	PGPError
PGPldapSimpleBind(
	PGPldapContextRef	pgpLDAP,
	PGPChar8 *			dn,
	PGPChar8 *			password,
	PGPldapMessageID *	messageID );

	/* ldap_bind_s */
	PGPError
PGPldapBindSync(
	PGPldapContextRef	pgpLDAP, 
	PGPChar8 *			dn,
	PGPChar8 *			password,
	PGPldapAuth			method );

	/* ldap_bind */
	PGPError
PGPldapBind(
	PGPldapContextRef	pgpLDAP,
	PGPChar8 *			dn,
	PGPChar8 *			password,
	PGPldapAuth			method,
	PGPldapMessageID *	messageID );

	/* ldap_unbind */
	PGPError
PGPldapUnbind(
	PGPldapContextRef	pgpLDAP );


/* Searching functions */
	/* ldap_search_s */
	PGPError 
PGPldapSearchSync(
	PGPldapContextRef	pgpLDAP,
	PGPChar8 *			base,
	PGPldapScope		scope,
	PGPChar8 *			filter,
	PGPChar8 *			attrs[],
	PGPBoolean			attrsOnly,
	PGPldapMessageRef	resultMessage );

	/* ldap_search_st */
	PGPError
PGPldapSearchSyncTimeout(
	PGPldapContextRef		pgpLDAP,
	PGPChar8 *				base,
	PGPldapScope			scope,
	PGPChar8 *				filter,
	PGPChar8 *				attrs[],
	PGPBoolean				attrsOnly,
	PGPSocketsTimeValue *	tv,
	PGPldapMessageRef		resultMessage );

	/* ldap_search */
	PGPError 
PGPldapSearch(
	PGPldapContextRef	pgpLDAP,
	PGPChar8 *			base,
	PGPldapScope		scope,
	PGPChar8 *			filter,
	PGPChar8 *			attrs[],
	PGPBoolean			attrsOnly,
	PGPldapMessageID *	messageID );

	/* ldap_count_entries */
	PGPError
PGPldapCountEntries(
	PGPldapContextRef	pgpLDAP, 
	PGPldapMessageRef	result,
	PGPInt32 *			num );

	/* ldap_first_entry */
	PGPError
PGPldapFirstEntry(
	PGPldapContextRef	pgpLDAP,
	PGPldapMessageRef	result,
	PGPldapMessageRef *	outMessage );

	/* ldap_next_entry */
	PGPError
PGPldapNextEntry(
	PGPldapContextRef	pgpLDAP,
	PGPldapMessageRef	prevEntry,
	PGPldapMessageRef *	outMessage );

	/* ldap_get_dn */
	PGPError
PGPldapGetDN(
	PGPldapContextRef	pgpLDAP,
	PGPldapMessageRef	entry,
	PGPChar8 **			dn );	/* Should be freed by PGPFreeData */

	/* ldap_first_attribute */
	PGPError
PGPldapFirstAttribute( 
	PGPldapContextRef	pgpLDAP,
	PGPldapMessageRef	entry,
	PGPberElementRef *	ber,
	PGPChar8 **				attr );	/* Should be freed by PGPFreeData */

	/* ldap_next_attribute */
	PGPError
PGPldapNextAttribute(
	PGPldapContextRef	pgpLDAP,
	PGPldapMessageRef	entry,
	PGPberElementRef	ber,
	PGPChar8 **				attr );	/* Should be freed by PGPFreeData */

	/* ldap_get_values */
	PGPError
PGPldapGetValues(
	PGPldapContextRef	pgpLDAP,
	PGPldapMessageRef	entry,
	PGPChar8 *			attr,
	PGPChar8 ***		values );	/* Should be freed by PGPFreeLDAPValues */

	/* ldap_get_values_len */
	PGPError
PGPldapGetValuesLen(
	PGPldapContextRef	pgpLDAP,
	PGPldapMessageRef	entry,
	PGPChar8 *			attr,
	PGPberValue ***		values );	/* Should be freed by PGPFreeLDAPValuesLen */

	/* ldap_count_values */
	PGPError
PGPldapCountValues(
	PGPldapContextRef	pgpLDAP,
	PGPChar8 **			values,
	PGPSize *			num );

	/* ldap_count_values_len */
	PGPError
PGPldapCountValuesLen(
	PGPldapContextRef	pgpLDAP,
	PGPberValue **		values,
	PGPSize *			num );


/* Update functions */

	/* ldap_modify */
	PGPError
PGPldapModify(
	PGPldapContextRef	pgpLDAP,
	PGPChar8 *			dn,
	PGPldapMod *		mod[],
	PGPldapMessageID *	messageID );

	/* ldap_modify_s */
	PGPError
PGPldapModifySync(
	PGPldapContextRef	pgpLDAP,
	PGPChar8 *			dn,
	PGPldapMod *		mod[] );

	/* ldap_modify_st (extension) */
	PGPError
PGPldapModifySyncTimeout(
	PGPldapContextRef	pgpLDAP,
	PGPChar8 *			dn,
	PGPldapMod *		mod[],
	PGPSocketsTimeValue *tv);

	/* ldap_add */
	PGPError
PGPldapAdd(
	PGPldapContextRef	pgpLDAP,
	PGPChar8 *			dn,
	PGPldapMod *		mod[],
	PGPldapMessageID *	messageID );

	/* ldap_add_s */
	PGPError
PGPldapAddSync(
	PGPldapContextRef	pgpLDAP,
	PGPChar8 *			dn,
	PGPldapMod *		mod[] );

	/* ldap_add_st (extension) */
	PGPError
PGPldapAddSyncTimeout(
	PGPldapContextRef	pgpLDAP,
	PGPChar8 *			dn,
	PGPldapMod *		mod[],
	PGPSocketsTimeValue	*tv );

	/* ldap_delete */
	PGPError
PGPldapDelete(
	PGPldapContextRef	pgpLDAP,
	PGPChar8 *			dn,
	PGPldapMessageID *	messageID );

	/* ldap_delete_s */
	PGPError
PGPldapDeleteSync(
	PGPldapContextRef	pgpLDAP,
	PGPChar8 *			dn );

	/* ldap_delete_st (extension) */
	PGPError
PGPldapDeleteSyncTimeout(
	PGPldapContextRef	pgpLDAP,
	PGPChar8 *			dn,
	PGPSocketsTimeValue *tv);

	/* ldap_modrdn2 */
	PGPError
PGPldapModifyRDN(
	PGPldapContextRef	pgpLDAP,
	PGPChar8 *			dn,
	PGPChar8 *			newRDN,
	PGPBoolean			deleteOldRDN,
	PGPldapMessageID *	messageID );

	/* ldap_modrdn2_s */
	PGPError
PGPldapModifyRDNSync(
	PGPldapContextRef	pgpLDAP,
	PGPChar8 *			dn,
	PGPChar8 *			newRDN,
	PGPBoolean			deleteOldRDN );

	/* ldap_compare */
	PGPError
PGPldapCompare(
	PGPldapContextRef	pgpLDAP,
	PGPChar8 *			dn,
	PGPChar8 *			type,
	PGPChar8 *			value,
	PGPldapMessageID *	messageID );

	/* ldap_compare_s */
	PGPError
PGPldapCompareSync(
	PGPldapContextRef	pgpLDAP,
	PGPChar8 *			dn,
	PGPChar8 *			type,
	PGPChar8 *			value,
	PGPBoolean *		equal );

	PGPError
PGPldapGetCompareResult(
	PGPldapContextRef	pgpLDAP,
	PGPldapMessageID	messageID,
	PGPBoolean *		equal );


/* Other functions */
	/* ldap_result */
	PGPError
PGPldapGetResult(
	PGPldapContextRef		pgpLDAP,
	PGPldapMessageID		messageID,
	PGPBoolean				all,
	PGPSocketsTimeValue *	timeout,
	PGPldapMessageRef		result,
	PGPldapType *			messageType );

	/* ldap_abandon */
	PGPError
PGPldapAbandon(
	PGPldapContextRef	pgpLDAP,
	PGPldapMessageID	messageID );

	/* ldap_msgid */
	PGPError
PGPldapGetMessageID(
	PGPldapContextRef	pgpLDAP, 
	PGPldapMessageRef	result,
	PGPldapMessageID *	messageID );

	/* ldap_is_ldap_url */
	PGPError
PGPldapIsLDAPURL(
	PGPldapContextRef	pgpLDAP,
	PGPChar8 *			url,
	PGPBoolean *		isURL );

	/* ldap_url_parse */
	PGPError
PGPldapURLParse( 
	PGPldapContextRef	pgpLDAP,
	PGPChar8 *			url,
	PGPldapURLDesc **	outLDAPDesc );	/* Should be freed by PGPFreeLDAPURLDesc */

	/* ldap_url_search */
	PGPError
PGPldapURLSearch(
	PGPldapContextRef	pgpLDAP,
	PGPChar8 *			url,
	PGPBoolean			attrsOnly,
	PGPldapMessageID *	messageID );

	/* ldap_url_search */
	PGPError
PGPldapURLSearchSync(
	PGPldapContextRef	pgpLDAP,
	PGPChar8 *			url,
	PGPBoolean			attrsOnly,
	PGPldapMessageRef	resultMessage );

	/* ldap_url_search */
	PGPError
PGPldapURLSearchSyncTimeout(
	PGPldapContextRef		pgpLDAP,
	PGPChar8 *				url,
	PGPBoolean				attrsOnly,
	PGPSocketsTimeValue *	tv,
	PGPldapMessageRef		resultMessage );

	/* ldap_explode_dn */
	PGPError
PGPldapExplodeDN(
	PGPldapContextRef	pgpLDAP,
	PGPChar8 *			dn,
	PGPBoolean			noTypes,
	PGPChar8 ***		components );	/* Should be freed by PGPFreeLDAPValues */

	/* ldap_sort_entries */
	PGPError
PGPldapSortEntries(
	PGPldapContextRef	pgpLDAP,
	PGPldapMessageRef	chain,
	PGPChar8 *			attr,
	PGPInt32			(*compare)( PGPChar8 ** a, PGPChar8 ** b ),
	PGPldapMessageRef	sortedChain );

	/* ldap_sort_values */
	PGPError
PGPldapSortValues(
	PGPldapContextRef	pgpLDAP,
	PGPChar8 **			vals,
	PGPInt32			(*compare)( PGPChar8 ** a, PGPChar8 ** b ),
	PGPChar8 ***		sortedVals );

	/* ldap_sort_strcasecmp */
	PGPInt32
PGPldapCompareStringsIgnoreCase(
	PGPChar8 **			s1,
	PGPChar8 **			s2 );

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpLDAP_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
