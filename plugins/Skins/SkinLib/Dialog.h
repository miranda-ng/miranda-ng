#ifndef __DIALOG_H__
# define __DIALOG_H__

#include <vector>
#include "Field.h"
#include "DialogInfo.h"

class DialogState;


/// It is responsible for freeing the Fields
class Dialog
{
public:
	Dialog(const char *name);
	virtual ~Dialog();

	virtual const char * getName() const;

	virtual bool addField(Field *field);
	virtual Field * getField(const char *name) const;
	virtual Field * getField(unsigned int pos) const;
	virtual int getIndexOf(Field *field) const;
	virtual unsigned int getFieldCount() const;

	virtual DialogInfo * getInfo();

	virtual const Size & getSize() const;
	virtual void setSize(const Size &size);

	virtual DialogState * createState();

private:
	const std::string name;
	std::vector<Field *> fields;
	DialogInfo info;
	Size size;
};


#endif // __DIALOG_H__
