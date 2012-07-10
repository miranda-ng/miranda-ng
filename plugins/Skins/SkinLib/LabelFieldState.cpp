#include "globals.h"
#include "LabelFieldState.h"


LabelFieldState::LabelFieldState(DialogState *dialog, LabelField *field) 
		: ControlFieldState(dialog, field)
{
}

LabelFieldState::~LabelFieldState()
{
}

Size LabelFieldState::getPreferedSize() const
{
	int style = GetWindowLong(getField()->getHWND(), GWL_STYLE);

	int format = DT_EXPANDTABS | DT_EDITCONTROL;
	if ((style & SS_LEFTNOWORDWRAP) || (style & SS_SIMPLE))
		format |= DT_SINGLELINE;
	if (style & SS_NOPREFIX)
		format |= DT_NOPREFIX;
	Size ret = getTextPreferedSize(format);

	int border = getField()->getBorderSize();
	ret.x += 2 * border;
	ret.y += 2 * border;

	return ret;
}

bool LabelFieldState::isEmpty() const
{
	return lstrlen(getText()) <= 0;
}