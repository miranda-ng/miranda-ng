/* ============================================================================
Hardware HotKeys plugin for Miranda NG.
Copyright © Eugene f2065, http://f2065.narod.ru, f2065 mail.ru, ICQ 35078112

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

#pragma once

#include <SDKDDKVer.h>
#include <windows.h>

#include <newpluginapi.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_options.h>
#include <m_clistint.h>

#include <Strsafe.h>

#include "resource.h"
#include "version.h"

int initializeOptions(WPARAM, LPARAM);
INT_PTR CALLBACK OptDlgProc(HWND, UINT, WPARAM, LPARAM);
VOID HwHotKeys_PrintFullKeyname(uint32_t);
BOOL HwHotKeys_CompareCurrentScancode(uint32_t);
LRESULT CALLBACK key_hook(INT, WPARAM, LPARAM);

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};

extern HWND hDialogWnd; // глобально используется для вывода туда в реалтайме сканкодов клавы, из хука
extern HHOOK hHook;

extern uint32_t code_Close_tmp;
extern uint32_t code_HideShow_tmp;
extern uint32_t code_ReadMsg_tmp;
extern uint32_t code_Close;
extern uint32_t code_HideShow;
extern uint32_t code_ReadMsg;

extern uint32_t key_code;
extern uint32_t key_code_raw;
extern uint32_t key_code_assign;


extern CHAR key_name_buffer[150];
extern const CHAR *key_tab[0x200];

extern const CHAR *t_CL;
extern const CHAR *t_CR;
extern const CHAR *t_CC;
extern const CHAR *t_AL;
extern const CHAR *t_AR;
extern const CHAR *t_AA;
extern const CHAR *t_WL;
extern const CHAR *t_WR;
extern const CHAR *t_WW;
extern const CHAR *t_SL;
extern const CHAR *t_SR;
extern const CHAR *t_SS;

// значения связаны с битами в HwHotKeys_CompareCurrentScancode
#define key_flag_sr 0x01000000
#define key_flag_cr 0x02000000
#define key_flag_ar 0x04000000
#define key_flag_wr 0x08000000
#define key_flag_sl 0x10000000
#define key_flag_cl 0x20000000
#define key_flag_al 0x40000000
#define key_flag_wl 0x80000000
#define key_flag_ss 0x00100000
#define key_flag_cc 0x00200000
#define key_flag_aa 0x00400000
#define key_flag_ww 0x00800000
