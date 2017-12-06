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
 * Module:       Credentials handling, reads/writes old Skype credentials format
 *
 */
#include "common.h"
#include "objects.h"
#include "random.h"
#include "crc.h"
#include "credentials.h"
#ifdef _WIN32
#define CRC_SIZE sizeof(short)
#else
#define CRC_SIZE 0
#endif


Memory_U Credentials_Write(Skype_Inst *pInst)
{
	Memory_U creds;
	uchar *Browser;
	uint Crc;

	creds.MsZ = sizeof(pInst->LoginD.LoginHash) + MODULUS_SZ + pInst->LoginD.SignedCredentials.MsZ + CRC_SIZE;
	if (!(Browser = creds.Memory = malloc(creds.MsZ)))
	{
		creds.MsZ = 0;
		return creds;
	}
	memcpy(Browser, pInst->LoginD.LoginHash, sizeof(pInst->LoginD.LoginHash));
	Browser+=sizeof(pInst->LoginD.LoginHash);
	BN_bn2bin(pInst->LoginD.RSAKeys->d, Browser);
	Browser+=MODULUS_SZ;
	memcpy(Browser, pInst->LoginD.SignedCredentials.Memory, pInst->LoginD.SignedCredentials.MsZ);
	Browser+=pInst->LoginD.SignedCredentials.MsZ;
#ifdef _WIN32
	Crc = crc32(creds.Memory, Browser-creds.Memory, -1);
	*Browser++ = *((uchar *)(&Crc) + 0);
	*Browser++ = *((uchar *)(&Crc) + 1);
#endif
	return creds;
}

/*
	 0	-	OK
	-1	-	Invalid size of credentials blob
	-2	-	Invalid CRC
	-3	-	Out of memory
	-4	-	Unable to parse Signed credentials
	-5	-	RSA public key not found
*/
int Credentials_Read(Skype_Inst *pInst, Memory_U creds, SResponse *LoginDatas)
{
	uchar *Browser;
	uint Crc, Idx;
	RSA	  *Keys;

	if (creds.MsZ < sizeof(pInst->LoginD.LoginHash) + MODULUS_SZ + 16 + CRC_SIZE)
		return -1;

#ifdef _WIN32
	Crc = crc32(creds.Memory, creds.MsZ-2, -1);
	if (*((uchar *)(&Crc) + 0) != *((uchar*)creds.Memory+creds.MsZ-2) ||
		*((uchar *)(&Crc) + 1) != *((uchar*)creds.Memory+creds.MsZ-1))
	{
		pInst->pfLog(pInst->pLogStream, "Credentials: Bad CRC!");
		return -2;
	}
#endif

	Browser = creds.Memory;
	memcpy (pInst->LoginD.LoginHash, Browser, sizeof(pInst->LoginD.LoginHash));
	Browser+=sizeof(pInst->LoginD.LoginHash);
	Keys=RSA_new();
	BN_hex2bn(&(Keys->e), "010001");
	Keys->d = BN_bin2bn(Browser, MODULUS_SZ, NULL);
	Browser+=MODULUS_SZ;
	if (pInst->LoginD.SignedCredentials.Memory) free(pInst->LoginD.SignedCredentials.Memory);
	pInst->LoginD.SignedCredentials.MsZ = creds.MsZ - CRC_SIZE - (Browser-creds.Memory);
	if (!(pInst->LoginD.SignedCredentials.Memory = malloc(pInst->LoginD.SignedCredentials.MsZ)))
	{
		pInst->LoginD.SignedCredentials.MsZ = 0;
		RSA_free(Keys);
		return -3;
	}
	memcpy(pInst->LoginD.SignedCredentials.Memory, Browser, pInst->LoginD.SignedCredentials.MsZ);

	// Now credentials are read, but we need to finish LoginD.RSAKeys by unpacking Signed Credentials
	if (Credentials_Parse(pInst, pInst->LoginD.SignedCredentials, LoginDatas)<0)
	{
		RSA_free(Keys);
		return -4;
	}

	for (Idx = 0; Idx < LoginDatas->NbObj; Idx++)
	{
		if (LoginDatas->Objs[Idx].Id == OBJ_ID_LDMODULUS)
		{
			Keys->n = BN_bin2bn(LoginDatas->Objs[Idx].Value.Memory.Memory, 
				LoginDatas->Objs[Idx].Value.Memory.MsZ, NULL);
			if (pInst->LoginD.RSAKeys) RSA_free(pInst->LoginD.RSAKeys);
			pInst->LoginD.RSAKeys = Keys;
			return 0;
		}
	}

	RSA_free(Keys);
	return -5;
}

int Credentials_Parse(Skype_Inst *pInst, Memory_U Pcred, SResponse *LoginDatas)
{
	uchar	*PostProcessed, *Browser;
	char	*Key;
	uint	KeyIdx, PPsZ;
	RSA		*SkypeRSA;
	Memory_U creds = Pcred;

	if (!(creds.Memory = malloc(creds.MsZ)))
		return -2;
	KeyIdx = htonl(*(uint *)Pcred.Memory);
	Pcred.Memory += sizeof(uint);
	Pcred.MsZ -= sizeof(uint);
	
	SkypeRSA = RSA_new();
	Key = KeySelect(KeyIdx);
	BN_hex2bn(&(SkypeRSA->n), Key);
	BN_hex2bn(&(SkypeRSA->e), "010001");
	PPsZ = RSA_public_decrypt(Pcred.MsZ, Pcred.Memory, creds.Memory, SkypeRSA, RSA_NO_PADDING);
	RSA_free(SkypeRSA);
	if (PPsZ == -1)
	{
		char szErr[256];

		ERR_load_crypto_strings();
		ERR_error_string(ERR_get_error(), szErr);
		pInst->pfLog(pInst->pLogStream, "Credentials decryption failed: %s\n", szErr);
		free(creds.Memory);
		return -1;
	}

	PostProcessed = FinalizeLoginDatas(pInst, creds.Memory, &PPsZ, NULL, 0);

	if (PostProcessed == NULL)
	{
		pInst->pfLog(pInst->pLogStream, "Bad Datas Finalization..\n");
		free (creds.Memory);
		return -1;
	}

	LoginDatas->Objs = NULL;
	LoginDatas->NbObj = 0;
	Browser = PostProcessed;

	ManageObjects(&Browser, PPsZ, LoginDatas);
	free(PostProcessed);
	free(creds.Memory);
	return 0;
}
