#include "commonheaders.h"


// return SignID
int getSecureSig(LPCSTR szMsg, LPSTR *szPlainMsg=NULL) {
	if(szPlainMsg) *szPlainMsg=(LPSTR)szMsg;
	for(int i=0;signs[i].len;i++) {
		if (memcmp(szMsg,signs[i].sig,signs[i].len)==0) {
/*			for(int i=strlen(szMsg)-1;i;i--) {
				if( szMsg[i] == '\x0D' || szMsg[i] == '\x0A' )
					((LPSTR)szMsg)[i] = '\0';
				else
					break;
			}*/
			if(szPlainMsg) *szPlainMsg = (LPSTR)(szMsg+signs[i].len);
			if(signs[i].key==SiG_GAME && !bDGP)
				return SiG_NONE;
			return signs[i].key;
		}
	}
	return SiG_NONE;
}


int returnNoError(HANDLE hContact) {
	HANDLE hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	unsigned int tID;
	CloseHandle( (HANDLE) _beginthreadex(NULL, 0, sttFakeAck, new TFakeAckParams(hEvent,hContact,777,0), 0, &tID) );
	SetEvent( hEvent );
	return 777;
}


int returnError(HANDLE hContact, LPCSTR err) {
	HANDLE hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	unsigned int tID;
	CloseHandle( (HANDLE) _beginthreadex(NULL, 0, sttFakeAck, new TFakeAckParams(hEvent,hContact,666,err), 0, &tID) );
	SetEvent( hEvent );
	return 666;
}


LPSTR szUnrtfMsg = NULL;


// RecvMsg handler
INT_PTR __cdecl onRecvMsg(WPARAM wParam, LPARAM lParam) {

	CCSDATA *pccsd = (CCSDATA *)lParam;
	PROTORECVEVENT *ppre = (PROTORECVEVENT *)pccsd->lParam;
	pUinKey ptr = getUinKey(pccsd->hContact);
	LPSTR szEncMsg = ppre->szMessage, szPlainMsg = NULL;

#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("onRecvMsg: %s", szEncMsg);
#endif

	// cut rtf tags
	if( pRtfconvString && memcmp(szEncMsg,"{\\rtf1",6)==0 ) {
		SAFE_FREE(szUnrtfMsg);
		int len = (int)strlen(szEncMsg)+1;
		LPWSTR szTemp = (LPWSTR)mir_alloc(len*sizeof(WCHAR));
	   	if(ppre->flags & PREF_UNICODE)
	   		rtfconvW((LPWSTR)(szEncMsg+len),szTemp);
	   	else
	   		rtfconvA(szEncMsg,szTemp);
	   	len = (int)wcslen(szTemp)-1;
	   	while(len) {
	   		if( szTemp[len] == 0x0D || szTemp[len] == 0x0A )
	   			szTemp[len] = 0;
	   		else
	   			break;
			len--;
	   	}
	   	len = (int)wcslen(&szTemp[1])+1;
	   	szUnrtfMsg = (LPSTR)mir_alloc(len*(sizeof(WCHAR)+1));
		WideCharToMultiByte(CP_ACP, 0, &szTemp[1], -1, szUnrtfMsg, len*(sizeof(WCHAR)+1), NULL, NULL);
		memcpy(szUnrtfMsg+len,&szTemp[1],len*sizeof(WCHAR));
	   	ppre->szMessage = szEncMsg = szUnrtfMsg;
	   	ppre->flags |= PREF_UNICODE;
	   	mir_free(szTemp);
	}

	int ssig = getSecureSig(ppre->szMessage,&szEncMsg);
	BOOL bSecured = isContactSecured(pccsd->hContact)&SECURED;
	BOOL bPGP = isContactPGP(pccsd->hContact);
	BOOL bGPG = isContactGPG(pccsd->hContact);

//	HANDLE hMetaContact = getMetaContact(pccsd->hContact);
//	if( hMetaContact ) {
//		ptr = getUinKey(hMetaContact);
//	}

	// pass any unchanged message
	if( !ptr ||
		ssig==SiG_GAME ||
		!isSecureProtocol(pccsd->hContact) ||
		(isProtoMetaContacts(pccsd->hContact) && (pccsd->wParam & PREF_SIMNOMETA)) ||
		isChatRoom(pccsd->hContact) ||
		(ssig==SiG_NONE && !ptr->msgSplitted && !bSecured && !bPGP && !bGPG)
	  ) {
#if defined(_DEBUG) || defined(NETLIB_LOG)
		Sent_NetLog("onRecvMsg: pass unhandled");
#endif
		return CallService(MS_PROTO_CHAINRECV, wParam, lParam);
	}

	// drop message: fake, unsigned or from invisible contacts
	if( isContactInvisible(pccsd->hContact) || ssig==SiG_FAKE ) {
#if defined(_DEBUG) || defined(NETLIB_LOG)
		Sent_NetLog("onRecvMsg: drop unhandled (contact invisible or hidden)");
#endif
		return 1;
	}

	// receive non-secure message in secure mode
	if( ssig==SiG_NONE && !ptr->msgSplitted ) {
#if defined(_DEBUG) || defined(NETLIB_LOG)
		Sent_NetLog("onRecvMsg: non-secure message");
#endif
		if(ppre->flags & PREF_UNICODE) {
			szPlainMsg = m_awstrcat(Translate(sim402),szEncMsg);
		}
		else {
			szPlainMsg = m_aastrcat(Translate(sim402),szEncMsg);
		}
		ppre->szMessage = szPlainMsg;
		pccsd->wParam |= PREF_SIMNOMETA;
		int ret = CallService(MS_PROTO_CHAINRECV, wParam, lParam);
		mir_free(szPlainMsg);
		return ret;
	}

	// received non-pgp secure message from disabled contact
	if( ssig!=SiG_PGPM && !bPGP && !bGPG && ptr->status==STATUS_DISABLED ) {
#if defined(_DEBUG) || defined(NETLIB_LOG)
	    Sent_NetLog("onRecvMsg: message from disabled");
#endif
	    if( ptr->mode==MODE_NATIVE ) {
		// tell to the other side that we have the plugin disabled with him
	    	pccsd->wParam |= PREF_METANODB;
		pccsd->lParam = (LPARAM) SIG_DISA;
		pccsd->szProtoService = PSS_MESSAGE;
		CallService(MS_PROTO_CHAINSEND, wParam, lParam);

		showPopUp(sim003,pccsd->hContact,g_hPOP[POP_PU_DIS],0);
	    }
	    else {
		createRSAcntx(ptr);
		exp->rsa_disabled(ptr->cntx);
		deleteRSAcntx(ptr);
	    }
	    SAFE_FREE(ptr->msgSplitted);
	    return 1;
	}

	// combine message splitted by protocol (no tags)
	if( ssig==SiG_NONE && ptr->msgSplitted ) {
#if defined(_DEBUG) || defined(NETLIB_LOG)
		Sent_NetLog("onRecvMsg: combine untagged splitted message");
#endif
		LPSTR tmp = (LPSTR) mir_alloc(strlen(ptr->msgSplitted)+strlen(szEncMsg)+1);
		strcpy(tmp,ptr->msgSplitted);
		strcat(tmp,szEncMsg);
		mir_free(ptr->msgSplitted);
		ptr->msgSplitted = szEncMsg = ppre->szMessage = tmp;
		ssig = getSecureSig(tmp,&szEncMsg);
	}
	else {
		SAFE_FREE(ptr->msgSplitted);
	}

	// combine message splitted by secureim (with tags)
	if( ssig==SiG_SECP || ssig==SiG_PART ) {
		LPSTR msg = combineMessage(ptr,szEncMsg);
		if( !msg ) return 1;
		szEncMsg = ppre->szMessage = msg;
		ssig = getSecureSig(msg,&szEncMsg);
	}

	// decrypt PGP/GPG message
	if( ssig==SiG_PGPM &&
	   ((bPGPloaded && (bPGPkeyrings || bPGPprivkey))||
	   (bGPGloaded && bGPGkeyrings))
	  ) {
#if defined(_DEBUG) || defined(NETLIB_LOG)
		Sent_NetLog("onRecvMsg: PGP/GPG message");
#endif
		szEncMsg = ppre->szMessage;
		if( !ptr->cntx ) {
			ptr->cntx = cpp_create_context(((bGPGloaded && bGPGkeyrings)?CPP_MODE_GPG:CPP_MODE_PGP) | ((DBGetContactSettingByte(pccsd->hContact,szModuleName,"gpgANSI",0))?CPP_MODE_GPG_ANSI:0));
			ptr->keyLoaded = 0;
		}

		if(!strstr(szEncMsg,"-----END PGP MESSAGE-----"))
			return 1; // no end tag, don't display it ...

		LPSTR szNewMsg = NULL;
		LPSTR szOldMsg = NULL;

		if(!ptr->keyLoaded && bPGPloaded) ptr->keyLoaded = LoadKeyPGP(ptr);
		if(!ptr->keyLoaded && bGPGloaded) ptr->keyLoaded = LoadKeyGPG(ptr);

		if(ptr->keyLoaded==1) szOldMsg = pgp_decode(ptr->cntx, szEncMsg);
		else
		if(ptr->keyLoaded==2) szOldMsg = gpg_decode(ptr->cntx, szEncMsg);

		if(!szOldMsg) { // error while decrypting message, send error
			SAFE_FREE(ptr->msgSplitted);
			ppre->flags &= ~(PREF_UNICODE|PREF_UTF);
			pccsd->wParam &= ~(PREF_UNICODE|PREF_UTF);
			ppre->szMessage = Translate(sim401);
			return CallService(MS_PROTO_CHAINRECV, wParam, lParam);
		}

		// receive encrypted message in non-encrypted mode
		if(!isContactPGP(pccsd->hContact) && !isContactGPG(pccsd->hContact)) {
			szNewMsg = m_ustrcat(TranslateU(sim403),szOldMsg);
			szOldMsg = szNewMsg;
		}
		szNewMsg = utf8_to_miranda(szOldMsg,ppre->flags); pccsd->wParam = ppre->flags;
		ppre->szMessage = szNewMsg;

		// show decoded message
		showPopUpRM(ptr->hContact);
		SAFE_FREE(ptr->msgSplitted);
		pccsd->wParam |= PREF_SIMNOMETA;
		int ret = CallService(MS_PROTO_CHAINRECV, wParam, lParam);
		mir_free(szNewMsg);
		return ret;
	}

#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("onRecvMsg: switch(ssig)=%d",ssig);
#endif
	switch(ssig) {

	case SiG_PGPM:
		return CallService(MS_PROTO_CHAINRECV, wParam, lParam);

	case SiG_SECU: { // new secured msg, pass to rsa_recv
#if defined(_DEBUG) || defined(NETLIB_LOG)
		Sent_NetLog("onRecvMsg: RSA/AES message");
#endif
		if( ptr->mode==MODE_NATIVE ) {
		    ptr->mode = MODE_RSAAES;
		    deleteRSAcntx(ptr);
		    DBWriteContactSettingByte(ptr->hContact, szModuleName, "mode", ptr->mode);
		}
		createRSAcntx(ptr);
		loadRSAkey(ptr);
		if( exp->rsa_get_state(ptr->cntx)==0 )
		    showPopUpKR(ptr->hContact);

		LPSTR szOldMsg = exp->rsa_recv(ptr->cntx,szEncMsg);
		if( !szOldMsg )	return 1; // don't display it ...

		LPSTR szNewMsg = utf8_to_miranda(szOldMsg,ppre->flags); pccsd->wParam = ppre->flags;
		ppre->szMessage = szNewMsg;

		// show decoded message
		showPopUpRM(ptr->hContact);
		SAFE_FREE(ptr->msgSplitted);
		pccsd->wParam |= PREF_SIMNOMETA;
		int ret = CallService(MS_PROTO_CHAINRECV, wParam, lParam);
		mir_free(szNewMsg);
		return ret;
	} break;

	case SiG_ENON: { // online message
#if defined(_DEBUG) || defined(NETLIB_LOG)
		Sent_NetLog("onRecvMsg: Native SiG_ENON message");
#endif
		if( cpp_keyx(ptr->cntx) ) {
			// decrypting message
			szPlainMsg = decodeMsg(ptr,lParam,szEncMsg);
			if(!ptr->decoded) {
				mir_free(szPlainMsg);
				SAFE_FREE(ptr->msgSplitted);
				ptr->msgSplitted=mir_strdup(szEncMsg);
				return 1; // don't display it ...
			}
//			showPopUpRM(ptr->hContact);
		}
		else {
			// reinit key exchange user has send an encrypted message and i have no key
			cpp_reset_context(ptr->cntx);

			LPSTR reSend = (LPSTR) mir_alloc(strlen(szEncMsg)+LEN_RSND);
			strcpy(reSend,SIG_RSND); // copy resend sig
			strcat(reSend,szEncMsg); // add mess

	    		pccsd->wParam |= PREF_METANODB;
			pccsd->lParam = (LPARAM) reSend; // reSend Message to reemit
			pccsd->szProtoService = PSS_MESSAGE;
			CallService(MS_PROTO_CHAINSEND, wParam, lParam); // send back cipher message
			mir_free(reSend);

			LPSTR keyToSend = InitKeyA(ptr,0); // calculate public and private key

			pccsd->lParam = (LPARAM) keyToSend;
			CallService(MS_PROTO_CHAINSEND, wParam, lParam); // send new key
			mir_free(keyToSend);

			showPopUp(sim005,NULL,g_hPOP[POP_PU_DIS],0);
			showPopUpKS(ptr->hContact);

			return 1;
		}
	} break;

	case SiG_ENOF: { // offline message
#if defined(_DEBUG) || defined(NETLIB_LOG)
		Sent_NetLog("onRecvMsg: Native SiG_ENOF message");
#endif
		// if offline key is set and we have not an offline message unset key
		if (ptr->offlineKey && cpp_keyx(ptr->cntx)) {
			cpp_reset_context(ptr->cntx);
			ptr->offlineKey = false;
		}
		// decrypting message with last offline key
		DBVARIANT dbv;
		dbv.type = DBVT_BLOB;

		if( DBGetContactSetting(ptr->hContact,szModuleName,"offlineKey",&dbv) == 0 ) {
			// if valid key is succefully retrieved
			ptr->offlineKey = true;
			InitKeyX(ptr,dbv.pbVal);
			DBFreeVariant(&dbv);

			// decrypting message
			szPlainMsg = decodeMsg(ptr,lParam,szEncMsg);

//			showPopUpRM(ptr->hContact);
			ShowStatusIconNotify(ptr->hContact);
		}
		else {
			// exit and show messsage
			return CallService(MS_PROTO_CHAINRECV, wParam, lParam);
		}
	} break;

	case SiG_RSND: { // resend message
#if defined(_DEBUG) || defined(NETLIB_LOG)
		Sent_NetLog("onRecvMsg: Native SiG_RSND message");
#endif
		if (cpp_keyx(ptr->cntx)) {
			// decrypt sended back message and save message for future sending with a new secret key
			szPlainMsg = decodeMsg(ptr,(LPARAM)pccsd,szEncMsg);
			addMsg2Queue(ptr,pccsd->wParam,szPlainMsg);
			mir_free(szPlainMsg);

			showPopUpRM(ptr->hContact);
			showPopUp(sim004,NULL,g_hPOP[POP_PU_DIS],0);
		}
		return 1; // don't display it ...
	} break;

	case SiG_DISA: { // disabled message
#if defined(_DEBUG) || defined(NETLIB_LOG)
		Sent_NetLog("onRecvMsg: Native SiG_DISA message");
#endif
//		ptr->status=ptr->tstatus=STATUS_DISABLED;
//		DBWriteContactSettingByte(ptr->hContact, szModuleName, "StatusID", ptr->status);
	}
	case SiG_DEIN: { // deinit message
		// other user has disabled SecureIM with you
		cpp_delete_context(ptr->cntx); ptr->cntx=0;

		showPopUpDC(ptr->hContact);
		ShowStatusIconNotify(ptr->hContact);

		waitForExchange(ptr,3); // дослать нешифрованно
		return 1;
	} break;

	case SiG_KEYR:   // key3 message
	case SiG_KEYA:   // keyA message
	case SiG_KEYB: { // keyB message
		if( ptr->mode==MODE_RSAAES ) {
		    ptr->mode = MODE_NATIVE;
		    cpp_delete_context(ptr->cntx);
		    ptr->cntx = 0;
		    ptr->keyLoaded = 0;
		    DBWriteContactSettingByte(ptr->hContact, szModuleName, "mode", ptr->mode);
		}
		switch(ssig) {
		case SiG_KEYR: { // key3 message
#if defined(_DEBUG) || defined(NETLIB_LOG)
			Sent_NetLog("onRecvMsg: SiG_KEYR received");
#endif
			// receive KeyB from user;
			showPopUpKR(ptr->hContact);

			if( ptr->cntx && cpp_keyb(ptr->cntx) ) {
				// reinit key exchange if an old key from user is found
				cpp_reset_context(ptr->cntx);
			}

			if( InitKeyB(ptr,szEncMsg)!=CPP_ERROR_NONE ) {
#if defined(_DEBUG) || defined(NETLIB_LOG)
				Sent_NetLog("onRecvMsg: SiG_KEYR InitKeyB error");
#endif
				// tell to the other side that we have the plugin disabled with him
/*
	    			pccsd->wParam |= PREF_METANODB;
				pccsd->lParam = (LPARAM) SIG_DISA;
				pccsd->szProtoService = PSS_MESSAGE;
				CallService(MS_PROTO_CHAINSEND, wParam, lParam);
*/
				showPopUp(sim013,ptr->hContact,g_hPOP[POP_PU_DIS],0);
				ShowStatusIconNotify(ptr->hContact);

				waitForExchange(ptr,3); // дослать нешифрованно
				return 1;
			}

			// other side support RSA mode ?
			if( ptr->features & CPP_FEATURES_RSA ) {
				// switch to RSAAES mode
				ptr->mode = MODE_RSAAES;
				DBWriteContactSettingByte(ptr->hContact, szModuleName, "mode", ptr->mode);

				resetRSAcntx(ptr);
				loadRSAkey(ptr);
				exp->rsa_connect(ptr->cntx);

				showPopUpKS(pccsd->hContact);
				ShowStatusIconNotify(pccsd->hContact);
				return 1;
			}

			// other side support new key format ?
			if( ptr->features & CPP_FEATURES_NEWPG ) {
				cpp_reset_context(ptr->cntx);

				LPSTR keyToSend = InitKeyA(ptr,CPP_FEATURES_NEWPG|KEY_A_SIG); // calculate NEW public and private key
#if defined(_DEBUG) || defined(NETLIB_LOG)
				Sent_NetLog("onRecvMsg: Sending KEYA %s", keyToSend);
#endif
	    			pccsd->wParam |= PREF_METANODB;
				pccsd->lParam = (LPARAM)keyToSend;
				pccsd->szProtoService = PSS_MESSAGE;
				CallService(MS_PROTO_CHAINSEND, wParam, lParam);
				mir_free(keyToSend);

				showPopUpKS(ptr->hContact);
				waitForExchange(ptr); // запустим ожидание
				return 1;
			}

			// auto send my public key to keyB user if not done before
			if( !cpp_keya(ptr->cntx) ) {
				LPSTR keyToSend = InitKeyA(ptr,0); // calculate public and private key
#if defined(_DEBUG) || defined(NETLIB_LOG)
				Sent_NetLog("onRecvMsg: Sending KEYA %s", keyToSend);
#endif
	    			pccsd->wParam |= PREF_METANODB;
				pccsd->lParam = (LPARAM)keyToSend;
				pccsd->szProtoService = PSS_MESSAGE;
				CallService(MS_PROTO_CHAINSEND, wParam, lParam);
				mir_free(keyToSend);

				showPopUpKS(ptr->hContact);
			}
		} break;

		case SiG_KEYA: { // keyA message
#if defined(_DEBUG) || defined(NETLIB_LOG)
			Sent_NetLog("onRecvMsg: SiG_KEYA received");
#endif
			// receive KeyA from user;
			showPopUpKR(ptr->hContact);

			cpp_reset_context(ptr->cntx);
			if(InitKeyB(ptr,szEncMsg)!=CPP_ERROR_NONE) {
#if defined(_DEBUG) || defined(NETLIB_LOG)
				Sent_NetLog("onRecvMsg: SiG_KEYA InitKeyB error");
#endif
				// tell to the other side that we have the plugin disabled with him
/*
	    			pccsd->wParam |= PREF_METANODB;
				pccsd->lParam = (LPARAM) SIG_DISA;
				pccsd->szProtoService = PSS_MESSAGE;
				CallService(MS_PROTO_CHAINSEND, wParam, lParam);
*/
				showPopUp(sim013,ptr->hContact,g_hPOP[POP_PU_DIS],0);
				ShowStatusIconNotify(ptr->hContact);

				waitForExchange(ptr,3); // дослать нешифрованно
				return 1;
			}

			LPSTR keyToSend = InitKeyA(ptr,CPP_FEATURES_NEWPG|KEY_B_SIG); // calculate NEW public and private key
#if defined(_DEBUG) || defined(NETLIB_LOG)
			Sent_NetLog("onRecvMsg: Sending KEYB %s", keyToSend);
#endif
	    		pccsd->wParam |= PREF_METANODB;
			pccsd->lParam = (LPARAM)keyToSend;
			pccsd->szProtoService = PSS_MESSAGE;
			CallService(MS_PROTO_CHAINSEND, wParam, lParam);
			mir_free(keyToSend);
		} break;

		case SiG_KEYB: { // keyB message
#if defined(_DEBUG) || defined(NETLIB_LOG)
			Sent_NetLog("onRecvMsg: SiG_KEYB received");
#endif
			// receive KeyB from user;
			showPopUpKR(ptr->hContact);

			// clear all and send DISA if received KeyB, and not exist KeyA or error on InitKeyB
			if(!cpp_keya(ptr->cntx) || InitKeyB(ptr,szEncMsg)!=CPP_ERROR_NONE) {
#if defined(_DEBUG) || defined(NETLIB_LOG)
				Sent_NetLog("onRecvMsg: SiG_KEYB InitKeyB error");
#endif
				// tell to the other side that we have the plugin disabled with him
/*
	    			pccsd->wParam |= PREF_METANODB;
				pccsd->lParam = (LPARAM) SIG_DISA;
				pccsd->szProtoService = PSS_MESSAGE;
				CallService(MS_PROTO_CHAINSEND, wParam, lParam);
*/
				showPopUp(sim013,ptr->hContact,g_hPOP[POP_PU_DIS],0);
				ShowStatusIconNotify(ptr->hContact);

				cpp_reset_context(ptr->cntx);
				waitForExchange(ptr,3); // дослать нешифрованно
				return 1;
			}
		} break;

		} // switch

		/* common part (CalcKeyX & SendQueue) */
		//  calculate KeyX
		if( cpp_keya(ptr->cntx) && cpp_keyb(ptr->cntx) && !cpp_keyx(ptr->cntx) )
			CalculateKeyX(ptr,ptr->hContact);

		ShowStatusIconNotify(ptr->hContact);
#if defined(_DEBUG) || defined(NETLIB_LOG)
		Sent_NetLog("onRecvMsg: Session established");
#endif

		waitForExchange(ptr,2); // дошлем через шифрованное соединение
		return 1;
		/* common part (CalcKeyX & SendQueue) */
	} break;

	} //switch

	// receive message
	if( cpp_keyx(ptr->cntx) && (ssig==SiG_ENON||ssig==SiG_ENOF) ) {
#if defined(_DEBUG) || defined(NETLIB_LOG)
		Sent_NetLog("onRecvMsg: message received");
#endif
		showPopUpRM(ptr->hContact);
	}
#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("onRecvMsg: exit");
#endif
	pccsd->wParam |= PREF_SIMNOMETA;
	int ret = CallService(MS_PROTO_CHAINRECV, wParam, lParam);
	SAFE_FREE(szPlainMsg);
	return ret;
}


// SendMsgW handler
INT_PTR __cdecl onSendMsgW(WPARAM wParam, LPARAM lParam) {
	if(!lParam) return 0;

	CCSDATA *ccs = (CCSDATA *) lParam;
	if( !(ccs->wParam & PREF_UTF) )
		ccs->wParam |= PREF_UNICODE;
	
	return onSendMsg(wParam, lParam);
}


// SendMsg handler
INT_PTR __cdecl onSendMsg(WPARAM wParam, LPARAM lParam) {

	CCSDATA *pccsd = (CCSDATA *)lParam;
	pUinKey ptr = getUinKey(pccsd->hContact);
	int ssig = getSecureSig((LPCSTR)pccsd->lParam);
	int stat = getContactStatus(pccsd->hContact);

//	HANDLE hMetaContact = getMetaContact(pccsd->hContact);
//	if( hMetaContact ) {
//		ptr = getUinKey(hMetaContact);
//	}
#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("onSend: %s",(LPSTR)pccsd->lParam);
#endif
	// pass unhandled messages
	if ( !ptr ||
		ssig==SiG_GAME || ssig==SiG_PGPM || ssig==SiG_SECU || ssig==SiG_SECP ||
		isChatRoom(pccsd->hContact) ||
/*		(ssig!=SiG_NONE && hMetaContact && (pccsd->wParam & PREF_METANODB)) || */
		stat==-1 ||
		(ssig==SiG_NONE && ptr->sendQueue) ||
		(ssig==SiG_NONE && ptr->status==STATUS_DISABLED) // Disabled - pass unhandled
	   ) {
		return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
#if defined(_DEBUG) || defined(NETLIB_LOG)
		Sent_NetLog("onSendMsg: pass unhandled");
#endif
	}

	//
	// PGP/GPG mode
	// 
	if( ptr->mode==MODE_PGP || ptr->mode==MODE_GPG ) {
#if defined(_DEBUG) || defined(NETLIB_LOG)
	    Sent_NetLog("onSendMsg: PGP|GPG mode");
#endif
	    // если можно зашифровать - шифруем
            if( isContactPGP(ptr->hContact) || isContactGPG(ptr->hContact) ) {
/*
		if(stat==ID_STATUS_OFFLINE) {
			if (msgbox1(0,sim110,szModuleName,MB_YESNO|MB_ICONQUESTION)==IDNO) {
				return returnNoError(pccsd->hContact);
			}
			// exit and send unencrypted message
			return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
		}
*/
		if( !ptr->cntx ) {
			ptr->cntx = cpp_create_context((isContactGPG(ptr->hContact)?CPP_MODE_GPG:CPP_MODE_PGP) | ((DBGetContactSettingByte(ptr->hContact,szModuleName,"gpgANSI",0))?CPP_MODE_GPG_ANSI:0));
			ptr->keyLoaded = 0;
		}
		if( !ptr->keyLoaded && bPGPloaded ) ptr->keyLoaded = LoadKeyPGP(ptr);
		if( !ptr->keyLoaded && bGPGloaded ) ptr->keyLoaded = LoadKeyGPG(ptr);
		if( !ptr->keyLoaded ) return returnError(pccsd->hContact,Translate(sim108));

		LPSTR szNewMsg = NULL;
		LPSTR szUtfMsg = miranda_to_utf8((LPCSTR)pccsd->lParam,pccsd->wParam);
		if( ptr->keyLoaded == 1 ) { // PGP
    			szNewMsg = pgp_encode(ptr->cntx,szUtfMsg);
    		}
    		else
		if( ptr->keyLoaded == 2 ) { // GPG
    			szNewMsg = gpg_encode(ptr->cntx,szUtfMsg);
		}
		mir_free(szUtfMsg);
		if( !szNewMsg ) {
			return returnError(pccsd->hContact,Translate(sim109));
		}

		// отправляем зашифрованное сообщение
		splitMessageSend(ptr,szNewMsg);

		showPopUpSM(ptr->hContact);

		return returnNoError(pccsd->hContact);
	    }
	    else {
	    	// отправляем незашифрованное
		return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
	    }
	}

	// get contact SecureIM status
	int stid = ptr->status;

	//
	// RSA/AES mode
	//
	if( ptr->mode==MODE_RSAAES ) {
#if defined(_DEBUG) || defined(NETLIB_LOG)
		Sent_NetLog("onSendMsg: RSA/AES mode");
#endif
		// contact is offline
		if ( stat==ID_STATUS_OFFLINE ) {
        		if( ptr->cntx ) {
        			if( exp->rsa_get_state(ptr->cntx)!=0 ) {
        				resetRSAcntx(ptr);
				}
        		}
        		else {
        			createRSAcntx(ptr);
        		}
			if( !bSOM || (!isClientMiranda(ptr,1) && !isSecureIM(ptr,1)) || !loadRSAkey(ptr) ) {
				if( ssig==SiG_NONE ) {
        				// просто шлем незашифрованное в оффлайн
					return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
				}
				else {
					// ничего не шлем дальше - это служебное сообщение
					return returnNoError(pccsd->hContact);
				}
			}
			// шлем шифрованное в оффлайн
			LPSTR szUtfMsg = miranda_to_utf8((LPCSTR)pccsd->lParam,pccsd->wParam);
			exp->rsa_send(ptr->cntx,szUtfMsg);
			mir_free(szUtfMsg);
			showPopUpSM(ptr->hContact);
			return returnNoError(pccsd->hContact);
		}
		// SecureIM connection with this contact is disabled
		if( stid==STATUS_DISABLED ) {
			if( ptr->cntx ) {
				exp->rsa_disabled(ptr->cntx);
				deleteRSAcntx(ptr);
			}
			if( ssig==SiG_NONE ) {
			    // просто шлем незашифрованное
			    return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
			}
			// ничего не шлем дальше - это служебное сообщение
			return returnNoError(pccsd->hContact);
		}
		// разорвать соединение
		if( ssig==SiG_DEIN ) {
			if( ptr->cntx ) {
				exp->rsa_disconnect(ptr->cntx);
				deleteRSAcntx(ptr);
			}
			waitForExchange(ptr,3); // дошлем нешифрованно
			return returnNoError(pccsd->hContact);
		}
		// соединение установлено
		if( ptr->cntx && exp->rsa_get_state(ptr->cntx)==7 ) {
			LPSTR szUtfMsg = miranda_to_utf8((LPCSTR)pccsd->lParam,pccsd->wParam);
			exp->rsa_send(ptr->cntx,szUtfMsg);
			mir_free(szUtfMsg);
			showPopUpSM(ptr->hContact);
			return returnNoError(pccsd->hContact);
		}
		// просто сообщение (без тэгов, нет контекста и работают AIP & NOL)
		if( ssig==SiG_NONE && isSecureIM(ptr->hContact) ) {
			// добавим его в очередь
			addMsg2Queue(ptr, pccsd->wParam, (LPSTR)pccsd->lParam);
			// запускаем процесс установки соединения
			ssig=SiG_INIT;
			// запускаем трэд ожидания и досылки
			waitForExchange(ptr);
		}
		// установить соединение
		if( ssig==SiG_INIT ) {
			createRSAcntx(ptr);
			loadRSAkey(ptr);
			exp->rsa_connect(ptr->cntx);
			showPopUpKS(pccsd->hContact);
			ShowStatusIconNotify(pccsd->hContact);
			return returnNoError(pccsd->hContact);
		}
		// просто шлем незашифрованное (не знаю даже когда такое случится)
		return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
	}

	//
	// Native mode
	//
#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("onSendMsg: Native mode");
#endif

	// SecureIM connection with this contact is disabled
	if( stid==STATUS_DISABLED ) {
#if defined(_DEBUG) || defined(NETLIB_LOG)
		Sent_NetLog("onSendMsg: message for Disabled");
#endif
		// if user try initialize connection
		if( ssig==SiG_INIT ) {
			// secure IM is disabled ...
			return returnError(pccsd->hContact,Translate(sim105));
		}
		if (ptr->cntx) { // if exist secure context
			cpp_delete_context(ptr->cntx); ptr->cntx=0;

			CCSDATA ccsd;
			memcpy(&ccsd, (HLOCAL)lParam, sizeof(CCSDATA));

	    		pccsd->wParam |= PREF_METANODB;
			ccsd.lParam = (LPARAM) SIG_DEIN;
			ccsd.szProtoService = PSS_MESSAGE;
			CallService(MS_PROTO_CHAINSEND, wParam, (LPARAM)&ccsd);

			showPopUpDC(pccsd->hContact);
			ShowStatusIconNotify(pccsd->hContact);
		}
		return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
	}

	// contact is offline
	if( stat==ID_STATUS_OFFLINE ) {
#if defined(_DEBUG) || defined(NETLIB_LOG)
		Sent_NetLog("onSendMsg: message for offline");
#endif
		if( ssig==SiG_INIT && cpp_keyx(ptr->cntx) ) {
			// reinit key exchange
			cpp_reset_context(ptr->cntx);
		}

		if( !bSOM ) {
		    if( ssig!=SiG_NONE ) {
				return returnNoError(pccsd->hContact);
		    }
		    // exit and send unencrypted message
		    return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
		}
		BOOL isMiranda = isClientMiranda(ptr->hContact);

		if( stid==STATUS_ALWAYSTRY && isMiranda ) {  // always try && Miranda
			// set key for offline user
			DBVARIANT dbv; dbv.type = DBVT_BLOB;
			if( DBGetContactSettingDword(ptr->hContact, szModuleName, "offlineKeyTimeout", 0) > gettime() &&
			    DBGetContactSetting(ptr->hContact, szModuleName, "offlineKey", &dbv) == 0
			  ) {
				// if valid key is succefully retrieved
				ptr->offlineKey = true;
				InitKeyX(ptr,dbv.pbVal);
				DBFreeVariant(&dbv);
			}
			else {
				DBDeleteContactSetting(ptr->hContact,szModuleName,"offlineKey");
				DBDeleteContactSetting(ptr->hContact,szModuleName,"offlineKeyTimeout");
				if (msgbox1(0,sim106,szModuleName,MB_YESNO|MB_ICONQUESTION)==IDNO) {
					return returnNoError(pccsd->hContact);
				}
				// exit and send unencrypted message
				return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
			}
		}
		else {
/*			if (stid==STATUS_ALWAYSTRY && !isMiranda || stid!=STATUS_ALWAYSTRY && isMiranda) {
				int res=msgbox1(0,"User is offline now, Do you want to send your message ?\nIt will be unencrypted !","Can't Send Encrypted Message !",MB_YESNO);
				if (res==IDNO) return 1;
			}*/
		    if( ssig!=SiG_NONE ) {
			return returnNoError(pccsd->hContact);
		    }
			// exit and send unencrypted message
			return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
		}

	}
	else {
#if defined(_DEBUG) || defined(NETLIB_LOG)
		Sent_NetLog("onSendMsg: message for online");
#endif
		// contact is online and we use an offline key -> reset offline key
		if( ptr->offlineKey ) {
			cpp_reset_context(ptr->cntx);
			ptr->offlineKey = false;
			ShowStatusIconNotify(ptr->hContact);
		}
	}

	// if init is called from contact menu list reinit secure im connection
	if( ssig==SiG_INIT ) {
#if defined(_DEBUG) || defined(NETLIB_LOG)
		Sent_NetLog("onSendMsg: SiG_INIT");
#endif
		cpp_reset_context(ptr->cntx);
	}

	// if deinit is called from contact menu list deinit secure im connection
	if( ssig==SiG_DEIN ) {
#if defined(_DEBUG) || defined(NETLIB_LOG)
		Sent_NetLog("onSendMsg: SiG_DEIN");
#endif
		// disable SecureIM only if it was enabled
		if (ptr->cntx) {
			cpp_delete_context(ptr->cntx); ptr->cntx=0;

			pccsd->wParam |= PREF_METANODB;
			CallService(MS_PROTO_CHAINSEND, wParam, lParam);

			showPopUpDC(pccsd->hContact);
			ShowStatusIconNotify(pccsd->hContact);
		}
		return returnNoError(pccsd->hContact);
	}

	if( cpp_keya(ptr->cntx) && cpp_keyb(ptr->cntx) && !cpp_keyx(ptr->cntx) )
		CalculateKeyX(ptr,ptr->hContact);

	ShowStatusIconNotify(pccsd->hContact);

	// if cryptokey exist
	if( cpp_keyx(ptr->cntx) ) {
#if defined(_DEBUG) || defined(NETLIB_LOG)
		Sent_NetLog("onSendMsg: cryptokey exist");
#endif
/*	    if( !hMetaContact && isProtoMetaContacts(pccsd->hContact) && (DBGetContactSettingByte(NULL, "MetaContacts", "SubcontactHistory", 1) == 1)) {
		// add sent event to subcontact
    		DBEVENTINFO dbei; HANDLE hC = getMostOnline(pccsd->hContact);
		ZeroMemory(&dbei, sizeof(dbei));
		dbei.cbSize = sizeof(dbei);
		dbei.szModule = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hC, 0);
		dbei.flags = DBEF_SENT;
		dbei.timestamp = time(NULL);
		dbei.eventType = EVENTTYPE_MESSAGE;
		if(pccsd->wParam & PREF_RTL) dbei.flags |= DBEF_RTL;
		if(pccsd->wParam & PREF_UTF) dbei.flags |= DBEF_UTF;
		dbei.cbBlob = strlen((char *)pccsd->lParam) + 1;
		if ( pccsd->wParam & PREF_UNICODE )
			dbei.cbBlob *= ( sizeof( wchar_t )+1 );
		dbei.pBlob = (PBYTE)pccsd->lParam;

		CallService(MS_DB_EVENT_ADD, (WPARAM)hC, (LPARAM)&dbei);
	    } */

	    LPSTR szNewMsg = encodeMsg(ptr,(LPARAM)pccsd);

#if defined(_DEBUG) || defined(NETLIB_LOG)
	    Sent_NetLog("onSend: encrypted msg '%s'",szNewMsg);
#endif

	    pccsd->wParam |= PREF_METANODB;
	    pccsd->lParam = (LPARAM) szNewMsg;
            pccsd->szProtoService = PSS_MESSAGE;
            int ret = CallService(MS_PROTO_CHAINSEND, wParam, lParam);

	    mir_free(szNewMsg);

	    showPopUpSM(ptr->hContact);

	    return ret;
	}
	else {
#if defined(_DEBUG) || defined(NETLIB_LOG)
		Sent_NetLog("onSendMsg: cryptokey not exist, try establishe connection");
#endif
	  	// send KeyA if init || always_try || waitkey || always_if_possible
  		if( ssig==SiG_INIT || (stid==STATUS_ALWAYSTRY && isClientMiranda(ptr->hContact)) || isSecureIM(ptr->hContact) || ptr->waitForExchange ) {
			if (ssig==SiG_NONE) {
				addMsg2Queue(ptr, pccsd->wParam, (LPSTR)pccsd->lParam);
			}
			if( !ptr->waitForExchange ) {
				// init || always_try || always_if_possible
				LPSTR keyToSend = InitKeyA(ptr,0);	// calculate public and private key & fill KeyA
#if defined(_DEBUG) || defined(NETLIB_LOG)
				Sent_NetLog("Sending KEY3: %s", keyToSend);
#endif
				pccsd->wParam &= ~PREF_UNICODE;
				pccsd->wParam |= PREF_METANODB;
				pccsd->lParam = (LPARAM) keyToSend;
	  			pccsd->szProtoService = PSS_MESSAGE;
	  			CallService(MS_PROTO_CHAINSEND, wParam, lParam);
	  			mir_free(keyToSend);

	  			showPopUpKS(pccsd->hContact);
	  			ShowStatusIconNotify(pccsd->hContact);

				waitForExchange(ptr); // запускаем ожидание
	  		}
			return returnNoError(pccsd->hContact);
	  	}
#if defined(_DEBUG) || defined(NETLIB_LOG)
		Sent_NetLog("onSendMsg: pass unchanged to chain");
#endif
 		return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
	}
}


int file_idx = 0;

INT_PTR __cdecl onSendFile(WPARAM wParam, LPARAM lParam) {

	CCSDATA *pccsd=(CCSDATA*)lParam;

	pUinKey ptr = getUinKey(pccsd->hContact);
	if (!ptr || !bSFT) return CallService(PSS_FILE, wParam, lParam);

	if( isContactSecured(pccsd->hContact)&SECURED ) {

		char **file=(char **)pccsd->lParam;
		if(file_idx==100) file_idx=0;
		int i;
		for(i=0;file[i];i++) {

			if (strstr(file[i],".AESHELL")) continue;

		    char *name = strrchr(file[i],'\\');
		    if( !name ) name = file[i];
		    else name++;

			char *file_out = (char*) mir_alloc(TEMP_SIZE+strlen(name)+20);
			sprintf(file_out,"%s\\%s.AESHELL(%d)",TEMP,name,file_idx++);

			char buf[MAX_PATH];
			sprintf(buf,"%s\n%s",Translate(sim011),file[i]);
			showPopUp(buf,NULL,g_hPOP[POP_PU_MSS],2);

			if( ptr->mode==MODE_RSAAES ) {
				exp->rsa_encrypt_file(ptr->cntx,file[i],file_out);
			}
			else {
				cpp_encrypt_file(ptr->cntx,file[i],file_out);
			}

			mir_free(file[i]);
			file[i]=file_out;
		}
		if( ptr->fileSend ) { // очистим сохраненный список
			for(int j=0;ptr->fileSend[j];j++) {
				mir_free(ptr->fileSend[j]);
			}
			SAFE_FREE(ptr->fileSend);
		}
		if( i ) { // скопируем новый список
			ptr->fileSend = (char **) mir_alloc(sizeof(char*)*(i+1));
			for(i=0;file[i];i++) {
				ptr->fileSend[i] = mir_strdup(file[i]);
			}
			ptr->fileSend[i] = NULL;
		}
	}
	return CallService(PSS_FILE, wParam, lParam);
}


/*
typedef struct {
	size_t cbSize;
	HANDLE hContact;
	int sending;	//true if sending, false if receiving
	char **files;
	int totalFiles;
	int currentFileNumber;
	unsigned long totalBytes;
	unsigned long totalProgress;
	char *workingDir;
	char *currentFile;
	unsigned long currentFileSize;
	unsigned long currentFileProgress;
	unsigned long currentFileTime;  //as seconds since 1970
} PROTOFILETRANSFERSTATUS;
*/

int __cdecl onProtoAck(WPARAM wParam,LPARAM lParam) {

	ACKDATA *ack=(ACKDATA*)lParam;
	if (ack->type!=ACKTYPE_FILE) return 0; //quit if not file transfer event
	PROTOFILETRANSFERSTATUS *f = (PROTOFILETRANSFERSTATUS*) ack->lParam;

	pUinKey ptr = getUinKey(ack->hContact);
	if (!ptr || (f && f->sending && !bSFT)) return 0;

	if( isContactSecured(ack->hContact)&SECURED ) {
		switch(ack->result) {
//		case ACKRESULT_FILERESUME:
		case ACKRESULT_DATA: {
			if( !f->sending ) {
				ptr->finFileRecv = (f->currentFileSize == f->currentFileProgress);
				if( !ptr->lastFileRecv ) ptr->lastFileRecv = mir_strdup(f->currentFile);
			}
			else
			if( f->sending ) {
				ptr->finFileSend = (f->currentFileSize == f->currentFileProgress);
				if( !ptr->lastFileSend ) ptr->lastFileSend = mir_strdup(f->currentFile);
			}
		} break;
//		case ACKRESULT_INITIALISING:
		case ACKRESULT_DENIED:
		case ACKRESULT_FAILED: {
			if( ptr->lastFileRecv ) {
				if (strstr(ptr->lastFileRecv,".AESHELL")) mir_unlink(ptr->lastFileRecv);
				SAFE_FREE(ptr->lastFileRecv);
			}
			if( ptr->lastFileSend ) {
				if (strstr(ptr->lastFileSend,".AESHELL")) mir_unlink(ptr->lastFileSend);
				SAFE_FREE(ptr->lastFileSend);
			}
			if( ptr->fileSend ) {
				char **file=ptr->fileSend;
        			for(int j=0;file[j];j++) {
					if( strstr(file[j],".AESHELL") ) mir_unlink(file[j]);
					mir_free(file[j]);
				}
				SAFE_FREE(ptr->fileSend);
			}
			return 0;
		} break;
		case ACKRESULT_NEXTFILE:
		case ACKRESULT_SUCCESS: {
			if( ptr->finFileRecv && ptr->lastFileRecv ) {
				if( strstr(ptr->lastFileRecv,".AESHELL") ) {
					char buf[MAX_PATH];
					LPSTR file_out=mir_strdup(ptr->lastFileRecv);
					LPSTR pos=strrchr(file_out,'.'); //find last .
					if (pos) *pos='\0'; //remove AESHELL from name

					if( isFileExist(file_out) ) {
						buf[0]='\0';
						LPSTR p=strrchr(file_out,'.');
						LPSTR x=strrchr(file_out,'\\');
						if(p>x) {
							strcpy(buf,p);
							pos=p;
						}
						for(int i=1;i<10000;i++) {
							sprintf(pos," (%d)%s",i,buf);
							if( !isFileExist(file_out) ) break;
						}
					}

					sprintf(buf,"%s\n%s",Translate(sim012),file_out);
					showPopUp(buf,NULL,g_hPOP[POP_PU_MSR],2);

					if( ptr->mode==MODE_RSAAES ) {
						exp->rsa_decrypt_file(ptr->cntx,ptr->lastFileRecv,file_out);
					}
					else {
						cpp_decrypt_file(ptr->cntx,ptr->lastFileRecv,file_out);
					}
					mir_free(file_out);
					mir_unlink(ptr->lastFileRecv);
				}
				SAFE_FREE(ptr->lastFileRecv);
				ptr->finFileRecv = false;
			}
			if( ptr->finFileSend && ptr->lastFileSend ) {
				if( strstr(ptr->lastFileSend,".AESHELL") ) mir_unlink(ptr->lastFileSend);
				SAFE_FREE(ptr->lastFileSend);
				ptr->finFileSend = false;
			}
		} break;
		} // switch
	}
	return 0;
}


// EOF
