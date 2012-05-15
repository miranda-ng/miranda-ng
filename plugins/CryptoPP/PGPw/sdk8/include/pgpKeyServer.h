/*____________________________________________________________________________
        Copyright (C) 2002 PGP Corporation
        All rights reserved.

        $Id: pgpKeyServer.h,v 1.20 2004/01/23 08:12:49 dallen Exp $
____________________________________________________________________________*/

#ifndef Included_pgpKeyServer_h
#define Included_pgpKeyServer_h

#include "pgpOptionList.h"
#include "pgpErrors.h"
#include "pgpGroups.h"
#include "pgpTLS.h"


enum PGPKeyServerState_
{
	kPGPKeyServerState_Invalid						= 0,
	kPGPKeyServerState_Opening						= 1,
	kPGPKeyServerState_Querying						= 2,
	kPGPKeyServerState_ReceivingResults				= 3,
	kPGPKeyServerState_ProcessingResults			= 4,
	kPGPKeyServerState_Uploading					= 5,
	kPGPKeyServerState_Deleting						= 6,
	kPGPKeyServerState_Disabling					= 7,
	kPGPKeyServerState_Closing						= 8,

	kPGPKeyServerState_TLSUnableToSecureConnection	= 9,
	kPGPKeyServerState_TLSConnectionSecured			= 10,

	PGP_ENUM_FORCE(PGPKeyServerState_)
};

PGPENUM_TYPEDEF(PGPKeyServerState_, PGPKeyServerState);

enum PGPKeyServerProtocol_
{
	kPGPKeyServerProtocol_Invalid			= 0,
	kPGPKeyServerProtocol_LDAP				= 1,
	kPGPKeyServerProtocol_HTTP				= 2,
	kPGPKeyServerProtocol_LDAPS				= 3,
	kPGPKeyServerProtocol_HTTPS				= 4,
	
	PGP_ENUM_FORCE( PGPKeyServerProtocol_ )
};

PGPENUM_TYPEDEF( PGPKeyServerProtocol_, PGPKeyServerProtocol );

enum PGPKeyServerClass_
{
	kPGPKeyServerClass_Invalid		= 0,
	kPGPKeyServerClass_PGP			= 1,
	kPGPKeyServerClass_LDAPX509		= 2,
	kPGPKeyServerClass_LDAPPGP		= 3,	/* deprecated, use kPGPKeyServerClass_PGP instead */

	/* CA types */
	kPGPKeyServerClass_NetToolsCA	= 10,
	kPGPKeyServerClass_Verisign		= 11,
	kPGPKeyServerClass_Entrust		= 12,
	kPGPKeyServerClass_Netscape		= 13,
	kPGPKeyServerClass_Microsoft	= 14,
	
	PGP_ENUM_FORCE( PGPKeyServerClass_ )
};

PGPENUM_TYPEDEF( PGPKeyServerClass_, PGPKeyServerClass );

enum PGPKeyServerKeySpace_ /* These are only valid for LDAP keyservers */
{
	kPGPKeyServerKeySpace_Invalid	= 0,
	kPGPKeyServerKeySpace_Default	= 1,
	kPGPKeyServerKeySpace_Normal	= 2,
	kPGPKeyServerKeySpace_Pending	= 3,

	PGP_ENUM_FORCE( PGPKeyServerKeySpace_ )
};

PGPENUM_TYPEDEF( PGPKeyServerKeySpace_, PGPKeyServerKeySpace );

enum PGPKeyServerAccessType_ /* These are only valid for LDAP keyservers */
{
	kPGPKeyServerAccessType_Invalid			= 0,
	kPGPKeyServerAccessType_Default			= 1,
	kPGPKeyServerAccessType_Normal			= 2,
	kPGPKeyServerAccessType_Administrator	= 3,

	PGP_ENUM_FORCE( PGPKeyServerAccessType_ )
};

PGPENUM_TYPEDEF( PGPKeyServerAccessType_, PGPKeyServerAccessType );

enum PGPProxyServerType_
{
	kPGPProxyServerType_Invalid     = 0,
	kPGPProxyServerType_HTTP        = 1,
	kPGPProxyServerType_HTTPS       = 2,

	PGP_ENUM_FORCE(PGPProxyServerType_)
};

PGPENUM_TYPEDEF(PGPProxyServerType_, PGPProxyServerType);

/*	PGPKeyServerMonitorValues are null terminated linked lists.
	The values member is a null terminated array of char*s.
*/

typedef struct PGPKeyServerMonitorValues
{
	PGPChar8 *							name;
	PGPChar8 **							values;
	struct PGPKeyServerMonitorValues *	next;
} PGPKeyServerMonitorValues;

typedef struct PGPKeyServerMonitor 
{
	PGPKeyServerRef				keyServerRef;
	PGPKeyServerMonitorValues *	valuesHead;
} PGPKeyServerMonitor;


typedef struct PGPKeyServerThreadStorage *	PGPKeyServerThreadStorageRef;
# define kInvalidPGPKeyServerThreadStorageRef				\
			((PGPKeyServerThreadStorageRef) NULL)
#define PGPKeyServerThreadStorageRefIsValid(ref)			\
			((ref) != kInvalidPGPKeyServerThreadStorageRef)

PGP_BEGIN_C_DECLARATIONS


/*	Use the idle event handler to receive periodic idle events during
	network calls. Usually this is used only in non-preemptive multi-tasking
	OSes to allow yielding in threads. Pre-emptive multi-tasking systems
	should probably not use the call as it interrupts the efficient wait state
	of threads waiting on network calls.
	
	Idle event handlers need to be added on a per thread basis.
	
	Returning an error from the idle event handler will cause the keyserver
	to quit processing and to return a kPGPError_UserAbort. */
PGPError			PGPSetKeyServerIdleEventHandler(
						PGPEventHandlerProcPtr inCallback,
						PGPUserValue inUserData);

PGPError			PGPGetKeyServerIdleEventHandler(
						PGPEventHandlerProcPtr * outCallback,
						PGPUserValue * outUserData);

/* Network library options */

#undef			PGPONetURL
PGPOptionListRef	PGPONetURL(PGPContextRef context, const PGPChar8 *url);

#undef			PGPONetHostName
PGPOptionListRef	PGPONetHostName(PGPContextRef context,
							const PGPChar8 *hostName, PGPUInt16 port);

PGPOptionListRef	PGPONetHostAddress(PGPContextRef context,
							PGPUInt32 hostAddress, PGPUInt16 port);

PGPOptionListRef	PGPONetConnectTimeout(PGPContextRef context,
							PGPUInt32 timeout);

PGPOptionListRef	PGPONetReadTimeout(PGPContextRef context,
							PGPUInt32 timeout);

PGPOptionListRef	PGPONetWriteTimeout(PGPContextRef context,
							PGPUInt32 timeout);

PGPOptionListRef	PGPOKeyServerProtocol(PGPContextRef context,
							PGPKeyServerProtocol serverProtocol);

PGPOptionListRef	PGPOKeyServerKeySpace(PGPContextRef context,
							PGPKeyServerKeySpace serverSpace);

#undef			PGPOKeyServerKeyStoreDN
PGPOptionListRef	PGPOKeyServerKeyStoreDN(PGPContextRef context,
							const PGPChar8 *szKeyStoreDn);

PGPOptionListRef	PGPOKeyServerAccessType(PGPContextRef context,
							PGPKeyServerAccessType accessType);
							
PGPOptionListRef	PGPOKeyServerCAKey(PGPContextRef context,
							PGPKeyDBObjRef caKeyDBObjRef);

PGPOptionListRef	PGPOKeyServerRequestKey(PGPContextRef context,
							PGPKeyDBObjRef requestKeyDBObjRef);

PGPOptionListRef	PGPOKeyServerSearchKey(PGPContextRef context,
							PGPKeyDBObjRef searchKeyDBObjRef);
							
PGPOptionListRef	PGPOKeyServerSearchFilter(PGPContextRef context,
							PGPFilterRef searchFilter);

/* Static storage creation */
PGPError		PGPKeyServerCreateThreadStorage(
					PGPKeyServerThreadStorageRef * outPreviousStorage);
PGPError		PGPKeyServerDisposeThreadStorage(
					PGPKeyServerThreadStorageRef inPreviousStorage);
					
/* Initialize and close the keyserver library */
PGPError			PGPKeyServerInit(void);

PGPError			PGPKeyServerCleanup(void);


/*	Creating and freeing a keyserver ref. */
PGPError			PGPNewKeyServer(
						PGPContextRef 			inContext,
						PGPKeyServerClass		inClass,
						PGPKeyServerRef 		*outKeyServerRef,
						PGPOptionListRef		firstOption,
						... );
						
PGPError 			PGPFreeKeyServer(PGPKeyServerRef inKeyServerRef);
PGPError			PGPIncKeyServerRefCount(PGPKeyServerRef inKeyServerRef);


/*	Set and get the keyserver's event handler. Note that returning an error
	for a keyserver event will abort the current call. */
PGPError			PGPSetKeyServerEventHandler(
						PGPKeyServerRef inKeyServerRef,
						PGPEventHandlerProcPtr inCallback,
						PGPUserValue inUserData);

PGPError			PGPGetKeyServerEventHandler(
						PGPKeyServerRef inKeyServerRef,
						PGPEventHandlerProcPtr * outCallback,
						PGPUserValue * outUserData);
						
						
/*	Canceling a call to a keyserver. This is the only call that can be made
	to a keyserver that is currently in another call. Also, once you have
	returned from a canceled call, you may only close the keyserver. */
PGPError			PGPCancelKeyServerCall(PGPKeyServerRef inKeyServerRef);
						
						
/*	Opening and closing the keyserver. A keyserver ref can be opened and
	closed multiple times as necessary. */
PGPError			PGPKeyServerOpen(PGPKeyServerRef inKeyServerRef,
						PGPtlsSessionRef inTLSSessionRef);

PGPError			PGPKeyServerClose(PGPKeyServerRef inKeyServerRef);


/*	Get keyserver info. */
PGPError			PGPGetKeyServerTLSSession(PGPKeyServerRef inKeyServerRef,
						PGPtlsSessionRef * outTLSSessionRef);

PGPError			PGPGetKeyServerProtocol(PGPKeyServerRef inKeyServerRef,
						PGPKeyServerProtocol * outType);

PGPError			PGPGetKeyServerAccessType(PGPKeyServerRef inKeyServerRef,
						PGPKeyServerAccessType * outAccessType);

PGPError			PGPGetKeyServerKeySpace(PGPKeyServerRef inKeyServerRef,
						PGPKeyServerKeySpace * outKeySpace);

PGPError			PGPGetKeyServerPort(PGPKeyServerRef inKeyServerRef,
						PGPUInt16 * outPort);

#undef				PGPGetKeyServerHostName
PGPError			PGPGetKeyServerHostName(PGPKeyServerRef inKeyServerRef,
						PGPChar8 ** outHostName); /* Use PGPFreeData to free */

PGPError			PGPGetKeyServerAddress(PGPKeyServerRef inKeyServerRef,
						PGPUInt32 * outAddress);
					
#undef				PGPGetKeyServerPath
PGPError			PGPGetKeyServerPath(PGPKeyServerRef inKeyServerRef,
						PGPChar8 ** outPath); /* Use PGPFreeData to free */

PGPContextRef		PGPGetKeyServerContext(PGPKeyServerRef inKeyServerRef);

/*	If there was an error string returned from the server, you can get it with
	this function. Note that if there is no string, the function will return
	kPGPError_NoErr and *outErrorString will be	NULL */
#undef				PGPGetLastKeyServerErrorString
PGPError			PGPGetLastKeyServerErrorString(
						PGPKeyServerRef inKeyServerRef,
						PGPChar8 ** outErrorString); /* Use PGPFreeData to free */


/* These functions may be used with both HTTP and LDAP keyservers */
PGPError 			PGPQueryKeyServer(PGPKeyServerRef inKeyServerRef, 
						PGPFilterRef inFilterRef, 
						PGPKeyDBRef *searchResultsDB);

PGPError 			PGPUploadToKeyServer(PGPKeyServerRef inKeyServerRef, 
						PGPKeySetRef inKeysToUpload, 
						PGPKeySetRef * outKeysThatFailed);
						

/* These functions may only be used with LDAP keyservers */
PGPError			PGPDeleteFromKeyServer(PGPKeyServerRef inKeyServerRef,
						PGPKeySetRef inKeysToDelete,
						PGPKeySetRef * outKeysThatFailed);

PGPError			PGPDisableFromKeyServer(PGPKeyServerRef inKeyServerRef,
						PGPKeySetRef inKeysToDisable,
						PGPKeySetRef * outKeysThatFailed);
						
PGPError			PGPSendGroupsToServer(PGPKeyServerRef inKeyServerRef,
						PGPGroupSetRef inGroupSetRef);
						
PGPError			PGPRetrieveGroupsFromServer(
						PGPKeyServerRef inKeyServerRef,
						PGPGroupSetRef * outGroupSetRef);

PGPError			PGPNewServerMonitor(PGPKeyServerRef inKeyServerRef,
						PGPKeyServerMonitor ** outMonitor);
						
PGPError			PGPFreeServerMonitor(PGPKeyServerMonitor * inMonitor);

/* X.509 Certificate Request functions */

PGPError			PGPSendCertificateRequest( 
						PGPKeyServerRef 	inKeyServerRef,
						PGPOptionListRef	firstOption,
						... );

PGPError			PGPRetrieveCertificate( 
						PGPKeyServerRef 	inKeyServerRef,
						PGPOptionListRef	firstOption,
						... );

PGPError			PGPRetrieveCertificateRevocationList( 
						PGPKeyServerRef 	inKeyServerRef,
						PGPOptionListRef	firstOption,
						... );
/* Queries HTTP proxy information. proxyAddress must be freed with PGPFreeData */
#undef				PGPGetProxyServer
PGPError 			PGPGetProxyServer(
						PGPContextRef context, PGPProxyServerType type,
						PGPChar8 **proxyAddress, PGPUInt16 *proxyPort );
												
PGP_END_C_DECLARATIONS

#endif
