//
// Utils.cpp
//

#include "stdafx.h"

BOOL IsNt50()
{
	WORD wOsVersion = LOWORD(GetVersion());
	BYTE bMajorVer = LOBYTE(wOsVersion);
	BYTE bMinorVer = HIBYTE(wOsVersion);

	return (bMajorVer>=5);
}

void TruncateWithDots(TCHAR* szString, size_t iNewLen)
{
	size_t iOrigLen = _tcslen(szString);
	if (iNewLen < iOrigLen) {
		TCHAR* p = szString+iNewLen;
		*p = _T('\0');
		for(int i=0; --p>=szString && i<3; i++)
			*p = _T('.');
	}
}

