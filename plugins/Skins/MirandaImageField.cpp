#include "MirandaImageField.h"

MirandaImageField::MirandaImageField(MirandaSkinnedDialog *dlg, const char *name, const char *description)
		: ImageField(dlg, name)
{
}

MirandaImageField::~MirandaImageField()
{
}

MirandaSkinnedDialog * MirandaImageField::getDialog() const
{
	return (MirandaSkinnedDialog *) ImageField::getDialog();
}

void MirandaImageField::configure()
{
}