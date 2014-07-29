/*
 * Copyright (C) 2001-2006 Jacek Sieka, arnetheduck on gmail point com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#if !defined(TIGER_HASH_H)
#define TIGER_HASH_H

#pragma once

typedef unsigned __int8 uint8_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;

#ifdef __GNUC__
#define _ULL(x) x##ull
#else
#define _ULL(x) x
#endif

class TigerHash {
public:
	/** Hash size in bytes */
	enum { HASH_SIZE = 24 };

	TigerHash() : pos(0) {
		res[0]=_ULL(0x0123456789ABCDEF);
		res[1]=_ULL(0xFEDCBA9876543210);
		res[2]=_ULL(0xF096A5B4C3B2E187);
	}

	~TigerHash() {
	}

	TCHAR* toBase32(TCHAR dst[((HASH_SIZE * 8) / 5) + 2]) {
		const TCHAR base32Alphabet[] = _T("ABCDEFGHIJKLMNOPQRSTUVWXYZ234567");
		uint8_t hash[HASH_SIZE];
		memcpy(hash, res, HASH_SIZE);

		// Code snagged from the bitzi bitcollider
		size_t i, index;
		uint8_t word;
		//wchar_t* dst = new wchar_t[];
		memset(dst, 0, sizeof(dst));

		int j = 0;
		for(i = 0, index = 0; i < HASH_SIZE; j++) {
			/* Is the current word going to span a byte boundary? */
			if (index > 3) {
				word = (uint8_t)(hash[i] & (0xFF >> index));
				index = (index + 5) % 8;
				word <<= index;
				if ((i + 1) < HASH_SIZE)
					word |= hash[i + 1] >> (8 - index);

				i++;
			} else {
				word = (uint8_t)(hash[i] >> (8 - (index + 5))) & 0x1F;
				index = (index + 5) % 8;
				if (index == 0)
					i++;
			}

			//dcassert(word < 32);
			dst[j] = base32Alphabet[word];
		}
		dst[39] = NULL;
		return dst;
	}

	/** Calculates the Tiger hash of the data. */
	void update(const void* data, size_t len);
	/** Call once all data has been processed. */
	uint8_t* finalize();

private:
	enum { BLOCK_SIZE = 512/8 };
	/** 512 bit blocks for the compress function */
	uint8_t tmp[512/8];
	/** State / final hash value */
	uint64_t res[3];
	/** Total number of bytes compressed */
	uint64_t pos;
	/** S boxes */
	static uint64_t table[];

	void tigerCompress(const uint64_t* data, uint64_t state[3]);
};

#endif // !defined(TIGER_HASH_H)

/**
 * @file
 * $Id: TigerHash.h,v 1.10 2006/10/13 20:04:32 bigmuscle Exp $
 */
