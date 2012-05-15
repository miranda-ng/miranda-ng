#ifndef __CRYPTOPP_H__
#define __CRYPTOPP_H__

#define CPP_FEATURES_UTF8		0x01
#define CPP_FEATURES_BASE64		0x02
#define CPP_FEATURES_GZIP		0x04
#define CPP_FEATURES_CRC32		0x08
#define CPP_FEATURES_PSK		0x10
#define CPP_FEATURES_NEWPG		0x20
#define CPP_FEATURES_RSA		0x40

#define CPP_MODE_BASE16			0x0000
#define CPP_MODE_BASE64			0x0001
#define CPP_MODE_PGP			0x0002
#define CPP_MODE_GPG			0x0004
#define CPP_MODE_GPG_ANSI		0x0008
#define CPP_MODE_PRIV_KEY		0x0010
#define CPP_MODE_RSA_2048		0x0020
#define CPP_MODE_RSA_4096		0x0040
#define CPP_MODE_RSA			CPP_MODE_RSA_4096
#define CPP_MODE_RSA_ONLY		0x0080
#define CPP_MODE_RSA_ZLIB		0x0100
#define CPP_MODE_RSA_BER 		0x0200

#define CPP_ERROR_NONE			0
#define CPP_ERROR_SEH			1
#define CPP_ERROR_NO_KEYA		2
#define CPP_ERROR_NO_KEYB		3
#define CPP_ERROR_NO_KEYX		4
#define CPP_ERROR_BAD_LEN		5
#define CPP_ERROR_BAD_CRC		6
#define CPP_ERROR_NO_PSK		7
#define CPP_ERROR_BAD_PSK		8
#define CPP_ERROR_BAD_KEYB		9
#define CPP_ERROR_NO_PGP_KEY		10

typedef struct {
    int (__cdecl *rsa_gen_keypair)(short);				// генерит RSA-ключи для указанной длины (либо тока 2048, либо 2048 и 4096)
    int (__cdecl *rsa_get_keypair)(short,PBYTE,int*,PBYTE,int*);	// возвращает пару ключей для указанной длины
    int (__cdecl *rsa_get_keyhash)(short,PBYTE,int*,PBYTE,int*);	// возвращает hash пары ключей для указанной длины
    int (__cdecl *rsa_set_keypair)(short,PBYTE,int);			// устанавливает ключи, указанной длины
    int (__cdecl *rsa_get_pubkey)(HANDLE,PBYTE,int*);			// возвращает паблик ключ из указанного контекста
    int (__cdecl *rsa_set_pubkey)(HANDLE,PBYTE,int);			// загружает паблик ключ для указанного контекста
    void (__cdecl *rsa_set_timeout)(int);				// установить таймаут для установки секюрного соединения
    int (__cdecl *rsa_get_state)(HANDLE);				// получить статус указанного контекста
    int (__cdecl *rsa_get_hash)(PBYTE,int,PBYTE,int*);			// вычисляет SHA1(key)
    int (__cdecl *rsa_connect)(HANDLE);					// запускает процесс установки содинения с указанным контекстом
    int (__cdecl *rsa_disconnect)(HANDLE);				// разрывает соединение с указанным контекстом
    int (__cdecl *rsa_disabled)(HANDLE);				// разрывает соединение по причине "disabled"
    LPSTR (__cdecl *rsa_recv)(HANDLE,LPCSTR);				// необходимо передавать сюда все входящие протокольные сообщения
    int   (__cdecl *rsa_send)(HANDLE,LPCSTR);				// вызываем для отправки сообщения клиенту
    int (__cdecl *rsa_encrypt_file)(HANDLE,LPCSTR,LPCSTR);
    int (__cdecl *rsa_decrypt_file)(HANDLE,LPCSTR,LPCSTR);
    LPSTR  (__cdecl *utf8encode)(LPCWSTR);
    LPWSTR (__cdecl *utf8decode)(LPCSTR);
    int (__cdecl *is_7bit_string)(LPCSTR);
    int (__cdecl *is_utf8_string)(LPCSTR);
    int (__cdecl *rsa_export_keypair)(short,LPSTR,LPSTR,LPSTR);		// export private key
    int (__cdecl *rsa_import_keypair)(short,LPSTR,LPSTR);		// import & activate private key
    int (__cdecl *rsa_export_pubkey)(HANDLE,LPSTR);			// export public key from context
    int (__cdecl *rsa_import_pubkey)(HANDLE,LPSTR);			// import public key into context
} RSA_EXPORT;
typedef RSA_EXPORT* pRSA_EXPORT;

typedef struct {
    int  (__cdecl *rsa_inject)(HANDLE,LPCSTR);			// вставляет сообщение в очередь на отправку
    int  (__cdecl *rsa_check_pub)(HANDLE,PBYTE,int,PBYTE,int);	// проверяет интерактивно SHA и сохраняет ключ, если все нормально
    void (__cdecl *rsa_notify)(HANDLE,int);			// нотификация о смене состояния
} RSA_IMPORT;
typedef RSA_IMPORT* pRSA_IMPORT;


#endif
