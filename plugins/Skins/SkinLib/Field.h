#ifndef __FIELD_H__
# define __FIELD_H__

#include <windows.h>
#include <tchar.h>
#include "tstring.h"
#include "Size.h"
#include "Position.h"


enum FieldType
{
	SIMPLE_TEXT = 1,
	SIMPLE_IMAGE,
	SIMPLE_ICON,
	CONTROL_LABEL,
	CONTROL_BUTTON,
	CONTROL_EDIT,
	USER_DEFINED = 0x100
};

class Dialog;
class DialogState;
class Field;
class FieldState;

typedef void (*FieldCallback)(void *param, const Field *field);


class Field
{
public:
	Field(Dialog *dlg, const char *name);
	virtual ~Field();

	virtual Dialog * getDialog() const;
	virtual const char * getName() const;
	virtual FieldType getType() const = 0;

	virtual bool isEnabled() const;
	virtual void setEnabled(bool enabled);

	virtual const TCHAR * getToolTip() const;
	virtual void setToolTip(const TCHAR *tooltip);

	virtual FieldState * createState(DialogState *dialogState) = 0;

	virtual void setOnChangeCallback(FieldCallback cb, void *param = NULL);

protected:
	void fireOnChange() const;

private:
	Dialog *dlg;
	const std::string name;
	bool enabled;
	std::tstring tooltip;

	FieldCallback onChangeCallback;
	void *onChangeCallbackParam;
};




#endif // __FIELD_H__