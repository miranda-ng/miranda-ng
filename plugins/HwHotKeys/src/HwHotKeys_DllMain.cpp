/* ============================================================================
Hardware HotKeys plugin for Miranda NG.
Copyright � Eugene f2065, http://f2065.narod.ru, f2065 mail.ru, ICQ 35078112

This program is free software; you can redistribute it and / or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111 - 1307, USA.
============================================================================ */



#include "stdafx.h"

HINSTANCE hInstance;
int hLangpack;
HWND hDialogWnd = 0; // ����� ���� ��������, �� ��������� ������������ ��� ������ ���� � ��������� ��������� ����� �� ����
HHOOK hHook;
CLIST_INTERFACE *pcli;

// ��� �������� ��������� - �������� ������ ��� ��������/������/������, �� ������� ������ ����������� ��������������� ��������
DWORD code_Close = 0;
DWORD code_HideShow = 0;
DWORD code_ReadMsg = 0;
// � _tmp ���������� ��������� �� ����� ������� ��������
DWORD code_Close_tmp = 0;
DWORD code_HideShow_tmp = 0;
DWORD code_ReadMsg_tmp = 0;
// ������ ��������� ��� �����:
// ������� 0...7 ���� - ��� ���������� ������� (��� �� �������� � KbdLLHookStruct.scanCode&0xFF)
// 8 ��� - ��� ��� �� ����� ��������, ���� LLKHF_EXTENDED ((LOBYTE(KbdLLHookStruct.flags)&1) - ����� ������ � ���� ������ 
// (������� 101-��������� ����� �� ������� 255 ���������, �.�. ��� ����� ������ ������ ������� ��� ����� ������� ���� F13-F20).
// ����, ������ ��������� 9-������� ���������� (����� ������ ���� ���� ��������� ��������� � ������� *key_tab[0x200]).
// ���� 9-19 - �� ������������.
// ���� 20-31 - ��� ����� ���������, Shift/Ctrl/Alt/Win. �� 4 ������ - 12 �����. ������ ��� ������ ������ 3 ����� ���� - �����, ������, ��� �����.
// ������� ����� ��������� ����������� � key_flag_sr � �������� ������

// ������ ��������� 10 - ������� "����������" ������������ �������.
// ������� 0...7 ���� - ��� ������ ���������� ������� (��� �� �������� � KbdLLHookStruct.scanCode&0xFF)
// 8 ��� - ��� ��� �� ����� ��������, ���� LLKHF_EXTENDED ((LOBYTE(KbdLLHookStruct.flags)&1) - ����� ������ � ���� ������ 
// (������� 101-��������� ����� �� ������� 255 ���������, �.�. ��� ����� ������ ������ ������� ��� ����� ������� ���� F13-F20).
// 9 ��� - ��� ������ ��� � ����� 0...7 �� ���������� �������, � VK-��� (KbdLLHookStruct.vkCode), � 8 ��� ��� ���� ������ 0.
// ������� ��� ������ ��� ��������� USB-���������� �� ������ � ��� ���������� ��������, � ���� ��� 0 - �� ����� ����������� VK-���.
// (����� ������ ���� ���� ��������� ��������� � ������� *key_tab[0x300]).
// ���� 10-19 - �� ������������.
// ���� 20-31 - ��� ����� ���������, Shift/Ctrl/Alt/Win. �� 4 ������ - 12 �����. ������ ��� ������ ������ 3 ����� ���� - �����, ������, ��� �����.
// ������� ����� ��������� ����������� � key_flag_sr � �������� ������




DWORD key_code = 0; // ��� ��������� ����������� ������� ������� ������� ����������
DWORD key_code_raw = 0;
DWORD key_code_assign = 0;  // �� ����� �������� ��� �������� ������� ��������� ������� ����������, ������� ����� ��������� ��� ��������.

CHAR key_name_buffer[150]; // ����� ���� ���������� ��� ������ � ����������. ANSI. 
// ����� ������� � ������ ������ - �L_Shift + L_Ctrl + L_Alt + L_Win + R_Shift + R_Ctrl + R_Alt + R_Win + MSO_Print, HP_TouchPadOff� - ����� 100 ����.

// ============================================================================

BOOL APIENTRY DllMain( HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	hInstance = hModule;
	return TRUE;
}

// ============================================================================

PLUGININFOEX PluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION_MIR,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {315B3800-8258-44C4-B60E-58C50B9303B6} - GUID ��� ������� HwHotKeys
	{ 0x315b3800, 0x8258, 0x44c4, { 0xb6, 0xe, 0x58, 0xc5, 0xb, 0x93, 0x3, 0xb6 } }
};

extern "C" __declspec(dllexport) PLUGININFOEX * MirandaPluginInfoEx(DWORD)
{
	return &PluginInfoEx;
}

// ============================================================================

extern "C" __declspec(dllexport) int Load(void)
{
	// ��������� (�� ��) ��������� �������
	code_Close = db_get_dw(0, __DbModName, "Close", 0);
	code_HideShow = db_get_dw(0, __DbModName, "HideShow", 0);
	code_ReadMsg = db_get_dw(0, __DbModName, "ReadMsg", 0);

	// ��������� ���� ����������� - ������ mir_getLP(PlgInfoEx)
	mir_getLP(&PluginInfoEx);

	// ��������� ������������ - ������ ��������� CLIST_INTERFACE *pcli;
	mir_getCLI();

	// ����������� ������� �����
	HookEvent(ME_OPT_INITIALISE, initializeOptions);

	// ��������� ���� ��� �������������� ��������� �������
	hHook = SetWindowsHookExA(WH_KEYBOARD_LL, key_hook, hInstance, 0);

//	StringCbPrintfA(key_name_buffer, 100, "hHook = 0x%x, Err = %u", hHook, GetLastError);
//	MessageBoxA(0, key_name_buffer, 0, 0);

	return 0;
}

// ====================================

int initializeOptions(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE OptDlgPg = { sizeof(OptDlgPg) };
	OptDlgPg.position = 100000000;
	OptDlgPg.hInstance = hInstance;
	OptDlgPg.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	OptDlgPg.pszTemplate = MAKEINTRESOURCEA(dlg_options);
	OptDlgPg.ptszGroup = LPGENT("Customize");
	OptDlgPg.ptszTitle = LPGENT("Hardware HotKeys");
	OptDlgPg.pfnDlgProc = OptDlgProc;
	OptDlgPg.hLangpack = hLangpack;
	Options_AddPage(wParam, &OptDlgPg);
	return 0;
}

// ============================================================================

extern "C" __declspec(dllexport) int Unload(void)
{
	UnhookWindowsHookEx(hHook);
	return 0;
}

// ============================================================================

