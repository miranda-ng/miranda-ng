/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

 Copyright 2000 Alexandre Julliard of Wine project 
 (UTF-8 conversion routines)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "dbchecker.h"

bool is_utf8_string(const char* str)
{
	int expect_bytes = 0, utf_found = 0;

	if (!str) return 0;

	while (*str) {
		if ((*str & 0x80) == 0) {
			/* Looks like an ASCII character */
			if (expect_bytes)
				/* byte of UTF-8 character expected */
				return 0;
		}
		else {
			/* Looks like byte of an UTF-8 character */
			if (expect_bytes) {
				/* expect_bytes already set: first byte of UTF-8 char already seen */
				if ((*str & 0xC0) != 0x80) {
					/* again first byte ?!?! */
					return 0;
				}
			}
			else {
				/* First byte of the UTF-8 character */
				/* count initial one bits and set expect_bytes to 1 less */
				char ch = *str;
				while (ch & 0x80) {
					expect_bytes++;
					ch = (ch & 0x7f) << 1;
				}
			}
			/* OK, next byte of UTF-8 character */
			/* Decrement number of expected bytes */
			if (--expect_bytes == 0)
				utf_found = 1;
		}
		str++;
	}

	return (utf_found && expect_bytes == 0);
}
