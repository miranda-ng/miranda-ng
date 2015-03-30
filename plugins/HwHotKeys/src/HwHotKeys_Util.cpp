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



// сравнивает текущий нажатый сканкод с указанным
BOOL HwHotKeys_CompareCurrentScancode(DWORD scancode)
{
	if (key_code == scancode) // key_code - текущий нажатый на клавиатуре сканкод.
	{
		return true; // сканкод напрямую совпал - TRUE.
	}
	if (!(scancode & 0x00F00000)) // проверяем нет ли в сканкоде запроса на любой из левого/правого регистра.
	{
		return false;
	}
	DWORD tmp1, tmp2; // битовые маски: 0xF0000000 - левые кнопки Sft/Ctr/Alt/Win, 0x0F000000 - правые кнопки, 0x00F00000 - любые кнопки Sft/Ctr/Alt/Win
	tmp1 = key_code >> 8;    // сдвигаем биты левых и правых кнопок.
	tmp2 = key_code >> 4;
	tmp1 |= tmp2;
	tmp1 &= 0x00F00000;
	tmp2 = (key_code | tmp1) & 0x00FFFFFF;
	if (tmp2 == scancode)
	{
		return true;
	}
	return false;
}



// печатает в буфер key_name_buffer текстовое описание нажатой комбинации кнопок
// Кодировка ANSI (для экономии места, всё равно названия клавиш английские)
VOID HwHotKeys_PrintFullKeyname(DWORD scancode)
{
	key_name_buffer[0] = 0;
	if (!scancode) // если нулевой сканкод - то пустую строку делаем.
	{
		return;
	}

	if (scancode & key_flag_sl) // L_Shift
	{
		StringCbCatA(key_name_buffer, sizeof(key_name_buffer), t_SL);
		StringCbCatA(key_name_buffer, sizeof(key_name_buffer), " + ");
	}
	if (scancode & key_flag_sr) // R_Shift
	{
		StringCbCatA(key_name_buffer, sizeof(key_name_buffer), t_SR);
		StringCbCatA(key_name_buffer, sizeof(key_name_buffer), " + ");
	}
	if (scancode & key_flag_ss) // Shift
	{
		StringCbCatA(key_name_buffer, sizeof(key_name_buffer), t_SS);
		StringCbCatA(key_name_buffer, sizeof(key_name_buffer), " + ");
	}

	if (scancode & key_flag_cl) // L_Ctrl
	{
		StringCbCatA(key_name_buffer, sizeof(key_name_buffer), t_CL);
		StringCbCatA(key_name_buffer, sizeof(key_name_buffer), " + ");
	}
	if (scancode & key_flag_cr) // R_Ctrl
	{
		StringCbCatA(key_name_buffer, sizeof(key_name_buffer), t_CR);
		StringCbCatA(key_name_buffer, sizeof(key_name_buffer), " + ");
	}
	if (scancode & key_flag_cc) // Ctrl
	{
		StringCbCatA(key_name_buffer, sizeof(key_name_buffer), t_CC);
		StringCbCatA(key_name_buffer, sizeof(key_name_buffer), " + ");
	}

	if (scancode & key_flag_al) // L_Alt
	{
		StringCbCatA(key_name_buffer, sizeof(key_name_buffer), t_AL);
		StringCbCatA(key_name_buffer, sizeof(key_name_buffer), " + ");
	}
	if (scancode & key_flag_ar) // R_Alt
	{
		StringCbCatA(key_name_buffer, sizeof(key_name_buffer), t_AR);
		StringCbCatA(key_name_buffer, sizeof(key_name_buffer), " + ");
	}
	if (scancode & key_flag_aa) // Alt
	{
		StringCbCatA(key_name_buffer, sizeof(key_name_buffer), t_AA);
		StringCbCatA(key_name_buffer, sizeof(key_name_buffer), " + ");
	}

	if (scancode & key_flag_wl) // L_Win
	{
		StringCbCatA(key_name_buffer, sizeof(key_name_buffer), t_WL);
		StringCbCatA(key_name_buffer, sizeof(key_name_buffer), " + ");
	}
	if (scancode & key_flag_wr) // R_Win
	{
		StringCbCatA(key_name_buffer, sizeof(key_name_buffer), t_WR);
		StringCbCatA(key_name_buffer, sizeof(key_name_buffer), " + ");
	}
	if (scancode & key_flag_ww) // Win
	{
		StringCbCatA(key_name_buffer, sizeof(key_name_buffer), t_WW);
		StringCbCatA(key_name_buffer, sizeof(key_name_buffer), " + ");
	}

	if ((scancode & 0x1FF) < sizeof(key_tab)) // защита от ошибочных данных - чтобы не выйти из таблицы имён.
	{
		StringCbCatA(key_name_buffer, sizeof(key_name_buffer), key_tab[(scancode & 0x1FF)]);
	}
	else
	{
		StringCbCatA(key_name_buffer, sizeof(key_name_buffer), "out of range");
	}

	INT_PTR tmp1 = mir_strlen(key_name_buffer); // допечатываем в конеце строки сканкод - для удобства работы с пока ещё не известными (безимянными) кнопками
	StringCbPrintfA(key_name_buffer + tmp1, sizeof(key_name_buffer) - tmp1, " (%03X)", (scancode & 0x1FF));

	return;
}


