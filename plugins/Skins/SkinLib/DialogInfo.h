#ifndef __DIALOG_INFO_H__
# define __DIALOG_INFO_H__

#include <windows.h>
#include <vector>


enum DialogInfoType
{
	UNKNOWN = 0,
	TYPE_VARIABLE,
	TYPE_INT,
	TYPE_DOUBLE,
	TYPE_BOOL,
	TYPE_STRING
};


class DialogInfoValue;

class DialogInfo
{
public:
	DialogInfo();
	~DialogInfo();

	void set(const char *name, const TCHAR *value);
	void set(const char *name, int value);
	void set(const char *name, double value);
	void set(const char *name, bool value);

	void remove(const char *name);

	DialogInfoType getType(const char *name);

	const TCHAR * getAsString(const char *name);
	int getAsInt(const char *name);
	double getAsDouble(const char *name);
	bool getAsBool(const char *name);

private:
	std::vector<DialogInfoValue *> values; 

	DialogInfoValue * findValue(const char *name, bool create = false, bool *isVar = NULL);

};



#endif // __DIALOG_INFO_H__