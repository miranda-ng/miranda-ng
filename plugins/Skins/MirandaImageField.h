#ifndef __MIRANDA_IMAGE_FIELD_H__
# define __MIRANDA_IMAGE_FIELD_H__

#include "commons.h"
#include "MirandaField.h"


class MirandaImageField : public ImageField, public MirandaField
{
public:
	MirandaImageField(MirandaSkinnedDialog *dlg, const char *name, const char *description);
	virtual ~MirandaImageField();

	virtual MirandaSkinnedDialog * getDialog() const;

	virtual void configure();
};


#endif // __MIRANDA_IMAGE_FIELD_H__

