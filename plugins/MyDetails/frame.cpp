/* 
Copyright (C) 2005 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/


#include "commons.h"
#include "frame.h"
#include "wingdi.h"
#include "winuser.h"
#include <m_skin_eng.h>


// Prototypes /////////////////////////////////////////////////////////////////////////////////////


#define WINDOW_NAME_PREFIX "mydetails_window"
#define WINDOW_CLASS_NAME "MyDetailsFrame"
#define CONTAINER_CLASS_NAME "MyDetailsFrameContainer"

#define ID_FRAME_TIMER			1011
#define ID_RECALC_TIMER			1012
#define ID_STATUSMESSAGE_TIMER	1013

#define RECALC_TIME				500

#define IDC_HAND				MAKEINTRESOURCE(32649)


// Messages
#define MWM_REFRESH				(WM_USER+10)
#define MWM_REFRESH_DATA        (WM_USER+18)


HWND hwnd_frame = NULL;
HWND hwnd_container = NULL;

int frame_id = -1;

HANDLE hMenuShowHideFrame = 0;

int CreateFrame();
void FixMainMenu();
void UpdateFrameData();
void RedrawFrame();


// used when no multiwindow functionality available
BOOL MyDetailsFrameVisible();
void SetMyDetailsFrameVisible(BOOL visible);
int ShowHideMenuFunc(WPARAM wParam, LPARAM lParam);
int ShowFrameFunc(WPARAM wParam, LPARAM lParam);
int HideFrameFunc(WPARAM wParam, LPARAM lParam);
int ShowHideFrameFunc(WPARAM wParam, LPARAM lParam);



LRESULT CALLBACK FrameContainerWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK FrameWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void SetCycleTime();
void SetCycleTime(HWND hwnd);
void SetStatusMessageRefreshTime();
void SetStatusMessageRefreshTime(HWND hwnd);
int SettingsChangedHook(WPARAM wParam, LPARAM lParam);
int AvatarChangedHook(WPARAM wParam, LPARAM lParam);
int ProtoAckHook(WPARAM wParam, LPARAM lParam);
int SmileyAddOptionsChangedHook(WPARAM wParam,LPARAM lParam);
int ListeningtoEnableStateChangedHook(WPARAM wParam,LPARAM lParam);
int AccListChanged(WPARAM wParam, LPARAM lParam);


void ExternalRect(RECT &ret, const RECT r1, const RECT r2);
bool InsideRect(const POINT &p, const RECT &r);


int operator==(const RECT& left, const RECT& right)
{
	return left.left == right.left && left.right == right.right
			&& left.top == right.top && left.bottom == right.bottom;
}

class ToolTipArea
{
public:
	ToolTipArea() : hwndTT(0), hwndParent(0) { memset(&rc, 0, sizeof(rc)); }
	~ToolTipArea() { removeTooltip(); }

	void createTooltip(HWND hwnd, const RECT &rc, const TCHAR *text)
	{
		if (text == NULL || text[0] == 0)
		{
			removeTooltip();
			return;
		}

		this->text = text;

		if (this->rc == rc && hwndParent == hwnd && hwndTT != NULL)
			return;

		removeTooltip();

		this->rc = rc;
		this->hwndParent = hwnd;
		this->hwndTT = CreateTooltip(this->hwndParent, this->rc);
	}

	void removeTooltip()
	{
		if (hwndTT == NULL)
			return;

		DestroyWindow(hwndTT);
		hwndTT = NULL;
		hwndParent = NULL;
	}

	const TCHAR * getTextFor(HWND hwndFrom)
	{
		if (hwndTT == NULL || hwndTT != hwndFrom)
			return NULL;
		return text.c_str();
	}


private:
	
	HWND hwndTT;
	RECT rc;
	HWND hwndParent;
	std::tstring text;

	HWND CreateTooltip(HWND hwnd, RECT &rect)
	{
			  // struct specifying control classes to register
		INITCOMMONCONTROLSEX iccex; 
		HWND hwndTT;                 // handle to the ToolTip control
			  // struct specifying info about tool in ToolTip control
		TOOLINFO ti;
		unsigned int uid = 0;       // for ti initialization

		// Load the ToolTip class from the DLL.
		iccex.dwSize = sizeof(iccex);
		iccex.dwICC  = ICC_BAR_CLASSES;

		if(!InitCommonControlsEx(&iccex))
		   return NULL;

		/* CREATE A TOOLTIP WINDOW */
		hwndTT = CreateWindowEx(WS_EX_TOPMOST,
			TOOLTIPS_CLASS,
			NULL,
			WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,		
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			hwnd,
			NULL,
			hInst,
			NULL
			);

		/* Gives problem with mToolTip
		SetWindowPos(hwndTT,
			HWND_TOPMOST,
			0,
			0,
			0,
			0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		*/

		/* INITIALIZE MEMBERS OF THE TOOLINFO STRUCTURE */
		ti.cbSize = sizeof(TOOLINFO);
		ti.uFlags = TTF_SUBCLASS;
		ti.hwnd = hwnd;
		ti.hinst = hInst;
		ti.uId = uid;
		ti.lpszText = LPSTR_TEXTCALLBACK;
			// ToolTip control will cover the whole window
		ti.rect.left = rect.left;    
		ti.rect.top = rect.top;
		ti.rect.right = rect.right;
		ti.rect.bottom = rect.bottom;

		/* SEND AN ADDTOOL MESSAGE TO THE TOOLTIP CONTROL WINDOW */
		SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);	
		SendMessage(hwndTT, TTM_SETDELAYTIME, (WPARAM) (DWORD) TTDT_AUTOPOP, (LPARAM) MAKELONG(24 * 60 * 60 * 1000, 0));	

		return hwndTT;
	} 
};


struct SimpleItem
{
	RECT rc;
	bool draw;
	bool mouseOver;
	ToolTipArea tt;
	bool alignRight;

	SimpleItem() : draw(FALSE), mouseOver(FALSE), alignRight(FALSE)
	{ 
		memset(&rc, 0, sizeof(rc)); 
	}
	virtual ~SimpleItem() {}

	virtual void hide()
	{
		draw = false;
		mouseOver = false;
		tt.removeTooltip();
	}

	virtual void update(HWND hwnd, SkinFieldState *item)
	{
		draw = item->isVisible();
		alignRight = ( item->getHorizontalAlign() == SKN_HALIGN_RIGHT );

		if (draw)
		{
			rc = item->getRect();
			tt.createTooltip(hwnd, rc, item->getToolTip());
		}
		else
		{
			tt.removeTooltip();
		}
	}

	virtual bool hitTest(const POINT &p)
	{
		return draw && InsideRect(p, rc);
	}

	virtual const TCHAR * getToolTipFor(HWND hwndFrom)
	{
		return tt.getTextFor(hwndFrom);
	}

	bool setMouseOver(POINT *mousePos)
	{
		bool over = (mousePos != NULL && hitTest(*mousePos));

		if (mouseOver == over)
			return FALSE; 

		mouseOver = over;
		return TRUE; 
	}

};

struct IconAndItem : public SimpleItem
{
	RECT rcIcon;
	RECT rcItem;
	BOOL drawIcon;
	BOOL drawItem;
	ToolTipArea ttIcon;

	IconAndItem() : drawIcon(FALSE), drawItem(FALSE) 
	{ 
		memset(&rcIcon, 0, sizeof(rcIcon)); 
		memset(&rcItem, 0, sizeof(rcItem)); 
	}
	virtual ~IconAndItem() {}
	
	virtual void hide()
	{
		SimpleItem::hide();
		drawIcon = FALSE;
		drawItem = FALSE;
	}

	virtual void update(HWND hwnd, SkinIconFieldState *icon, SkinTextFieldState *item)
	{
		drawIcon = icon->isVisible();
		drawItem = item->isVisible();
		alignRight = ( item->getHorizontalAlign() == SKN_HALIGN_RIGHT );

		draw = drawIcon || drawItem;
		if (draw)
		{
			if (drawIcon)
				rcIcon = icon->getRect();
			if (drawItem)
				rcItem = item->getRect();
			
			if (drawIcon && drawItem)
				ExternalRect(rc, rcIcon, rcItem);
			else if (drawIcon)
				rc = rcIcon;
			else // if (drawItem)
				rc = rcItem;
		}

		if (drawItem)
			tt.createTooltip(hwnd, rcItem, item->getToolTip());
		else
			tt.removeTooltip();

		if (drawIcon)
			ttIcon.createTooltip(hwnd, rcIcon, icon->getToolTip());
		else
			ttIcon.removeTooltip();
	}

	virtual const TCHAR * getToolTipFor(HWND hwndFrom)
	{
		const TCHAR * ret = tt.getTextFor(hwndFrom);

		if (ret == NULL)
			ret = ttIcon.getTextFor(hwndFrom);

		return ret;
	}
};


struct MyDetailsFrameData
{
	std::vector<SimpleItem*> items;
	SimpleItem proto;
	SimpleItem proto_cycle_next;
	SimpleItem proto_cycle_prev;
	SimpleItem avatar;
	SimpleItem nick;
	IconAndItem status;
	SimpleItem away_msg;
	IconAndItem listening_to;
	IconAndItem email;

	bool showing_menu;

	bool tracking_exit;

	MyDetailsFrameData() 
		: showing_menu(false)
		, tracking_exit(false)
	{
		items.push_back(&proto);
		items.push_back(&proto_cycle_next);
		items.push_back(&proto_cycle_prev);
		items.push_back(&avatar);
		items.push_back(&nick);
		items.push_back(&status);
		items.push_back(&away_msg);
		items.push_back(&listening_to);
		items.push_back(&email);
	}
};



// Functions //////////////////////////////////////////////////////////////////////////////////////

void InitFrames()
{
	InitContactListSmileys();

	CreateFrame();

	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, SettingsChangedHook);
	HookEvent(ME_AV_MYAVATARCHANGED, AvatarChangedHook);
	HookEvent(ME_PROTO_ACK, ProtoAckHook);
	HookEvent(ME_SMILEYADD_OPTIONSCHANGED,SmileyAddOptionsChangedHook);
	HookEvent(ME_LISTENINGTO_ENABLE_STATE_CHANGED,ListeningtoEnableStateChangedHook);
	HookEvent(ME_PROTO_ACCLISTCHANGED, AccListChanged);
}


void DeInitFrames()
{
	if(ServiceExists(MS_CLIST_FRAMES_REMOVEFRAME) && frame_id != -1) 
	{
		CallService(MS_CLIST_FRAMES_REMOVEFRAME, (WPARAM)frame_id, 0);
	}

	if (hwnd_frame != NULL) DestroyWindow(hwnd_frame);
	if (hwnd_container != NULL) DestroyWindow(hwnd_container);
}

int SmileyAddOptionsChangedHook(WPARAM wParam,LPARAM lParam)
{
	UpdateFrameData();
	return 0;
}

int SkinEngineDrawCallback(HWND hWnd, HDC hDC, RECT * rcPaint, HRGN rgn, DWORD dFlags, void * CallBackData);

int CreateFrame() 
{
	WNDCLASS wndclass;
	wndclass.style         = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW; //CS_PARENTDC | CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = FrameWindowProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = hInst;
	wndclass.hIcon         = NULL;
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
	wndclass.hbrBackground = 0; //(HBRUSH)(COLOR_3DFACE+1);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = WINDOW_CLASS_NAME;
	RegisterClass(&wndclass);

	if (ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) 
	{
		hwnd_frame = CreateWindow(WINDOW_CLASS_NAME, Translate("My Details"), 
				WS_CHILD | WS_VISIBLE, 
				0,0,10,10, (HWND)CallService(MS_CLUI_GETHWND, 0, 0), NULL, hInst, NULL);

		CLISTFrame Frame = {0};
		
		Frame.cbSize = sizeof(Frame);
		Frame.name = "My Details";
		Frame.TBname = Translate("My Details");
		Frame.hWnd = hwnd_frame;
		Frame.align = alTop;
		Frame.Flags = F_VISIBLE | F_SHOWTB | F_SHOWTBTIP | F_NOBORDER | F_NO_SUBCONTAINER;
		Frame.height = 100;

		frame_id = CallService(MS_CLIST_FRAMES_ADDFRAME, (WPARAM)&Frame, 0);

		if (ServiceExists(MS_SKINENG_REGISTERPAINTSUB)) 
		{
			CallService(MS_BACKGROUNDCONFIG_REGISTER,(WPARAM)"My Details Background/MyDetails", 0);
			CallService(MS_SKINENG_REGISTERPAINTSUB, (WPARAM) Frame.hWnd, (LPARAM) SkinEngineDrawCallback);
		}
		
		if (DBGetContactSettingByte(NULL, "MyDetails", "ForceHideFrame", 0))
		{
			int flags = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, frame_id), 0);
			if(flags & F_VISIBLE) 
				CallService(MS_CLIST_FRAMES_SHFRAME, frame_id, 0);

			DBDeleteContactSetting(NULL, "MyDetails", "ForceHideFrame");
		}

		if (DBGetContactSettingByte(NULL, "MyDetails", "ForceShowFrame", 0))
		{	
			int flags = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, frame_id), 0);
			if(!(flags & F_VISIBLE)) 
				CallService(MS_CLIST_FRAMES_SHFRAME, frame_id, 0);

			DBDeleteContactSetting(NULL, "MyDetails", "ForceShowFrame");
		}
	}
	else 
	{
		wndclass.style         = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;//CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc   = FrameContainerWindowProc;
		wndclass.cbClsExtra    = 0;
		wndclass.cbWndExtra    = 0;
		wndclass.hInstance     = hInst;
		wndclass.hIcon         = NULL;
		wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
		wndclass.hbrBackground = 0; //(HBRUSH)(COLOR_3DFACE+1);
		wndclass.lpszMenuName  = NULL;
		wndclass.lpszClassName = CONTAINER_CLASS_NAME;
		RegisterClass(&wndclass);

		hwnd_container = CreateWindowEx(WS_EX_TOOLWINDOW, CONTAINER_CLASS_NAME, Translate("My Details"), 
			(WS_THICKFRAME | WS_CAPTION | WS_SYSMENU) & ~WS_VISIBLE,
			0,0,200,130, (HWND)CallService(MS_CLUI_GETHWND, 0, 0), NULL, hInst, NULL);
	
		hwnd_frame = CreateWindow(WINDOW_CLASS_NAME, Translate("My Details"), 
			WS_CHILD | WS_VISIBLE,
			0,0,10,10, hwnd_container, NULL, hInst, NULL);

		SetWindowLong(hwnd_container, GWL_USERDATA, (LONG)hwnd_frame);
		SendMessage(hwnd_container, WM_SIZE, 0, 0);

		// Create menu item

		CLISTMENUITEM menu = {0};

		menu.cbSize=sizeof(menu);
		menu.flags = CMIM_ALL;
		menu.popupPosition = -0x7FFFFFFF;
		menu.pszPopupName = Translate("My Details");
		menu.position = 1; // 500010000
		menu.hIcon = LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
		menu.pszName = Translate("Show My Details");
		menu.pszService= MODULE_NAME "/ShowHideMyDetails";
		hMenuShowHideFrame = (HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM)&menu);

		if(DBGetContactSettingByte(0, MODULE_NAME, SETTING_FRAME_VISIBLE, 1) == 1) 
		{
			ShowWindow(hwnd_container, SW_SHOW);
			FixMainMenu();
		}
	}

	CreateServiceFunction(MS_MYDETAILS_SHOWFRAME, ShowFrameFunc);
	CreateServiceFunction(MS_MYDETAILS_HIDEFRAME, HideFrameFunc);
	CreateServiceFunction(MS_MYDETAILS_SHOWHIDEFRAME, ShowHideFrameFunc);

	return 0;
}


BOOL FrameIsFloating() 
{
	if (frame_id == -1) 
	{
		return true; // no frames, always floating
	}
	
	return (CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLOATING, frame_id), 0) != 0);
}


LRESULT CALLBACK FrameContainerWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) 
	{
		case WM_SHOWWINDOW:
		{
			if ((BOOL)wParam)
				Utils_RestoreWindowPosition(hwnd, 0, MODULE_NAME, WINDOW_NAME_PREFIX);
			else
				Utils_SaveWindowPosition(hwnd, 0, MODULE_NAME, WINDOW_NAME_PREFIX);
			break;
		}

		case WM_ERASEBKGND:
		{
			HWND child = (HWND)GetWindowLong(hwnd, GWL_USERDATA);

			SendMessage(child, WM_ERASEBKGND, wParam, lParam);
			break;
		}

		case WM_SIZE:
		{
			HWND child = (HWND)GetWindowLong(hwnd, GWL_USERDATA);
			RECT r;
			GetClientRect(hwnd, &r);

			SetWindowPos(child, 0, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOZORDER | SWP_NOACTIVATE);
			InvalidateRect(child, NULL, TRUE);

			return TRUE;
		}

		case WM_CLOSE:
		{
			DBWriteContactSettingByte(0, MODULE_NAME, SETTING_FRAME_VISIBLE, 0);
			ShowWindow(hwnd, SW_HIDE);
			FixMainMenu();
			return TRUE;
		}
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}



BOOL ScreenToClient(HWND hWnd, LPRECT lpRect)
{
	BOOL ret;

	POINT pt;

	pt.x = lpRect->left;
	pt.y = lpRect->top;

	ret = ScreenToClient(hWnd, &pt);

	if (!ret) return ret;

	lpRect->left = pt.x;
	lpRect->top = pt.y;


	pt.x = lpRect->right;
	pt.y = lpRect->bottom;

	ret = ScreenToClient(hWnd, &pt);

	lpRect->right = pt.x;
	lpRect->bottom = pt.y;

	return ret;
}


BOOL MoveWindow(HWND hWnd, const RECT &rect, BOOL bRepaint)
{
	return MoveWindow(hWnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, bRepaint);
}


RECT GetInnerRect(const RECT &rc, const RECT &clipping)
{
	RECT rc_ret = rc;

	rc_ret.left = max(rc.left, clipping.left);
	rc_ret.top = max(rc.top, clipping.top);
	rc_ret.right = min(rc.right, clipping.right);
	rc_ret.bottom = min(rc.bottom, clipping.bottom);

	return rc_ret;
}





void ExternalRect(RECT &ret, const RECT r1, const RECT r2)
{
	ret.left = min(r1.left, r2.left);
	ret.right = max(r1.right, r2.right);
	ret.top = min(r1.top, r2.top);
	ret.bottom = max(r1.bottom, r2.bottom);
}


HBITMAP CreateBitmap32(int cx, int cy)
{
   BITMAPINFO RGB32BitsBITMAPINFO; 
    UINT * ptPixels;
    HBITMAP DirectBitmap;

    ZeroMemory(&RGB32BitsBITMAPINFO,sizeof(BITMAPINFO));
    RGB32BitsBITMAPINFO.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    RGB32BitsBITMAPINFO.bmiHeader.biWidth=cx;//bm.bmWidth;
    RGB32BitsBITMAPINFO.bmiHeader.biHeight=cy;//bm.bmHeight;
    RGB32BitsBITMAPINFO.bmiHeader.biPlanes=1;
    RGB32BitsBITMAPINFO.bmiHeader.biBitCount=32;

    DirectBitmap = CreateDIBSection(NULL, 
                                       (BITMAPINFO *)&RGB32BitsBITMAPINFO, 
                                       DIB_RGB_COLORS,
                                       (void **)&ptPixels, 
                                       NULL, 0);
    return DirectBitmap;
}


BOOL UseLayeredMode()
{
	return isLayeredEnabled() && !FrameIsFloating();
}


void EraseBackground(HWND hwnd, HDC hdc)
{
	RECT r;
	GetClientRect(hwnd, &r);

	if (isSkinEngineEnabled())
	{
		if (FrameIsFloating())
		{
			HBRUSH hB = CreateSolidBrush(opts.bkg_color);
			FillRect(hdc, &r, hB);
			DeleteObject(hB);
		}
		else
		{
			SkinDrawWindowBack(hwnd, hdc, &r, "Main,ID=Background");
		}

		SkinDrawGlyph(hdc, &r, &r,"MyDetails,ID=Background");
	}
	else
	{
		HBRUSH hB = CreateSolidBrush(opts.bkg_color);
		FillRect(hdc, &r, hB);
		DeleteObject(hB);
	}
}

static int Width(const RECT &rc)
{
	return rc.right - rc.left;
}

static int Height(const RECT &rc)
{
	return rc.bottom - rc.top;
}

static HICON CreateOverlayedIcon(HICON icon, HICON overlay)
{
	HIMAGELIST il = ImageList_Create(
				GetSystemMetrics(SM_CXICON),
				GetSystemMetrics(SM_CYICON),
				ILC_COLOR32|ILC_MASK, 2, 2);
	ImageList_AddIcon(il, icon);
	ImageList_AddIcon(il, overlay);
	HIMAGELIST newImage = ImageList_Merge(il,0,il,1,0,0);
	ImageList_Destroy(il);
	HICON hIcon = ImageList_GetIcon(newImage, 0, 0);
	ImageList_Destroy(newImage);
	return hIcon; // the result should be destroyed by DestroyIcon()
}

void Draw(HDC hdc, SkinIconFieldState &state)
{
	if (!state.isVisible())
		return;

	RECT rc = state.getInsideRect();
	HRGN rgn = CreateRectRgnIndirect(&rc);
	SelectClipRgn(hdc, rgn);

	rc = state.getInsideRect(true);

	skin_DrawIconEx(hdc, rc.left, rc.top, state.getIcon(), Width(rc), Height(rc), 0, NULL, DI_NORMAL);

	SelectClipRgn(hdc, NULL);
	DeleteObject(rgn);
}

void Draw(HDC hdc, SkinTextFieldState &state, BOOL replace_smileys = FALSE, const char *protocol = NULL)
{
	if (!state.isVisible())
		return;

	RECT rc = state.getInsideRect();
	HRGN rgn = CreateRectRgnIndirect(&rc);
	SelectClipRgn(hdc, rgn);

	HGDIOBJ oldFont = SelectObject(hdc, state.getFont());
	COLORREF oldColor = SetTextColor(hdc, state.getFontColor());

	UINT uFormat = DT_NOPREFIX | DT_END_ELLIPSIS | (opts.draw_text_rtl ? DT_RTLREADING : 0);

	switch(state.getHorizontalAlign())
	{
		case SKN_HALIGN_RIGHT:
			uFormat |= DT_RIGHT;
			break;
		case SKN_HALIGN_CENTER:
			uFormat |= DT_CENTER;
			break;
		case SKN_HALIGN_LEFT:
			uFormat |= DT_LEFT;
			break;
	}

	if (replace_smileys && opts.replace_smileys)
	{
		uFormat |= DT_SINGLELINE;

		// Draw only first line of text
		char *tmp = strdup(state.getText());
		char *pos = strchr(tmp, '\r');
		if (pos != NULL) 
			pos[0] = '\0';
		pos = strchr(tmp, '\n');
		if (pos != NULL) 
			pos[0] = '\0';

		Smileys_DrawText(hdc, tmp, -1, &rc, uFormat | (opts.resize_smileys ? DT_RESIZE_SMILEYS : 0), 
			opts.use_contact_list_smileys ? "clist" : protocol, NULL);
	}
	else
	{
		skin_DrawText(hdc, state.getText(), -1, &rc, uFormat);
	}


	SelectObject(hdc, oldFont);
	SetTextColor(hdc, oldColor);

	SelectClipRgn(hdc, NULL);
	DeleteObject(rgn);			
}


void DrawMouseOver(HDC hdc, RECT *lprc, const char *place)
{
	if (isSkinEngineEnabled())
	{
		SkinDrawGlyph(hdc, lprc, lprc, "MyDetails,ID=MouseOver");

		char glyph[1024];
		mir_snprintf(glyph, MAX_REGS(glyph), "MyDetails,ID=MouseOver%s", place);
		SkinDrawGlyph(hdc, lprc, lprc, glyph);
	}
	else
	{
		FrameRect(hdc, lprc, (HBRUSH) GetStockObject(GRAY_BRUSH));
	}
}


void Draw(HWND hwnd, HDC hdc_orig)
{
	MyDetailsFrameData *data = (MyDetailsFrameData *) GetWindowLong(hwnd, GWL_USERDATA);

	Protocol *proto = GetCurrentProtocol();
	if (proto == NULL)
	{
		EraseBackground(hwnd, hdc_orig);
		return;
	}

	if (ServiceExists(MS_CLIST_FRAMES_SETFRAMEOPTIONS) && frame_id != -1)
	{
		int flags = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, frame_id), 0);
		if(flags & F_UNCOLLAPSED) 
		{
			RECT rf;
			GetClientRect(hwnd, &rf);

			if (rf.bottom - rf.top != 0)
			{
				if (FrameIsFloating()) 
				{
					HWND parent = GetParent(hwnd);

					if (parent != NULL)
					{
						RECT rp_client, rp_window, r_window;
						GetClientRect(parent, &rp_client);
						GetWindowRect(parent, &rp_window);
						GetWindowRect(hwnd, &r_window);
						int diff = (rp_window.bottom - rp_window.top) - (rp_client.bottom - rp_client.top);
						if(ServiceExists(MS_CLIST_FRAMES_ADDFRAME))
							diff += (r_window.top - rp_window.top);

						SetWindowPos(parent, 0, 0, 0, rp_window.right - rp_window.left, diff, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
					}
				}
			}

			for (size_t i = 0; i < data->items.size(); ++i)
				data->items[i]->hide();

			return;
		}
	}


	RECT r_full;
	GetClientRect(hwnd, &r_full);

	HDC hdc;
	HBITMAP hBmp;
	BOOL useLayeredMode = UseLayeredMode();
	if (useLayeredMode)
	{
		hdc = hdc_orig;
		hBmp = NULL;
	}
	else
	{
		hdc = CreateCompatibleDC(hdc_orig);
		hBmp = CreateBitmap32(Width(r_full), Height(r_full));
		SelectObject(hdc, hBmp);
	}

	int old_bk_mode = SetBkMode(hdc, TRANSPARENT);
	HFONT old_font = (HFONT) GetCurrentObject(hdc, OBJ_FONT);
	COLORREF old_color = GetTextColor(hdc);
	SetStretchBltMode(hdc, HALFTONE);


	HICON hStatusIcon;
	bool freeStatusIcon = false;
	if (proto->GetCustomStatus() != 0 && proto->CanCall(PS_ICQ_GETCUSTOMSTATUSICON))
		hStatusIcon = (HICON) proto->Call(PS_ICQ_GETCUSTOMSTATUSICON, proto->GetCustomStatus(), LR_SHARED);
	else
		hStatusIcon = LoadSkinnedProtoIcon(proto->GetName(), proto->GetStatus());

	if (proto->IsLocked())
	{
		HICON hLockOverlay = LoadSkinnedIcon(SKINICON_OTHER_STATUS_LOCKED);
		if (hLockOverlay != NULL)
		{
			freeStatusIcon = true;
			hStatusIcon = CreateOverlayedIcon(hStatusIcon, hLockOverlay);
		}
	}


	HICON hListeningIcon = IcoLib_LoadIcon("LISTENING_TO_ICON");
	HICON hEmailIcon = IcoLib_LoadIcon("MYDETAILS_EMAIL");
	HICON hNextIcon = IcoLib_LoadIcon("MYDETAILS_NEXT_PROTOCOL");
	HICON hPrevIcon = IcoLib_LoadIcon("MYDETAILS_PREV_PROTOCOL");

	{
		dialog->setInfoBool("resize_frame", opts.resize_frame);
		dialog->setInfoBool("protocol.locked", proto->IsLocked());


		if (opts.resize_frame)
			dialog->setSize(Width(r_full), 0x1FFFFFFF);
		else
			dialog->setSize(Width(r_full), Height(r_full));



		SkinImageField avatar = dialog->getImageField("avatar");
		if (proto->CanGetAvatar() && proto->GetAvatarImage() != NULL)
		{
			avatar.setEnabled(TRUE);
			avatar.setImage(proto->GetAvatarImage());
		}
		else
		{
			avatar.setEnabled(FALSE);
			avatar.setImage(NULL);
		}

		SkinTextField nickname = dialog->getTextField("nickname");
		nickname.setText(proto->GetNick());

		SkinTextField protocol = dialog->getTextField("protocol");
		protocol.setText(proto->GetDescription());

		SkinIconField status_icon = dialog->getIconField("status_icon");
		status_icon.setIcon(hStatusIcon);

		SkinTextField status_name = dialog->getTextField("status_name");
		status_name.setText(proto->GetStatusName());

		SkinTextField status_msg = dialog->getTextField("status_msg");
		if (proto->CanGetStatusMsg()) 
		{
			status_msg.setEnabled(TRUE);
			status_msg.setText(proto->GetStatusMsg());
		}
		else
		{
			status_msg.setEnabled(FALSE);
			status_msg.setText(_T(""));
		}

		SkinIconField listening_icon = dialog->getIconField("listening_icon");
		SkinTextField listening = dialog->getTextField("listening");
		if (proto->ListeningToEnabled() && proto->GetStatus() > ID_STATUS_OFFLINE 
				&& proto->GetListeningTo()[0] != 0) 
		{
			listening_icon.setEnabled(TRUE);
			listening.setEnabled(TRUE);
			listening_icon.setIcon(hListeningIcon);
			listening.setText(proto->GetListeningTo());
		}
		else
		{
			listening_icon.setEnabled(FALSE);
			listening.setEnabled(FALSE);
			listening_icon.setIcon(NULL);
			listening.setText(_T(""));
		}

		SkinIconField email_icon = dialog->getIconField("email_icon");
		SkinTextField email = dialog->getTextField("email");
		if (proto->CanGetEmailCount())
		{
			email_icon.setEnabled(TRUE);
			email.setEnabled(TRUE);
			email_icon.setIcon(hEmailIcon);

			TCHAR tmp[64];
			_sntprintf(tmp, MAX_REGS(tmp), _T("%d"), proto->GetEmailCount());
			email.setText(tmp);
		}
		else
		{
			email_icon.setEnabled(FALSE);
			email.setEnabled(FALSE);
			email_icon.setIcon(NULL);
			email.setText(_T(""));
		}

		SkinIconField next_proto = dialog->getIconField("next_proto");
		SkinIconField prev_proto = dialog->getIconField("prev_proto");
		prev_proto.setIcon(hPrevIcon);
		next_proto.setIcon(hNextIcon);
	}

	SkinDialogState state = dialog->run();
	SkinImageFieldState avatar = state.getImageField("avatar");
	SkinTextFieldState nickname = state.getTextField("nickname");
	SkinTextFieldState protocol = state.getTextField("protocol");
	SkinIconFieldState status_icon = state.getIconField("status_icon");
	SkinTextFieldState status_name = state.getTextField("status_name");
	SkinTextFieldState status_msg = state.getTextField("status_msg");
	SkinIconFieldState listening_icon = state.getIconField("listening_icon");
	SkinTextFieldState listening = state.getTextField("listening");
	SkinIconFieldState email_icon = state.getIconField("email_icon");
	SkinTextFieldState email = state.getTextField("email");
	SkinIconFieldState next_proto = state.getIconField("next_proto");
	SkinIconFieldState prev_proto = state.getIconField("prev_proto");
		

	{
		data->proto.update(hwnd, &protocol);
		data->proto_cycle_next.update(hwnd, &next_proto);
		data->proto_cycle_prev.update(hwnd, &prev_proto);
		data->avatar.update(hwnd, &avatar);
		data->nick.update(hwnd, &nickname);
		data->status.update(hwnd, &status_icon, &status_name);
		data->away_msg.update(hwnd, &status_msg);
		data->listening_to.update(hwnd, &listening_icon, &listening);
		data->email.update(hwnd, &email_icon, &email);

		
		POINT p = {0};
		GetCursorPos(&p);
		ScreenToClient(hwnd, &p);

		for(size_t i = 0; i < data->items.size(); ++i)
			data->items[i]->setMouseOver(&p);
	}

	// Erase
	EraseBackground(hwnd, hdc);

	// Draw items

	UINT uFormat = DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS 
					| (opts.draw_text_rtl ? DT_RTLREADING : 0);

	// Image
	if (avatar.isVisible() && proto->CanGetAvatar() && proto->GetAvatarImage() != NULL)
	{
		RECT rc = avatar.getInsideRect();
		HRGN rgn = CreateRectRgnIndirect(&rc);
		SelectClipRgn(hdc, rgn);

		rc = avatar.getInsideRect(true);

		int width = Width(rc);
		int height = Height(rc);

		int round_radius;
		if (opts.draw_avatar_round_corner)
		{
			if (opts.draw_avatar_use_custom_corner_size)
				round_radius = opts.draw_avatar_custom_corner_size;
			else
				round_radius = min(width, height) / 6;
		}
		else
		{
			round_radius = 0;
		}


		AVATARDRAWREQUEST adr = {0};

		adr.cbSize = sizeof(AVATARDRAWREQUEST);
		adr.hTargetDC = hdc;
		adr.rcDraw = rc;

		adr.dwFlags = AVDRQ_OWNPIC | AVDRQ_HIDEBORDERONTRANSPARENCY | 
			(opts.draw_avatar_border ? AVDRQ_DRAWBORDER : 0 ) |
			(opts.draw_avatar_round_corner ? AVDRQ_ROUNDEDCORNER : 0 );

		if (useLayeredMode)
			adr.dwFlags |= AVDRQ_AERO;

		adr.clrBorder =  opts.draw_avatar_border_color;
		adr.radius = round_radius;
		adr.alpha = 255;
		adr.szProto = (char *) proto->GetName();

		CallService(MS_AV_DRAWAVATAR, 0, (LPARAM) &adr);

		// Clipping rgn
		SelectClipRgn(hdc, NULL);
		DeleteObject(rgn);
	}

	// Nick
	if (data->nick.draw && data->nick.mouseOver && proto->CanSetNick())
		DrawMouseOver(hdc, &nickname.getRect(), "Nick");

	Draw(hdc, nickname, TRUE, proto->GetName());


	// Protocol
	if (data->proto.draw && data->proto.mouseOver)
		DrawMouseOver(hdc, &data->proto.rc, "Proto");
	
	Draw(hdc, protocol);
	

	// Status
	if (data->status.draw && data->status.mouseOver)
		DrawMouseOver(hdc, &data->status.rc, "Status");
	
	Draw(hdc, status_icon);
	Draw(hdc, status_name);


	// Away message
	if (data->away_msg.draw && data->away_msg.mouseOver && proto->CanSetStatusMsg())
		DrawMouseOver(hdc, &data->away_msg.rc, "StatusMsg");
	
	Draw(hdc, status_msg, TRUE, proto->GetName());


	// Listening to
	Draw(hdc, listening_icon);
	Draw(hdc, listening);
	
	if (data->listening_to.draw && data->listening_to.mouseOver && protocols->CanSetListeningTo())
		DrawMouseOver(hdc, &data->listening_to.rc, "Listening");
	

	// Unread email count
	Draw(hdc, email_icon);
	Draw(hdc, email);

	// Protocol cycle icon
	Draw(hdc, next_proto);
	Draw(hdc, prev_proto);


	SelectObject(hdc, old_font);
	SetTextColor(hdc, old_color);
	SetBkMode(hdc, old_bk_mode);

	if (!useLayeredMode)
	{
		BitBlt(hdc_orig, r_full.left, r_full.top, r_full.right - r_full.left, 
			r_full.bottom - r_full.top, hdc, r_full.left, r_full.top, SRCCOPY);
		DeleteDC(hdc);
		DeleteObject(hBmp);
	}

	if (freeStatusIcon)
		DestroyIcon(hStatusIcon);
	IcoLib_ReleaseIcon(hListeningIcon);
	IcoLib_ReleaseIcon(hEmailIcon);
	IcoLib_ReleaseIcon(hPrevIcon);
	IcoLib_ReleaseIcon(hNextIcon);

	if (opts.resize_frame && ServiceExists(MS_CLIST_FRAMES_SETFRAMEOPTIONS) && frame_id != -1)
	{
		RECT rf;
		GetClientRect(hwnd, &rf);

		int currentSize = Height(r_full);

		int expectedSize = 0;
		for(size_t i = 0; i < data->items.size(); ++i)
		{
			SimpleItem *item = data->items[i];
			if (!item->draw)
				continue;

			expectedSize = max(expectedSize, item->rc.bottom);
		}
		expectedSize += state.getBorders().bottom;

		if (expectedSize != currentSize)
		{
			if (FrameIsFloating()) 
			{
				HWND parent = GetParent(hwnd);

				if (parent != NULL)
				{
					RECT rp_client, rp_window, r_window;
					GetClientRect(parent, &rp_client);
					GetWindowRect(parent, &rp_window);
					GetWindowRect(hwnd, &r_window);
					int diff = (rp_window.bottom - rp_window.top) - (rp_client.bottom - rp_client.top);
					if(ServiceExists(MS_CLIST_FRAMES_ADDFRAME))
						diff += (r_window.top - rp_window.top);

					SetWindowPos(parent, 0, 0, 0, rp_window.right - rp_window.left, expectedSize + diff, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
				}
			}
			else if (IsWindowVisible(hwnd) && ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) 
			{
				int flags = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, frame_id), 0);
				if(flags & F_VISIBLE) 
				{
					CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS, MAKEWPARAM(FO_HEIGHT, frame_id), (LPARAM) expectedSize);
					CallService(MS_CLIST_FRAMES_UPDATEFRAME, (WPARAM)frame_id, (LPARAM)(FU_TBREDRAW | FU_FMREDRAW | FU_FMPOS));
				}
			}
		}
	}
}

int SkinEngineDrawCallback(HWND hWnd, HDC hDC, RECT * rcPaint, HRGN rgn, DWORD dFlags, void * CallBackData)
{
	Draw(hWnd, hDC);
	return 0;
}

bool InsideRect(const POINT &p, const RECT &r)
{
	return p.x >= r.left && p.x < r.right && p.y >= r.top && p.y < r.bottom;
}

int ShowPopupMenu(HWND hwnd, HMENU submenu, SimpleItem &item)
{
	POINT p;
	if (item.alignRight)
		p.x = item.rc.right;
	else
		p.x = item.rc.left;
	p.y =  item.rc.bottom+1;
	ClientToScreen(hwnd, &p);
	
	return TrackPopupMenu(submenu, TPM_TOPALIGN|TPM_RIGHTBUTTON|TPM_RETURNCMD
			| (item.alignRight ? TPM_RIGHTALIGN : TPM_LEFTALIGN), p.x, p.y, 0, hwnd, NULL);
}


void ShowGlobalStatusMenu(HWND hwnd, MyDetailsFrameData *data, Protocol *proto, POINT &p)
{
	HMENU submenu = (HMENU) CallService(MS_CLIST_MENUGETSTATUS,0,0);
	
	int ret = ShowPopupMenu(hwnd, submenu, data->status);
	if(ret)
		CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(ret),MPCF_MAINMENU),(LPARAM)NULL);
}

void ShowProtocolStatusMenu(HWND hwnd, MyDetailsFrameData *data, Protocol *proto, POINT &p)
{
	HMENU menu = (HMENU) CallService(MS_CLIST_MENUGETSTATUS,0,0);
	HMENU submenu = NULL;

	if (menu != NULL)
	{
		// Find the correct menu item
		int count = GetMenuItemCount(menu);
		for (int i = 0 ; i < count && submenu == NULL; i++)
		{
			MENUITEMINFO mii = {0};

			mii.cbSize = sizeof(mii);

			if(!IsWinVer98Plus()) 
			{
				mii.fMask = MIIM_TYPE;
			}
			else 
			{
				mii.fMask = MIIM_STRING;
			}

			GetMenuItemInfo(menu, i, TRUE, &mii);

			if (mii.cch != 0)
			{
				mii.cch++;
				mii.dwTypeData = (char *)malloc(sizeof(char) * mii.cch);
				GetMenuItemInfo(menu, i, TRUE, &mii);

				if (strcmp(mii.dwTypeData, proto->GetDescription()) == 0)
				{
					submenu = GetSubMenu(menu, i);
				}

				free(mii.dwTypeData);
			}
		}

		if (submenu == NULL && GetNumProtocols() == 1)
		{
			submenu = menu;
		}
	}

	if (submenu != NULL)
	{
		int ret = ShowPopupMenu(hwnd, submenu, data->status);
		if(ret)
			CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(ret),MPCF_MAINMENU),(LPARAM)NULL);
	}
	else
	{
		// Well, lets do it by hand
		static int statusModePf2List[]={0xFFFFFFFF,PF2_ONLINE,PF2_SHORTAWAY,PF2_LONGAWAY,PF2_LIGHTDND,PF2_HEAVYDND,PF2_FREECHAT,PF2_INVISIBLE,PF2_ONTHEPHONE,PF2_OUTTOLUNCH};

		menu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1));
		submenu = GetSubMenu(menu, 0);
		CallService(MS_LANGPACK_TRANSLATEMENU,(WPARAM)submenu,0);

		DWORD flags = proto->Call(PS_GETCAPS, PFLAGNUM_2);
		for ( int i = GetMenuItemCount(submenu) -1  ; i >= 0 ; i-- )
		{
			if (!(flags & statusModePf2List[i]))
			{
				// Hide menu
				RemoveMenu(submenu, i, MF_BYPOSITION);
			}
		}

		int ret = ShowPopupMenu(hwnd, submenu, data->status);
		DestroyMenu(menu);

		if(ret) 
			proto->SetStatus(ret);
	}
}

void ShowListeningToMenu(HWND hwnd, MyDetailsFrameData *data, Protocol *proto, POINT &p)
{
	HMENU menu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1));
	HMENU submenu = GetSubMenu(menu, 5);
	CallService(MS_LANGPACK_TRANSLATEMENU,(WPARAM)submenu,0);

	// Add this proto to menu
	char tmp[128];
	mir_snprintf(tmp, sizeof(tmp), Translate("Enable Listening To for %s"), proto->GetDescription());

	MENUITEMINFO mii = {0};
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_ID | MIIM_TYPE | MIIM_STATE;
	mii.fType = MFT_STRING;
	mii.fState = proto->ListeningToEnabled() ? MFS_CHECKED : 0;
	mii.dwTypeData = tmp;
	mii.cch = strlen(tmp);
	mii.wID = 1;

	if (!proto->CanSetListeningTo())
	{
		mii.fState |= MFS_DISABLED;
	}

	InsertMenuItem(submenu, 0, TRUE, &mii);

	ZeroMemory(&mii, sizeof(mii));
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_STATE;
	mii.fState = protocols->ListeningToEnabled() ? MFS_CHECKED : 0;

	if (!protocols->CanSetListeningTo())
	{
		mii.fState |= MFS_DISABLED;
	}

	SetMenuItemInfo(submenu, ID_LISTENINGTOPOPUP_SENDLISTENINGTO, FALSE, &mii);
	
	int ret = ShowPopupMenu(hwnd, submenu, data->listening_to);

	DestroyMenu(menu);

	switch(ret)
	{
		case 1:
		{
			CallService(MS_LISTENINGTO_ENABLE, (LPARAM) proto->GetName(), !proto->ListeningToEnabled());
			break;
		}
		case ID_LISTENINGTOPOPUP_SENDLISTENINGTO:
		{
			CallService(MS_LISTENINGTO_ENABLE, 0, !protocols->ListeningToEnabled());
			break;
		}
	}

}


LRESULT CALLBACK FrameWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	switch(msg) 
	{
		case WM_CREATE: 
		{
			MyDetailsFrameData *data = new MyDetailsFrameData();
			SetWindowLong(hwnd, GWL_USERDATA, (LONG) data);

			SetCurrentProtocol(DBGetContactSettingWord(NULL, "MyDetails", "ProtocolNumber", 0));

			SetCycleTime(hwnd);

			SetStatusMessageRefreshTime(hwnd);

			return TRUE;
		}


		case WM_ERASEBKGND:
		{
			//EraseBackground(hwnd, (HDC)wParam); 
			//Draw(hwnd, (HDC)wParam); 
			return TRUE;
		}

		/*
		case WM_PRINTCLIENT:
		{
			Draw(hwnd, (HDC)wParam);
			return TRUE;
		}
		*/

		case WM_PAINT:
		{
			if (UseLayeredMode())
			{
				CallService(MS_SKINENG_INVALIDATEFRAMEIMAGE, (WPARAM) hwnd, 0);
				ValidateRect(hwnd, NULL);
			}
			else
			{
				RECT r;
				if(GetUpdateRect(hwnd, &r, FALSE)) 
				{
					PAINTSTRUCT ps;
					
					HDC hdc = BeginPaint(hwnd, &ps);
					Draw(hwnd, hdc);
					EndPaint(hwnd, &ps);
				}
			}
			
			return TRUE;
		}

		case WM_SIZE:
		{
			//InvalidateRect(hwnd, NULL, FALSE);
			MyDetailsFrameData *data = (MyDetailsFrameData *)GetWindowLong(hwnd, GWL_USERDATA);
			RedrawFrame();
			break;
		}

		case WM_TIMER:
		{
			MyDetailsFrameData *data = (MyDetailsFrameData *)GetWindowLong(hwnd, GWL_USERDATA);

			if (wParam == ID_FRAME_TIMER)
			{
				if (!data->showing_menu)
					CallService(MS_MYDETAILS_SHOWNEXTPROTOCOL, 0, 0);
			}
			else if (wParam == ID_RECALC_TIMER)
			{
				KillTimer(hwnd, ID_RECALC_TIMER);

				PostMessage(hwnd, MWM_REFRESH_DATA, 0, 0);
			}
			else if (wParam == ID_STATUSMESSAGE_TIMER)
			{
				SetStatusMessageRefreshTime(hwnd);

				PostMessage(hwnd, MWM_REFRESH_DATA, 0, 0);
			}

			return TRUE;
		}

		case WM_LBUTTONUP:
		{
			MyDetailsFrameData *data = (MyDetailsFrameData *)GetWindowLong(hwnd, GWL_USERDATA);
			Protocol *proto = GetCurrentProtocol();
			if (proto == NULL)
				break;

			POINT p;
			p.x = LOWORD(lParam); 
			p.y = HIWORD(lParam); 

			// In proto cycle button?
			if (data->proto_cycle_next.hitTest(p))
			{
				CallService(MS_MYDETAILS_SHOWNEXTPROTOCOL, 0, 0);
			}
			else if (data->proto_cycle_prev.hitTest(p))
			{
				CallService(MS_MYDETAILS_SHOWPREVIOUSPROTOCOL, 0, 0);
			}
			// In image?
			else if (data->avatar.hitTest(p) && proto->CanSetAvatar())
			{
				if (opts.global_on_avatar)
					CallService(MS_MYDETAILS_SETMYAVATARUI, 0, 0);
				else
					CallService(MS_MYDETAILS_SETMYAVATARUI, 0, (LPARAM) proto->GetName());
			}
			// In nick?
			else if (data->nick.hitTest(p) && proto->CanSetNick())
			{
				if (opts.global_on_nickname)
					CallService(MS_MYDETAILS_SETMYNICKNAMEUI, 0, 0);
				else
					CallService(MS_MYDETAILS_SETMYNICKNAMEUI, 0, (LPARAM) proto->GetName());
			}
			// In status message?
			else if (data->away_msg.hitTest(p) && proto->CanSetStatusMsg())
			{
				if (opts.global_on_status_message)
					CallService(MS_MYDETAILS_SETMYSTATUSMESSAGEUI, 0, 0);
				else
					CallService(MS_MYDETAILS_SETMYSTATUSMESSAGEUI, 0, (LPARAM) proto->GetName());
			}
			// In status?
			else if (data->status.hitTest(p))
			{
				data->showing_menu = true;

				if (opts.global_on_status)
					ShowGlobalStatusMenu(hwnd, data, proto, p);
				else
					ShowProtocolStatusMenu(hwnd, data, proto, p);

				data->showing_menu = false;
			}
			// In listening to?
			else if (data->listening_to.hitTest(p) && protocols->CanSetListeningTo())
			{
				ShowListeningToMenu(hwnd, data, proto, p);
			}
			// In protocol?
			else if (data->proto.hitTest(p))
			{
				data->showing_menu = true;

				HMENU menu = CreatePopupMenu();

				std::vector<Protocol> protos;
				GetProtocols(&protos);

				int current = GetCurrentProtocolIndex();
				
				int protosSize = (int) protos.size();
				for (int i = protosSize - 1 ; i >= 0 ; i--)
				{
					Protocol &proto = protos[i];

					MENUITEMINFO mii = {0};
					mii.cbSize = sizeof(mii);
					mii.fMask = MIIM_ID | MIIM_TYPE;
					mii.fType = MFT_STRING;
					mii.dwTypeData = (char *) proto.GetDescription();
					mii.cch = strlen(mii.dwTypeData);
					mii.wID = i + 1;

					if (i == current)
					{
						mii.fMask |= MIIM_STATE;
						mii.fState = MFS_DISABLED;
					}

					InsertMenuItem(menu, 0, TRUE, &mii);
				}
				
				int ret = ShowPopupMenu(hwnd, menu, data->proto);

				DestroyMenu(menu);

				if (ret != 0)
					PluginCommand_ShowProtocol(NULL, (WPARAM) GetProtocolByIndex(ret - 1).GetName());

				data->showing_menu = false;
			}

			break;
		}

		case WM_MEASUREITEM:
		{
			return CallService(MS_CLIST_MENUMEASUREITEM,wParam,lParam);
		}
		case WM_DRAWITEM:
		{
			return CallService(MS_CLIST_MENUDRAWITEM,wParam,lParam);
		}

		case WM_CONTEXTMENU:
		{
			MyDetailsFrameData *data = (MyDetailsFrameData *)GetWindowLong(hwnd, GWL_USERDATA);
			Protocol *proto = GetCurrentProtocol();
			if (proto == NULL)
				break;

			POINT p;
			p.x = LOWORD(lParam); 
			p.y = HIWORD(lParam); 

			ScreenToClient(hwnd, &p);

			data->showing_menu = true;

			// In proto cycle button?
			if (data->proto_cycle_next.hitTest(p))
			{
				CallService(MS_MYDETAILS_SHOWPREVIOUSPROTOCOL, 0, 0);
			}
			else if (data->proto_cycle_prev.hitTest(p))
			{
				CallService(MS_MYDETAILS_SHOWNEXTPROTOCOL, 0, 0);
			}
			// In image?
			else if (data->avatar.hitTest(p))
			{
				HMENU menu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1));
				HMENU submenu = GetSubMenu(menu, 4);
				CallService(MS_LANGPACK_TRANSLATEMENU,(WPARAM)submenu,0);

				// Add this proto to menu
				char tmp[128];
				mir_snprintf(tmp, sizeof(tmp), Translate("Set My Avatar for %s..."), proto->GetDescription());

				MENUITEMINFO mii = {0};
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_ID | MIIM_TYPE;
				mii.fType = MFT_STRING;
				mii.dwTypeData = tmp;
				mii.cch = strlen(tmp);
				mii.wID = 1;

				if (!proto->CanSetAvatar())
				{
					mii.fMask |= MIIM_STATE;
					mii.fState = MFS_DISABLED;
				}

				InsertMenuItem(submenu, 0, TRUE, &mii);
				
				ClientToScreen(hwnd, &p);
	
				int ret = TrackPopupMenu(submenu, TPM_TOPALIGN|TPM_LEFTALIGN|TPM_RIGHTBUTTON|TPM_RETURNCMD, p.x, p.y, 0, hwnd, NULL);
				DestroyMenu(menu);

				switch(ret)
				{
					case 1:
					{
						CallService(MS_MYDETAILS_SETMYAVATARUI, 0, (LPARAM) proto->GetName());
						break;
					}
					case ID_AVATARPOPUP_SETMYAVATAR:
					{
						CallService(MS_MYDETAILS_SETMYAVATARUI, 0, 0);
						break;
					}
				}
			}
			// In nick?
			else if (data->nick.hitTest(p))
			{
				HMENU menu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1));
				HMENU submenu = GetSubMenu(menu, 2);
				CallService(MS_LANGPACK_TRANSLATEMENU,(WPARAM)submenu,0);

				// Add this proto to menu
				char tmp[128];
				mir_snprintf(tmp, sizeof(tmp), Translate("Set My Nickname for %s..."), proto->GetDescription());

				MENUITEMINFO mii = {0};
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_ID | MIIM_TYPE;
				mii.fType = MFT_STRING;
				mii.dwTypeData = tmp;
				mii.cch = strlen(tmp);
				mii.wID = 1;

				if (!proto->CanSetNick())
				{
					mii.fMask |= MIIM_STATE;
					mii.fState = MFS_DISABLED;
				}

				InsertMenuItem(submenu, 0, TRUE, &mii);
				
				ClientToScreen(hwnd, &p);
	
				int ret = TrackPopupMenu(submenu, TPM_TOPALIGN|TPM_LEFTALIGN|TPM_RIGHTBUTTON|TPM_RETURNCMD, p.x, p.y, 0, hwnd, NULL);
				DestroyMenu(menu);

				switch(ret)
				{
					case 1:
					{
						CallService(MS_MYDETAILS_SETMYNICKNAMEUI, 0, (LPARAM) proto->GetName());
						break;
					}
					case ID_NICKPOPUP_SETMYNICKNAME:
					{
						CallService(MS_MYDETAILS_SETMYNICKNAMEUI, 0, 0);
						break;
					}
				}
			}
			// In status message?
			else if (data->away_msg.hitTest(p))
			{
				char tmp[128];

				HMENU menu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1));
				HMENU submenu = GetSubMenu(menu, 3);
				CallService(MS_LANGPACK_TRANSLATEMENU,(WPARAM)submenu,0);

				// Add this proto to menu
				mir_snprintf(tmp, sizeof(tmp), Translate("Set My Status Message for %s..."), 
							 proto->GetDescription());

				MENUITEMINFO mii = {0};
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_ID | MIIM_TYPE;
				mii.fType = MFT_STRING;
				mii.dwTypeData = tmp;
				mii.cch = strlen(tmp);
				mii.wID = 1;

				if (!proto->CanSetStatusMsg())
				{
					mii.fMask |= MIIM_STATE;
					mii.fState = MFS_DISABLED;
				}

				InsertMenuItem(submenu, 0, TRUE, &mii);
				
				{
					// Add this to menu
					mir_snprintf(tmp, sizeof(tmp), Translate("Set My Status Message for %s..."), 
								 CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, proto->GetStatus(), 0));

					MENUITEMINFO mii = {0};
					mii.cbSize = sizeof(mii);
					mii.fMask = MIIM_ID | MIIM_TYPE;
					mii.fType = MFT_STRING;
					mii.dwTypeData = tmp;
					mii.cch = strlen(tmp);
					mii.wID = 2;

					if (proto->GetStatus() == ID_STATUS_OFFLINE)
					{
						mii.fMask |= MIIM_STATE;
						mii.fState = MFS_DISABLED;
					}

					InsertMenuItem(submenu, 0, TRUE, &mii);
				}
				
				ClientToScreen(hwnd, &p);
	
				int ret = TrackPopupMenu(submenu, TPM_TOPALIGN|TPM_LEFTALIGN|TPM_RIGHTBUTTON|TPM_RETURNCMD, p.x, p.y, 0, hwnd, NULL);
				DestroyMenu(menu);

				switch(ret)
				{
					case 1:
					{
						CallService(MS_MYDETAILS_SETMYSTATUSMESSAGEUI, 0, (LPARAM) proto->GetName());
						break;
					}
					case 2:
					{
						CallService(MS_MYDETAILS_SETMYSTATUSMESSAGEUI, (WPARAM) proto->GetStatus(), 0);
						break;
					}
					case ID_STATUSMESSAGEPOPUP_SETMYSTATUSMESSAGE:
					{
						CallService(MS_MYDETAILS_SETMYSTATUSMESSAGEUI, 0, 0);
						break;
					}
				}
			}
			// In status?
			else if (data->status.hitTest(p))
			{
				if (opts.global_on_status)
					ShowProtocolStatusMenu(hwnd, data, proto, p);
				else
					ShowGlobalStatusMenu(hwnd, data, proto, p);
			}
			// In listening to?
			else if (data->listening_to.hitTest(p) && protocols->CanSetListeningTo())
			{
				ShowListeningToMenu(hwnd, data, proto, p);
			}
			// In protocol?
			else if (data->proto.hitTest(p))
			{
			}
			// Default context menu
			else 
			{
				HMENU menu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1));
				HMENU submenu = GetSubMenu(menu, 1);
				CallService(MS_LANGPACK_TRANSLATEMENU,(WPARAM)submenu,0);

				if (opts.cycle_through_protocols)
					RemoveMenu(submenu, ID_CYCLE_THROUGH_PROTOS, MF_BYCOMMAND);
				else
					RemoveMenu(submenu, ID_DONT_CYCLE_THROUGH_PROTOS, MF_BYCOMMAND);

				// Add this proto to menu
				char tmp[128];
				MENUITEMINFO mii = {0};

				mir_snprintf(tmp, sizeof(tmp), Translate("Enable Listening To for %s"), proto->GetDescription());

				ZeroMemory(&mii, sizeof(mii));
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_ID | MIIM_TYPE | MIIM_STATE;
				mii.fType = MFT_STRING;
				mii.fState = proto->ListeningToEnabled() ? MFS_CHECKED : 0;
				mii.dwTypeData = tmp;
				mii.cch = strlen(tmp);
				mii.wID = 5;

				if (!proto->CanSetListeningTo())
				{
					mii.fState |= MFS_DISABLED;
				}

				InsertMenuItem(submenu, 0, TRUE, &mii);

				// Add this to menu
				mir_snprintf(tmp, sizeof(tmp), Translate("Set My Status Message for %s..."), 
							 CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, proto->GetStatus(), 0));

				ZeroMemory(&mii, sizeof(mii));
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_ID | MIIM_TYPE;
				mii.fType = MFT_STRING;
				mii.dwTypeData = tmp;
				mii.cch = strlen(tmp);
				mii.wID = 4;

				if (proto->GetStatus() == ID_STATUS_OFFLINE)
				{
					mii.fMask |= MIIM_STATE;
					mii.fState = MFS_DISABLED;
				}

				InsertMenuItem(submenu, 0, TRUE, &mii);

				// Add this proto to menu
				mir_snprintf(tmp, sizeof(tmp), Translate("Set My Status Message for %s..."), proto->GetDescription());

				ZeroMemory(&mii, sizeof(mii));
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_ID | MIIM_TYPE;
				mii.fType = MFT_STRING;
				mii.dwTypeData = tmp;
				mii.cch = strlen(tmp);
				mii.wID = 3;

				if (!proto->CanSetStatusMsg())
				{
					mii.fMask |= MIIM_STATE;
					mii.fState = MFS_DISABLED;
				}

				InsertMenuItem(submenu, 0, TRUE, &mii);

				mir_snprintf(tmp, sizeof(tmp), Translate("Set My Nickname for %s..."), proto->GetDescription());

				ZeroMemory(&mii, sizeof(mii));
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_ID | MIIM_TYPE;
				mii.fType = MFT_STRING;
				mii.dwTypeData = tmp;
				mii.cch = strlen(tmp);
				mii.wID = 2;

				if (!proto->CanSetNick())
				{
					mii.fMask |= MIIM_STATE;
					mii.fState = MFS_DISABLED;
				}

				InsertMenuItem(submenu, 0, TRUE, &mii);

				mir_snprintf(tmp, sizeof(tmp), Translate("Set My Avatar for %s..."), proto->GetDescription());

				ZeroMemory(&mii, sizeof(mii));
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_ID | MIIM_TYPE;
				mii.fType = MFT_STRING;
				mii.dwTypeData = tmp;
				mii.cch = strlen(tmp);
				mii.wID = 1;

				if (!proto->CanSetAvatar())
				{
					mii.fMask |= MIIM_STATE;
					mii.fState = MFS_DISABLED;
				}

				InsertMenuItem(submenu, 0, TRUE, &mii);

				ZeroMemory(&mii, sizeof(mii));
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_STATE;
				mii.fState = protocols->ListeningToEnabled() ? MFS_CHECKED : 0;

				if (!protocols->CanSetListeningTo())
				{
					mii.fState |= MFS_DISABLED;
				}

				SetMenuItemInfo(submenu, ID_CONTEXTPOPUP_ENABLELISTENINGTO, FALSE, &mii);

				ClientToScreen(hwnd, &p);
	
				int ret = TrackPopupMenu(submenu, TPM_TOPALIGN|TPM_LEFTALIGN|TPM_RIGHTBUTTON|TPM_RETURNCMD, p.x, p.y, 0, hwnd, NULL);
				DestroyMenu(menu);

				switch(ret)
				{
					case 1:
					{
						CallService(MS_MYDETAILS_SETMYAVATARUI, 0, (LPARAM) proto->GetName());
						break;
					}
					case ID_AVATARPOPUP_SETMYAVATAR:
					{
						CallService(MS_MYDETAILS_SETMYAVATARUI, 0, 0);
						break;
					}
					case 2:
					{
						CallService(MS_MYDETAILS_SETMYNICKNAMEUI, 0, (LPARAM) proto->GetName());
						break;
					}
					case ID_NICKPOPUP_SETMYNICKNAME:
					{
						CallService(MS_MYDETAILS_SETMYNICKNAMEUI, 0, 0);
						break;
					}
					case 3:
					{
						CallService(MS_MYDETAILS_SETMYSTATUSMESSAGEUI, 0, (LPARAM) proto->GetName());
						break;
					}
					case 4:
					{
						CallService(MS_MYDETAILS_SETMYSTATUSMESSAGEUI, (WPARAM) proto->GetStatus(), 0);
						break;
					}
					case ID_STATUSMESSAGEPOPUP_SETMYSTATUSMESSAGE:
					{
						CallService(MS_MYDETAILS_SETMYSTATUSMESSAGEUI, 0, 0);
						break;
					}
					case 5:
					{
						CallService(MS_LISTENINGTO_ENABLE, (LPARAM) proto->GetName(), !proto->ListeningToEnabled());
						break;
					}
					case ID_CONTEXTPOPUP_ENABLELISTENINGTO:
					{
						CallService(MS_LISTENINGTO_ENABLE, 0, !protocols->ListeningToEnabled());
						break;
					}
					case ID_SHOW_NEXT_PROTO:
					{
						CallService(MS_MYDETAILS_SHOWNEXTPROTOCOL, 0, 0);
						break;
					}
					case ID_SHOW_PREV_PROTO:
					{
						CallService(MS_MYDETAILS_SHOWPREVIOUSPROTOCOL, 0, 0);
						break;
					}
					case ID_CYCLE_THROUGH_PROTOS:
					{
						CallService(MS_MYDETAILS_CYCLE_THROUGH_PROTOCOLS, TRUE, 0);
						break;
					}
					case ID_DONT_CYCLE_THROUGH_PROTOS:
					{
						CallService(MS_MYDETAILS_CYCLE_THROUGH_PROTOCOLS, FALSE, 0);
						break;
					}
				}
			}

			data->showing_menu = false;


			break;
		}

		case WM_NCMOUSELEAVE:
		case WM_MOUSELEAVE:
		{
			MyDetailsFrameData *data = (MyDetailsFrameData *)GetWindowLong(hwnd, GWL_USERDATA);
			data->tracking_exit = false;
		}
		case WM_NCMOUSEMOVE:
		{
			MyDetailsFrameData *data = (MyDetailsFrameData *)GetWindowLong(hwnd, GWL_USERDATA);

			bool changed = false;
			for(size_t i = 0; i < data->items.size(); ++i)
				changed = changed || data->items[i]->setMouseOver(NULL);

			if (changed)
				InvalidateRect(hwnd, NULL, FALSE);

			break;
		}

		case WM_MOUSEMOVE:
		{
			MyDetailsFrameData *data = (MyDetailsFrameData *)GetWindowLong(hwnd, GWL_USERDATA);
			Protocol *proto = GetCurrentProtocol();
			if (proto == NULL)
				break;

			if (!data->tracking_exit)
			{
				TRACKMOUSEEVENT tme;
				tme.cbSize = sizeof(TRACKMOUSEEVENT);
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = hwnd;
				tme.dwHoverTime = HOVER_DEFAULT;
				TrackMouseEvent(&tme);

				data->tracking_exit = true;
			}

			POINT p;
			p.x = LOWORD(lParam); 
			p.y = HIWORD(lParam); 

			bool changed = false;
			for(size_t i = 0; i < data->items.size(); ++i)
				changed = changed || data->items[i]->setMouseOver(&p);

			if (changed)
				InvalidateRect(hwnd, NULL, FALSE);

			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR lpnmhdr = (LPNMHDR) lParam;

			int i = (int) lpnmhdr->code;

			switch (lpnmhdr->code) {
				case TTN_GETDISPINFO:
				{
					MyDetailsFrameData *data = (MyDetailsFrameData *)GetWindowLong(hwnd, GWL_USERDATA);

					LPNMTTDISPINFO lpttd = (LPNMTTDISPINFO) lpnmhdr;
					SendMessage(lpnmhdr->hwndFrom, TTM_SETMAXTIPWIDTH, 0, 300);
					
					for(int i = 0; i < data->items.size(); i++)
					{
						lpttd->lpszText = (char *) data->items[i]->getToolTipFor(lpnmhdr->hwndFrom);
						if (lpttd->lpszText != NULL)
							break;
					}

					return 0;
				}
			}

			break;
		}

		case WM_DESTROY:
		{
			KillTimer(hwnd, ID_FRAME_TIMER);

			MyDetailsFrameData *tmp = (MyDetailsFrameData *)GetWindowLong(hwnd, GWL_USERDATA);
			if (tmp != NULL) delete tmp;

			break;
		}

		// Custom Messages //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		case MWM_REFRESH:
		{
			KillTimer(hwnd, ID_RECALC_TIMER);
			SetTimer(hwnd, ID_RECALC_TIMER, RECALC_TIME, NULL);
			break;
		}

		case MWM_REFRESH_DATA:
		{
			Protocol *proto = GetCurrentProtocol(false);
			if (proto)
			{
				proto->UpdateAll();
				RedrawFrame();
			}
			break;
		}
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}


int ShowHideFrameFunc(WPARAM wParam, LPARAM lParam) 
{
	if (ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) 
	{
		CallService(MS_CLIST_FRAMES_SHFRAME, frame_id, 0);
	}
	else
	{
		if (MyDetailsFrameVisible())
		{
			SendMessage(hwnd_container, WM_CLOSE, 0, 0);
		}
		else 
		{
			ShowWindow(hwnd_container, SW_SHOW);
			DBWriteContactSettingByte(0, MODULE_NAME, SETTING_FRAME_VISIBLE, 1);
		}

		FixMainMenu();
	}
	return 0;
}


int ShowFrameFunc(WPARAM wParam, LPARAM lParam)
{
	if (ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) 
	{
		int flags = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, frame_id), 0);
		if(!(flags & F_VISIBLE)) 
			CallService(MS_CLIST_FRAMES_SHFRAME, frame_id, 0);
	}
	else
	{
		if (!MyDetailsFrameVisible())
		{
			ShowWindow(hwnd_container, SW_SHOW);
			DBWriteContactSettingByte(0, MODULE_NAME, SETTING_FRAME_VISIBLE, 1);

			FixMainMenu();
		}

	}
	return 0;
}


int HideFrameFunc(WPARAM wParam, LPARAM lParam)
{
	if (ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) 
	{
		int flags = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, frame_id), 0);
		if (flags & F_VISIBLE) 
			CallService(MS_CLIST_FRAMES_SHFRAME, frame_id, 0);
	}
	else
	{
		if (MyDetailsFrameVisible())
		{
			SendMessage(hwnd_container, WM_CLOSE, 0, 0);

			FixMainMenu();
		}
	}
	return 0;
}


void FixMainMenu() 
{
	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(CLISTMENUITEM);
	mi.flags = CMIM_NAME;

	if(MyDetailsFrameVisible())
		mi.pszName = Translate("Hide My Details");
	else
		mi.pszName = Translate("Show My Details");

	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuShowHideFrame, (LPARAM)&mi);
}

#include <math.h>

void RedrawFrame() 
{
	if (frame_id == -1) 
	{
		InvalidateRect(hwnd_container, NULL, TRUE);
	}
	else
	{
		CallService(MS_CLIST_FRAMES_UPDATEFRAME, (WPARAM)frame_id, (LPARAM)FU_TBREDRAW | FU_FMREDRAW);
	}
}

void UpdateFrameData() 
{
	if (hwnd_frame != NULL)
		PostMessage(hwnd_frame, MWM_REFRESH, 0, 0);
}

// only used when no multiwindow functionality is available
BOOL MyDetailsFrameVisible() 
{
	return IsWindowVisible(hwnd_container) ? true : false;
}

void SetMyDetailsFrameVisible(BOOL visible) 
{
	if (frame_id == -1 && hwnd_container != 0) 
	{
		ShowWindow(hwnd_container, visible ? SW_SHOW : SW_HIDE);
	}
}

void SetCycleTime()
{
	if (hwnd_frame != NULL)
		SetCycleTime(hwnd_frame);
}

void SetCycleTime(HWND hwnd)
{
	KillTimer(hwnd, ID_FRAME_TIMER);

	if (opts.cycle_through_protocols)
		SetTimer(hwnd, ID_FRAME_TIMER, opts.seconds_to_show_protocol * 1000, 0);
}

void SetStatusMessageRefreshTime()
{
	if (hwnd_frame != NULL)
		SetStatusMessageRefreshTime(hwnd_frame);
}

void SetStatusMessageRefreshTime(HWND hwnd)
{
	KillTimer(hwnd, ID_STATUSMESSAGE_TIMER);

	opts.refresh_status_message_timer = DBGetContactSettingWord(NULL, "MyDetails", "RefreshStatusMessageTimer",5);
	if (opts.refresh_status_message_timer > 0)
	{
		SetTimer(hwnd, ID_STATUSMESSAGE_TIMER, opts.refresh_status_message_timer * 1000, NULL);
	}
}

int PluginCommand_ShowNextProtocol(WPARAM wParam,LPARAM lParam)
{
	if (hwnd_frame == NULL)
		return -1;

	SetCurrentProtocol(GetCurrentProtocolIndex() + 1);

	SetCycleTime();

	RedrawFrame();

	return 0;
}

int PluginCommand_ShowPreviousProtocol(WPARAM wParam,LPARAM lParam)
{
	if (hwnd_frame == NULL)
		return -1;

	SetCurrentProtocol(GetCurrentProtocolIndex() - 1);

	SetCycleTime();

	RedrawFrame();

	return 0;
}

int PluginCommand_ShowProtocol(WPARAM wParam,LPARAM lParam)
{
	char * proto = (char *)lParam;

	if (proto == NULL)
		return -1;

	int proto_num = GetProtocolIndexByName(proto);
	if (proto_num == -1)
		return -2;

	if (hwnd_frame == NULL)
		return -3;

	SetCurrentProtocol(proto_num);

	SetCycleTime();

	RedrawFrame();

	return 0;
}

int SettingsChangedHook(WPARAM wParam, LPARAM lParam) 
{
	if (hwnd_frame == NULL)
		return 0;

	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;

	if (wParam != NULL)
		return 0;
	
	if (strstr(cws->szModule,"Away"))
	{
		// Status message changed
		UpdateFrameData();
		return 0;
	}
	
	Protocol *proto = GetCurrentProtocol(false);
	if (proto == NULL || strcmp(proto->GetName(), cws->szModule) != 0)
		return 0;

	if (!strcmp(cws->szSetting,"Status")
			|| !strcmp(cws->szSetting,"StatusMood")
			|| !strcmp(cws->szSetting,"XStatusName")
			|| !strcmp(cws->szSetting,"XStatusMsg")
			|| !strcmp(cws->szSetting,"XStatusId")
			|| ( proto->GetCustomStatus() != 0 && !strcmp(cws->szSetting, proto->GetCustomStatusNameKey()) )
			|| ( proto->GetCustomStatus() != 0 && !strcmp(cws->szSetting, proto->GetCustomStatusMessageKey()) ))
	{
		// Status changed
		UpdateFrameData();
	}
	else if(!strcmp(cws->szSetting,"MyHandle")
			|| !strcmp(cws->szSetting,"UIN") 
			|| !strcmp(cws->szSetting,"Nick") 
			|| !strcmp(cws->szSetting,"FirstName") 
			|| !strcmp(cws->szSetting,"e-mail") 
			|| !strcmp(cws->szSetting,"LastName") 
			|| !strcmp(cws->szSetting,"JID"))
	{
		// Name changed
		UpdateFrameData();
	}
	else if (strcmp(cws->szSetting,"ListeningTo") == 0)
	{
		UpdateFrameData();
	}
	else if (strcmp(cws->szSetting,"LockMainStatus") == 0)
	{
		UpdateFrameData();
	}

	return 0;
}

int AvatarChangedHook(WPARAM wParam, LPARAM lParam) 
{
	if (hwnd_frame == NULL)
		return 0;

	Protocol *proto = GetCurrentProtocol(false);
	if (proto == NULL || strcmp(proto->GetName(), (const char *) wParam) != 0)
		return 0;

	UpdateFrameData();

	return 0;
}

int ProtoAckHook(WPARAM wParam, LPARAM lParam)
{
	if (hwnd_frame == NULL)
		return 0;

	ACKDATA *ack = (ACKDATA*) lParam;
	if (ack->hContact != NULL)
		return 0;

	Protocol *proto = GetCurrentProtocol(false);
	if (proto == NULL || strcmp(proto->GetName(), ack->szModule) != 0)
		return 0;

	if (ack->type == ACKTYPE_STATUS) 
	{
		UpdateFrameData();
	}
	else if (ack->type == ACKTYPE_AWAYMSG)
	{
		UpdateFrameData();
	}
	else if (ack->type == ACKTYPE_EMAIL)
	{
		UpdateFrameData();
	}

	return 0;
}

int ListeningtoEnableStateChangedHook(WPARAM wParam,LPARAM lParam)
{
	if (hwnd_frame == NULL)
		return 0;

	Protocol *proto = GetCurrentProtocol(false);
	if (proto == NULL || strcmp(proto->GetName(), (const char *) wParam) != 0)
		return 0;

	UpdateFrameData();

	return 0;
}

int AccListChanged(WPARAM wParam, LPARAM lParam)
{
	SetCurrentProtocol(0);

	RedrawFrame();

	return 0;
}
