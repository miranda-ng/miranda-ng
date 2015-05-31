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
	CCSDATA *pccsd = (CCSDATA *)lParam;
	PROTORECVEVENT *ppre = (PROTORECVEVENT *)pccsd->lParam;
	pUinKey ptr = getUinKey(pccsd->hContact);
	LPSTR szEncMsg = ppre->szMessage, szPlainMsg = NULL;

	Sent_NetLog("onRecvMsg: %s", szEncMsg);

	int ssig = getSecureSig(ppre->szMessage, &szEncMsg);
	bool bSecured = (isContactSecured(pccsd->hContact)&SECURED) != 0;
	bool bPGP = isContactPGP(pccsd->hContact);
	bool bGPG = isContactGPG(pccsd->hContact);

	// pass any unchanged message
	if (!ptr || ssig == SiG_GAME || !isSecureProtocol(pccsd->hContact) ||
		 (db_mc_isMeta(pccsd->hContact) && (pccsd->wParam & PREF_SIMNOMETA)) || isChatRoom(pccsd->hContact) ||
		 (ssig == SiG_NONE && !ptr->msgSplitted && !bSecured && !bPGP && !bGPG)) {
		Sent_NetLog("onRecvMsg: pass unhandled");
		return CallService(MS_PROTO_CHAINRECV, wParam, lParam);
	}

	// drop message: fake, unsigned or from invisible contacts
	if (isContactInvisible(pccsd->hContact) || ssig == SiG_FAKE) {
		Sent_NetLog("onRecvMsg: drop unhandled (contact invisible or hidden)");
		return 1;
	}

	// receive non-secure message in secure mode
	if (ssig == SiG_NONE && !ptr->msgSplitted) {
		Sent_NetLog("onRecvMsg: non-secure message");

		ptrA szPlainMsg(m_aastrcat(Translate(sim402), szEncMsg));
		ppre->szMessage = szPlainMsg;
		pccsd->wParam |= PREF_SIMNOMETA;
		return CallService(MS_PROTO_CHAINRECV, wParam, lParam);
	}

	// received non-pgp secure message from disabled contact
	if (ssig != SiG_PGPM && !bPGP && !bGPG && ptr->status == STATUS_DISABLED) {
		Sent_NetLog("onRecvMsg: message from disabled");

		if (ptr->mode == MODE_NATIVE) {
			// tell to the other side that we have the plugin disabled with him
			pccsd->wParam |= PREF_METANODB;
			pccsd->lParam = (LPARAM)SIG_DISA;
			pccsd->szProtoService = PSS_MESSAGE;
			CallService(MS_PROTO_CHAINSEND, wParam, lParam);

			showPopup(sim003, pccsd->hContact, g_hPOP[POP_PU_DIS], 0);
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
			ptr->cntx = cpp_create_context(((bGPGloaded && bGPGkeyrings) ? CPP_MODE_GPG : CPP_MODE_PGP) | ((db_get_b(pccsd->hContact, MODULENAME, "gpgANSI", 0)) ? CPP_MODE_GPG_ANSI : 0));
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
			return CallService(MS_PROTO_CHAINRECV, wParam, lParam);
		}

		// receive encrypted message in non-encrypted mode
		if (!isContactPGP(pccsd->hContact) && !isContactGPG(pccsd->hContact)) {
			szNewMsg = m_ustrcat(Translate(sim403), szOldMsg);
			szOldMsg = szNewMsg;
		}
		ptrA szMsgUtf(utf8_to_miranda(szOldMsg, ppre->flags));
		pccsd->wParam = ppre->flags;
		ppre->szMessage = szMsgUtf;

		// show decoded message
		showPopupRM(ptr->hContact);
		SAFE_FREE(ptr->msgSplitted);
		pccsd->wParam |= PREF_SIMNOMETA;
		return CallService(MS_PROTO_CHAINRECV, wParam, lParam);
	}

	Sent_NetLog("onRecvMsg: switch(ssig)=%d", ssig);

	switch (ssig) {
	case SiG_PGPM:
		return CallService(MS_PROTO_CHAINRECV, wParam, lParam);

	case SiG_SECU: // new secured msg, pass to rsa_recv
		Sent_NetLog("onRecvMsg: RSA/AES message");

		if (ptr->mode == MODE_NATIVE) {
			ptr->mode = MODE_RSAAES;
			deleteRSAcntx(ptr);
			db_set_b(ptr->hContact, MODULENAME, "mode", ptr->mode);
		}
		createRSAcntx(ptr);
		loadRSAkey(ptr);
		if (exp->rsa_get_state(ptr->cntx) == 0)
			showPopupKR(ptr->hContact);

		{
			LPSTR szOldMsg = exp->rsa_recv(ptr->cntx, szEncMsg);
			if (!szOldMsg)
				return 1; // don't display it ...

			ptrA szNewMsg(utf8_to_miranda(szOldMsg, ppre->flags));
			pccsd->wParam = ppre->flags;
			ppre->szMessage = szNewMsg;

			// show decoded message
			showPopupRM(ptr->hContact);
			SAFE_FREE(ptr->msgSplitted);
			pccsd->wParam |= PREF_SIMNOMETA;
			return CallService(MS_PROTO_CHAINRECV, wParam, lParam);
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

			pccsd->wParam |= PREF_METANODB;
			pccsd->lParam = (LPARAM)reSend; // reSend Message to reemit
			pccsd->szProtoService = PSS_MESSAGE;
			CallService(MS_PROTO_CHAINSEND, wParam, lParam); // send back cipher message

			ptrA keyToSend(InitKeyA(ptr, 0)); // calculate public and private key
			pccsd->lParam = (LPARAM)(char*)keyToSend;
			CallService(MS_PROTO_CHAINSEND, wParam, lParam); // send new key

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
			return CallService(MS_PROTO_CHAINRECV, wParam, lParam); // exit and show messsage

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
			addMsg2Queue(ptr, pccsd->wParam, ptrA(decodeMsg(ptr, (LPARAM)pccsd, szEncMsg)));
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

		waitForExchange(ptr, 3); // ������� ������������
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

				waitForExchange(ptr, 3); // ������� ������������
				return 1;
			}

			// other side support RSA mode ?
			if (ptr->features & CPP_FEATURES_RSA) {
				// switch to RSAAES mode
				ptr->mode = MODE_RSAAES;
				db_set_b(ptr->hContact, MODULENAME, "mode", ptr->mode);

				resetRSAcntx(ptr);
				loadRSAkey(ptr);
				exp->rsa_connect(ptr->cntx);

				showPopupKS(pccsd->hContact);
				ShowStatusIconNotify(pccsd->hContact);
				return 1;
			}

			// other side support new key format ?
			if (ptr->features & CPP_FEATURES_NEWPG) {
				cpp_reset_context(ptr->cntx);

				ptrA keyToSend(InitKeyA(ptr, CPP_FEATURES_NEWPG | KEY_A_SIG)); // calculate NEW public and private key
				Sent_NetLog("onRecvMsg: Sending KEYA %s", keyToSend);

				pccsd->wParam |= PREF_METANODB;
				pccsd->lParam = (LPARAM)keyToSend;
				pccsd->szProtoService = PSS_MESSAGE;
				CallService(MS_PROTO_CHAINSEND, wParam, lParam);

				showPopupKS(ptr->hContact);
				waitForExchange(ptr); // �������� ��������
				return 1;
			}

			// auto send my public key to keyB user if not done before
			if (!cpp_keya(ptr->cntx)) {
				ptrA keyToSend(InitKeyA(ptr, 0)); // calculate public and private key
				Sent_NetLog("onRecvMsg: Sending KEYA %s", keyToSend);

				pccsd->wParam |= PREF_METANODB;
				pccsd->lParam = (LPARAM)keyToSend;
				pccsd->szProtoService = PSS_MESSAGE;
				CallService(MS_PROTO_CHAINSEND, wParam, lParam);

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

				waitForExchange(ptr, 3); // ������� ������������
				return 1;
			}
			else {
				ptrA keyToSend(InitKeyA(ptr, CPP_FEATURES_NEWPG | KEY_B_SIG)); // calculate NEW public and private key
				Sent_NetLog("onRecvMsg: Sending KEYB %s", keyToSend);

				pccsd->wParam |= PREF_METANODB;
				pccsd->lParam = (LPARAM)keyToSend;
				pccsd->szProtoService = PSS_MESSAGE;
				CallService(MS_PROTO_CHAINSEND, wParam, lParam);
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
				waitForExchange(ptr, 3); // ������� ������������
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

		waitForExchange(ptr, 2); // ������ ����� ����������� ����������
		return 1;
	}

	// receive message
	if (cpp_keyx(ptr->cntx) && (ssig == SiG_ENON || ssig == SiG_ENOF)) {
		Sent_NetLog("onRecvMsg: message received");
		showPopupRM(ptr->hContact);
	}

	Sent_NetLog("onRecvMsg: exit");

	pccsd->wParam |= PREF_SIMNOMETA;
	int ret = CallService(MS_PROTO_CHAINRECV, wParam, lParam);
	SAFE_FREE(szPlainMsg);
	return ret;
}

// SendMsg handler
INT_PTR __cdecl onSendMsg(WPARAM wParam, LPARAM lParam)
{
	CCSDATA *pccsd = (CCSDATA *)lParam;
	pUinKey ptr = getUinKey(pccsd->hContact);
	int ssig = getSecureSig((LPCSTR)pccsd->lParam);
	int stat = getContactStatus(pccsd->hContact);

	Sent_NetLog("onSend: %s", (LPSTR)pccsd->lParam);

	// pass unhandled messages
	if (!ptr || ssig == SiG_GAME || ssig == SiG_PGPM || ssig == SiG_SECU || ssig == SiG_SECP ||
		isChatRoom(pccsd->hContact) || stat == -1 ||
		(ssig == SiG_NONE && ptr->sendQueue) || (ssig == SiG_NONE && ptr->status == STATUS_DISABLED)) {
			Sent_NetLog("onSendMsg: pass unhandled");
			return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
	}

	//
	// PGP/GPG mode
	// 
	if (ptr->mode == MODE_PGP || ptr->mode == MODE_GPG) {
		Sent_NetLog("onSendMsg: PGP|GPG mode");

		// ���� ����� ����������� - �������
		if (isContactPGP(ptr->hContact) || isContactGPG(ptr->hContact)) {
			if (!ptr->cntx) {
				ptr->cntx = cpp_create_context((isContactGPG(ptr->hContact) ? CPP_MODE_GPG : CPP_MODE_PGP) | ((db_get_b(ptr->hContact, MODULENAME, "gpgANSI", 0)) ? CPP_MODE_GPG_ANSI : 0));
				ptr->keyLoaded = 0;
			}
			if (!ptr->keyLoaded && bPGPloaded) ptr->keyLoaded = LoadKeyPGP(ptr);
			if (!ptr->keyLoaded && bGPGloaded) ptr->keyLoaded = LoadKeyGPG(ptr);
			if (!ptr->keyLoaded) return returnError(pccsd->hContact, Translate(sim108));

			LPSTR szNewMsg = NULL;
			ptrA szUtfMsg(miranda_to_utf8((LPCSTR)pccsd->lParam, pccsd->wParam));
			if (ptr->keyLoaded == 1) // PGP
				szNewMsg = pgp_encode(ptr->cntx, szUtfMsg);
			else if (ptr->keyLoaded == 2) // GPG
				szNewMsg = gpg_encode(ptr->cntx, szUtfMsg);

			if (!szNewMsg)
				return returnError(pccsd->hContact, Translate(sim109));

			// ���������� ������������� ���������
			splitMessageSend(ptr, szNewMsg);

			showPopupSM(ptr->hContact);

			return returnNoError(pccsd->hContact);
		}

		// ���������� ���������������
		return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
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
				if (exp->rsa_get_state(ptr->cntx) != 0)
					resetRSAcntx(ptr);
			}
			else createRSAcntx(ptr);

			if (!bSOM || (!isClientMiranda(ptr, 1) && !isSecureIM(ptr, 1)) || !loadRSAkey(ptr)) {
				if (ssig == SiG_NONE)
					// ������ ���� ��������������� � �������
					return CallService(MS_PROTO_CHAINSEND, wParam, lParam);

				// ������ �� ���� ������ - ��� ��������� ���������
				return returnNoError(pccsd->hContact);
			}

			// ���� ����������� � �������
			exp->rsa_send(ptr->cntx, ptrA(miranda_to_utf8((LPCSTR)pccsd->lParam, pccsd->wParam)));
			showPopupSM(ptr->hContact);
			return returnNoError(pccsd->hContact);
		}

		// SecureIM connection with this contact is disabled
		if (stid == STATUS_DISABLED) {
			if (ptr->cntx) {
				exp->rsa_disabled(ptr->cntx);
				deleteRSAcntx(ptr);
			}

			if (ssig == SiG_NONE) // ������ ���� ���������������
				return CallService(MS_PROTO_CHAINSEND, wParam, lParam);

			// ������ �� ���� ������ - ��� ��������� ���������
			return returnNoError(pccsd->hContact);
		}

		// ��������� ����������
		if (ssig == SiG_DEIN) {
			if (ptr->cntx) {
				exp->rsa_disconnect(ptr->cntx);
				deleteRSAcntx(ptr);
			}
			ShowStatusIconNotify(ptr->hContact);
			waitForExchange(ptr, 3); // ������ ������������
			return returnNoError(pccsd->hContact);
		}

		// ���������� �����������
		if (ptr->cntx && exp->rsa_get_state(ptr->cntx) == 7) {
			exp->rsa_send(ptr->cntx, ptrA(miranda_to_utf8((LPCSTR)pccsd->lParam, pccsd->wParam)));
			ShowStatusIconNotify(ptr->hContact);
			showPopupSM(ptr->hContact);
			return returnNoError(pccsd->hContact);
		}

		// ������ ��������� (��� �����, ��� ��������� � �������� AIP & NOL)
		if (ssig == SiG_NONE && isSecureIM(ptr->hContact)) {
			// ������� ��� � �������
			addMsg2Queue(ptr, pccsd->wParam, (LPSTR)pccsd->lParam);
			// ��������� ������� ��������� ����������
			ssig = SiG_INIT;
			// ��������� ���� �������� � �������
			waitForExchange(ptr);
		}

		// ���������� ����������
		if (ssig == SiG_INIT) {
			createRSAcntx(ptr);
			loadRSAkey(ptr);
			exp->rsa_connect(ptr->cntx);
			showPopupKS(pccsd->hContact);
			ShowStatusIconNotify(pccsd->hContact);
			return returnNoError(pccsd->hContact);
		}

		// ������ ���� ��������������� (�� ���� ���� ����� ����� ��������)
		return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
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
			return returnError(pccsd->hContact, Translate(sim105));

		if (ptr->cntx) { // if secure context exists
			cpp_delete_context(ptr->cntx); ptr->cntx = 0;

			CCSDATA ccsd;
			memcpy(&ccsd, (HLOCAL)lParam, sizeof(CCSDATA));

			pccsd->wParam |= PREF_METANODB;
			ccsd.lParam = (LPARAM)SIG_DEIN;
			ccsd.szProtoService = PSS_MESSAGE;
			CallService(MS_PROTO_CHAINSEND, wParam, (LPARAM)&ccsd);

			showPopupDC(pccsd->hContact);
			ShowStatusIconNotify(pccsd->hContact);
		}
		return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
	}

	// contact is offline
	if (stat == ID_STATUS_OFFLINE) {
		Sent_NetLog("onSendMsg: message for offline");

		if (ssig == SiG_INIT && cpp_keyx(ptr->cntx)) // reinit key exchange
			cpp_reset_context(ptr->cntx);

		if (!bSOM) {
			if (ssig != SiG_NONE)
				return returnNoError(pccsd->hContact);

			// exit and send unencrypted message
			return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
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
					return returnNoError(pccsd->hContact);

				// exit and send unencrypted message
				return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
			}
		}
		else {
			if (ssig != SiG_NONE)
				return returnNoError(pccsd->hContact);

			// exit and send unencrypted message
			return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
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

			pccsd->wParam |= PREF_METANODB;
			CallService(MS_PROTO_CHAINSEND, wParam, lParam);

			showPopupDC(pccsd->hContact);
			ShowStatusIconNotify(pccsd->hContact);
		}
		return returnNoError(pccsd->hContact);
	}

	if (cpp_keya(ptr->cntx) && cpp_keyb(ptr->cntx) && !cpp_keyx(ptr->cntx))
		CalculateKeyX(ptr, ptr->hContact);

	ShowStatusIconNotify(pccsd->hContact);

	// if cryptokey exist
	if (cpp_keyx(ptr->cntx)) {
		Sent_NetLog("onSendMsg: cryptokey exist");

		ptrA szNewMsg(encodeMsg(ptr, (LPARAM)pccsd));
		Sent_NetLog("onSend: encrypted msg '%s'", szNewMsg);

		pccsd->wParam |= PREF_METANODB;
		pccsd->lParam = (LPARAM)(char*)szNewMsg;
		pccsd->szProtoService = PSS_MESSAGE;
		int ret = CallService(MS_PROTO_CHAINSEND, wParam, lParam);

		showPopupSM(ptr->hContact);
		return ret;
	}

	Sent_NetLog("onSendMsg: cryptokey not exist, try establishe connection");

	// send KeyA if init || always_try || waitkey || always_if_possible
	if (ssig == SiG_INIT || (stid == STATUS_ALWAYSTRY && isClientMiranda(ptr->hContact)) || isSecureIM(ptr->hContact) || ptr->waitForExchange) {
		if (ssig == SiG_NONE)
			addMsg2Queue(ptr, pccsd->wParam, (LPSTR)pccsd->lParam);

		if (!ptr->waitForExchange) {
			// init || always_try || always_if_possible
			ptrA keyToSend(InitKeyA(ptr, 0));	// calculate public and private key & fill KeyA
			Sent_NetLog("Sending KEY3: %s", keyToSend);

			pccsd->wParam |= PREF_METANODB;
			pccsd->lParam = (LPARAM)keyToSend;
			pccsd->szProtoService = PSS_MESSAGE;
			CallService(MS_PROTO_CHAINSEND, wParam, lParam);

			showPopupKS(pccsd->hContact);
			ShowStatusIconNotify(pccsd->hContact);

			waitForExchange(ptr); // ��������� ��������
		}
		return returnNoError(pccsd->hContact);
	}

	Sent_NetLog("onSendMsg: pass unchanged to chain");
	return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
}
