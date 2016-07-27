//
// Utils.cpp
//

#include "stdafx.h"

BOOL IsNt50()
{
	WORD wOsVersion = LOWORD(GetVersion());
	BYTE bMajorVer = LOBYTE(wOsVersion);
	return (bMajorVer>=5);
}

void TruncateWithDots(wchar_t* szString, size_t iNewLen)
{
	size_t iOrigLen = mir_wstrlen(szString);
	if (iNewLen < iOrigLen) {
		wchar_t* p = szString+iNewLen;
		*p = '\0';
		for(int i=0; --p>=szString && i<3; i++)
			*p = '.';
	}
}

