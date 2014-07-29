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

#include "stdafx.h"

#ifdef _WIN32
#if defined(_M_X64)
#define TIGER_ARCH64
#endif
#if !(defined(_M_IX86) || defined(_M_X64))
#define TIGER_BIG_ENDIAN
#endif
#else // _WIN32
#if defined(__x86_64__) || defined(__alpha)
#define TIGER_ARCH64
#endif
#if !(defined(__i386__) || defined(__x86_64__) || defined(__alpha))
#define TIGER_BIG_ENDIAN
#endif
#endif // _WIN32

#define PASSES 3

#define t1 (table)
#define t2 (table+256)
#define t3 (table+256*2)
#define t4 (table+256*3)

#define save_abc \
	aa = a; \
	bb = b; \
	cc = c;

#ifdef TIGER_ARCH64
#define round(a,b,c,x,mul) \
	c ^= x; \
	a -= t1[((c)>>(0*8))&0xFF] ^ t2[((c)>>(2*8))&0xFF] ^ \
	     t3[((c)>>(4*8))&0xFF] ^ t4[((c)>>(6*8))&0xFF] ; \
	b += t4[((c)>>(1*8))&0xFF] ^ t3[((c)>>(3*8))&0xFF] ^ \
	     t2[((c)>>(5*8))&0xFF] ^ t1[((c)>>(7*8))&0xFF] ; \
	b *= mul;
#else
#define round(a,b,c,x,mul) \
	c ^= x; \
	a -= t1[(uint8_t)(c)] ^ \
	t2[(uint8_t)(((uint32_t)(c))>>(2*8))] ^ \
	t3[(uint8_t)(((uint64_t)(c))>>(4*8))] ^ \
	t4[(uint8_t)(((uint32_t)(((uint64_t)(c))>>(4*8)))>>(2*8))] ; \
	b += t4[(uint8_t)(((uint32_t)(c))>>(1*8))] ^ \
	t3[(uint8_t)(((uint32_t)(c))>>(3*8))] ^ \
	t2[(uint8_t)(((uint32_t)(((uint64_t)(c))>>(4*8)))>>(1*8))] ^ \
	t1[(uint8_t)(((uint32_t)(((uint64_t)(c))>>(4*8)))>>(3*8))]; \
	b *= mul;
#endif

#define pass(a,b,c,mul) \
	round(a,b,c,x0,mul) \
	round(b,c,a,x1,mul) \
	round(c,a,b,x2,mul) \
	round(a,b,c,x3,mul) \
	round(b,c,a,x4,mul) \
	round(c,a,b,x5,mul) \
	round(a,b,c,x6,mul) \
	round(b,c,a,x7,mul)

#define key_schedule \
	x0 -= x7 ^ _ULL(0xA5A5A5A5A5A5A5A5); \
	x1 ^= x0; \
	x2 += x1; \
	x3 -= x2 ^ ((~x1)<<19); \
	x4 ^= x3; \
	x5 += x4; \
	x6 -= x5 ^ ((~x4)>>23); \
	x7 ^= x6; \
	x0 += x7; \
	x1 -= x0 ^ ((~x7)<<19); \
	x2 ^= x1; \
	x3 += x2; \
	x4 -= x3 ^ ((~x2)>>23); \
	x5 ^= x4; \
	x6 += x5; \
	x7 -= x6 ^ _ULL(0x0123456789ABCDEF);

#define feedforward \
	a ^= aa; \
	b -= bb; \
	c += cc;

#ifdef TIGER_ARCH64
#define compress \
	save_abc \
	pass(a,b,c,5) \
	key_schedule \
	pass(c,a,b,7) \
	key_schedule \
	pass(b,c,a,9) \
	for(pass_no=3; pass_no<PASSES; pass_no++) { \
	 key_schedule \
	 pass(a,b,c,9) \
	 tmpa=a; a=c; c=b; b=tmpa;} \
	feedforward
#else
#define compress \
	save_abc \
	for(pass_no=0; pass_no<PASSES; pass_no++) { \
	if(pass_no != 0) {key_schedule} \
	pass(a,b,c,(pass_no==0?5:pass_no==1?7:9)); \
	tmpa=a; a=c; c=b; b=tmpa;} \
	feedforward
#endif

#define tiger_compress_macro(str, state) \
{ \
	register uint64_t a, b, c, tmpa; \
	uint64_t aa, bb, cc; \
	register uint64_t x0, x1, x2, x3, x4, x5, x6, x7; \
	int pass_no; \
	\
	a = state[0]; \
	b = state[1]; \
	c = state[2]; \
	\
	x0=str[0]; x1=str[1]; x2=str[2]; x3=str[3]; \
	x4=str[4]; x5=str[5]; x6=str[6]; x7=str[7]; \
	\
	compress; \
	\
	state[0] = a; \
	state[1] = b; \
	state[2] = c; \
}

/* The compress function is a function. Requires smaller cache?    */
void TigerHash::tigerCompress(const uint64_t *str, uint64_t state[3]) {
	tiger_compress_macro(((const uint64_t*)str), ((uint64_t*)state));
}

void TigerHash::update(const void* data, size_t length) {
	size_t tmppos = (uint32_t)(pos & BLOCK_SIZE-1);
#ifdef TIGER_BIG_ENDIAN
	uint8_t buf[BLOCK_SIZE];
	int j;
#endif
	const uint8_t* str = (const uint8_t*)data;
	// First empty tmp buffer if possible
	if(tmppos > 0) {
		size_t n = min(length, BLOCK_SIZE-tmppos);
		memcpy(tmp + tmppos, str, n);
		str += n;
		pos += n;
		length -= n;

		if ((tmppos + n) == BLOCK_SIZE) {
#ifdef TIGER_BIG_ENDIAN
			for(j=0; j<BLOCK_SIZE; j++)
			        buf[j^7]=((uint8_t*)tmp)[j];
			tiger_compress_macro(((uint64_t*)buf), res);
#else
			tiger_compress_macro(((uint64_t*)tmp), res);
#endif
			tmppos = 0;
		}
	}

	// So, now either tmp is empty or all data has been consumed...
	//ASSERT(length == 0 || tmppos == 0);

	// Process the bulk of data
	while(length>=BLOCK_SIZE) {
#ifdef TIGER_BIG_ENDIAN
		for(j=0; j<BLOCK_SIZE; j++)
		        buf[j^7]=((uint8_t*)str)[j];
		tiger_compress_macro(((uint64_t*)buf), res);
#else
		tiger_compress_macro(((uint64_t*)str), res);
#endif
		str += BLOCK_SIZE;
		pos += BLOCK_SIZE;
		length -= BLOCK_SIZE;
	}

	// Copy the rest to the tmp buffer
	memcpy(tmp, str, length);
	pos += length;
}

uint8_t* TigerHash::finalize() {
	size_t tmppos = (size_t)(pos & BLOCK_SIZE-1);
#ifdef TIGER_BIG_ENDIAN
	uint8_t buf[BLOCK_SIZE];
	int j;
#endif
	// Tmp buffer always has at least one pos, otherwise it would have
	// been processed in update()

	tmp[tmppos++] = 0x01;

	if(tmppos > (BLOCK_SIZE - sizeof(uint64_t))) {
		memset(tmp + tmppos, 0, BLOCK_SIZE - tmppos);
#ifdef TIGER_BIG_ENDIAN
		for(j=0; j<BLOCK_SIZE; j++)
		        buf[j^7]=((uint8_t*)tmp)[j];
		tiger_compress_macro(((uint64_t*)buf), res);
#else
		tiger_compress_macro(((uint64_t*)tmp), res);
#endif
		memset(tmp, 0, BLOCK_SIZE);
	} else {
		memset(tmp + tmppos, 0, BLOCK_SIZE - tmppos - sizeof(uint64_t));
#ifdef TIGER_BIG_ENDIAN
		for(j=0; j<BLOCK_SIZE; j++)
		        buf[j^7]=((uint8_t*)tmp)[j];
		memcpy(tmp, buf, BLOCK_SIZE);
#endif
	}

	((uint64_t*)(&(tmp[56])))[0] = pos<<3;
	tiger_compress_macro(((uint64_t*)tmp), res);
#ifdef TIGER_BIG_ENDIAN
	for(j=0; j<HASH_SIZE; j++)
	        buf[j^7]=((uint8_t*)res)[j];
	memcpy(res, buf, HASH_SIZE);
#endif
	return (uint8_t*) res;
}

uint64_t TigerHash::table[4*256] = {
	_ULL(0x02AAB17CF7E90C5E)   /*    0 */,    _ULL(0xAC424B03E243A8EC)   /*    1 */,
		_ULL(0x72CD5BE30DD5FCD3)   /*    2 */,    _ULL(0x6D019B93F6F97F3A)   /*    3 */,
		_ULL(0xCD9978FFD21F9193)   /*    4 */,    _ULL(0x7573A1C9708029E2)   /*    5 */,
		_ULL(0xB164326B922A83C3)   /*    6 */,    _ULL(0x46883EEE04915870)   /*    7 */,
		_ULL(0xEAACE3057103ECE6)   /*    8 */,    _ULL(0xC54169B808A3535C)   /*    9 */,
		_ULL(0x4CE754918DDEC47C)   /*   10 */,    _ULL(0x0AA2F4DFDC0DF40C)   /*   11 */,
		_ULL(0x10B76F18A74DBEFA)   /*   12 */,    _ULL(0xC6CCB6235AD1AB6A)   /*   13 */,
		_ULL(0x13726121572FE2FF)   /*   14 */,    _ULL(0x1A488C6F199D921E)   /*   15 */,
		_ULL(0x4BC9F9F4DA0007CA)   /*   16 */,    _ULL(0x26F5E6F6E85241C7)   /*   17 */,
		_ULL(0x859079DBEA5947B6)   /*   18 */,    _ULL(0x4F1885C5C99E8C92)   /*   19 */,
		_ULL(0xD78E761EA96F864B)   /*   20 */,    _ULL(0x8E36428C52B5C17D)   /*   21 */,
		_ULL(0x69CF6827373063C1)   /*   22 */,    _ULL(0xB607C93D9BB4C56E)   /*   23 */,
		_ULL(0x7D820E760E76B5EA)   /*   24 */,    _ULL(0x645C9CC6F07FDC42)   /*   25 */,
		_ULL(0xBF38A078243342E0)   /*   26 */,    _ULL(0x5F6B343C9D2E7D04)   /*   27 */,
		_ULL(0xF2C28AEB600B0EC6)   /*   28 */,    _ULL(0x6C0ED85F7254BCAC)   /*   29 */,
		_ULL(0x71592281A4DB4FE5)   /*   30 */,    _ULL(0x1967FA69CE0FED9F)   /*   31 */,
		_ULL(0xFD5293F8B96545DB)   /*   32 */,    _ULL(0xC879E9D7F2A7600B)   /*   33 */,
		_ULL(0x860248920193194E)   /*   34 */,    _ULL(0xA4F9533B2D9CC0B3)   /*   35 */,
		_ULL(0x9053836C15957613)   /*   36 */,    _ULL(0xDB6DCF8AFC357BF1)   /*   37 */,
		_ULL(0x18BEEA7A7A370F57)   /*   38 */,    _ULL(0x037117CA50B99066)   /*   39 */,
		_ULL(0x6AB30A9774424A35)   /*   40 */,    _ULL(0xF4E92F02E325249B)   /*   41 */,
		_ULL(0x7739DB07061CCAE1)   /*   42 */,    _ULL(0xD8F3B49CECA42A05)   /*   43 */,
		_ULL(0xBD56BE3F51382F73)   /*   44 */,    _ULL(0x45FAED5843B0BB28)   /*   45 */,
		_ULL(0x1C813D5C11BF1F83)   /*   46 */,    _ULL(0x8AF0E4B6D75FA169)   /*   47 */,
		_ULL(0x33EE18A487AD9999)   /*   48 */,    _ULL(0x3C26E8EAB1C94410)   /*   49 */,
		_ULL(0xB510102BC0A822F9)   /*   50 */,    _ULL(0x141EEF310CE6123B)   /*   51 */,
		_ULL(0xFC65B90059DDB154)   /*   52 */,    _ULL(0xE0158640C5E0E607)   /*   53 */,
		_ULL(0x884E079826C3A3CF)   /*   54 */,    _ULL(0x930D0D9523C535FD)   /*   55 */,
		_ULL(0x35638D754E9A2B00)   /*   56 */,    _ULL(0x4085FCCF40469DD5)   /*   57 */,
		_ULL(0xC4B17AD28BE23A4C)   /*   58 */,    _ULL(0xCAB2F0FC6A3E6A2E)   /*   59 */,
		_ULL(0x2860971A6B943FCD)   /*   60 */,    _ULL(0x3DDE6EE212E30446)   /*   61 */,
		_ULL(0x6222F32AE01765AE)   /*   62 */,    _ULL(0x5D550BB5478308FE)   /*   63 */,
		_ULL(0xA9EFA98DA0EDA22A)   /*   64 */,    _ULL(0xC351A71686C40DA7)   /*   65 */,
		_ULL(0x1105586D9C867C84)   /*   66 */,    _ULL(0xDCFFEE85FDA22853)   /*   67 */,
		_ULL(0xCCFBD0262C5EEF76)   /*   68 */,    _ULL(0xBAF294CB8990D201)   /*   69 */,
		_ULL(0xE69464F52AFAD975)   /*   70 */,    _ULL(0x94B013AFDF133E14)   /*   71 */,
		_ULL(0x06A7D1A32823C958)   /*   72 */,    _ULL(0x6F95FE5130F61119)   /*   73 */,
		_ULL(0xD92AB34E462C06C0)   /*   74 */,    _ULL(0xED7BDE33887C71D2)   /*   75 */,
		_ULL(0x79746D6E6518393E)   /*   76 */,    _ULL(0x5BA419385D713329)   /*   77 */,
		_ULL(0x7C1BA6B948A97564)   /*   78 */,    _ULL(0x31987C197BFDAC67)   /*   79 */,
		_ULL(0xDE6C23C44B053D02)   /*   80 */,    _ULL(0x581C49FED002D64D)   /*   81 */,
		_ULL(0xDD474D6338261571)   /*   82 */,    _ULL(0xAA4546C3E473D062)   /*   83 */,
		_ULL(0x928FCE349455F860)   /*   84 */,    _ULL(0x48161BBACAAB94D9)   /*   85 */,
		_ULL(0x63912430770E6F68)   /*   86 */,    _ULL(0x6EC8A5E602C6641C)   /*   87 */,
		_ULL(0x87282515337DDD2B)   /*   88 */,    _ULL(0x2CDA6B42034B701B)   /*   89 */,
		_ULL(0xB03D37C181CB096D)   /*   90 */,    _ULL(0xE108438266C71C6F)   /*   91 */,
		_ULL(0x2B3180C7EB51B255)   /*   92 */,    _ULL(0xDF92B82F96C08BBC)   /*   93 */,
		_ULL(0x5C68C8C0A632F3BA)   /*   94 */,    _ULL(0x5504CC861C3D0556)   /*   95 */,
		_ULL(0xABBFA4E55FB26B8F)   /*   96 */,    _ULL(0x41848B0AB3BACEB4)   /*   97 */,
		_ULL(0xB334A273AA445D32)   /*   98 */,    _ULL(0xBCA696F0A85AD881)   /*   99 */,
		_ULL(0x24F6EC65B528D56C)   /*  100 */,    _ULL(0x0CE1512E90F4524A)   /*  101 */,
		_ULL(0x4E9DD79D5506D35A)   /*  102 */,    _ULL(0x258905FAC6CE9779)   /*  103 */,
		_ULL(0x2019295B3E109B33)   /*  104 */,    _ULL(0xF8A9478B73A054CC)   /*  105 */,
		_ULL(0x2924F2F934417EB0)   /*  106 */,    _ULL(0x3993357D536D1BC4)   /*  107 */,
		_ULL(0x38A81AC21DB6FF8B)   /*  108 */,    _ULL(0x47C4FBF17D6016BF)   /*  109 */,
		_ULL(0x1E0FAADD7667E3F5)   /*  110 */,    _ULL(0x7ABCFF62938BEB96)   /*  111 */,
		_ULL(0xA78DAD948FC179C9)   /*  112 */,    _ULL(0x8F1F98B72911E50D)   /*  113 */,
		_ULL(0x61E48EAE27121A91)   /*  114 */,    _ULL(0x4D62F7AD31859808)   /*  115 */,
		_ULL(0xECEBA345EF5CEAEB)   /*  116 */,    _ULL(0xF5CEB25EBC9684CE)   /*  117 */,
		_ULL(0xF633E20CB7F76221)   /*  118 */,    _ULL(0xA32CDF06AB8293E4)   /*  119 */,
		_ULL(0x985A202CA5EE2CA4)   /*  120 */,    _ULL(0xCF0B8447CC8A8FB1)   /*  121 */,
		_ULL(0x9F765244979859A3)   /*  122 */,    _ULL(0xA8D516B1A1240017)   /*  123 */,
		_ULL(0x0BD7BA3EBB5DC726)   /*  124 */,    _ULL(0xE54BCA55B86ADB39)   /*  125 */,
		_ULL(0x1D7A3AFD6C478063)   /*  126 */,    _ULL(0x519EC608E7669EDD)   /*  127 */,
		_ULL(0x0E5715A2D149AA23)   /*  128 */,    _ULL(0x177D4571848FF194)   /*  129 */,
		_ULL(0xEEB55F3241014C22)   /*  130 */,    _ULL(0x0F5E5CA13A6E2EC2)   /*  131 */,
		_ULL(0x8029927B75F5C361)   /*  132 */,    _ULL(0xAD139FABC3D6E436)   /*  133 */,
		_ULL(0x0D5DF1A94CCF402F)   /*  134 */,    _ULL(0x3E8BD948BEA5DFC8)   /*  135 */,
		_ULL(0xA5A0D357BD3FF77E)   /*  136 */,    _ULL(0xA2D12E251F74F645)   /*  137 */,
		_ULL(0x66FD9E525E81A082)   /*  138 */,    _ULL(0x2E0C90CE7F687A49)   /*  139 */,
		_ULL(0xC2E8BCBEBA973BC5)   /*  140 */,    _ULL(0x000001BCE509745F)   /*  141 */,
		_ULL(0x423777BBE6DAB3D6)   /*  142 */,    _ULL(0xD1661C7EAEF06EB5)   /*  143 */,
		_ULL(0xA1781F354DAACFD8)   /*  144 */,    _ULL(0x2D11284A2B16AFFC)   /*  145 */,
		_ULL(0xF1FC4F67FA891D1F)   /*  146 */,    _ULL(0x73ECC25DCB920ADA)   /*  147 */,
		_ULL(0xAE610C22C2A12651)   /*  148 */,    _ULL(0x96E0A810D356B78A)   /*  149 */,
		_ULL(0x5A9A381F2FE7870F)   /*  150 */,    _ULL(0xD5AD62EDE94E5530)   /*  151 */,
		_ULL(0xD225E5E8368D1427)   /*  152 */,    _ULL(0x65977B70C7AF4631)   /*  153 */,
		_ULL(0x99F889B2DE39D74F)   /*  154 */,    _ULL(0x233F30BF54E1D143)   /*  155 */,
		_ULL(0x9A9675D3D9A63C97)   /*  156 */,    _ULL(0x5470554FF334F9A8)   /*  157 */,
		_ULL(0x166ACB744A4F5688)   /*  158 */,    _ULL(0x70C74CAAB2E4AEAD)   /*  159 */,
		_ULL(0xF0D091646F294D12)   /*  160 */,    _ULL(0x57B82A89684031D1)   /*  161 */,
		_ULL(0xEFD95A5A61BE0B6B)   /*  162 */,    _ULL(0x2FBD12E969F2F29A)   /*  163 */,
		_ULL(0x9BD37013FEFF9FE8)   /*  164 */,    _ULL(0x3F9B0404D6085A06)   /*  165 */,
		_ULL(0x4940C1F3166CFE15)   /*  166 */,    _ULL(0x09542C4DCDF3DEFB)   /*  167 */,
		_ULL(0xB4C5218385CD5CE3)   /*  168 */,    _ULL(0xC935B7DC4462A641)   /*  169 */,
		_ULL(0x3417F8A68ED3B63F)   /*  170 */,    _ULL(0xB80959295B215B40)   /*  171 */,
		_ULL(0xF99CDAEF3B8C8572)   /*  172 */,    _ULL(0x018C0614F8FCB95D)   /*  173 */,
		_ULL(0x1B14ACCD1A3ACDF3)   /*  174 */,    _ULL(0x84D471F200BB732D)   /*  175 */,
		_ULL(0xC1A3110E95E8DA16)   /*  176 */,    _ULL(0x430A7220BF1A82B8)   /*  177 */,
		_ULL(0xB77E090D39DF210E)   /*  178 */,    _ULL(0x5EF4BD9F3CD05E9D)   /*  179 */,
		_ULL(0x9D4FF6DA7E57A444)   /*  180 */,    _ULL(0xDA1D60E183D4A5F8)   /*  181 */,
		_ULL(0xB287C38417998E47)   /*  182 */,    _ULL(0xFE3EDC121BB31886)   /*  183 */,
		_ULL(0xC7FE3CCC980CCBEF)   /*  184 */,    _ULL(0xE46FB590189BFD03)   /*  185 */,
		_ULL(0x3732FD469A4C57DC)   /*  186 */,    _ULL(0x7EF700A07CF1AD65)   /*  187 */,
		_ULL(0x59C64468A31D8859)   /*  188 */,    _ULL(0x762FB0B4D45B61F6)   /*  189 */,
		_ULL(0x155BAED099047718)   /*  190 */,    _ULL(0x68755E4C3D50BAA6)   /*  191 */,
		_ULL(0xE9214E7F22D8B4DF)   /*  192 */,    _ULL(0x2ADDBF532EAC95F4)   /*  193 */,
		_ULL(0x32AE3909B4BD0109)   /*  194 */,    _ULL(0x834DF537B08E3450)   /*  195 */,
		_ULL(0xFA209DA84220728D)   /*  196 */,    _ULL(0x9E691D9B9EFE23F7)   /*  197 */,
		_ULL(0x0446D288C4AE8D7F)   /*  198 */,    _ULL(0x7B4CC524E169785B)   /*  199 */,
		_ULL(0x21D87F0135CA1385)   /*  200 */,    _ULL(0xCEBB400F137B8AA5)   /*  201 */,
		_ULL(0x272E2B66580796BE)   /*  202 */,    _ULL(0x3612264125C2B0DE)   /*  203 */,
		_ULL(0x057702BDAD1EFBB2)   /*  204 */,    _ULL(0xD4BABB8EACF84BE9)   /*  205 */,
		_ULL(0x91583139641BC67B)   /*  206 */,    _ULL(0x8BDC2DE08036E024)   /*  207 */,
		_ULL(0x603C8156F49F68ED)   /*  208 */,    _ULL(0xF7D236F7DBEF5111)   /*  209 */,
		_ULL(0x9727C4598AD21E80)   /*  210 */,    _ULL(0xA08A0896670A5FD7)   /*  211 */,
		_ULL(0xCB4A8F4309EBA9CB)   /*  212 */,    _ULL(0x81AF564B0F7036A1)   /*  213 */,
		_ULL(0xC0B99AA778199ABD)   /*  214 */,    _ULL(0x959F1EC83FC8E952)   /*  215 */,
		_ULL(0x8C505077794A81B9)   /*  216 */,    _ULL(0x3ACAAF8F056338F0)   /*  217 */,
		_ULL(0x07B43F50627A6778)   /*  218 */,    _ULL(0x4A44AB49F5ECCC77)   /*  219 */,
		_ULL(0x3BC3D6E4B679EE98)   /*  220 */,    _ULL(0x9CC0D4D1CF14108C)   /*  221 */,
		_ULL(0x4406C00B206BC8A0)   /*  222 */,    _ULL(0x82A18854C8D72D89)   /*  223 */,
		_ULL(0x67E366B35C3C432C)   /*  224 */,    _ULL(0xB923DD61102B37F2)   /*  225 */,
		_ULL(0x56AB2779D884271D)   /*  226 */,    _ULL(0xBE83E1B0FF1525AF)   /*  227 */,
		_ULL(0xFB7C65D4217E49A9)   /*  228 */,    _ULL(0x6BDBE0E76D48E7D4)   /*  229 */,
		_ULL(0x08DF828745D9179E)   /*  230 */,    _ULL(0x22EA6A9ADD53BD34)   /*  231 */,
		_ULL(0xE36E141C5622200A)   /*  232 */,    _ULL(0x7F805D1B8CB750EE)   /*  233 */,
		_ULL(0xAFE5C7A59F58E837)   /*  234 */,    _ULL(0xE27F996A4FB1C23C)   /*  235 */,
		_ULL(0xD3867DFB0775F0D0)   /*  236 */,    _ULL(0xD0E673DE6E88891A)   /*  237 */,
		_ULL(0x123AEB9EAFB86C25)   /*  238 */,    _ULL(0x30F1D5D5C145B895)   /*  239 */,
		_ULL(0xBB434A2DEE7269E7)   /*  240 */,    _ULL(0x78CB67ECF931FA38)   /*  241 */,
		_ULL(0xF33B0372323BBF9C)   /*  242 */,    _ULL(0x52D66336FB279C74)   /*  243 */,
		_ULL(0x505F33AC0AFB4EAA)   /*  244 */,    _ULL(0xE8A5CD99A2CCE187)   /*  245 */,
		_ULL(0x534974801E2D30BB)   /*  246 */,    _ULL(0x8D2D5711D5876D90)   /*  247 */,
		_ULL(0x1F1A412891BC038E)   /*  248 */,    _ULL(0xD6E2E71D82E56648)   /*  249 */,
		_ULL(0x74036C3A497732B7)   /*  250 */,    _ULL(0x89B67ED96361F5AB)   /*  251 */,
		_ULL(0xFFED95D8F1EA02A2)   /*  252 */,    _ULL(0xE72B3BD61464D43D)   /*  253 */,
		_ULL(0xA6300F170BDC4820)   /*  254 */,    _ULL(0xEBC18760ED78A77A)   /*  255 */,
		_ULL(0xE6A6BE5A05A12138)   /*  256 */,    _ULL(0xB5A122A5B4F87C98)   /*  257 */,
		_ULL(0x563C6089140B6990)   /*  258 */,    _ULL(0x4C46CB2E391F5DD5)   /*  259 */,
		_ULL(0xD932ADDBC9B79434)   /*  260 */,    _ULL(0x08EA70E42015AFF5)   /*  261 */,
		_ULL(0xD765A6673E478CF1)   /*  262 */,    _ULL(0xC4FB757EAB278D99)   /*  263 */,
		_ULL(0xDF11C6862D6E0692)   /*  264 */,    _ULL(0xDDEB84F10D7F3B16)   /*  265 */,
		_ULL(0x6F2EF604A665EA04)   /*  266 */,    _ULL(0x4A8E0F0FF0E0DFB3)   /*  267 */,
		_ULL(0xA5EDEEF83DBCBA51)   /*  268 */,    _ULL(0xFC4F0A2A0EA4371E)   /*  269 */,
		_ULL(0xE83E1DA85CB38429)   /*  270 */,    _ULL(0xDC8FF882BA1B1CE2)   /*  271 */,
		_ULL(0xCD45505E8353E80D)   /*  272 */,    _ULL(0x18D19A00D4DB0717)   /*  273 */,
		_ULL(0x34A0CFEDA5F38101)   /*  274 */,    _ULL(0x0BE77E518887CAF2)   /*  275 */,
		_ULL(0x1E341438B3C45136)   /*  276 */,    _ULL(0xE05797F49089CCF9)   /*  277 */,
		_ULL(0xFFD23F9DF2591D14)   /*  278 */,    _ULL(0x543DDA228595C5CD)   /*  279 */,
		_ULL(0x661F81FD99052A33)   /*  280 */,    _ULL(0x8736E641DB0F7B76)   /*  281 */,
		_ULL(0x15227725418E5307)   /*  282 */,    _ULL(0xE25F7F46162EB2FA)   /*  283 */,
		_ULL(0x48A8B2126C13D9FE)   /*  284 */,    _ULL(0xAFDC541792E76EEA)   /*  285 */,
		_ULL(0x03D912BFC6D1898F)   /*  286 */,    _ULL(0x31B1AAFA1B83F51B)   /*  287 */,
		_ULL(0xF1AC2796E42AB7D9)   /*  288 */,    _ULL(0x40A3A7D7FCD2EBAC)   /*  289 */,
		_ULL(0x1056136D0AFBBCC5)   /*  290 */,    _ULL(0x7889E1DD9A6D0C85)   /*  291 */,
		_ULL(0xD33525782A7974AA)   /*  292 */,    _ULL(0xA7E25D09078AC09B)   /*  293 */,
		_ULL(0xBD4138B3EAC6EDD0)   /*  294 */,    _ULL(0x920ABFBE71EB9E70)   /*  295 */,
		_ULL(0xA2A5D0F54FC2625C)   /*  296 */,    _ULL(0xC054E36B0B1290A3)   /*  297 */,
		_ULL(0xF6DD59FF62FE932B)   /*  298 */,    _ULL(0x3537354511A8AC7D)   /*  299 */,
		_ULL(0xCA845E9172FADCD4)   /*  300 */,    _ULL(0x84F82B60329D20DC)   /*  301 */,
		_ULL(0x79C62CE1CD672F18)   /*  302 */,    _ULL(0x8B09A2ADD124642C)   /*  303 */,
		_ULL(0xD0C1E96A19D9E726)   /*  304 */,    _ULL(0x5A786A9B4BA9500C)   /*  305 */,
		_ULL(0x0E020336634C43F3)   /*  306 */,    _ULL(0xC17B474AEB66D822)   /*  307 */,
		_ULL(0x6A731AE3EC9BAAC2)   /*  308 */,    _ULL(0x8226667AE0840258)   /*  309 */,
		_ULL(0x67D4567691CAECA5)   /*  310 */,    _ULL(0x1D94155C4875ADB5)   /*  311 */,
		_ULL(0x6D00FD985B813FDF)   /*  312 */,    _ULL(0x51286EFCB774CD06)   /*  313 */,
		_ULL(0x5E8834471FA744AF)   /*  314 */,    _ULL(0xF72CA0AEE761AE2E)   /*  315 */,
		_ULL(0xBE40E4CDAEE8E09A)   /*  316 */,    _ULL(0xE9970BBB5118F665)   /*  317 */,
		_ULL(0x726E4BEB33DF1964)   /*  318 */,    _ULL(0x703B000729199762)   /*  319 */,
		_ULL(0x4631D816F5EF30A7)   /*  320 */,    _ULL(0xB880B5B51504A6BE)   /*  321 */,
		_ULL(0x641793C37ED84B6C)   /*  322 */,    _ULL(0x7B21ED77F6E97D96)   /*  323 */,
		_ULL(0x776306312EF96B73)   /*  324 */,    _ULL(0xAE528948E86FF3F4)   /*  325 */,
		_ULL(0x53DBD7F286A3F8F8)   /*  326 */,    _ULL(0x16CADCE74CFC1063)   /*  327 */,
		_ULL(0x005C19BDFA52C6DD)   /*  328 */,    _ULL(0x68868F5D64D46AD3)   /*  329 */,
		_ULL(0x3A9D512CCF1E186A)   /*  330 */,    _ULL(0x367E62C2385660AE)   /*  331 */,
		_ULL(0xE359E7EA77DCB1D7)   /*  332 */,    _ULL(0x526C0773749ABE6E)   /*  333 */,
		_ULL(0x735AE5F9D09F734B)   /*  334 */,    _ULL(0x493FC7CC8A558BA8)   /*  335 */,
		_ULL(0xB0B9C1533041AB45)   /*  336 */,    _ULL(0x321958BA470A59BD)   /*  337 */,
		_ULL(0x852DB00B5F46C393)   /*  338 */,    _ULL(0x91209B2BD336B0E5)   /*  339 */,
		_ULL(0x6E604F7D659EF19F)   /*  340 */,    _ULL(0xB99A8AE2782CCB24)   /*  341 */,
		_ULL(0xCCF52AB6C814C4C7)   /*  342 */,    _ULL(0x4727D9AFBE11727B)   /*  343 */,
		_ULL(0x7E950D0C0121B34D)   /*  344 */,    _ULL(0x756F435670AD471F)   /*  345 */,
		_ULL(0xF5ADD442615A6849)   /*  346 */,    _ULL(0x4E87E09980B9957A)   /*  347 */,
		_ULL(0x2ACFA1DF50AEE355)   /*  348 */,    _ULL(0xD898263AFD2FD556)   /*  349 */,
		_ULL(0xC8F4924DD80C8FD6)   /*  350 */,    _ULL(0xCF99CA3D754A173A)   /*  351 */,
		_ULL(0xFE477BACAF91BF3C)   /*  352 */,    _ULL(0xED5371F6D690C12D)   /*  353 */,
		_ULL(0x831A5C285E687094)   /*  354 */,    _ULL(0xC5D3C90A3708A0A4)   /*  355 */,
		_ULL(0x0F7F903717D06580)   /*  356 */,    _ULL(0x19F9BB13B8FDF27F)   /*  357 */,
		_ULL(0xB1BD6F1B4D502843)   /*  358 */,    _ULL(0x1C761BA38FFF4012)   /*  359 */,
		_ULL(0x0D1530C4E2E21F3B)   /*  360 */,    _ULL(0x8943CE69A7372C8A)   /*  361 */,
		_ULL(0xE5184E11FEB5CE66)   /*  362 */,    _ULL(0x618BDB80BD736621)   /*  363 */,
		_ULL(0x7D29BAD68B574D0B)   /*  364 */,    _ULL(0x81BB613E25E6FE5B)   /*  365 */,
		_ULL(0x071C9C10BC07913F)   /*  366 */,    _ULL(0xC7BEEB7909AC2D97)   /*  367 */,
		_ULL(0xC3E58D353BC5D757)   /*  368 */,    _ULL(0xEB017892F38F61E8)   /*  369 */,
		_ULL(0xD4EFFB9C9B1CC21A)   /*  370 */,    _ULL(0x99727D26F494F7AB)   /*  371 */,
		_ULL(0xA3E063A2956B3E03)   /*  372 */,    _ULL(0x9D4A8B9A4AA09C30)   /*  373 */,
		_ULL(0x3F6AB7D500090FB4)   /*  374 */,    _ULL(0x9CC0F2A057268AC0)   /*  375 */,
		_ULL(0x3DEE9D2DEDBF42D1)   /*  376 */,    _ULL(0x330F49C87960A972)   /*  377 */,
		_ULL(0xC6B2720287421B41)   /*  378 */,    _ULL(0x0AC59EC07C00369C)   /*  379 */,
		_ULL(0xEF4EAC49CB353425)   /*  380 */,    _ULL(0xF450244EEF0129D8)   /*  381 */,
		_ULL(0x8ACC46E5CAF4DEB6)   /*  382 */,    _ULL(0x2FFEAB63989263F7)   /*  383 */,
		_ULL(0x8F7CB9FE5D7A4578)   /*  384 */,    _ULL(0x5BD8F7644E634635)   /*  385 */,
		_ULL(0x427A7315BF2DC900)   /*  386 */,    _ULL(0x17D0C4AA2125261C)   /*  387 */,
		_ULL(0x3992486C93518E50)   /*  388 */,    _ULL(0xB4CBFEE0A2D7D4C3)   /*  389 */,
		_ULL(0x7C75D6202C5DDD8D)   /*  390 */,    _ULL(0xDBC295D8E35B6C61)   /*  391 */,
		_ULL(0x60B369D302032B19)   /*  392 */,    _ULL(0xCE42685FDCE44132)   /*  393 */,
		_ULL(0x06F3DDB9DDF65610)   /*  394 */,    _ULL(0x8EA4D21DB5E148F0)   /*  395 */,
		_ULL(0x20B0FCE62FCD496F)   /*  396 */,    _ULL(0x2C1B912358B0EE31)   /*  397 */,
		_ULL(0xB28317B818F5A308)   /*  398 */,    _ULL(0xA89C1E189CA6D2CF)   /*  399 */,
		_ULL(0x0C6B18576AAADBC8)   /*  400 */,    _ULL(0xB65DEAA91299FAE3)   /*  401 */,
		_ULL(0xFB2B794B7F1027E7)   /*  402 */,    _ULL(0x04E4317F443B5BEB)   /*  403 */,
		_ULL(0x4B852D325939D0A6)   /*  404 */,    _ULL(0xD5AE6BEEFB207FFC)   /*  405 */,
		_ULL(0x309682B281C7D374)   /*  406 */,    _ULL(0xBAE309A194C3B475)   /*  407 */,
		_ULL(0x8CC3F97B13B49F05)   /*  408 */,    _ULL(0x98A9422FF8293967)   /*  409 */,
		_ULL(0x244B16B01076FF7C)   /*  410 */,    _ULL(0xF8BF571C663D67EE)   /*  411 */,
		_ULL(0x1F0D6758EEE30DA1)   /*  412 */,    _ULL(0xC9B611D97ADEB9B7)   /*  413 */,
		_ULL(0xB7AFD5887B6C57A2)   /*  414 */,    _ULL(0x6290AE846B984FE1)   /*  415 */,
		_ULL(0x94DF4CDEACC1A5FD)   /*  416 */,    _ULL(0x058A5BD1C5483AFF)   /*  417 */,
		_ULL(0x63166CC142BA3C37)   /*  418 */,    _ULL(0x8DB8526EB2F76F40)   /*  419 */,
		_ULL(0xE10880036F0D6D4E)   /*  420 */,    _ULL(0x9E0523C9971D311D)   /*  421 */,
		_ULL(0x45EC2824CC7CD691)   /*  422 */,    _ULL(0x575B8359E62382C9)   /*  423 */,
		_ULL(0xFA9E400DC4889995)   /*  424 */,    _ULL(0xD1823ECB45721568)   /*  425 */,
		_ULL(0xDAFD983B8206082F)   /*  426 */,    _ULL(0xAA7D29082386A8CB)   /*  427 */,
		_ULL(0x269FCD4403B87588)   /*  428 */,    _ULL(0x1B91F5F728BDD1E0)   /*  429 */,
		_ULL(0xE4669F39040201F6)   /*  430 */,    _ULL(0x7A1D7C218CF04ADE)   /*  431 */,
		_ULL(0x65623C29D79CE5CE)   /*  432 */,    _ULL(0x2368449096C00BB1)   /*  433 */,
		_ULL(0xAB9BF1879DA503BA)   /*  434 */,    _ULL(0xBC23ECB1A458058E)   /*  435 */,
		_ULL(0x9A58DF01BB401ECC)   /*  436 */,    _ULL(0xA070E868A85F143D)   /*  437 */,
		_ULL(0x4FF188307DF2239E)   /*  438 */,    _ULL(0x14D565B41A641183)   /*  439 */,
		_ULL(0xEE13337452701602)   /*  440 */,    _ULL(0x950E3DCF3F285E09)   /*  441 */,
		_ULL(0x59930254B9C80953)   /*  442 */,    _ULL(0x3BF299408930DA6D)   /*  443 */,
		_ULL(0xA955943F53691387)   /*  444 */,    _ULL(0xA15EDECAA9CB8784)   /*  445 */,
		_ULL(0x29142127352BE9A0)   /*  446 */,    _ULL(0x76F0371FFF4E7AFB)   /*  447 */,
		_ULL(0x0239F450274F2228)   /*  448 */,    _ULL(0xBB073AF01D5E868B)   /*  449 */,
		_ULL(0xBFC80571C10E96C1)   /*  450 */,    _ULL(0xD267088568222E23)   /*  451 */,
		_ULL(0x9671A3D48E80B5B0)   /*  452 */,    _ULL(0x55B5D38AE193BB81)   /*  453 */,
		_ULL(0x693AE2D0A18B04B8)   /*  454 */,    _ULL(0x5C48B4ECADD5335F)   /*  455 */,
		_ULL(0xFD743B194916A1CA)   /*  456 */,    _ULL(0x2577018134BE98C4)   /*  457 */,
		_ULL(0xE77987E83C54A4AD)   /*  458 */,    _ULL(0x28E11014DA33E1B9)   /*  459 */,
		_ULL(0x270CC59E226AA213)   /*  460 */,    _ULL(0x71495F756D1A5F60)   /*  461 */,
		_ULL(0x9BE853FB60AFEF77)   /*  462 */,    _ULL(0xADC786A7F7443DBF)   /*  463 */,
		_ULL(0x0904456173B29A82)   /*  464 */,    _ULL(0x58BC7A66C232BD5E)   /*  465 */,
		_ULL(0xF306558C673AC8B2)   /*  466 */,    _ULL(0x41F639C6B6C9772A)   /*  467 */,
		_ULL(0x216DEFE99FDA35DA)   /*  468 */,    _ULL(0x11640CC71C7BE615)   /*  469 */,
		_ULL(0x93C43694565C5527)   /*  470 */,    _ULL(0xEA038E6246777839)   /*  471 */,
		_ULL(0xF9ABF3CE5A3E2469)   /*  472 */,    _ULL(0x741E768D0FD312D2)   /*  473 */,
		_ULL(0x0144B883CED652C6)   /*  474 */,    _ULL(0xC20B5A5BA33F8552)   /*  475 */,
		_ULL(0x1AE69633C3435A9D)   /*  476 */,    _ULL(0x97A28CA4088CFDEC)   /*  477 */,
		_ULL(0x8824A43C1E96F420)   /*  478 */,    _ULL(0x37612FA66EEEA746)   /*  479 */,
		_ULL(0x6B4CB165F9CF0E5A)   /*  480 */,    _ULL(0x43AA1C06A0ABFB4A)   /*  481 */,
		_ULL(0x7F4DC26FF162796B)   /*  482 */,    _ULL(0x6CBACC8E54ED9B0F)   /*  483 */,
		_ULL(0xA6B7FFEFD2BB253E)   /*  484 */,    _ULL(0x2E25BC95B0A29D4F)   /*  485 */,
		_ULL(0x86D6A58BDEF1388C)   /*  486 */,    _ULL(0xDED74AC576B6F054)   /*  487 */,
		_ULL(0x8030BDBC2B45805D)   /*  488 */,    _ULL(0x3C81AF70E94D9289)   /*  489 */,
		_ULL(0x3EFF6DDA9E3100DB)   /*  490 */,    _ULL(0xB38DC39FDFCC8847)   /*  491 */,
		_ULL(0x123885528D17B87E)   /*  492 */,    _ULL(0xF2DA0ED240B1B642)   /*  493 */,
		_ULL(0x44CEFADCD54BF9A9)   /*  494 */,    _ULL(0x1312200E433C7EE6)   /*  495 */,
		_ULL(0x9FFCC84F3A78C748)   /*  496 */,    _ULL(0xF0CD1F72248576BB)   /*  497 */,
		_ULL(0xEC6974053638CFE4)   /*  498 */,    _ULL(0x2BA7B67C0CEC4E4C)   /*  499 */,
		_ULL(0xAC2F4DF3E5CE32ED)   /*  500 */,    _ULL(0xCB33D14326EA4C11)   /*  501 */,
		_ULL(0xA4E9044CC77E58BC)   /*  502 */,    _ULL(0x5F513293D934FCEF)   /*  503 */,
		_ULL(0x5DC9645506E55444)   /*  504 */,    _ULL(0x50DE418F317DE40A)   /*  505 */,
		_ULL(0x388CB31A69DDE259)   /*  506 */,    _ULL(0x2DB4A83455820A86)   /*  507 */,
		_ULL(0x9010A91E84711AE9)   /*  508 */,    _ULL(0x4DF7F0B7B1498371)   /*  509 */,
		_ULL(0xD62A2EABC0977179)   /*  510 */,    _ULL(0x22FAC097AA8D5C0E)   /*  511 */,
		_ULL(0xF49FCC2FF1DAF39B)   /*  512 */,    _ULL(0x487FD5C66FF29281)   /*  513 */,
		_ULL(0xE8A30667FCDCA83F)   /*  514 */,    _ULL(0x2C9B4BE3D2FCCE63)   /*  515 */,
		_ULL(0xDA3FF74B93FBBBC2)   /*  516 */,    _ULL(0x2FA165D2FE70BA66)   /*  517 */,
		_ULL(0xA103E279970E93D4)   /*  518 */,    _ULL(0xBECDEC77B0E45E71)   /*  519 */,
		_ULL(0xCFB41E723985E497)   /*  520 */,    _ULL(0xB70AAA025EF75017)   /*  521 */,
		_ULL(0xD42309F03840B8E0)   /*  522 */,    _ULL(0x8EFC1AD035898579)   /*  523 */,
		_ULL(0x96C6920BE2B2ABC5)   /*  524 */,    _ULL(0x66AF4163375A9172)   /*  525 */,
		_ULL(0x2174ABDCCA7127FB)   /*  526 */,    _ULL(0xB33CCEA64A72FF41)   /*  527 */,
		_ULL(0xF04A4933083066A5)   /*  528 */,    _ULL(0x8D970ACDD7289AF5)   /*  529 */,
		_ULL(0x8F96E8E031C8C25E)   /*  530 */,    _ULL(0xF3FEC02276875D47)   /*  531 */,
		_ULL(0xEC7BF310056190DD)   /*  532 */,    _ULL(0xF5ADB0AEBB0F1491)   /*  533 */,
		_ULL(0x9B50F8850FD58892)   /*  534 */,    _ULL(0x4975488358B74DE8)   /*  535 */,
		_ULL(0xA3354FF691531C61)   /*  536 */,    _ULL(0x0702BBE481D2C6EE)   /*  537 */,
		_ULL(0x89FB24057DEDED98)   /*  538 */,    _ULL(0xAC3075138596E902)   /*  539 */,
		_ULL(0x1D2D3580172772ED)   /*  540 */,    _ULL(0xEB738FC28E6BC30D)   /*  541 */,
		_ULL(0x5854EF8F63044326)   /*  542 */,    _ULL(0x9E5C52325ADD3BBE)   /*  543 */,
		_ULL(0x90AA53CF325C4623)   /*  544 */,    _ULL(0xC1D24D51349DD067)   /*  545 */,
		_ULL(0x2051CFEEA69EA624)   /*  546 */,    _ULL(0x13220F0A862E7E4F)   /*  547 */,
		_ULL(0xCE39399404E04864)   /*  548 */,    _ULL(0xD9C42CA47086FCB7)   /*  549 */,
		_ULL(0x685AD2238A03E7CC)   /*  550 */,    _ULL(0x066484B2AB2FF1DB)   /*  551 */,
		_ULL(0xFE9D5D70EFBF79EC)   /*  552 */,    _ULL(0x5B13B9DD9C481854)   /*  553 */,
		_ULL(0x15F0D475ED1509AD)   /*  554 */,    _ULL(0x0BEBCD060EC79851)   /*  555 */,
		_ULL(0xD58C6791183AB7F8)   /*  556 */,    _ULL(0xD1187C5052F3EEE4)   /*  557 */,
		_ULL(0xC95D1192E54E82FF)   /*  558 */,    _ULL(0x86EEA14CB9AC6CA2)   /*  559 */,
		_ULL(0x3485BEB153677D5D)   /*  560 */,    _ULL(0xDD191D781F8C492A)   /*  561 */,
		_ULL(0xF60866BAA784EBF9)   /*  562 */,    _ULL(0x518F643BA2D08C74)   /*  563 */,
		_ULL(0x8852E956E1087C22)   /*  564 */,    _ULL(0xA768CB8DC410AE8D)   /*  565 */,
		_ULL(0x38047726BFEC8E1A)   /*  566 */,    _ULL(0xA67738B4CD3B45AA)   /*  567 */,
		_ULL(0xAD16691CEC0DDE19)   /*  568 */,    _ULL(0xC6D4319380462E07)   /*  569 */,
		_ULL(0xC5A5876D0BA61938)   /*  570 */,    _ULL(0x16B9FA1FA58FD840)   /*  571 */,
		_ULL(0x188AB1173CA74F18)   /*  572 */,    _ULL(0xABDA2F98C99C021F)   /*  573 */,
		_ULL(0x3E0580AB134AE816)   /*  574 */,    _ULL(0x5F3B05B773645ABB)   /*  575 */,
		_ULL(0x2501A2BE5575F2F6)   /*  576 */,    _ULL(0x1B2F74004E7E8BA9)   /*  577 */,
		_ULL(0x1CD7580371E8D953)   /*  578 */,    _ULL(0x7F6ED89562764E30)   /*  579 */,
		_ULL(0xB15926FF596F003D)   /*  580 */,    _ULL(0x9F65293DA8C5D6B9)   /*  581 */,
		_ULL(0x6ECEF04DD690F84C)   /*  582 */,    _ULL(0x4782275FFF33AF88)   /*  583 */,
		_ULL(0xE41433083F820801)   /*  584 */,    _ULL(0xFD0DFE409A1AF9B5)   /*  585 */,
		_ULL(0x4325A3342CDB396B)   /*  586 */,    _ULL(0x8AE77E62B301B252)   /*  587 */,
		_ULL(0xC36F9E9F6655615A)   /*  588 */,    _ULL(0x85455A2D92D32C09)   /*  589 */,
		_ULL(0xF2C7DEA949477485)   /*  590 */,    _ULL(0x63CFB4C133A39EBA)   /*  591 */,
		_ULL(0x83B040CC6EBC5462)   /*  592 */,    _ULL(0x3B9454C8FDB326B0)   /*  593 */,
		_ULL(0x56F56A9E87FFD78C)   /*  594 */,    _ULL(0x2DC2940D99F42BC6)   /*  595 */,
		_ULL(0x98F7DF096B096E2D)   /*  596 */,    _ULL(0x19A6E01E3AD852BF)   /*  597 */,
		_ULL(0x42A99CCBDBD4B40B)   /*  598 */,    _ULL(0xA59998AF45E9C559)   /*  599 */,
		_ULL(0x366295E807D93186)   /*  600 */,    _ULL(0x6B48181BFAA1F773)   /*  601 */,
		_ULL(0x1FEC57E2157A0A1D)   /*  602 */,    _ULL(0x4667446AF6201AD5)   /*  603 */,
		_ULL(0xE615EBCACFB0F075)   /*  604 */,    _ULL(0xB8F31F4F68290778)   /*  605 */,
		_ULL(0x22713ED6CE22D11E)   /*  606 */,    _ULL(0x3057C1A72EC3C93B)   /*  607 */,
		_ULL(0xCB46ACC37C3F1F2F)   /*  608 */,    _ULL(0xDBB893FD02AAF50E)   /*  609 */,
		_ULL(0x331FD92E600B9FCF)   /*  610 */,    _ULL(0xA498F96148EA3AD6)   /*  611 */,
		_ULL(0xA8D8426E8B6A83EA)   /*  612 */,    _ULL(0xA089B274B7735CDC)   /*  613 */,
		_ULL(0x87F6B3731E524A11)   /*  614 */,    _ULL(0x118808E5CBC96749)   /*  615 */,
		_ULL(0x9906E4C7B19BD394)   /*  616 */,    _ULL(0xAFED7F7E9B24A20C)   /*  617 */,
		_ULL(0x6509EADEEB3644A7)   /*  618 */,    _ULL(0x6C1EF1D3E8EF0EDE)   /*  619 */,
		_ULL(0xB9C97D43E9798FB4)   /*  620 */,    _ULL(0xA2F2D784740C28A3)   /*  621 */,
		_ULL(0x7B8496476197566F)   /*  622 */,    _ULL(0x7A5BE3E6B65F069D)   /*  623 */,
		_ULL(0xF96330ED78BE6F10)   /*  624 */,    _ULL(0xEEE60DE77A076A15)   /*  625 */,
		_ULL(0x2B4BEE4AA08B9BD0)   /*  626 */,    _ULL(0x6A56A63EC7B8894E)   /*  627 */,
		_ULL(0x02121359BA34FEF4)   /*  628 */,    _ULL(0x4CBF99F8283703FC)   /*  629 */,
		_ULL(0x398071350CAF30C8)   /*  630 */,    _ULL(0xD0A77A89F017687A)   /*  631 */,
		_ULL(0xF1C1A9EB9E423569)   /*  632 */,    _ULL(0x8C7976282DEE8199)   /*  633 */,
		_ULL(0x5D1737A5DD1F7ABD)   /*  634 */,    _ULL(0x4F53433C09A9FA80)   /*  635 */,
		_ULL(0xFA8B0C53DF7CA1D9)   /*  636 */,    _ULL(0x3FD9DCBC886CCB77)   /*  637 */,
		_ULL(0xC040917CA91B4720)   /*  638 */,    _ULL(0x7DD00142F9D1DCDF)   /*  639 */,
		_ULL(0x8476FC1D4F387B58)   /*  640 */,    _ULL(0x23F8E7C5F3316503)   /*  641 */,
		_ULL(0x032A2244E7E37339)   /*  642 */,    _ULL(0x5C87A5D750F5A74B)   /*  643 */,
		_ULL(0x082B4CC43698992E)   /*  644 */,    _ULL(0xDF917BECB858F63C)   /*  645 */,
		_ULL(0x3270B8FC5BF86DDA)   /*  646 */,    _ULL(0x10AE72BB29B5DD76)   /*  647 */,
		_ULL(0x576AC94E7700362B)   /*  648 */,    _ULL(0x1AD112DAC61EFB8F)   /*  649 */,
		_ULL(0x691BC30EC5FAA427)   /*  650 */,    _ULL(0xFF246311CC327143)   /*  651 */,
		_ULL(0x3142368E30E53206)   /*  652 */,    _ULL(0x71380E31E02CA396)   /*  653 */,
		_ULL(0x958D5C960AAD76F1)   /*  654 */,    _ULL(0xF8D6F430C16DA536)   /*  655 */,
		_ULL(0xC8FFD13F1BE7E1D2)   /*  656 */,    _ULL(0x7578AE66004DDBE1)   /*  657 */,
		_ULL(0x05833F01067BE646)   /*  658 */,    _ULL(0xBB34B5AD3BFE586D)   /*  659 */,
		_ULL(0x095F34C9A12B97F0)   /*  660 */,    _ULL(0x247AB64525D60CA8)   /*  661 */,
		_ULL(0xDCDBC6F3017477D1)   /*  662 */,    _ULL(0x4A2E14D4DECAD24D)   /*  663 */,
		_ULL(0xBDB5E6D9BE0A1EEB)   /*  664 */,    _ULL(0x2A7E70F7794301AB)   /*  665 */,
		_ULL(0xDEF42D8A270540FD)   /*  666 */,    _ULL(0x01078EC0A34C22C1)   /*  667 */,
		_ULL(0xE5DE511AF4C16387)   /*  668 */,    _ULL(0x7EBB3A52BD9A330A)   /*  669 */,
		_ULL(0x77697857AA7D6435)   /*  670 */,    _ULL(0x004E831603AE4C32)   /*  671 */,
		_ULL(0xE7A21020AD78E312)   /*  672 */,    _ULL(0x9D41A70C6AB420F2)   /*  673 */,
		_ULL(0x28E06C18EA1141E6)   /*  674 */,    _ULL(0xD2B28CBD984F6B28)   /*  675 */,
		_ULL(0x26B75F6C446E9D83)   /*  676 */,    _ULL(0xBA47568C4D418D7F)   /*  677 */,
		_ULL(0xD80BADBFE6183D8E)   /*  678 */,    _ULL(0x0E206D7F5F166044)   /*  679 */,
		_ULL(0xE258A43911CBCA3E)   /*  680 */,    _ULL(0x723A1746B21DC0BC)   /*  681 */,
		_ULL(0xC7CAA854F5D7CDD3)   /*  682 */,    _ULL(0x7CAC32883D261D9C)   /*  683 */,
		_ULL(0x7690C26423BA942C)   /*  684 */,    _ULL(0x17E55524478042B8)   /*  685 */,
		_ULL(0xE0BE477656A2389F)   /*  686 */,    _ULL(0x4D289B5E67AB2DA0)   /*  687 */,
		_ULL(0x44862B9C8FBBFD31)   /*  688 */,    _ULL(0xB47CC8049D141365)   /*  689 */,
		_ULL(0x822C1B362B91C793)   /*  690 */,    _ULL(0x4EB14655FB13DFD8)   /*  691 */,
		_ULL(0x1ECBBA0714E2A97B)   /*  692 */,    _ULL(0x6143459D5CDE5F14)   /*  693 */,
		_ULL(0x53A8FBF1D5F0AC89)   /*  694 */,    _ULL(0x97EA04D81C5E5B00)   /*  695 */,
		_ULL(0x622181A8D4FDB3F3)   /*  696 */,    _ULL(0xE9BCD341572A1208)   /*  697 */,
		_ULL(0x1411258643CCE58A)   /*  698 */,    _ULL(0x9144C5FEA4C6E0A4)   /*  699 */,
		_ULL(0x0D33D06565CF620F)   /*  700 */,    _ULL(0x54A48D489F219CA1)   /*  701 */,
		_ULL(0xC43E5EAC6D63C821)   /*  702 */,    _ULL(0xA9728B3A72770DAF)   /*  703 */,
		_ULL(0xD7934E7B20DF87EF)   /*  704 */,    _ULL(0xE35503B61A3E86E5)   /*  705 */,
		_ULL(0xCAE321FBC819D504)   /*  706 */,    _ULL(0x129A50B3AC60BFA6)   /*  707 */,
		_ULL(0xCD5E68EA7E9FB6C3)   /*  708 */,    _ULL(0xB01C90199483B1C7)   /*  709 */,
		_ULL(0x3DE93CD5C295376C)   /*  710 */,    _ULL(0xAED52EDF2AB9AD13)   /*  711 */,
		_ULL(0x2E60F512C0A07884)   /*  712 */,    _ULL(0xBC3D86A3E36210C9)   /*  713 */,
		_ULL(0x35269D9B163951CE)   /*  714 */,    _ULL(0x0C7D6E2AD0CDB5FA)   /*  715 */,
		_ULL(0x59E86297D87F5733)   /*  716 */,    _ULL(0x298EF221898DB0E7)   /*  717 */,
		_ULL(0x55000029D1A5AA7E)   /*  718 */,    _ULL(0x8BC08AE1B5061B45)   /*  719 */,
		_ULL(0xC2C31C2B6C92703A)   /*  720 */,    _ULL(0x94CC596BAF25EF42)   /*  721 */,
		_ULL(0x0A1D73DB22540456)   /*  722 */,    _ULL(0x04B6A0F9D9C4179A)   /*  723 */,
		_ULL(0xEFFDAFA2AE3D3C60)   /*  724 */,    _ULL(0xF7C8075BB49496C4)   /*  725 */,
		_ULL(0x9CC5C7141D1CD4E3)   /*  726 */,    _ULL(0x78BD1638218E5534)   /*  727 */,
		_ULL(0xB2F11568F850246A)   /*  728 */,    _ULL(0xEDFABCFA9502BC29)   /*  729 */,
		_ULL(0x796CE5F2DA23051B)   /*  730 */,    _ULL(0xAAE128B0DC93537C)   /*  731 */,
		_ULL(0x3A493DA0EE4B29AE)   /*  732 */,    _ULL(0xB5DF6B2C416895D7)   /*  733 */,
		_ULL(0xFCABBD25122D7F37)   /*  734 */,    _ULL(0x70810B58105DC4B1)   /*  735 */,
		_ULL(0xE10FDD37F7882A90)   /*  736 */,    _ULL(0x524DCAB5518A3F5C)   /*  737 */,
		_ULL(0x3C9E85878451255B)   /*  738 */,    _ULL(0x4029828119BD34E2)   /*  739 */,
		_ULL(0x74A05B6F5D3CECCB)   /*  740 */,    _ULL(0xB610021542E13ECA)   /*  741 */,
		_ULL(0x0FF979D12F59E2AC)   /*  742 */,    _ULL(0x6037DA27E4F9CC50)   /*  743 */,
		_ULL(0x5E92975A0DF1847D)   /*  744 */,    _ULL(0xD66DE190D3E623FE)   /*  745 */,
		_ULL(0x5032D6B87B568048)   /*  746 */,    _ULL(0x9A36B7CE8235216E)   /*  747 */,
		_ULL(0x80272A7A24F64B4A)   /*  748 */,    _ULL(0x93EFED8B8C6916F7)   /*  749 */,
		_ULL(0x37DDBFF44CCE1555)   /*  750 */,    _ULL(0x4B95DB5D4B99BD25)   /*  751 */,
		_ULL(0x92D3FDA169812FC0)   /*  752 */,    _ULL(0xFB1A4A9A90660BB6)   /*  753 */,
		_ULL(0x730C196946A4B9B2)   /*  754 */,    _ULL(0x81E289AA7F49DA68)   /*  755 */,
		_ULL(0x64669A0F83B1A05F)   /*  756 */,    _ULL(0x27B3FF7D9644F48B)   /*  757 */,
		_ULL(0xCC6B615C8DB675B3)   /*  758 */,    _ULL(0x674F20B9BCEBBE95)   /*  759 */,
		_ULL(0x6F31238275655982)   /*  760 */,    _ULL(0x5AE488713E45CF05)   /*  761 */,
		_ULL(0xBF619F9954C21157)   /*  762 */,    _ULL(0xEABAC46040A8EAE9)   /*  763 */,
		_ULL(0x454C6FE9F2C0C1CD)   /*  764 */,    _ULL(0x419CF6496412691C)   /*  765 */,
		_ULL(0xD3DC3BEF265B0F70)   /*  766 */,    _ULL(0x6D0E60F5C3578A9E)   /*  767 */,
		_ULL(0x5B0E608526323C55)   /*  768 */,    _ULL(0x1A46C1A9FA1B59F5)   /*  769 */,
		_ULL(0xA9E245A17C4C8FFA)   /*  770 */,    _ULL(0x65CA5159DB2955D7)   /*  771 */,
		_ULL(0x05DB0A76CE35AFC2)   /*  772 */,    _ULL(0x81EAC77EA9113D45)   /*  773 */,
		_ULL(0x528EF88AB6AC0A0D)   /*  774 */,    _ULL(0xA09EA253597BE3FF)   /*  775 */,
		_ULL(0x430DDFB3AC48CD56)   /*  776 */,    _ULL(0xC4B3A67AF45CE46F)   /*  777 */,
		_ULL(0x4ECECFD8FBE2D05E)   /*  778 */,    _ULL(0x3EF56F10B39935F0)   /*  779 */,
		_ULL(0x0B22D6829CD619C6)   /*  780 */,    _ULL(0x17FD460A74DF2069)   /*  781 */,
		_ULL(0x6CF8CC8E8510ED40)   /*  782 */,    _ULL(0xD6C824BF3A6ECAA7)   /*  783 */,
		_ULL(0x61243D581A817049)   /*  784 */,    _ULL(0x048BACB6BBC163A2)   /*  785 */,
		_ULL(0xD9A38AC27D44CC32)   /*  786 */,    _ULL(0x7FDDFF5BAAF410AB)   /*  787 */,
		_ULL(0xAD6D495AA804824B)   /*  788 */,    _ULL(0xE1A6A74F2D8C9F94)   /*  789 */,
		_ULL(0xD4F7851235DEE8E3)   /*  790 */,    _ULL(0xFD4B7F886540D893)   /*  791 */,
		_ULL(0x247C20042AA4BFDA)   /*  792 */,    _ULL(0x096EA1C517D1327C)   /*  793 */,
		_ULL(0xD56966B4361A6685)   /*  794 */,    _ULL(0x277DA5C31221057D)   /*  795 */,
		_ULL(0x94D59893A43ACFF7)   /*  796 */,    _ULL(0x64F0C51CCDC02281)   /*  797 */,
		_ULL(0x3D33BCC4FF6189DB)   /*  798 */,    _ULL(0xE005CB184CE66AF1)   /*  799 */,
		_ULL(0xFF5CCD1D1DB99BEA)   /*  800 */,    _ULL(0xB0B854A7FE42980F)   /*  801 */,
		_ULL(0x7BD46A6A718D4B9F)   /*  802 */,    _ULL(0xD10FA8CC22A5FD8C)   /*  803 */,
		_ULL(0xD31484952BE4BD31)   /*  804 */,    _ULL(0xC7FA975FCB243847)   /*  805 */,
		_ULL(0x4886ED1E5846C407)   /*  806 */,    _ULL(0x28CDDB791EB70B04)   /*  807 */,
		_ULL(0xC2B00BE2F573417F)   /*  808 */,    _ULL(0x5C9590452180F877)   /*  809 */,
		_ULL(0x7A6BDDFFF370EB00)   /*  810 */,    _ULL(0xCE509E38D6D9D6A4)   /*  811 */,
		_ULL(0xEBEB0F00647FA702)   /*  812 */,    _ULL(0x1DCC06CF76606F06)   /*  813 */,
		_ULL(0xE4D9F28BA286FF0A)   /*  814 */,    _ULL(0xD85A305DC918C262)   /*  815 */,
		_ULL(0x475B1D8732225F54)   /*  816 */,    _ULL(0x2D4FB51668CCB5FE)   /*  817 */,
		_ULL(0xA679B9D9D72BBA20)   /*  818 */,    _ULL(0x53841C0D912D43A5)   /*  819 */,
		_ULL(0x3B7EAA48BF12A4E8)   /*  820 */,    _ULL(0x781E0E47F22F1DDF)   /*  821 */,
		_ULL(0xEFF20CE60AB50973)   /*  822 */,    _ULL(0x20D261D19DFFB742)   /*  823 */,
		_ULL(0x16A12B03062A2E39)   /*  824 */,    _ULL(0x1960EB2239650495)   /*  825 */,
		_ULL(0x251C16FED50EB8B8)   /*  826 */,    _ULL(0x9AC0C330F826016E)   /*  827 */,
		_ULL(0xED152665953E7671)   /*  828 */,    _ULL(0x02D63194A6369570)   /*  829 */,
		_ULL(0x5074F08394B1C987)   /*  830 */,    _ULL(0x70BA598C90B25CE1)   /*  831 */,
		_ULL(0x794A15810B9742F6)   /*  832 */,    _ULL(0x0D5925E9FCAF8C6C)   /*  833 */,
		_ULL(0x3067716CD868744E)   /*  834 */,    _ULL(0x910AB077E8D7731B)   /*  835 */,
		_ULL(0x6A61BBDB5AC42F61)   /*  836 */,    _ULL(0x93513EFBF0851567)   /*  837 */,
		_ULL(0xF494724B9E83E9D5)   /*  838 */,    _ULL(0xE887E1985C09648D)   /*  839 */,
		_ULL(0x34B1D3C675370CFD)   /*  840 */,    _ULL(0xDC35E433BC0D255D)   /*  841 */,
		_ULL(0xD0AAB84234131BE0)   /*  842 */,    _ULL(0x08042A50B48B7EAF)   /*  843 */,
		_ULL(0x9997C4EE44A3AB35)   /*  844 */,    _ULL(0x829A7B49201799D0)   /*  845 */,
		_ULL(0x263B8307B7C54441)   /*  846 */,    _ULL(0x752F95F4FD6A6CA6)   /*  847 */,
		_ULL(0x927217402C08C6E5)   /*  848 */,    _ULL(0x2A8AB754A795D9EE)   /*  849 */,
		_ULL(0xA442F7552F72943D)   /*  850 */,    _ULL(0x2C31334E19781208)   /*  851 */,
		_ULL(0x4FA98D7CEAEE6291)   /*  852 */,    _ULL(0x55C3862F665DB309)   /*  853 */,
		_ULL(0xBD0610175D53B1F3)   /*  854 */,    _ULL(0x46FE6CB840413F27)   /*  855 */,
		_ULL(0x3FE03792DF0CFA59)   /*  856 */,    _ULL(0xCFE700372EB85E8F)   /*  857 */,
		_ULL(0xA7BE29E7ADBCE118)   /*  858 */,    _ULL(0xE544EE5CDE8431DD)   /*  859 */,
		_ULL(0x8A781B1B41F1873E)   /*  860 */,    _ULL(0xA5C94C78A0D2F0E7)   /*  861 */,
		_ULL(0x39412E2877B60728)   /*  862 */,    _ULL(0xA1265EF3AFC9A62C)   /*  863 */,
		_ULL(0xBCC2770C6A2506C5)   /*  864 */,    _ULL(0x3AB66DD5DCE1CE12)   /*  865 */,
		_ULL(0xE65499D04A675B37)   /*  866 */,    _ULL(0x7D8F523481BFD216)   /*  867 */,
		_ULL(0x0F6F64FCEC15F389)   /*  868 */,    _ULL(0x74EFBE618B5B13C8)   /*  869 */,
		_ULL(0xACDC82B714273E1D)   /*  870 */,    _ULL(0xDD40BFE003199D17)   /*  871 */,
		_ULL(0x37E99257E7E061F8)   /*  872 */,    _ULL(0xFA52626904775AAA)   /*  873 */,
		_ULL(0x8BBBF63A463D56F9)   /*  874 */,    _ULL(0xF0013F1543A26E64)   /*  875 */,
		_ULL(0xA8307E9F879EC898)   /*  876 */,    _ULL(0xCC4C27A4150177CC)   /*  877 */,
		_ULL(0x1B432F2CCA1D3348)   /*  878 */,    _ULL(0xDE1D1F8F9F6FA013)   /*  879 */,
		_ULL(0x606602A047A7DDD6)   /*  880 */,    _ULL(0xD237AB64CC1CB2C7)   /*  881 */,
		_ULL(0x9B938E7225FCD1D3)   /*  882 */,    _ULL(0xEC4E03708E0FF476)   /*  883 */,
		_ULL(0xFEB2FBDA3D03C12D)   /*  884 */,    _ULL(0xAE0BCED2EE43889A)   /*  885 */,
		_ULL(0x22CB8923EBFB4F43)   /*  886 */,    _ULL(0x69360D013CF7396D)   /*  887 */,
		_ULL(0x855E3602D2D4E022)   /*  888 */,    _ULL(0x073805BAD01F784C)   /*  889 */,
		_ULL(0x33E17A133852F546)   /*  890 */,    _ULL(0xDF4874058AC7B638)   /*  891 */,
		_ULL(0xBA92B29C678AA14A)   /*  892 */,    _ULL(0x0CE89FC76CFAADCD)   /*  893 */,
		_ULL(0x5F9D4E0908339E34)   /*  894 */,    _ULL(0xF1AFE9291F5923B9)   /*  895 */,
		_ULL(0x6E3480F60F4A265F)   /*  896 */,    _ULL(0xEEBF3A2AB29B841C)   /*  897 */,
		_ULL(0xE21938A88F91B4AD)   /*  898 */,    _ULL(0x57DFEFF845C6D3C3)   /*  899 */,
		_ULL(0x2F006B0BF62CAAF2)   /*  900 */,    _ULL(0x62F479EF6F75EE78)   /*  901 */,
		_ULL(0x11A55AD41C8916A9)   /*  902 */,    _ULL(0xF229D29084FED453)   /*  903 */,
		_ULL(0x42F1C27B16B000E6)   /*  904 */,    _ULL(0x2B1F76749823C074)   /*  905 */,
		_ULL(0x4B76ECA3C2745360)   /*  906 */,    _ULL(0x8C98F463B91691BD)   /*  907 */,
		_ULL(0x14BCC93CF1ADE66A)   /*  908 */,    _ULL(0x8885213E6D458397)   /*  909 */,
		_ULL(0x8E177DF0274D4711)   /*  910 */,    _ULL(0xB49B73B5503F2951)   /*  911 */,
		_ULL(0x10168168C3F96B6B)   /*  912 */,    _ULL(0x0E3D963B63CAB0AE)   /*  913 */,
		_ULL(0x8DFC4B5655A1DB14)   /*  914 */,    _ULL(0xF789F1356E14DE5C)   /*  915 */,
		_ULL(0x683E68AF4E51DAC1)   /*  916 */,    _ULL(0xC9A84F9D8D4B0FD9)   /*  917 */,
		_ULL(0x3691E03F52A0F9D1)   /*  918 */,    _ULL(0x5ED86E46E1878E80)   /*  919 */,
		_ULL(0x3C711A0E99D07150)   /*  920 */,    _ULL(0x5A0865B20C4E9310)   /*  921 */,
		_ULL(0x56FBFC1FE4F0682E)   /*  922 */,    _ULL(0xEA8D5DE3105EDF9B)   /*  923 */,
		_ULL(0x71ABFDB12379187A)   /*  924 */,    _ULL(0x2EB99DE1BEE77B9C)   /*  925 */,
		_ULL(0x21ECC0EA33CF4523)   /*  926 */,    _ULL(0x59A4D7521805C7A1)   /*  927 */,
		_ULL(0x3896F5EB56AE7C72)   /*  928 */,    _ULL(0xAA638F3DB18F75DC)   /*  929 */,
		_ULL(0x9F39358DABE9808E)   /*  930 */,    _ULL(0xB7DEFA91C00B72AC)   /*  931 */,
		_ULL(0x6B5541FD62492D92)   /*  932 */,    _ULL(0x6DC6DEE8F92E4D5B)   /*  933 */,
		_ULL(0x353F57ABC4BEEA7E)   /*  934 */,    _ULL(0x735769D6DA5690CE)   /*  935 */,
		_ULL(0x0A234AA642391484)   /*  936 */,    _ULL(0xF6F9508028F80D9D)   /*  937 */,
		_ULL(0xB8E319A27AB3F215)   /*  938 */,    _ULL(0x31AD9C1151341A4D)   /*  939 */,
		_ULL(0x773C22A57BEF5805)   /*  940 */,    _ULL(0x45C7561A07968633)   /*  941 */,
		_ULL(0xF913DA9E249DBE36)   /*  942 */,    _ULL(0xDA652D9B78A64C68)   /*  943 */,
		_ULL(0x4C27A97F3BC334EF)   /*  944 */,    _ULL(0x76621220E66B17F4)   /*  945 */,
		_ULL(0x967743899ACD7D0B)   /*  946 */,    _ULL(0xF3EE5BCAE0ED6782)   /*  947 */,
		_ULL(0x409F753600C879FC)   /*  948 */,    _ULL(0x06D09A39B5926DB6)   /*  949 */,
		_ULL(0x6F83AEB0317AC588)   /*  950 */,    _ULL(0x01E6CA4A86381F21)   /*  951 */,
		_ULL(0x66FF3462D19F3025)   /*  952 */,    _ULL(0x72207C24DDFD3BFB)   /*  953 */,
		_ULL(0x4AF6B6D3E2ECE2EB)   /*  954 */,    _ULL(0x9C994DBEC7EA08DE)   /*  955 */,
		_ULL(0x49ACE597B09A8BC4)   /*  956 */,    _ULL(0xB38C4766CF0797BA)   /*  957 */,
		_ULL(0x131B9373C57C2A75)   /*  958 */,    _ULL(0xB1822CCE61931E58)   /*  959 */,
		_ULL(0x9D7555B909BA1C0C)   /*  960 */,    _ULL(0x127FAFDD937D11D2)   /*  961 */,
		_ULL(0x29DA3BADC66D92E4)   /*  962 */,    _ULL(0xA2C1D57154C2ECBC)   /*  963 */,
		_ULL(0x58C5134D82F6FE24)   /*  964 */,    _ULL(0x1C3AE3515B62274F)   /*  965 */,
		_ULL(0xE907C82E01CB8126)   /*  966 */,    _ULL(0xF8ED091913E37FCB)   /*  967 */,
		_ULL(0x3249D8F9C80046C9)   /*  968 */,    _ULL(0x80CF9BEDE388FB63)   /*  969 */,
		_ULL(0x1881539A116CF19E)   /*  970 */,    _ULL(0x5103F3F76BD52457)   /*  971 */,
		_ULL(0x15B7E6F5AE47F7A8)   /*  972 */,    _ULL(0xDBD7C6DED47E9CCF)   /*  973 */,
		_ULL(0x44E55C410228BB1A)   /*  974 */,    _ULL(0xB647D4255EDB4E99)   /*  975 */,
		_ULL(0x5D11882BB8AAFC30)   /*  976 */,    _ULL(0xF5098BBB29D3212A)   /*  977 */,
		_ULL(0x8FB5EA14E90296B3)   /*  978 */,    _ULL(0x677B942157DD025A)   /*  979 */,
		_ULL(0xFB58E7C0A390ACB5)   /*  980 */,    _ULL(0x89D3674C83BD4A01)   /*  981 */,
		_ULL(0x9E2DA4DF4BF3B93B)   /*  982 */,    _ULL(0xFCC41E328CAB4829)   /*  983 */,
		_ULL(0x03F38C96BA582C52)   /*  984 */,    _ULL(0xCAD1BDBD7FD85DB2)   /*  985 */,
		_ULL(0xBBB442C16082AE83)   /*  986 */,    _ULL(0xB95FE86BA5DA9AB0)   /*  987 */,
		_ULL(0xB22E04673771A93F)   /*  988 */,    _ULL(0x845358C9493152D8)   /*  989 */,
		_ULL(0xBE2A488697B4541E)   /*  990 */,    _ULL(0x95A2DC2DD38E6966)   /*  991 */,
		_ULL(0xC02C11AC923C852B)   /*  992 */,    _ULL(0x2388B1990DF2A87B)   /*  993 */,
		_ULL(0x7C8008FA1B4F37BE)   /*  994 */,    _ULL(0x1F70D0C84D54E503)   /*  995 */,
		_ULL(0x5490ADEC7ECE57D4)   /*  996 */,    _ULL(0x002B3C27D9063A3A)   /*  997 */,
		_ULL(0x7EAEA3848030A2BF)   /*  998 */,    _ULL(0xC602326DED2003C0)   /*  999 */,
		_ULL(0x83A7287D69A94086)   /* 1000 */,    _ULL(0xC57A5FCB30F57A8A)   /* 1001 */,
		_ULL(0xB56844E479EBE779)   /* 1002 */,    _ULL(0xA373B40F05DCBCE9)   /* 1003 */,
		_ULL(0xD71A786E88570EE2)   /* 1004 */,    _ULL(0x879CBACDBDE8F6A0)   /* 1005 */,
		_ULL(0x976AD1BCC164A32F)   /* 1006 */,    _ULL(0xAB21E25E9666D78B)   /* 1007 */,
		_ULL(0x901063AAE5E5C33C)   /* 1008 */,    _ULL(0x9818B34448698D90)   /* 1009 */,
		_ULL(0xE36487AE3E1E8ABB)   /* 1010 */,    _ULL(0xAFBDF931893BDCB4)   /* 1011 */,
		_ULL(0x6345A0DC5FBBD519)   /* 1012 */,    _ULL(0x8628FE269B9465CA)   /* 1013 */,
		_ULL(0x1E5D01603F9C51EC)   /* 1014 */,    _ULL(0x4DE44006A15049B7)   /* 1015 */,
		_ULL(0xBF6C70E5F776CBB1)   /* 1016 */,    _ULL(0x411218F2EF552BED)   /* 1017 */,
		_ULL(0xCB0C0708705A36A3)   /* 1018 */,    _ULL(0xE74D14754F986044)   /* 1019 */,
		_ULL(0xCD56D9430EA8280E)   /* 1020 */,    _ULL(0xC12591D7535F5065)   /* 1021 */,
		_ULL(0xC83223F1720AEF96)   /* 1022 */,    _ULL(0xC3A0396F7363A51F)   /* 1023 */
};

/**
 * @file
 * $Id: TigerHash.cpp,v 1.13 2006/12/18 20:36:02 bigmuscle Exp $
 */
