#ifndef __CRYPTOPP_H__
#define __CRYPTOPP_H__

#include <queue>
#include <deque>
#include <list>
#include <map>

#pragma warning(disable: 4231 4250 4251 4275 4660 4661 4700 4706 4786 4355)
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include "../crypto/src/modes.h"
#include "../crypto/src/osrng.h"
#include "../crypto/src/rsa.h"
#include "../crypto/src/aes.h"
#include "../crypto/src/dh.h"
#include "../crypto/src/crc.h"
#include "../crypto/src/ripemd.h"
#include "../crypto/src/sha.h"
#include "../crypto/src/tiger.h"
#include "../crypto/src/gzip.h"
#include "../crypto/src/zlib.h"
#include "../crypto/src/files.h"

USING_NAMESPACE(CryptoPP);
USING_NAMESPACE(std)

#define KEYSIZE 256
#define DEFMSGS 4096

#define HEADER 0xABCD1234
#define FOOTER 0x9876FEDC
#define EMPTYH 0xF1E2D3C4

typedef struct __CNTX {
	u_int	header;     // HEADER
	short	mode;       // mode of encoding
	short	features;   // features of client
	short	error;      // error code of last operation
	uint8_t*	pdata;   // data block
	PVOID	udata;      // user data
	LPSTR	tmp;        // return string
	u_int	deleted;    // delete time&flag to delete
	u_int	footer;     // FOOTER
} CNTX;
typedef CNTX* pCNTX;


#define FEATURES_UTF8			0x01
#define FEATURES_BASE64			0x02
#define FEATURES_GZIP			0x04
#define FEATURES_CRC32			0x08
#define FEATURES_PSK			0x10
#define FEATURES_NEWPG			0x20
#define FEATURES_RSA			0x40

#define MODE_BASE16			0x0000
#define MODE_BASE64			0x0001
#define MODE_PGP			0x0002
#define MODE_GPG			0x0004
#define MODE_GPG_ANSI			0x0008
#define MODE_PRIV_KEY			0x0010
#define MODE_RSA_2048			0x0020
#define MODE_RSA_4096			0x0040
#define MODE_RSA			MODE_RSA_4096
#define MODE_RSA_ONLY			0x0080
#define MODE_RSA_ZLIB			0x0100
#define MODE_RSA_BER 			0x0200

#define DATA_GZIP			1

typedef struct __SIMDATA {
	DH	*dh;         // diffie-hellman
	uint8_t*	PubA;  // public keyA 2048 bit
	uint8_t*	KeyA;  // private keyA 2048 bit
	uint8_t*	KeyB;  // public keyB 2048 bit
	uint8_t*	KeyX;  // secret keyX 192 bit
	uint8_t*	KeyP;  // pre-shared keyP 192 bit
} SIMDATA;
typedef SIMDATA* pSIMDATA;


typedef struct __PGPDATA {
	uint8_t*	pgpKeyID; // PGP KeyID
	uint8_t*	pgpKey;   // PGP Key
} PGPDATA;
typedef PGPDATA* pPGPDATA;


typedef struct __GPGDATA {
	uint8_t	*gpgKeyID; // GPG KeyID
} GPGDATA;
typedef GPGDATA* pGPGDATA;


#define RSA_KEYSIZE		SHA1::DIGESTSIZE
#define RSA_CalculateDigest	SHA1().CalculateDigest


typedef struct __RSAPRIV {
	string	priv_k;	// private key string
	string	priv_s;	// hash(priv_k)
	RSA::PrivateKey priv; // private key
	string	pub_k;	// public key string
	string	pub_s;	// hash(pub_k)
} RSAPRIV;
typedef RSAPRIV* pRSAPRIV;


typedef deque<string, allocator<string> > STRINGDEQUE;
typedef queue<string, STRINGDEQUE> STRINGQUEUE;


typedef struct __RSADATA {
	short			state;	// 0 - нифига нет, 1..6 - keyexchange, 7 - соединение установлено
	u_int			time;	// для прерывания keyexchange, если долго нет ответа
	string			pub_k;	// public key string
	string			pub_s;	// hash(pub_k)
	RSA::PublicKey		pub;	// public key
	string			aes_k;	// aes key
	string			aes_v;	// aes iv
	HANDLE	 		thread; // thread handle
	BOOL			thread_exit;
	HANDLE			event;	// thread event
	STRINGQUEUE		*queue;  // thread queue
} RSADATA;
typedef RSADATA* pRSADATA;


#define ERROR_NONE			0
#define ERROR_SEH			1
#define ERROR_NO_KEYA			2
#define ERROR_NO_KEYB			3
#define ERROR_NO_KEYX			4
#define ERROR_BAD_LEN			5
#define ERROR_BAD_CRC			6
#define ERROR_NO_PSK			7
#define ERROR_BAD_PSK			8
#define ERROR_BAD_KEYB			9
#define ERROR_NO_PGP_KEY		10
#define ERROR_NO_PGP_PASS		11
#define ERROR_NO_GPG_KEY		12
#define ERROR_NO_GPG_PASS		13

#if defined(_DEBUG)
#define FEATURES (FEATURES_UTF8 | FEATURES_BASE64 | FEATURES_GZIP | FEATURES_CRC32 | FEATURES_NEWPG)
#else
#define FEATURES (FEATURES_UTF8 | FEATURES_BASE64 | FEATURES_GZIP | FEATURES_CRC32 | FEATURES_NEWPG | FEATURES_RSA)
#endif

#define DLLEXPORT __declspec(dllexport)

extern LPCSTR szModuleName;
extern LPCSTR szVersionStr;

pCNTX get_context_on_id(int);
pCNTX get_context_on_id(HANDLE);
void cpp_free_keys(pCNTX);
uint8_t *cpp_gzip(uint8_t*, size_t, size_t&);
uint8_t *cpp_gunzip(uint8_t*, size_t, size_t&);
string cpp_zlibc(string&);
string cpp_zlibd(string&);

typedef struct {
	int(__cdecl *rsa_gen_keypair)(short);				// генерит RSA-ключи для указанной длины (либо тока 2048, либо 2048 и 4096)
	int(__cdecl *rsa_get_keypair)(short, uint8_t*, int*, uint8_t*, int*);	// возвращает пару ключей для указанной длины
	int(__cdecl *rsa_get_keyhash)(short, uint8_t*, int*, uint8_t*, int*);	// возвращает hash пары ключей для указанной длины
	int(__cdecl *rsa_set_keypair)(short, uint8_t*, int);			// устанавливает ключи, указанной длины
	int(__cdecl *rsa_get_pubkey)(HANDLE, uint8_t*, int*);			// возвращает паблик ключ из указанного контекста
	int(__cdecl *rsa_set_pubkey)(HANDLE, uint8_t*, int);			// загружает паблик ключ для указанного контекста
	void(__cdecl *rsa_set_timeout)(int);				// установить таймаут для установки секюрного соединения
	int(__cdecl *rsa_get_state)(HANDLE);				// получить статус указанного контекста
	int(__cdecl *rsa_get_hash)(uint8_t*, int, uint8_t*, int*);			// вычисляет SHA1(key)
	int(__cdecl *rsa_connect)(HANDLE);					// запускает процесс установки содинения с указанным контекстом
	int(__cdecl *rsa_disconnect)(HANDLE);				// разрывает соединение с указанным контекстом
	int(__cdecl *rsa_disabled)(HANDLE);				// разрывает соединение по причине "disabled"
	LPSTR(__cdecl *rsa_recv)(HANDLE, LPCSTR);				// необходимо передавать сюда все входящие протокольные сообщения
	int(__cdecl *rsa_send)(HANDLE, LPCSTR);				// вызываем для отправки сообщения клиенту
	int(__cdecl *rsa_encrypt_file)(HANDLE, LPCSTR, LPCSTR);
	int(__cdecl *rsa_decrypt_file)(HANDLE, LPCSTR, LPCSTR);
	LPSTR(__cdecl *utf8encode)(LPCWSTR);
	LPWSTR(__cdecl *utf8decode)(LPCSTR);
	int(__cdecl *is_7bit_string)(LPCSTR);
	int(__cdecl *is_utf8_string)(LPCSTR);
	int(__cdecl *rsa_export_keypair)(short, LPSTR, LPSTR, LPSTR);		// export private key
	int(__cdecl *rsa_import_keypair)(short, LPSTR, LPSTR);		// import & activate private key
	int(__cdecl *rsa_export_pubkey)(HANDLE, LPSTR);			// export public key from context
	int(__cdecl *rsa_import_pubkey)(HANDLE, LPSTR);			// import public key into context
} RSA_EXPORT;
typedef RSA_EXPORT* pRSA_EXPORT;

typedef struct {
	int(__cdecl *rsa_inject)(HANDLE, LPCSTR);			// вставляет сообщение в очередь на отправку
	int(__cdecl *rsa_check_pub)(HANDLE, uint8_t*, int, uint8_t*, int);	// проверяет интерактивно SHA и сохраняет ключ, если все нормально
	void(__cdecl *rsa_notify)(HANDLE, int);			// нотификация о смене состояния
} RSA_IMPORT;
typedef RSA_IMPORT* pRSA_IMPORT;

NAMESPACE_BEGIN(CryptoPP)
typedef RSASS<PKCS1v15, SHA256>::Signer RSASSA_PKCS1v15_SHA256_Signer;
typedef RSASS<PKCS1v15, SHA256>::Verifier RSASSA_PKCS1v15_SHA256_Verifier;
NAMESPACE_END

#endif
