// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2010 Joe Kucera
// Copyright © 2012-2017 Miranda NG Team
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// -----------------------------------------------------------------------------
//  DESCRIPTION:
//
//  Helper functions to convert text messages between different character sets.
// -----------------------------------------------------------------------------

#ifndef __I18N_H
#define __I18N_H

BOOL   CALLBACK IsUSASCII(const char *pBuffer, size_t nSize);
BOOL   CALLBACK IsUnicodeAscii(const WCHAR *pBuffer, size_t nSize);
int    CALLBACK UTF8_IsValid(const char *pszInput);

int    CALLBACK get_utf8_size(const WCHAR *unicode);

char*  CALLBACK detect_decode_utf8(const char *from);

WCHAR* CALLBACK make_unicode_string(const char *utf8);
WCHAR* CALLBACK make_unicode_string_static(const char *utf8, WCHAR *unicode, size_t unicode_size);

char*  CALLBACK make_utf8_string(const WCHAR *unicode);
char*  CALLBACK make_utf8_string_static(const WCHAR *unicode, char *utf8, size_t utf_size);

char*  CALLBACK ansi_to_utf8(const char *ansi);
char*  CALLBACK ansi_to_utf8_codepage(const char *ansi, WORD wCp);

WCHAR* CALLBACK ansi_to_unicode(const char *ansi);
char*  CALLBACK unicode_to_ansi(const WCHAR *unicode);
char*  CALLBACK unicode_to_ansi_static(const WCHAR *unicode, char *ansi, size_t ansi_size);

int    CALLBACK utf8_encode(const char *from, char **to);
int    CALLBACK utf8_decode(const char *from, char **to);
int    CALLBACK utf8_decode_codepage(const char *from, char **to, WORD wCp);
int    CALLBACK utf8_decode_static(const char *from, char *to, size_t to_size);

void InitI18N(void);

#endif /* __I18N_H */
