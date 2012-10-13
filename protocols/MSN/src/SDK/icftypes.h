

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0366 */
/* Compiler settings for icftypes.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#if defined(_MSC_VER) 
#pragma warning( disable: 4049 )  /* more than 64k source lines */
#endif


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__


#ifndef __icftypes_h__
#define __icftypes_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

/* header files for imported files */
#include "wtypes.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

/* interface __MIDL_itf_icftypes_0000 */
/* [local] */ 

typedef 
enum NET_FW_POLICY_TYPE_
    {	NET_FW_POLICY_GROUP	= 0,
	NET_FW_POLICY_LOCAL	= NET_FW_POLICY_GROUP + 1,
	NET_FW_POLICY_EFFECTIVE	= NET_FW_POLICY_LOCAL + 1,
	NET_FW_POLICY_TYPE_MAX	= NET_FW_POLICY_EFFECTIVE + 1
    } 	NET_FW_POLICY_TYPE;

typedef 
enum NET_FW_PROFILE_TYPE_
    {	NET_FW_PROFILE_DOMAIN	= 0,
	NET_FW_PROFILE_STANDARD	= NET_FW_PROFILE_DOMAIN + 1,
	NET_FW_PROFILE_CURRENT	= NET_FW_PROFILE_STANDARD + 1,
	NET_FW_PROFILE_TYPE_MAX	= NET_FW_PROFILE_CURRENT + 1
    } 	NET_FW_PROFILE_TYPE;

typedef 
enum NET_FW_IP_VERSION_
    {	NET_FW_IP_VERSION_V4	= 0,
	NET_FW_IP_VERSION_V6	= NET_FW_IP_VERSION_V4 + 1,
	NET_FW_IP_VERSION_ANY	= NET_FW_IP_VERSION_V6 + 1,
	NET_FW_IP_VERSION_MAX	= NET_FW_IP_VERSION_ANY + 1
    } 	NET_FW_IP_VERSION;

typedef 
enum NET_FW_SCOPE_
    {	NET_FW_SCOPE_ALL	= 0,
	NET_FW_SCOPE_LOCAL_SUBNET	= NET_FW_SCOPE_ALL + 1,
	NET_FW_SCOPE_CUSTOM	= NET_FW_SCOPE_LOCAL_SUBNET + 1,
	NET_FW_SCOPE_MAX	= NET_FW_SCOPE_CUSTOM + 1
    } 	NET_FW_SCOPE;

typedef 
enum NET_FW_IP_PROTOCOL_
    {	NET_FW_IP_PROTOCOL_TCP	= 6,
	NET_FW_IP_PROTOCOL_UDP	= 17
    } 	NET_FW_IP_PROTOCOL;

typedef 
enum NET_FW_SERVICE_TYPE_
    {	NET_FW_SERVICE_FILE_AND_PRINT	= 0,
	NET_FW_SERVICE_UPNP	= NET_FW_SERVICE_FILE_AND_PRINT + 1,
	NET_FW_SERVICE_REMOTE_DESKTOP	= NET_FW_SERVICE_UPNP + 1,
	NET_FW_SERVICE_NONE	= NET_FW_SERVICE_REMOTE_DESKTOP + 1,
	NET_FW_SERVICE_TYPE_MAX	= NET_FW_SERVICE_NONE + 1
    } 	NET_FW_SERVICE_TYPE;



extern RPC_IF_HANDLE __MIDL_itf_icftypes_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_icftypes_0000_v0_0_s_ifspec;

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


