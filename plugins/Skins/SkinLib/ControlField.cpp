#include "globals.h"
#include "ControlField.h"
#include "ControlFieldState.h"


ControlField::ControlField(Dialog *dlg, const char *name, HWND aHwnd) 
		: Field(dlg, name), hwnd(aHwnd), textSet(false), hFont(NULL)
{
}


ControlField::~ControlField()
{
}


HWND ControlField::getHWND()
{
	return hwnd;
}



void ControlField::setText(const TCHAR *text)
{
	if (text == NULL)
	{
		if (!textSet)
			return;

		textSet = false;
		this->text.clear();
		fireOnChange();
	}
	else
	{
		textSet = true;
		if (this->text == text)
			return;

		this->text = text;
	//	SetWindowText(hwnd, text);
		fireOnChange();
	}
}


const TCHAR * ControlField::getText()
{
	if (textSet)
		return text.c_str();

	// Control text is the default value
	int length = GetWindowTextLength(hwnd);
	if (length <= 0)
	{
		text = _T("");
	}
	else
	{
		TCHAR *tmp = new TCHAR[length+1];

		if (GetWindowText(hwnd, tmp, length+1) == 0)
			tmp[0] = 0;

		text = tmp;

		delete[] tmp;
	}

	return text.c_str();
}


void ControlField::setFont(HFONT hFont)
{
	if (this->hFont == hFont)
		return;

	this->hFont = hFont;
//	SendMessage(hwnd, WM_SETFONT, (WPARAM) hFont, FALSE);
	fireOnChange();
}


HFONT ControlField::getFont() const
{
	if (hFont != NULL)
		return hFont;
	
	// Control font is the default value
	return (HFONT) SendMessage(hwnd, WM_GETFONT, 0, 0);
}


COLORREF ControlField::getFontColor() const
{
	return GetSysColor(COLOR_WINDOWTEXT);
}


int ControlField::getBorderSize() const
{
	int exstyle = GetWindowLong(hwnd, GWL_EXSTYLE);
	if (exstyle & WS_EX_CLIENTEDGE) 
		return GetSystemMetrics(SM_CXEDGE);
	if (exstyle & WS_EX_STATICEDGE) 
		return GetSystemMetrics(SM_CXBORDER);

	int style = GetWindowLong(hwnd, GWL_STYLE);
	if (style & WS_BORDER) 
		return GetSystemMetrics(SM_CXBORDER);

	return 0;
}

bool ControlField::isScrollVisible(bool horizontal) const
{
	SCROLLBARINFO sbi = {0};
	sbi.cbSize = sizeof(SCROLLBARINFO);
	GetScrollBarInfo(hwnd, horizontal ? OBJID_HSCROLL : OBJID_VSCROLL, &sbi);
	return (sbi.rgstate[0] & STATE_SYSTEM_INVISIBLE) == 0;
}
