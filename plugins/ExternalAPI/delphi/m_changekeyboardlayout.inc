{$IFNDEF M_CHANGEKEYBOARDLAYOUT}
{$DEFINE M_CHANGEKEYBOARDLAYOUT}

const
// change keyboard layout of text
// wParam - HWND or NULL for current window
// lParam must be 0
// returns 0 on success and returns non-zero (-1) on error.
  MS_CKL_CHANGELAYOUT:PAnsiChar = 'ChangeKeyboardLayout/ChangeLayout';

//wParam должен быть ноль.
//lParam - LPCTSTR текста, раскладку которого требуетс€ определить,
//¬озвращает HKL раскладку текста, или NULL в случае ошибки.
//ѕримечание: ѕри определении раскладки учитываетс€ опци€ "–аскладка текста - текуща€ раскладка"
  MS_CKL_GETLAYOUTOFTEXT:PAnsiChar = 'ChangeKeyboardLayout/GetLayoutOfText';

type
  CKLLayouts = record
    hklFrom:HKL;   // layout of the current text
    hklTo  :HKL;   // layout of the result text
    bTwoWay:bool;
  end;

const
//wParam - LPCTSTR исходного текста
//lParam - указатель на структуру CKLLayouts, содержащую раскладки дл€ 
//изменени€ текста и опцию "двунаправленного преобразовани€"
//¬озвращает LPTSTR на результирующую строку
  MS_CKL_CHANGETEXTLAYOUT:PAnsiChar = 'ChangeKeyboardLayout/ChangeTextLayout';

{$ENDIF}
