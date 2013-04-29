/*
* FIPS-197 compliant AES implementation
*
* Copyright (C) 2001-2004 Christophe Devine
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "aes.h"

/* forward S-box & tables */

unsigned long int FSb[256];
unsigned long int FT0[256];
unsigned long int FT1[256];
unsigned long int FT2[256];
unsigned long int FT3[256];

/* reverse S-box & tables */

unsigned long int RSb[256];
unsigned long int RT0[256];
unsigned long int RT1[256];
unsigned long int RT2[256];
unsigned long int RT3[256];

/* round constants */

unsigned long int RCON[10];

/* tables generation flag */

int do_init = 1;

/* tables generation routine */

#define ROTR8(x) (((x << 24) & 0xFFFFFFFF) | ((x & 0xFFFFFFFF) >> 8))

#define XTIME(x) ((x << 1) ^ ((x & 0x80) ? 0x1B : 0x00))
#define MUL(x, y) ((x && y) ? pow[(log[x] + log[y]) % 255] : 0)

void aes_gen_tables(void)
{
	int i;
	unsigned char x, y;
	unsigned char pow[256];
	unsigned char log[256];

	/* compute pow and log tables over GF(2^8) */

	for (i = 0, x = 1; i < 256; i++, x ^= XTIME(x))
	{
		pow[i] = x;
		log[x] = i;
	}

	/* calculate the round constants */

	for (i = 0, x = 1; i < 10; i++, x = XTIME(x))
	{
		RCON[i] = (unsigned long int)x << 24;
	}

	/* generate the forward and reverse S-boxes */

	FSb[0x00] = 0x63;
	RSb[0x63] = 0x00;

	for (i = 1; i < 256; i++)
	{
		x = pow[255 - log[i]];

		y = x;
		y = (y << 1) | (y >> 7);
		x ^= y;
		y = (y << 1) | (y >> 7);
		x ^= y;
		y = (y << 1) | (y >> 7);
		x ^= y;
		y = (y << 1) | (y >> 7);
		x ^= y ^ 0x63;

		FSb[i] = x;
		RSb[x] = i;
	}

	/* generate the forward and reverse tables */

	for (i = 0; i < 256; i++)
	{
		x = (unsigned char)FSb[i];
		y = XTIME(x);

		FT0[i] = (unsigned long int)(x ^ y) ^
				 ((unsigned long int)x << 8) ^
				 ((unsigned long int)x << 16) ^
				 ((unsigned long int)y << 24);

		FT0[i] &= 0xFFFFFFFF;

		FT1[i] = ROTR8(FT0[i]);
		FT2[i] = ROTR8(FT1[i]);
		FT3[i] = ROTR8(FT2[i]);

		y = (unsigned char)RSb[i];

		RT0[i] = ((unsigned long int)MUL(0x0B, y)) ^
				 ((unsigned long int)MUL(0x0D, y) << 8) ^
				 ((unsigned long int)MUL(0x09, y) << 16) ^
				 ((unsigned long int)MUL(0x0E, y) << 24);

		RT0[i] &= 0xFFFFFFFF;

		RT1[i] = ROTR8(RT0[i]);
		RT2[i] = ROTR8(RT1[i]);
		RT3[i] = ROTR8(RT2[i]);
	}
}

/* platform-independant 32-bit integer manipulation macros */

#define GET_UINT32(n, b, i)							\
{													\
	(n) = ((unsigned long int)(b)[(i)] << 24)		\
		| ((unsigned long int)(b)[(i) + 1] << 16)	\
		| ((unsigned long int)(b)[(i) + 2] << 8)	\
		| ((unsigned long int)(b)[(i) + 3]);		\
}

#define PUT_UINT32(n, b, i)						\
{												\
	(b)[(i)] = (unsigned char)((n) >> 24);		\
	(b)[(i) + 1] = (unsigned char)((n) >> 16);	\
	(b)[(i) + 2] = (unsigned char)((n) >> 8);	\
	(b)[(i) + 3] = (unsigned char)((n));		\
}

/* decryption key schedule tables */

int KT_init = 1;

unsigned long int KT0[256];
unsigned long int KT1[256];
unsigned long int KT2[256];
unsigned long int KT3[256];

/* AES key scheduling routine */

int aes_set_key(aes_context *ctx, unsigned char *key, int nbits)
{
	int i;
	unsigned long int *RK, *SK;

	if (do_init)
	{
		aes_gen_tables();

		do_init = 0;
	}

	switch (nbits)
	{
		case 128: ctx->nr = 10; break;
		case 192: ctx->nr = 12; break;
		case 256: ctx->nr = 14; break;
		default : return 1;
	}

	RK = ctx->erk;

	for (i = 0; i < (nbits >> 5); i++)
	{
		GET_UINT32(RK[i], key, i * 4);
	}

	/* setup encryption round keys */

	switch (nbits)
	{
	case 128:
		for (i = 0; i < 10; i++, RK += 4)
		{
			RK[4] = RK[0] ^ RCON[i] ^
						(FSb[(unsigned char)(RK[3] >> 16)] << 24) ^
						(FSb[(unsigned char)(RK[3] >> 8)] << 16) ^
						(FSb[(unsigned char)(RK[3])] << 8) ^
						(FSb[(unsigned char)(RK[3] >> 24)]);

			RK[5] = RK[1] ^ RK[4];
			RK[6] = RK[2] ^ RK[5];
			RK[7] = RK[3] ^ RK[6];
		}
		break;

	case 192:
		for (i = 0; i < 8; i++, RK += 6)
		{
			RK[6] = RK[0] ^ RCON[i] ^
						(FSb[(unsigned char)(RK[5] >> 16)] << 24) ^
						(FSb[(unsigned char)(RK[5] >> 8)] << 16) ^
						(FSb[(unsigned char)(RK[5])] << 8) ^
						(FSb[(unsigned char)(RK[5] >> 24)]);

			RK[7] = RK[1] ^ RK[6];
			RK[8] = RK[2] ^ RK[7];
			RK[9] = RK[3] ^ RK[8];
			RK[10] = RK[4] ^ RK[9];
			RK[11] = RK[5] ^ RK[10];
		}
		break;

	case 256:
		for (i = 0; i < 7; i++, RK += 8)
		{
			RK[8] = RK[0] ^ RCON[i] ^
						(FSb[(unsigned char)(RK[7] >> 16)] << 24) ^
						(FSb[(unsigned char)(RK[7] >> 8)] << 16) ^
						(FSb[(unsigned char)(RK[7])] << 8) ^
						(FSb[(unsigned char)(RK[7] >> 24)]);

			RK[9] = RK[1] ^ RK[8];
			RK[10] = RK[2] ^ RK[9];
			RK[11] = RK[3] ^ RK[10];

			RK[12] = RK[4] ^
						(FSb[(unsigned char)(RK[11] >> 24)] << 24) ^
						(FSb[(unsigned char)(RK[11] >> 16)] << 16) ^
						(FSb[(unsigned char)(RK[11] >> 8)] << 8) ^
						(FSb[(unsigned char)(RK[11])]);

			RK[13] = RK[5] ^ RK[12];
			RK[14] = RK[6] ^ RK[13];
			RK[15] = RK[7] ^ RK[14];
		}
		break;
	}

	/* setup decryption round keys */

	if (KT_init)
	{
		for (i = 0; i < 256; i++)
		{
			KT0[i] = RT0[FSb[i]];
			KT1[i] = RT1[FSb[i]];
			KT2[i] = RT2[FSb[i]];
			KT3[i] = RT3[FSb[i]];
		}

		KT_init = 0;
	}

	SK = ctx->drk;

	*SK++ = *RK++;
	*SK++ = *RK++;
	*SK++ = *RK++;
	*SK++ = *RK++;

	for (i = 1; i < ctx->nr; i++)
	{
		RK -= 8;

		*SK++ = KT0[(unsigned char)(*RK >> 24)] ^
				KT1[(unsigned char)(*RK >> 16)] ^
				KT2[(unsigned char)(*RK >> 8)] ^
				KT3[(unsigned char)(*RK)];
		RK++;

		*SK++ = KT0[(unsigned char)(*RK >> 24)] ^
				KT1[(unsigned char)(*RK >> 16)] ^
				KT2[(unsigned char)(*RK >> 8)] ^
				KT3[(unsigned char)(*RK)];
		RK++;

		*SK++ = KT0[(unsigned char)(*RK >> 24)] ^
				KT1[(unsigned char)(*RK >> 16)] ^
				KT2[(unsigned char)(*RK >> 8)] ^
				KT3[(unsigned char)(*RK)];
		RK++;

		*SK++ = KT0[(unsigned char)(*RK >> 24)] ^
				KT1[(unsigned char)(*RK >> 16)] ^
				KT2[(unsigned char)(*RK >> 8)] ^
				KT3[(unsigned char)(*RK)];
		RK++;
	}

	RK -= 8;

	*SK++ = *RK++;
	*SK++ = *RK++;
	*SK++ = *RK++;
	*SK++ = *RK++;

	return 0;
}

/* AES 128-bit block encryption routine */

void aes_encrypt(aes_context *ctx, unsigned char input[16], unsigned char output[16])
{
	unsigned long int *RK, X0, X1, X2, X3, Y0, Y1, Y2, Y3;

	RK = ctx->erk;

	GET_UINT32(X0, input, 0); X0 ^= RK[0];
	GET_UINT32(X1, input, 4); X1 ^= RK[1];
	GET_UINT32(X2, input, 8); X2 ^= RK[2];
	GET_UINT32(X3, input, 12); X3 ^= RK[3];

#define AES_FROUND(X0, X1, X2, X3, Y0, Y1, Y2, Y3)	\
{													\
	RK += 4;										\
													\
	X0 = RK[0] ^ FT0[(unsigned char)(Y0 >> 24)] ^	\
				 FT1[(unsigned char)(Y1 >> 16)] ^	\
				 FT2[(unsigned char)(Y2 >> 8)] ^	\
				 FT3[(unsigned char)(Y3)];			\
													\
	X1 = RK[1] ^ FT0[(unsigned char)(Y1 >> 24)] ^	\
				 FT1[(unsigned char)(Y2 >> 16)] ^	\
				 FT2[(unsigned char)(Y3 >> 8)] ^	\
				 FT3[(unsigned char)(Y0)];			\
													\
	X2 = RK[2] ^ FT0[(unsigned char)(Y2 >> 24)] ^	\
				 FT1[(unsigned char)(Y3 >> 16)] ^	\
				 FT2[(unsigned char)(Y0 >> 8)] ^	\
				 FT3[(unsigned char)(Y1)];			\
													\
	X3 = RK[3] ^ FT0[(unsigned char)(Y3 >> 24)] ^	\
				 FT1[(unsigned char)(Y0 >> 16)] ^	\
				 FT2[(unsigned char)(Y1 >> 8)] ^	\
				 FT3[(unsigned char)(Y2)];			\
}

	AES_FROUND(Y0, Y1, Y2, Y3, X0, X1, X2, X3); /* round 1 */
	AES_FROUND(X0, X1, X2, X3, Y0, Y1, Y2, Y3); /* round 2 */
	AES_FROUND(Y0, Y1, Y2, Y3, X0, X1, X2, X3); /* round 3 */
	AES_FROUND(X0, X1, X2, X3, Y0, Y1, Y2, Y3); /* round 4 */
	AES_FROUND(Y0, Y1, Y2, Y3, X0, X1, X2, X3); /* round 5 */
	AES_FROUND(X0, X1, X2, X3, Y0, Y1, Y2, Y3); /* round 6 */
	AES_FROUND(Y0, Y1, Y2, Y3, X0, X1, X2, X3); /* round 7 */
	AES_FROUND(X0, X1, X2, X3, Y0, Y1, Y2, Y3); /* round 8 */
	AES_FROUND(Y0, Y1, Y2, Y3, X0, X1, X2, X3); /* round 9 */

	if (ctx->nr > 10)
	{
		AES_FROUND(X0, X1, X2, X3, Y0, Y1, Y2, Y3); /* round 10 */
		AES_FROUND(Y0, Y1, Y2, Y3, X0, X1, X2, X3); /* round 11 */
	}

	if (ctx->nr > 12)
	{
		AES_FROUND(X0, X1, X2, X3, Y0, Y1, Y2, Y3); /* round 12 */
		AES_FROUND(Y0, Y1, Y2, Y3, X0, X1, X2, X3); /* round 13 */
	}

	/* last round */

	RK += 4;

	X0 = RK[0] ^ (FSb[(unsigned char)(Y0 >> 24)] << 24) ^
				 (FSb[(unsigned char)(Y1 >> 16)] << 16) ^
				 (FSb[(unsigned char)(Y2 >> 8)] << 8) ^
				 (FSb[(unsigned char)(Y3)]);

	X1 = RK[1] ^ (FSb[(unsigned char)(Y1 >> 24)] << 24) ^
				 (FSb[(unsigned char)(Y2 >> 16)] << 16) ^
				 (FSb[(unsigned char)(Y3 >> 8)] << 8) ^
				 (FSb[(unsigned char)(Y0)]);

	X2 = RK[2] ^ (FSb[(unsigned char)(Y2 >> 24)] << 24) ^
				 (FSb[(unsigned char)(Y3 >> 16)] << 16) ^
				 (FSb[(unsigned char)(Y0 >> 8)] << 8) ^
				 (FSb[(unsigned char)(Y1)]);

	X3 = RK[3] ^ (FSb[(unsigned char)(Y3 >> 24)] << 24) ^
				 (FSb[(unsigned char)(Y0 >> 16)] << 16) ^
				 (FSb[(unsigned char)(Y1 >> 8)] << 8) ^
				 (FSb[(unsigned char)(Y2)]);

	PUT_UINT32(X0, output, 0);
	PUT_UINT32(X1, output, 4);
	PUT_UINT32(X2, output, 8);
	PUT_UINT32(X3, output, 12);
}

/* AES 128-bit block decryption routine */

void aes_decrypt(aes_context *ctx, unsigned char input[16], unsigned char output[16])
{
	unsigned long int *RK, X0, X1, X2, X3, Y0, Y1, Y2, Y3;

	RK = ctx->drk;

	GET_UINT32(X0, input, 0);
	X0 ^= RK[0];
	GET_UINT32(X1, input, 4);
	X1 ^= RK[1];
	GET_UINT32(X2, input, 8);
	X2 ^= RK[2];
	GET_UINT32(X3, input, 12);
	X3 ^= RK[3];

#define AES_RROUND(X0, X1, X2, X3, Y0, Y1, Y2, Y3)	\
{													\
	RK += 4;										\
													\
	X0 = RK[0] ^ RT0[(unsigned char)(Y0 >> 24)] ^	\
				 RT1[(unsigned char)(Y3 >> 16)] ^	\
				 RT2[(unsigned char)(Y2 >> 8)] ^	\
				 RT3[(unsigned char)(Y1)];			\
													\
	X1 = RK[1] ^ RT0[(unsigned char)(Y1 >> 24)] ^	\
				 RT1[(unsigned char)(Y0 >> 16)] ^	\
				 RT2[(unsigned char)(Y3 >> 8)] ^	\
				 RT3[(unsigned char)(Y2)];			\
													\
	X2 = RK[2] ^ RT0[(unsigned char)(Y2 >> 24)] ^	\
				 RT1[(unsigned char)(Y1 >> 16)] ^	\
				 RT2[(unsigned char)(Y0 >> 8)] ^	\
				 RT3[(unsigned char)(Y3)];			\
													\
	X3 = RK[3] ^ RT0[(unsigned char)(Y3 >> 24)] ^	\
				 RT1[(unsigned char)(Y2 >> 16)] ^	\
				 RT2[(unsigned char)(Y1 >> 8)] ^	\
				 RT3[(unsigned char)(Y0)];			\
}

	AES_RROUND(Y0, Y1, Y2, Y3, X0, X1, X2, X3); /* round 1 */
	AES_RROUND(X0, X1, X2, X3, Y0, Y1, Y2, Y3); /* round 2 */
	AES_RROUND(Y0, Y1, Y2, Y3, X0, X1, X2, X3); /* round 3 */
	AES_RROUND(X0, X1, X2, X3, Y0, Y1, Y2, Y3); /* round 4 */
	AES_RROUND(Y0, Y1, Y2, Y3, X0, X1, X2, X3); /* round 5 */
	AES_RROUND(X0, X1, X2, X3, Y0, Y1, Y2, Y3); /* round 6 */
	AES_RROUND(Y0, Y1, Y2, Y3, X0, X1, X2, X3); /* round 7 */
	AES_RROUND(X0, X1, X2, X3, Y0, Y1, Y2, Y3); /* round 8 */
	AES_RROUND(Y0, Y1, Y2, Y3, X0, X1, X2, X3); /* round 9 */

	if (ctx->nr > 10)
	{
		AES_RROUND(X0, X1, X2, X3, Y0, Y1, Y2, Y3); /* round 10 */
		AES_RROUND(Y0, Y1, Y2, Y3, X0, X1, X2, X3); /* round 11 */
	}

	if (ctx->nr > 12)
	{
		AES_RROUND(X0, X1, X2, X3, Y0, Y1, Y2, Y3); /* round 12 */
		AES_RROUND(Y0, Y1, Y2, Y3, X0, X1, X2, X3); /* round 13 */
	}

	/* last round */

	RK += 4;

	X0 = RK[0] ^ (RSb[(unsigned char)(Y0 >> 24)] << 24) ^
				 (RSb[(unsigned char)(Y3 >> 16)] << 16) ^
				 (RSb[(unsigned char)(Y2 >> 8)] << 8) ^
				 (RSb[(unsigned char)(Y1)]);

	X1 = RK[1] ^ (RSb[(unsigned char)(Y1 >> 24)] << 24) ^
				 (RSb[(unsigned char)(Y0 >> 16)] << 16) ^
				 (RSb[(unsigned char)(Y3 >> 8)] << 8) ^
				 (RSb[(unsigned char)(Y2)]);

	X2 = RK[2] ^ (RSb[(unsigned char)(Y2 >> 24)] << 24) ^
				 (RSb[(unsigned char)(Y1 >> 16)] << 16) ^
				 (RSb[(unsigned char)(Y0 >> 8)] << 8) ^
				 (RSb[(unsigned char)(Y3)]);

	X3 = RK[3] ^ (RSb[(unsigned char)(Y3 >> 24)] << 24) ^
				 (RSb[(unsigned char)(Y2 >> 16)] << 16) ^
				 (RSb[(unsigned char)(Y1 >> 8)] << 8) ^
				 (RSb[(unsigned char)(Y0)]);

	PUT_UINT32(X0, output, 0);
	PUT_UINT32(X1, output, 4);
	PUT_UINT32(X2, output, 8);
	PUT_UINT32(X3, output, 12);
}