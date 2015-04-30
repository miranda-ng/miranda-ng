#include "stdafx.h"

INT_PTR SVC_OTRSendMessage(WPARAM wParam,LPARAM lParam){
	if (!lParam) return 0;

	CCSDATA *ccs = (CCSDATA *) lParam;

	DEBUGOUT_T("SENDING\n");
		DEBUGOUTA("'");
		DEBUGOUTA((const char*)ccs->lParam);
		DEBUGOUTA("'\n");

	if (ccs->wParam & PREF_BYPASS_OTR) // bypass for OTR-messages
		return CallService(MS_PROTO_CHAINSEND, wParam, lParam);

	char *proto = GetContactProto(ccs->hContact);
	if(proto && strcmp(proto, META_PROTO) == 0) // bypass for metacontacts
		return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
	
	if (!proto || !ccs->hContact)
		return 1; // error

	const char *oldmessage = (const char *)ccs->lParam;
	char *oldmessage_utf = NULL;

	//MessageBox(0, "Send message - converting to UTF-8", "msg", MB_OK);
	
	if(ccs->wParam & PREF_UTF) {
		oldmessage_utf = (char*)oldmessage;
	}
	else if(ccs->wParam & PREF_UNICODE) {
		oldmessage_utf = mir_utf8encodeW((wchar_t*)&oldmessage[strlen(oldmessage)+1]);
	}
	else {
		oldmessage_utf = mir_utf8encode(oldmessage);
	}
	if (!oldmessage_utf) return 1;

	// don't filter OTR messages being sent (OTR messages should only happen *after* the otrl_message_sending call below)
	if(strncmp(oldmessage_utf, "?OTR", 4) == 0) {
		DEBUGOUT_T("OTR message without PREF_BYPASS_OTR\n");
		if (!(ccs->wParam & PREF_UTF))
			mir_free(oldmessage_utf);
		return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
	}

	char *tmpencode = NULL;
	ConnContext *context = otrl_context_find_miranda(otr_user_state, ccs->hContact);
	if (db_get_b(ccs->hContact, MODULENAME, "HTMLConv", 0) && otr_context_get_trust(context) >= TRUST_UNVERIFIED) {
		tmpencode = encode_html_entities_utf8(oldmessage_utf);
		if (tmpencode != NULL) {
			if (!(ccs->wParam & PREF_UTF)) mir_free(oldmessage_utf);
			oldmessage_utf = tmpencode;
		}
	}
	
	char *newmessage = NULL;
	char *username = contact_get_id(ccs->hContact);
	gcry_error_t err = otrl_message_sending(otr_user_state, &ops, (void*)ccs->hContact, proto, proto, username, OTRL_INSTAG_BEST, oldmessage_utf, NULL, &newmessage, OTRL_FRAGMENT_SEND_ALL_BUT_LAST, NULL, add_appdata, (void*)ccs->hContact);
	if (tmpencode!= NULL || !(ccs->wParam & PREF_UTF))
		mir_free(oldmessage_utf);
	mir_free(username);
	
	if (err) { /* Be *sure* not to send out plaintext */
		DEBUGOUTA("otrl_message_sending err");
		ShowError(TranslateT(LANG_ENCRYPTION_ERROR));
		otrl_message_free(newmessage);
		return 1;
	} else if (newmessage) {
		if(!newmessage[0]){
			otrl_message_free(newmessage);
			return 1; // skip empty messages (OTR might prevent us sending unencrypted messages by replacing them with empty ones)
		}
		WPARAM oldflags = ccs->wParam;
		if(ccs->wParam & (PREF_UTF|PREF_UNICODE)) {
			ccs->lParam = (LPARAM)newmessage;
			ccs->wParam &= ~PREF_UNICODE;
			ccs->wParam |= PREF_UTF;
		} else {
			mir_utf8decode(newmessage, NULL);
			ccs->lParam = (LPARAM)newmessage;
		}
		INT_PTR ret = CallService(MS_PROTO_CHAINSEND, wParam, lParam);

		DEBUGOUTA("OTR - sending raw message: '");
		DEBUGOUTA((const char*)ccs->lParam);
		DEBUGOUTA("'\n");
		otrl_message_free(newmessage);
		
		// reset to original values
		ccs->lParam = (LPARAM)oldmessage;
		ccs->wParam = oldflags;
		return ret;
	}
	return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
}


INT_PTR SVC_OTRRecvMessage(WPARAM wParam,LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA *) lParam;
	PROTORECVEVENT *pre = (PROTORECVEVENT *) ccs->lParam;

	DEBUGOUTA("OTR - receiving message: '");
	DEBUGOUTA(pre->szMessage);
	DEBUGOUTA("'\n");

	if (pre->flags & PREF_BYPASS_OTR)  { // bypass for our inline messages
		return CallService(MS_PROTO_CHAINRECV, wParam, lParam);
	}

	char *proto = GetContactProto(ccs->hContact);
	if (!proto || !ccs->hContact)
		return 1; //error
	else if(proto && strcmp(proto, META_PROTO) == 0) // bypass for metacontacts
		return CallService(MS_PROTO_CHAINRECV, wParam, lParam);

	char *oldmessage = pre->szMessage;
	char *oldmessage_utf = NULL;
	// convert oldmessage to utf-8
	if(pre->flags & PREF_UTF) {
		oldmessage_utf = oldmessage;
	} else if(pre->flags & PREF_UNICODE) {
		oldmessage_utf = mir_utf8encodeW((wchar_t*)(&oldmessage[strlen(oldmessage)+1]));
	} else {
		oldmessage_utf = mir_utf8encode(oldmessage);
	}
	if (!oldmessage_utf) return 1;


	ConnContext* context=NULL;
	char *uname = contact_get_id(ccs->hContact);
	char *newmessage = NULL;
	OtrlTLV *tlvs = NULL;
	
	lib_cs_lock();
	int ignore_msg = otrl_message_receiving(otr_user_state, &ops, (void*)ccs->hContact,
		proto, proto, uname, oldmessage_utf,
		&newmessage, &tlvs, &context, add_appdata, (void*)ccs->hContact);
	mir_free(uname);

	if ( !(pre->flags & PREF_UTF))
		mir_free(oldmessage_utf);
	
	OtrlTLV *tlv = otrl_tlv_find(tlvs, OTRL_TLV_DISCONNECTED);
	if (tlv && !Miranda_Terminated()) {
		/* Notify the user that the other side disconnected. */
		TCHAR buff[256];
		mir_sntprintf(buff, SIZEOF(buff), TranslateT(LANG_SESSION_TERMINATED_BY_OTR), contact_get_nameT(ccs->hContact));
		SetEncryptionStatus(ccs->hContact, otr_context_get_trust(context)); // required since libotr 4!?
		ShowMessage(ccs->hContact, buff);
	}
	otrl_tlv_free(tlvs);

	if (ignore_msg) {
		if (newmessage)
			otrl_message_free(newmessage);
		return 1; // discard internal protocol messages
	} else if (newmessage) {
		DWORD oldflags = pre->flags;
		pre->flags &= ~PREF_UNICODE;
		pre->flags |= PREF_UTF; // just use UTF, so we do not have to recode the message
		
		typedef void (*msg_free_t)(void*);
		msg_free_t msg_free = (msg_free_t)otrl_message_free;
		if (db_get_b(ccs->hContact, MODULENAME, "HTMLConv", 0)) {
			char* tmp = striphtml(newmessage);
			msg_free(newmessage);
			newmessage = tmp;
			msg_free = mir_free;
		}
		if (options.prefix_messages) {
			size_t len = (strlen(options.prefix)+strlen(newmessage)+1)*sizeof(char);
			char* tmp = (char*)mir_alloc( len );
			strcpy(tmp, options.prefix);
			strcat(tmp, newmessage);
			msg_free(newmessage);
			newmessage = tmp;
			msg_free = mir_free;
		}
		pre->szMessage = newmessage;
		BOOL ret = CallService(MS_PROTO_CHAINRECV, wParam, lParam);
/// @todo (White-Tiger#1#03/23/15): why are we doing this?
		pre->flags = oldflags;
		pre->szMessage = oldmessage;
		msg_free(newmessage);
		return ret;
	}
	return CallService(MS_PROTO_CHAINRECV, wParam, lParam);

}

/* Abort the SMP protocol.  Used when malformed or unexpected messages
 * are received. */
void otr_abort_smp(ConnContext *context)
{
	otrl_message_abort_smp(otr_user_state, &ops, context->app_data, context);
}

/* Start the Socialist Millionaires' Protocol over the current connection,
 * using the given initial secret, and optionally a question to pass to
 * the buddy. */
void otr_start_smp(ConnContext *context, const char *question,
	const unsigned char *secret, size_t secretlen)
{
	otrl_message_initiate_smp_q(otr_user_state, &ops, context->app_data,
		context, question, secret, secretlen);
}

/* Continue the Socialist Millionaires' Protocol over the current connection,
 * using the given initial secret (ie finish step 2). */
void otr_continue_smp(ConnContext *context,
	const unsigned char *secret, size_t secretlen)
{
	otrl_message_respond_smp(otr_user_state, &ops, context->app_data,
		context, secret, secretlen);
}
