#include "globals.h"
#include "Dialog.h"
#include "DialogState.h"


Dialog::Dialog(const char *aName) : name(aName)
{
}


Dialog::~Dialog()
{
	for(unsigned int i = 0; i < fields.size(); i++) 
		delete fields[i];

	fields.clear();
}


const char * Dialog::getName() const
{
	return name.c_str();
}


bool Dialog::addField(Field *field)
{
	if (getField(field->getName()) != NULL)
		return false;

	fields.push_back(field);
	return true;
}


Field * Dialog::getField(const char *name) const
{
	if (name == NULL || name[0] == 0)
		return NULL;

	for(unsigned int i = 0; i < fields.size(); i++) 
	{
		Field *field = fields[i];
		if (strcmp(name, field->getName()) == 0)
			return field;
	}

	return NULL;
}

Field * Dialog::getField(unsigned int pos) const
{
	if (pos >= fields.size())
		return NULL;
	return fields[pos];
}

int Dialog::getIndexOf(Field *field) const
{
	for(unsigned int i = 0; i < fields.size(); i++) 
	{
		Field *f = fields[i];
		if (field == f)
			return i;
	}

	return -1;
}

unsigned int Dialog::getFieldCount() const
{
	return fields.size();
}

DialogInfo * Dialog::getInfo()
{
	return &info;
}

const Size & Dialog::getSize() const
{
	return size;
}


void Dialog::setSize(const Size &size)
{
	this->size = size;
}


DialogState * Dialog::createState()
{
	DialogState *ret = new DialogState(this);

	for(unsigned int i = 0; i < fields.size(); i++) 
		ret->fields.push_back(fields[i]->createState(ret));

	return ret;
}
