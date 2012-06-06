#ifndef _M_CHANGEKEYBOARDLAYOUT_H
#define _M_CHANGEKEYBOARDLAYOUT_H

#define MIID_CKL_UNICODE {0xc5ef53a8, 0x80d4, 0x4ce9, { 0xb3, 0x41, 0xec, 0x90, 0xd3, 0xec, 0x91, 0x56 }} //{c5ef53a8-80d4-4ce9-b341-ec90d3ec9156}
#define MIID_CKL_ANSI {0x87af74ba, 0x035c, 0x4d0d, { 0xb6, 0x8d, 0xd0, 0xd6, 0xae, 0x1e, 0xbf, 0xcd }} //{87af74ba-035c-4d0d-b68d-d0d6ae1ebfcd}
// Меняет раскладку текста для окна с указанных хэндлом
// wParam - HWND окна, или NULL для окна в фокусе
// lParam должен быть 0
// Возвращает 0 в случае успеха и ненулевое значение (1) при ошибке.
// Примечание: Режим "текущего слова" определяется по соответствующей опции для основной горячей клавиши.
#define MS_CKL_CHANGELAYOUT "ChangeKeyboardLayout/ChangeLayout"

//wParam должен быть ноль.
//lParam - LPCTSTR текста, раскладку которого требуется определить,
//Возвращает HKL раскладку текста, или NULL в случае ошибки.
//Примечание: При определении раскладки учитывается опция "Раскладка текста - текущая раскладка"
#define MS_CKL_GETLAYOUTOFTEXT "ChangeKeyboardLayout/GetLayoutOfText"

typedef struct  
{
	HKL hklFrom;				//layout of the current text
	HKL hklTo;				    //layout of the result text
	BOOL bTwoWay;
}CKLLayouts;

//wParam - LPCTSTR исходного текста
//lParam - указатель на структуру CKLLayouts, содержащую раскладки для 
//изменения текста и опцию "двунаправленного преобразования"
//Возвращает LPTSTR на результирующую строку
#define MS_CKL_CHANGETEXTLAYOUT "ChangeKeyboardLayout/ChangeTextLayout"

#endif