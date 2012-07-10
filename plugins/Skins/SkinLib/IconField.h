#ifndef __ICON_FIELD_H__
# define __ICON_FIELD_H__

#include "Field.h"


class IconField : public Field
{
public:
	IconField(Dialog *dlg, const char *name);
	virtual ~IconField();

	virtual FieldType getType() const;

	virtual HICON getIcon() const;
	virtual void setIcon(HICON hIcon);

	virtual FieldState * createState(DialogState *dialogState);

private:
	HICON hIcon;

};



#endif // __ICON_FIELD_H__