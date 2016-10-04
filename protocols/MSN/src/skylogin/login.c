/*  
 * Skype Login
 * 
 * Based on:
 *   FakeSkype : Skype reverse engineering proof-of-concept client
 *               Ouanilo MEDEGAN (c) 2006   http://www.oklabs.net
 *   pyskype   : Skype login Python script by uunicorn
 *
 * Written by:   leecher@dose.0wnz.at (c) 2015 
 *
 * Module:       Skype server login
 *
 */
#include "common.h"
#include "objects.h"
#include "random.h"
#include "platform.h"
#include "crc.h"
#ifdef _DEBUG
#include <time.h>
#include <stdio.h>
#endif

#ifdef USE_RC4
#include "rc4comm.c"
#else
#define RC4Comm_Init(conn) 0
#define RC4Comm_Send(conn,buf,len) send(conn->LSSock,buf,len,0)
#define RC4Comm_Recv(conn,buf,len) recv(conn->LSSock,buf,len,0)
#endif

static Host LoginServers[] = {
	{"91.190.216.17", 33033},
	{"91.190.218.40", 33033},
};

static BOOL SendHandShake2LS(Skype_Inst *pInst, LSConnection *pConn, Host *CurLS)
{
	uchar				HandShakePkt[HANDSHAKE_SZ] = {0};
	HttpsPacketHeader	*HSHeader, Response;
	struct sockaddr_in	Sender={0};

	HSHeader = (HttpsPacketHeader *)HandShakePkt;
	memcpy(HSHeader->MAGIC, HTTPS_HSR_MAGIC, sizeof(HSHeader->MAGIC));
	HSHeader->ResponseLen = 0;
	pInst->pfLog(pInst->pLogStream, "Sending Handshake to Login Server %s..\n", CurLS->ip);
	Sender.sin_family = AF_INET;
	Sender.sin_port = htons((short)CurLS->port);
	Sender.sin_addr.s_addr = inet_addr(CurLS->ip);
	if (connect(pConn->LSSock, (struct sockaddr *)&Sender, sizeof(Sender)) < 0)
	{
		pInst->pfLog(pInst->pLogStream, "Connection refused..\n");
		return FALSE;
	}
	if (RC4Comm_Init(pConn) < 0 ||
		RC4Comm_Send(pConn, (const char *)HandShakePkt, HANDSHAKE_SZ)<=0 ||
		RC4Comm_Recv(pConn, (char*)&Response, sizeof(Response))<=0 ||
		memcmp(Response.MAGIC, HTTPS_HSRR_MAGIC, sizeof(Response.MAGIC)))
		return FALSE;
	return TRUE;
}

/* If Pass is NULL, User is assumed to be OAuth string and OAuth logon is performed */
static int SendAuthentificationBlobLS(Skype_Inst *pInst, LSConnection *pConn, const char *User, const char *Pass)
{
	int64_t				PlatForm;
	uchar				AuthBlob[0xFFFF] = {0};
	uchar				SHAResult[32] = {0};
	uchar				Modulus[MODULUS_SZ * 2] = {0};
	uchar				ivec[AES_BLOCK_SIZE] = {0};
	uchar				ecount_buf[AES_BLOCK_SIZE] = {0};
	uint				MiscDatas[0x05] = {0};
	uchar				SessionKey[SK_SZ];
	uchar				*Browser;
	uchar				*Mark;
	uchar				*MarkObjL;
	uint				Idx, Size, Crc, BSize, ret = 0;
	HttpsPacketHeader	*HSHeader;
	uchar				HSHeaderBuf[sizeof(HttpsPacketHeader)], RecvBuf[0x1000];
	AES_KEY				AesKey;
	MD5_CTX				Context;
	RSA					*SkypeRSA;
	ObjectDesc			Obj2000, ObjSessionKey, ObjZBool1, ObjRequestCode, ObjZBool2, ObjModulus, ObjPlatForm, ObjLang, ObjMiscDatas, ObjVer, ObjPubAddr;
	SResponse			Response={0};
	

	if (!pInst->LoginD.RSAKeys)
	{
		BIGNUM				*KeyExp;

		pInst->pfLog(pInst->pLogStream, "Generating RSA Keys Pair (Size = %d Bits)..\n", KEYSZ);
		pInst->LoginD.RSAKeys = RSA_new();
		KeyExp = BN_new();
		BN_set_word(KeyExp, RSA_F4);
		Idx = RSA_generate_key_ex(pInst->LoginD.RSAKeys, KEYSZ * 2, KeyExp, NULL);
		BN_free(KeyExp);
		if (Idx == -1)
		{
			pInst->pfLog(pInst->pLogStream, "Error generating Keys..\n\n");
			RSA_free(pInst->LoginD.RSAKeys);
			pInst->LoginD.RSAKeys = NULL;
			return (0);
		}
	}

	Idx = BN_bn2bin(pInst->LoginD.RSAKeys->n, Modulus);
	Idx = BN_bn2bin(pInst->LoginD.RSAKeys->d, Modulus + Idx);

	Browser = AuthBlob;

	HSHeader = (HttpsPacketHeader *)Browser;
	memcpy(HSHeader->MAGIC, HTTPS_HSR_MAGIC, sizeof(HSHeader->MAGIC));
	HSHeader->ResponseLen = htons(0xCD);
	Browser += sizeof(HttpsPacketHeader);

	*Browser++ = RAW_PARAMS;
	*Browser++ = 0x03;

	Obj2000.Family = OBJ_FAMILY_NBR;
	Obj2000.Id = OBJ_ID_2000;
	Obj2000.Value.Nbr = 0x2000;
	WriteObject(&Browser, Obj2000);

	SpecialSHA(pInst->SessionKey, SK_SZ, SHAResult, 32);
	AES_set_encrypt_key(SHAResult, 256, &AesKey);

	SkypeRSA = RSA_new();
	BN_hex2bn(&(SkypeRSA->n), SkypeModulus1536[1]);
	BN_hex2bn(&(SkypeRSA->e), "010001");
	Idx = RSA_public_encrypt(SK_SZ, pInst->SessionKey, SessionKey, SkypeRSA, RSA_NO_PADDING);
	RSA_free(SkypeRSA);
	if (Idx < 0)
	{
		pInst->pfLog(pInst->pLogStream, "RSA_public_encrypt failed..\n\n");
		return (0);
	}

	ObjSessionKey.Family = OBJ_FAMILY_BLOB;
	ObjSessionKey.Id = OBJ_ID_SK;
	ObjSessionKey.Value.Memory.Memory = (uchar *)&SessionKey;
	ObjSessionKey.Value.Memory.MsZ = SK_SZ;
	WriteObject(&Browser, ObjSessionKey);

	ObjZBool1.Family = OBJ_FAMILY_NBR;
	ObjZBool1.Id = OBJ_ID_ZBOOL1;
	ObjZBool1.Value.Nbr = 0x01;
	WriteObject(&Browser, ObjZBool1);

	Mark = Browser;
	HSHeader = (HttpsPacketHeader *)Browser;
	memcpy(HSHeader->MAGIC, HTTPS_HSRR_MAGIC, sizeof(HSHeader->MAGIC));
	HSHeader->ResponseLen = 0x00;
	Browser += sizeof(HttpsPacketHeader);

	MarkObjL = Browser;
	if (Pass)
	{
		ObjectDesc ObjUserName, ObjSharedSecret;

		*Browser++ = RAW_PARAMS;
		*Browser++ = 0x04;

		ObjRequestCode.Family = OBJ_FAMILY_NBR;
		ObjRequestCode.Id = OBJ_ID_REQCODE;
		ObjRequestCode.Value.Nbr = 0x1399;
		WriteObject(&Browser, ObjRequestCode);

		ObjZBool2.Family = OBJ_FAMILY_NBR;
		ObjZBool2.Id = OBJ_ID_ZBOOL2;
		ObjZBool2.Value.Nbr = 0x01;
		WriteObject(&Browser, ObjZBool2);

		ObjUserName.Family = OBJ_FAMILY_STRING;
		ObjUserName.Id = OBJ_ID_USERNAME;
		ObjUserName.Value.Memory.Memory = (uchar *)User;
		ObjUserName.Value.Memory.MsZ = (uchar)strlen(User);
		WriteObject(&Browser, ObjUserName);

		MD5_Init(&Context);
		MD5_Update(&Context, User, (ulong)strlen(User));
		MD5_Update(&Context, CONCAT_SALT, (ulong)strlen(CONCAT_SALT));
		MD5_Update(&Context, Pass, (ulong)strlen(Pass));
		MD5_Final(pInst->LoginD.LoginHash, &Context);

		ObjSharedSecret.Family = OBJ_FAMILY_BLOB;
		ObjSharedSecret.Id = OBJ_ID_USERPASS;
		ObjSharedSecret.Value.Memory.Memory = (uchar *)pInst->LoginD.LoginHash;
		ObjSharedSecret.Value.Memory.MsZ = MD5_DIGEST_LENGTH;
		WriteObject(&Browser, ObjSharedSecret);

		*Browser++ = RAW_PARAMS;
		*Browser++ = 0x06;

		ObjModulus.Family = OBJ_FAMILY_BLOB;
		ObjModulus.Id = OBJ_ID_MODULUS;
		ObjModulus.Value.Memory.Memory = (uchar *)Modulus;
		ObjModulus.Value.Memory.MsZ = MODULUS_SZ;
		WriteObject(&Browser, ObjModulus);

		PlatForm = PlatFormSpecific();

		ObjPlatForm.Family = OBJ_FAMILY_TABLE;
		ObjPlatForm.Id = OBJ_ID_PLATFORM;
		memcpy(ObjPlatForm.Value.Table, (uchar *)&PlatForm, sizeof(ObjPlatForm.Value.Table));
		WriteObject(&Browser, ObjPlatForm);

		ObjLang.Family = OBJ_FAMILY_STRING;
		ObjLang.Id = OBJ_ID_LANG;
		ObjLang.Value.Memory.Memory = pInst->Language;
		ObjLang.Value.Memory.MsZ = sizeof(pInst->Language);
		WriteObject(&Browser, ObjLang);

		FillMiscDatas(pInst, MiscDatas);
		ObjMiscDatas.Family = OBJ_FAMILY_INTLIST;
		ObjMiscDatas.Id = OBJ_ID_MISCD;
		ObjMiscDatas.Value.Memory.Memory = (uchar *)MiscDatas;
		ObjMiscDatas.Value.Memory.MsZ = 0x05;
		WriteObject(&Browser, ObjMiscDatas);

		ObjVer.Family = OBJ_FAMILY_STRING;
		ObjVer.Id = OBJ_ID_VERSION;
		ObjVer.Value.Memory.Memory = (uchar *)VER_STR;
		ObjVer.Value.Memory.MsZ = (uchar)strlen(VER_STR);
		WriteObject(&Browser, ObjVer);

		ObjPubAddr.Family = OBJ_FAMILY_NBR;
		ObjPubAddr.Id = OBJ_ID_PUBADDR;
		ObjPubAddr.Value.Nbr = pInst->PublicIP;
		WriteObject(&Browser, ObjPubAddr);
	}
	else
	{
		int64_t			PartnerId = 999;
		ObjectDesc		ObjPartnerId, ObjOauth;

		// OAuth logon
		*Browser++ = RAW_PARAMS;
		*Browser++ = 0x02;

		ObjRequestCode.Family = OBJ_FAMILY_NBR;
		ObjRequestCode.Id = OBJ_ID_REQCODE;
		ObjRequestCode.Value.Nbr = 0x13a3;
		WriteObject(&Browser, ObjRequestCode);

		ObjZBool2.Family = OBJ_FAMILY_NBR;
		ObjZBool2.Id = OBJ_ID_ZBOOL2;
		ObjZBool2.Value.Nbr = 0x3d;
		WriteObject(&Browser, ObjZBool2);

		*Browser++ = RAW_PARAMS;
		*Browser++ = 0x08;

		ObjModulus.Family = OBJ_FAMILY_BLOB;
		ObjModulus.Id = OBJ_ID_MODULUS;
		ObjModulus.Value.Memory.Memory = (uchar *)Modulus;
		ObjModulus.Value.Memory.MsZ = MODULUS_SZ;
		WriteObject(&Browser, ObjModulus);

		PlatForm = PlatFormSpecific();

		ObjPlatForm.Family = OBJ_FAMILY_TABLE;
		ObjPlatForm.Id = OBJ_ID_PLATFORM;
		memcpy(ObjPlatForm.Value.Table, (uchar *)&PlatForm, sizeof(ObjPlatForm.Value.Table));
		WriteObject(&Browser, ObjPlatForm);

		FillMiscDatas(pInst, MiscDatas);
		ObjMiscDatas.Family = OBJ_FAMILY_INTLIST;
		ObjMiscDatas.Id = OBJ_ID_MISCD;
		ObjMiscDatas.Value.Memory.Memory = (uchar *)MiscDatas;
		ObjMiscDatas.Value.Memory.MsZ = 0x05;
		WriteObject(&Browser, ObjMiscDatas);

		ObjLang.Family = OBJ_FAMILY_STRING;
		ObjLang.Id = OBJ_ID_LANG;
		ObjLang.Value.Memory.Memory = pInst->Language;
		ObjLang.Value.Memory.MsZ = sizeof(pInst->Language);
		WriteObject(&Browser, ObjLang);

		ObjPartnerId.Family = OBJ_FAMILY_TABLE;
		ObjPlatForm.Id = OBJ_ID_PARTNERID;
		memcpy(ObjPlatForm.Value.Table, (uchar *)&PartnerId, sizeof(ObjPlatForm.Value.Table));
		WriteObject(&Browser, ObjPlatForm);

		ObjOauth.Family = OBJ_FAMILY_STRING;
		ObjOauth.Id = OBJ_ID_OAUTH;
		ObjOauth.Value.Memory.Memory = (uchar *)User;
		ObjOauth.Value.Memory.MsZ = (ulong)strlen(User);
		WriteObject(&Browser, ObjOauth);

		ObjVer.Family = OBJ_FAMILY_STRING;
		ObjVer.Id = OBJ_ID_VERSION;
		ObjVer.Value.Memory.Memory = (uchar *)VER_STR;
		ObjVer.Value.Memory.MsZ = (uchar)strlen(VER_STR);
		WriteObject(&Browser, ObjVer);

		ObjPubAddr.Family = OBJ_FAMILY_NBR;
		ObjPubAddr.Id = OBJ_ID_PUBADDR;
		ObjPubAddr.Value.Nbr = pInst->PublicIP;
		WriteObject(&Browser, ObjPubAddr);
	}

	Size = (uint)(Browser - MarkObjL);
	HSHeader->ResponseLen = htons((u_short)(Size + 0x02));

	Idx = 0;
	memset(ivec, 0, AES_BLOCK_SIZE);
	memset(ecount_buf, 0, AES_BLOCK_SIZE);
	AES_ctr128_encrypt(MarkObjL, MarkObjL, Size, &AesKey, ivec, ecount_buf, &Idx);

	Crc = crc32(MarkObjL, Size, -1);
	*Browser++ = *((uchar *)(&Crc) + 0);
	*Browser++ = *((uchar *)(&Crc) + 1);

	Size = (uint)(Browser - AuthBlob);

	if (RC4Comm_Send(pConn, (const char *)AuthBlob, Size)<=0)
	{
		pInst->pfLog(pInst->pLogStream, "Sending to LS failed :'(..\n");
		return (-1);
	}

	while (!ret && RC4Comm_Recv(pConn, (char *)&HSHeaderBuf, sizeof(HSHeaderBuf))>0)
	{
		HSHeader = (HttpsPacketHeader *)HSHeaderBuf;
		if (strncmp((const char *)HSHeader->MAGIC, HTTPS_HSRR_MAGIC, strlen(HTTPS_HSRR_MAGIC)) ||
			RC4Comm_Recv(pConn, (char *)RecvBuf, (BSize=htons(HSHeader->ResponseLen)))<=0)
		{
			pInst->pfLog(pInst->pLogStream, "Bad Response..\n");
			return (-2);
		}
		pInst->pfLog(pInst->pLogStream, "Auth Response Got..\n\n");

		Idx = 0;
		memset(ivec, 0, AES_BLOCK_SIZE);
		memset(ecount_buf, 0, AES_BLOCK_SIZE);
		BSize-=2;
		ivec[3] = 0x01;
		ivec[7] = 0x01;
		AES_ctr128_encrypt(RecvBuf, RecvBuf, BSize, &AesKey, ivec, ecount_buf, &Idx);

		Browser = RecvBuf;
		while (Browser<RecvBuf+BSize)
			ManageObjects(&Browser, BSize, &Response);
		for (Idx = 0; Idx < Response.NbObj; Idx++)
		{
			switch (Response.Objs[Idx].Id)
			{
			case OBJ_ID_LOGINANSWER:
				switch (Response.Objs[Idx].Value.Nbr)
				{
				case LOGIN_OK:
					pInst->pfLog(pInst->pLogStream, "Login Successful..\n");
					ret = 1;
					break;
				default :
					pInst->pfLog(pInst->pLogStream, "Login Failed.. Bad Credentials..\n");
					FreeResponse(&Response);
					return 0;
				}
				break;
			case OBJ_ID_CIPHERDLOGD:
				if (pInst->LoginD.SignedCredentials.Memory) free(pInst->LoginD.SignedCredentials.Memory);
				if (!(pInst->LoginD.SignedCredentials.Memory = malloc(Response.Objs[Idx].Value.Memory.MsZ)))
				{
					FreeResponse(&Response);
					return -2;
				}
				memcpy (pInst->LoginD.SignedCredentials.Memory, Response.Objs[Idx].Value.Memory.Memory, 
					(pInst->LoginD.SignedCredentials.MsZ = Response.Objs[Idx].Value.Memory.MsZ));				
				break;
			}
		}
		FreeResponse(&Response);
	}

	return ret;
}


int PerformLogin(Skype_Inst *pInst, const char *User, const char *Pass)
{
	uint			ReUse = 1;
	int				i;
	LSConnection	conn={0};
	int				iRet = 0;

	for (i=0; !iRet && i<sizeof(LoginServers)/sizeof(LoginServers[0]); i++)
	{
		conn.LSSock = socket(AF_INET, SOCK_STREAM, 0);
		setsockopt(conn.LSSock, SOL_SOCKET, SO_REUSEADDR, (const char *)&ReUse, sizeof(ReUse));

		if (SendHandShake2LS(pInst, &conn, &LoginServers[i]))
		{
			pInst->pfLog(pInst->pLogStream, "Login Server %s OK ! Let's authenticate..\n", LoginServers[i].ip);
			iRet = SendAuthentificationBlobLS(pInst, &conn, User, Pass);
		}
		closesocket(conn.LSSock);
	}

	if (!iRet) pInst->pfLog(pInst->pLogStream, "Login Failed..\n");
	return iRet;
}


void InitInstance(Skype_Inst *pInst)
{
	memset(pInst, 0, sizeof(Skype_Inst));
	GenSessionKey(pInst->SessionKey, sizeof(pInst->SessionKey));
	InitNodeId(pInst);
	memcpy(pInst->Language, "en", 2);
	pInst->PublicIP = 0x7F000001;	// 127.0.0.1, we could use hostscan to get real IP, but not necessary for just login
#ifdef DEBUG
	pInst->pfLog = (int (__cdecl *)(void *, const char *, ...))fprintf;
	pInst->pLogStream = stdout;
#endif
}
