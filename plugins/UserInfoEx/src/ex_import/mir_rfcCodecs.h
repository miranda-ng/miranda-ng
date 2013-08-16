/*
UserinfoEx plugin for Miranda IM

Copyright:
© 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

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
*/

//Not including CRLFs
//NOTE: For BASE64 and UUENCODE, this actually
//represents the amount of unencoded characters
//per line
#define TSSMTPMAX_QP_LINE_LENGTH			 76
#define TSSMTPMAX_BASE64_LINE_LENGTH	 57
#define TSSMTPMAX_UUENCODE_LINE_LENGTH 45

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

inline BOOL QPEncode(BYTE* pbSrcData, INT_PTR nSrcLen, LPSTR szDest, INT_PTR* pnDestLen, BYTE *bEncoded, DWORD dwFlags = 0)
{
	//The hexadecimal character set
	static const CHAR s_chHexChars[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
											'A', 'B', 'C', 'D', 'E', 'F'};
	INT_PTR nRead = 0, nWritten = 0, nLineLen = 0;
	CHAR ch;
	BYTE bChanged = FALSE;


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
