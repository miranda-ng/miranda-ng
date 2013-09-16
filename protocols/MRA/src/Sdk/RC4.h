/*
 * Copyright (c) 2007 Rozhuk Ivan <rozhuk.im@gmail.com>
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



#if !defined(AFX_RC4__H__INCLUDED_)
#define AFX_RC4__H__INCLUDED_

#pragma once

__inline void RC4(LPBYTE lpBuff,size_t dwBuffSize,LPBYTE lpKey,size_t dwKeySize)
{// RC4
// www.codeproject.com/cpp/crypt_routine.asp%3Fdf%3D100%26forumid%3D4418%26exp%3D0%26select%3D251879+RC4+c%2B%2B+source+DWORD&hl=ru&gl=ru&ct=clnk&cd=2
// http://www.thecodeproject.com/cpp/crypt_routine.asp
// http://forum.ixbt.com/topic.cgi?id=40:3020

	//we will consider size of sbox 256 bytes
    //(extra byte are only to prevent any mishep just in case)
    BYTE temp;
	BYTE Sbox[260]={0},Sbox2[260]={0};
    size_t i,j,t,x;

    i=j=t=x=0;
    temp=0;

    //initialize sbox i
    for(i=0;i<256;i++) Sbox[i]=(BYTE)i;

    j=0;
    //whether user has sent any inpur lpKey
    //initialize the sbox2 with user lpKey
    for(i=0;i<256;i++)
    {
        if (j==dwKeySize) j=0;
        Sbox2[i]=lpKey[j++];
    }    

    j=0; //Initialize j
    //scramble sbox1 with sbox2
    for(i=0;i<256;i++)
    {
        j=(j+(unsigned long)Sbox[i]+(unsigned long)Sbox2[i]) % 256U;
        temp=Sbox[i];                    
        Sbox[i]=Sbox[j];
        Sbox[j]=temp;
    }

    i=j=0;
    for(x=0;x<dwBuffSize;x++)
    {
        //increment i
        i=(i+1U)%256U;
        //increment j
        j=(j+(unsigned long)Sbox[i])%256U;

        //Scramble SBox #1 further so encryption routine will
        //will repeat itself at great interval
        temp=Sbox[i];
        Sbox[i]=Sbox[j];
        Sbox[j]=temp;

        //Get ready to create pseudo random  byte for encryption lpKey
        t=((unsigned long)Sbox[i]+(unsigned long)Sbox[j])%256U;

        //get the random byte
        //xor with the data and done
        lpBuff[x]=(lpBuff[x]^Sbox[t]);
    }    
}



#endif // !defined(AFX_RC4__H__INCLUDED_)


