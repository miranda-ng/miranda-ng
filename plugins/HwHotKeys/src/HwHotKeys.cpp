// HwHotKeys.cpp: ���������� ���������������� ������� ��� ���������� DLL.
//

#include "stdafx.h"
#include "HwHotKey.h"



// ������ ���������������� ����������
HWHOTKEYS_API int nHwHotKeys=0;

// ������ ���������������� �������.
HWHOTKEYS_API int fnHwHotKeys(void)
{
	return 42;
}

// ����������� ��� ����������������� ������.
// ��. ����������� ������ � HwHotKeys.h
CHwHotKeys::CHwHotKeys()
{
	return;
}
