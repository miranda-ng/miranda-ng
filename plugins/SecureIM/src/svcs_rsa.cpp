#include "commonheaders.h"

pRSA_EXPORT exp = NULL;
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
	// ���������� ���������
	splitMessageSend(ptr, buf);
	mir_free(buf);
	return 1;
}

#define MSGSIZE 1024

int __cdecl rsa_check_pub(HANDLE context, PBYTE pub, int pubLen, PBYTE sig, int sigLen)
{
	int v = 0, k = 0;
	pUinKey ptr = getUinCtx(context); if (!ptr) return 0;
	LPSTR cnm = (LPSTR)mir_alloc(NAMSIZE); getContactNameA(ptr->hContact, cnm);
	LPSTR uin = (LPSTR)mir_alloc(KEYSIZE); getContactUinA(ptr->hContact, uin);
	LPSTR msg = (LPSTR)mir_alloc(MSGSIZE);
	LPSTR sha = mir_strdup(to_hex(sig, sigLen));
	LPSTR sha_old = NULL;

	Sent_NetLog("rsa_check_pub: %s %s %s", cnm, uin, sha);

	DBVARIANT dbv;
	dbv.type = DBVT_BLOB;
	if (db_get(ptr->hContact, MODULENAME, "rsa_pub", &dbv) == 0) {
		k = 1;
		PBYTE buf = (PBYTE)alloca(sigLen); int len;
		exp->rsa_get_hash((PBYTE)dbv.pbVal, dbv.cpbVal, (PBYTE)buf, &len);
		sha_old = mir_strdup(to_hex(buf, len));
		db_free(&dbv);
	}
	if (bAAK) {
		if (k)	mir_snprintf(msg, MSGSIZE, Translate(sim523), cnm, uin, sha, sha_old);
		else	mir_snprintf(msg, MSGSIZE, Translate(sim521), cnm, uin, sha);
		showPopupKRmsg(ptr->hContact, msg);
		HistoryLog(ptr->hContact, msg);
		v = 1;
#if defined(_DEBUG) || defined(NETLIB_LOG)
		Sent_NetLog("rsa_check_pub: auto accepted");
#endif
	}
	else {
		if (k) mir_snprintf(msg, MSGSIZE, Translate(sim522), cnm, sha, sha_old);
		else	mir_snprintf(msg, MSGSIZE, Translate(sim520), cnm, sha);
		v = (msgbox(0, msg, MODULENAME, MB_YESNO | MB_ICONQUESTION) == IDYES);
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
	LPCSTR msg = NULL;

	Sent_NetLog("rsa_notify: 0x%x", state);

	switch (state) {
	case 1:
		showPopupEC(ptr->hContact);
		ShowStatusIconNotify(ptr->hContact);
		waitForExchange(ptr, 2); // �������� ��������� �� �������
		return;

	case -1: // ������ ��������� �� ������, �������� ��� ���������
		msg = sim501; break;
	case -2: // ������ ��������� �� ������ ������ ��������
		msg = sim502; break;
	case -5: // ������ ������������� AES ���������
		msg = sim505; break;
	case -6: // ������ ������������� RSA ���������
		msg = sim506; break;
	case -7: // ������� ��������� ���������� (10 ������)
		msg = sim507; break;
	case -8: // ������ ��������� �� ������� "disabled"
		msg = sim508; break;
	case -0x10: // ������ ��������� �� ������
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
		mir_snprintf(buf, sim510, -state);
		showPopupDCmsg(ptr->hContact, buf);
		ShowStatusIconNotify(ptr->hContact);
		if (ptr->cntx) deleteRSAcntx(ptr);
		waitForExchange(ptr, 3); // �������� �����������
	}
		return;

	case -3: // ���������� ��������� �������
	case -4: // ���������� ��������� ������� ������ ��������
		showPopupDC(ptr->hContact);
		ShowStatusIconNotify(ptr->hContact);
		if (ptr->cntx) deleteRSAcntx(ptr);
		waitForExchange(ptr, 3); // �������� �����������
		return;

	default:
		return;
	}
	showPopupDCmsg(ptr->hContact, msg);
	ShowStatusIconNotify(ptr->hContact);
	if (ptr->cntx) deleteRSAcntx(ptr);
	waitForExchange(ptr, 3); // �������� �����������
}

void sttGenerateRSA(LPVOID)
{
	char priv_key[4096]; int priv_len;
	char pub_key[4096]; int pub_len;

	exp->rsa_gen_keypair(CPP_MODE_RSA_4096);
	exp->rsa_get_keypair(CPP_MODE_RSA_4096, (PBYTE)&priv_key, &priv_len, (PBYTE)&pub_key, &pub_len);

	db_set_blob(NULL, MODULENAME, "rsa_priv", priv_key, priv_len);
	db_set_blob(NULL, MODULENAME, "rsa_pub", pub_key, pub_len);
	rsa_4096 = 1;
}

// ��������� ������-���� � RSA ��������
BYTE loadRSAkey(pUinKey ptr)
{
	if (!ptr->keyLoaded) {
		DBVARIANT dbv;
		dbv.type = DBVT_BLOB;
		if (db_get(ptr->hContact, MODULENAME, "rsa_pub", &dbv) == 0) {
			ptr->keyLoaded = exp->rsa_set_pubkey(ptr->cntx, dbv.pbVal, dbv.cpbVal);
			Sent_NetLog("loadRSAkey %d", ptr->keyLoaded);
			db_free(&dbv);
		}
	}
	return ptr->keyLoaded;
}

// ������� RSA ��������
void createRSAcntx(pUinKey ptr)
{
	if (!ptr->cntx) {
		ptr->cntx = cpp_create_context(CPP_MODE_RSA);
		ptr->keyLoaded = 0;
	}
}

// ����������� RSA ��������
void resetRSAcntx(pUinKey ptr)
{
	if (ptr->cntx) {
		cpp_delete_context(ptr->cntx);
		ptr->cntx = cpp_create_context(CPP_MODE_RSA);
		ptr->keyLoaded = 0;
	}
}

// ������� RSA ��������
void deleteRSAcntx(pUinKey ptr)
{
	cpp_delete_context(ptr->cntx);
	ptr->cntx = 0;
	ptr->keyLoaded = 0;
}
