/*
 * Copyright (c) 2003 Rozhuk Ivan <rozhuk.im@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */


#if !defined(AFX__SHA1_H__INCLUDED_)
#define AFX__SHA1_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



// see
// RFC 3174 - SHA1
// RFC 2104 - HMAC
// RFC 2617 - CvtHex



#define SHA1HashSize 20
#define SHA1HashHexSize 40

/* This structure will hold context information for the SHA-1 hashing operation */
typedef struct SHA1Context
{
	DWORD Intermediate_Hash[SHA1HashSize/4];/* Message Digest  */
	ULARGE_INTEGER Length;					/* Message length in bits      */
	BYTE Message_Block[64];					/* 512-bit message blocks      */
	BOOL Computed;							/* Is the digest computed?         */
} SHA1Context;


#ifdef UNICODE
	#define SHA1HMACGetString	SHA1HMACGetStringW
	#define SHA1GetStringDigest	SHA1GetStringDigestW
	#define SHA1CvtString		SHA1CvtStringW
#else
	#define SHA1HMACGetString	SHA1HMACGetStringA
	#define SHA1GetStringDigest	SHA1GetStringDigestA
	#define SHA1CvtString		SHA1CvtStringA
#endif


#ifndef SHA1_MAX_SPEED
	#ifdef SecureZeroMemory
		#define SHA1SecureZeroMemory SecureZeroMemory
	#else
		#define SHA1SecureZeroMemory bzero
	#endif
#else
	#define SHA1SecureZeroMemory
#endif


/*
 *  Description:
 *      This file implements the Secure Hashing Algorithm 1 as
 *      defined in FIPS PUB 180-1 published April 17, 1995.
 *
 *      The SHA-1, produces a 160-bit message digest for a given
 *      data stream.  It should take about 2**n steps to find a
 *      message with the same digest as a given message and
 *      2**(n/2) to find any two messages with the same digest,
 *      when n is the digest size in bits.  Therefore, this
 *      algorithm can serve as a means of providing a
 *      "fingerprint" for a message.
 *
 *  Portability Issues:
 *      SHA-1 is defined in terms of 32-bit "words".  This code
 *      uses <stdint.h> (included via "sha1.h" to define 32 and 8
 *      bit unsigned integer types.  If your C compiler does not
 *      support 32 bit unsigned integers, this code is not
 *      appropriate.
 *
 *  Caveats:
 *      SHA-1 is designed to work with messages less than 2^64 bits
 *      long.  Although SHA-1 allows a message digest to be generated
 *      for messages of any number of bits less than 2^64, this
 *      implementation only works with messages with a length that is
 *      a multiple of the size of an 8-bit character.
 *
 */



/* Define the SHA1 circular left shift macro */
#define SHA1CircularShift(bits,word) (((word) << (bits)) | ((word) >> (32-(bits))))

/* Local Function Prototyptes */
//void SHA1PadMessage(SHA1Context *);
//void SHA1ProcessMessageBlock(SHA1Context *);


__inline DWORD BSWAP(DWORD dwIn)
{
return((((dwIn<<8) & 0x00ff0000) | (dwIn<<24) | ((dwIn>>8) & 0x0000ff00) | (dwIn>>24)));
}

__inline void CopyMemoryReverseDWORD(LPCVOID lpcDestination,LPCVOID lpcSource,SIZE_T dwSize)
{
#ifdef  _WIN64
	BYTE *pDestination=(BYTE*)lpcDestination,*pSource=(BYTE*)lpcSource;

	//for(SIZE_T i=0;i<dwSize;i++) pDestination[i]=pSource[(i&~0x00000003)+(3-(i&0x00000003))];
	for(SIZE_T i=0;i<dwSize;i+=4) (*((DWORD*)(pDestination+i)))=BSWAP((*((DWORD*)(pSource+i))));

#else
	__asm{
		push	edi					// сохраняем регистр
		push	esi					// сохраняем регистр
	
		mov		ecx,dwSize			// ecx = длинна входного буффера
		mov     edi,lpcDestination	// edi = адрес выходного буффера
		mov     esi,lpcSource		// esi = указатель на входной буффер
		cld

	read_loop:
		lodsd						// читаем 4 байта
		bswap   eax
		stosd
		sub     ecx,4
		jg		short read_loop		// если длинна 3 и более байт, то продолжаем дальше
		
		pop		esi					// восстанавливаем содержимое регистра
		pop		edi					// восстанавливаем содержимое регистра
	}
#endif
}



/*
*  SHA1ProcessMessageBlock
*
*  Description:
*      This function will process the next 512 bits of the message
*      stored in the Message_Block array.
*
*  Parameters:
*      None.
*
*  Returns:
*      Nothing.
*
*  Comments:
*      Many of the variable names in this code, especially the
*      single character names, were used because those were the
*      names used in the publication.
*
*
*/
__inline void SHA1ProcessMessageBlock(SHA1Context *context,BYTE *Message_Block)
{
	 /* Constants defined in SHA-1   */
	const DWORD K[]={0x5A827999,0x6ED9EBA1,0x8F1BBCDC,0xCA62C1D6};
	DWORD t;								/* Loop counter                */
	DWORD temp;								/* Temporary word value        */
	DWORD W[80];							/* Word sequence               */
	DWORD 	A=context->Intermediate_Hash[0],/* Word buffers                */
			B=context->Intermediate_Hash[1],
			C=context->Intermediate_Hash[2],
			D=context->Intermediate_Hash[3],
			E=context->Intermediate_Hash[4];

	/* Initialize the first 16 words in the array W */
	CopyMemoryReverseDWORD(W,Message_Block,64);

	for(t=16;t<80;t++)
	{
		W[t]=SHA1CircularShift(1,W[t-3]^W[t-8]^W[t-14]^W[t-16]);
	}

	for(t=0;t<20;t++)
	{
		temp=SHA1CircularShift(5,A) + ((B&C) | ((~B)&D)) + E + W[t] + K[0];
		E=D;
		D=C;
		C=SHA1CircularShift(30,B);
		B=A;
		A=temp;
	}

	for(t=20;t<40;t++)
	{
		temp=SHA1CircularShift(5,A) + (B^C^D) + E + W[t] + K[1];
		E=D;
		D=C;
		C=SHA1CircularShift(30,B);
		B=A;
		A=temp;
	}

	for(t=40;t<60;t++)
	{
		temp=SHA1CircularShift(5,A) + ((B&C) | (B&D) | (C&D)) + E + W[t] + K[2];
		E=D;
		D=C;
		C=SHA1CircularShift(30,B);
		B=A;
		A=temp;
	}

	for(t=60;t<80;t++)
	{
		temp=SHA1CircularShift(5,A) + (B^C^D) + E + W[t] + K[3];
		E=D;
		D=C;
		C=SHA1CircularShift(30,B);
		B=A;
		A=temp;
	}

	context->Intermediate_Hash[0]+=A;
	context->Intermediate_Hash[1]+=B;
	context->Intermediate_Hash[2]+=C;
	context->Intermediate_Hash[3]+=D;
	context->Intermediate_Hash[4]+=E;

	/* Zeroize sensitive information.*/
	SHA1SecureZeroMemory(W,sizeof(W));
}


/*
*  SHA1PadMessage
*
*  Description:
*      According to the standard, the message must be padded to an even
*      512 bits.  The first padding bit must be a '1'.  The last 64
*      bits represent the length of the original message.  All bits in
*      between should be 0.  This function will pad the message
*      according to those rules by filling the Message_Block array
*      accordingly.  It will also call the ProcessMessageBlock function
*      provided appropriately.  When it returns, it can be assumed that
*      the message digest has been computed.
*
*  Parameters:
*      context: [in/out]
*          The context to pad
*      ProcessMessageBlock: [in]
*          The appropriate SHA*ProcessMessageBlock function
*  Returns:
*      Nothing.
*
*/
__inline void SHA1PadMessage(SHA1Context *context)
{
	/*
	*  Check to see if the current message block is too small to hold
	*  the initial padding bits and length.  If so, we will pad the
	*  block, process it, and then continue padding into a second
	*  block.
	*/
	SIZE_T Message_Block_Index=(SIZE_T)((context->Length.LowPart>>3) & 0x3F);
    context->Message_Block[Message_Block_Index++]=0x80;
	if (Message_Block_Index>56)
	{
		memset(&context->Message_Block[Message_Block_Index], 0, (64-Message_Block_Index));
		SHA1ProcessMessageBlock(context,context->Message_Block);
		memset(&context->Message_Block, 0, 56);
	}else{
		memset(&context->Message_Block[Message_Block_Index], 0, (56-Message_Block_Index));
	}

	/* Store the message length as the last 8 octets */
	context->Message_Block[56]=(BYTE)(context->Length.HighPart>>24);
	context->Message_Block[57]=(BYTE)(context->Length.HighPart>>16);
	context->Message_Block[58]=(BYTE)(context->Length.HighPart>>8);
	context->Message_Block[59]=(BYTE)(context->Length.HighPart);
	context->Message_Block[60]=(BYTE)(context->Length.LowPart>>24);
	context->Message_Block[61]=(BYTE)(context->Length.LowPart>>16);
	context->Message_Block[62]=(BYTE)(context->Length.LowPart>>8);
	context->Message_Block[63]=(BYTE)(context->Length.LowPart);

	SHA1ProcessMessageBlock(context,context->Message_Block);
}



/*
*  SHA1Reset
*
*  Description:
*      This function will initialize the SHA1Context in preparation
*      for computing a new SHA1 message digest.
*
*  Parameters:
*      context: [in/out]
*          The context to reset.
*
*  Returns:
*      sha Error Code.
*
*/
__inline DWORD SHA1Reset(SHA1Context *context)
{
	context->Intermediate_Hash[0]=0x67452301;
	context->Intermediate_Hash[1]=0xEFCDAB89;
	context->Intermediate_Hash[2]=0x98BADCFE;
	context->Intermediate_Hash[3]=0x10325476;
	context->Intermediate_Hash[4]=0xC3D2E1F0;
	context->Length.QuadPart=0;
	context->Computed=FALSE;

return(NO_ERROR);
}

/*
*  SHA1Result
*
*  Description:
*      This function will return the 160-bit message digest into the
*      Message_Digest array  provided by the caller.
*      NOTE: The first octet of hash is stored in the 0th element,
*            the last octet of hash in the 19th element.
*
*  Parameters:
*      context: [in/out]
*          The context to use to calculate the SHA-1 hash.
*      Message_Digest: [out]
*          Where the digest is returned.
*
*  Returns:
*      sha Error Code.
*
*/
__inline DWORD SHA1Result(SHA1Context *context,BYTE *Message_Digest)
{
	if (context->Computed==FALSE)
	{
		SHA1PadMessage(context);
		SHA1SecureZeroMemory(context->Message_Block,64);/* message may be sensitive, clear it out */
		context->Length.QuadPart=0;				/* and clear length */
		context->Computed=TRUE;
	}

	CopyMemoryReverseDWORD(Message_Digest,context->Intermediate_Hash,SHA1HashSize);

return(NO_ERROR);
}

/*
*  SHA1Input
*
*  Description:
*      This function accepts an array of octets as the next portion
*      of the message.
*
*  Parameters:
*      context: [in/out]
*          The SHA context to update
*      message_array: [in]
*          An array of characters representing the next portion of
*          the message.
*      length: [in]
*          The length of the message in message_array
*
*  Returns:
*      sha Error Code.
*
*/
__inline DWORD SHA1Input(SHA1Context *context,const BYTE *message_array,SIZE_T length)
{
	if (context->Computed==TRUE) return(ERROR_INVALID_HANDLE_STATE);

	if ((context->Length.QuadPart+(length<<3))>=(length<<3))
	{
		SIZE_T i,Message_Block_Index,partLen;
		/* Compute number of bytes mod 64 */
		Message_Block_Index=(SIZE_T)((context->Length.LowPart>>3) & 0x3F);
		/* Update number of bits */
		context->Length.QuadPart+=(((ULONGLONG)length)<<3);
		partLen=(64-Message_Block_Index);
		/* Transform as many times as possible.*/
		if (length>=partLen)
		{
			memmove(&context->Message_Block[Message_Block_Index],message_array,partLen);
			SHA1ProcessMessageBlock(context,context->Message_Block);
			for (i=partLen;(i+63)<length;i+=64) SHA1ProcessMessageBlock(context,(BYTE*)&message_array[i]);
			Message_Block_Index=0;
		}else{
			i=0;
		}
		/* Buffer remaining input */
		memmove(&context->Message_Block[Message_Block_Index],&message_array[i],(length-i));
	}else{
		return(RPC_S_STRING_TOO_LONG);/* Message is too long */
	}
return(NO_ERROR);
}
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//////////////////////////////RFC 2104//////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
__inline void hmac_sha1(BYTE *text,SIZE_T text_len,BYTE *key,SIZE_T key_len,BYTE *digest)
{
//BYTE*	text;		/* pointer to data stream */
//int	text_len;	/* length of data stream */
//BYTE*	key;		/* pointer to authentication key */
//int	key_len;	/* length of authentication key */
//HASH	digest;		/* caller digest to be filled in */
	SHA1Context context;
	BYTE k_ipad[65];	/* inner padding - key XORd with ipad */
	BYTE k_opad[65];	/* outer padding - key XORd with opad */
	BYTE tk[SHA1HashSize];
    /* if key is longer than 64 bytes reset it to key=SHA1(key) */
	if (key_len>64)
	{
		SHA1Context tctx;

		SHA1Reset(&tctx);
		SHA1Input(&tctx,key,key_len);
		SHA1Result(&tctx,(BYTE*)&tk);

		key=tk;
		key_len=SHA1HashSize;
	}

	/*
	* the HMAC_SHA1 transform looks like:
	*
	* SHA1(K XOR opad, SHA1(K XOR ipad, text))
	*
	* where K is an n byte key
	* ipad is the byte 0x36 repeated 64 times
	* opad is the byte 0x5c repeated 64 times
	* and text is the data being protected
	*/

	/* start out by storing key in pads */
	memmove(&k_ipad,key,key_len);
	memmove(&k_opad,key,key_len);
    memset(&k_ipad[key_len], 0, (sizeof(k_ipad)-key_len));
    memset(&k_opad[key_len], 0 , (sizeof(k_opad)-key_len));

	/* XOR key with ipad and opad values */
	for (SIZE_T i=0;i<(64/sizeof(ULONGLONG));i++)
	{
        ((ULONGLONG*)k_ipad)[i]^=0x3636363636363636;
        ((ULONGLONG*)k_opad)[i]^=0x5C5C5C5C5C5C5C5C;
	}
	/* perform inner SHA1 */
	SHA1Reset(&context);				/* init context for 1st pass */
	SHA1Input(&context,k_ipad,64);		/* start with inner pad */
	SHA1Input(&context,text,text_len);	/* then text of datagram */
	SHA1Result(&context,digest);		/* finish up 1st pass */
	/* perform outer SHA1 */
	SHA1Reset(&context);							/* init context for 2nd pass */
	SHA1Input(&context,k_opad,64);					/* start with outer pad */
	SHA1Input(&context,(BYTE*)digest,SHA1HashSize);	/* then results of 1st hash */
	SHA1Result(&context,digest);					/* finish up 2nd pass */

	SHA1SecureZeroMemory(k_ipad,sizeof(k_ipad));
	SHA1SecureZeroMemory(k_opad,sizeof(k_opad));
	SHA1SecureZeroMemory(tk,sizeof(tk));
}
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//////////////////////////////RFC 2617//////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
__inline void SHA1CvtHex(BYTE *Bin,BYTE *Hex)
{
	BYTE j;

	for (SIZE_T i=0;i<SHA1HashSize;i++)
	{
		j=(Bin[i]>>4)&0xf;
		if(j<=9)
		{
			Hex[(i*2)]=(j+'0');
		}else{
			Hex[(i*2)]=(j+'a'-10);
		}

		j=Bin[i]&0xf;
        if(j<=9)
		{
			Hex[(i*2+1)]=(j+'0');
		}else{
			Hex[(i*2+1)]=(j+'a'-10);
		}
    };
    Hex[SHA1HashHexSize]=0;
};

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
__inline void SHA1CvtStringA(BYTE *digest,LPSTR lpszDigest)
{
	SHA1CvtHex(digest,(BYTE*)lpszDigest);
};


__inline void SHA1CvtStringW(BYTE *digest,LPWSTR lpszDigest)
{
	SIZE_T i,p=0;
	for (i=0;i<SHA1HashSize;i++,p+=2)
	{
		wsprintfW((LPWSTR)(lpszDigest+p),L"%02x",digest[i]);
	}
	lpszDigest[SHA1HashHexSize]=0;
};
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
__inline void SHA1HMACGetDigest(LPVOID lpBuff,SIZE_T dwBuffSize,LPVOID lpKey,SIZE_T dwKeySize,BYTE *digest)
{
	hmac_sha1((BYTE*)lpBuff,dwBuffSize,(BYTE*)lpKey,dwKeySize,digest);
}


__inline void SHA1HMACGetStringA(LPSTR lpszBuff,SIZE_T dwBuffSize,LPSTR lpszKey,SIZE_T dwKeySize,LPSTR lpszDigest)
{
	BYTE digest[SHA1HashSize];
	hmac_sha1((BYTE*)lpszBuff,dwBuffSize,(BYTE*)lpszKey,dwKeySize,digest);
	SHA1CvtHex(digest,(BYTE*)lpszDigest);
}


__inline void SHA1HMACGetStringW(LPWSTR lpszBuff,SIZE_T dwBuffSize,LPWSTR lpszKey,SIZE_T dwKeySize,LPWSTR lpszDigest)
{
	BYTE digest[SHA1HashSize];
	hmac_sha1((BYTE*)lpszBuff,dwBuffSize,(BYTE*)lpszKey,dwKeySize,digest);
	SHA1CvtStringW(digest,lpszDigest);
}



__inline void SHA1GetDigest(LPVOID lpBuff,SIZE_T dwBuffSize,BYTE *digest)
{
	SHA1Context sha;

	SHA1Reset(&sha);
	SHA1Input(&sha,(BYTE*)lpBuff,dwBuffSize);
	SHA1Result(&sha,digest);
}


__inline void SHA1GetStringDigestA(LPSTR lpszBuff,SIZE_T dwBuffSize,LPSTR lpszDigest)
{
	SHA1Context sha;
	BYTE digest[SHA1HashSize];

	SHA1Reset(&sha);
	SHA1Input(&sha,(BYTE*)lpszBuff,dwBuffSize);
	SHA1Result(&sha,digest);

	SHA1CvtHex(digest,(BYTE*)lpszDigest);
}


__inline void SHA1GetStringDigestW(LPWSTR lpszBuff,SIZE_T dwBuffSize,LPWSTR lpszDigest)
{
	SHA1Context sha;
	BYTE digest[SHA1HashSize];

	SHA1Reset(&sha);
	SHA1Input(&sha,(BYTE*)lpszBuff,dwBuffSize);
	SHA1Result(&sha,digest);

	SHA1CvtStringW(digest,lpszDigest);
}




#endif //AFX__SHA1_H__INCLUDED_