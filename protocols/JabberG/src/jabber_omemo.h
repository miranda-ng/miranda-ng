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