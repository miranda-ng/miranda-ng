#pragma once

extern OtrlUserState otr_user_state;
extern OtrlMessageAppOps ops;

/// @todo : add OTR version 3 instance tag choice (currently we use the most secure/active one)
#define MIROTR_PROTO_LATEST 3 // on change, also update HELLO below
#define MIROTR_PROTO_HELLO "?OTRv23?"
/* OTR link normally is https://otr.cypherpunks.ca/ but it's blocked by ICQ as of March 2015 (even something like "ot-r.cyp her_ pun ks. ca")
	It might be a good idea to use http://wikipedia.org/wiki/Off-the-Record_Messaging instead as it's not yet blocked by ICQ but might still help users to find a client/plugin */
#define MIROTR_PROTO_HELLO_MSG _T(" has requested an Off-the-Record private conversation. However, you do not have a plugin to support that.\nSee http://wikipedia.org/wiki/Off-the-Record_Messaging for more information.")

extern "C" {

	/* Return the OTR policy for the given context. */
	OtrlPolicy otr_gui_policy(void *opdata, ConnContext *context);

	/* Create a private key for the given accountname/protocol if
	* desired. */
	void otr_gui_create_privkey(void *opdata, const char *account_name, const char *protocol);

	int otr_gui_is_logged_in(void *opdata, const char *accountname, const char *protocol, const char *recipient);

	void otr_gui_inject_message(void *opdata, const char *accountname, const char *protocol, const char *recipient, const char *message);

	void otr_gui_update_context_list(void *opdata);

	void otr_gui_new_fingerprint(void *opdata, OtrlUserState us, const char *accountname, const char *protocol, const char *username, unsigned char fingerprint[20]);

	void otr_gui_write_fingerprints(void *opdata);

	void otr_gui_gone_secure(void *opdata, ConnContext *context);

	void otr_gui_gone_insecure(void *opdata, ConnContext *context);

	void otr_gui_still_secure(void *opdata, ConnContext *context, int is_reply);

	int max_message_size(void *opdata, ConnContext *context);

	const char *account_name(void *opdata, const char *account, const char *protocol);

	void account_name_free(void *opdata, const char *account_name);

	void add_appdata(void *data, ConnContext *context);

	const char* resent_msg_prefix(void *opdata, ConnContext *context);
	void resent_msg_prefix_free(void *opdata, const char *prefix);
	
	void handle_smp_event(void *opdata, OtrlSMPEvent smp_event, ConnContext *context, unsigned short progress_percent, char *question);
	
	void handle_msg_event(void *opdata, OtrlMessageEvent msg_event, ConnContext *context, const char *message, gcry_error_t err);
	
	void otr_create_instag(void *opdata, const char *accountname, const char *protocol);
}
