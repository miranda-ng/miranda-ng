#include "common.h"
#include "subst.h"
#include "popwin.h"
#include "message_pump.h"
#include "str_utils.h"

#define TITLE_TEXT_LEN		512

#define ANIM_STEPS			20
#define ANIM_ELAPSE			10

#define CHECKMOUSE_ELAPSE	250

#define ID_TIMER_ANIMATE	0x0100
#define ID_TIMER_CHECKMOUSE	0x0101

typedef struct {
	TCHAR *swzLabel, *swzValue;
	bool value_newline;
	bool line_above;
	int label_height, value_height, total_height;
} RowData;

struct PopupWindowData {
	HBRUSH bkBrush, barBrush;
	HPEN bPen, dPen;
	int tb_height, av_height, text_height, sm_height, lm_height, label_width;
	int real_av_width, real_av_height;
	//TCHAR *lm_text, *sm_text;
	HANDLE hContact;
	int iconIndex;
	CLCINFOTIPEX clcit;
	TCHAR swzTitle[TITLE_TEXT_LEN];
	RowData *rows;
	int row_count;
	int anim_step;
	bool text_tip;
	int indent, sb_width;
	POINT start_cursor_pos; // work around bugs with hiding tips (timer check mouse position)
	HBITMAP hBm;
};

LRESULT CALLBACK PopupWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	PopupWindowData *pwd = (PopupWindowData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch(uMsg) {
		case WM_CREATE:
			{

				CREATESTRUCT *cs = (CREATESTRUCT *)lParam;
				pwd = (PopupWindowData *)malloc(sizeof(PopupWindowData));
				memset(pwd, 0, sizeof(PopupWindowData));
				pwd->clcit = *(CLCINFOTIPEX *)cs->lpCreateParams;
				pwd->iconIndex = -1;
				pwd->bkBrush = CreateSolidBrush(options.bg_col);
				pwd->barBrush = CreateSolidBrush(options.sidebar_col);
				pwd->bPen = options.border ? (HPEN)CreatePen(PS_SOLID, 1, options.border_col) : (HPEN)CreatePen(PS_SOLID, 1, options.bg_col); 
				pwd->dPen = (HPEN)CreatePen(PS_SOLID, 1, options.div_col);
				
				SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pwd);

				// work around bug hiding tips
				GetCursorPos(&pwd->start_cursor_pos);
				SetTimer(hwnd, ID_TIMER_CHECKMOUSE, CHECKMOUSE_ELAPSE, 0);

				if(pwd->clcit.proto) {
					pwd->text_tip = false;
					pwd->indent = options.text_indent;
					pwd->sb_width = options.sidebar_width;

					a2t(pwd->clcit.proto, pwd->swzTitle, TITLE_TEXT_LEN);

					TCHAR uid_buff[256], uid_name_buff[256];
					if(uid(0, pwd->clcit.proto, uid_buff, 256) && uid_name(pwd->clcit.proto, uid_name_buff, 253)) { // 253 to leave room for ': '
						_tcscat(uid_name_buff, _T(": "));
						
						pwd->rows = (RowData *) realloc(pwd->rows, sizeof(RowData) * (pwd->row_count + 1));								

						pwd->rows[pwd->row_count].swzLabel = _tcsdup(uid_name_buff);
						pwd->rows[pwd->row_count].swzValue = _tcsdup(uid_buff);
						pwd->rows[pwd->row_count].value_newline = false;
						pwd->rows[pwd->row_count].line_above = false;
						pwd->row_count++;
					}

					WORD status = CallProtoService(pwd->clcit.proto, PS_GETSTATUS, 0, 0);
					TCHAR *strptr = (TCHAR *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, status, GSMDF_TCHAR);
					if (strptr) 
					{
						pwd->rows = (RowData *) realloc(pwd->rows, sizeof(RowData) * (pwd->row_count + 1));								

						pwd->rows[pwd->row_count].swzLabel = _tcsdup(TranslateT("Status:"));
						pwd->rows[pwd->row_count].swzValue = _tcsdup(strptr);
						pwd->rows[pwd->row_count].value_newline = false;
						pwd->rows[pwd->row_count].line_above = false;
						pwd->row_count++;
					}

					if(status >= ID_STATUS_OFFLINE && status <= ID_STATUS_IDLE) 
					{
						TCHAR *swzText = 0;
						char *status_msg = 0;

						// supported by protocols from Miranda 0.8+
						TCHAR* p = (TCHAR *)CallProtoService(pwd->clcit.proto, PS_GETMYAWAYMSG, 0, SGMA_TCHAR);
						if ((INT_PTR)p != CALLSERVICE_NOTFOUND)
						{
							if (p == NULL) 
								status_msg = (char *)CallProtoService(pwd->clcit.proto, PS_GETMYAWAYMSG, 0, 0);
							else 
							{
								swzText = _tcsdup( p );
								mir_free( p );
							}
						}
						else 
							status_msg = (char *)CallService(MS_AWAYMSG_GETSTATUSMSG, status, 0);
						
						if(status_msg && status_msg[0]) swzText = a2t(status_msg);

						if(swzText && swzText[0]) {
							StripBBCodesInPlace(swzText);
							pwd->rows = (RowData *) realloc(pwd->rows, sizeof(RowData) * (pwd->row_count + 1));								

							pwd->rows[pwd->row_count].swzLabel = _tcsdup(TranslateT("Status message:"));
							pwd->rows[pwd->row_count].swzValue = swzText;
							pwd->rows[pwd->row_count].value_newline = true;
							pwd->rows[pwd->row_count].line_above = true;
							pwd->row_count++;

						}

						if(status_msg) mir_free(status_msg);
					}
				} else if(pwd->clcit.text) {
					pwd->text_tip = true;
					pwd->indent = 0;
					pwd->sb_width = 0;


					//MessageBox(0, swzText, _T("tip"), MB_OK);
					
					TCHAR buff[2048], *swzText = pwd->clcit.text;
					int buff_pos, i = 0, size = (int)_tcslen(pwd->clcit.text);
					bool top_message = false;

					while(i < size && swzText[i] != _T('<')) {
						buff_pos = 0;
						while(swzText[i] != _T('\n') && swzText[i] != _T('\r') && i < size && buff_pos < 2048) {
							if(swzText[i] != _T('\t')) 
								buff[buff_pos++] = swzText[i];
							i++;
						}
						buff[buff_pos] = 0;

						if(buff_pos) {
							pwd->rows = (RowData *)realloc(pwd->rows, sizeof(RowData) * (pwd->row_count + 1));
							pwd->rows[pwd->row_count].line_above = false;
							pwd->rows[pwd->row_count].value_newline = true;
							pwd->rows[pwd->row_count].swzLabel = _tcsdup(_T(""));
							pwd->rows[pwd->row_count].swzValue = _tcsdup(buff);
							pwd->row_count++;
							top_message = true;
						}
						while(i < size && (swzText[i] == _T('\n') || swzText[i] == _T('\r')))
							i++;
					}

					// parse bold bits into labels and the rest into items
					while(i < size) {
						while(i + 2 < size 
							&& (swzText[i] != _T('<')
								|| swzText[i + 1] != _T('b') 
								|| swzText[i + 2] != _T('>'))) 
						{
							i++;
						}
						
						i += 3;

						buff_pos = 0;
						while(i + 3 < size 
							&& buff_pos < 2048
							&& (swzText[i] != _T('<') 
								|| swzText[i + 1] != _T('/')
								|| swzText[i + 2] != _T('b')
								|| swzText[i + 3] != _T('>')))
						{
							if(swzText[i] != _T('\t'))
								buff[buff_pos++] = swzText[i];
							i++;
						}

						i += 4;

						buff[buff_pos] = 0;

						if(buff_pos) {
							pwd->rows = (RowData *)realloc(pwd->rows, sizeof(RowData) * (pwd->row_count + 1));
							pwd->rows[pwd->row_count].value_newline = false;
							pwd->rows[pwd->row_count].swzLabel = _tcsdup(buff);
							if(pwd->row_count == 1 && top_message)
								pwd->rows[pwd->row_count].line_above = true;
							else
								pwd->rows[pwd->row_count].line_above = false;

							buff_pos = 0;
							while(i < size 
								&& buff_pos < 2048
								&& swzText[i] != _T('\n'))
							{
								if(swzText[i] != _T('\t') && swzText[i] != _T('\r'))
									buff[buff_pos++] = swzText[i];
								i++;
							}
							buff[buff_pos] = 0;

							pwd->rows[pwd->row_count].swzValue = _tcsdup(buff);

							pwd->row_count++;
						}

						i++;
					}

					if(pwd->row_count == 0) {
						// single item
						pwd->row_count = 1;
						pwd->rows = (RowData *)malloc(sizeof(RowData));
						pwd->rows[0].line_above = pwd->rows[0].value_newline = false;
						pwd->rows[0].swzLabel = 0;
						pwd->rows[0].swzValue = _tcsdup(swzText);
					}
					
					free(pwd->clcit.text);
					pwd->clcit.text = 0;
				} else {
					pwd->text_tip = false;
					pwd->indent = options.text_indent;
					pwd->sb_width = options.sidebar_width;
					pwd->hContact = pwd->clcit.hItem;
					pwd->iconIndex = (int)CallService(MS_CLIST_GETCONTACTICON, (WPARAM)pwd->hContact, 0);

					CallContactService(pwd->hContact, PSS_GETINFO, SGIF_ONOPEN, 0);

					// don't use stored status message
					// problem with delete setting?
					//DBDeleteContactSetting(pwd->hContact, MODULE, "TempStatusMsg");
					DBWriteContactSettingTString(pwd->hContact, MODULE, "TempStatusMsg", _T(""));

					TCHAR *stzCDN = (TCHAR *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)pwd->hContact, GCDNF_TCHAR);

					if(stzCDN) _tcsncpy(pwd->swzTitle, stzCDN, TITLE_TEXT_LEN);
					else _tcscpy(pwd->swzTitle, TranslateT("(Unknown)"));

					//get avatar if possible
					PROTO_AVATAR_INFORMATION AI = { sizeof(AI),  pwd->hContact };
					char *szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)pwd->hContact, 0);
					CallProtoService(szProto, PS_GETAVATARINFO, GAIF_FORCE, (LPARAM)&AI);
					SendMessage(hwnd, PUM_REFRESH_VALUES, 0, 0);
				}
			}

			// transparency
#ifdef WS_EX_LAYERED 
			SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
#endif

// not defined in my mingw winuser.h
#ifdef __GNUC__
#define CS_DROPSHADOW		0x00020000
#endif
#ifdef CS_DROPSHADOW
			if (options.drop_shadow) SetClassLong(hwnd, GCL_STYLE, CS_DROPSHADOW);
			else SetClassLong(hwnd, GCL_STYLE, 0);
#endif

#ifdef LWA_ALPHA
			if(MySetLayeredWindowAttributes) {
				MySetLayeredWindowAttributes(hwnd, RGB(0,0,0), (int)(options.opacity / 100.0 * 255), LWA_ALPHA);
				if(options.trans_bg) {
					MySetLayeredWindowAttributes(hwnd, options.bg_col, 0, LWA_COLORKEY);
				}
			}
#endif
			SendMessage(hwnd, PUM_GETHEIGHT, 0, 0); // calculate window height
			SendMessage(hwnd, PUM_CALCPOS, 0, 0);

			if(options.animate)
				SetTimer(hwnd, ID_TIMER_ANIMATE, ANIM_ELAPSE, 0);

			ShowWindow(hwnd, SW_SHOWNOACTIVATE);
			UpdateWindow(hwnd);
			/*
			// move clist under?
			{
				HWND hwndClist = (HWND)CallService(MS_CLUI_GETHWND, 0, 0);
				if(GetWindowLong(hwndClist, GWL_EXSTYLE) & WS_EX_TOPMOST)
					SetWindowPos(hwndClist, hwnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			}
			*/
			// since tipper win is topmost, this should put it at top of topmost windows
			SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			return 0;
		case WM_ERASEBKGND:
			{
				HDC hdc = (HDC) wParam; 
				RECT r, r_bar;
				GetClientRect(hwnd, &r); 
				
				// bg
				if (options.bg_fn[0])
				{
					BITMAP bm;
					HBITMAP hBg = (HBITMAP)CallService(MS_IMG_LOAD, (WPARAM)options.bg_fn, IMGL_TCHAR);		
					if(hBg) {
						HDC hdcMem = CreateCompatibleDC(hdc);
						HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hBg);

						GetObject(hBg, sizeof(bm), &bm);

						if(options.stretch_bg_img) {
							StretchBlt(hdc, pwd->sb_width, 0, r.right, r.bottom, hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
						} else {
							BitBlt(hdc, pwd->sb_width, 0, r.right, r.bottom, hdcMem, 0, 0, SRCCOPY);
						}

						SelectObject(hdcMem, hbmOld);
						DeleteDC(hdcMem);
						DeleteObject(hBg);
					} else
						FillRect(hdc, &r, pwd->bkBrush);
				} else
					FillRect(hdc, &r, pwd->bkBrush);

				// sidebar
				r_bar = r;
				r_bar.right = r.left + pwd->sb_width;
				FillRect(hdc, &r_bar, pwd->barBrush);
				// border
				if(options.border) {

					HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
					HPEN hOldPen = (HPEN)SelectObject(hdc, pwd->bPen);

					int h = 0;
					if(options.round) {
						int v;
						int w=14;
						h=(r.right-r.left)>(w*2)?w:(r.right-r.left);
						v=(r.bottom-r.top)>(w*2)?w:(r.bottom-r.top);
						h=(h<v)?h:v;
					//} else {
					//	Rectangle(hdc, r.left, r.top, (r.right - r.left), (r.bottom - r.top));
					}
					RoundRect(hdc, 0, 0, (r.right - r.left), (r.bottom - r.top), h, h);

					SelectObject(hdc, hOldBrush);
					SelectObject(hdc, hOldPen);
				}
			}
			return TRUE;
		case WM_PAINT:
			{
				RECT r, r2;
				//if(GetUpdateRect(hwnd, &r, TRUE)) {
					PAINTSTRUCT ps;
					BeginPaint(hwnd, &ps);
					HDC hdc = ps.hdc;
					GetClientRect(hwnd, &r);
					r2 = r;
					HFONT hOldFont = (HFONT)GetCurrentObject(hdc,OBJ_FONT);

					// text background
					//SetBkColor(ps.hdc, options.bg_col);
					SetBkMode(hdc, TRANSPARENT);

					// avatar
					if(!pwd->text_tip && options.av_layout != PAV_NONE && pwd->av_height && pwd->hBm) {
						RECT avr;
						avr.top = options.av_padding;
			
						if(options.av_layout == PAV_LEFT) {
							avr.left = r.left + options.av_padding;
							avr.right = avr.left + pwd->real_av_width;
							r2.left += pwd->real_av_width + (2 * options.av_padding - options.padding); // padding re-added for text
						} else if(options.av_layout == PAV_RIGHT) {
							avr.right = r.right - options.av_padding;
							avr.left = avr.right - pwd->real_av_width;
							r2.right -= pwd->real_av_width + (2 * options.av_padding - options.padding);
						}
						
						avr.bottom = avr.top + pwd->real_av_height;

						HDC hdcMem = CreateCompatibleDC(hdc);
						HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, pwd->hBm);
						BITMAP bm;

						GetObject(pwd->hBm, sizeof(bm), &bm);
						HRGN rgn = 0;
						if(options.av_round) {
							rgn = CreateRoundRectRgn(avr.left, avr.top, avr.right + 1, avr.bottom + 1, 10, 10);
							SelectClipRgn(hdc, rgn);
						}

						if(options.no_resize_av) {
							BitBlt(hdc, avr.left, avr.top, avr.right - avr.left, avr.bottom - avr.top, hdcMem, 0, 0, SRCCOPY);
						} else {
					        SetStretchBltMode(hdc, HALFTONE);
							StretchBlt(hdc, avr.left, avr.top, avr.right - avr.left, avr.bottom - avr.top, hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
						}

						if(rgn) {
							SelectClipRgn(hdc, 0);
							DeleteObject(rgn);
						}

						SelectObject(hdcMem, hbmOld);
						DeleteDC(hdcMem);

						/*
							AVATARDRAWREQUEST adr = {0};
							adr.cbSize = sizeof(adr);
							adr.hContact = pwd->hContact;
							adr.hTargetDC = ps.hdc;
							adr.rcDraw = avr;
							adr.dwFlags = (options.av_round ? AVDRQ_ROUNDEDCORNER : 0);// | (options.no_resize_av ? AVDRQ_DONTRESIZEIFSMALLER : 0);
							if(!pwd->hContact) { // status bar tip?
								adr.dwFlags |= AVDRQ_OWNPIC;
								adr.szProto = pwd->clcit.proto;
							}
							adr.radius = (options.av_round ? 5 : 0);
							
							CallService(MS_AV_DRAWAVATAR, 0, (LPARAM)&adr);
						*/
					}
					
					RECT tr;
					tr.left = r2.left + options.padding; tr.right = r2.right - options.padding; tr.top = tr.bottom = 0;
					if(!pwd->text_tip && options.title_layout != PTL_NOTITLE) {
						// title icon
						if(options.title_layout != PTL_NOICON) {
							HICON hIcon = 0;
							bool destroy_icon = true;
							if(pwd->iconIndex != -1) hIcon = ImageList_GetIcon((HIMAGELIST)CallService(MS_CLIST_GETICONSIMAGELIST, 0, 0), pwd->iconIndex, 0);
							else if(!pwd->hContact) {
								WORD status = CallProtoService(pwd->clcit.proto, PS_GETSTATUS, 0, 0);
								hIcon = LoadSkinnedProtoIcon(pwd->clcit.proto, status);
								destroy_icon = false;
							}
							if(hIcon) {
								int iconx;
								if(options.title_layout == PTL_RIGHTICON) {
									iconx = r2.right - 16 - options.padding;
									tr.right -= 16 + options.padding;
								} else {
									iconx = r2.left + options.padding;
									tr.left += 16 + options.padding;
								}
								DrawIconEx(ps.hdc, iconx, options.padding + (pwd->tb_height - options.padding - 16) / 2, hIcon, 16, 16, 0, NULL, DI_NORMAL);
								if(destroy_icon) DestroyIcon(hIcon);
							}
						}

						// title text
						if(hFontTitle) SelectObject(ps.hdc, (HGDIOBJ)hFontTitle);
						
						SetTextColor(ps.hdc, options.title_col);

						tr.top = r.top + options.padding; tr.bottom = tr.top + pwd->tb_height - options.padding;
						DrawText(ps.hdc, pwd->swzTitle, (int)_tcslen(pwd->swzTitle), &tr, DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS | DT_SINGLELINE | DT_NOPREFIX);
					}

					// values
					pwd->text_height = 0;
					bool use_r = true;
					int row_height;
					for(int i = 0; i < pwd->row_count; i++) {
						tr.top = tr.bottom;
						use_r = (tr.top + options.text_padding >= pwd->av_height);

						if(use_r) {
							if(pwd->rows[i].line_above) {
								HPEN hOldPen = (HPEN)SelectObject(hdc, pwd->dPen);
								tr.top += options.text_padding;
								Rectangle(hdc, r.left + options.padding + pwd->indent, tr.top, r.right - options.padding, tr.top + 1);
								SelectObject(hdc, hOldPen);
							}
							tr.left = r.left + options.padding + pwd->indent;
							if(pwd->rows[i].value_newline)
								tr.right = r.right - options.padding;
							else
								tr.right = r.left + options.padding + pwd->indent + pwd->label_width;
						} else {
							if(pwd->rows[i].line_above) {
								HPEN hOldPen = (HPEN)SelectObject(hdc, pwd->dPen);
								tr.top += options.text_padding;
								Rectangle(hdc, r2.left + options.padding + pwd->indent, tr.top, r2.right - options.padding, tr.top + 1);
								SelectObject(hdc, hOldPen);
							}
							tr.left = r2.left + options.padding + pwd->indent;
							if(pwd->rows[i].value_newline)
								tr.right = r2.right - options.padding;
							else
								tr.right = r2.left + options.padding + pwd->indent + pwd->label_width;
						}

						if(pwd->rows[i].value_newline)
							row_height = pwd->rows[i].label_height;
						else
							row_height = max(pwd->rows[i].label_height, pwd->rows[i].value_height);

						if(hFontLabels) SelectObject(hdc, (HGDIOBJ)hFontLabels);
						if(pwd->rows[i].label_height) {
							tr.top += options.text_padding;
							tr.bottom = tr.top + row_height;
							SetTextColor(ps.hdc, options.label_col);
							DrawText(ps.hdc, pwd->rows[i].swzLabel, (int)_tcslen(pwd->rows[i].swzLabel), &tr, options.label_valign | ((options.label_halign == DT_RIGHT && !pwd->rows[i].value_newline) ? DT_RIGHT : DT_LEFT) | DT_END_ELLIPSIS | DT_SINGLELINE | DT_NOPREFIX);
							if(pwd->rows[i].value_newline)
								tr.top = tr.bottom;
						} else
							tr.bottom = tr.top;

						if(pwd->rows[i].value_newline)
							row_height = pwd->rows[i].value_height;

						if(hFontValues) SelectObject(hdc, (HGDIOBJ)hFontValues);
						SetTextColor(ps.hdc, options.value_col);
						if(use_r) {
							if(pwd->rows[i].value_newline)
								tr.left = r.left + options.padding + pwd->indent;
							else
								tr.left = r.left + options.padding + pwd->indent + pwd->label_width + options.padding;
							tr.right = r.right - options.padding;
						} else {
							if(pwd->rows[i].value_newline)
								tr.left = r2.left + options.padding + pwd->indent;
							else
								tr.left = r2.left + options.padding + pwd->indent + pwd->label_width + options.padding;
							tr.right = r2.right - options.padding;
						}
						if(pwd->rows[i].value_height) {
							if(pwd->rows[i].value_newline || !pwd->rows[i].label_height) tr.top += options.text_padding;
							tr.bottom = tr.top + row_height;
							DrawText(ps.hdc, pwd->rows[i].swzValue, (int)_tcslen(pwd->rows[i].swzValue), &tr, options.value_valign | options.value_halign | DT_WORDBREAK | DT_WORD_ELLIPSIS | DT_END_ELLIPSIS | DT_NOPREFIX);
						}
					}

					SelectObject(hdc, hOldFont);
					EndPaint(hwnd, &ps);
				//}
			}
			return 0;
		case WM_DESTROY: 
			{
				KillTimer(hwnd, ID_TIMER_CHECKMOUSE);
				KillTimer(hwnd, ID_TIMER_ANIMATE);
				ShowWindow(hwnd, SW_HIDE);				

				DeleteObject(pwd->bkBrush);
				DeleteObject(pwd->barBrush);
				DeleteObject(pwd->bPen);
				DeleteObject(pwd->dPen);

				for(int i = 0; i < pwd->row_count; i++) {
					free(pwd->rows[i].swzLabel);
					free(pwd->rows[i].swzValue);
				}
				if(pwd->rows) free(pwd->rows);

				free(pwd); pwd = 0;
				SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
			}
			break;

		case WM_TIMER:
			if(wParam == ID_TIMER_ANIMATE) {
				pwd->anim_step++;
				if(pwd->anim_step == ANIM_STEPS)
					KillTimer(hwnd, ID_TIMER_ANIMATE);
				SendMessage(hwnd, PUM_UPDATERGN, 0, 0);
				InvalidateRect(hwnd, 0, TRUE);
				UpdateWindow(hwnd);
			} else if(wParam == ID_TIMER_CHECKMOUSE) {
				// workaround for tips that just won't go away

				POINT pt;
				GetCursorPos(&pt);

				/*
				// works well, except in e.g. options->events->ignore :(
				bool hide = false;
				if(pwd->text_tip) {
					// tip may be off clist (e.g. systray)
					if(pt.x != pwd->start_cursor_pos.x || pt.y != pwd->start_cursor_pos.y) // mouse has moved
						hide = false;
				} else {
					// check window under cursor - hide if not clist
					HWND hwnd_clist = (HWND)CallService(MS_CLUI_GETHWND, 0, 0),
						hwnd_under = WindowFromPoint(pt);
					if(hwnd_under != hwnd_clist && !IsChild(hwnd_clist, hwnd_under))
						hide = true;
				}
				if(hide) PostMPMessage(MUM_DELETEPOPUP, 0, 0);
				*/

				if(abs(pt.x - pwd->start_cursor_pos.x) > options.mouse_tollerance 
					|| abs(pt.y - pwd->start_cursor_pos.y) > options.mouse_tollerance) // mouse has moved beyond tollerance
				{
					PostMPMessage(MUM_DELETEPOPUP, 0, 0);
				}
			}
			break;

		case PUM_SETSTATUSTEXT:
			if (pwd && (HANDLE)wParam == pwd->hContact) 
			{
				// in case we have the status message in a row
				DBVARIANT dbv;
				if (!DBGetContactSettingTString(pwd->hContact, MODULE, "TempStatusMsg", &dbv))
				{
					if (_tcscmp(dbv.ptszVal, (TCHAR*)lParam))
					{
						DBWriteContactSettingTString(pwd->hContact, MODULE, "TempStatusMsg", (TCHAR*)lParam);
						SendMessage(hwnd, PUM_REFRESH_VALUES, 0, 0);
					}
					DBFreeVariant(&dbv);
				}
			}
			if(lParam) free((void *)lParam);
			return TRUE;

		case PUM_SETAVATAR:
			if(pwd && (HANDLE)wParam == pwd->hContact) {
				SendMessage(hwnd, PUM_GETHEIGHT, 0, 0); // calculate window height
				SendMessage(hwnd, PUM_CALCPOS, 0, 0);
				InvalidateRect(hwnd, 0, TRUE);
			}
			return TRUE;

		case PUM_REFRESH_VALUES:
			if(pwd && pwd->clcit.proto == 0 && pwd->text_tip == false) {
				for(int i = 0; i < pwd->row_count; i++) {
					free(pwd->rows[i].swzLabel);
					free(pwd->rows[i].swzValue);
				}
				if(pwd->rows) {
					free(pwd->rows);
					pwd->rows = 0;
				}
				pwd->row_count = 0;

				DIListNode *node = options.di_list;
				TCHAR buff_label[LABEL_LEN], buff[VALUE_LEN];
				while(node) {
					if(GetLabelText(pwd->hContact, node->di, buff_label, LABEL_LEN) && GetValueText(pwd->hContact, node->di, buff, VALUE_LEN)) {
						if(node->di.line_above // we have a line above
							&& pwd->row_count > 0 // and we're not the first row
							&& pwd->rows[pwd->row_count - 1].line_above // and above us there's a line above
							&& pwd->rows[pwd->row_count - 1].swzLabel[0] == 0 // with no label
							&& pwd->rows[pwd->row_count - 1].swzValue[0] == 0) // and no value
						{
							// overwrite item above
							pwd->row_count--;
							free(pwd->rows[pwd->row_count].swzLabel);
							free(pwd->rows[pwd->row_count].swzValue);
						} else
							pwd->rows = (RowData *) realloc(pwd->rows, sizeof(RowData) * (pwd->row_count + 1));								

						pwd->rows[pwd->row_count].swzLabel = _tcsdup(buff_label);
						pwd->rows[pwd->row_count].swzValue = _tcsdup(buff);
						pwd->rows[pwd->row_count].value_newline = node->di.value_newline;
						pwd->rows[pwd->row_count].line_above = node->di.line_above;
						pwd->row_count++;
					}
					node = node->next;
				}

				// if the last item is just a divider, remove it
				if(pwd->row_count > 0
					&& pwd->rows[pwd->row_count - 1].line_above // and above us there's a line above
					&& pwd->rows[pwd->row_count - 1].swzLabel[0] == 0 // with no label
					&& pwd->rows[pwd->row_count - 1].swzValue[0] == 0) // and no value
				{
					pwd->row_count--;
					free(pwd->rows[pwd->row_count].swzLabel);
					free(pwd->rows[pwd->row_count].swzValue);

					if(pwd->row_count == 0) {
						free(pwd->rows);
						pwd->rows = NULL;
					}
				}

				SendMessage(hwnd, PUM_GETHEIGHT, 0, 0); // calculate window height
				SendMessage(hwnd, PUM_CALCPOS, 0, 0);
				InvalidateRect(hwnd, 0, TRUE);
			}
			return TRUE;
		case PUM_GETHEIGHT:
			{
				int *pHeight = (int *)wParam;
				HDC hdc = GetDC(hwnd);
				SIZE size;
				RECT r;
				r.top = r.left = 0;
				r.right = options.win_width;
				int width = options.padding;
				HFONT hOldFont = (HFONT)GetCurrentObject(hdc,OBJ_FONT);

				// titlebar height
				if(!pwd->text_tip && pwd->swzTitle && options.title_layout != PTL_NOTITLE) {
					if(hFontTitle) SelectObject(hdc, (HGDIOBJ)hFontTitle);
					GetTextExtentPoint32(hdc, pwd->swzTitle, (int)_tcslen(pwd->swzTitle), &size);
					width += options.padding + size.cx;
					if(options.title_layout != PTL_NOICON) {
						pwd->tb_height = options.padding + max(size.cy, 16);
						width += 16 + options.padding;
					} else
						pwd->tb_height = options.padding + size.cy;
				} else
					pwd->tb_height = 0;

				// avatar height
				pwd->av_height = 0;
				if (!pwd->text_tip && options.av_layout != PAV_NONE && ServiceExists(MS_AV_DRAWAVATAR))
				{
					AVATARCACHEENTRY *ace = 0;
					if (pwd->hContact) ace = (AVATARCACHEENTRY *)CallService(MS_AV_GETAVATARBITMAP, (WPARAM)pwd->hContact, 0);
					else ace = (AVATARCACHEENTRY *)CallService(MS_AV_GETMYAVATAR, 0, (LPARAM)pwd->clcit.proto);

					if (ace && (ace->dwFlags & AVS_BITMAP_VALID) && !(ace->dwFlags & AVS_HIDEONCLIST))
					{
						pwd->hBm = ace->hbmPic;
						if (options.no_resize_av || (ace->bmHeight <= options.av_size && ace->bmWidth <= options.av_size)) {
							pwd->real_av_width = ace->bmWidth;
							pwd->real_av_height = ace->bmHeight;
						} else {
							if (ace->bmHeight >= ace->bmWidth) {
								pwd->real_av_height = options.av_size;
								pwd->real_av_width = options.av_size * ace->bmWidth / ace->bmHeight;
							} else {
								pwd->real_av_height = options.av_size * ace->bmHeight / ace->bmWidth;
								pwd->real_av_width = options.av_size;
							}
						}
						pwd->av_height = options.av_padding * 2 + pwd->real_av_height;		
						width += pwd->real_av_width + (2 * options.av_padding - options.padding);
					}
				}

				int i;
				// text height
				pwd->text_height = pwd->label_width = 0;
				// iterate once to find max label width for items with label and value on same line, but don't consider width of labels on a new line
				for(i = 0; i < pwd->row_count; i++) {
					if(pwd->rows[i].swzLabel && pwd->rows[i].value_newline == false) {
						if(hFontLabels) SelectObject(hdc, (HGDIOBJ)hFontLabels);
						GetTextExtentPoint32(hdc, pwd->rows[i].swzLabel, (int)_tcslen(pwd->rows[i].swzLabel), &size);
						if(size.cx > pwd->label_width)
							pwd->label_width = size.cx;
					}
				}

				for(i = 0; i < pwd->row_count; i++) {
					if(hFontLabels) SelectObject(hdc, (HGDIOBJ)hFontLabels);
					if(pwd->rows[i].swzLabel)
						GetTextExtentPoint32(hdc, pwd->rows[i].swzLabel, (int)_tcslen(pwd->rows[i].swzLabel), &size);
					else
						size.cy = size.cx = 0;

					// save so we don't have to recalculate
					pwd->rows[i].label_height = size.cy;

					if(hFontValues) SelectObject(hdc, (HGDIOBJ)hFontValues);
					RECT smr;
					smr.top = smr.bottom = 0;
					smr.left = r.left + options.padding + pwd->indent;
					smr.right = r.right;
					if(pwd->tb_height + pwd->text_height + options.text_padding < pwd->av_height)
						smr.right -= pwd->real_av_width + 2 * options.av_padding;
					else
						smr.right -= options.padding;
					if(!pwd->rows[i].value_newline) smr.right -= pwd->label_width + options.padding;
					if(pwd->rows[i].swzValue)
						DrawText(hdc, pwd->rows[i].swzValue, (int)_tcslen(pwd->rows[i].swzValue), &smr, DT_CALCRECT | DT_VCENTER | DT_LEFT | DT_WORDBREAK | DT_WORD_ELLIPSIS | DT_END_ELLIPSIS | DT_NOPREFIX);

					// save so we don't have to recalculate
					pwd->rows[i].value_height = smr.bottom;

					pwd->rows[i].total_height = (pwd->rows[i].line_above ? options.text_padding : 0);
					if(pwd->rows[i].value_newline) {
						if(size.cy) pwd->rows[i].total_height += size.cy + options.text_padding;
						if(smr.bottom) pwd->rows[i].total_height += smr.bottom + options.text_padding;
					} else {
						int maxheight = max(size.cy, smr.bottom);
						if(maxheight) pwd->rows[i].total_height += maxheight + options.text_padding;
					}

					// only consider this item's width, and include it's height, if it doesn't make the window too big
					if(max(pwd->tb_height + pwd->text_height + options.padding + pwd->rows[i].total_height, pwd->av_height) <= options.win_max_height) {
						if(width < options.win_width) {
							int wid = options.padding + pwd->indent + (pwd->rows[i].value_newline ? max(size.cx, smr.right - smr.left) : pwd->label_width + options.padding + (smr.right - smr.left));
							if(pwd->tb_height + pwd->text_height + options.text_padding < pwd->av_height)
								width = max(width, wid + pwd->real_av_width + 2 * options.av_padding);
							else
								width = max(width, wid + options.padding);
						}

						pwd->text_height += pwd->rows[i].total_height;
					}
				}

				SelectObject(hdc, hOldFont);
				ReleaseDC(hwnd, hdc);

				int height = max(pwd->tb_height + pwd->text_height + options.padding, pwd->av_height);

				if(height < options.min_height) height = options.min_height;
				// ignore minwidth for text tips
				if((!pwd->text_tip) && width < options.min_width) width = options.min_width;

				if(height > options.win_max_height) height = options.win_max_height;				
				if(width > options.win_width) width = options.win_width;

				GetWindowRect(hwnd, &r);
				if(r.right - r.left != width || r.bottom - r.top != height) {

					SetWindowPos(hwnd, 0, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
					SendMessage(hwnd, PUM_UPDATERGN, 0, 0);

					InvalidateRect(hwnd, 0, TRUE);
				}

				if(pHeight) *pHeight = height;
			}
			return TRUE;
		case PUM_UPDATERGN:
			{
				HRGN hRgn1;
				RECT r;
				
				int v,h;
				int w=11;
				GetWindowRect(hwnd,&r);
				r.right -= r.left;
				r.left = 0;
				r.bottom -= r.top;
				r.top = 0;

				if(options.animate && pwd->anim_step < ANIM_STEPS) {
					float frac = 1.0f - pwd->anim_step / (float)ANIM_STEPS;
					int wi = r.right, hi = r.bottom;

					r.left += (int)(wi / 2.0f * frac + 0.5f);
					r.right -= (int)(wi / 2.0f * frac + 0.5f);
					r.top += (int)(hi / 2.0f * frac + 0.5f);
					r.bottom -= (int)(hi / 2.0f * frac + 0.5f);
				}

				// round corners
				if(options.round) {
					h=(r.right-r.left)>(w*2)?w:(r.right-r.left);
					v=(r.bottom-r.top)>(w*2)?w:(r.bottom-r.top);
					h=(h<v)?h:v;
				} else
					h = 0;
				hRgn1=CreateRoundRectRgn(r.left,r.top,r.right + 1,r.bottom + 1,h,h);
				SetWindowRgn(hwnd,hRgn1,FALSE);
			}
			return TRUE;

		case PUM_CALCPOS:
			{
				RECT wa_rect, r;

				SystemParametersInfo(SPI_GETWORKAREA, 0, &wa_rect, FALSE);
				if (MyMonitorFromPoint)
				{
					HMONITOR hMon = MyMonitorFromPoint(pwd->clcit.ptCursor, MONITOR_DEFAULTTONEAREST);
					MONITORINFO mi;
					mi.cbSize = sizeof(mi);
					if (MyGetMonitorInfo(hMon, &mi))
						wa_rect = mi.rcWork;
				}

				GetWindowRect(hwnd, &r);

				CURSORINFO ci = {sizeof(CURSORINFO)};
				GetCursorInfo(&ci);

				int x = 0, y = 0, width = (r.right - r.left), height = (r.bottom - r.top);
				
				switch(options.pos) {
					case PP_BOTTOMRIGHT:
						x = pwd->clcit.ptCursor.x + GetSystemMetrics(SM_CXSMICON); // cursor size is too large - use small icon size
						y = pwd->clcit.ptCursor.y + GetSystemMetrics(SM_CYSMICON);
						break;
					case PP_BOTTOMLEFT:
						x = pwd->clcit.ptCursor.x - width - GetSystemMetrics(SM_CXSMICON);
						y = pwd->clcit.ptCursor.y + GetSystemMetrics(SM_CYSMICON);
						break;
					case PP_TOPRIGHT:
						x = pwd->clcit.ptCursor.x + GetSystemMetrics(SM_CXSMICON);
						y = pwd->clcit.ptCursor.y - height - GetSystemMetrics(SM_CYSMICON);
						break;
					case PP_TOPLEFT:
						x = pwd->clcit.ptCursor.x - width - GetSystemMetrics(SM_CXSMICON);
						y = pwd->clcit.ptCursor.y - height - GetSystemMetrics(SM_CYSMICON);
						break;
				}

				if(x + width + 8 > wa_rect.right)
					x = wa_rect.right - width - 8;
				if(y + height > wa_rect.bottom)
					y = pwd->clcit.ptCursor.y - height - 8;
				if(x - 8 < wa_rect.left)
					x = wa_rect.left + 8;
				if(y - 8 < wa_rect.top)
					y = pwd->clcit.ptCursor.y + GetSystemMetrics(SM_CYSMICON);

				SetWindowPos(hwnd, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
			}
			return TRUE;
	}


	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int AvatarChanged(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;
	PostMPMessage(MUM_GOTAVATAR, (WPARAM)hContact, 0);
	return 0;
}


