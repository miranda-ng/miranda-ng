#ifndef M_CKL_TEXT_OPERATIONS_H
#define M_CKL_TEXT_OPERATIONS_H

#include "stdafx.h"

BOOL CopyTextToClipboard(LPTSTR ptszText);
LPSTR GetNameOfLayout(HKL hklLayout);
LPTSTR GetShortNameOfLayout(HKL hklLayout);
LPTSTR GenerateLayoutString(HKL hklLayout);
LPTSTR GetLayoutString(HKL hklLayout);
HKL GetLayoutOfText(LPCTSTR ptzsInText);
LPTSTR ChangeTextCase(LPCTSTR ptszInText);
LPTSTR ChangeTextLayout(LPCTSTR ptzsInText, HKL hklCurLay, HKL hklToLay, BOOL TwoWay);
int ChangeLayout(HWND hTextWnd, BYTE bTextOperation, BOOL CurrentWord);

#endif