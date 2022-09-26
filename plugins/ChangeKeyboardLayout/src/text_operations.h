#ifndef M_CKL_TEXT_OPERATIONS_H
#define M_CKL_TEXT_OPERATIONS_H

#include "stdafx.h"

LPSTR GetNameOfLayout(HKL hklLayout);
wchar_t *GetShortNameOfLayout(HKL hklLayout);
wchar_t *GenerateLayoutString(HKL hklLayout);
wchar_t *GetLayoutString(HKL hklLayout);
HKL GetLayoutOfText(LPCTSTR ptzsInText);
wchar_t *ChangeTextCase(LPCTSTR ptszInText);
wchar_t *ChangeTextLayout(LPCTSTR ptzsInText, HKL hklCurLay, HKL hklToLay, BOOL TwoWay);
int ChangeLayout(HWND hTextWnd, uint8_t bTextOperation, BOOL CurrentWord);

#endif