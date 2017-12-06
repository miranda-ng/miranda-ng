#ifndef __COMMON_H__
#define __COMMON_H__

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock.h>
#pragma comment (lib,"ws2_32.lib")
#define int64_t __int64

#else

#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>
typedef int SOCKET;
#define closesocket close
typedef int BOOL;
#define TRUE  1
#define FALSE 0

#endif

#ifdef DEBUG
#include <stdio.h>
#define DBGPRINT printf
#else
#define DBGPRINT
#endif

/* Use Skype 5+ Diffie-Hellmann RC4 wrapped login comunication. 
 * Fortunately Login servers also still work without it too. 
 * To keep library small and clean, we keep it disabled too,
 * but you can enable it if you want.
 */
//#define USE_RC4

#ifdef CRYPT_WOLFSSL
#include "wolfssl.h"
#else
#include <openssl/md5.h>
#include <openssl/aes.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>
#include <openssl/err.h>
#ifdef USE_RC4
#include <openssl/rc4.h>
#include <openssl/dh.h>
#endif
#endif

typedef	 unsigned char		uchar;
typedef	 unsigned short		ushort;
typedef	 unsigned int		uint;
typedef	 unsigned long		ulong;

#define	 MAX_IP_LEN			15
#define	 HTTPS_PORT			443

#define	 NODEID_SZ			8
#define	 HANDSHAKE_SZ		0x05
#define  CONCAT_SALT		"\nskyper\n"
#define  KEYSZ				0x200
#define	 SK_SZ				0xC0
#define  MODULUS_SZ			0x80
#define  HTTPS_HSR_MAGIC	"\x16\x03\x01"
#define  HTTPS_HSRR_MAGIC	"\x17\x03\x01"
#define	 LOGIN_OK			4200

#define	 RAW_PARAMS			0x41
#define	 EXT_PARAMS			0x42

#define	 VER_STR			"0/7.44.0.104"

typedef  struct
{
	char		ip[MAX_IP_LEN + 1];
	int			port;
} Host;

typedef struct
{
	uchar	*Memory;
	unsigned long MsZ;
}	Memory_U;

typedef struct
{
	uchar		*User;
	uchar		LoginHash[MD5_DIGEST_LENGTH];
	uint		Expiry;
	RSA			*RSAKeys;
	//Memory_U	Modulus;
	Memory_U	SignedCredentials;
}	SLoginDatas;

typedef struct
{
	uchar			SessionKey[SK_SZ];
	uchar			NodeID[NODEID_SZ];
	uchar			Language[2];
	uint			PublicIP;
	SLoginDatas		LoginD; 
	int (__cdecl *pfLog)(void *stream, const char *format, ...);
	void *pLogStream;
}	Skype_Inst;

typedef struct 
{
	SOCKET			LSSock;
#ifdef USE_RC4
	RC4_KEY			rc4_send;
	RC4_KEY			rc4_recv;
#endif
}	LSConnection;

#pragma	pack(1)
typedef struct
{
	unsigned char  MAGIC[3];
	unsigned short ResponseLen;
}	HttpsPacketHeader;

#pragma pack()

extern char				*SkypeModulus1536[];

char *KeySelect(uint KeyIndex);
#endif
