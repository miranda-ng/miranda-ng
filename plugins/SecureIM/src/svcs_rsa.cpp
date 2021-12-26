#include "commonheaders.h"

pRSA_EXPORT mir_exp = nullptr;
RSA_IMPORT imp =
{
	rsa_inject,
	rsa_check_pub,
	rsa_notify
};

BOOL rsa_4096 = 0;

int __cdecl rsa_inject(HANDLE context, LPCSTR msg)
{
	pUinKey ptr = getUinCtx(context); if (!ptr) return 0;

	Sent_NetLog("rsa_inject: '%s'", msg);

	int len = (int)mir_strlen(msg) + 1;
	LPSTR buf = (LPSTR)mir_alloc(LEN_SECU + len);
	memcpy(buf, SIG_SECU, LEN_SECU);
	memcpy(buf + LEN_SECU, msg, len);
	// отправляем сообщение
	splitMessageSend(ptr, buf);
	mir_free(buf);
	return 1;
}

#define MSGSIZE 1024

int __cdecl rsa_check_pub(HANDLE context, uint8_t *pub, int pubLen, PBYTE sig, int sigLen)
{
	int v = 0, k = 0;
	pUinKey ptr = getUinCtx(context); if (!ptr) return 0;
	LPSTR cnm = (LPSTR)mir_alloc(NAMSIZE); strncpy_s(cnm, NAMSIZE, _T2A(Clist_GetContactDisplayName(ptr->hContact)), _TRUNCATE);
	LPSTR uin = (LPSTR)mir_alloc(KEYSIZE); getContactUinA(ptr->hContact, uin);
	LPSTR msg = (LPSTR)mir_alloc(MSGSIZE);
	LPSTR sha = mir_strdup(to_hex(sig, sigLen));
	LPSTR sha_old = nullptr;

	Sent_NetLog("rsa_check_pub: %s %s %s", cnm, uin, sha);

	DBVARIANT dbv;
	dbv.type = DBVT_BLOB;
	if (db_get(ptr->hContact, MODULENAME, "rsa_pub", &dbv) == 0) {
		k = 1;
		uint8_t *buf = (uint8_t*)alloca(sigLen); int len;
		mir_exp->rsa_get_hash((uint8_t*)dbv.pbVal, dbv.cpbVal, (uint8_t*)buf, &len);
		sha_old = mir_strdup(to_hex(buf, len));
		db_free(&dbv);
	}
	if (bAAK) {
		if (k)
			mir_snprintf(msg, MSGSIZE, Translate("SecureIM auto accepted NEW RSA Public key from: %s uin: %s New SHA-1: %s Old SHA-1: %s"), cnm, uin, sha, sha_old);
		else
			mir_snprintf(msg, MSGSIZE, Translate("SecureIM auto accepted RSA Public key from: %s uin: %s SHA-1: %s"), cnm, uin, sha);
		showPopupKRmsg(ptr->hContact, msg);
		HistoryLog(ptr->hContact, msg);
		v = 1;
#if defined(_DEBUG) || defined(NETLIB_LOG)
		Sent_NetLog("rsa_check_pub: auto accepted");
#endif
	}
	else {
		if (k)
			mir_snprintf(msg, MSGSIZE, Translate("SecureIM received NEW RSA Public Key from \"%s\"\n\nNew SHA-1: %s\n\nOld SHA-1: %s\n\nDo you Replace this Key?"), cnm, sha, sha_old);
		else
			mir_snprintf(msg, MSGSIZE, Translate("SecureIM received RSA Public Key from \"%s\"\n\nSHA-1: %s\n\nDo you Accept this Key?"), cnm, sha);
		v = (msgbox(nullptr, msg, MODULENAME, MB_YESNO | MB_ICONQUESTION) == IDYES);
#if defined(_DEBUG) || defined(NETLIB_LOG)
		Sent_NetLog("rsa_check_pub: manual accepted %d", v);
#endif
	}
	if (v) {
		db_set_blob(ptr->hContact, MODULENAME, "rsa_pub", pub, pubLen);
		ptr->keyLoaded = true;
	}
	mir_free(cnm);
	mir_free(uin);
	mir_free(msg);
	mir_free(sha);
	SAFE_FREE(sha_old);
	return v;
}

void __cdecl rsa_notify(HANDLE context, int state)
{
	pUinKey ptr = getUinCtx(context); if (!ptr) return;
	LPCSTR msg = nullptr;

	Sent_NetLog("rsa_notify: 0x%x", state);

	switch (state) {
	case 1:
		showPopupEC(ptr->hContact);
		ShowStatusIconNotify(ptr->hContact);
		waitForExchange(ptr, 2); // досылаем сообщения из очереди
		return;

	case -1: // сессия разорвана по ошибке, неверный тип сообщения
		msg = LPGEN("Session closed by receiving incorrect message type"); break;
	case -2: // сессия разорвана по ошибке другой стороной
		msg = LPGEN("Session closed by other side on error"); break;
	case -5: // ошибка декодирования AES сообщения
		msg = LPGEN("Error while decoding AES message"); break;
	case -6: // ошибка декодирования RSA сообщения
		msg = LPGEN("Error while decoding RSA message"); break;
	case -7: // таймаут установки соединения (10 секунд)
		msg = LPGEN("Session closed on timeout"); break;
	case -8: // сессия разорвана по причине "disabled"
		msg = LPGEN("Session closed by other side when status \"disabled\""); break;
	case -0x10: // сессия разорвана по ошибке
	case -0x21:
	case -0x22:
	case -0x23:
	case -0x24:
	case -0x32:
	case -0x33:
	case -0x34:
	case -0x40:
	case -0x50:
	case -0x60:
	{
		char buf[1024];
		mir_snprintf(buf, LPGEN("Session closed on error: %02x"), -state);
		showPopupDCmsg(ptr->hContact, buf);
		ShowStatusIconNotify(ptr->hContact);
		if (ptr->cntx) deleteRSAcntx(ptr);
		waitForExchange(ptr, 3); // досылаем нешифровано
	}
		return;

	case -3: // соединение разорвано вручную
	case -4: // соединение разорвано вручную другой стороной
		showPopupDC(ptr->hContact);
		ShowStatusIconNotify(ptr->hContact);
		if (ptr->cntx) deleteRSAcntx(ptr);
		waitForExchange(ptr, 3); // досылаем нешифровано
		return;

	default:
		return;
	}
	showPopupDCmsg(ptr->hContact, msg);
	ShowStatusIconNotify(ptr->hContact);
	if (ptr->cntx) deleteRSAcntx(ptr);
	waitForExchange(ptr, 3); // досылаем нешифровано
}

void sttGenerateRSA(LPVOID)
{
	char priv_key[4096]; int priv_len;
	char pub_key[4096]; int pub_len;

	mir_exp->rsa_gen_keypair(CPP_MODE_RSA_4096);
	mir_exp->rsa_get_keypair(CPP_MODE_RSA_4096, (uint8_t*)&priv_key, &priv_len, (uint8_t*)&pub_key, &pub_len);

	db_set_blob(0, MODULENAME, "rsa_priv", priv_key, priv_len);
	db_set_blob(0, MODULENAME, "rsa_pub", pub_key, pub_len);
	rsa_4096 = 1;
}

// загружает паблик-ключ в RSA контекст
uint8_t loadRSAkey(pUinKey ptr)
{
	if (!ptr->keyLoaded) {
		DBVARIANT dbv;
		dbv.type = DBVT_BLOB;
		if (db_get(ptr->hContact, MODULENAME, "rsa_pub", &dbv) == 0) {
			ptr->keyLoaded = mir_exp->rsa_set_pubkey(ptr->cntx, dbv.pbVal, dbv.cpbVal);
			Sent_NetLog("loadRSAkey %d", ptr->keyLoaded);
			db_free(&dbv);
		}
	}
	return ptr->keyLoaded;
}

// создает RSA контекст
void createRSAcntx(pUinKey ptr)
{
	if (!ptr->cntx) {
		ptr->cntx = cpp_create_context(CPP_MODE_RSA);
		ptr->keyLoaded = 0;
	}
}

// пересоздает RSA контекст
void resetRSAcntx(pUinKey ptr)
{
	if (ptr->cntx) {
		cpp_delete_context(ptr->cntx);
		ptr->cntx = cpp_create_context(CPP_MODE_RSA);
		ptr->keyLoaded = 0;
	}
}

// удаляет RSA контекст
void deleteRSAcntx(pUinKey ptr)
{
	cpp_delete_context(ptr->cntx);
	ptr->cntx = nullptr;
	ptr->keyLoaded = 0;
}
