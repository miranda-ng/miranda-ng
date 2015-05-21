#include "commonheaders.h"

void *operator new(size_t sz)
{
	return mir_calloc(sz);
}

void *operator new[](size_t size)
{
	return operator new(size);
}

void operator delete(void *p)
{
	mir_free(p);
}

void operator delete[](void *p)
{
	operator delete(p);
}

// ANSIzUCS2z + ANSIzUCS2z = ANSIzUCS2z
char* m_wwstrcat(LPCSTR strA, LPCSTR strB)
{
	int lenA = (int)strlen(strA);
	int lenB = (int)strlen(strB);
	LPSTR str = (LPSTR)mir_alloc((lenA + lenB + 1)*(sizeof(WCHAR) + 1));
	memcpy(str, strA, lenA);
	memcpy(str + lenA, strB, lenB + 1);
	memcpy(str + lenA + lenB + 1, strA + lenA + 1, lenA*sizeof(WCHAR));
	memcpy(str + lenA + lenB + 1 + lenA*sizeof(WCHAR), strB + lenB + 1, (lenB + 1)*sizeof(WCHAR));
	return str;
}

// ANSIz + ANSIzUCS2z = ANSIzUCS2z
char* m_awstrcat(LPCSTR strA, LPCSTR strB)
{
	int lenA = (int)strlen(strA);
	LPSTR tmpA = (LPSTR)mir_alloc((lenA + 1)*(sizeof(WCHAR) + 1));
	strcpy(tmpA, strA);
	MultiByteToWideChar(CP_ACP, 0, strA, -1, (LPWSTR)(tmpA + lenA + 1), (lenA + 1)*sizeof(WCHAR));
	LPSTR str = m_wwstrcat(tmpA, strB);
	mir_free(tmpA);
	return str;
}

// ANSIz + ANSIz = ANSIzUCS2z
char* m_aastrcat(LPCSTR strA, LPCSTR strB)
{
	int lenA = (int)strlen(strA);
	int lenB = (int)strlen(strB);
	LPSTR str = (LPSTR)mir_alloc((lenA + lenB + 1)*(sizeof(WCHAR) + 1));
	strcpy(str, strA);
	strcat(str, strB);
	MultiByteToWideChar(CP_ACP, 0, str, -1, (LPWSTR)(str + lenA + lenB + 1), (lenA + lenB + 1)*sizeof(WCHAR));
	return str;
}

LPSTR m_string = NULL;

// ANSIz + ANSIz = ANSIz
char* m_ustrcat(LPCSTR strA, LPCSTR strB)
{
	SAFE_FREE(m_string);
	m_string = (LPSTR)mir_alloc(strlen(strA) + strlen(strB) + 1);
	strcpy(m_string, strA); strcat(m_string, strB);
	return m_string;
}

LPSTR m_hex = NULL;

LPSTR to_hex(PBYTE bin, int len)
{
	SAFE_FREE(m_hex);
	m_hex = (LPSTR)mir_alloc(len * 3 + 1);
	LPSTR m_ptr = m_hex;
	for (int i = 0; i < len; i++) {
		if (i) {
			*m_ptr = ' '; m_ptr++;
		}
		mir_snprintf(m_ptr, 4, "%02X", bin[i]);
		m_ptr += 2;
	}
	*m_ptr = 0;
	return m_hex;
}

void __fastcall safe_free(void** p)
{
	if (*p) {
		mir_free(*p);
		*p = NULL;
	}
}

void __fastcall safe_delete(void** p)
{
	if (*p) {
		delete(*p);
		*p = NULL;
	}
}

// ����������� ����� �� ������� UTF8 � ������ �������
LPSTR utf8_to_miranda(LPCSTR szUtfMsg, DWORD& flags)
{
	return mir_strdup(szUtfMsg);
}

// ����������� ����� �� ������� ������� � ������ UTF8
LPSTR miranda_to_utf8(LPCSTR szMirMsg, DWORD flags)
{
	return mir_strdup(szMirMsg);
}
