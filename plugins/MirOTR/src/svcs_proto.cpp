#include "stdafx.h"

//TODO: Social-Millionaire-Dialoge
INT_PTR SVC_OTRSendMessage(WPARAM wParam,LPARAM lParam){
	if (!lParam) return 0;

	CCSDATA *ccs = (CCSDATA *) lParam;

	DEBUGOUT_T("SENDING")

	if (ccs->wParam & PREF_BYPASS_OTR) // bypass for OTR-messages
		return CallService(MS_PROTO_CHAINSEND, wParam, lParam);

	char *proto = GetContactProto(ccs->hContact);
	if(proto && strcmp(proto, META_PROTO) == 0) // bypass for metacontacts
		return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
	
	if (!proto || !ccs->hContact)
		return 1; // error

	char *oldmessage = (char *)ccs->lParam;
	char *oldmessage_utf = 0;

	//MessageBox(0, "Send message - converting to UTF-8", "msg", MB_OK);
	
	if(ccs->wParam & PREF_UTF) {
		oldmessage_utf = oldmessage;
	}
	else if(ccs->wParam & PREF_UNICODE) {
		//TODO: check if this is correct or oldmessage[strlen(oldmessage)+1] is needed
		//oldmessage_utf = mir_utf8encodeW((wchar_t*)oldmessage);
		//should be the thing with strlen
		oldmessage_utf = mir_utf8encodeW((wchar_t*)&oldmessage[strlen(oldmessage)+1]);
	}
	else {
		oldmessage_utf = mir_utf8encode(oldmessage);
	}
	if (!oldmessage_utf) return 1;

	// don't filter OTR messages being sent (OTR messages should only happen *after* the otrl_message_sending call below)
	if(strncmp(oldmessage_utf, "?OTR", 4) == 0) {
		DEBUGOUT_T("OTR message without PREF_BYPASS_OTR")
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
	
	char *newmessage = 0;
	char *username = contact_get_id(ccs->hContact);
	gcry_error_t err = otrl_message_sending(otr_user_state, &ops, (void*)ccs->hContact,
		proto, proto, username, oldmessage_utf, NULL, &newmessage,
		add_appdata, (void*)ccs->hContact);
	if (tmpencode!= NULL || !(ccs->wParam & PREF_UTF))
		mir_free(oldmessage_utf);
	oldmessage_utf = NULL;
	mir_free(username);
	
	if (err && newmessage == NULL) {
		/* Be *sure* not to send out plaintext */
		ShowError(TranslateT(LANG_ENCRYPTION_ERROR));
		return 1;
	}
	else if (newmessage) {
		/* Fragment the message if necessary, and send all but the last
		 * fragment over the network.  We will send the last segment later */
						
		// oldmessage_utf is not used anymore, so use it as buffer
		err = otrl_message_fragment_and_send(&ops, (void*)ccs->hContact, context,
			newmessage, OTRL_FRAGMENT_SEND_ALL_BUT_LAST, &oldmessage_utf);
		// newmessage is not needed anymore
		otrl_message_free(newmessage);
		// move the last fragment to newmessage, oldmessage_utf will be buffer later
		newmessage = oldmessage_utf;
					
		WPARAM oldflags = ccs->wParam;
		if(ccs->wParam & PREF_UTF) {
			ccs->lParam = (LPARAM)newmessage;
		}
		else if(ccs->wParam & PREF_UNICODE) {
			// just send UTF, hope that proto supports it.
			// don't want to create the Unicode-Format
			// oldmessage_utf = (char*)mir_utf8decodeW(newmessage);
			ccs->lParam = (LPARAM)newmessage;
			ccs->wParam &= ~PREF_UNICODE;
			ccs->wParam |= PREF_UTF;
		}
		else {
			 mir_utf8decode(newmessage, NULL);
			 ccs->lParam = (LPARAM)newmessage;
		}
		INT_PTR ret = CallService(MS_PROTO_CHAINSEND, wParam, lParam);

#ifdef _DEBUG
		if(ccs->wParam & PREF_UNICODE)
		{
			TCHAR *mes = mir_a2t((const char *)ccs->lParam);
			MessageBox(0, mes, _T("OTR - sending raw message"), MB_OK);
			mir_free(mes);
		}
		else 
			MessageBoxA(0, (char *)ccs->lParam, ("OTR - sending raw message"), MB_OK);
#endif
		
		// reset to original values
		ccs->lParam = (LPARAM)oldmessage;
		ccs->wParam = oldflags;
		otrl_message_free(newmessage);
		return ret;
	}
	return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
}

/*
#define MESSAGE_PREFIX			"(OTR) "
#define MESSAGE_PREFIXW			L"(OTR) "
#define MESSAGE_PREFIX_LEN		6
*/

INT_PTR SVC_OTRRecvMessage(WPARAM wParam,LPARAM lParam){
	//PUShowMessage("OTR Recv Message", SM_NOTIFY);
	CCSDATA *ccs = (CCSDATA *) lParam;
	PROTORECVEVENT *pre = (PROTORECVEVENT *) ccs->lParam;

#ifdef _DEBUG
	if(pre->flags & PREF_UNICODE)
	{
		TCHAR *mes = mir_a2t(pre->szMessage);
		MessageBox(0, mes, _T("OTR - receiving message"), MB_OK);
		mir_free(mes);
	}
	else 
		MessageBoxA(0, (char *)pre->szMessage, ("OTR - receiving message"), MB_OK);
#endif

	if (pre->flags & PREF_BYPASS_OTR)  { // bypass for our inline messages
		return CallService(MS_PROTO_CHAINRECV, wParam, lParam);
	}

	char *proto = GetContactProto(ccs->hContact);
	if (!proto)
		return 1; //error
	else if(proto && strcmp(proto, META_PROTO) == 0) // bypass for metacontacts
		return CallService(MS_PROTO_CHAINRECV, wParam, lParam);

	if (!ccs->hContact)
		return 1; //error

	char *oldmessage = pre->szMessage;
	char *oldmessage_utf = NULL;
	// convert oldmessage to utf-8
	if(pre->flags & PREF_UTF) {
		oldmessage_utf = oldmessage;
	} else if(pre->flags & PREF_UNICODE) {  //UNSURE!!
		oldmessage_utf = mir_utf8encodeW((wchar_t*)(&oldmessage[strlen(oldmessage)+1]));
	} else {
		oldmessage_utf = mir_utf8encode(oldmessage);
	}
	if (!oldmessage_utf) return 1;


	char *newmessage = NULL;
	OtrlTLV *tlvs = NULL;
	
	char *uname = contact_get_id(ccs->hContact);
	lib_cs_lock();
	BOOL ignore_msg = otrl_message_receiving(otr_user_state, &ops, (void*)ccs->hContact,
		proto, proto, uname, oldmessage_utf,
		&newmessage, &tlvs, add_appdata, (void*)ccs->hContact);
	lib_cs_unlock();
	mir_free(uname);

	if ( !(pre->flags & PREF_UTF))
		mir_free(oldmessage_utf);
	oldmessage_utf = NULL;
	
		
	OtrlTLV *tlv = otrl_tlv_find(tlvs, OTRL_TLV_DISCONNECTED);
	if (tlv && !Miranda_Terminated()) {
		/* Notify the user that the other side disconnected. */
		TCHAR buff[256];
		mir_sntprintf(buff, SIZEOF(buff), TranslateT(LANG_SESSION_TERMINATED_BY_OTR), contact_get_nameT(ccs->hContact));
		//MessageBox(0, buff, Translate("OTR Information"), MB_OK);
		ShowMessage(ccs->hContact, buff);
	}

	lib_cs_lock();
	ConnContext *context = otrl_context_find_miranda(otr_user_state, ccs->hContact);
	lib_cs_unlock();
	/* Keep track of our current progress in the Socialist Millionaires'
	 * Protocol. */
		
		/*
			if (context && ( (context->smstate->sm_prog_state == OTRL_SMP_PROG_CHEATED) ||
			otrl_tlv_find(tlvs, OTRL_TLV_SMP1Q) ||
			otrl_tlv_find(tlvs, OTRL_TLV_SMP1) ||
			otrl_tlv_find(tlvs, OTRL_TLV_SMP2) || 
			otrl_tlv_find(tlvs, OTRL_TLV_SMP3) ||
			otrl_tlv_find(tlvs, OTRL_TLV_SMP4) )
			)
			{
			otr_abort_smp(context); // we do not support it (yet), notify partner to shorten wait time
			}
			*/
		
			
	if (context) {
		NextExpectedSMP nextMsg = context->smstate->nextExpected;
				
		if (context->smstate->sm_prog_state == OTRL_SMP_PROG_CHEATED) {
			otr_abort_smp(context);
			//otrg_dialog_update_smp(context, 0.0);
			SMPDialogUpdate(context, 0);
			context->smstate->nextExpected = OTRL_SMP_EXPECT1;
			context->smstate->sm_prog_state = OTRL_SMP_PROG_OK;
		}
		else {			
			tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP1Q);
			if (tlv) {
				if (nextMsg != OTRL_SMP_EXPECT1)
					otr_abort_smp(context);
				else {
					char *question = (char *)tlv->data;
					char *eoq = (char*)memchr(question, '\0', tlv->len);
					if (eoq) {
						SMPDialogReply(context, question);
					}
				}
			}
			tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP1);
			if (tlv) {
				if (nextMsg != OTRL_SMP_EXPECT1)
					otr_abort_smp(context);
				else {
					SMPDialogReply(context, NULL);
				}
			}
			tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP2);
			if (tlv) {
				if (nextMsg != OTRL_SMP_EXPECT2)
					otr_abort_smp(context);
				else {
					context->smstate->nextExpected = OTRL_SMP_EXPECT4;
					SMPDialogUpdate(context, 60);					
				}
			}
			tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP3);
			if (tlv) {
				if (nextMsg != OTRL_SMP_EXPECT3)
					otr_abort_smp(context);
				else {
					context->smstate->nextExpected = OTRL_SMP_EXPECT1;
					SMPDialogUpdate(context, 100);
					//otrg_dialog_update_smp(context, 1.0);
				}
			}
			tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP4);
			if (tlv) {
				if (nextMsg != OTRL_SMP_EXPECT4)
					otr_abort_smp(context);
				else {
					SMPDialogUpdate(context, 100);
					//otrg_dialog_update_smp(context, 1.0);
						context->smstate->nextExpected = OTRL_SMP_EXPECT1;
					}
			}
			tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP_ABORT);
			if (tlv) {
				SMPDialogUpdate(context, 0);
				context->smstate->nextExpected = OTRL_SMP_EXPECT1;
			}
		}
	}
	otrl_tlv_free(tlvs);

	/* If we're supposed to ignore this incoming message (because it's a
	 * protocol message), set it to NULL, so that other plugins that
	 * catch receiving-im-msg don't return 0, and cause it to be
	 * displayed anyway. */
	if (ignore_msg) {
		/* should not be required ;)
		char t[3] = {0, 0, 0};
		char t[3] = {0, 0, 0};
		if(pre->flags & PREF_UNICODE) { //TODO!! mir_tstrlen(pre->szMessage)+1 ??
			if (pre->szMessage && strlen(pre->szMessage)) memcpy(pre->szMessage, t,3 );
		} else {
			if (pre->szMessage && strlen(pre->szMessage)) memcpy(pre->szMessage, t, 1);
		}
		*/
		if (newmessage)
			otrl_message_free(newmessage);
		return 1;
	}
	else if (newmessage) {
		//bool free=false;
		DWORD flags = pre->flags;
		/* if(pre->flags & PREF_UTF) {
			oldmessage_utf = newmessage;
		} else if(pre->flags & PREF_UNICODE) { //TODO!! mir_tstrlen(pre->szMessage)+1 ??
			oldmessage_utf = (char*)mir_utf8decodeW(newmessage);
			free = true;
		} else {
			 mir_utf8decode(newmessage, NULL);
			 oldmessage_utf = newmessage;
		} */ 

		//pre->szMessage = oldmessage_utf;
		pre->flags &= ~PREF_UNICODE;
		pre->flags |= PREF_UTF; // just use UTF, so we do not have to recode the message
					
		bool is_miralloc = false;
		if (context) {
			TrustLevel level = otr_context_get_trust(context);
			if (level >= TRUST_UNVERIFIED) {
				char *premsg;
				if (db_get_b(ccs->hContact, MODULENAME, "HTMLConv", 0)) {
					premsg = striphtml(newmessage);
					otrl_message_free(newmessage);
					newmessage = premsg;
					is_miralloc = true;
				}
							
				if (options.prefix_messages) {
					size_t len = (strlen(options.prefix)+strlen(newmessage)+1)*sizeof(char);
					premsg = (char*)mir_alloc( len );
					memset(premsg, 0, len);
					strcpy(premsg, options.prefix);
					strcat(premsg, newmessage);
					(is_miralloc) ? mir_free(newmessage) : otrl_message_free(newmessage);
					newmessage = premsg;
					is_miralloc = true;
				}
			}
		}
		pre->szMessage = newmessage;
		BOOL ret = CallService(MS_PROTO_CHAINRECV, wParam, lParam);
		pre->flags = flags;
		pre->szMessage = oldmessage;
		if (is_miralloc)
			mir_free(newmessage);
		else
			otrl_message_free(newmessage);
					
		// if (free) mir_free(oldmessage_utf);
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