#pragma once

extern OtrlUserState otr_user_state;
extern OtrlMessageAppOps ops;

extern "C" {

	/* Return the OTR policy for the given context. */
	OtrlPolicy otr_gui_policy(void *opdata, ConnContext *context);

	/* Create a private key for the given accountname/protocol if
	* desired. */
	void otr_gui_create_privkey(void *opdata, const char *account_name, const char *protocol);

	int otr_gui_is_logged_in(void *opdata, const char *accountname, const char *protocol, const char *recipient);

	void otr_gui_inject_message(void *opdata, const char *accountname, const char *protocol, const char *recipient, const char *message);

	void otr_gui_notify(void *opdata, OtrlNotifyLevel level, const char *accountname, const char *protocol, const char *username, const char *title, const char *primary, const char *secondary);

	int otr_gui_display_otr_message(void *opdata, const char *accountname, const char *protocol, const char *username, const char *msg);

	void otr_gui_update_context_list(void *opdata);

	const char *otr_gui_protocol_name(void *opdata, const char *protocol);

	void otr_gui_protocol_name_free(void *opdata, const char *protocol_name);

	void otr_gui_new_fingerprint(void *opdata, OtrlUserState us, const char *accountname, const char *protocol, const char *username, unsigned char fingerprint[20]);

	void otr_gui_write_fingerprints(void *opdata);

	void otr_gui_gone_secure(void *opdata, ConnContext *context);

	void otr_gui_gone_insecure(void *opdata, ConnContext *context);

	void otr_gui_still_secure(void *opdata, ConnContext *context, int is_reply);

	void otr_gui_log_message(void *opdata, const char *message);

	int max_message_size(void *opdata, ConnContext *context);

	const char *account_name(void *opdata, const char *account, const char *protocol);

	void account_name_free(void *opdata, const char *account_name);

	void add_appdata(void *data, ConnContext *context);

}

gcry_error_t otrl_privkey_write(OtrlUserState us, const char *filename);