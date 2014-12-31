/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-15 Miranda NG project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#include "commonheaders.h"

bool getRandomBytes(BYTE *buf, size_t bufLen)
{
	// try to use Intel hardware randomizer first
	HCRYPTPROV hProvider = NULL;
	if (::CryptAcquireContext(&hProvider, NULL, _T("Intel Hardware Cryptographic Service Provider"), PROV_INTEL_SEC, 0) ||
		 ::CryptAcquireContext(&hProvider, NULL, MS_STRONG_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) ||
		 ::CryptAcquireContext(&hProvider, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
	{
		::CryptGenRandom(hProvider, DWORD(bufLen), buf);
		::CryptReleaseContext(hProvider, 0);
	}
	// no luck? try to use Windows NT RTL
	else {
		typedef BOOL(WINAPI *pfnGetRandom)(PVOID RandomBuffer, ULONG RandomBufferLength);
		pfnGetRandom fnGetRandom = (pfnGetRandom)GetProcAddress(GetModuleHandleA("advapi32.dll"), "SystemFunction036");
		if (fnGetRandom == NULL)
			return false;

		fnGetRandom(buf, DWORD(bufLen));
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct SHA256_CONTEXT
{
	UINT32  h0, h1, h2, h3, h4, h5, h6, h7;
	UINT32  nblocks;
	BYTE buf[64];
	int  count;
};

static void sha256_init(SHA256_CONTEXT *hd)
{
	hd->h0 = 0x6a09e667;
	hd->h1 = 0xbb67ae85;
	hd->h2 = 0x3c6ef372;
	hd->h3 = 0xa54ff53a;
	hd->h4 = 0x510e527f;
	hd->h5 = 0x9b05688c;
	hd->h6 = 0x1f83d9ab;
	hd->h7 = 0x5be0cd19;

	hd->nblocks = 0;
	hd->count = 0;
}

/*
Transform the message X which consists of 16 32-bit-words. See FIPS
180-2 for details.  */
#define ror(x,n) ( ((x) >> (n)) | ((x) << (32-(n))) )
#define S0(x) (ror ((x), 7) ^ ror ((x), 18) ^ ((x) >> 3))       /* (4.6) */
#define S1(x) (ror ((x), 17) ^ ror ((x), 19) ^ ((x) >> 10))     /* (4.7) */
#define R(a,b,c,d,e,f,g,h,k,w) do                                 \
{                                                       \
	t1 = (h)+Sum1((e)) + Cho((e), (f), (g)) + (k)+(w);  \
	t2 = Sum0((a)) + Maj((a), (b), (c));                    \
	h = g;                                                \
	g = f;                                                \
	f = e;                                                \
	e = d + t1;                                           \
	d = c;                                                \
	c = b;                                                \
	b = a;                                                \
	a = t1 + t2;                                          \
} while (0)

/* (4.2) same as SHA-1's F1.  */
static inline UINT32
Cho(UINT32 x, UINT32 y, UINT32 z)
{
	return (z ^ (x & (y ^ z)));
}

/* (4.3) same as SHA-1's F3 */
static inline UINT32
Maj(UINT32 x, UINT32 y, UINT32 z)
{
	return ((x & y) | (z & (x | y)));
}

/* (4.4) */
static inline UINT32 Sum0(UINT32 x)
{
	return (ror(x, 2) ^ ror(x, 13) ^ ror(x, 22));
}

/* (4.5) */
static inline UINT32
Sum1(UINT32 x)
{
	return (ror(x, 6) ^ ror(x, 11) ^ ror(x, 25));
}


static void transform(SHA256_CONTEXT *hd, const unsigned char *data)
{
	static const UINT32 K[64] = {
		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
		0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
		0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
		0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
		0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
		0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
		0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
		0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
		0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
		0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
		0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
		0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
		0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
	};

	UINT32 a, b, c, d, e, f, g, h, t1, t2;
	UINT32 x[16];
	UINT32 w[64];
	int i;

	a = hd->h0;
	b = hd->h1;
	c = hd->h2;
	d = hd->h3;
	e = hd->h4;
	f = hd->h5;
	g = hd->h6;
	h = hd->h7;

#ifdef WORDS_BIGENDIAN
	memcpy(x, data, 64);
#else
	{
		BYTE *p2;

		for (i = 0, p2 = (BYTE*)x; i < 16; i++, p2 += 4) {
			p2[3] = *data++;
			p2[2] = *data++;
			p2[1] = *data++;
			p2[0] = *data++;
		}
	}
#endif

	for (i = 0; i < 16; i++)
		w[i] = x[i];
	for (; i < 64; i++)
		w[i] = S1(w[i - 2]) + w[i - 7] + S0(w[i - 15]) + w[i - 16];

	for (i = 0; i < 64;)
	{
#if 0
		R(a, b, c, d, e, f, g, h, K[i], w[i]);
		i++;
#else
		t1 = h + Sum1(e) + Cho(e, f, g) + K[i] + w[i];
		t2 = Sum0(a) + Maj(a, b, c);
		d += t1;
		h = t1 + t2;

		t1 = g + Sum1(d) + Cho(d, e, f) + K[i + 1] + w[i + 1];
		t2 = Sum0(h) + Maj(h, a, b);
		c += t1;
		g = t1 + t2;

		t1 = f + Sum1(c) + Cho(c, d, e) + K[i + 2] + w[i + 2];
		t2 = Sum0(g) + Maj(g, h, a);
		b += t1;
		f = t1 + t2;

		t1 = e + Sum1(b) + Cho(b, c, d) + K[i + 3] + w[i + 3];
		t2 = Sum0(f) + Maj(f, g, h);
		a += t1;
		e = t1 + t2;

		t1 = d + Sum1(a) + Cho(a, b, c) + K[i + 4] + w[i + 4];
		t2 = Sum0(e) + Maj(e, f, g);
		h += t1;
		d = t1 + t2;

		t1 = c + Sum1(h) + Cho(h, a, b) + K[i + 5] + w[i + 5];
		t2 = Sum0(d) + Maj(d, e, f);
		g += t1;
		c = t1 + t2;

		t1 = b + Sum1(g) + Cho(g, h, a) + K[i + 6] + w[i + 6];
		t2 = Sum0(c) + Maj(c, d, e);
		f += t1;
		b = t1 + t2;

		t1 = a + Sum1(f) + Cho(f, g, h) + K[i + 7] + w[i + 7];
		t2 = Sum0(b) + Maj(b, c, d);
		e += t1;
		a = t1 + t2;

		i += 8;
#endif
	}

	hd->h0 += a;
	hd->h1 += b;
	hd->h2 += c;
	hd->h3 += d;
	hd->h4 += e;
	hd->h5 += f;
	hd->h6 += g;
	hd->h7 += h;
}
#undef S0
#undef S1
#undef R


/* Update the message digest with the contents of INBUF with length
INLEN.  */
static void sha256_write(SHA256_CONTEXT *hd, const void *inbuf_arg, size_t inlen)
{
	const unsigned char *inbuf = (const unsigned char *)inbuf_arg;

	if (hd->count == 64)
	{ /* flush the buffer */
		transform(hd, hd->buf);
		hd->count = 0;
		hd->nblocks++;
	}
	if (!inbuf)
		return;
	if (hd->count)
	{
		for (; inlen && hd->count < 64; inlen--)
			hd->buf[hd->count++] = *inbuf++;
		sha256_write(hd, NULL, 0);
		if (!inlen)
			return;
	}

	while (inlen >= 64)
	{
		transform(hd, inbuf);
		hd->count = 0;
		hd->nblocks++;
		inlen -= 64;
		inbuf += 64;
	}
	for (; inlen && hd->count < 64; inlen--)
		hd->buf[hd->count++] = *inbuf++;
}


/*
The routine finally terminates the computation and returns the
digest.  The handle is prepared for a new cycle, but adding bytes
to the handle will the destroy the returned buffer.  Returns: 32
bytes with the message the digest.  */
static void sha256_final(SHA256_CONTEXT *hd, BYTE tmpHash[32])
{
	UINT32 t, msb, lsb;

	sha256_write(hd, NULL, 0); /* flush */;

	t = hd->nblocks;
	/* multiply by 64 to make a BYTE count */
	lsb = t << 6;
	msb = t >> 26;
	/* add the count */
	t = lsb;
	if ((lsb += hd->count) < t)
		msb++;
	/* multiply by 8 to make a bit count */
	t = lsb;
	lsb <<= 3;
	msb <<= 3;
	msb |= t >> 29;

	if (hd->count < 56)
	{ /* enough room */
		hd->buf[hd->count++] = 0x80; /* pad */
		while (hd->count < 56)
			hd->buf[hd->count++] = 0;  /* pad */
	}
	else
	{ /* need one extra block */
		hd->buf[hd->count++] = 0x80; /* pad character */
		while (hd->count < 64)
			hd->buf[hd->count++] = 0;
		sha256_write(hd, NULL, 0);  /* flush */;
		memset(hd->buf, 0, 56); /* fill next block with zeroes */
	}
	/* append the 64 bit count */
	hd->buf[56] = msb >> 24;
	hd->buf[57] = msb >> 16;
	hd->buf[58] = msb >> 8;
	hd->buf[59] = msb;
	hd->buf[60] = lsb >> 24;
	hd->buf[61] = lsb >> 16;
	hd->buf[62] = lsb >> 8;
	hd->buf[63] = lsb;
	transform(hd, hd->buf);

	BYTE *p = tmpHash;
#ifdef WORDS_BIGENDIAN
#define X(a) do { *(UINT32*)p = hd->h##a ; p += 4; } while(0)
#else /* little endian */
#define X(a) do { *p++ = hd->h##a >> 24; *p++ = hd->h##a >> 16;	 \
	*p++ = hd->h##a >> 8; *p++ = hd->h##a; } while (0)
#endif
	X(0);
	X(1);
	X(2);
	X(3);
	X(4);
	X(5);
	X(6);
	X(7);
#undef X
}

static void make_sha256(const void *buf, size_t bufLen, BYTE *tmpHash)
{
	SHA256_CONTEXT tmp;
	sha256_init(&tmp);
	sha256_write(&tmp, buf, bufLen);
	sha256_final(&tmp, tmpHash);
}

void slow_hash(const void *buf, size_t bufLen, BYTE* tmpHash)
{
	make_sha256(buf, bufLen, tmpHash);

	for (int i = 0; i < 50000; i++)
		make_sha256(tmpHash, 32, tmpHash);
}
