// HwHotKeys.cpp: определяет экспортированные функции для приложения DLL.
//

#include "stdafx.h"
#include "HwHotKey.h"



// Пример экспортированной переменной
HWHOTKEYS_API int nHwHotKeys=0;

// Пример экспортированной функции.
HWHOTKEYS_API int fnHwHotKeys(void)
{
	return 42;
}

// Конструктор для экспортированного класса.
// см. определение класса в HwHotKeys.h
CHwHotKeys::CHwHotKeys()
{
	return;
}
