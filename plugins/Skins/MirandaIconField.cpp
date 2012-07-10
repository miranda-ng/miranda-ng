#include "MirandaIconField.h"

MirandaIconField::MirandaIconField(MirandaSkinnedDialog *dlg, const char *name, const char *description)
		: IconField(dlg, name)
{
}

MirandaIconField::~MirandaIconField()
{
}

MirandaSkinnedDialog * MirandaIconField::getDialog() const
{
	return (MirandaSkinnedDialog *) IconField::getDialog();
}

void MirandaIconField::configure()
{
}