//
// Utils.cpp
//

#include "stdafx.h"

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

