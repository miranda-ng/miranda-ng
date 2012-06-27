/*
  Name: NewEventNotify - Plugin for Miranda IM
  File: utils.c - Utility functions for Unicode support & other tiny stuff
  Version: 2.2.0
  Description: Notifies you when you receive a message
  Author: Joe @ Whale, <jokusoftware@miranda-im.org>
  Date: 05.07.07 13:04
  Copyright: (C) 2007-2008 Joe @ Whale

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "neweventnotify.h"


BOOL __fastcall NENGetSettingBool(char* szSetting, BOOL bDef)
{
  return (BOOL)DBGetContactSettingByte(NULL, MODULE, szSetting, bDef);
}

int __fastcall NENWriteSettingBool(char* szSetting, BOOL bValue)
{
  return DBWriteContactSettingByte(NULL, MODULE, szSetting, (BYTE)bValue);
}

/* a strlen() that likes NULL */
size_t __fastcall strlennull(const char *string)
{
  if (string)
    return strlen(string);

  return 0;
}

size_t __fastcall strlenT(const TCHAR *string)
{
  if (string)
  {
    if (g_UnicodeCore)
      return wcslen((WCHAR*)string);
    else
      return strlen((char*)string);
  }
  return 0;
}

TCHAR* __fastcall strdupT(const TCHAR *string)
{
  if (string)
  {
    if (g_UnicodeCore)
      return (TCHAR*)wcsdup((WCHAR*)string);
    else
      return (TCHAR*)strdup((char*)string);
  }
  return NULL;
}

TCHAR* __fastcall strcpyT(TCHAR* dest, const TCHAR* src)
{
  if (src)
  {
    if (g_UnicodeCore)
      return (TCHAR*)wcscpy((WCHAR*)dest, (WCHAR*)src);
    else
      return (TCHAR*)strcpy((char*)dest, (char*)src);
  }
  return dest;
}

TCHAR* __fastcall strncpyT(TCHAR* dest, const TCHAR* src, size_t len)
{
  if (src)
  {
    if (g_UnicodeCore)
      return (TCHAR*)wcsncpy((WCHAR*)dest, (WCHAR*)src, len);
    else
      return (TCHAR*)strncpy((char*)dest, (char*)src, len);
  }
  return dest;
}

TCHAR* __fastcall strcatT(TCHAR* dest, const TCHAR* src)
{
  if (src)
  {
    if (g_UnicodeCore)
      return (TCHAR*)wcscat((WCHAR*)dest, (WCHAR*)src);
    else
      return (TCHAR*)strcat((char*)dest, (char*)src);
  }
  return dest;
}

int _snprintfT(TCHAR *buffer, size_t count, const char* fmt, ...)
{
  va_list va;
  int len;

  va_start(va, fmt);
  if (g_UnicodeCore)
  {
    TCHAR* wfmt = ansi_to_tchar(fmt);

    len = _vsnwprintf((WCHAR*)buffer, count-1, (WCHAR*)wfmt, va);
    ((WCHAR*)buffer)[count-1] = 0;
    free(wfmt);
  }
  else
  {
    len = _vsnprintf((char*)buffer, count-1, fmt, va);
    ((char*)buffer)[count-1] = 0;
  }
  va_end(va);
  return len;
}

TCHAR* __fastcall NENTranslateT(const char* src, const WCHAR* unibuf)
{ // this takes Ascii strings only!!!
  char* szRes = NULL;

  if (!strlennull(src))
  { // for the case of empty strings
    return _T("");
  }

  if (g_UnicodeCore)
  { // we give WCHAR
    WCHAR *unicode;
    int wchars, err;

    wchars = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, src,
      strlennull(src), NULL, 0);

    if (wchars == 0) return NULL; // Failure

    unicode = (WCHAR*)unibuf;
    if (!unicode)
      unicode = (WCHAR*)malloc((wchars + 1) * sizeof(WCHAR));

    unicode[wchars] = 0;

    err = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, src,
      strlennull(src), unicode, wchars);
    if (err != wchars) return NULL; // Failure

    return (TCHAR*)TranslateW(unicode);
  }
  else
    return (TCHAR*)Translate(src);
}

static BOOL bHasCP_UTF8 = FALSE;


void InitI18N(void)
{
  CPINFO CPInfo;


  bHasCP_UTF8 = GetCPInfo(CP_UTF8, &CPInfo);
}


// Scans a string encoded in UTF-8 to verify that it contains
// only valid sequences. It will return 1 if the string contains
// only legitimate encoding sequences; otherwise it will return 0;
// From 'Secure Programming Cookbook', John Viega & Matt Messier, 2003
int UTF8_IsValid(const unsigned char* pszInput)
{
  int nb, i;
  const unsigned char* c = pszInput;

  if (!pszInput) return 0;

  for (c = pszInput; *c; c += (nb + 1))
  {
    if (!(*c & 0x80))
      nb = 0;
    else if ((*c & 0xc0) == 0x80) return 0;
    else if ((*c & 0xe0) == 0xc0) nb = 1;
    else if ((*c & 0xf0) == 0xe0) nb = 2;
    else if ((*c & 0xf8) == 0xf0) nb = 3;
    else if ((*c & 0xfc) == 0xf8) nb = 4;
    else if ((*c & 0xfe) == 0xfc) nb = 5;

    for (i = 1; i<=nb; i++) // we this forward, do not cross end of string
      if ((*(c + i) & 0xc0) != 0x80)
        return 0;
  }

  return 1;
}


/*
 * The following UTF8 routines are
 *
 * Copyright (C) 2001 Peter Harris <peter.harris@hummingbird.com>
 * Copyright (C) 2001 Edmund Grimley Evans <edmundo@rano.org>
 *
 * under a GPL license
 *
 * --------------------------------------------------------------
 * Convert a string between UTF-8 and the locale's charset.
 * Invalid bytes are replaced by '#', and characters that are
 * not available in the target encoding are replaced by '?'.
 *
 * If the locale's charset is not set explicitly then it is
 * obtained using nl_langinfo(CODESET), where available, the
 * environment variable CHARSET, or assumed to be US-ASCII.
 *
 * Return value of conversion functions:
 *
 *  -1 : memory allocation failed
 *   0 : data was converted exactly
 *   1 : valid data was converted approximately (using '?')
 *   2 : input was invalid (but still converted, using '#')
 *   3 : unknown encoding (but still converted, using '?')
 */



/*
 * Convert a string between UTF-8 and the locale's charset.
 */
unsigned char *make_utf8_string(const wchar_t *unicode)
{
  int size = 0;
  int index = 0;
  int out_index = 0;
  unsigned char* out;
  unsigned short c;

  if (!unicode) return NULL;

  /* first calculate the size of the target string */
  c = unicode[index++];
  while (c)
  {
    if (c < 0x0080) 
      size += 1;
    else if (c < 0x0800) 
      size += 2;
    else 
      size += 3;
    c = unicode[index++];
  }

  out = (unsigned char*)malloc(size + 1);
  if (out == NULL)
    return NULL;
  index = 0;

  c = unicode[index++];
  while (c)
  {
    if (c < 0x080) 
    {
      out[out_index++] = (unsigned char)c;
    }
    else if (c < 0x800) 
    {
      out[out_index++] = 0xc0 | (c >> 6);
      out[out_index++] = 0x80 | (c & 0x3f);
    }
    else
    {
      out[out_index++] = 0xe0 | (c >> 12);
      out[out_index++] = 0x80 | ((c >> 6) & 0x3f);
      out[out_index++] = 0x80 | (c & 0x3f);
    }
    c = unicode[index++];
  }
  out[out_index] = 0x00;

  return out;
}



WCHAR *make_unicode_string(const unsigned char *utf8)
{
  int size = 0, index = 0, out_index = 0;
  wchar_t *out;
  unsigned char c;

  if (!utf8) return NULL;

  /* first calculate the size of the target string */
  c = utf8[index++];
  while (c) 
  {
    if ((c & 0x80) == 0) 
    {
      index += 0;
    }
    else if ((c & 0xe0) == 0xe0) 
    {
      index += 2;
    }
    else
    {
      index += 1;
    }
    size += 1;
    c = utf8[index++];
  }

  out = (wchar_t*)malloc((size + 1) * sizeof(wchar_t));
  if (out == NULL)
    return NULL;
  index = 0;

  c = utf8[index++];
  while (c)
  {
    if((c & 0x80) == 0) 
    {
      out[out_index++] = c;
    } 
    else if((c & 0xe0) == 0xe0) 
    {
      out[out_index] = (c & 0x1F) << 12;
      c = utf8[index++];
      out[out_index] |= (c & 0x3F) << 6;
      c = utf8[index++];
      out[out_index++] |= (c & 0x3F);
    }
    else
    {
      out[out_index] = (c & 0x3F) << 6;
      c = utf8[index++];
      out[out_index++] |= (c & 0x3F);
    }
    c = utf8[index++];
  }
  out[out_index] = 0;

  return out;
}



int utf8_encode(const char *from, char **to)
{
  wchar_t *unicode;
  int wchars, err;


  wchars = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, from,
      strlennull(from), NULL, 0);

  if (wchars == 0)
  {
//    fprintf(stderr, "Unicode translation error %d\n", GetLastError());
    return -1;
  }

  unicode = (wchar_t*)mir_alloc((wchars + 1) * sizeof(unsigned short));
  unicode[wchars] = 0;

  err = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, from,
      strlennull(from), unicode, wchars);
  if(err != wchars)
  {
//    fprintf(stderr, "Unicode translation error %d\n", GetLastError());
    return -1;
  }

  /* On NT-based windows systems, we could use WideCharToMultiByte(), but
   * MS doesn't actually have a consistent API across win32.
   */
  *to = make_utf8_string(unicode);

  return 0;
}



// Returns 0 on error, 1 on success
int utf8_decode(const char *from, char **to)
{
  int nResult = 0;

//  _ASSERTE(!(*to)); // You passed a non-zero pointer, make sure it doesnt point to unfreed memory

  // Validate the string
  if (!UTF8_IsValid(from))
    return 0;

  // Use the native conversion routines when available
  if (bHasCP_UTF8)
  {
    WCHAR *wszTemp = NULL;
    int inlen = strlennull(from);

    wszTemp = (WCHAR *)mir_alloc(sizeof(WCHAR) * (inlen + 1));

    // Convert the UTF-8 string to UCS
    if (MultiByteToWideChar(CP_UTF8, 0, from, -1, wszTemp, inlen + 1))
    {
      // Convert the UCS string to local ANSI codepage
      *to = (char*)malloc(inlen+1);
      if (WideCharToMultiByte(CP_ACP, 0, wszTemp, -1, *to, inlen+1, NULL, NULL))
      {
        nResult = 1;
      }
      else
      {
        free(*to);
      }
    }
  }
  else
  {
    wchar_t *unicode;
    int chars;
    int err;

    unicode = make_unicode_string(from);
    if (unicode == NULL)
    {
//      fprintf(stderr, "Out of memory processing string from UTF8 to UNICODE16\n");
      return 0;
    }

    chars = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, unicode, -1, NULL, 0, NULL, NULL);

    if(chars == 0)
    {
//      fprintf(stderr, "Unicode translation error %d\n", GetLastError());
      free(unicode);
      return 0;
    }

    *to = (char*)malloc((chars + 1)*sizeof(unsigned char));
    if(*to == NULL)
    {
//      fprintf(stderr, "Out of memory processing string to local charset\n");
      free(unicode);
      return 0;
    }

    err = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, unicode, -1, *to, chars, NULL, NULL);
    if (err != chars)
    {
//      fprintf(stderr, "Unicode translation error %d\n", GetLastError());
      free(unicode);
      free(*to);
      return 0;
    }

    free(unicode);

    nResult = 1;
  }

  return nResult;
}



TCHAR* ansi_to_tchar(const char* src)
{
  if (g_UnicodeCore)
  {
    WCHAR *unicode;
    int wchars, err;

    wchars = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, src, strlennull(src), NULL, 0);

    if (wchars == 0) return NULL; // Failure

    unicode = (WCHAR*)malloc((wchars + 1) * sizeof(WCHAR));
    unicode[wchars] = 0;

    err = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, src, strlennull(src), unicode, wchars);
    if (err != wchars)
    {
      free(unicode);
      return NULL; // Failure
    }

    return (TCHAR*)unicode;
  }
  else
    return strdupT((TCHAR*)src);
}

char* tchar_to_ansi(const TCHAR* src)
{
  if (g_UnicodeCore)
  {
    char *ansi;
    int chars;
    int err;

    chars = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, (WCHAR*)src, -1, NULL, 0, NULL, NULL);

    if (chars == 0) return NULL; // Failure

    ansi = (char*)malloc((chars + 1)*sizeof(char));
    if (ansi == NULL) return NULL; // Failure

    err = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, (WCHAR*)src, -1, ansi, chars, NULL, NULL);
    if (err != chars)
    {
      free(ansi);
      return NULL;
    }
    return ansi;
  }
  else
    return (char*)strdupT(src);
}

TCHAR* utf8_to_tchar(const unsigned char* utf)
{
  if (g_UnicodeCore)
    return (TCHAR*)make_unicode_string(utf);
  else
  {
    char* szAnsi = NULL;

    if (utf8_decode(utf, &szAnsi)) 
      return (TCHAR*)szAnsi;
    else
      return NULL; // Failure
  }
}