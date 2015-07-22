#include "commonheaders.h"

// return SignID
int getSecureSig(LPCSTR szMsg, LPSTR *szPlainMsg = NULL)
{
	if (szPlainMsg) *szPlainMsg = (LPSTR)szMsg;
	for (int i = 0; signs[i].len; i++) {
		if (memcmp(szMsg, signs[i].sig, signs[i].len) == 0) {
			if (szPlainMsg) *szPlainMsg = (LPSTR)(szMsg + signs[i].len);
			if (signs[i].key == SiG_GAME && !bDGP)
				return SiG_NONE;

			return signs[i].key;
		}
	}
	return SiG_NONE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void sttFakeAck(LPVOID param)
{
	TFakeAckParams *tParam = (TFakeAckParams*)param;

	Sleep(100);
	if (tParam->msg == NULL)
		SendBroadcast(tParam->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)tParam->id, 0);
	else
		SendBroadcast(tParam->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)tParam->id, LPARAM(tParam->msg));

	delete tParam;
}

int returnNoError(MCONTACT hContact)
{
	mir_forkthread(sttFakeAck, new TFakeAckParams(hContact, 777, 0));
	return 777;
}

int returnError(MCONTACT hContact, LPCSTR err)
{
	mir_forkthread(sttFakeAck, new TFakeAckParams(hContact, 666, err));
	return 666;
}

LPSTR szUnrtfMsg = NULL;

// RecvMsg handler
INT_PTR __cdecl onRecvMsg(WPARAM wParam, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA *)lParam;
	PROTORECVEVENT *ppre = (PROTORECVEVENT *)ccs->lParam;
	pUinKey ptr = getUinKey(ccs->hContact);
	LPSTR szEncMsg = ppre->szMessage, szPlainMsg = NULL;

	Sent_NetLog("onRecvMsg: %s", szEncMsg);

	int ssig = getSecureSig(ppre->szMessage, &szEncMsg);
	bool bSecured = (isContactSecured(ccs->hContact)&SECURED) != 0;
	bool bPGP = isContactPGP(ccs->hContact);
	bool bGPG = isContactGPG(ccs->hContact);

	// pass any unchanged message
	if (!ptr || ssig == SiG_GAME || !isSecureProtocol(ccs->hContact) ||
		 (db_mc_isMeta(ccs->hContact) && (ccs->wParam & PREF_SIMNOMETA)) || isChatRoom(ccs->hContact) ||
		 (ssig == SiG_NONE && !ptr->msgSplitted && !bSecured && !bPGP && !bGPG)) {
		Sent_NetLog("onRecvMsg: pass unhandled");
		return Proto_ChainRecv(wParam, ccs);
	}

	// drop message: fake, unsigned or from invisible contacts
	if (isContactInvisible(ccs->hContact) || ssig == SiG_FAKE) {
		Sent_NetLog("onRecvMsg: drop unhandled (contact invisible or hidden)");
		return 1;
	}

	// receive non-secure message in secure mode
	if (ssig == SiG_NONE && !ptr->msgSplitted) {
		Sent_NetLog("onRecvMsg: non-secure message");

		ptrA szPlainMsg(m_aastrcat(Translate(sim402), szEncMsg));
		ppre->szMessage = szPlainMsg;
		ccs->wParam |= PREF_SIMNOMETA;
		return Proto_ChainRecv(wParam, ccs);
	}

	// received non-pgp secure message from disabled contact
	if (ssig != SiG_PGPM && !bPGP && !bGPG && ptr->status == STATUS_DISABLED) {
		Sent_NetLog("onRecvMsg: message from disabled");

		if (ptr->mode == MODE_NATIVE) {
			// tell to the other side that we have the plugin disabled with him
			ccs->wParam |= PREF_METANODB;
			ccs->lParam = (LPARAM)SIG_DISA;
			ccs->szProtoService = PSS_MESSAGE;
			Proto_ChainSend(wParam, ccs);

			showPopup(sim003, ccs->hContact, g_hPOP[POP_PU_DIS], 0);
		}
		else {
			createRSAcntx(ptr);
			mir_exp->rsa_disabled(ptr->cntx);
			deleteRSAcntx(ptr);
		}
		SAFE_FREE(ptr->msgSplitted);
		return 1;
	}

	// combine message splitted by protocol (no tags)
	if (ssig == SiG_NONE && ptr->msgSplitted) {
		Sent_NetLog("onRecvMsg: combine untagged splitted message");

		LPSTR tmp = (LPSTR)mir_alloc(mir_strlen(ptr->msgSplitted) + mir_strlen(szEncMsg) + 1);
		mir_strcpy(tmp, ptr->msgSplitted);
		mir_strcat(tmp, szEncMsg);
		mir_free(ptr->msgSplitted);
		ptr->msgSplitted = szEncMsg = ppre->szMessage = tmp;
		ssig = getSecureSig(tmp, &szEncMsg);
	}
	else SAFE_FREE(ptr->msgSplitted);

	// combine message splitted by secureim (with tags)
	if (ssig == SiG_SECP || ssig == SiG_PART) {
		LPSTR msg = combineMessage(ptr, szEncMsg);
		if (!msg) return 1;
		szEncMsg = ppre->szMessage = msg;
		ssig = getSecureSig(msg, &szEncMsg);
	}

	// decrypt PGP/GPG message
	if (ssig == SiG_PGPM && ((bPGPloaded && (bPGPkeyrings || bPGPprivkey)) || (bGPGloaded && bGPGkeyrings))) {
		Sent_NetLog("onRecvMsg: PGP/GPG message");

		szEncMsg = ppre->szMessage;
		if (!ptr->cntx) {
			ptr->cntx = cpp_create_context(((bGPGloaded && bGPGkeyrings) ? CPP_MODE_GPG : CPP_MODE_PGP) | ((db_get_b(ccs->hContact, MODULENAME, "gpgANSI", 0)) ? CPP_MODE_GPG_ANSI : 0));
			ptr->keyLoaded = 0;
		}

		if (!strstr(szEncMsg, "-----END PGP MESSAGE-----"))
			return 1; // no end tag, don't display it ...

		LPSTR szNewMsg = NULL;
		LPSTR szOldMsg = NULL;

		if (!ptr->keyLoaded && bPGPloaded) ptr->keyLoaded = LoadKeyPGP(ptr);
		if (!ptr->keyLoaded && bGPGloaded) ptr->keyLoaded = LoadKeyGPG(ptr);

		if (ptr->keyLoaded == 1) szOldMsg = pgp_decode(ptr->cntx, szEncMsg);
		else
			if (ptr->keyLoaded == 2) szOldMsg = gpg_decode(ptr->cntx, szEncMsg);

		if (!szOldMsg) { // error while decrypting message, send error
			SAFE_FREE(ptr->msgSplitted);
			ppre->szMessage = Translate(sim401);
			return Proto_ChainRecv(wParam, ccs);
		}

		// receive encrypted message in non-encrypted mode
		if (!isContactPGP(ccs->hContact) && !isContactGPG(ccs->hContact)) {
			szNewMsg = m_ustrcat(Translate(sim403), szOldMsg);
			szOldMsg = szNewMsg;
		}
		ptrA szMsgUtf(utf8_to_miranda(szOldMsg, ppre->flags));
		ccs->wParam = ppre->flags;
		ppre->szMessage = szMsgUtf;

		// show decoded message
		showPopupRM(ptr->hContact);
		SAFE_FREE(ptr->msgSplitted);
		ccs->wParam |= PREF_SIMNOMETA;
		return Proto_ChainRecv(wParam, ccs);
	}

	Sent_NetLog("onRecvMsg: switch(ssig)=%d", ssig);

	switch (ssig) {
	case SiG_PGPM:
		return Proto_ChainRecv(wParam, ccs);

	case SiG_SECU: // new secured msg, pass to rsa_recv
		Sent_NetLog("onRecvMsg: RSA/AES message");

		if (ptr->mode == MODE_NATIVE) {
			ptr->mode = MODE_RSAAES;
			deleteRSAcntx(ptr);
			db_set_b(ptr->hContact, MODULENAME, "mode", ptr->mode);
		}
		createRSAcntx(ptr);
		loadRSAkey(ptr);
		if (mir_exp->rsa_get_state(ptr->cntx) == 0)
			showPopupKR(ptr->hContact);

		{
			LPSTR szOldMsg = mir_exp->rsa_recv(ptr->cntx, szEncMsg);
			if (!szOldMsg)
				return 1; // don't display it ...

			ptrA szNewMsg(utf8_to_miranda(szOldMsg, ppre->flags));
			ccs->wParam = ppre->flags;
			ppre->szMessage = szNewMsg;

			// show decoded message
			showPopupRM(ptr->hContact);
			SAFE_FREE(ptr->msgSplitted);
			ccs->wParam |= PREF_SIMNOMETA;
			return Proto_ChainRecv(wParam, ccs);
		}

	case SiG_ENON: // online message
		Sent_NetLog("onRecvMsg: Native SiG_ENON message");

		if (cpp_keyx(ptr->cntx)) {
			// decrypting message
			szPlainMsg = decodeMsg(ptr, lParam, szEncMsg);
			if (!ptr->decoded) {
				mir_free(szPlainMsg);
				SAFE_FREE(ptr->msgSplitted);
				ptr->msgSplitted = mir_strdup(szEncMsg);
				return 1; // don't display it ...
			}
		}
		else {
			// reinit key exchange user has send an encrypted message and i have no key
			cpp_reset_context(ptr->cntx);

			ptrA reSend((LPSTR)mir_alloc(mir_strlen(szEncMsg) + LEN_RSND));
			mir_strcpy(reSend, SIG_RSND); // copy resend sig
			mir_strcat(reSend, szEncMsg); // add mess

			ccs->wParam |= PREF_METANODB;
			ccs->lParam = (LPARAM)reSend; // reSend Message to reemit
			ccs->szProtoService = PSS_MESSAGE;
			Proto_ChainSend(wParam, ccs); // send back cipher message

			ptrA keyToSend(InitKeyA(ptr, 0)); // calculate public and private key
			ccs->lParam = keyToSend;
			Proto_ChainSend(wParam, ccs); // send new key

			showPopup(sim005, NULL, g_hPOP[POP_PU_DIS], 0);
			showPopupKS(ptr->hContact);
			return 1;
		}
		break;

	case SiG_ENOF: // offline message
		Sent_NetLog("onRecvMsg: Native SiG_ENOF message");

		// if offline key is set and we have not an offline message unset key
		if (ptr->offlineKey && cpp_keyx(ptr->cntx)) {
			cpp_reset_context(ptr->cntx);
			ptr->offlineKey = false;
		}

		// decrypting message with last offline key
		DBVARIANT dbv;
		dbv.type = DBVT_BLOB;
		if (db_get(ptr->hContact, MODULENAME, "offlineKey", &dbv))
			return Proto_ChainRecv(wParam, ccs); // exit and show messsage

		// if valid key is succefully retrieved
		ptr->offlineKey = true;
		InitKeyX(ptr, dbv.pbVal);
		db_free(&dbv);

		// decrypting message
		szPlainMsg = decodeMsg(ptr, lParam, szEncMsg);
		ShowStatusIconNotify(ptr->hContact);
		break;

	case SiG_RSND: // resend message
		Sent_NetLog("onRecvMsg: Native SiG_RSND message");

		if (cpp_keyx(ptr->cntx)) {
			// decrypt sended back message and save message for future sending with a new secret key
			addMsg2Queue(ptr, ccs->wParam, ptrA(decodeMsg(ptr, (LPARAM)ccs, szEncMsg)));
			showPopupRM(ptr->hContact);
			showPopup(sim004, NULL, g_hPOP[POP_PU_DIS], 0);
		}
		return 1; // don't display it ...

	case SiG_DISA: // disabled message
		Sent_NetLog("onRecvMsg: Native SiG_DISA message");

	case SiG_DEIN: // deinit message
		// other user has disabled SecureIM with you
		cpp_delete_context(ptr->cntx); ptr->cntx = 0;

		showPopupDC(ptr->hContact);
		ShowStatusIconNotify(ptr->hContact);

		waitForExchange(ptr, 3); // дослать нешифрованно
		return 1;

	case SiG_KEYR: // key3 message
	case SiG_KEYA: // keyA message
	case SiG_KEYB: // keyB message
		if (ptr->mode == MODE_RSAAES) {
			ptr->mode = MODE_NATIVE;
			cpp_delete_context(ptr->cntx);
			ptr->cntx = 0;
			ptr->keyLoaded = 0;
			db_set_b(ptr->hContact, MODULENAME, "mode", ptr->mode);
		}

		switch (ssig) {
		case SiG_KEYR: // key3 message
			Sent_NetLog("onRecvMsg: SiG_KEYR received");

			// receive KeyB from user;
			showPopupKR(ptr->hContact);

			if (ptr->cntx && cpp_keyb(ptr->cntx)) {
				// reinit key exchange if an old key from user is found
				cpp_reset_context(ptr->cntx);
			}

			if (InitKeyB(ptr, szEncMsg) != CPP_ERROR_NONE) {
				Sent_NetLog("onRecvMsg: SiG_KEYR InitKeyB error");

				// tell to the other side that we have the plugin disabled with him
				showPopup(sim013, ptr->hContact, g_hPOP[POP_PU_DIS], 0);
				ShowStatusIconNotify(ptr->hContact);

				waitForExchange(ptr, 3); // дослать нешифрованно
				return 1;
			}

			// other side support RSA mode ?
			if (ptr->features & CPP_FEATURES_RSA) {
				// switch to RSAAES mode
				ptr->mode = MODE_RSAAES;
				db_set_b(ptr->hContact, MODULENAME, "mode", ptr->mode);

				resetRSAcntx(ptr);
				loadRSAkey(ptr);
				mir_exp->rsa_connect(ptr->cntx);

				showPopupKS(ccs->hContact);
				ShowStatusIconNotify(ccs->hContact);
				return 1;
			}

			// other side support new key format ?
			if (ptr->features & CPP_FEATURES_NEWPG) {
				cpp_reset_context(ptr->cntx);

				ptrA keyToSend(InitKeyA(ptr, CPP_FEATURES_NEWPG | KEY_A_SIG)); // calculate NEW public and private key
				Sent_NetLog("onRecvMsg: Sending KEYA %s", keyToSend);

				ccs->wParam |= PREF_METANODB;
				ccs->lParam = (LPARAM)keyToSend;
				ccs->szProtoService = PSS_MESSAGE;
				Proto_ChainSend(wParam, ccs);

				showPopupKS(ptr->hContact);
				waitForExchange(ptr); // запустим ожидание
				return 1;
			}

			// auto send my public key to keyB user if not done before
			if (!cpp_keya(ptr->cntx)) {
				ptrA keyToSend(InitKeyA(ptr, 0)); // calculate public and private key
				Sent_NetLog("onRecvMsg: Sending KEYA %s", keyToSend);

				ccs->wParam |= PREF_METANODB;
				ccs->lParam = (LPARAM)keyToSend;
				ccs->szProtoService = PSS_MESSAGE;
				Proto_ChainSend(wParam, ccs);

				showPopupKS(ptr->hContact);
			}
			break;

		case SiG_KEYA: // keyA message
			Sent_NetLog("onRecvMsg: SiG_KEYA received");

			// receive KeyA from user;
			showPopupKR(ptr->hContact);

			cpp_reset_context(ptr->cntx);
			if (InitKeyB(ptr, szEncMsg) != CPP_ERROR_NONE) {
				Sent_NetLog("onRecvMsg: SiG_KEYA InitKeyB error");

				// tell to the other side that we have the plugin disabled with him
				showPopup(sim013, ptr->hContact, g_hPOP[POP_PU_DIS], 0);
				ShowStatusIconNotify(ptr->hContact);

				waitForExchange(ptr, 3); // дослать нешифрованно
				return 1;
			}
			else {
				ptrA keyToSend(InitKeyA(ptr, CPP_FEATURES_NEWPG | KEY_B_SIG)); // calculate NEW public and private key
				Sent_NetLog("onRecvMsg: Sending KEYB %s", keyToSend);

				ccs->wParam |= PREF_METANODB;
				ccs->lParam = keyToSend;
				ccs->szProtoService = PSS_MESSAGE;
				Proto_ChainSend(wParam, ccs);
			}
			break;

		case SiG_KEYB: // keyB message
			Sent_NetLog("onRecvMsg: SiG_KEYB received");

			// receive KeyB from user;
			showPopupKR(ptr->hContact);

			// clear all and send DISA if received KeyB, and not exist KeyA or error on InitKeyB
			if (!cpp_keya(ptr->cntx) || InitKeyB(ptr, szEncMsg) != CPP_ERROR_NONE) {
				Sent_NetLog("onRecvMsg: SiG_KEYB InitKeyB error");

				// tell to the other side that we have the plugin disabled with him
				showPopup(sim013, ptr->hContact, g_hPOP[POP_PU_DIS], 0);
				ShowStatusIconNotify(ptr->hContact);

				cpp_reset_context(ptr->cntx);
				waitForExchange(ptr, 3); // дослать нешифрованно
				return 1;
			}
			break;
		}

		/* common part (CalcKeyX & SendQueue) */
		//  calculate KeyX
		if (cpp_keya(ptr->cntx) && cpp_keyb(ptr->cntx) && !cpp_keyx(ptr->cntx))
			CalculateKeyX(ptr, ptr->hContact);

		ShowStatusIconNotify(ptr->hContact);
		Sent_NetLog("onRecvMsg: Session established");

		waitForExchange(ptr, 2); // дошлем через шифрованное соединение
		return 1;
	}

	// receive message
	if (cpp_keyx(ptr->cntx) && (ssig == SiG_ENON || ssig == SiG_ENOF)) {
		Sent_NetLog("onRecvMsg: message received");
		showPopupRM(ptr->hContact);
	}

	Sent_NetLog("onRecvMsg: exit");

	ccs->wParam |= PREF_SIMNOMETA;
	int ret = Proto_ChainRecv(wParam, ccs);
	SAFE_FREE(szPlainMsg);
	return ret;
}

// SendMsg handler
INT_PTR __cdecl onSendMsg(WPARAM wParam, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA*)lParam;
	pUinKey ptr = getUinKey(ccs->hContact);
	int ssig = getSecureSig((LPCSTR)ccs->lParam);
	int stat = getContactStatus(ccs->hContact);

	Sent_NetLog("onSend: %s", (LPSTR)ccs->lParam);

	// pass unhandled messages
	if (!ptr || ssig == SiG_GAME || ssig == SiG_PGPM || ssig == SiG_SECU || ssig == SiG_SECP ||
		isChatRoom(ccs->hContact) || stat == -1 ||
		(ssig == SiG_NONE && ptr->sendQueue) || (ssig == SiG_NONE && ptr->status == STATUS_DISABLED)) {
			Sent_NetLog("onSendMsg: pass unhandled");
			return Proto_ChainSend(wParam, ccs);
	}

	//
	// PGP/GPG mode
	// 
	if (ptr->mode == MODE_PGP || ptr->mode == MODE_GPG) {
		Sent_NetLog("onSendMsg: PGP|GPG mode");

		// если можно зашифровать - шифруем
		if (isContactPGP(ptr->hContact) || isContactGPG(ptr->hContact)) {
			if (!ptr->cntx) {
				ptr->cntx = cpp_create_context((isContactGPG(ptr->hContact) ? CPP_MODE_GPG : CPP_MODE_PGP) | ((db_get_b(ptr->hContact, MODULENAME, "gpgANSI", 0)) ? CPP_MODE_GPG_ANSI : 0));
				ptr->keyLoaded = 0;
			}
			if (!ptr->keyLoaded && bPGPloaded) ptr->keyLoaded = LoadKeyPGP(ptr);
			if (!ptr->keyLoaded && bGPGloaded) ptr->keyLoaded = LoadKeyGPG(ptr);
			if (!ptr->keyLoaded) return returnError(ccs->hContact, Translate(sim108));

			LPSTR szNewMsg = NULL;
			ptrA szUtfMsg(miranda_to_utf8((LPCSTR)ccs->lParam, ccs->wParam));
			if (ptr->keyLoaded == 1) // PGP
				szNewMsg = pgp_encode(ptr->cntx, szUtfMsg);
			else if (ptr->keyLoaded == 2) // GPG
				szNewMsg = gpg_encode(ptr->cntx, szUtfMsg);

			if (!szNewMsg)
				return returnError(ccs->hContact, Translate(sim109));

			// отправляем зашифрованное сообщение
			splitMessageSend(ptr, szNewMsg);

			showPopupSM(ptr->hContact);

			return returnNoError(ccs->hContact);
		}

		// отправляем незашифрованное
		return Proto_ChainSend(wParam, ccs);
	}

	// get contact SecureIM status
	int stid = ptr->status;

	//
	// RSA/AES mode
	//
	if (ptr->mode == MODE_RSAAES) {
		Sent_NetLog("onSendMsg: RSA/AES mode");

		// contact is offline
		if (stat == ID_STATUS_OFFLINE) {
			if (ptr->cntx) {
				if (mir_exp->rsa_get_state(ptr->cntx) != 0)
					resetRSAcntx(ptr);
			}
			else createRSAcntx(ptr);

			if (!bSOM || (!isClientMiranda(ptr, 1) && !isSecureIM(ptr, 1)) || !loadRSAkey(ptr)) {
				if (ssig == SiG_NONE)
					// просто шлем незашифрованное в оффлайн
					return Proto_ChainSend(wParam, ccs);

				// ничего не шлем дальше - это служебное сообщение
				return returnNoError(ccs->hContact);
			}

			// шлем шифрованное в оффлайн
			mir_exp->rsa_send(ptr->cntx, ptrA(miranda_to_utf8((LPCSTR)ccs->lParam, ccs->wParam)));
			showPopupSM(ptr->hContact);
			return returnNoError(ccs->hContact);
		}

		// SecureIM connection with this contact is disabled
		if (stid == STATUS_DISABLED) {
			if (ptr->cntx) {
				mir_exp->rsa_disabled(ptr->cntx);
				deleteRSAcntx(ptr);
			}

			if (ssig == SiG_NONE) // просто шлем незашифрованное
				return Proto_ChainSend(wParam, ccs);

			// ничего не шлем дальше - это служебное сообщение
			return returnNoError(ccs->hContact);
		}

		// разорвать соединение
		if (ssig == SiG_DEIN) {
			if (ptr->cntx) {
				mir_exp->rsa_disconnect(ptr->cntx);
				deleteRSAcntx(ptr);
			}
			ShowStatusIconNotify(ptr->hContact);
			waitForExchange(ptr, 3); // дошлем нешифрованно
			return returnNoError(ccs->hContact);
		}

		// соединение установлено
		if (ptr->cntx && mir_exp->rsa_get_state(ptr->cntx) == 7) {
			mir_exp->rsa_send(ptr->cntx, ptrA(miranda_to_utf8((LPCSTR)ccs->lParam, ccs->wParam)));
			ShowStatusIconNotify(ptr->hContact);
			showPopupSM(ptr->hContact);
			return returnNoError(ccs->hContact);
		}

		// просто сообщение (без тэгов, нет контекста и работают AIP & NOL)
		if (ssig == SiG_NONE && isSecureIM(ptr->hContact)) {
			// добавим его в очередь
			addMsg2Queue(ptr, ccs->wParam, (LPSTR)ccs->lParam);
			// запускаем процесс установки соединения
			ssig = SiG_INIT;
			// запускаем трэд ожидания и досылки
			waitForExchange(ptr);
		}

		// установить соединение
		if (ssig == SiG_INIT) {
			createRSAcntx(ptr);
			loadRSAkey(ptr);
			mir_exp->rsa_connect(ptr->cntx);
			showPopupKS(ccs->hContact);
			ShowStatusIconNotify(ccs->hContact);
			return returnNoError(ccs->hContact);
		}

		// просто шлем незашифрованное (не знаю даже когда такое случится)
		return Proto_ChainSend(wParam, ccs);
	}

	//
	// Native mode
	//
	Sent_NetLog("onSendMsg: Native mode");

	// SecureIM connection with this contact is disabled
	if (stid == STATUS_DISABLED) {
		Sent_NetLog("onSendMsg: message for Disabled");

		// if user try initialize connection
		if (ssig == SiG_INIT) // secure IM is disabled ...
			return returnError(ccs->hContact, Translate(sim105));

		if (ptr->cntx) { // if secure context exists
			cpp_delete_context(ptr->cntx); ptr->cntx = 0;

			CCSDATA ccsd;
			memcpy(&ccsd, (HLOCAL)lParam, sizeof(CCSDATA));

			ccs->wParam |= PREF_METANODB;
			ccsd.lParam = (LPARAM)SIG_DEIN;
			ccsd.szProtoService = PSS_MESSAGE;
			Proto_ChainSend(wParam, &ccsd);

			showPopupDC(ccs->hContact);
			ShowStatusIconNotify(ccs->hContact);
		}
		return Proto_ChainSend(wParam, ccs);
	}

	// contact is offline
	if (stat == ID_STATUS_OFFLINE) {
		Sent_NetLog("onSendMsg: message for offline");

		if (ssig == SiG_INIT && cpp_keyx(ptr->cntx)) // reinit key exchange
			cpp_reset_context(ptr->cntx);

		if (!bSOM) {
			if (ssig != SiG_NONE)
				return returnNoError(ccs->hContact);

			// exit and send unencrypted message
			return Proto_ChainSend(wParam, ccs);
		}
		BOOL isMiranda = isClientMiranda(ptr->hContact);

		if (stid == STATUS_ALWAYSTRY && isMiranda) {  // always try && Miranda
			// set key for offline user
			DBVARIANT dbv; dbv.type = DBVT_BLOB;
			if (db_get_dw(ptr->hContact, MODULENAME, "offlineKeyTimeout", 0) > gettime() &&
				 db_get(ptr->hContact, MODULENAME, "offlineKey", &dbv) == 0) {
				// if valid key is succefully retrieved
				ptr->offlineKey = true;
				InitKeyX(ptr, dbv.pbVal);
				db_free(&dbv);
			}
			else {
				db_unset(ptr->hContact, MODULENAME, "offlineKey");
				db_unset(ptr->hContact, MODULENAME, "offlineKeyTimeout");
				if (msgbox1(0, sim106, MODULENAME, MB_YESNO | MB_ICONQUESTION) == IDNO)
					return returnNoError(ccs->hContact);

				// exit and send unencrypted message
				return Proto_ChainSend(wParam, ccs);
			}
		}
		else {
			if (ssig != SiG_NONE)
				return returnNoError(ccs->hContact);

			// exit and send unencrypted message
			return Proto_ChainSend(wParam, ccs);
		}
	}
	else {
		Sent_NetLog("onSendMsg: message for online");

		// contact is online and we use an offline key -> reset offline key
		if (ptr->offlineKey) {
			cpp_reset_context(ptr->cntx);
			ptr->offlineKey = false;
			ShowStatusIconNotify(ptr->hContact);
		}
	}

	// if init is called from contact menu list reinit secure im connection
	if (ssig == SiG_INIT) {
		Sent_NetLog("onSendMsg: SiG_INIT");
		cpp_reset_context(ptr->cntx);
	}

	// if deinit is called from contact menu list deinit secure im connection
	if (ssig == SiG_DEIN) {
		Sent_NetLog("onSendMsg: SiG_DEIN");

		// disable SecureIM only if it was enabled
		if (ptr->cntx) {
			cpp_delete_context(ptr->cntx); ptr->cntx = 0;

			ccs->wParam |= PREF_METANODB;
			Proto_ChainSend(wParam, ccs);

			showPopupDC(ccs->hContact);
			ShowStatusIconNotify(ccs->hContact);
		}
		return returnNoError(ccs->hContact);
	}

	if (cpp_keya(ptr->cntx) && cpp_keyb(ptr->cntx) && !cpp_keyx(ptr->cntx))
		CalculateKeyX(ptr, ptr->hContact);

	ShowStatusIconNotify(ccs->hContact);

	// if cryptokey exist
	if (cpp_keyx(ptr->cntx)) {
		Sent_NetLog("onSendMsg: cryptokey exist");

		ptrA szNewMsg(encodeMsg(ptr, (LPARAM)ccs));
		Sent_NetLog("onSend: encrypted msg '%s'", szNewMsg);

		ccs->wParam |= PREF_METANODB;
		ccs->lParam = szNewMsg;
		ccs->szProtoService = PSS_MESSAGE;
		int ret = Proto_ChainSend(wParam, ccs);

		showPopupSM(ptr->hContact);
		return ret;
	}

	Sent_NetLog("onSendMsg: cryptokey not exist, try establishe connection");

	// send KeyA if init || always_try || waitkey || always_if_possible
	if (ssig == SiG_INIT || (stid == STATUS_ALWAYSTRY && isClientMiranda(ptr->hContact)) || isSecureIM(ptr->hContact) || ptr->waitForExchange) {
		if (ssig == SiG_NONE)
			addMsg2Queue(ptr, ccs->wParam, (LPSTR)ccs->lParam);

		if (!ptr->waitForExchange) {
			// init || always_try || always_if_possible
			ptrA keyToSend(InitKeyA(ptr, 0));	// calculate public and private key & fill KeyA
			Sent_NetLog("Sending KEY3: %s", keyToSend);

			ccs->wParam |= PREF_METANODB;
			ccs->lParam = (LPARAM)keyToSend;
			ccs->szProtoService = PSS_MESSAGE;
			Proto_ChainSend(wParam, ccs);

			showPopupKS(ccs->hContact);
			ShowStatusIconNotify(ccs->hContact);

			waitForExchange(ptr); // запускаем ожидание
		}
		return returnNoError(ccs->hContact);
	}

	Sent_NetLog("onSendMsg: pass unchanged to chain");
	return Proto_ChainSend(wParam, ccs);
}
