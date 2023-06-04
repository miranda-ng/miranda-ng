/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2017-23 Miranda NG team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef JABBER_OMEMO_H
#define JABBER_OMEMO_H

#include <cstddef>
#include <map>
#include <list>


#define FP_BAD 0
#define FP_TOFU 1
#define FP_VERIFIED 2
#define FP_ABSENT -1

struct CJabberProto;
struct signal_crypto_provider;

namespace omemo
{
	const char IdentityPrefix[] = "OmemoSignalIdentity_";
	const char DevicePrefix[] = "OmemoDeviceId";

	CMStringW FormatFingerprint(const char* pszHexString);
	CMStringA hex_string(const uint8_t* pData, const size_t length);

	struct omemo_impl
	{
		omemo_impl(CJabberProto *p);
		~omemo_impl();

		void init();
		void deinit();
		int GetOwnDeviceId();
		void RefreshPrekeys(ratchet_identity_key_pair *device_key, int32_t dev_id);
		bool create_session_store();
		bool build_session(const char *jid, const char *dev_id, const char *key_id, const char *pre_key_public, const char *signed_pre_key_id,
			const char *signed_pre_key_public, const char *signed_pre_key_signature, const char *identity_key);

		__forceinline void lock() { _signal_cs.Lock(); }
		__forceinline void unlock() { _signal_cs.Unlock(); }

		signal_context *global_context = nullptr;
		signal_protocol_store_context *store_context = nullptr;
		std::list<struct outgoing_message> outgoing_messages;

		int dbGetDeviceId(MCONTACT hContact, uint32_t number);
		CMStringA dbGetSuffix(MCONTACT hContact, int device_id);
		CMStringA dbGetSuffix(const char* jid, int device_id);

		TiXmlDocument doc;

	private:
		CJabberProto *proto;
		mir_cs _signal_cs;
	};
};

#endif