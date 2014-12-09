#include "stdafx.h"

OtrlUserState otr_user_state;
#define WMU_ENDDIALOG (WM_USER+244)

OtrlMessageAppOps ops = {
	otr_gui_policy,
	otr_gui_create_privkey,
	otr_gui_is_logged_in,
	otr_gui_inject_message,
	otr_gui_notify,
	otr_gui_display_otr_message,
	otr_gui_update_context_list,
	otr_gui_protocol_name,
	otr_gui_protocol_name_free,
	otr_gui_new_fingerprint,
	otr_gui_write_fingerprints,
	otr_gui_gone_secure,
	otr_gui_gone_insecure,
	otr_gui_still_secure,
	otr_gui_log_message,
	max_message_size,
	account_name,
	account_name_free
};

struct GenKeyData{
	HWND dialog;
	const char *proto;
};

static unsigned int CALLBACK generate_key_thread(void* param)
{
	Thread_Push(0);
	GenKeyData *data = (GenKeyData *)param;
	//lib_cs_lock();
	otrl_privkey_generate(otr_user_state, g_private_key_filename, data->proto, data->proto);
	//lib_cs_unlock();
	PostMessage(data->dialog, WMU_ENDDIALOG, 0, 0);
	mir_free(data);
	Thread_Pop();
	return 0;
}

INT_PTR CALLBACK GenKeyDlgFunc(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
		case WM_INITDIALOG:
			{
				if (!lParam) {
					EndDialog(hWndDlg, 0);
					return 0;
				}
				TranslateDialogDefault(hWndDlg);
				SetClassLongPtr(hWndDlg, GCLP_HICON, (LONG_PTR)LoadIcon(ICON_OTR,1) );
				TCHAR buff[256];
				TCHAR *proto = mir_a2t((char*)lParam);
				mir_sntprintf(buff, SIZEOF(buff), TranslateT(LANG_GENERATE_KEY), proto);
				mir_free(proto);
				SetDlgItemText(hWndDlg, IDC_GENERATE, buff);
				GenKeyData *data = (GenKeyData *)mir_calloc(sizeof(GenKeyData));
				data->dialog = hWndDlg;
				data->proto = (char*)lParam;
				CloseHandle((HANDLE)_beginthreadex(0, 0, generate_key_thread, (void*)data, 0, 0));
			}break;
		case WMU_ENDDIALOG:
			EndDialog(hWndDlg, 0);
			return TRUE;
		case WM_DESTROY:
			SetClassLongPtr(hWndDlg, GCLP_HICON, 0);
			ReleaseIcon(ICON_OTR,1);
	}
	return FALSE;
}

extern "C" {
	/* Return the OTR policy for the given context. */
	OtrlPolicy otr_gui_policy(void *opdata, ConnContext *context) {
		DEBUGOUT_T("OTR_GUI_POLICY")
		MCONTACT hContact = (MCONTACT)opdata;
		DWORD pol;
		if(hContact) {
			pol = db_get_dw(hContact, MODULENAME, "Policy", CONTACT_DEFAULT_POLICY); 
			if (options.bHaveSecureIM && pol != OTRL_POLICY_MANUAL_MOD && pol != OTRL_POLICY_NEVER && db_get_b(hContact, "SecureIM" , "StatusID", 0)) {
				// if SecureIM is not disabled for this contact, MirOTR will be set to manual
				db_set_dw(hContact, MODULENAME, "Policy", OTRL_POLICY_MANUAL_MOD);
				return OTRL_POLICY_MANUAL_MOD;
			}
			if(pol != CONTACT_DEFAULT_POLICY) return pol ;
		}
		if(context->protocol) {
			pol = db_get_dw(0,MODULENAME"_ProtoPol", context->protocol, CONTACT_DEFAULT_POLICY);
			if(pol != CONTACT_DEFAULT_POLICY) return pol ;
		}

		return options.default_policy ;
	}

	/* Create a private key for the given accountname/protocol if
	* desired. */
	void otr_gui_create_privkey(void *opdata, const char *account_name, const char *protocol) {
		DEBUGOUT_T("OTR_GUI_CREATE_PRIVKEY")
		//if(MessageBox(0, Translate("Would you like to generate a new private key for this protocol?"), Translate("OTR"), MB_YESNO) == IDYES)
		//if(options.err_method == ED_POP) 
		//ShowPopup(Translate("Generating new private key."), 0 /*Translate("Please wait.")*/, 5);	

		//NewKeyData *nkd = (NewKeyData *)malloc(sizeof(NewKeyData));
		//nkd->account_name = strdup(account_name);
		//nkd->protocol = strdup(protocol);

		//DWORD tid;
		//CloseHandle(CreateThread(0, 0, newKeyThread, (VOID *)nkd, 0, &tid));
		//QueueUserAPC(newKeyAPC, Global::mainThread, (DWORD)nkd);
		if (opdata) protocol = contact_get_proto((MCONTACT)opdata);
		if (!protocol) return;
		DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_GENKEYNOTIFY), 0, GenKeyDlgFunc, (LPARAM)protocol );

	}

	/* Report whether you think the given user is online.  Return 1 if
	* you think he is, 0 if you think he isn't, -1 if you're not sure.
	*
	* If you return 1, messages such as heartbeats or other
	* notifications may be sent to the user, which could result in "not
	* logged in" errors if you're wrong. */
	int otr_gui_is_logged_in(void *opdata, const char *accountname, const char *protocol, const char *recipient) {
		DEBUGOUT_T("OTR_GUI_IS_LOGGED_IN")
		MCONTACT hContact = (MCONTACT)opdata;
		if(hContact) {
			WORD status = db_get_w(hContact, contact_get_proto(hContact), "Status", ID_STATUS_OFFLINE);
			if(status == ID_STATUS_OFFLINE) return 0;
			else return 1;
		}

		return -1;
	}

	/* Send the given IM to the given recipient from the given
	* accountname/protocol. */
	void otr_gui_inject_message(void *opdata, const char *accountname, const char *protocol, const char *recipient, const char *message) {
		DEBUGOUT_T("OTR_GUI_INJECT_MESSAGE")
		//MessageBox(0, message, "OTR Inject", MB_OK);
		MCONTACT hContact = (MCONTACT)opdata;

		if(protocol && db_get_w(hContact, protocol, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE)
			CallContactService(hContact, PSS_MESSAGE, PREF_UTF|PREF_BYPASS_OTR, (LPARAM)message);
	}

	/* Display a notification message for a particular accountname /
	* protocol / username conversation. */
	void otr_gui_notify(void *opdata, OtrlNotifyLevel level, const char *accountname, const char *protocol, const char *username, const char *title, const char *primary, const char *secondary) {
		DEBUGOUT_T("OTR_GUI_NOTIFY")
		const TCHAR* uname = contact_get_nameT((MCONTACT)opdata);
		TCHAR* title_t = mir_utf8decodeT(title);
		TCHAR *notify = TranslateT(LANG_OTR_NOTIFY);
		
		size_t len = _tcslen(uname) + _tcslen(title_t) + _tcslen(notify);
		TCHAR *buff1 = new TCHAR[len];
		mir_sntprintf(buff1, len, notify, TranslateTS(title_t), uname);
		mir_free(title_t);

		len = strlen(primary) + strlen(secondary) + 5;
		char *buff2 = new char[len];
		mir_snprintf(buff2, len, "%s\n%s", Translate(primary), Translate(secondary));
		TCHAR* buff2_t = mir_utf8decodeT(buff2);
		delete buff2;

		DEBUGOUT_T("OTR_GUI_NOTIFY")

		ShowPopup(buff1, buff2_t, 0);
		mir_free(buff2_t);
		delete buff1;
	}

	/* Display an OTR control message for a particular accountname /
	* protocol / username conversation.  Return 0 if you are able to
	* successfully display it.  If you return non-0 (or if this
	* function is NULL), the control message will be displayed inline,
	* as a received message, or else by using the above notify()
	* callback. */
	int otr_gui_display_otr_message(void *opdata, const char *accountname, const char *protocol, const char *username, const char *msg) {
		DEBUGOUT_T("OTR_GUI_DISPLAY_OTR_MESSAGE")
		if(options.msg_inline)
			ShowMessageInlineUtf((MCONTACT)opdata, Translate(msg));
		if(options.msg_popup) {
			TCHAR buff[512];
			TCHAR* proto = mir_a2t(protocol);
			mir_sntprintf(buff, SIZEOF(buff), TranslateT(LANG_OTR_USERMESSAGE), contact_get_nameT((MCONTACT)opdata), proto);
			mir_free(proto);
			TCHAR *msg_t = mir_utf8decodeT(msg);
			ShowPopup(buff, TranslateTS(msg_t), 0);	
			mir_free(msg_t);
		}
		return 0;
	}

	/* When the list of ConnContexts changes (including a change in
	* state), this is called so the UI can be updated. */
	void otr_gui_update_context_list(void *opdata) {
		//MessageBox(0, "Update Context List", "OTR Callback", MB_OK);
		DEBUGOUT_T("OTR: Update Context List");
	}

	/* Return a newly-allocated string containing a human-friendly name
	* for the given protocol id */
	const char *otr_gui_protocol_name(void *opdata, const char *protocol) {
		//return strdup(protocol);
		DEBUGOUT_T("OTR_GUI_PROTOCOL_NAME")
		return protocol;
	}

	/* Deallocate a string allocated by protocol_name */
	void otr_gui_protocol_name_free(void *opdata, const char *protocol_name) {
		//free((void *)protocol_name);
	}

	/* A new fingerprint for the given user has been received. */
	void otr_gui_new_fingerprint(void *opdata, OtrlUserState us, const char *accountname, const char *protocol, const char *username, unsigned char fingerprint[20]) {
		DEBUGOUT_T("OTR_GUI_NEW_FUNGERPRINT")
		ConnContext *context = otrl_context_find(us, username, accountname, protocol, TRUE, 0, add_appdata, opdata);
		Fingerprint *fp = otrl_context_find_fingerprint(context, fingerprint, TRUE, 0);

		//CloseHandle((HANDLE)_beginthreadex(0, 0, trust_fp_thread, (void *)fp, 0, 0));

		otrl_context_set_trust(fp, NULL);
		otrl_privkey_write_fingerprints(otr_user_state, g_fingerprint_store_filename);
	}

	/* The list of known fingerprints has changed.  Write them to disk. */
	void otr_gui_write_fingerprints(void *opdata) {
		DEBUGOUT_T("OTR_GUI_WRITE_FINGERPRINTS")
		//if(MessageBox(0, Translate("Would you like to save the current fingerprint list?"), Translate(MODULE), MB_YESNO) == IDYES)
		otrl_privkey_write_fingerprints(otr_user_state, g_fingerprint_store_filename);
	}

	/* A ConnContext has entered a secure state. */
	void otr_gui_gone_secure(void *opdata, ConnContext *context) {
		DEBUGOUT_T("OTR_GUI_GONE_SECURE")
		TrustLevel trusted = otr_context_get_trust(context);
		// opdata is hContact
		SetEncryptionStatus((MCONTACT)opdata, trusted);
		TCHAR buff[1024];
		if(trusted == TRUST_PRIVATE) {
			mir_sntprintf(buff, SIZEOF(buff), TranslateT(LANG_SESSION_START_OTR), contact_get_nameT((MCONTACT)opdata));
		} else if (trusted == TRUST_UNVERIFIED) {
			if (options.autoshow_verify) SMPInitDialog(context); //VerifyContextDialog(context);
			mir_sntprintf(buff, SIZEOF(buff), TranslateT(LANG_SESSION_START_OTR_VERIFY), contact_get_nameT((MCONTACT)opdata));
		} else { // should never happen
			mir_sntprintf(buff, SIZEOF(buff), TranslateT(LANG_SESSION_NOT_STARTED_OTR), contact_get_nameT((MCONTACT)opdata));
		}
		// opdata is hContact
		ShowMessage((MCONTACT)opdata, buff);

	}

	/* A ConnContext has left a secure state. */
	void otr_gui_gone_insecure(void *opdata, ConnContext *context) {
		DEBUGOUT_T("OTR_GUI_GONE_INSECURE")
		TCHAR buff[512];
		mir_sntprintf(buff, SIZEOF(buff), TranslateT(LANG_SESSION_TERMINATED_BY_OTR), contact_get_nameT((MCONTACT)opdata));
		//MessageBox(0, buff, Translate("OTR Information"), MB_OK);
		if (!Miranda_Terminated()) {
			ShowMessage((MCONTACT)opdata, buff);
		}

		// opdata is hContact
		SetEncryptionStatus((MCONTACT)opdata, otr_context_get_trust(context));
	}

	/* We have completed an authentication, using the D-H keys we
	* already knew.  is_reply indicates whether we initiated the AKE. */
	void otr_gui_still_secure(void *opdata, ConnContext *context, int is_reply) {
		DEBUGOUT_T("OTR_GUI_STILL_SECURE")
		TrustLevel trusted = otr_context_get_trust(context);
		SetEncryptionStatus((MCONTACT)opdata, trusted);
		TCHAR buff[1024];
		if (!is_reply) {
			if(trusted == TRUST_PRIVATE) {
				mir_sntprintf(buff, SIZEOF(buff), TranslateT(LANG_SESSION_CONTINUE_OTR), contact_get_nameT((MCONTACT)opdata));
			} else if (trusted == TRUST_UNVERIFIED) {
				if (options.autoshow_verify) SMPInitDialog(context); //VerifyContextDialog(context);
				mir_sntprintf(buff, SIZEOF(buff), TranslateT(LANG_SESSION_CONTINUE_OTR_VERIFY), contact_get_nameT((MCONTACT)opdata));
			} else { // should never happen
				mir_sntprintf(buff, SIZEOF(buff), TranslateT(LANG_SESSION_NOT_STARTED_OTR), contact_get_nameT((MCONTACT)opdata));
			}
			// opdata is hContact
			ShowMessage((MCONTACT)opdata, buff);
		} else {
			if(trusted == TRUST_PRIVATE) {
				mir_sntprintf(buff, SIZEOF(buff), TranslateT(LANG_SESSION_HAS_CONTINUE_OTR), contact_get_nameT((MCONTACT)opdata));
			} else if (trusted == TRUST_UNVERIFIED) {
				mir_sntprintf(buff, SIZEOF(buff), TranslateT(LANG_SESSION_HAS_CONTINUE_OTR_VERIFY), contact_get_nameT((MCONTACT)opdata));
			} else { // should never happen
				mir_sntprintf(buff, SIZEOF(buff), TranslateT(LANG_SESSION_NOT_STARTED_OTR), contact_get_nameT((MCONTACT)opdata));
			}
			
		}
		SetEncryptionStatus((MCONTACT)opdata, trusted);
	}

	/* Log a message.  The passed message will end in "\n". */
	void otr_gui_log_message(void *opdata, const char *message) {
		//MessageBox(0, message, Translate("OTR Log Message"), MB_OK);
		//ShowMessageInline((MCONTACT)opdata, message);
#ifdef _DEBUG
		char* msg = strcpy((char*)mir_alloc(strlen(message)+15), "OTR message: ");
		strcat(msg, message);
		DEBUGOUTA(msg)
		mir_free(msg);
#endif
	}

	int max_message_size(void *opdata, ConnContext *context) {
		const char *proto;
		if (context && context->protocol)
			proto = context->protocol;
		else 
			proto = contact_get_proto((MCONTACT)opdata);
		// ugly wokaround for ICQ. ICQ protocol reports more than 7k, but in SMP this is too long.
		// possibly ICQ doesn't allow single words without spaces to become longer than ~2340?
		if (strcmp("ICQ", proto)==0 || strncmp("ICQ_", proto, 4)==0)
			return 2340;
		return CallProtoService(proto, PS_GETCAPS, PFLAG_MAXLENOFMESSAGE, (LPARAM)opdata);
	}

	const char *account_name(void *opdata, const char *account, const char *protocol) {
		return account;
	}

	void account_name_free(void *opdata, const char *account_name) {
	}

	void add_appdata(void *data, ConnContext *context) {
		if(context)	context->app_data = data;
	}
}

// Forward decl
gcry_error_t otrl_privkey_write_FILEp(OtrlUserState us, FILE *privf);
/* Generate a private DSA key for a given account, storing it into a
* file on disk, and loading it into the given OtrlUserState.  Overwrite any
* previously generated keys for that account in that OtrlUserState. */
 gcry_error_t otrl_privkey_write(OtrlUserState us, const char *filename)
 {
	 gcry_error_t err;
	 FILE *privf;
	 #ifndef WIN32
		 mode_t oldmask;
	 #endif
		 
		 #ifndef WIN32
		 oldmask = umask(077);
	 #endif
		 privf = fopen(filename, "w+b");
	 if (!privf) {
		 #ifndef WIN32
			 umask(oldmask);
		 #endif
			 err = gcry_error_from_errno(errno);
		 return err;
		 }

		 err = otrl_privkey_write_FILEp(us, privf);

		 fclose(privf);
	 #ifndef WIN32
		 umask(oldmask);
	 #endif
		 return err;
	 }

/* Just store the private keys of an OtrlUserState.
 * The FILE* must be open for reading and writing. */
	 gcry_error_t otrl_privkey_write_FILEp(OtrlUserState us, FILE *privf)
	 {
		 OtrlPrivKey *p;
		 
			 if (!privf) return gcry_error(GPG_ERR_NO_ERROR);
		 
				 
			 
			 /* Output the other keys we know */
			 fprintf(privf, "(privkeys\n");
		 for (p=us->privkey_root; p; p=p->next) {
			 otrl_account_write(privf, p->accountname, p->protocol, p->privkey);
			 }
		 if ( fprintf(privf, ")\n") < 0 )
			 return gcry_error_from_errno(errno);
		 return gcry_error(GPG_ERR_NO_ERROR);
		 
}