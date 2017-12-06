/*

Jabber Protocol Plugin for Miranda NG

Copyright (�) 2017 Miranda NG project

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

struct CJabberProto;
struct signal_crypto_provider;

namespace omemo {
	struct omemo_device;
	struct omemo_impl {
		omemo_impl(CJabberProto *p);
		~omemo_impl();

		void init();
		void deinit();
		bool IsFirstRun();
		unsigned long GetOwnDeviceId();
		void RefreshDevice();
		omemo_device* create_device();
		bool create_session_store(MCONTACT hContact, LPCTSTR device_id);
		bool build_session(MCONTACT hContact, LPCTSTR jid, LPCTSTR dev_id, LPCTSTR key_id, LPCTSTR pre_key_public, LPCTSTR signed_pre_key_id,
			LPCTSTR signed_pre_key_public, LPCTSTR signed_pre_key_signature, LPCTSTR identity_key);


		mir_cslockfull *signal_mutex;
		void *sessions_internal;
		void *session_checked;
		void *message_queue_internal;
	private:
		CJabberProto *proto;
		mir_cs _signal_cs;
		signal_crypto_provider *provider;
	};
};

#endif