/*

dbx_tree: tree database driver for Miranda IM

Copyright 2007-2008 Michael "Protogenes" Kunz,

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "HC256.h"
#include <stdlib.h>
#include <string.h>

#define rotr(x,n) _lrotr(x,n)

#define h1(x,y) { \
	uint8_t a,b,c,d; \
	a = (uint8_t) (x); \
	b = (uint8_t) ((x) >> 8); \
	c = (uint8_t) ((x) >> 16); \
	d = (uint8_t) ((x) >> 24); \
	(y) = Q[a]+Q[256+b]+Q[512+c]+Q[768+d]; \
}
#define h2(x,y) { \
	uint8_t a,b,c,d; \
	a = (uint8_t) (x); \
	b = (uint8_t) ((x) >> 8); \
	c = (uint8_t) ((x) >> 16); \
	d = (uint8_t) ((x) >> 24); \
	(y) = P[a]+P[256+b]+P[512+c]+P[768+d]; \
}
#define step_A(u,v,a,b,c,d,m){ \
	uint32_t tem0,tem1,tem2,tem3; \
	tem0 = rotr((v),23); \
	tem1 = rotr((c),10); \
	tem2 = ((v) ^ (c)) & 0x3ff; \
	(u) += (b)+(tem0^tem1)+Q[tem2]; \
	(a) = (u); \
	h1((d),tem3); \
	(m) ^= tem3 ^ (u) ; \
}
#define step_B(u,v,a,b,c,d,m){ \
	uint32_t tem0,tem1,tem2,tem3; \
	tem0 = rotr((v),23); \
	tem1 = rotr((c),10); \
	tem2 = ((v) ^ (c)) & 0x3ff; \
	(u) += (b)+(tem0^tem1)+P[tem2]; \
	(a) = (u); \
	h2((d),tem3); \
	(m) ^= tem3 ^ (u) ; \
}


#define f1(x) (rotr((x),7) ^ rotr((x),18) ^ ((x) >> 3))
#define f2(x) (rotr((x),17) ^ rotr((x),19) ^ ((x) >> 10))
#define f(a,b,c,d) (f2((a)) + (b) + f1((c)) + (d))
#define feedback_1(u,v,b,c) { \
	uint32_t tem0,tem1,tem2; \
	tem0 = rotr((v),23); \
	tem1 = rotr((c),10); \
	tem2 = ((v) ^ (c)) & 0x3ff; \
	(u) += (b)+(tem0^tem1)+Q[tem2]; \
}
#define feedback_2(u,v,b,c) { \
	uint32_t tem0,tem1,tem2; \
	tem0 = rotr((v),23); \
	tem1 = rotr((c),10); \
	tem2 = ((v) ^ (c)) & 0x3ff; \
	(u) += (b)+(tem0^tem1)+P[tem2]; \
}

const wchar_t * HC256::Name()
{
	return cName;
}
const wchar_t * HC256::Description()
{
	return cDescription;
}
const uint32_t  HC256::BlockSizeBytes()
{
	return cBlockSizeBytes;
}
const bool      HC256::IsStreamCipher()
{
	return cIsStreamCipher;
}

HC256::HC256()
{

}
HC256::~HC256()
{

}
CCipher::TCipherInterface* HC256::Create() 
{
	return (new HC256())->m_Interface;
}

void HC256::SetKey(void* Key, uint32_t KeyLength)
{
	uint8_t k[32] = {0};

	for (uint32_t i = 0; i < KeyLength; ++i)
	{
		k[i & 0x1f] ^= ((uint8_t *)Key)[i];
	}
	CreateTables(k);
}
void HC256::Encrypt(void* Data, uint32_t Size, uint32_t Nonce, uint32_t StartByte)
{
	memcpy(X, BackX, sizeof(X));
	memcpy(Y, BackY, sizeof(Y));
	memcpy(P, BackP, sizeof(P));
	memcpy(Q, BackQ, sizeof(Q));

	counter2048 = (Nonce + (Nonce >> 11) + (Nonce >> 22)) & 0x7ff;

	for (uint32_t i = 0; i <= Size - BlockSizeBytes(); i += BlockSizeBytes())
	{
		EncryptBlock((uint32_t*)((uint8_t*)Data + i));
		StartByte += BlockSizeBytes();
	}
}
void HC256::Decrypt(void* Data, uint32_t Size, uint32_t Nonce, uint32_t StartByte)
{
	Encrypt(Data, Size, Nonce, StartByte);
}

inline void HC256::EncryptBlock(uint32_t *Data)
{
	uint32_t cc,dd;
	cc = counter2048 & 0x3ff;
	dd = (cc + 16) & 0x3ff;
	if (counter2048 < 1024)
	{
		counter2048 = (counter2048 + 16) & 0x7ff;
		step_A(P[cc+0], P[cc+1], X[0], X[6], X[13],X[4], Data[0]);
		step_A(P[cc+1], P[cc+2], X[1], X[7], X[14],X[5], Data[1]);
		step_A(P[cc+2], P[cc+3], X[2], X[8], X[15],X[6], Data[2]);
		step_A(P[cc+3], P[cc+4], X[3], X[9], X[0], X[7], Data[3]);
		step_A(P[cc+4], P[cc+5], X[4], X[10],X[1], X[8], Data[4]);
		step_A(P[cc+5], P[cc+6], X[5], X[11],X[2], X[9], Data[5]);
		step_A(P[cc+6], P[cc+7], X[6], X[12],X[3], X[10],Data[6]);
		step_A(P[cc+7], P[cc+8], X[7], X[13],X[4], X[11],Data[7]);
		step_A(P[cc+8], P[cc+9], X[8], X[14],X[5], X[12],Data[8]);
		step_A(P[cc+9], P[cc+10],X[9], X[15],X[6], X[13],Data[9]);
		step_A(P[cc+10],P[cc+11],X[10],X[0], X[7], X[14],Data[10]);
		step_A(P[cc+11],P[cc+12],X[11],X[1], X[8], X[15],Data[11]);
		step_A(P[cc+12],P[cc+13],X[12],X[2], X[9], X[0], Data[12]);
		step_A(P[cc+13],P[cc+14],X[13],X[3], X[10],X[1], Data[13]);
		step_A(P[cc+14],P[cc+15],X[14],X[4], X[11],X[2], Data[14]);
		step_A(P[cc+15],P[dd+0], X[15],X[5], X[12],X[3], Data[15]);

	}	else {

		counter2048 = (counter2048 + 16) & 0x7ff;
		step_B(Q[cc+0], Q[cc+1], Y[0], Y[6], Y[13],Y[4], Data[0]);
		step_B(Q[cc+1], Q[cc+2], Y[1], Y[7], Y[14],Y[5], Data[1]);
		step_B(Q[cc+2], Q[cc+3], Y[2], Y[8], Y[15],Y[6], Data[2]);
		step_B(Q[cc+3], Q[cc+4], Y[3], Y[9], Y[0], Y[7], Data[3]);
		step_B(Q[cc+4], Q[cc+5], Y[4], Y[10],Y[1], Y[8], Data[4]);
		step_B(Q[cc+5], Q[cc+6], Y[5], Y[11],Y[2], Y[9], Data[5]);
		step_B(Q[cc+6], Q[cc+7], Y[6], Y[12],Y[3], Y[10],Data[6]);
		step_B(Q[cc+7], Q[cc+8], Y[7], Y[13],Y[4], Y[11],Data[7]);
		step_B(Q[cc+8], Q[cc+9], Y[8], Y[14],Y[5], Y[12],Data[8]);
		step_B(Q[cc+9], Q[cc+10],Y[9], Y[15],Y[6], Y[13],Data[9]);
		step_B(Q[cc+10],Q[cc+11],Y[10],Y[0], Y[7], Y[14],Data[10]);
		step_B(Q[cc+11],Q[cc+12],Y[11],Y[1], Y[8], Y[15],Data[11]);
		step_B(Q[cc+12],Q[cc+13],Y[12],Y[2], Y[9], Y[0], Data[12]);
		step_B(Q[cc+13],Q[cc+14],Y[13],Y[3], Y[10],Y[1], Data[13]);
		step_B(Q[cc+14],Q[cc+15],Y[14],Y[4], Y[11],Y[2], Data[14]);
		step_B(Q[cc+15],Q[dd+0], Y[15],Y[5], Y[12],Y[3], Data[15]);
	}
}
inline void HC256::CreateTables(uint8_t* Key)
{
	uint32_t i, j;
	uint8_t iv[32] = "Miranda NG dbx_tree Protogenes!";
	//expand the key and iv into P and Q
	for (i = 0; i < 8; i++)
		P[i] = Key[i];
	for (i = 8; i < 16; i++)
		P[i] = iv[i - 8];
	for (i = 16; i < 528; i++)
		P[i] = f(P[i - 2], P[i - 7], P[i - 15], P[i - 16]) + i;
	for (i = 0; i < 16; i++)
		P[i] = P[i + 512];
	for (i = 16; i < 1024; i++)
		P[i] = f(P[i - 2], P[i - 7], P[i - 15], P[i - 16]) + 512 + i;
	for (i = 0; i < 16; i++)
		Q[i] = P[1024 - 16 + i];
	for (i = 16; i < 32; i++)
		Q[i] = f(Q[i - 2], Q[i - 7], Q[i - 15], Q[i - 16]) + 1520 + i;
	for (i = 0; i < 16; i++)
		Q[i] = Q[i + 16];
	for (i = 16; i < 1024;i++)
		Q[i] = f(Q[i - 2], Q[i - 7], Q[i - 15], Q[i - 16]) + 1536 + i;
	//run the cipher 4096 steps without generating output
	for (i = 0; i < 2; i++)
	{
		for (j = 0; j < 10; j++)
			feedback_1(P[j], P[j + 1], P[(j - 10) & 0x3ff], P[(j - 3) & 0x3ff]);
		for (j = 10; j < 1023; j++)
			feedback_1(P[j], P[j + 1], P[j - 10], P[j - 3]);
		feedback_1(P[1023], P[0], P[1013], P[1020]);
		for (j = 0; j < 10; j++)
			feedback_2(Q[j], Q[j+1], Q[(j-10) & 0x3ff], Q[(j - 3) & 0x3ff]);
		for (j = 10; j < 1023; j++)
			feedback_2(Q[j], Q[j + 1], Q[j - 10], Q[j - 3]);
		feedback_2(Q[1023], Q[0], Q[1013], Q[1020]);
	}
	//initialize counter2048, and tables X and Y
	counter2048 = 0;
	for (i = 0; i < 16; i++)
		X[i] = P[1008 + i];
	for (i = 0; i < 16; i++)
		Y[i] = Q[1008 + i];
}

extern "C" __declspec(dllexport) const TCipherInfo* CipherInfo(void * Reserved)
{
	return &HC256::cCipherInfo;
}
