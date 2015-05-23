// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2010 Joe Kucera
// Copyright © 2012-2014 Miranda NG Team
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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// -----------------------------------------------------------------------------
//  DESCRIPTION:
//
//  Contains helper functions to handle oscar user capabilities. Scanning and
//  adding capabilities are assumed to be more timecritical than looking up
//  capabilites. During the login sequence there could possibly be many hundred
//  scans but only a few lookups. So when you add or change something in this
//  code you must have this in mind, dont do anything that will slow down the
//  adding process too much.
// -----------------------------------------------------------------------------

#include "stdafx.h"

struct icq_capability
{
	DWORD capID;          // A bitmask, we use it in order to save database space
	capstr capCLSID;      // A binary representation of a oscar capability
};

static const icq_capability CapabilityRecord[] =
{
	{ CAPF_SRV_RELAY,  { CAP_SRV_RELAY  }},
	{ CAPF_UTF,        { CAP_UTF        }},
	{ CAPF_RTF,        { CAP_RTF        }},
	{ CAPF_CONTACTS,   { CAP_CONTACTS   }},
	{ CAPF_TYPING,     { CAP_TYPING     }},
	{ CAPF_ICQDIRECT,  { CAP_ICQDIRECT  }},
	{ CAPF_XTRAZ,      { CAP_XTRAZ      }},
	{ CAPF_OSCAR_FILE, { CAP_OSCAR_FILE }}
};

// Mask of all handled capabilities' flags
#define CapabilityFlagsMask  (CAPF_SRV_RELAY | CAPF_UTF | CAPF_RTF | CAPF_CONTACTS | CAPF_TYPING | CAPF_ICQDIRECT | CAPF_XTRAZ | CAPF_OSCAR_FILE)


#ifdef _DEBUG
struct icq_capability_name
{
	DWORD capID;
	const char* capName;
};

static const icq_capability_name CapabilityNames[] =
{
	{ CAPF_SRV_RELAY,      "ServerRelay"},
	{ CAPF_UTF,            "UTF8 Messages"},
	{ CAPF_RTF,            "RTF Messages"},
	{ CAPF_CONTACTS,       "Contact Transfer"},
	{ CAPF_TYPING,         "Typing Notifications"},
	{ CAPF_ICQDIRECT,      "Direct Connections"},
	{ CAPF_XTRAZ,          "Xtraz"},
	{ CAPF_OSCAR_FILE,     "File Transfers"},
	{ CAPF_STATUS_MESSAGES,"Individual Status Messages"},
	{ CAPF_STATUS_MOOD,    "Mood"},
	{ CAPF_XSTATUS,        "Custom Status"}
};

void NetLog_CapabilityChange(CIcqProto *ppro, const char *szChange, DWORD fdwCapabilities)
{
	char szBuffer[MAX_PATH] = {0};

	if (!fdwCapabilities) return;

	for (int nIndex = 0; nIndex < SIZEOF(CapabilityNames); nIndex++) {
		// Check if the current capability is present
		if ((fdwCapabilities & CapabilityNames[nIndex].capID) == CapabilityNames[nIndex].capID) {
			if (mir_strlen(szBuffer))
				mir_strcat(szBuffer, ", ");
			mir_strcat(szBuffer, CapabilityNames[nIndex].capName);
		}
	}
	// Log the change
	ppro->debugLogA("Capabilities: %s %s", szChange, szBuffer);
}
#endif

// Deletes all oscar capabilities for a given contact
void CIcqProto::ClearAllContactCapabilities(MCONTACT hContact)
{
	setDword(hContact, DBSETTING_CAPABILITIES, 0);
}

// Deletes one or many oscar capabilities for a given contact
void CIcqProto::ClearContactCapabilities(MCONTACT hContact, DWORD fdwCapabilities)
{
	// Get current capability flags
	DWORD fdwContactCaps = getDword(hContact, DBSETTING_CAPABILITIES, 0);
	if (fdwContactCaps != (fdwContactCaps & ~fdwCapabilities)) {
#ifdef _DEBUG
		NetLog_CapabilityChange(this, "Removed", fdwCapabilities & fdwContactCaps);
#endif
		// Clear unwanted capabilities
		fdwContactCaps &= ~fdwCapabilities;

		// And write it back to disk
		setDword(hContact, DBSETTING_CAPABILITIES, fdwContactCaps);
	}
}

// Sets one or many oscar capabilities for a given contact
void CIcqProto::SetContactCapabilities(MCONTACT hContact, DWORD fdwCapabilities)
{
	// Get current capability flags
	DWORD fdwContactCaps = getDword(hContact, DBSETTING_CAPABILITIES, 0);
	if (fdwContactCaps != (fdwContactCaps | fdwCapabilities)) {
#ifdef _DEBUG
		NetLog_CapabilityChange(this, "Added", fdwCapabilities & ~fdwContactCaps);
#endif
		// Update them
		fdwContactCaps |= fdwCapabilities;

		// And write it back to disk
		setDword(hContact, DBSETTING_CAPABILITIES, fdwContactCaps);
	}
}

// Returns true if the given contact supports the requested capabilites
BOOL CIcqProto::CheckContactCapabilities(MCONTACT hContact, DWORD fdwCapabilities)
{
	// Get current capability flags
	DWORD fdwContactCaps =  getDword(hContact, DBSETTING_CAPABILITIES, 0);

	// Check if all requested capabilities are supported
	if ((fdwContactCaps & fdwCapabilities) == fdwCapabilities)
		return TRUE;

	return FALSE;
}


// Scan capability against the capability buffer
capstr* MatchCapability(BYTE *buf, size_t bufsize, const capstr *cap, size_t capsize)
{
	while (bufsize >= BINARY_CAP_SIZE) { // search the buffer for a capability
		if (!memcmp(buf, cap, capsize))
			return (capstr*)buf; // give found capability for version info

		buf += BINARY_CAP_SIZE;
		bufsize -= BINARY_CAP_SIZE;
	}
	return 0;
}


// Scan short capability against the capability buffer
capstr* MatchShortCapability(BYTE *buf, size_t bufsize, const shortcapstr *cap)
{
	capstr fullCap;
	memcpy(fullCap, capShortCaps, BINARY_CAP_SIZE);
	fullCap[2] = (*cap)[0];
	fullCap[3] = (*cap)[1];

	return MatchCapability(buf, bufsize, &fullCap, BINARY_CAP_SIZE);
}

// Scans a binary buffer for OSCAR capabilities.
DWORD GetCapabilitiesFromBuffer(BYTE *pBuffer, int nLength)
{
	DWORD fdwCaps = 0;

	// Calculate the number of records
	int nRecordSize = SIZEOF(CapabilityRecord);

	// Loop over all capabilities in the buffer and
	// compare them to our own record of capabilities
	for (int nIndex = 0; nIndex < nRecordSize; nIndex++) {
		// Match, add capability flag
		if (MatchCapability(pBuffer, nLength, &CapabilityRecord[nIndex].capCLSID, BINARY_CAP_SIZE))
			fdwCaps |= CapabilityRecord[nIndex].capID;
	}

	return fdwCaps;
}

// Scans a binary buffer for oscar capabilities and adds them to the contact.
// You probably want to call ClearAllContactCapabilities() first.
void CIcqProto::AddCapabilitiesFromBuffer(MCONTACT hContact, BYTE *pBuffer, int nLength)
{
	// Get current capability flags
	DWORD fdwContactCaps = getDword(hContact, DBSETTING_CAPABILITIES, 0);
	// Get capability flags from buffer
	DWORD fdwCapabilities = GetCapabilitiesFromBuffer(pBuffer, nLength);

	if (fdwContactCaps != (fdwContactCaps | fdwCapabilities)) {
#ifdef _DEBUG
		NetLog_CapabilityChange(this, "Added", fdwCapabilities & ~fdwContactCaps);
#endif
		// Add capability flags from buffer
		fdwContactCaps |= fdwCapabilities;

		// And write them back to database
		setDword(hContact, DBSETTING_CAPABILITIES, fdwContactCaps);
	}
}

// Scans a binary buffer for oscar capabilities and adds them to the contact.
// You probably want to call ClearAllContactCapabilities() first.
void CIcqProto::SetCapabilitiesFromBuffer(MCONTACT hContact, BYTE *pBuffer, int nLength, BOOL bReset)
{
	// Get current capability flags
	DWORD fdwContactCaps = bReset ? 0 : getDword(hContact, DBSETTING_CAPABILITIES, 0);
	// Get capability flags from buffer
	DWORD fdwCapabilities = GetCapabilitiesFromBuffer(pBuffer, nLength);

#ifdef _DEBUG
	if (bReset)
		NetLog_CapabilityChange(this, "Set", fdwCapabilities);
	else {
		NetLog_CapabilityChange(this, "Removed", fdwContactCaps & ~fdwCapabilities & CapabilityFlagsMask);
		NetLog_CapabilityChange(this, "Added", fdwCapabilities & ~fdwContactCaps);
	}
#endif

	if (fdwCapabilities != (fdwContactCaps & ~CapabilityFlagsMask)) {
		// Get current unmanaged capability flags
		fdwContactCaps &= ~CapabilityFlagsMask;

		// Add capability flags from buffer
		fdwContactCaps |= fdwCapabilities;

		// And write them back to database
		setDword(hContact, DBSETTING_CAPABILITIES, fdwContactCaps);
	}
}
