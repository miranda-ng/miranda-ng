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

#include "HwHotKeys.h"

// тут описаны известные имена кнопок по аппаратным сканкодам (KbdLLHookStruct.scanCode & 0xFF) и ((LOBYTE(KbdLLHookStruct.flags)&1) 
// и по VK-кодам (KbdLLHookStruct.vkCode)

// при обнаружении новых кнопок (на мультимедийных клавиатурах) просьба сообщать об этом автору!



// имена для KbdLLHookStruct.scanCode
const CHAR *k000 = "Undefined!";
const CHAR *k001 = "Esc";
const CHAR *k002 = "1";
const CHAR *k003 = "2";
const CHAR *k004 = "3";
const CHAR *k005 = "4";
const CHAR *k006 = "5";
const CHAR *k007 = "6";
const CHAR *k008 = "7";
const CHAR *k009 = "8";
const CHAR *k00A = "9";
const CHAR *k00B = "0";
const CHAR *k00C = "-_";
const CHAR *k00D = "=+";
const CHAR *k00E = "Backspace";
const CHAR *k00F = "Tab";
const CHAR *k010 = "Q";
const CHAR *k011 = "W";
const CHAR *k012 = "E";
const CHAR *k013 = "R";
const CHAR *k014 = "T";
const CHAR *k015 = "Y";
const CHAR *k016 = "U";
const CHAR *k017 = "I";
const CHAR *k018 = "O";
const CHAR *k019 = "P";
const CHAR *k01A = "[{";
const CHAR *k01B = "]}";
const CHAR *k01C = "Enter";
const CHAR *k01D = "L_Ctrl";
const CHAR *k01E = "A";
const CHAR *k01F = "S";
const CHAR *k020 = "D";
const CHAR *k021 = "F";
const CHAR *k022 = "G";
const CHAR *k023 = "H";
const CHAR *k024 = "J";
const CHAR *k025 = "K";
const CHAR *k026 = "L";
const CHAR *k027 = ";:";
const CHAR *k028 = "\'\""; // '" 
const CHAR *k029 = "`~";
const CHAR *k02A = "L_Shift";
const CHAR *k02B = "\\|";
const CHAR *k02C = "Z";
const CHAR *k02D = "X";
const CHAR *k02E = "C";
const CHAR *k02F = "V";
const CHAR *k030 = "B";
const CHAR *k031 = "N";
const CHAR *k032 = "M";
const CHAR *k033 = ",<";
const CHAR *k034 = ".>";
const CHAR *k035 = "/?";
const CHAR *k036 = "R_Shift";
const CHAR *k037 = "Keypad_*"; // 37 (Keypad - *) or(*/ PrtScn) on a 83 / 84 - key keyboard
const CHAR *k038 = "L_Alt";
const CHAR *k039 = "Space";
const CHAR *k03A = "Caps_Lock";
const CHAR *k03B = "F1";
const CHAR *k03C = "F2";
const CHAR *k03D = "F3";
const CHAR *k03E = "F4";
const CHAR *k03F = "F5";
const CHAR *k040 = "F6";
const CHAR *k041 = "F7";
const CHAR *k042 = "F8";
const CHAR *k043 = "F9";
const CHAR *k044 = "F10";
const CHAR *k045 = "Pause";
const CHAR *k046 = "Scroll_Lock";
const CHAR *k047 = "Keypad_7_Home";
const CHAR *k048 = "Keypad_8_Up";
const CHAR *k049 = "Keypad_9_PageUp";
const CHAR *k04A = "Keypad_-";
const CHAR *k04B = "Keypad_4_Left";
const CHAR *k04C = "Keypad_5";
const CHAR *k04D = "Keypad_6_Right";
const CHAR *k04E = "Keypad_+";
const CHAR *k04F = "Keypad_1_End";
const CHAR *k050 = "Keypad_2_Down";
const CHAR *k051 = "Keypad_3_PageDown";
const CHAR *k052 = "Keypad_0_Insert";
const CHAR *k053 = "Keypad_._Delete";
const CHAR *k054 = "Alt_SysRq"; // Alt + PrtScr

const CHAR *k057 = "F11";
const CHAR *k058 = "F12";

const CHAR *k105 = "Messenger";

const CHAR *k107 = "MSO_Redo";
const CHAR *k108 = "MSO_Undo";
const CHAR *k10A = "HP_Lock";
const CHAR *k10E = "HP_DVD";
const CHAR *k110 = "Media_PrevTrack";
const CHAR *k116 = "LogOff";
const CHAR *k119 = "Media_NextTrack";
const CHAR *k11C = "Keypad_Enter";
const CHAR *k11D = "R_Ctrl";
const CHAR *k120 = "Volume_Mute";
const CHAR *k121 = "Calculator";
const CHAR *k122 = "Media_Play/Pause";
const CHAR *k123 = "MSO_Spell";
const CHAR *k124 = "Media_Stop";
const CHAR *k12A = "L_Shift_fake";
const CHAR *k12E = "Volume_Down";
const CHAR *k130 = "Volume_Up";
const CHAR *k131 = "HP_Help";
const CHAR *k132 = "Browser_Start";
const CHAR *k135 = "Keypad_/";
const CHAR *k136 = "R_Shift_fake";
const CHAR *k137 = "PrintScreen"; // ? ? ? no - ext ?
const CHAR *k138 = "R_Alt";


const CHAR *k13B = "MSO_Help";
const CHAR *k13C = "MyMusic";
const CHAR *k13E = "MSO_New";
const CHAR *k13F = "MSO_Open";
const CHAR *k140 = "MSO_Close";
const CHAR *k141 = "MSO_Reply";
const CHAR *k142 = "MSO_Fwd";
const CHAR *k143 = "MSO_Send";

const CHAR *k145 = "Num_Lock";
const CHAR *k146 = "Ctrl_Break"; // Ctrl + Pause
const CHAR *k147 = "Home"; // grey
const CHAR *k148 = "Up"; // grey
const CHAR *k149 = "PageUp"; // grey
const CHAR *k14B = "Left"; // grey
const CHAR *k14C = "MyDocuments";
const CHAR *k14D = "Right"; // grey
const CHAR *k14F = "End"; // grey
const CHAR *k150 = "Down"; // grey
const CHAR *k151 = "PageDown"; // grey
const CHAR *k152 = "Insert"; // grey
const CHAR *k153 = "Delete"; // grey

const CHAR *k157 = "MSO_Save";
const CHAR *k158 = "MSO_Print,HP_TouchPadOff";
const CHAR *k159 = "HP_TouchPadOn";

const CHAR *k15B = "L_Win";
const CHAR *k15C = "R_Win";
const CHAR *k15D = "Context_Menu";
const CHAR *k15E = "Power";
const CHAR *k15F = "Sleep";
const CHAR *k163 = "Wake";
const CHAR *k164 = "MyPictures";
const CHAR *k16C = "Mail";
const CHAR *k16D = "Media_Select";

const CHAR *k165 = "Browser_Search";
const CHAR *k166 = "Browser_Favorites";
const CHAR *k167 = "Browser_Refresh";
const CHAR *k168 = "Browser_Stop";
const CHAR *k169 = "Browser_Forward";
const CHAR *k16A = "Browser_Back";


// кнопки регистров (Shift,Ctrl,Alt,Win)
const CHAR *t_CL = k01D;
const CHAR *t_CR = k11D;
const CHAR *t_CC = k01D + 2;
const CHAR *t_AL = k038;
const CHAR *t_AR = k138;
const CHAR *t_AA = k038 + 2;
const CHAR *t_WL = k15B;
const CHAR *t_WR = k15C;
const CHAR *t_WW = k038 + 2;
const CHAR *t_SL = k02A;
const CHAR *t_SR = k036;
const CHAR *t_SS = k02A + 2;



const CHAR *key_tab[0x200] = 
{
	k000, k001, k002, k003, k004, k005, k006, k007, k008, k009, k00A, k00B, k00C, k00D, k00E, k00F, // 000 - простые KbdLLHookStruct.scanCode
	k010, k011, k012, k013, k014, k015, k016, k017, k018, k019, k01A, k01B, k01C, k01D, k01E, k01F, // 010
	k020, k021, k022, k023, k024, k025, k026, k027, k028, k029, k02A, k02B, k02C, k02D, k02E, k02F, // 020
	k030, k031, k032, k033, k034, k035, k036, k037, k038, k039, k03A, k03B, k03C, k03D, k03E, k03F, // 030
	k040, k041, k042, k043, k044, k045, k046, k047, k048, k049, k04A, k04B, k04C, k04D, k04E, k04F, // 040
	k050, k051, k052, k053, k054, k000, k000, k057, k058, k000, k000, k000, k000, k000, k000, k000, // 050
	k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, // 060
	k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, // 070
	k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, // 080
	k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, // 090
	k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, // 0A0
	k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, // 0B0
	k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, // 0C0
	k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, // 0D0
	k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, // 0E0
	k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, // 0F0
  //  0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
	k000, k000, k000, k000, k000, k105, k000, k107, k108, k000, k10A, k000, k000, k000, k10E, k000, // 100 - LLKHF_EXTENDED KbdLLHookStruct.scanCode
	k110, k000, k000, k000, k000, k000, k116, k000, k000, k119, k000, k000, k11C, k11D, k000, k000, // 110
	k120, k121, k122, k123, k124, k000, k000, k000, k000, k000, k12A, k000, k000, k000, k12E, k000, // 120
	k130, k131, k132, k000, k000, k135, k136, k137, k138, k000, k000, k13B, k13C, k000, k13E, k13F, // 130
	k140, k141, k142, k143, k000, k145, k146, k147, k148, k149, k000, k14B, k14C, k14D, k000, k14F, // 140
	k150, k151, k152, k153, k000, k000, k000, k157, k158, k159, k000, k15B, k15C, k15D, k15E, k15F, // 150
	k000, k000, k000, k163, k164, k165, k166, k167, k168, k169, k16A, k000, k16C, k16D, k000, k000, // 160
	k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, // 170
	k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, // 180
	k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, // 190
	k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, // 1A0
	k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, // 1B0
	k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, // 1C0
	k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, // 1D0
	k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, // 1E0
	k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, k000, // 1F0
  //  0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
};
