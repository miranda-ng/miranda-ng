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
 * Module:       Data creation functions, i.e. random data
 *
 */
#include <string.h>
#include "common.h"
#include "random.h"
#include "platform.h"

#define skrand(x) ((x)*0x00010DCD+0x00004271)

unsigned int	BytesSHA1(uchar *Data, uint Length)
{
	uchar		Buffer[SHA_DIGEST_LENGTH];
	SHA_CTX		Context;

	SHA1_Init(&Context);
	SHA1_Update(&Context, Data, Length);
	SHA1_Final(Buffer, &Context);
	return *(unsigned int *)Buffer;
}

int64_t BytesSHA1I64(uchar *Data, uint Length)
{
	uchar		Buffer[SHA_DIGEST_LENGTH];
	SHA_CTX		Context;

	SHA1_Init(&Context);
	SHA1_Update(&Context, Data, Length);
	SHA1_Final(Buffer, &Context);
	return *(int64_t *)Buffer;
}

unsigned int	BytesRandom()
{
	uchar		Buffer[0x464];

	FillRndBuffer(Buffer);
	return BytesSHA1(Buffer, 0x464);
}

int64_t BytesRandomI64()
{
	uchar		Buffer[0x464];

	FillRndBuffer(Buffer);
	return BytesSHA1I64(Buffer, 0x464);
}

unsigned short		BytesRandomWord()
{
	unsigned short	RandomW;
	unsigned int	RandomDW;

	RandomDW = BytesRandom();
	RandomW = *(unsigned short *)&RandomDW;
	RandomW += 0;
	return (RandomW);
}

void	SpecialSHA(uchar *SessionKey, uint SkSz, uchar *SHAResult, uint ResSz)
{
	SHA_CTX		Context;
	uchar		Buffer[SHA_DIGEST_LENGTH];
	char		*Salts[] = {"\x00\x00\x00\x00", "\x00\x00\x00\x01"};
	uint		Idx = 0;

	if (ResSz > 40)
		return ;
	while (ResSz > 20)
	{
		SHA1_Init(&Context);
		SHA1_Update(&Context, Salts[Idx], 0x04);
		SHA1_Update(&Context, SessionKey, SkSz);
		SHA1_Final(Buffer, &Context);
		memcpy(SHAResult + (Idx * SHA_DIGEST_LENGTH), Buffer, SHA_DIGEST_LENGTH);
		Idx++;
		ResSz -= SHA_DIGEST_LENGTH;
	}

	SHA1_Init(&Context);
	SHA1_Update(&Context, Salts[Idx], 0x04);
	SHA1_Update(&Context, SessionKey, SkSz);
	SHA1_Final(Buffer, &Context);
	memcpy(SHAResult + (Idx * SHA_DIGEST_LENGTH), Buffer, ResSz);
}

void		BuildUnFinalizedDatas(uchar *Datas, uint Size, uchar *Result)
{
	uchar			*Mark;
	uint			Idx;
	SHA_CTX			MDCtx;

	Result[0x00] = 0x4B;
	for (Idx = 1; Idx < (0x80 - (Size + SHA_DIGEST_LENGTH) - 2); Idx++)
		Result[Idx] = 0xBB;
	Result[Idx++] = 0xBA;

	Mark = Result + Idx;

	memcpy(Result + Idx, Datas, Size);
	Idx += Size;

	SHA1_Init(&MDCtx);
	SHA1_Update(&MDCtx, Mark, Size);
	SHA1_Final(Result + Idx, &MDCtx);
	Idx += SHA_DIGEST_LENGTH;

	Result[Idx] = 0xBC;
}

uchar		*FinalizeLoginDatas(Skype_Inst *pInst, uchar *Buffer, uint *Size, uchar *Suite, int SuiteSz)
{
	int		Idx;
	uchar	*Result;
	SHA_CTX	Context;
	uchar	SHARes[SHA_DIGEST_LENGTH] = {0};

	Idx = 0;
	if (Buffer[*Size - 1] != 0xBC)
		return (NULL);
	if (SuiteSz)
	{
		if (*Buffer != 0x6A)
			return (NULL);
		*Size = 0x6A + SuiteSz;
		Idx += 1;
		goto Copy;
	}
	while ((Buffer[Idx] & 0x0F) == 0x0B)
		Idx++;
	if ((Buffer[Idx] & 0x0F) != 0x0A)
		return (NULL);
	Idx += 1;
	*Size = (*Size - 0x15) - Idx;

Copy:
	Result = (uchar *)malloc(*Size);
	memcpy(Result, Buffer + Idx, *Size - SuiteSz);
	if (SuiteSz) memcpy(Result + (*Size - SuiteSz), Suite, SuiteSz);

	SHA1_Init(&Context);
	SHA1_Update(&Context, Result, *Size);
	SHA1_Final(SHARes, &Context);

	if (strncmp((char *)SHARes, (char *)(Buffer + Idx + (*Size - SuiteSz)), SHA_DIGEST_LENGTH))
	{
		pInst->pfLog(pInst->pLogStream, "Bad SHA Digest for unencrypted Datas..\n");
		free(Result);
		return (NULL);
	}

	return (Result);
}

void			GenSessionKey(uchar *Buffer, uint Size)
{
	uint		Idx, Rander;

	Rander = BytesRandom();
	for (Idx = 0; Idx < Size; Idx++)
	{
		Rander = skrand(Rander);
		Buffer[Idx] = ((uchar *)&Rander)[sizeof(Rander) - 1];
		//Buffer[Idx] = (uchar)(Idx + 1);
	}
	Buffer[0] = 0x01;
}

