#ifndef _M_CHANGEKEYBOARDLAYOUT_H
#define _M_CHANGEKEYBOARDLAYOUT_H

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