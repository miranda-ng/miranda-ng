
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

/// Miranda NG development start
// LPGEN - don't want depend meanwhile from miranda headers, but want to have translatable strings
#define LPGEN(s)			s

char* mwError(guint32 code) {
  const char *m;

  switch(code) {

    /* 8.3.1.1 General error/success codes */
    CASE(ERR_SUCCESS, LPGEN("Success"));
	CASE(ERR_FAILURE, LPGEN("General failure"));
	CASE(ERR_REQUEST_DELAY, LPGEN("Request delayed"));
	CASE(ERR_REQUEST_INVALID, LPGEN("Request is invalid"));
	CASE(ERR_NOT_LOGGED_IN, LPGEN("Not logged in"));
	CASE(ERR_NOT_AUTHORIZED, LPGEN("Not authorized"));
	CASE(ERR_ABORT, LPGEN("Operation aborted"));
	CASE(ERR_NO_ELEMENT, LPGEN("No element"));
	CASE(ERR_NO_USER, LPGEN("User is not online"));
	CASE(ERR_BAD_DATA, LPGEN("Invalid data"));
	CASE(ERR_NOT_IMPLEMENTED, LPGEN("Not implemented"));
	CASE(ERR_UNKNOWN_ERROR, LPGEN("Unknown error"));
	CASE(ERR_STARVING, LPGEN("Not enough resources"));
	CASE(ERR_CHANNEL_NO_SUPPORT, LPGEN("Requested channel is not supported"));
	CASE(ERR_CHANNEL_EXISTS, LPGEN("Requested channel already exists"));
	CASE(ERR_SERVICE_NO_SUPPORT, LPGEN("Requested service is not supported"));
	CASE(ERR_PROTOCOL_NO_SUPPORT, LPGEN("Requested protocol is not supported"));
	CASE(ERR_PROTOCOL_NO_SUPPORT2, LPGEN("Requested protocol is not supported"));
	CASE(ERR_VERSION_NO_SUPPORT, LPGEN("Version is not supported"));
    CASE(ERR_USER_SKETCHY, LPGEN("User is invalid or not trusted"));
    CASE(ERR_ALREADY_INITIALIZED, LPGEN("Already initialized"));
	CASE(ERR_NOT_OWNER, LPGEN("Not an owner"));
	CASE(ERR_TOKEN_INVALID, LPGEN("Invalid token"));
	CASE(ERR_TOKEN_EXPIRED, LPGEN("Token expired"));
	CASE(ERR_TOKEN_IP_MISMATCH, LPGEN("Token mismatch"));
	CASE(ERR_PORT_IN_USE, LPGEN("Port in use"));
	CASE(ERR_NETWORK_DEAD, LPGEN("Network error"));
	CASE(ERR_NO_MASTER_CHANNEL, LPGEN("Master channel error"));
	CASE(ERR_ALREADY_SUBSCRIBED, LPGEN("Already subscribed"));
	CASE(ERR_NOT_SUBSCRIBED, LPGEN("Not subscribed"));
    CASE(ERR_ENCRYPT_NO_SUPPORT, LPGEN("Encryption method not supported"));
	CASE(ERR_ENCRYPT_UNINITIALIZED, LPGEN("Encryption not initialized"));
	CASE(ERR_ENCRYPT_UNACCEPTABLE, LPGEN("Encryption too low"));
	CASE(ERR_ENCRYPT_INVALID, LPGEN("Invalid encrypted data"));
    CASE(ERR_NO_COMMON_ENCRYPT, LPGEN("No common encryption method"));
	CASE(ERR_CHANNEL_DESTROYED, LPGEN("Channel destroyed"));
	CASE(ERR_CHANNEL_REDIRECTED, LPGEN("Channel redirected"));
	CASE(ERR_INCORRECT_ENTRY, LPGEN("Incorrect entry"));

    /* 8.3.1.2 Connection/disconnection errors */
    CASE(VERSION_MISMATCH, LPGEN("Version mismatch"));
	CASE(INSUF_BUFFER, LPGEN("Not enough buffers memory"));
	CASE(NOT_IN_USE, LPGEN("Not in use"));
	CASE(INSUF_SOCKET, LPGEN("Not enough sockets"));
	CASE(HARDWARE_ERROR, LPGEN("Hardware error"));
	CASE(NETWORK_DOWN, LPGEN("Network error"));
	CASE(HOST_DOWN, LPGEN("Host error"));
	CASE(HOST_UNREACHABLE, LPGEN("Host unreachable"));
	CASE(TCPIP_ERROR, LPGEN("Internet protocol error"));
    CASE(FAT_MESSAGE, LPGEN("Message is too large"));
	CASE(PROXY_ERROR, LPGEN("Proxy error"));
	CASE(SERVER_FULL, LPGEN("Server full"));
	CASE(SERVER_NORESPOND, LPGEN("Server not responding"));
	CASE(CANT_CONNECT, LPGEN("Connection error"));
	CASE(USER_REMOVED, LPGEN("User removed"));
	CASE(PROTOCOL_ERROR, LPGEN("Sametime protocol error"));
	CASE(USER_RESTRICTED, LPGEN("User restricted"));
    CASE(INCORRECT_LOGIN, LPGEN("Incorrect Username/Password"));
	CASE(ENCRYPT_MISMATCH, LPGEN("Encryption mismatch"));
	CASE(USER_UNREGISTERED, LPGEN("User unregistered"));
    CASE(VERIFICATION_DOWN, LPGEN("Login verification down or unavailable"));
	CASE(USER_TOO_IDLE, LPGEN("User too idle"));
    CASE(GUEST_IN_USE, LPGEN("The guest name is currently being used"));
	CASE(USER_EXISTS, LPGEN("User exists"));
	CASE(USER_RE_LOGIN, LPGEN("User relogin"));
	CASE(BAD_NAME, LPGEN("Bad name"));
	CASE(REG_MODE_NS, LPGEN("Registration error"));
	CASE(WRONG_USER_PRIV, LPGEN("Privilege error"));
	CASE(NEED_EMAIL, LPGEN("Need email"));
	CASE(DNS_ERROR, LPGEN("DNS error"));
	CASE(DNS_FATAL_ERROR, LPGEN("DNS fatal error"));
	CASE(DNS_NOT_FOUND, LPGEN("DNS not found"));
	CASE(CONNECTION_BROKEN, LPGEN("Connection broken"));
	CASE(CONNECTION_ABORTED, LPGEN("Connection aborted"));
	CASE(CONNECTION_REFUSED, LPGEN("Connection refused"));
	CASE(CONNECTION_RESET, LPGEN("Connection reset"));
	CASE(CONNECTION_TIMED, LPGEN("Connection timed out"));
	CASE(CONNECTION_CLOSED, LPGEN("Connection closed"));
    CASE(MULTI_SERVER_LOGIN, LPGEN("Login to two different servers concurrently (1)"));
    CASE(MULTI_SERVER_LOGIN2, LPGEN("Login to two different servers concurrently (2)"));
	CASE(MULTI_LOGIN_COMP, LPGEN("Already logged on, disconnected"));
	CASE(MUTLI_LOGIN_ALREADY, LPGEN("Already logged on"));
    CASE(SERVER_BROKEN, LPGEN("Server misconfiguration"));
	CASE(SERVER_PATH_OLD, LPGEN("Server needs upgrade"));
	CASE(APPLET_LOGOUT, LPGEN("Applet Logout"));

    /* 8.3.1.3 Client error codes */
    CASE(ERR_CLIENT_USER_GONE, LPGEN("User is not online"));
    CASE(ERR_CLIENT_USER_DND, LPGEN("User is in Do Not Disturb mode"));
    CASE(ERR_CLIENT_USER_ELSEWHERE, LPGEN("Already logged in elsewhere"));

    /* 8.3.1.4 IM error codes */
    CASE(ERR_IM_COULDNT_REGISTER, LPGEN("Cannot register a reserved type"));
    CASE(ERR_IM_ALREADY_REGISTERED, LPGEN("Requested type is already registered"));
    CASE(ERR_IM_NOT_REGISTERED, LPGEN("Requested type is not registered"));

	/* 8.3.1.5 Resolve error codes */
	CASE(ERR_RESOLVE_NOTCOMPLETED, LPGEN("Resolve not completed"));
	CASE(ERR_RESOLVE_NAMENOTUNIQUE, LPGEN("Resolve name not unique"));
	CASE(ERR_RESOLVE_NAMENOTRESOLVABLE, LPGEN("Resolve name not resolvable"));

  default:
    m = err_to_str(code);
  }

  return g_strdup(m);
}

char* mwErrorDesc(guint32 code) {
	const char *m;

	switch (code) {

		/* 8.3.1.1 General error/success codes */
		CASE(ERR_SUCCESS, LPGEN("Operation succeeded"));
		CASE(ERR_FAILURE, LPGEN("Operation failed"));
		CASE(ERR_REQUEST_DELAY, LPGEN("Request accepted but will be served later"));
		CASE(ERR_REQUEST_INVALID, LPGEN("Request is invalid due to invalid state or parameters"));
		CASE(ERR_NOT_LOGGED_IN, LPGEN("Not logged in to community"));
		CASE(ERR_NOT_AUTHORIZED, LPGEN("Unauthorized to perform an action or access a resource"));
		CASE(ERR_ABORT, LPGEN("Operation has been aborted"));
		CASE(ERR_NO_ELEMENT, LPGEN("The element is non-existent"));
		CASE(ERR_NO_USER, LPGEN("The user is non-existent"));
		CASE(ERR_BAD_DATA, LPGEN("The data are invalid or corrupted"));
		CASE(ERR_NOT_IMPLEMENTED, LPGEN("The requested feature is not implemented"));
		CASE(ERR_UNKNOWN_ERROR, LPGEN("Unknown error"));
		CASE(ERR_STARVING, LPGEN("Not enough resources to perform the operation"));
		CASE(ERR_CHANNEL_NO_SUPPORT, LPGEN("The requested channel is not supported"));
		CASE(ERR_CHANNEL_EXISTS, LPGEN("The requested channel already exists"));
		CASE(ERR_SERVICE_NO_SUPPORT, LPGEN("The requested service is not supported"));
		CASE(ERR_PROTOCOL_NO_SUPPORT, LPGEN("The requested protocol is not supported (1)"));
		CASE(ERR_PROTOCOL_NO_SUPPORT2, LPGEN("The requested protocol is not supported (2)"));
		CASE(ERR_VERSION_NO_SUPPORT, LPGEN("The version is not supported"));
		CASE(ERR_USER_SKETCHY, LPGEN("User is invalid or not trusted"));
		CASE(ERR_ALREADY_INITIALIZED, LPGEN("Already initialized"));
		CASE(ERR_NOT_OWNER, LPGEN("Not an owner of the requested resource"));
		CASE(ERR_TOKEN_INVALID, LPGEN("Invalid token"));
		CASE(ERR_TOKEN_EXPIRED, LPGEN("Token has expired"));
		CASE(ERR_TOKEN_IP_MISMATCH, LPGEN("Token IP mismatch"));
		CASE(ERR_PORT_IN_USE, LPGEN("WK port is in use"));
		CASE(ERR_NETWORK_DEAD, LPGEN("Low-level network error occurred"));
		CASE(ERR_NO_MASTER_CHANNEL, LPGEN("No master channel exists"));
		CASE(ERR_ALREADY_SUBSCRIBED, LPGEN("Already subscribed to object(s) or event(s)"));
		CASE(ERR_NOT_SUBSCRIBED, LPGEN("Not subscribed to object(s) or event(s)"));
		CASE(ERR_ENCRYPT_NO_SUPPORT, LPGEN("Encryption is not supported or failed unexpectedly"));
		CASE(ERR_ENCRYPT_UNINITIALIZED, LPGEN("Encryption mechanism has not been initialized yet"));
		CASE(ERR_ENCRYPT_UNACCEPTABLE, LPGEN("The requested encryption level is unacceptably low"));
		CASE(ERR_ENCRYPT_INVALID, LPGEN("The encryption data passed are invalid or corrupted"));
		CASE(ERR_NO_COMMON_ENCRYPT, LPGEN("There is no common encryption method"));
		CASE(ERR_CHANNEL_DESTROYED, LPGEN("The channel is destroyed after a recommendation is made connect elsewhere"));
		CASE(ERR_CHANNEL_REDIRECTED, LPGEN("The channel has been redirected to another destination"));
		CASE(ERR_INCORRECT_ENTRY, LPGEN("Incorrect entry for server in cluster document"));

		/* 8.3.1.3 Client error codes */
		CASE(VERSION_MISMATCH, LPGEN("Versions don't match"));
		CASE(INSUF_BUFFER, LPGEN("Not enough resources for connection (buffers)"));
		CASE(NOT_IN_USE, LPGEN("Not in use"));
		CASE(INSUF_SOCKET, LPGEN("Not enough resources for connection (socket id)"));
		CASE(HARDWARE_ERROR, LPGEN("Hardware error occurred"));
		CASE(NETWORK_DOWN, LPGEN("Network down"));
		CASE(HOST_DOWN, LPGEN("Host down"));
		CASE(HOST_UNREACHABLE, LPGEN("Host unreachable"));
		CASE(TCPIP_ERROR, LPGEN("TCP/IP protocol error"));
		CASE(FAT_MESSAGE, LPGEN("The message is too large"));
		CASE(PROXY_ERROR, LPGEN("Proxy error"));
		CASE(SERVER_FULL, LPGEN("Server is full"));
		CASE(SERVER_NORESPOND, LPGEN("Server is not responding"));
		CASE(CANT_CONNECT, LPGEN("Cannot connect"));
		CASE(USER_REMOVED, LPGEN("User has been removed from the server"));
		CASE(PROTOCOL_ERROR, LPGEN("Virtual Places protocol error"));
		CASE(USER_RESTRICTED, LPGEN("Cannot connect because user has been restricted"));
		CASE(INCORRECT_LOGIN, LPGEN("Incorrect login"));
		CASE(ENCRYPT_MISMATCH, LPGEN("Encryption mismatch"));
		CASE(USER_UNREGISTERED, LPGEN("User is unregistered"));
		CASE(VERIFICATION_DOWN, LPGEN("Verification service down"));
		CASE(USER_TOO_IDLE, LPGEN("User has been idle for too long"));
		CASE(GUEST_IN_USE, LPGEN("The guest name is currently being used"));
		CASE(USER_EXISTS, LPGEN("The user is already signed on"));
		CASE(USER_RE_LOGIN, LPGEN("The user has signed on again"));
		CASE(BAD_NAME, LPGEN("The name cannot be used"));
		CASE(REG_MODE_NS, LPGEN("The registration mode is not supported"));
		CASE(WRONG_USER_PRIV, LPGEN("User does not have appropriate privilege level"));
		CASE(NEED_EMAIL, LPGEN("Email address must be used"));
		CASE(DNS_ERROR, LPGEN("Error in DNS"));
		CASE(DNS_FATAL_ERROR, LPGEN("Fatal error in DNS"));
		CASE(DNS_NOT_FOUND, LPGEN("Server name not found"));
		CASE(CONNECTION_BROKEN, LPGEN("The connection has been broken"));
		CASE(CONNECTION_ABORTED, LPGEN("An established connection was aborted by the software in the host machine"));
		CASE(CONNECTION_REFUSED, LPGEN("The connection has been refused"));
		CASE(CONNECTION_RESET, LPGEN("The connection has been reset"));
		CASE(CONNECTION_TIMED, LPGEN("The connection has timed out"));
		CASE(CONNECTION_CLOSED, LPGEN("The connection has been closed"));
		CASE(MULTI_SERVER_LOGIN, LPGEN("Disconnected due to login in two Sametime servers concurrently (1)"));
		CASE(MULTI_SERVER_LOGIN2, LPGEN("Disconnected due to login in two Sametime servers concurrently (2)"));
		CASE(MULTI_LOGIN_COMP, LPGEN("Disconnected due to login from another computer."));
		CASE(MUTLI_LOGIN_ALREADY, LPGEN("Unable to log in because you are already logged on from another computer"));
		CASE(SERVER_BROKEN, LPGEN("Unable to log in because the server is either unreachable, or not configured properly."));
		CASE(SERVER_PATH_OLD, LPGEN("Unable to log in to home Sametime server through the requested server, since your home server needs to be upgraded."));
		CASE(APPLET_LOGOUT, LPGEN("The applet was logged out with this reason. Perform relogin and you will return to the former state."));

		/* 8.3.1.3 Client error codes */
		CASE(ERR_CLIENT_USER_GONE, LPGEN("The user is not online"));
		CASE(ERR_CLIENT_USER_DND, LPGEN("The user is in do not disturb mode"));
		CASE(ERR_CLIENT_USER_ELSEWHERE, LPGEN("Cannot login because already logged in with a different user name (Java only)"));

		/* 8.3.1.4 IM error codes */
		CASE(ERR_IM_COULDNT_REGISTER, LPGEN("Cannot register a reserved type"));
		CASE(ERR_IM_ALREADY_REGISTERED, LPGEN("The requested type is already registered"));
		CASE(ERR_IM_NOT_REGISTERED, LPGEN("The requested type is not registered"));

		/* 8.3.1.5 Resolve error codes */
		CASE(ERR_RESOLVE_NOTCOMPLETED, LPGEN("The resolve process was not completed, but a partial response is available"));
		CASE(ERR_RESOLVE_NAMENOTUNIQUE, LPGEN("The name was found, but is not unique (request was for unique only)"));
		CASE(ERR_RESOLVE_NAMENOTRESOLVABLE, LPGEN("The name is not resolvable due to its format, for example an Internet email address"));

	default:
		m = LPGEN("Unknown error code");
		break;
	}

	return g_strdup(m);
}

struct mwReturnCodeDesc *mwGetReturnCodeDesc(guint32 code) {
	struct mwReturnCodeDesc *rcDesc = g_new(struct mwReturnCodeDesc, 1);

	if (code & ERR_FAILURE)
		rcDesc->type = mwReturnCodeError;
	else
		rcDesc->type = mwReturnCodeInfo;
	
	rcDesc->codeString = g_strdup(err_to_str(code));
	rcDesc->name = mwError(code);
	rcDesc->description = mwErrorDesc(code);

	return rcDesc;
}
/// Miranda NG development end



#undef CASE
