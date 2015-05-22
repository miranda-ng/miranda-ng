/* -------------------------------------------------------------------- */
#include "StdAfx.h"

tstring tstringprintf(tstring strFormat,...) {
	va_list vlist;

	va_start(vlist, strFormat);
	
	int mlen = (int)strFormat.length()+128;
	TCHAR *text = (TCHAR*)malloc(mlen*sizeof(TCHAR));
	_vsntprintf(text,mlen,strFormat.c_str(),vlist);
	va_end(vlist);

	strFormat = text;
	free(text);

	return strFormat;
}

// Returns true if the unicode buffer only contains 7-bit characters.
BOOL IsUnicodeAscii(const wchar_t* pBuffer, int nSize)
{
	BOOL bResult = TRUE;
	int nIndex;

	for (nIndex = 0; nIndex < nSize; nIndex++) {
		if (pBuffer[nIndex] > 0x7F) {
			bResult = FALSE;
			break;
		}
	}
	return bResult;
}


wstring 
toWideString( const char* pStr , int len )
{
	if ( pStr == NULL )
        return L"" ;

    //ASSERT_PTR( pStr ) ; 
    ASSERT( len >= 0 || len == -1 , _T("Invalid string length: ") << len ) ; 

    // figure out how many wide characters we are going to get 
    int nChars = MultiByteToWideChar( CP_ACP , 0 , pStr , len , NULL , 0 ) ; 
    if ( len == -1 )
        -- nChars ; 
    if ( nChars == 0 )
        return L"" ;

    // convert the narrow string to a wide string 
    // nb: slightly naughty to write directly into the string like this
    wstring buf ;
    buf.resize( nChars ) ; 
    MultiByteToWideChar( CP_ACP , 0 , pStr , len , 
        const_cast<wchar_t*>(buf.c_str()) , nChars ) ; 

    return buf ;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  */

string 
toNarrowString( const wchar_t* pStr , int len )
{
    //ASSERT_PTR( pStr ) ; 
    ASSERT( len >= 0 || len == -1 , _T("Invalid string length: ") << len ) ; 

    // figure out how many narrow characters we are going to get 
    int nChars = WideCharToMultiByte( CP_ACP , 0 , 
             pStr , len , NULL , 0 , NULL , NULL ) ; 
    if ( len == -1 )
        -- nChars ; 
    if ( nChars == 0 )
        return "" ;

    // convert the wide string to a narrow string
    // nb: slightly naughty to write directly into the string like this
    string buf ;
    buf.resize( nChars  ) ;
	//char *test = (char*)malloc((nChars+1)*sizeof(char));
	WideCharToMultiByte( CP_ACP , 0 , pStr , len , 
          const_cast<char*>(buf.c_str()), nChars , NULL , NULL ) ; 

    return buf ; 
}

/// get lower string
tstring toLower(const tstring &i_str)
{
  tstring str(i_str);
  for (size_t i = 0; i < str.size(); ++ i)
  {
    if (_istlead(str[i]))
      ++ i;
    else
      str[i] = tolower(str[i]);
  }
  return str;
}

/*
 * decodes UTF-8 to unicode
 * taken from jabber protocol implementation and slightly modified
 * free() the return value
 */

#if defined(_UNICODE)


tstring Utf8_Decode(const char *str)
{
	tstring strRes = _T("");

	int i, len;
	char *p;
	WCHAR *wszTemp = NULL;

	if (str == NULL)
		return strRes;

	len = mir_strlen(str);

    if ((wszTemp = (WCHAR *) malloc(sizeof(TCHAR) * (len + 2))) == NULL)
		return strRes;
	
	p = (char *) str;
	i = 0;
	while (*p) {
		if ((*p & 0x80) == 0)
			wszTemp[i++] = *(p++);
		else if ((*p & 0xe0) == 0xe0) {
			wszTemp[i] = (*(p++) & 0x1f) << 12;
			wszTemp[i] |= (*(p++) & 0x3f) << 6;
			wszTemp[i++] |= (*(p++) & 0x3f);
		}
		else {
			wszTemp[i] = (*(p++) & 0x3f) << 6;
			wszTemp[i++] |= (*(p++) & 0x3f);
		}
	}
	wszTemp[i] = (TCHAR)'\0';

	strRes = wszTemp;
	free(wszTemp);
	return strRes;
}

/*
 * convert unicode to UTF-8
 * code taken from jabber protocol implementation and slightly modified.
 * free() the return value
 */

string Utf8_Encode(const WCHAR *str)
{
	string strRes = "";

	unsigned char *szOut = NULL;
	int len, i;
	const WCHAR *wszTemp, *w;
    
	if (str == NULL) 
        return strRes;

    wszTemp = str;

	// Convert unicode to utf8
	len = 0;
	for (w=wszTemp; *w; w++) {
		if (*w < 0x0080) len++;
		else if (*w < 0x0800) len += 2;
		else len += 3;
	}

	if ((szOut = (unsigned char *) malloc(len + 2)) == NULL)
		return strRes;

	i = 0;
	for (w=wszTemp; *w; w++) {
		if (*w < 0x0080)
			szOut[i++] = (unsigned char) *w;
		else if (*w < 0x0800) {
			szOut[i++] = 0xc0 | ((*w) >> 6);
			szOut[i++] = 0x80 | ((*w) & 0x3f);
		}
		else {
			szOut[i++] = 0xe0 | ((*w) >> 12);
			szOut[i++] = 0x80 | (((*w) >> 6) & 0x3f);
			szOut[i++] = 0x80 | ((*w) & 0x3f);
		}
	}
	szOut[i] = '\0';
	strRes = (char *) szOut;
	free(szOut);
	return strRes;
}

#endif


// Zufallszahlen
int GetRandomInt(int iMin, int iMax)
{
	double r = ((double)rand() / (RAND_MAX +1));

	int iRes = r*(iMax + 1- iMin) + iMin;
	if(iRes > iMax)
		Sleep(1);
	return iRes;
}

double GetRandomDouble()
{
	return ((double)rand() / (RAND_MAX +1));
}