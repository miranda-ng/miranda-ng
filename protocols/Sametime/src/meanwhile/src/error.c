
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

#include <stdio.h>
#include <string.h>

#include "mw_error.h"


static char *err_to_str(guint32 code) {
  static char b[11]; /* 0x12345678 + NULL terminator */
  sprintf((char *) b, "0x%08x", code);
  b[10] = '\0';
  return b;
}


#define CASE(val, str) \
case val: \
  m = str; \
  break;


char* mwError(guint32 code) {
  const char *m;

  switch(code) {

    /* 8.3.1.1 General error/success codes */
    CASE(ERR_SUCCESS, "Success");
    CASE(ERR_FAILURE, "General failure");
    CASE(ERR_REQUEST_DELAY, "Request delayed");
    CASE(ERR_REQUEST_INVALID, "Request is invalid");
    CASE(ERR_NOT_AUTHORIZED, "Not authorized");
    CASE(ERR_NO_USER, "User is not online");
    CASE(ERR_CHANNEL_NO_SUPPORT, "Requested channel is not supported");
    CASE(ERR_CHANNEL_EXISTS, "Requested channel already exists");
    CASE(ERR_SERVICE_NO_SUPPORT, "Requested service is not supported");
    CASE(ERR_PROTOCOL_NO_SUPPORT, "Requested protocol is not supported");
    CASE(ERR_VERSION_NO_SUPPORT, "Version is not supported");
    CASE(ERR_USER_SKETCHY, "User is invalid or not trusted");
    CASE(ERR_ALREADY_INITIALIZED, "Already initialized");
    CASE(ERR_ENCRYPT_NO_SUPPORT, "Encryption method not supported");
    CASE(ERR_NO_COMMON_ENCRYPT, "No common encryption method");
    
    /* 8.3.1.2 Connection/disconnection errors */
    CASE(VERSION_MISMATCH, "Version mismatch");
    CASE(FAT_MESSAGE, "Message is too large");
    CASE(CONNECTION_BROKEN, "Connection broken");
    CASE(CONNECTION_ABORTED, "Connection aborted");
    CASE(CONNECTION_REFUSED, "Connection refused");
    CASE(CONNECTION_RESET, "Connection reset");
    CASE(CONNECTION_TIMED, "Connection timed out");
    CASE(CONNECTION_CLOSED, "Connection closed");
    CASE(INCORRECT_LOGIN, "Incorrect Username/Password");
    CASE(VERIFICATION_DOWN, "Login verification down or unavailable");
    CASE(GUEST_IN_USE, "The guest name is currently being used");
    CASE(MULTI_SERVER_LOGIN, "Login to two different servers concurrently");
    CASE(MULTI_SERVER_LOGIN2, "Login to two different servers concurrently");
    CASE(SERVER_BROKEN, "Server misconfiguration");

    /* 8.3.1.3 Client error codes */
    CASE(ERR_CLIENT_USER_GONE, "User is not online");
    CASE(ERR_CLIENT_USER_DND, "User is in Do Not Disturb mode");
    CASE(ERR_CLIENT_USER_ELSEWHERE, "Already logged in elsewhere");

    /* 8.3.1.4 IM error codes */
    CASE(ERR_IM_COULDNT_REGISTER, "Cannot register a reserved type");
    CASE(ERR_IM_ALREADY_REGISTERED, "Requested type is already registered");
    CASE(ERR_IM_NOT_REGISTERED, "Requested type is not registered");

  default:
    m = err_to_str(code);
  }

  return g_strdup(m);
}


#undef CASE
