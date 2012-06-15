/*
 * strpos.c - Returns the offset of a string in a substring.
 *
 * Copyright (c) 2001, 2002 Andre Guibert de Bruet. <andre@siliconlandmark.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer,
 *    without modification, immediately at the beginning of the file.
 * 2. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/*
 * You might want to place this next block in a header file somewhere:
 */


# ifndef TCHAR
#  define       TCHAR   wchar_t
# endif /* TCHAR */
# define        STRING  wchar_t *




int strpos( STRING haystack, STRING needle )
{
	STRING pDest;
	int position;

#ifdef DEBUG_VERBOSE
	TCHAR lpBuf[1024];
#endif /* def DEBUG_VERBOSE */
	
	pDest = (STRING) wcsstr( haystack, needle );
	

	if ( pDest )
		position = pDest - haystack;
	else
	{
#ifdef DEBUG_VERBOSE
	printf( L"strpos(): Could not find '%s' in '%s'.\tFAIL.", needle, haystack );
#endif /* def DEBUG_VERBOSE */
		return -1;
	}

#ifdef DEBUG_VERBOSE
	printf( L"strpos(): Found '%s' at position: %d.\t\tOK.", needle, position );
#endif /* def DEBUG_VERBOSE */

	return position;
}
