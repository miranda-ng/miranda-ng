/*

dbx_tree: tree database driver for Miranda IM

Copyright 2007-2010 Michael "Protogenes" Kunz,

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

#pragma once

inline int strcpy_s(
    char *strDestination,
    size_t numberOfElements,
    const char *strSource
)
{
    if (!strDestination) return EINVAL;
    if (!strSource) {strDestination[0] = 0; return EINVAL;}
    size_t l = strlen(strSource);
    if (numberOfElements < l + 1) {strDestination[0] = 0; return ERANGE;}
    memcpy(strDestination, strSource, l + 1);
    return 0;
}

template <size_t size>
inline int strcpy_s(
    char (&strDestination)[size],
    const char *strSource
)
{
    if (!strDestination) return EINVAL;
    if (!strSource) {strDestination[0] = 0; return EINVAL;}
    size_t l = strlen(strSource);
    if (size < l + 1) {strDestination[0] = 0; return ERANGE;}

    memcpy(strDestination, strSource, l + 1);
    return 0;
}


inline int strcat_s(
    char *strDestination,
    size_t numberOfElements,
    const char *strSource
)
{
    if (!strDestination) return EINVAL;
    if (!strSource) {strDestination[0] = 0; return EINVAL;}
    size_t l = strlen(strSource);
    size_t m = strlen(strDestination);
    if (numberOfElements < l + m + 1) {strDestination[0] = 0; return ERANGE;}

    memcpy(&strDestination[m], strSource, l + 1);
    return 0;
}

template <size_t size>
inline int strcat_s(
    char (&strDestination)[size],
    const char *strSource
)
{
    if (!strDestination) return EINVAL;
    if (!strSource) {strDestination[0] = 0; return EINVAL;}
    size_t l = strlen(strSource);
    size_t m = strlen(strDestination);
    if (size < l + m + 1) {strDestination[0] = 0; return ERANGE;}

    memcpy(&strDestination[m], strSource, l + 1);
    return 0;
}

inline int strncpy_s(
    char *strDest,
    size_t numberOfElements,
    const char *strSource,
    size_t count
)
{
    if (count > numberOfElements)
        return strcpy_s(strDest, numberOfElements, strSource);
    else
        return strcpy_s(strDest, count + 1, strSource);
}


template <size_t size>
inline int sprintf_s(
    char (&buffer)[size],
    const char *format,
    ...
)
{
    va_list va;
    va_start(va, format);
    vsnprintf(buffer, size, format, va);
    va_end(va);
}

template <size_t size>
inline int swprintf_s(
    wchar_t (&buffer)[size],
    const wchar_t *format,
    ...
)
{
    va_list va;
    va_start(va, format);
    vsnwprintf(buffer, size, format, va);
    va_end(va);
}

#define vsprintf_s vsnprintf
