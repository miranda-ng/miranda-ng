#ifndef __MIRANDA_TEXT_FIELD_H__
# define __MIRANDA_TEXT_FIELD_H__

#include "commons.h"
#include "MirandaField.h"

class MirandaFont;


class MirandaTextField : public TextField, public MirandaField
{
public:
	MirandaTextField(MirandaSkinnedDialog *dlg, const char *name, const char *description);
	virtual ~MirandaTextField();

	virtual MirandaSkinnedDialog * getDialog() const;

	virtual void configure();

private:
	MirandaFont *font;

};



#endif // __MIRANDA_TEXT_FIELD_H__
