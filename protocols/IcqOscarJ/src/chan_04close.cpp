// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2009 Joe Kucera
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// -----------------------------------------------------------------------------

#include "icqoscar.h"

void CIcqProto::handleCloseChannel(BYTE *buf, size_t datalen, serverthread_info *info)
{
	oscar_tlv_chain *chain = NULL;

	// Parse server reply, prepare reconnection
	if (!info->bLoggedIn && datalen && !info->isNewServerReady)
		handleLoginReply(buf, datalen, info);

	if (info->isMigrating)
		handleMigration(info);

	if ((!info->bLoggedIn || info->isMigrating) && info->isNewServerReady) {
		if (!connectNewServer(info)) { // Connecting failed
			if (info->isMigrating)
				icq_LogUsingErrorCode(LOG_ERROR, GetLastError(), LPGEN("Unable to connect to migrated ICQ communication server"));
			else
				icq_LogUsingErrorCode(LOG_ERROR, GetLastError(), LPGEN("Unable to connect to ICQ communication server"));

			SetCurrentStatus(ID_STATUS_OFFLINE);

			info->isMigrating = false;
		}
		info->isNewServerReady = false;
		return;
	}

	if (chain = readIntoTLVChain(&buf, datalen, 0)) {
		// TLV 9 errors (runtime errors?)
		WORD wError = chain->getWord(0x09, 1);
		if (wError) {
			SetCurrentStatus(ID_STATUS_OFFLINE);

			handleRuntimeError(wError);
		}

		disposeChain(&chain);
	}
	// Server closed connection on error, or sign off
	NetLib_CloseConnection(&hServerConn, TRUE);
}

void CIcqProto::handleLoginReply(BYTE *buf, size_t datalen, serverthread_info *info)
{
	oscar_tlv_chain *chain = NULL;

	icq_sendCloseConnection(); // imitate icq5 behaviour

	if (!(chain = readIntoTLVChain(&buf, datalen, 0))) {
		debugLogA("Error: Missing chain on close channel");
		NetLib_CloseConnection(&hServerConn, TRUE);
		return; // Invalid data
	}

	// TLV 8 errors (signon errors?)
	WORD wError = chain->getWord(0x08, 1);
	if (wError) {
		handleSignonError(wError);

		// we return only if the server did not gave us cookie (possible to connect with soft error)
		if (!chain->getLength(0x06, 1)) {
			disposeChain(&chain);
			icq_serverDisconnect();
			return; // Failure
		}
	}

	// We are in the login phase and no errors were reported.
	// Extract communication server info.
	info->newServer = chain->getString(0x05, 1);
	info->newServerSSL = chain->getNumber(0x8E, 1);
	info->cookieData = (BYTE*)chain->getString(0x06, 1);
	info->cookieDataLen = chain->getLength(0x06, 1);

	// We dont need this anymore
	disposeChain(&chain);

	if (!info->newServer || !info->cookieData) {
		icq_LogMessage(LOG_FATAL, LPGEN("You could not sign on because the server returned invalid data. Try again."));

		SAFE_FREE(&info->newServer);
		SAFE_FREE((void**)&info->cookieData);
		info->cookieDataLen = 0;

		SetCurrentStatus(ID_STATUS_OFFLINE);
		NetLib_CloseConnection(&hServerConn, TRUE);
		return; // Failure
	}

	debugLogA("Authenticated.");
	info->isNewServerReady = true;
}

int CIcqProto::connectNewServer(serverthread_info *info)
{
	int res = 0;

	/* Get the ip and port */
	WORD wServerPort = info->wServerPort; // prepare default port
	parseServerAddress(info->newServer, &wServerPort);

	NETLIBOPENCONNECTION nloc = { 0 };
	nloc.flags = 0;
	nloc.szHost = info->newServer;
	nloc.wPort = wServerPort;

	if (!m_bGatewayMode) {
		NetLib_SafeCloseHandle(&info->hPacketRecver);
		NetLib_CloseConnection(&hServerConn, TRUE);

		debugLogA("Closed connection to login server");

		hServerConn = NetLib_OpenConnection(m_hNetlibUser, NULL, &nloc);
		if (hServerConn && info->newServerSSL) /* Start SSL session if requested */
			if (!CallService(MS_NETLIB_STARTSSL, (WPARAM)hServerConn, 0))
				NetLib_CloseConnection(&hServerConn, FALSE);

		if (hServerConn) {
			/* Time to recreate the packet receiver */
			info->hPacketRecver = (HANDLE)CallService(MS_NETLIB_CREATEPACKETRECVER, (WPARAM)hServerConn, 0x2400);
			if (!info->hPacketRecver)
				debugLogA("Error: Failed to create packet receiver.");
			else { // we need to reset receiving structs
				info->bReinitRecver = true;
				res = 1;
			}
		}
	}
	else { // TODO: We should really do some checks here
		debugLogA("Walking in Gateway to %s", info->newServer);
		// TODO: This REQUIRES more work (most probably some kind of mid-netlib module)
		icq_httpGatewayWalkTo(hServerConn, &nloc);
		res = 1;
	}
	if (!res) SAFE_FREE((void**)&info->cookieData);

	// Free allocated memory
	// NOTE: "cookie" will get freed when we have connected to the communication server.
	SAFE_FREE(&info->newServer);

	return res;
}

void CIcqProto::handleMigration(serverthread_info *info)
{
	// Check the data that was saved when the migration was announced
	debugLogA("Migrating to %s", info->newServer);
	if (!info->newServer || !info->cookieData) {
		icq_LogMessage(LOG_FATAL, LPGEN("You have been disconnected from the ICQ network because the current server shut down."));

		SAFE_FREE(&info->newServer);
		SAFE_FREE((void**)&info->cookieData);
		info->isNewServerReady = info->isMigrating = false;
	}
}

void CIcqProto::handleSignonError(WORD wError)
{
	switch (wError) {

	case 0x01: // Unregistered uin
	case 0x04: // Incorrect uin or password
	case 0x05: // Mismatch uin or password
	case 0x06: // Internal Client error (bad input to authorizer)
	case 0x07: // Invalid account
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_WRONGPASSWORD);
		ZeroMemory(m_szPassword, sizeof(m_szPassword));
		icq_LogFatalParam(LPGEN("Connection failed.\nYour ICQ number or password was rejected (%d)."), wError);
		break;

	case 0x02: // Service temporarily unavailable
	case 0x0D: // Bad database status
	case 0x10: // Service temporarily offline
	case 0x12: // Database send error
	case 0x14: // Reservation map error
	case 0x15: // Reservation link error
	case 0x1A: // Reservation timeout
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_NOSERVER);
		icq_LogFatalParam(LPGEN("Connection failed.\nThe server is temporarily unavailable (%d)."), wError);
		break;

	case 0x16: // The users num connected from this IP has reached the maximum
	case 0x17: // The users num connected from this IP has reached the maximum (reserved)
		icq_LogFatalParam(LPGEN("Connection failed.\nServer has too many connections from your IP (%d)."), wError);
		break;

	case 0x18: // Reservation rate limit exceeded
	case 0x1D: // Rate limit exceeded
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_NOSERVER);
		icq_LogFatalParam(LPGEN("Connection failed.\nYou have connected too quickly,\nplease wait and retry 10 to 20 minutes later (%d)."), wError);
		break;

	case 0x1B: // You are using an older version of ICQ. Upgrade required
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_WRONGPROTOCOL);
		icq_LogMessage(LOG_FATAL, LPGEN("Connection failed.\nThe server did not accept this client version."));
		break;

	case 0x1C: // You are using an older version of ICQ. Upgrade recommended
		icq_LogMessage(LOG_WARNING, LPGEN("The server sent warning, this version is getting old.\nTry to look for a new one."));
		break;

	case 0x1E: // Can't register on the ICQ network
		icq_LogMessage(LOG_FATAL, LPGEN("Connection failed.\nYou were rejected by the server for an unknown reason.\nThis can happen if the UIN is already connected."));
		break;

	case 0x0C: // Invalid database fields, MD5 login not supported
		icq_LogMessage(LOG_FATAL, LPGEN("Connection failed.\nSecure (MD5) login is not supported on this account."));
		break;

	case 0:    // No error
		break;

	case 0x08: // Deleted account
	case 0x09: // Expired account
	case 0x0A: // No access to database
	case 0x0B: // No access to resolver
	case 0x0E: // Bad resolver status
	case 0x0F: // Internal error
	case 0x11: // Suspended account
	case 0x13: // Database link error
	case 0x19: // User too heavily warned
	case 0x1F: // Token server timeout
	case 0x20: // Invalid SecureID number
	case 0x21: // MC error
	case 0x22: // Age restriction
	case 0x23: // RequireRevalidation
	case 0x24: // Link rule rejected
	case 0x25: // Missing information or bad SNAC format
	case 0x26: // Link broken
	case 0x27: // Invalid client IP
	case 0x28: // Partner rejected
	case 0x29: // SecureID missing
	case 0x2A: // Blocked account | Bump user

	default:
		icq_LogFatalParam(LPGEN("Connection failed.\nUnknown error during sign on: 0x%02x"), wError);
		break;
	}
}

void CIcqProto::handleRuntimeError(WORD wError)
{
	switch (wError) {
	case 0x01:
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_OTHERLOCATION);
		icq_LogMessage(LOG_FATAL, LPGEN("You have been disconnected from the ICQ network because you logged on from another location using the same ICQ number."));
		break;

	default:
		icq_LogFatalParam(LPGEN("Unknown runtime error: 0x%02x"), wError);
		break;
	}
}
