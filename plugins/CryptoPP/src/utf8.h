#ifndef __UTF_8__
#define __UTF_8__

LPSTR __cdecl utf8encode(LPCWSTR str);
LPWSTR __cdecl utf8decode(LPCSTR str);
int __cdecl is_7bit_string(LPCSTR);
int __cdecl is_utf8_string(LPCSTR);

#endif
