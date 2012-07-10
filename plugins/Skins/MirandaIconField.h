#ifndef __MIRANDA_ICON_FIELD_H__
# define __MIRANDA_ICON_FIELD_H__

#include "commons.h"
#include "MirandaField.h"


class MirandaIconField : public IconField, public MirandaField
{
public:
	MirandaIconField(MirandaSkinnedDialog *dlg, const char *name, const char *description);
	virtual ~MirandaIconField();

	virtual MirandaSkinnedDialog * getDialog() const;

	virtual void configure();
};



#endif // __MIRANDA_ICON_FIELD_H__
