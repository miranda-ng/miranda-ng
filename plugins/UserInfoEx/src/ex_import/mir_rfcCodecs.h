/*
UserinfoEx plugin for Miranda IM

Copyright:
� 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

===============================================================================

File name      : $HeadURL: https://userinfoex.googlecode.com/svn/trunk/ex_import/mir_rfcCodecs.h $
Revision       : $Revision: 190 $
Last change on : $Date: 2010-09-14 14:32:57 +0400 (Вт, 14 сен 2010) $
Last change by : $Author: ing.u.horn $

===============================================================================
*/

#include <stdlib.h>

//Not including CRLFs
//NOTE: For BASE64 and UUENCODE, this actually
//represents the amount of unencoded characters
//per line
#define TSSMTPMAX_QP_LINE_LENGTH			 76
#define TSSMTPMAX_BASE64_LINE_LENGTH	 57
#define TSSMTPMAX_UUENCODE_LINE_LENGTH 45

//=======================================================================
// Base64Encode/Base64Decode
// compliant with RFC 2045
//=======================================================================
//
#define BASE64_FLAG_NONE	0
#define BASE64_FLAG_NOPAD	1
#define BASE64_FLAG_NOCRLF	2

inline INT_PTR Base64EncodeGetRequiredLength(INT_PTR nSrcLen, DWORD dwFlags = BASE64_FLAG_NONE)
{
	INT_PTR nRet = nSrcLen*4/3;

	if ((dwFlags & BASE64_FLAG_NOPAD) == 0)
		nRet += nSrcLen % 3;

	INT_PTR nCRLFs = nRet / 76 + 3;
	INT_PTR nOnLastLine = nRet % 76;

	if (nOnLastLine) {
		if (nOnLastLine % 4)
			nRet += 4 - (nOnLastLine % 4);
	}

	nCRLFs *= 2;

	if ((dwFlags & BASE64_FLAG_NOCRLF) == 0)
		nRet += nCRLFs;

	return nRet;
}

inline INT_PTR Base64DecodeGetRequiredLength(INT_PTR nSrcLen)
{
	return nSrcLen;
}

inline BOOL Base64Encode(
	const BYTE *pbSrcData,
	INT_PTR nSrcLen,
	LPSTR szDest,
	INT_PTR *pnDestLen,
	DWORD dwFlags = BASE64_FLAG_NONE)
{
	static const char s_chBase64EncodingTable[64] = {
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q',
		'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g',	'h',
		'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y',
		'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/' };

	if (!pbSrcData || !szDest || !pnDestLen)
		return FALSE;

	INT_PTR nWritten(0);
	INT_PTR nLen1((nSrcLen / 3) * 4);
	INT_PTR nLen2(nLen1 / 76);
	INT_PTR nLen3(19);
	INT_PTR i, j, k, n;

	for (i = 0; i <= nLen2; i++) {
		if (i == nLen2)
			nLen3 = (nLen1 % 76) / 4;

		for (j = 0; j < nLen3; j++) {
			DWORD dwCurr(0);
			for (INT_PTR n = 0; n < 3; n++)	{
				dwCurr |= *pbSrcData++;
				dwCurr <<= 8;
			}
			for (k = 0; k < 4; k++) {
				BYTE b = (BYTE)(dwCurr >> 26);
				*szDest++ = s_chBase64EncodingTable[b];
				dwCurr <<= 6;
			}
		}
		nWritten += nLen3 * 4;

		if ((dwFlags & BASE64_FLAG_NOCRLF) == 0) {
			*szDest++ = '\r';
			*szDest++ = '\n';
			*szDest++ = '\t';		// as vcards have tabs in second line of binary data
			nWritten += 3;
		}
	}

	if (nWritten && (dwFlags & BASE64_FLAG_NOCRLF) == 0) {
		szDest -= 2;
		nWritten -= 2;
	}

	nLen2 = nSrcLen % 3 ? nSrcLen % 3 + 1 : 0;
	if (nLen2) {
		DWORD dwCurr(0);
		for (n = 0; n < 3; n++)
		{
			if (n < (nSrcLen % 3))
				dwCurr |= *pbSrcData++;
			dwCurr <<= 8;
		}
		for (k = 0; k < nLen2; k++) {
			BYTE b = (BYTE)(dwCurr >> 26);
			*szDest++ = s_chBase64EncodingTable[b];
			dwCurr <<= 6;
		}
		nWritten+= nLen2;
		if ((dwFlags & BASE64_FLAG_NOPAD) == 0) {
			nLen3 = nLen2 ? 4 - nLen2 : 0;
			for (j = 0; j < nLen3; j++)	{
				*szDest++ = '=';
			}
			nWritten+= nLen3;
		}
	}

	*pnDestLen = nWritten;
	return TRUE;
}

inline INT_PTR DecodeBase64Char(UINT ch) throw()
{
	// returns -1 if the character is invalid
	// or should be skipped
	// otherwise, returns the 6-bit code for the character
	// from the encoding table
	if (ch >= 'A' && ch <= 'Z')
		return ch - 'A' + 0;	// 0 range starts at 'A'
	if (ch >= 'a' && ch <= 'z')
		return ch - 'a' + 26;	// 26 range starts at 'a'
	if (ch >= '0' && ch <= '9')
		return ch - '0' + 52;	// 52 range starts at '0'
	if (ch == '+')
		return 62;
	if (ch == '/')
		return 63;
	return -1;
}

inline BOOL Base64Decode(LPCSTR szSrc, INT_PTR nSrcLen, BYTE *pbDest, INT_PTR *pnDestLen) throw()
{
	// walk the source buffer
	// each four character sequence is converted to 3 bytes
	// CRLFs and =, and any characters not in the encoding table
	// are skiped

	if (szSrc == NULL || pnDestLen == NULL) {
		return FALSE;
	}

	LPCSTR szSrcEnd = szSrc + nSrcLen;
	INT_PTR nWritten = 0;

	BOOL bOverflow = (pbDest == NULL) ? TRUE : FALSE;

	while (szSrc < szSrcEnd) {
		DWORD dwCurr = 0;
		INT_PTR i;
		INT_PTR nBits = 0;
		for (i=0; i<4; i++) {
			if (szSrc >= szSrcEnd)
				break;
			INT_PTR nCh = DecodeBase64Char(*szSrc);
			szSrc++;
			if (nCh == -1) {
				// skip this char
				i--;
				continue;
			}
			dwCurr <<= 6;
			dwCurr |= nCh;
			nBits += 6;
		}

		if (!bOverflow && nWritten + (nBits/8) > (*pnDestLen))
			bOverflow = TRUE;

		// dwCurr has the 3 bytes to write to the output buffer
		// left to right
		dwCurr <<= 24-nBits;
		for (i=0; i<nBits/8; i++) {
			if (!bOverflow) {
				*pbDest = (BYTE) ((dwCurr & 0x00ff0000) >> 16);
				pbDest++;
			}
			dwCurr <<= 8;
			nWritten++;
		}
	}
	*pnDestLen = nWritten;
	return bOverflow ? FALSE:TRUE;
}

//=======================================================================
// Quoted Printable encode/decode
// compliant with RFC 2045
//=======================================================================
//
#define TSSMTPQPENCODE_DOT 1
#define TSSMTPQPENCODE_TRAILING_SOFT 2

inline INT_PTR QPEncodeGetRequiredLength(INT_PTR nSrcLen)
{
	INT_PTR nRet = 3*((3*nSrcLen)/(TSSMTPMAX_QP_LINE_LENGTH-8));
	nRet += 3*nSrcLen;
	nRet += 3;
	return nRet;
}

inline INT_PTR QPDecodeGetRequiredLength(INT_PTR nSrcLen)
{
	return nSrcLen;
}

inline BOOL QPEncode(BYTE* pbSrcData, INT_PTR nSrcLen, LPSTR szDest, INT_PTR* pnDestLen, BOOLEAN *bEncoded, DWORD dwFlags = 0)
{
	//The hexadecimal character set
	static const CHAR s_chHexChars[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 
											'A', 'B', 'C', 'D', 'E', 'F'};
	INT_PTR nRead = 0, nWritten = 0, nLineLen = 0;
	CHAR ch;
	BOOLEAN bChanged = FALSE;


	if (!pbSrcData || !szDest || !pnDestLen) {
		return FALSE;
	}

	while (nRead < nSrcLen) {
		ch = *pbSrcData++;
		nRead++;
		if (nLineLen == 0 && ch == '.' && (dwFlags & TSSMTPQPENCODE_DOT)) {
			*szDest++ = '.';
			nWritten++;
			nLineLen++;
			bChanged = TRUE;
		}
		if ((ch > 32 && ch < 61) || (ch > 61 && ch < 127)) {
			*szDest++ = ch;
			nWritten++;
			nLineLen++;
		}
		else
		if ((ch == ' ' || ch == '\t') && (nLineLen < (TSSMTPMAX_QP_LINE_LENGTH - 12))) {
			*szDest++ = ch;
			nWritten++;
			nLineLen++;
		}	
		else {
			*szDest++ = '=';
			*szDest++ = s_chHexChars[(ch >> 4) & 0x0F];
			*szDest++ = s_chHexChars[ch & 0x0F];
			nWritten += 3;
			nLineLen += 3;
			bChanged = TRUE;
		}
		if (nLineLen >= (TSSMTPMAX_QP_LINE_LENGTH - 11)) {
			*szDest++ = '=';
			*szDest++ = '\r';
			*szDest++ = '\n';
			nLineLen = 0;
			nWritten += 3;
			bChanged = TRUE;
		}
	}
	if (dwFlags & TSSMTPQPENCODE_TRAILING_SOFT) {
		*szDest++ = '=';
		*szDest++ = '\r';
		*szDest++ = '\n';
		nWritten += 3;
		bChanged = TRUE;
	}
	*pnDestLen = nWritten;
	if (bEncoded) *bEncoded = bChanged;
	return TRUE;
}


inline BOOL QPDecode(BYTE* pbSrcData, INT_PTR nSrcLen, LPSTR szDest, INT_PTR* pnDestLen, DWORD dwFlags = 0)
{
	if (!pbSrcData || !szDest || !pnDestLen)
	{
		return FALSE;
	}

	INT_PTR nRead = 0, nWritten = 0, nLineLen = -1;
	char ch;
	while (nRead <= nSrcLen)
	{
		ch = *pbSrcData++;
		nRead++;
		nLineLen++;
		if (ch == '=')
		{
			//if the next character is a digit or a character, convert
			if (nRead < nSrcLen && (isdigit(*pbSrcData) || isalpha(*pbSrcData)))
			{
				char szBuf[5];
				szBuf[0] = *pbSrcData++;
				szBuf[1] = *pbSrcData++;
				szBuf[2] = '\0';
				char* tmp = '\0';
				*szDest++ = (BYTE)strtoul(szBuf, &tmp, 16);
				nWritten++;
				nRead += 2;
				continue;
			}
			//if the next character is a carriage return or line break, eat it
			if (nRead < nSrcLen && *pbSrcData == '\r' && (nRead+1 < nSrcLen) && *(pbSrcData+1)=='\n')
			{
				pbSrcData++;
				nRead++;
				nLineLen = -1;
				continue;
			}
			return FALSE;
		}
		if (ch == '\r' || ch == '\n')
		{
			nLineLen = -1;
			continue;
		}
		if ((dwFlags & TSSMTPQPENCODE_DOT) && ch == '.' && nLineLen == 0)
		{
			continue;
		}
		*szDest++ = ch;
		nWritten++;
	}

	*pnDestLen = nWritten-1;
	return TRUE;
}
