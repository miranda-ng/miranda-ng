#include "td/utils/common.h"

/*
 * Copyright (c) 1995, 1996, 1997 Kungliga Tekniska Högskolan
 * (Royal Institute of Technology, Stockholm, Sweden).
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
 
static const char *
inet_ntop_v4 (const void *src, char *dst, size_t size)
{
    const char digits[] = "0123456789";
    int i;
    struct in_addr *addr = (struct in_addr *)src;
    u_long a = ntohl(addr->s_addr);
    const char *orig_dst = dst;

    if (size < INET_ADDRSTRLEN) {
        errno = ENOSPC;
        return NULL;
    }
    for (i = 0; i < 4; ++i) {
    int n = (a >> (24 - i * 8)) & 0xFF;
    int non_zerop = 0;

    if (non_zerop || n / 100 > 0) {
        *dst++ = digits[n / 100];
        n %= 100;
        non_zerop = 1;
    }
    if (non_zerop || n / 10 > 0) {
        *dst++ = digits[n / 10];
        n %= 10;
        non_zerop = 1;
    }
    *dst++ = digits[n];
    if (i != 3)
        *dst++ = '.';
    }
    *dst++ = '\0';
    return orig_dst;
}

static const char *
inet_ntop_v6 (const void *src, char *dst, size_t size)
{
    const char xdigits[] = "0123456789abcdef";
    int i;
    const struct in6_addr *addr = (struct in6_addr *)src;
    const u_char *ptr = addr->s6_addr;
    const char *orig_dst = dst;
    int compressed = 0;

    if (size < INET6_ADDRSTRLEN) {
        errno = ENOSPC;
        return NULL;
    }
    for (i = 0; i < 8; ++i) {
    int non_zerop = 0;

        if (compressed == 0 &&
            ptr[0] == 0 && ptr[1] == 0 &&
            i <= 5 &&
            ptr[2] == 0 && ptr[3] == 0 &&
            ptr[4] == 0 && ptr[5] == 0) {

            compressed = 1;

            if (i == 0)
                *dst++ = ':';
            *dst++ = ':';

            for (ptr += 6, i += 3;
                 i < 8 && ptr[0] == 0 && ptr[1] == 0;
                 ++i, ptr += 2);

            if (i >= 8)
                break;
        }

    if (non_zerop || (ptr[0] >> 4)) {
        *dst++ = xdigits[ptr[0] >> 4];
        non_zerop = 1;
    }
    if (non_zerop || (ptr[0] & 0x0F)) {
        *dst++ = xdigits[ptr[0] & 0x0F];
        non_zerop = 1;
    }
    if (non_zerop || (ptr[1] >> 4)) {
        *dst++ = xdigits[ptr[1] >> 4];
        non_zerop = 1;
    }
    *dst++ = xdigits[ptr[1] & 0x0F];
    if (i != 7)
        *dst++ = ':';
    ptr += 2;
    }
    *dst++ = '\0';
    return orig_dst;
}

LPCSTR WSAAPI inet_ntop_(INT af, PVOID src, LPSTR dst, size_t size)
{
    LPCSTR pdst;
    if (!dst)
    {
        SetLastError( STATUS_INVALID_PARAMETER );
        return NULL;
    }
    switch (af) {
    case AF_INET :
        pdst = inet_ntop_v4 (src, dst, size);

    case AF_INET6 :
        pdst = inet_ntop_v6 (src, dst, size);

    default:
        WSASetLastError( WSAEAFNOSUPPORT );
        return NULL;
    }

    if (!pdst) SetLastError( STATUS_INVALID_PARAMETER );
    return pdst;
}

INT WSAAPI inet_pton_(INT af, LPCSTR csrc, PVOID dst)
{
    char * src;

    if (!dst || !csrc)
    {
        WSASetLastError(WSAEFAULT);
        return SOCKET_ERROR;
    }

    if (csrc == NULL || (src = strdup(csrc)) == NULL) {
        SetLastError( ENOMEM );
        return 0;
    }

    switch (af) {
    case AF_INET:
    {
        struct sockaddr_in  si4;
        INT r;
        INT s = sizeof(si4);

        si4.sin_family = AF_INET;
        r = WSAStringToAddressA(src, AF_INET, NULL, (LPSOCKADDR) &si4, &s);
        free(src);
        src = NULL;

        if (r == 0) {
            memcpy(dst, &si4.sin_addr, sizeof(si4.sin_addr));
            return 1;
        }
    }
    break;

    case AF_INET6:
    {
        struct sockaddr_in6 si6;
        INT r;
        INT s = sizeof(si6);

        si6.sin6_family = AF_INET6;
        r = WSAStringToAddressA(src, AF_INET6, NULL, (LPSOCKADDR) &si6, &s);
        free(src);
        src = NULL;

        if (r == 0) {
            memcpy(dst, &si6.sin6_addr, sizeof(si6.sin6_addr));
            return 1;
        }
    }
    break;

    default:
        WSASetLastError( WSAEAFNOSUPPORT );
        return SOCKET_ERROR;
    }

    /* the call failed */
    {
    int le = WSAGetLastError();

    if (le == WSAEINVAL)
        return SOCKET_ERROR;

    WSASetLastError(le);
    return -1;
    }
}
