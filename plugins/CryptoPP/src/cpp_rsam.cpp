#include "commonheaders.h"

///////////////////////////////////////////////////////////////////////////

#define RAND_SIZE (256/8)

RSA_EXPORT exports = {
	rsa_gen_keypair,
	rsa_get_keypair,
	rsa_get_keyhash,
	rsa_set_keypair,
	rsa_get_pubkey,
	rsa_set_pubkey,
	rsa_set_timeout,
	rsa_get_state,
	rsa_get_hash,
	rsa_connect,
	rsa_disconnect,
	rsa_disabled,
	rsa_recv,
	rsa_send,
	rsa_encrypt_file,
	rsa_decrypt_file,
	utf8encode,
	utf8decode,
	is_7bit_string,
	is_utf8_string,
	rsa_export_keypair,
	rsa_import_keypair,
	rsa_export_pubkey,
	rsa_import_pubkey
};

pRSA_IMPORT imp;

string null;
int timeout = 10;

const string priv_beg = "-----BEGIN SECUREIM PRIVATE KEY BLOCK-----";
const string priv_end = "-----END SECUREIM PRIVATE KEY BLOCK-----";

const string pub_beg = "-----BEGIN SECUREIM PUBLIC KEY BLOCK-----";
const string pub_end = "-----END SECUREIM PUBLIC KEY BLOCK-----";

const string crlf = "\x0D\x0A";

///////////////////////////////////////////////////////////////////////////

int __cdecl rsa_init(pRSA_EXPORT* e, pRSA_IMPORT i)
{
#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("rsa_init");
#endif
	*e = &exports;
	imp = i;
	if (!hRSA4096) {
		// create context for private rsa keys
		hRSA4096 = (HANDLE)cpp_create_context(MODE_RSA_4096 | MODE_PRIV_KEY);
		pCNTX tmp = (pCNTX)hRSA4096;
		pRSAPRIV p = new RSAPRIV;
		tmp->pdata = (PBYTE)p;
	}
	return 1;
}

int __cdecl rsa_done(void)
{
#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("rsa_done");
#endif
	if (hRSA4096) {
		pCNTX tmp = (pCNTX)hRSA4096;
		pRSAPRIV p = (pRSAPRIV)tmp->pdata;
		delete p;
		tmp->pdata = 0;
		cpp_delete_context(hRSA4096);
		hRSA4096 = NULL;
	}

	return 1;
}


///////////////////////////////////////////////////////////////////////////


pRSAPRIV rsa_gen_keys(HANDLE context)
{
	if (context != hRSA4096) return 0;

#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("rsa_gen_keys: %d", context);
#endif
	pCNTX ptr = get_context_on_id(context); if (!ptr) return 0;
	pRSAPRIV r = (pRSAPRIV)ptr->pdata;

	string priv, pub;
	GenerateRSAKey(4096, priv, pub);

	StringSource privsrc(priv, true, NULL);
	RSAES_PKCS1v15_Decryptor Decryptor(privsrc);

	priv = tlv(1, IntegerToBinary(Decryptor.GetTrapdoorFunction().GetModulus())) +
		tlv(2, IntegerToBinary(Decryptor.GetTrapdoorFunction().GetPublicExponent())) +
		tlv(3, IntegerToBinary(Decryptor.GetTrapdoorFunction().GetPrivateExponent())) +
		tlv(4, IntegerToBinary(Decryptor.GetTrapdoorFunction().GetPrime1())) +
		tlv(5, IntegerToBinary(Decryptor.GetTrapdoorFunction().GetPrime2())) +
		tlv(6, IntegerToBinary(Decryptor.GetTrapdoorFunction().GetModPrime1PrivateExponent())) +
		tlv(7, IntegerToBinary(Decryptor.GetTrapdoorFunction().GetModPrime2PrivateExponent())) +
		tlv(8, IntegerToBinary(Decryptor.GetTrapdoorFunction().GetMultiplicativeInverseOfPrime2ModPrime1()));

	init_priv(r, priv);

	return r;
}

pRSAPRIV rsa_get_priv(pCNTX ptr)
{
	pCNTX p = get_context_on_id(hRSA4096); if (!p) return 0;
	pRSAPRIV r = (pRSAPRIV)p->pdata;
	return r;
}

int __cdecl rsa_gen_keypair(short mode)
{
#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("rsa_gen_keypair: %d", mode);
#endif
	if (mode&MODE_RSA_4096) rsa_gen_keys(hRSA4096); // 4096

	return 1;
}

int __cdecl rsa_get_keypair(short mode, PBYTE privKey, int* privKeyLen, PBYTE pubKey, int* pubKeyLen)
{
#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("rsa_get_keypair: %d", mode);
#endif
	pCNTX ptr = get_context_on_id(hRSA4096); if (!ptr) return 0;
	pRSAPRIV r = (pRSAPRIV)ptr->pdata;

	*privKeyLen = (int)r->priv_k.length(); if (privKey) r->priv_k.copy((char*)privKey, *privKeyLen);
	*pubKeyLen = (int)r->pub_k.length(); if (pubKey) r->pub_k.copy((char*)pubKey, *pubKeyLen);
	return 1;
}

int __cdecl rsa_get_keyhash(short mode, PBYTE privKey, int* privKeyLen, PBYTE pubKey, int* pubKeyLen)
{
#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("rsa_get_keyhash: %d", mode);
#endif
	pCNTX ptr = get_context_on_id(hRSA4096); if (!ptr) return 0;
	pRSAPRIV r = (pRSAPRIV)ptr->pdata;

	if (privKey) { *privKeyLen = (int)r->priv_s.length(); r->priv_s.copy((char*)privKey, *privKeyLen); }
	if (pubKey) { *pubKeyLen = (int)r->pub_s.length(); r->pub_s.copy((char*)pubKey, *pubKeyLen); }

	return 1;
}

int __cdecl rsa_set_keypair(short mode, PBYTE privKey, int privKeyLen)
{
#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("rsa_set_keypair: %s", privKey);
#endif
	pCNTX ptr = get_context_on_id(hRSA4096); if (!ptr) return 0;
	pRSAPRIV r = (pRSAPRIV)ptr->pdata;

	if (privKey && privKeyLen) {
		string priv;
		priv.assign((char*)privKey, privKeyLen);

		if (mode & MODE_RSA_BER) {
			// old BER format
			StringStore s(priv);
			r->priv.BERDecode(s);

			RSAES_PKCS1v15_Decryptor Decryptor(r->priv);

			priv = tlv(1, IntegerToBinary(Decryptor.GetTrapdoorFunction().GetModulus())) +
				tlv(2, IntegerToBinary(Decryptor.GetTrapdoorFunction().GetPublicExponent())) +
				tlv(3, IntegerToBinary(Decryptor.GetTrapdoorFunction().GetPrivateExponent())) +
				tlv(4, IntegerToBinary(Decryptor.GetTrapdoorFunction().GetPrime1())) +
				tlv(5, IntegerToBinary(Decryptor.GetTrapdoorFunction().GetPrime2())) +
				tlv(6, IntegerToBinary(Decryptor.GetTrapdoorFunction().GetModPrime1PrivateExponent())) +
				tlv(7, IntegerToBinary(Decryptor.GetTrapdoorFunction().GetModPrime2PrivateExponent())) +
				tlv(8, IntegerToBinary(Decryptor.GetTrapdoorFunction().GetMultiplicativeInverseOfPrime2ModPrime1()));

		}
		init_priv(r, priv);
	}

	return 1;
}

int __cdecl rsa_get_pubkey(HANDLE context, PBYTE pubKey, int* pubKeyLen)
{
#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("rsa_get_pubkey: %s", pubKey);
#endif
	pCNTX ptr = get_context_on_id(context);	if (!ptr) return 0;
	pRSADATA p = (pRSADATA)cpp_alloc_pdata(ptr);

	*pubKeyLen = (int)p->pub_k.length(); if (pubKey) p->pub_k.copy((char*)pubKey, *pubKeyLen);

	return 1;
}

int __cdecl rsa_set_pubkey(HANDLE context, PBYTE pubKey, int pubKeyLen)
{
#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("rsa_set_pubkey: %s", pubKey);
#endif
	pCNTX ptr = get_context_on_id(context);	if (!ptr) return 0;
	pRSADATA p = (pRSADATA)cpp_alloc_pdata(ptr);

	if (pubKey && pubKeyLen) {
		string pub;
		pub.assign((char*)pubKey, pubKeyLen);
		init_pub(p, pub);
	}

	return 1;
}

void __cdecl rsa_set_timeout(int t)
{
#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("rsa_set_timeout: %d", t);
#endif
	timeout = t;
}

int __cdecl rsa_get_state(HANDLE context)
{
	pCNTX ptr = get_context_on_id(context);	if (!ptr) return 0;
	pRSADATA p = (pRSADATA)cpp_alloc_pdata(ptr);
	return p->state;
}

int __cdecl rsa_get_hash(PBYTE pubKey, int pubKeyLen, PBYTE pubHash, int* pubHashLen)
{
#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("rsa_get_hash: %d", pubKeyLen);
#endif
	string sig;
	sig = ::hash(pubKey, pubKeyLen);

	*pubHashLen = (int)sig.length();
	if (pubHash)
		sig.copy((char*)pubHash, *pubHashLen);

	return 1;
}

int __cdecl rsa_connect(HANDLE context)
{
#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("rsa_connect: %08x", context);
#endif
	pCNTX ptr = get_context_on_id(context); if (!ptr) return 0;
	pRSADATA p = (pRSADATA)cpp_alloc_pdata(ptr); if (p->state) return p->state;
	pRSAPRIV r = rsa_get_priv(ptr);

	if (ptr->mode&MODE_RSA_ONLY) {
		inject_msg(context, 0x0D, tlv(0, 0) + tlv(1, r->pub_k) + tlv(2, p->pub_s));
		p->state = 0x0D;
	}
	else {
		inject_msg(context, 0x10, tlv(0, 0) + tlv(1, r->pub_s) + tlv(2, p->pub_s));
		p->state = 2;
	}
	p->time = gettime() + timeout;

	return p->state;
}

int __cdecl rsa_disconnect(HANDLE context)
{
#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("rsa_disconnect: %08x", context);
#endif
	pCNTX ptr = get_context_on_id(context); if (!ptr) return 0;
	rsa_free(ptr); // удалим трэд и очередь сообщений

	pRSADATA p = (pRSADATA)cpp_alloc_pdata(ptr);
	if (!p->state) return 1;

	PBYTE buffer = (PBYTE)alloca(RAND_SIZE);
	GlobalRNG().GenerateBlock(buffer, RAND_SIZE);
	inject_msg(context, 0xF0, encode_msg(0, p, ::hash(buffer, RAND_SIZE)));

	p->state = 0;
	imp->rsa_notify(context, -3); // соединение разорвано вручную

	return 1;
}

int __cdecl rsa_disabled(HANDLE context)
{
#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("rsa_disabled: %08x", context);
#endif
	pCNTX ptr = get_context_on_id(context); if (!ptr) return 0;
	rsa_free(ptr); // удалим трэд и очередь сообщений

	pRSADATA p = (pRSADATA)cpp_alloc_pdata(ptr);
	p->state = 0;
	inject_msg(context, 0xFF, null);
	//	imp->rsa_notify(-context,-8); // соединение разорвано по причине "disabled"
	return 1;
}

LPSTR __cdecl rsa_recv(HANDLE context, LPCSTR msg)
{
#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("rsa_recv: %s", msg);
#endif
	pCNTX ptr = get_context_on_id(context);	if (!ptr) return 0;
	pRSADATA p = (pRSADATA)cpp_alloc_pdata(ptr);
	pRSAPRIV r = rsa_get_priv(ptr);

	rtrim(msg);

	string buf = base64decode(msg);
	if (!buf.length()) return 0;

	string data; int type;
	un_tlv(buf, type, data);
	if (type == -1) return 0;

#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("rsa_recv: %02x %d", type, p->state);
#endif
	if (type > 0x10 && type < 0xE0)  // проверим тип сообщения (когда соединение еще не установлено)
		if (p->state == 0 || p->state != (type >> 4)) { // неверное состояние
			// шлем перерывание сессии
			p->state = 0; p->time = 0;
			rsa_free(ptr); // удалим трэд и очередь сообщений
			null_msg(context, 0x00, -1); // сессия разорвана по ошибке, неверный тип сообщения
			return 0;
		}

	switch (type) {

	case 0x00: // прерывание сессии по ошибке другой стороной
	{
		// если соединение установлено - ничего не делаем
		if (p->state == 0 || p->state == 7) return 0;
		// иначе сбрасываем текущее состояние
		p->state = 0; p->time = 0;
		imp->rsa_notify(context, -2); // сессия разорвана по ошибке другой стороной
	} break;

	// это все будем обрабатывать в отдельном потоке, чтобы избежать таймаутов
	case 0x10: // запрос на установку соединения
	case 0x22: // получили удаленный паблик, отправляем уже криптоключ
	case 0x23: // отправляем локальный паблик
	case 0x24: // получили удаленный паблик, отправим локальный паблик
	case 0x33: // получили удаленный паблик, отправляем криптоключ
	case 0x34:
	case 0x21: // получили криптоключ, отправляем криптотест
	case 0x32:
	case 0x40:
	case 0x0D: // запрос паблика
	case 0xD0: // ответ пабликом
		if (!p->event) {
			p->event = CreateEvent(NULL, FALSE, FALSE, NULL);
			unsigned int tID;
			p->thread = (HANDLE)_beginthreadex(NULL, 0, sttConnectThread, (PVOID)context, 0, &tID);
#if defined(_DEBUG) || defined(NETLIB_LOG)
			Sent_NetLog("rsa_recv: _beginthreadex(sttConnectThread)");
#endif
		}
		{
			mir_cslock lck(localQueueMutex);
			p->queue->push(tlv(type, data));
		}
		SetEvent(p->event); // сказали обрабатывать :)
		break;

	case 0x50: // получили криптотест, отправляем свой криптотест
		if (!decode_msg(p, data).length()) {
			p->state = 0; p->time = 0;
			null_msg(context, 0x00, -type); // сессия разорвана по ошибке
			return 0;
		}
		{
			PBYTE buffer = (PBYTE)alloca(RAND_SIZE);
			GlobalRNG().GenerateBlock(buffer, RAND_SIZE);
			inject_msg(context, 0x60, encode_msg(0, p, ::hash(buffer, RAND_SIZE)));
		}
		p->state = 7; p->time = 0;
		rsa_free_thread(p); // удалим трэд и очередь сообщений
		imp->rsa_notify(context, 1);	// заебися, криптосессия установлена
		break;

	case 0x60: // получили криптотест, сессия установлена
		if (!decode_msg(p, data).length()) {
			p->state = 0; p->time = 0;
			null_msg(context, 0x00, -type); // сессия разорвана по ошибке
			return 0;
		}
		p->state = 7; p->time = 0;
		rsa_free_thread(p); // удалим трэд и очередь сообщений
		imp->rsa_notify(context, 1);	// заебися, криптосессия установлена
		break;

	case 0x70: // получили AES сообщение, декодируем
	{
		string msg = decode_msg(p, data);
		if (msg.length())
			replaceStr(ptr->tmp, mir_strdup(msg.c_str()));
		else {
			imp->rsa_notify(context, -5); // ошибка декодирования AES сообщения
			replaceStr(ptr->tmp, NULL);
		}
	}
		return ptr->tmp;

	case 0xE0: // получили RSA сообщение, декодируем
	{
		string msg = decode_rsa(p, r, data);
		if (msg.length())
			replaceStr(ptr->tmp, mir_strdup(msg.c_str()));
		else {
			imp->rsa_notify(context, -6); // ошибка декодирования RSA сообщения
			replaceStr(ptr->tmp, NULL);
		}
	}
		return ptr->tmp;

	case 0xF0: // разрыв соединения вручную
	{
		if (p->state != 7) return 0;
		string msg = decode_msg(p, data);
		if (!msg.length()) return 0;
		p->state = 0;
		rsa_free(ptr); // удалим трэд и очередь сообщений
		imp->rsa_notify(context, -4); // соединение разорвано вручную другой стороной
	}
		break;

	case 0xFF: // разрыв соединения по причине "disabled"
		p->state = 0;
		rsa_free(ptr); // удалим трэд и очередь сообщений
		imp->rsa_notify(context, -8); // соединение разорвано по причине "disabled"
		break;
	}

	if (p->state != 0 && p->state != 7)
		p->time = gettime() + timeout;

	return 0;
}

int __cdecl rsa_send(HANDLE context, LPCSTR msg)
{
	pCNTX ptr = get_context_on_id(context);	if (!ptr) return 0;
	pRSADATA p = (pRSADATA)cpp_alloc_pdata(ptr); if (p->state != 0 && p->state != 7) return 0;

	if (p->state == 7) // сессия установлена, шифруем AES и отправляем
		inject_msg(context, 0x70, encode_msg(1, p, string(msg)));
	else if (p->state == 0) { // сессия  установлена, отправляем RSA сообщение
		if (!p->pub_k.length()) return 0;
		// есть паблик ключ - отправим сообщение
		pRSAPRIV r = rsa_get_priv(ptr);
		inject_msg(context, 0xE0, encode_rsa(1, p, r, string(msg)));
	}

	return 1;
}

void inject_msg(HANDLE context, int type, const string& msg)
{
#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("inject_msg(%02x): %s", type, msg.c_str());
#endif
	string txt = base64encode(tlv(type, msg));
	imp->rsa_inject(context, (LPCSTR)txt.c_str());
}


string encode_msg(short z, pRSADATA p, string& msg)
{
#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("encode_msg: %s", msg.c_str());
#endif
	string zlib = (z) ? cpp_zlibc(msg) : msg;
	string sig = ::hash(zlib);

	string ciphered;
	try {
		CBC_Mode<AES>::Encryption enc((PBYTE)p->aes_k.data(), p->aes_k.length(), (PBYTE)p->aes_v.data());
		StreamTransformationFilter cbcEncryptor(enc, new StringSink(ciphered));
		cbcEncryptor.Put((PBYTE)zlib.data(), zlib.length());
		cbcEncryptor.MessageEnd();
	}
	catch (...) {
		;
	}
	return tlv(z, ciphered) + tlv(2, sig);
}

string decode_msg(pRSADATA p, string& msg)
{
#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("decode_msg: %s", msg.c_str());
#endif
	string ciphered, sig; int t1, t2;
	un_tlv(msg, t1, ciphered);
	un_tlv(msg, t2, sig);

	string unciphered, zlib;
	try {
		CBC_Mode<AES>::Decryption dec((PBYTE)p->aes_k.data(), p->aes_k.length(), (PBYTE)p->aes_v.data());
		StreamTransformationFilter cbcDecryptor(dec, new StringSink(zlib));
		cbcDecryptor.Put((PBYTE)ciphered.data(), ciphered.length());
		cbcDecryptor.MessageEnd();

		if (sig == ::hash(zlib))
			unciphered = (t1 == 1) ? cpp_zlibd(zlib) : zlib;
	}
	catch (...) {
		;
	}
	return unciphered;
}

string encode_rsa(short z, pRSADATA p, pRSAPRIV r, string& msg)
{
#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("encode_rsa: %s", msg.c_str());
#endif
	string zlib = (z) ? cpp_zlibc(msg) : msg;

	string enc = RSAEncryptString(p->pub, zlib);
	string sig = RSASignString(r->priv, zlib);

	return tlv(z, enc) + tlv(2, sig);
}

string decode_rsa(pRSADATA p, pRSAPRIV r, string& msg)
{
#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("decode_rsa: %s", msg.c_str());
#endif
	string ciphered, sig; int t1, t2;
	un_tlv(msg, t1, ciphered);
	un_tlv(msg, t2, sig);

	string unciphered, zlib;
	zlib = RSADecryptString(r->priv, ciphered);
	if (zlib.length() && RSAVerifyString(p->pub, zlib, sig)) {
		unciphered = (t1 == 1) ? cpp_zlibd(zlib) : zlib;
	}
	return unciphered;
}


string gen_aes_key_iv(short m, pRSADATA p, pRSAPRIV r)
{
#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("gen_aes_key_iv: %04x", m);
#endif
	PBYTE buffer = (PBYTE)alloca(RAND_SIZE);

	GlobalRNG().GenerateBlock(buffer, RAND_SIZE);
	p->aes_k = hash256(buffer, RAND_SIZE);

	GlobalRNG().GenerateBlock(buffer, RAND_SIZE);
	p->aes_v = hash256(buffer, RAND_SIZE);

	string buf = tlv(10, p->aes_k) + tlv(11, p->aes_v);

	return encode_rsa(0, p, r, buf);
}

void init_priv(pRSAPRIV r, string& priv)
{

	r->priv_k = priv;
	r->priv_s = ::hash(priv);

	int t;
	string tmp, n, e, d, p, q, dp, dq, u;

	while (priv.length()) {
		un_tlv(priv, t, tmp);
		switch (t) {
		case 1: n = tmp; break;
		case 2: e = tmp; break;
		case 3: d = tmp; break;
		case 4: p = tmp; break;
		case 5: q = tmp; break;
		case 6: dp = tmp; break;
		case 7: dq = tmp; break;
		case 8: u = tmp; break;
		}
	}

	// if ( !empty );

	r->pub_k = tlv(1, n) + tlv(2, e);
	r->pub_s = ::hash(r->pub_k);

	r->priv.Initialize(BinaryToInteger(n), BinaryToInteger(e),
							 BinaryToInteger(d), BinaryToInteger(p), BinaryToInteger(q),
							 BinaryToInteger(dp), BinaryToInteger(dq), BinaryToInteger(u));
}

void init_pub(pRSADATA p, string& pub)
{
	p->pub_k = pub;
	p->pub_s = ::hash(pub);

	int t;
	string tmp, n, e;

	while (pub.length()) {
		un_tlv(pub, t, tmp);
		switch (t) {
		case 1: n = tmp; break;
		case 2: e = tmp; break;
		}
	}

	// if ( !empty );

	p->pub.Initialize(BinaryToInteger(n), BinaryToInteger(e));
}

void null_msg(HANDLE context, int type, int status)
{
#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("null_msg: %02x", status);
#endif
	inject_msg(context, type, null);
	imp->rsa_notify(context, status);
}

void rsa_timeout(HANDLE context, pRSADATA p)
{
#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("rsa_timeout");
#endif
	p->state = 0; p->time = 0;
	//	null_msg(context,0x00,-7);
	imp->rsa_notify(context, -7); // сессия разорвана по таймауту
}

int __cdecl rsa_encrypt_file(HANDLE context, LPCSTR file_in, LPCSTR file_out)
{
	pCNTX ptr = get_context_on_id(context);	if (!ptr) return 0;
	pRSADATA p = (pRSADATA)cpp_alloc_pdata(ptr); if (p->state != 7) return 0;

	try {
		CBC_Mode<AES>::Encryption enc((PBYTE)p->aes_k.data(), p->aes_k.length(), (PBYTE)p->aes_v.data());
		FileSource *f = new FileSource(file_in, true, new StreamTransformationFilter(enc, new FileSink(file_out)));
		delete f;
	}
	catch (...) {
		return 0;
	}
	return 1;
}


int __cdecl rsa_decrypt_file(HANDLE context, LPCSTR file_in, LPCSTR file_out)
{
	pCNTX ptr = get_context_on_id(context);	if (!ptr) return 0;
	pRSADATA p = (pRSADATA)cpp_alloc_pdata(ptr); if (p->state != 7) return 0;

	try {
		CBC_Mode<AES>::Decryption dec((PBYTE)p->aes_k.data(), p->aes_k.length(), (PBYTE)p->aes_v.data());
		FileSource *f = new FileSource(file_in, true, new StreamTransformationFilter(dec, new FileSink(file_out)));
		delete f;
	}
	catch (...) {
		return 0;
	}
	return 1;
}

int __cdecl rsa_recv_thread(HANDLE context, string& msg)
{
#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("rsa_recv_thread: %s", msg.c_str());
#endif
	pCNTX ptr = get_context_on_id(context);	if (!ptr) return 0;
	pRSADATA p = (pRSADATA)cpp_alloc_pdata(ptr);
	pRSAPRIV r = rsa_get_priv(ptr);

	string data; int type;
	un_tlv(msg, type, data);
	if (type == -1) return 0;

#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("rsa_recv_thread: %02x %d", type, p->state);
#endif
	int t[4], features;

	switch (type) {
	case 0x10:
		{
			string sha1, sha2;
			un_tlv(un_tlv(un_tlv(data, t[0], features), t[1], sha1), t[2], sha2);
			BOOL lr = (p->pub_s == sha1); BOOL ll = (r->pub_s == sha2);
			switch ((lr << 4) | ll) {
			case 0x11:   // оба паблика совпали
				inject_msg(context, 0x21, gen_aes_key_iv(ptr->mode, p, r));
				p->state = 5;
				break;
			case 0x10:  // совпал удаленный паблик, нужен локальный
				inject_msg(context, 0x22, tlv(0, features) + tlv(1, r->pub_k) + tlv(2, r->pub_s));
				p->state = 3;
				break;
			case 0x01:  // совпал локальный паблик, нужен удаленный
				inject_msg(context, 0x23, tlv(0, features));
				p->state = 3;
				break;
			case 0x00:  // не совпали оба паблика
				inject_msg(context, 0x24, tlv(0, features) + tlv(1, r->pub_k) + tlv(2, r->pub_s));
				p->state = 3;
				break;
			}
		}
		break;

	case 0x22: // получили удаленный паблик, отправляем уже криптоключ
		{
			string pub;
			un_tlv(un_tlv(data, t[0], features), t[1], pub);
			string sig = ::hash(pub);
			if (!imp->rsa_check_pub(context, (PBYTE)pub.data(), (int)pub.length(), (PBYTE)sig.data(), (int)sig.length())) {
				p->state = 0; p->time = 0;
				null_msg(context, 0x00, -type); // сессия разорвана по ошибке
				return 0;
			}
			init_pub(p, pub);
			if (p->state == 0) { // timeout
				//			rsa_connect(context);
				return -1;
			}
			inject_msg(context, 0x32, gen_aes_key_iv(ptr->mode, p, r));
			p->state = 5;
		}
		break;

	case 0x23: // отправляем локальный паблик
		un_tlv(data, t[0], features);
		inject_msg(context, 0x33, tlv(1, r->pub_k) + tlv(2, r->pub_s));
		p->state = 4;
		break;

	case 0x24: // получили удаленный паблик, отправим локальный паблик
		{
			string pub;
			un_tlv(un_tlv(data, t[0], features), t[1], pub);
			string sig = ::hash(pub);
			if (!imp->rsa_check_pub(context, (PBYTE)pub.data(), (int)pub.length(), (PBYTE)sig.data(), (int)sig.length())) {
				p->state = 0; p->time = 0;
				null_msg(context, 0x00, -type); // сессия разорвана по ошибке
				return 0;
			}
			init_pub(p, pub);
			if (p->state == 0) { // timeout
				//			rsa_connect(context);
				return -1;
			}
			inject_msg(context, 0x34, tlv(1, r->pub_k) + tlv(2, r->pub_s));
			p->state = 4;
		}
		break;

	case 0x33: // получили удаленный паблик, отправляем криптоключ
	case 0x34:
		{
			string pub;
			un_tlv(data, t[0], pub);
			string sig = ::hash(pub);
			if (!imp->rsa_check_pub(context, (PBYTE)pub.data(), (int)pub.length(), (PBYTE)sig.data(), (int)sig.length())) {
				p->state = 0; p->time = 0;
				null_msg(context, 0x00, -type); // сессия разорвана по ошибке
				return 0;
			}
			init_pub(p, pub);
			if (p->state == 0) { // timeout
				//			rsa_connect(context);
				return -1;
			}
			inject_msg(context, 0x40, gen_aes_key_iv(ptr->mode, p, r));
			p->state = 5;
		}
		break;

	case 0x21: // получили криптоключ, отправляем криптотест
	case 0x32:
	case 0x40:
		{
			string key = decode_rsa(p, r, data);
			if (!key.length()) {
				p->state = 0; p->time = 0;
				null_msg(context, 0x00, -type); // сессия разорвана по ошибке
				return 0;
			}
			un_tlv(key, t[0], p->aes_k);
			un_tlv(key, t[1], p->aes_v);
			PBYTE buffer = (PBYTE)alloca(RAND_SIZE);
			GlobalRNG().GenerateBlock(buffer, RAND_SIZE);
			inject_msg(context, 0x50, encode_msg(0, p, ::hash(buffer, RAND_SIZE)));
			p->state = 6;
		}
		break;

	case 0x0D: // запрос паблика
	case 0xD0: // ответ пабликом
		{
			string pub, sha;
			un_tlv(un_tlv(un_tlv(data, t[0], features), t[1], pub), t[2], sha);
			if (p->pub_k != pub) { // пришел новый паблик
				string sig = ::hash(pub);
				if (!imp->rsa_check_pub(context, (PBYTE)pub.data(), (int)pub.length(), (PBYTE)sig.data(), (int)sig.length())) {
					p->state = 0; p->time = 0;
					null_msg(context, 0x00, -type); // сессия разорвана по ошибке
					return 0;
				}
				init_pub(p, pub);
			}
			if (type == 0x0D) // нужно отправить мой паблик
				inject_msg(context, 0xD0, tlv(0, features) + tlv(1, r->pub_k) + tlv(2, p->pub_s));

			p->state = 0; p->time = 0;
		}
		break;
	}

	if (p->state != 0 && p->state != 7)
		p->time = gettime() + timeout;

	return 1;
}

void rsa_alloc(pCNTX ptr)
{
	pRSADATA p = new RSADATA;
	p->state = 0;
	p->time = 0;
	p->thread = p->event = NULL;
	p->thread_exit = 0;
	p->queue = new STRINGQUEUE;
	ptr->pdata = (PBYTE)p;
}

int rsa_free(pCNTX ptr)
{
	pRSADATA p = (pRSADATA)ptr->pdata;
	if (p == NULL)
		return true;

	if (p->event) {
		p->thread_exit = 2; // отпускаем поток в свободное плавание
		SetEvent(p->event);
		return false;
	}

	delete p->queue;
	delete p;
	ptr->pdata = NULL;
	return true;
}

void rsa_free_thread(pRSADATA p)
{
	if (p->event) {
		p->thread_exit = 1;
		SetEvent(p->event);
		// ждем завершения потока
		WaitForSingleObject(p->thread, INFINITE);
		CloseHandle(p->thread);
		CloseHandle(p->event);
		p->thread = p->event = NULL;
		p->thread_exit = 0;
	}
	p->time = 0;
	clear_queue(p);
}

void clear_queue(pRSADATA p)
{
	mir_cslock lck(localQueueMutex);
	while (p->queue && !p->queue->empty())
		p->queue->pop();
}

// establish RSA/AES thread
unsigned __stdcall sttConnectThread(LPVOID arg)
{
	HANDLE context = (HANDLE)arg;

	pCNTX ptr = get_context_on_id(context);
	if (!ptr)
		return 0;

	pRSADATA p = (pRSADATA)cpp_alloc_pdata(ptr);

	while (true) {
#if defined(_DEBUG) || defined(NETLIB_LOG)
		Sent_NetLog("sttConnectThread: WaitForSingleObject");
#endif
		WaitForSingleObject(p->event, INFINITE); // dwMsec rc==WAIT_TIMEOUT
		if (p->thread_exit == 1) return 0;
		if (p->thread_exit == 2) {
			// мы в свободном плавании - освободим память и завершим трэд
			CloseHandle(p->thread);
			CloseHandle(p->event);
			SAFE_DELETE(p->queue);
			SAFE_DELETE(p);
			return 0;
		}

		// дождались сообщения в очереди
		while (!p->thread_exit && p->queue && !p->queue->empty()) {
			// обработаем сообщения из очереди
			if (rsa_recv_thread(context, p->queue->front()) == -1) {
				// очистить очередь
				clear_queue(p);
				break;
			}

			mir_cslock lck(localQueueMutex);
			p->queue->pop();
		}
	}
}

int __cdecl rsa_export_keypair(short mode, LPSTR privKey, LPSTR pubKey, LPSTR passPhrase)
{
#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("rsa_export_keypair: %d", mode);
#endif

	pCNTX ptr = get_context_on_id(hRSA4096); if (!ptr) return 0;
	pRSAPRIV r = (pRSAPRIV)ptr->pdata;

	if (pubKey) {
		string pub;
		pub = tlv(3, r->pub_k) + tlv(4, r->pub_s);
		pub = pub_beg + crlf + add_delim(base64encode(pub), crlf, 65) + pub_end + crlf;
		strcpy(pubKey, pub.c_str());
	}

	if (privKey && passPhrase) {
		string key = hash256(passPhrase);
		string iv = hash256(key);
		string priv = pad256(r->priv_k);

		string ciphered;
		try {
			CBC_Mode<AES>::Encryption enc((PBYTE)key.data(), key.length(), (PBYTE)iv.data());
			StreamTransformationFilter cbcEncryptor(enc, new StringSink(ciphered));
			cbcEncryptor.Put((PBYTE)priv.data(), priv.length());
			cbcEncryptor.MessageEnd();
		}
		catch (...) {
#if defined(_DEBUG) || defined(NETLIB_LOG)
			Sent_NetLog("rsa_export_keypair: error bad_passphrase");
#endif
			return 0;
		}

		priv = tlv(1, ciphered) + tlv(2, ::hash(ciphered));
		priv = priv_beg + crlf + add_delim(base64encode(priv), crlf, 65) + priv_end + crlf;

		strcpy(privKey, priv.c_str());
	}

	return 1;
}

int __cdecl rsa_import_keypair(short mode, LPSTR privKey, LPSTR passPhrase)
{
	if (!passPhrase)
		return 0;

#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("rsa_import_keypair: %d", mode);
#endif
	pCNTX ptr = get_context_on_id(hRSA4096); if (!ptr) return 0;
	pRSAPRIV r = (pRSAPRIV)ptr->pdata;

	string priv;
	priv.assign(privKey);
	del_delim(priv, crlf);

	size_t found = priv.find(priv_beg);
	if (found != string::npos) {
		priv = priv.substr(found + priv_beg.length());
		found = priv.find(priv_end);
		if (found != string::npos) {
			priv = base64decode(priv.substr(0, found));
			TLV k(priv);
			if (k.exist(1) && k.exist(2) && ::hash(k.get(1)) == k.get(2)) {
				priv = k.get(1);

				string key = hash256(passPhrase);
				string iv = hash256(key);

				string unciphered;
				try {
					CBC_Mode<AES>::Decryption dec((PBYTE)key.data(), key.length(), (PBYTE)iv.data());
					StreamTransformationFilter cbcDecryptor(dec, new StringSink(unciphered));
					cbcDecryptor.Put((PBYTE)priv.data(), priv.length());
					cbcDecryptor.MessageEnd();
				}
				catch (...) {
#if defined(_DEBUG) || defined(NETLIB_LOG)
					Sent_NetLog("rsa_import_keypair: error bad_passphrase");
#endif
					return 0;
				}

				init_priv(r, unciphered);
				return 1;
			}
		}
	}

	return 0;
}

int __cdecl rsa_export_pubkey(HANDLE context, LPSTR pubKey)
{
#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("rsa_export_pubkey: %08x", context);
#endif
	pCNTX ptr = get_context_on_id(context);	if (!ptr) return 0;
	pRSADATA p = (pRSADATA)cpp_alloc_pdata(ptr);
	if (!p->pub_k.length() || !pubKey)
		return 0;

	string pub;
	pub = tlv(3, p->pub_k) + tlv(4, p->pub_s);
	pub = pub_beg + crlf + add_delim(base64encode(pub), crlf, 65) + pub_end + crlf;
	strcpy(pubKey, pub.c_str());

	return 1;
}

int __cdecl rsa_import_pubkey(HANDLE context, LPSTR pubKey)
{
#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("rsa_import_pubkey: %08x", context);
#endif
	pCNTX ptr = get_context_on_id(context);	if (!ptr) return 0;
	pRSADATA p = (pRSADATA)cpp_alloc_pdata(ptr);
	if (!pubKey)
		return 0;

	string pub;
	pub.assign(pubKey);
	del_delim(pub, crlf);

	size_t found = pub.find(pub_beg);
	if (found != string::npos) {
		pub = pub.substr(found + pub_beg.length());
		found = pub.find(pub_end);
		if (found != string::npos) {
			pub = base64decode(pub.substr(0, found));
			TLV k(pub);
			if (k.exist(3) && k.exist(4) && ::hash(k.get(3)) == k.get(4)) {
				init_pub(p, k.get(3));
				return 1;
			}
		}
	}
	return 0;
}
