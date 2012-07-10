#include "MirandaTextField.h"
#include "MirandaFont.h"


MirandaTextField::MirandaTextField(MirandaSkinnedDialog *dlg, const char *name, const char *aDescription) 
		: TextField(dlg, name)
{
	font = new MirandaFont(this, aDescription);
}

MirandaTextField::~MirandaTextField()
{
	delete font;
}

MirandaSkinnedDialog * MirandaTextField::getDialog() const
{
	return (MirandaSkinnedDialog *) TextField::getDialog();
}

void MirandaTextField::configure()
{
	TextFieldState *field = (TextFieldState *) getDialog()->getDefaultState()->getField(getName());
	_ASSERT(field != NULL);

	font->registerFont(field->getFont());
}
