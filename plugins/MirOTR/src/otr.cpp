#include "stdafx.h"

OtrlUserState otr_user_state;
#define WMU_ENDDIALOG (WM_USER+244)

OtrlMessageAppOps ops = {
	otr_gui_policy,//policy
	otr_gui_create_privkey,//create_privkey
	otr_gui_is_logged_in,//is_logged_in
	otr_gui_inject_message,//inject_message
	otr_gui_update_context_list,//update_context_list
	otr_gui_new_fingerprint,//new_fingerprint
	otr_gui_write_fingerprints,//write_fingerprints
	otr_gui_gone_secure,//gone_secure
	otr_gui_gone_insecure,//gone_insecure
	otr_gui_still_secure,//still_secure
	max_message_size,//max_message_size
	account_name,//account_name
	account_name_free,//account_name_free
	nullptr,//received_symkey (optional)
	nullptr,//otr_error_message (optional, send to remote party so we could send them errors in a language they know)
	nullptr,//otr_error_message_free (optional)
	resent_msg_prefix,//resent_msg_prefix (optional)
	resent_msg_prefix_free,//resent_msg_prefix_free (optional)
	handle_smp_event,//handle_smp_event (optional)
	handle_msg_event,//handle_msg_event (optional)
	otr_create_instag,//create_instag (optional)
	nullptr,//convert_msg (optional)
	nullptr,//convert_free (optional)
	nullptr,//timer_control (optional) // @todo : implement security timers
};

struct GenKeyData{
	HWND dialog;
	const char *proto;
};

static unsigned int CALLBACK generate_key_thread(void* param)
{
	Thread_Push(nullptr);
	GenKeyData *data = (GenKeyData *)param;
	otrl_privkey_generate(otr_user_state, g_private_key_filename, data->proto, data->proto);
	PostMessage(data->dialog, WMU_ENDDIALOG, 0, 0);
	mir_free(data);
	Thread_Pop();
	return 0;
}

INT_PTR CALLBACK GenKeyDlgBoxProc(HWND hWndDlg, UINT msg, WPARAM, LPARAM lParam) {
	switch (msg) {
	case WM_INITDIALOG:
	{
		if (!lParam) {
			EndDialog(hWndDlg, 0);
			return 0;
		}
		TranslateDialogDefault(hWndDlg);
		SetClassLongPtr(hWndDlg, GCLP_HICON, (LONG_PTR)IcoLib_GetIconByHandle(iconList[ICON_OTR].hIcolib, 1));
		wchar_t buff[256];
		wchar_t *proto = mir_a2u((char*)lParam);
		mir_snwprintf(buff, TranslateW(LANG_GENERATE_KEY), proto);
		mir_free(proto);
		SetDlgItemText(hWndDlg, IDC_GENERATE, buff);
		GenKeyData *data = (GenKeyData *)mir_calloc(sizeof(GenKeyData));
		data->dialog = hWndDlg;
		data->proto = (char*)lParam;
		CloseHandle((HANDLE)_beginthreadex(nullptr, 0, generate_key_thread, data, 0, nullptr));
	}break;
	case WMU_ENDDIALOG:
		EndDialog(hWndDlg, 0);
		return TRUE;
	case WM_DESTROY:
		SetClassLongPtr(hWndDlg, GCLP_HICON, 0);
	}
	return FALSE;
}

extern "C" {
	/* Return the OTR policy for the given context. */
	OtrlPolicy otr_gui_policy(void *opdata, ConnContext *context) {
		DEBUGOUTA("OTR_GUI_POLICY\n");
		MCONTACT hContact = (UINT_PTR)opdata;
		uint32_t pol;
		if (hContact) {
			pol = g_plugin.getDword(hContact, "Policy", CONTACT_DEFAULT_POLICY);
			if (options.bHaveSecureIM && pol != OTRL_POLICY_MANUAL_MOD && pol != OTRL_POLICY_NEVER && db_get_b(hContact, "SecureIM", "StatusID", 0)) {
				// if SecureIM is not disabled for this contact, MirOTR will be set to manual
				g_plugin.setDword(hContact, "Policy", OTRL_POLICY_MANUAL_MOD);
				return OTRL_POLICY_MANUAL_MOD;
			}
			if (pol != CONTACT_DEFAULT_POLICY) return pol;
		}
		if (context->protocol) {
			pol = db_get_dw(0, MODULENAME"_ProtoPol", context->protocol, CONTACT_DEFAULT_POLICY);
			if (pol != CONTACT_DEFAULT_POLICY) return pol;
		}

		return options.default_policy;
	}

	/* Create a private key for the given accountname/protocol if
	* desired. */
	// otr_gui_create_privkey(void *opdata, const char *account_name, const char *protocol) {
	void otr_gui_create_privkey(void *opdata, const char *, const char *protocol) {
		DEBUGOUTA("OTR_GUI_CREATE_PRIVKEY\n");
		if (opdata) {
			protocol = Proto_GetBaseAccountName((UINT_PTR)opdata);
		}
		if (!protocol) return;
		DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_GENKEYNOTIFY), nullptr, GenKeyDlgBoxProc, (LPARAM)protocol);

	}

	/* Report whether you think the given user is online.  Return 1 if
	* you think he is, 0 if you think he isn't, -1 if you're not sure.
	*
	* If you return 1, messages such as heartbeats or other
	* notifications may be sent to the user, which could result in "not
	* logged in" errors if you're wrong. */
	//int otr_gui_is_logged_in(void *opdata, const char *accountname, const char *protocol, const char *recipient) {
	int otr_gui_is_logged_in(void *opdata, const char *, const char *, const char *) {
		DEBUGOUTA("OTR_GUI_IS_LOGGED_IN\n");
		MCONTACT hContact = (UINT_PTR)opdata;
		if (hContact) {
			uint16_t status = db_get_w(hContact, Proto_GetBaseAccountName(hContact), "Status", ID_STATUS_OFFLINE);
			if (status == ID_STATUS_OFFLINE) return 0;
			else return 1;
		}

		return -1;
	}

	/* Send the given IM to the given recipient from the given
	* accountname/protocol. */
	//void otr_gui_inject_message(void *opdata, const char *accountname, const char *protocol, const char *recipient, const char *message) {
	void otr_gui_inject_message(void *opdata, const char *, const char *protocol, const char *, const char *message) {
		DEBUGOUTA("OTR_GUI_INJECT_MESSAGE\n");
		MCONTACT hContact = (UINT_PTR)opdata;
		if (db_get_w(hContact, protocol, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE)
			ProtoChainSend(hContact, PSS_MESSAGE, PREF_BYPASS_OTR, (LPARAM)message);
	}

	/* When the list of ConnContexts changes (including a change in
	* state), this is called so the UI can be updated. */
	//void otr_gui_update_context_list(void *opdata) {
	void otr_gui_update_context_list(void *) {
		//MessageBox(0, "Update Context List", "OTR Callback", MB_OK);
		DEBUGOUTA("OTR: Update Context List\n");
	}

	/* A new fingerprint for the given user has been received. */
	void otr_gui_new_fingerprint(void *opdata, OtrlUserState us, const char *accountname, const char *protocol, const char *username, unsigned char fingerprint[20]) {
		DEBUGOUTA("OTR_GUI_NEW_FINGERPRINT\n");
		ConnContext *context = otrl_context_find(us, username, accountname, protocol, OTRL_INSTAG_BEST, TRUE, nullptr, add_appdata, opdata);
		Fingerprint *fp = otrl_context_find_fingerprint(context, fingerprint, TRUE, nullptr);

		//CloseHandle((HANDLE)_beginthreadex(0, 0, trust_fp_thread, (void *)fp, 0, 0));

		otrl_context_set_trust(fp, nullptr);
		otrl_privkey_write_fingerprints(otr_user_state, g_fingerprint_store_filename);
	}

	/* The list of known fingerprints has changed.  Write them to disk. */
	//void otr_gui_write_fingerprints(void *opdata) {
	void otr_gui_write_fingerprints(void *) {
		DEBUGOUTA("OTR_GUI_WRITE_FINGERPRINTS\n");
		//if(MessageBox(0, Translate("Would you like to save the current fingerprint list?"), Translate(MODULE), MB_YESNO) == IDYES)
		otrl_privkey_write_fingerprints(otr_user_state, g_fingerprint_store_filename);
	}

	/* A ConnContext has entered a secure state. */
	void otr_gui_gone_secure(void *opdata, ConnContext *context) {
		DEBUGOUTA("OTR_GUI_GONE_SECURE\n");
		MCONTACT hContact = (UINT_PTR)opdata;
		TrustLevel trusted = otr_context_get_trust(context);
		SetEncryptionStatus(hContact, trusted);
		wchar_t buff[512];
		if (trusted == TRUST_PRIVATE) {
			mir_snwprintf(buff, TranslateW(LANG_SESSION_START_OTR), contact_get_nameT(hContact));
		}
		else if (trusted == TRUST_UNVERIFIED) {
			if (options.autoshow_verify) SMPInitDialog(context); //VerifyContextDialog(context);
			mir_snwprintf(buff, TranslateW(LANG_SESSION_START_OTR_VERIFY), contact_get_nameT(hContact));
		}
		else { // should never happen
			mir_snwprintf(buff, TranslateW(LANG_SESSION_NOT_STARTED_OTR), contact_get_nameT(hContact));
		}
		if (context->protocol_version < MIROTR_PROTO_LATEST){
			size_t remaining = mir_wstrlen(buff);
			wchar_t *offset = buff + remaining;
			remaining = _countof(buff) - remaining;
			mir_snwprintf(offset, remaining, TranslateT("\nusing older protocol version %i"), context->protocol_version);
		}
		ShowMessage(hContact, buff);

	}

	/* A ConnContext has left a secure state. */
	void otr_gui_gone_insecure(void *opdata, ConnContext *context) {
		MCONTACT hContact = (UINT_PTR)opdata;
		DEBUGOUTA("OTR_GUI_GONE_INSECURE\n");
		wchar_t buff[512];
		mir_snwprintf(buff, TranslateW(LANG_SESSION_TERMINATED_BY_OTR), contact_get_nameT(hContact));
		//MessageBox(0, buff, Translate("OTR Information"), MB_OK);
		if (!Miranda_IsTerminated()) {
			ShowMessage(hContact, buff);
		}

		// opdata is hContact
		SetEncryptionStatus(hContact, otr_context_get_trust(context));
	}

	/* We have completed an authentication, using the D-H keys we
	* already knew.  is_reply indicates whether we initiated the AKE. */
	void otr_gui_still_secure(void *opdata, ConnContext *context, int is_reply) {
		MCONTACT hContact = (UINT_PTR)opdata;
		DEBUGOUTA("OTR_GUI_STILL_SECURE\n");
		TrustLevel trusted = otr_context_get_trust(context);
		SetEncryptionStatus(hContact, trusted);
		wchar_t buff[1024];
		if (!is_reply) {
			if (trusted == TRUST_PRIVATE) {
				mir_snwprintf(buff, TranslateW(LANG_SESSION_CONTINUE_OTR), contact_get_nameT(hContact));
			}
			else if (trusted == TRUST_UNVERIFIED) {
				if (options.autoshow_verify) SMPInitDialog(context); //VerifyContextDialog(context);
				mir_snwprintf(buff, TranslateW(LANG_SESSION_CONTINUE_OTR_VERIFY), contact_get_nameT(hContact));
			}
			else { // should never happen
				mir_snwprintf(buff, TranslateW(LANG_SESSION_NOT_STARTED_OTR), contact_get_nameT(hContact));
			}
			// opdata is hContact
			ShowMessage(hContact, buff);
		}
		else {
			if (trusted == TRUST_PRIVATE) {
				mir_snwprintf(buff, TranslateW(LANG_SESSION_HAS_CONTINUE_OTR), contact_get_nameT(hContact));
			}
			else if (trusted == TRUST_UNVERIFIED) {
				mir_snwprintf(buff, TranslateW(LANG_SESSION_HAS_CONTINUE_OTR_VERIFY), contact_get_nameT(hContact));
			}
			else { // should never happen
				mir_snwprintf(buff, TranslateW(LANG_SESSION_NOT_STARTED_OTR), contact_get_nameT(hContact));
			}

		}
		SetEncryptionStatus(hContact, trusted);
	}

	int max_message_size(void *opdata, ConnContext *context) {
		const char *proto;
		if (context && context->protocol)
			proto = context->protocol;
		else
			proto = Proto_GetBaseAccountName((UINT_PTR)opdata);
		// ugly wokaround for ICQ. ICQ protocol reports more than 7k, but in SMP this is too long.
		// possibly ICQ doesn't allow single words without spaces to become longer than ~2340?
		if (mir_strcmp("ICQ", proto) == 0 || strncmp("ICQ_", proto, 4) == 0)
			return 2340;
		return CallProtoService(proto, PS_GETCAPS, PFLAG_MAXLENOFMESSAGE, (LPARAM)opdata);
	}

	//const char *account_name(void *opdata, const char *account, const char *protocol) {
	const char *account_name(void *, const char *account, const char *) {
		return account;
	}

	//void account_name_free(void *opdata, const char *account_name) {
	void account_name_free(void *, const char *) {
	}

	void add_appdata(void *data, ConnContext *context) {
		if (context)	context->app_data = data;
	}

	//const char* resent_msg_prefix(void *opdata, ConnContext *context) {
	const char* resent_msg_prefix(void *, ConnContext *){
		return "[resent]";
	}

	//void resent_msg_prefix_free(void *opdata, const char *prefix) {
	void resent_msg_prefix_free(void *, const char *){
		return;
	}

	void handle_smp_event(void *opdata, OtrlSMPEvent smp_event, ConnContext *context, unsigned short progress_percent, char *question) {
		DEBUGOUTA("HANDLE_SMP_EVENT\n");
		if (!context) return;
		switch (smp_event){
		case OTRL_SMPEVENT_NONE:
			break;
		case OTRL_SMPEVENT_ASK_FOR_SECRET:
		case OTRL_SMPEVENT_ASK_FOR_ANSWER:
			SMPDialogReply(context, question);
			break;
		case OTRL_SMPEVENT_CHEATED:
			otrl_message_abort_smp(otr_user_state, &ops, opdata, context);
			// fall through
		case OTRL_SMPEVENT_IN_PROGRESS:
		case OTRL_SMPEVENT_SUCCESS:
		case OTRL_SMPEVENT_FAILURE:
		case OTRL_SMPEVENT_ABORT:
			SMPDialogUpdate(context, progress_percent);
			break;
		case OTRL_SMPEVENT_ERROR:
			otrl_message_abort_smp(otr_user_state, &ops, opdata, context);
			break;
		}
	}

	//void handle_msg_event(void *opdata, OtrlMessageEvent msg_event, ConnContext *context, const char *message, gcry_error_t err) {
	void handle_msg_event(void *opdata, OtrlMessageEvent msg_event, ConnContext *, const char *message, gcry_error_t err) {
		DEBUGOUTA("HANDLE_MSG_EVENT\n");
		MCONTACT hContact = (UINT_PTR)opdata;
		const wchar_t* contact = contact_get_nameT(hContact);

		typedef void(*msgfunc_t)(const MCONTACT, const wchar_t*);
		msgfunc_t msgfunc = ShowMessage;
		//		wchar_t* title = NULL;
		wchar_t msg[512];
		msg[0] = '\0';
		switch (msg_event){
		case OTRL_MSGEVENT_NONE:
		case OTRL_MSGEVENT_LOG_HEARTBEAT_RCVD:
		case OTRL_MSGEVENT_LOG_HEARTBEAT_SENT:
		case OTRL_MSGEVENT_RCVDMSG_UNRECOGNIZED:
			break;
		case OTRL_MSGEVENT_ENCRYPTION_REQUIRED:
			msgfunc = ShowMessageInline;
			mir_wstrncpy(msg, TranslateT("Attempting to start a private conversation..."), _countof(msg));
			break;
		case OTRL_MSGEVENT_ENCRYPTION_ERROR:
			msgfunc = ShowMessageInline;
			mir_wstrncpy(msg, TranslateT("An error occurred when encrypting your message.\nThe message was not sent"), _countof(msg));
			break;
		case OTRL_MSGEVENT_CONNECTION_ENDED:
			msgfunc = ShowMessageInline;
			mir_snwprintf(msg, _countof(msg), TranslateT("'%s' has already closed his/her private connection to you; you should do the same"), contact);
			break;
		case OTRL_MSGEVENT_SETUP_ERROR:
			//			title = TranslateT("OTR Error");
			if (!err) err = GPG_ERR_INV_VALUE;
			switch (gcry_err_code(err)){
			case GPG_ERR_INV_VALUE:
				mir_snwprintf(msg, _countof(msg), TranslateT("Error setting up private conversation: %s"), TranslateT("Malformed message received"));
				break;
			default:{
				wchar_t* tmp = mir_utf8decodeW(gcry_strerror(err));
				mir_snwprintf(msg, _countof(msg), TranslateT("Error setting up private conversation: %s"), tmp);
				mir_free(tmp); }
			}
			break;
		case OTRL_MSGEVENT_MSG_REFLECTED:
			//			title = TranslateT("OTR Error");
			mir_wstrncpy(msg, TranslateT("We are receiving our own OTR messages.\nYou are either trying to talk to yourself, or someone is reflecting your messages back at you"), _countof(msg));
			break;
		case OTRL_MSGEVENT_MSG_RESENT:
			//			title = TranslateT("Message resent");
			mir_snwprintf(msg, _countof(msg), TranslateT("The last message to '%s' was resent"), contact);
			break;
		case OTRL_MSGEVENT_RCVDMSG_NOT_IN_PRIVATE:
			//			title = TranslateT("Unreadable message");
			mir_snwprintf(msg, _countof(msg), TranslateT("The encrypted message received from '%s' is unreadable, as you are not currently communicating privately"), contact);
			break;
		case OTRL_MSGEVENT_RCVDMSG_UNREADABLE:
			//			title = TranslateT("OTR Error");
			mir_snwprintf(msg, _countof(msg), TranslateT("We received an unreadable encrypted message from '%s'"), contact);
			break;
		case OTRL_MSGEVENT_RCVDMSG_MALFORMED:
			//			title = TranslateT("OTR Error");
			mir_snwprintf(msg, _countof(msg), TranslateT("We received a malformed data message from '%s'"), contact);
			break;
		case OTRL_MSGEVENT_RCVDMSG_GENERAL_ERR:{
			//			title = TranslateT("OTR Error");
			wchar_t* tmp = mir_utf8decodeW(message);
			mir_wstrncpy(msg, tmp, _countof(msg));
			mir_free(tmp);
			break; }
		case OTRL_MSGEVENT_RCVDMSG_UNENCRYPTED:{
			//			title = TranslateT("Received unencrypted message");
			msgfunc = ShowMessageInline;
			wchar_t* tmp = mir_utf8decodeW(message);
			mir_snwprintf(msg, _countof(msg), TranslateT("The following message received from '%s' was NOT encrypted:\n\n%s"), contact, tmp);
			mir_free(tmp);
			ProtoChainSend(hContact, PSS_MESSAGE, PREF_BYPASS_OTR, (LPARAM)message);
			break; }
		case OTRL_MSGEVENT_RCVDMSG_FOR_OTHER_INSTANCE:
			//			title = TranslateT("Received message for a different session");
			mir_snwprintf(msg, _countof(msg), TranslateT("'%s' has sent a message intended for a different session. If you are logged in multiple times, another session may have received the message."), contact);
			break;
		default:
			//			title = TranslateT("OTR Error");
			mir_wstrncpy(msg, TranslateT("unknown OTR message received, please report that to Miranda NG developers"), _countof(msg));
		}
		if (msg[0])
			msgfunc(hContact, msg);
	}

	//void otr_create_instag(void *opdata, const char *accountname, const char *protocol) {
	void otr_create_instag(void *, const char *accountname, const char *protocol){
		DEBUGOUTA("OTR_CREATE_INSTAG\n");
		FILE* instagf = _wfopen(g_instag_filename, L"w+b");
		if (!instagf)
			return;
		otrl_instag_generate_FILEp(otr_user_state, instagf, accountname, protocol);
		fclose(instagf);
	}
}
