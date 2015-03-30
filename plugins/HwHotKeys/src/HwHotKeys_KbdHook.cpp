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

LRESULT CALLBACK key_hook(int nCode, WPARAM wParam, LPARAM lParam)  // https://msdn.microsoft.com/en-us/library/windows/desktop/ms644985%28v=vs.85%29.aspx
{
		KBDLLHOOKSTRUCT *pKbdLLHookStruct = (KBDLLHOOKSTRUCT *)lParam;
		if (nCode >= 0)
		{
			if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) // при отпускании любой кнопки - сбрасываем всю накопленную комбинацию
			{
				key_code = 0;
			}
			else if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
			{
				if (!(pKbdLLHookStruct->scanCode & 0xFFFFFF00) && (pKbdLLHookStruct->scanCode & 0xFF)) // && !(pKbdLLHookStruct->flags & LLKHF_INJECTED)) 
					// 0xFFFFFF00 фильтр дл€ странных двойных сканкодов например перед L_Alt/AltGr, 
					// 0xFF фильтр дл€ нулевых сканкодов(приход€т при вс€кой там эмул€ции нажатий и т.п.)
					// LLKHF_INJECTED фильтр дл€ эмулируемых нажатий, но есть проблема с некоторыми клавами
				{
					// строим комплексный код(сканкод + доп.флаг LLKHF_EXTENDED), от 0 до 1FF
					key_code_raw = (LOBYTE(pKbdLLHookStruct->scanCode) | ((LOBYTE(pKbdLLHookStruct->flags)&1) << 8));

					if (hDialogWnd) // если диалог настроек открыт - выводим в него нажимаемые кнопоки
					{
						StringCbPrintfA(key_name_buffer, sizeof(key_name_buffer), "%03X %s", key_code_raw, key_tab[(key_code_raw & 0x1FF)]);
						SetDlgItemTextA(hDialogWnd, dlg_scancode_edit, key_name_buffer);
					}

					switch (key_code_raw) // провер€ем кнопки модификаторов
						{
						case 0x01D: // L_Ctrl
							key_code |= key_flag_cl;
							break;   // после модификаторов - ничего не делаем, выход из хука.
						case 0x11D: // R_Ctrl
							key_code |= key_flag_cr;
							break;
						case 0x038: // L_Alt
							key_code |= key_flag_al;
							break;
						case 0x138: // R_Alt
							key_code |= key_flag_ar;
							break;
						case 0x15B: // L_Win
							key_code |= key_flag_wl;
							break;
						case 0x15C: // R_Win
							key_code |= key_flag_wr;
							break;
						case 0x02A: // L_Shift
							key_code |= key_flag_sl;
							break;
						case 0x036: // R_Shift
							key_code |= key_flag_sr;
							break;
						case 0x12A: // L_Shift_fake
							key_code |= key_flag_sl;
							break;
						case 0x136: // R_Shift_fake
							key_code |= key_flag_sr;
							break; 
						default:  // если это не кнопка модификатора - то дальнейша€ обработка
							key_code = (LOWORD(key_code_raw)) | (key_code & 0xFFFF0000); // в старшей половине там лежат биты модификаторов - их оставл€ем (они там могут быть от предыдущего раза)
							if (hDialogWnd) // если диалог настроек открыт - выводим в него комплексную комбинацию (типа Shift+Key), которую потом можно назначить;
							{
								if (IsDlgButtonChecked(hDialogWnd, dlg_combine)) // левые/правые модификаторы объедин€ть в один?
								{
									DWORD tmp1, tmp2;
									tmp1 = ((key_code >> 4) | (key_code >> 8)) & 0x00F00000;
									tmp2 = LOWORD(key_code) | tmp1;
									tmp2 &= 0x00F001FF;
									key_code_assign = tmp2;
								}
								else
								{
									key_code_assign = key_code;
								}
								HwHotKeys_PrintFullKeyname(key_code_assign);
								SetDlgItemTextA(hDialogWnd, dlg_keyname_edit, key_name_buffer);
							}
							else // хоткей выполн€ют действи€ только если закрыт диалог настроек (чтобы не мешал настраивать)
							{
								if (HwHotKeys_CompareCurrentScancode(code_Close)) // хоткей полного закрыти€ Miranda
								{
									// Beep(300, 200);
									CallService("CloseAction", 0, 0);
									return 1;
								}
								else if (HwHotKeys_CompareCurrentScancode(code_HideShow)) // хоткей сворачивани€/разворачивани€ главного окна Miranda
								{
									// Beep(700, 200);
									pcli->pfnShowHide(0, 0); // есть варианты
									return 1;
								}
								else if (HwHotKeys_CompareCurrentScancode(code_ReadMsg)) // хоткей чтени€ сообщени€
								{
									// Beep(1500, 200);
									if (pcli->pfnEventsProcessTrayDoubleClick(0) != 0) // клик по трею дл€ стандартного открыти€ сообщени€
									{ // иначе - окно чата уже открыто и надо его вытащить наверх...
										SetForegroundWindow(pcli->hwndContactList);
										SetFocus(pcli->hwndContactList);
										// хот€ всЄ равно это не очень работает в новой винде
										// надо http://www.rsdn.ru/article/qna/ui/wndsetfg.xml
										// но пока незнаю где тут вз€ть хэндл окна чата(причЄм именно дл€
										// выбранного контакта, их же несколько может быть если простой srmm)
										// у миранды в hkRead такие же проблемы
									}
									return 1; // возврат 1 запрещает дальнейшую работу кнопки, т.е. кнопка после нашего плагина никому более не достанетс€
								}
							}
						}
				}
			}
		}
	return CallNextHookEx(hHook, nCode, wParam, lParam);
}
