
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

#ifndef _MW_ERROR_H
#define _MW_ERROR_H


/** @file mw_error.h

    Common error code constants used by Meanwhile.

    Not all of these error codes (or even many, really) will ever
    actually appear from Meanwhile. These are taken directly from the
    houri draft, along with the minimal explanation for each.
*/


#include <glib.h>


#ifdef __cplusplus
extern "C" {
#endif


/** reference to a new string appropriate for the given error code.*/
char* mwError(guint32 code);


/* 8.3 Constants */
/* 8.3.1 Error Codes */
/* 8.3.1.1 General error/success codes */

/** @enum ERR_GENERAL
    general error codes */
enum ERR_GENERAL {
  ERR_SUCCESS                = 0x00000000,
  ERR_FAILURE                = 0x80000000,
  ERR_REQUEST_DELAY          = 0x00000001,
  ERR_REQUEST_INVALID        = 0x80000001,
  ERR_NOT_LOGGED_IN          = 0x80000002,
  ERR_NOT_AUTHORIZED         = 0x80000003,
  ERR_ABORT                  = 0x80000004,
  ERR_NO_ELEMENT             = 0x80000005,
  ERR_NO_USER                = 0x80000006,
  ERR_BAD_DATA               = 0x80000007,
  ERR_NOT_IMPLEMENTED        = 0x80000008,
  ERR_UNKNOWN_ERROR          = 0x80000009, /* what is this? */
  ERR_STARVING               = 0x8000000a,
  ERR_CHANNEL_NO_SUPPORT     = 0x8000000b,
  ERR_CHANNEL_EXISTS         = 0x8000000c,
  ERR_SERVICE_NO_SUPPORT     = 0x8000000d,
  ERR_PROTOCOL_NO_SUPPORT    = 0x8000000e,
  ERR_PROTOCOL_NO_SUPPORT2   = 0x8000000f, /* duplicate? */
  ERR_VERSION_NO_SUPPORT     = 0x80000010,
  ERR_USER_SKETCHY           = 0x80000011,
  ERR_ALREADY_INITIALIZED    = 0x80000013,
  ERR_NOT_OWNER              = 0x80000014,
  ERR_TOKEN_INVALID          = 0x80000015,
  ERR_TOKEN_EXPIRED          = 0x80000016,
  ERR_TOKEN_IP_MISMATCH      = 0x80000017,
  ERR_PORT_IN_USE            = 0x80000018,
  ERR_NETWORK_DEAD           = 0x80000019,
  ERR_NO_MASTER_CHANNEL      = 0x8000001a,
  ERR_ALREADY_SUBSCRIBED     = 0x8000001b,
  ERR_NOT_SUBSCRIBED         = 0x8000001c,
  ERR_ENCRYPT_NO_SUPPORT     = 0x8000001d,
  ERR_ENCRYPT_UNINITIALIZED  = 0x8000001e,
  ERR_ENCRYPT_UNACCEPTABLE   = 0x8000001f,
  ERR_ENCRYPT_INVALID        = 0x80000020,
  ERR_NO_COMMON_ENCRYPT      = 0x80000021,
  ERR_CHANNEL_DESTROYED      = 0x80000022,
  ERR_CHANNEL_REDIRECTED     = 0x80000023
};


/* 8.3.1.2 Connection/disconnection errors */

#define VERSION_MISMATCH     0x80000200
#define INSUF_BUFFER         0x80000201
#define NOT_IN_USE           0x80000202
#define INSUF_SOCKET         0x80000203
#define HARDWARE_ERROR       0x80000204
#define NETWORK_DOWN         0x80000205
#define HOST_DOWN            0x80000206
#define HOST_UNREACHABLE     0x80000207
#define TCPIP_ERROR          0x80000208
#define FAT_MESSAGE          0x80000209
#define PROXY_ERROR          0x8000020A
#define SERVER_FULL          0x8000020B
#define SERVER_NORESPOND     0x8000020C
#define CANT_CONNECT         0x8000020D
#define USER_REMOVED         0x8000020E
#define PROTOCOL_ERROR       0x8000020F
#define USER_RESTRICTED      0x80000210
#define INCORRECT_LOGIN      0x80000211
#define ENCRYPT_MISMATCH     0x80000212
#define USER_UNREGISTERED    0x80000213
#define VERIFICATION_DOWN    0x80000214
#define USER_TOO_IDLE        0x80000216
#define GUEST_IN_USE         0x80000217
#define USER_EXISTS          0x80000218
#define USER_RE_LOGIN        0x80000219
#define BAD_NAME             0x8000021A
#define REG_MODE_NS          0x8000021B
#define WRONG_USER_PRIV      0x8000021C
#define NEED_EMAIL           0x8000021D
#define DNS_ERROR            0x8000021E
#define DNS_FATAL_ERROR      0x8000021F
#define DNS_NOT_FOUND        0x80000220
#define CONNECTION_BROKEN    0x80000221
#define CONNECTION_ABORTED   0x80000222
#define CONNECTION_REFUSED   0x80000223
#define CONNECTION_RESET     0x80000224
#define CONNECTION_TIMED     0x80000225
#define CONNECTION_CLOSED    0x80000226
#define MULTI_SERVER_LOGIN   0x80000227
#define MULTI_SERVER_LOGIN2  0x80000228
#define MULTI_LOGIN_COMP     0x80000229
#define MUTLI_LOGIN_ALREADY  0x8000022A
#define SERVER_BROKEN        0x8000022B
#define SERVER_PATH_OLD      0x8000022C
#define APPLET_LOGOUT        0x8000022D


/* 8.3.1.3 Client error codes */

/** @enum ERR_CLIENT
    Client error codes */
enum ERR_CLIENT {
  ERR_CLIENT_USER_GONE       = 0x80002000, /* user isn't here */
  ERR_CLIENT_USER_DND        = 0x80002001, /* user is DND */
  ERR_CLIENT_USER_ELSEWHERE  = 0x80002002, /* already logged in elsewhere */
};


/* 8.3.1.4 IM error codes */

/** @enum ERR_IM
    IM error codes */
enum ERR_IM {
  ERR_IM_COULDNT_REGISTER    = 0x80002003,
  ERR_IM_ALREADY_REGISTERED  = 0x80002004,

  /** apparently, this is used to mean that the requested feature (per
      the channel create addtl data) is not supported by the client on
      the other end of the IM channel */
  ERR_IM_NOT_REGISTERED      = 0x80002005,
};


#ifdef __cplusplus
}
#endif


#endif /* _MW_ERROR_H */
