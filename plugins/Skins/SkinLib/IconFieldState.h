#ifndef __ICON_FIELD_STATE_H__
# define __ICON_FIELD_STATE_H__

#include "IconField.h"
#include "FieldState.h"


class IconFieldState : public FieldState
{
public:
	virtual ~IconFieldState();

	virtual IconField * getField() const;

	virtual Size getPreferedSize() const;

	virtual HICON getIcon() const;

	virtual bool isEmpty() const;

private:
	IconFieldState(DialogState *dialog, IconField *field);

	friend class IconField;
};


#endif // __ICON_FIELD_STATE_H__