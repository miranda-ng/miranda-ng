#include "globals.h"
#include "EditFieldState.h"


EditFieldState::EditFieldState(DialogState *dialog, EditField *field) 
		: ControlFieldState(dialog, field)
{
}

EditFieldState::~EditFieldState()
{
}

Size EditFieldState::getPreferedSize() const
{
	ControlField *field = getField();
	HWND hwnd = field->getHWND();

	int style = GetWindowLong(hwnd, GWL_STYLE);
	int exstyle = GetWindowLong(hwnd, GWL_EXSTYLE);

	int format = DT_NOPREFIX | DT_EDITCONTROL;
	if (!(style & ES_MULTILINE))
		format |= DT_SINGLELINE;
	Size ret = getTextPreferedSize(format);

	RECT rc = {0};
	SetRect(&rc, 0, 0, ret.x, ret.y);
	AdjustWindowRectEx(&rc, style, false, exstyle);

	bool hasHorScroll = field->isScrollVisible(true);
	if (hasHorScroll) 
		rc.bottom += GetSystemMetrics(SM_CYHSCROLL);
	if (field->isScrollVisible(false)) 
		rc.right += GetSystemMetrics(SM_CXVSCROLL);

	int margins = SendMessage(hwnd, EM_GETMARGINS, 0, 0);
	rc.left -= LOWORD(margins);
	rc.right += HIWORD(margins);
	if (hasHorScroll || (style & ES_AUTOHSCROLL)) 
		rc.right++;

	ret.x = rc.right - rc.left;
	ret.y = rc.bottom - rc.top;

	if ((exstyle & WS_EX_CLIENTEDGE) || (exstyle & WS_EX_STATICEDGE) || (style & WS_BORDER))
	{
		ret.x += 3;
		ret.y += 3;
	}

	return ret;
}

bool EditFieldState::isEmpty() const
{
	return false;
}