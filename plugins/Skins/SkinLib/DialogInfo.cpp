#include "globals.h"
#include "DialogInfo.h"
#include "tstring.h"

class DialogInfoValue
{
public:
	DialogInfoValue(const char *aName) : name(aName), type(UNKNOWN) {}

	const char * getName() { return name.c_str(); }

	void set(const TCHAR *value) { valueString = value; type = TYPE_STRING; }
	void set(int value) { valueInt = value; type = TYPE_INT; }
	void set(double value) { valueDouble = value; type = TYPE_DOUBLE; }
	void set(bool value) { valueBool = value; type = TYPE_BOOL; }

	DialogInfoType getType() { return type; }

	const TCHAR * getAsString() { return valueString.c_str(); }
	int getAsInt() { return valueInt; }
	double getAsDouble() { return valueDouble; }
	bool getAsBool() { return valueBool; }

private:
	std::string name;

	DialogInfoType type;

	std::tstring valueString;
	int valueInt;
	double valueDouble;
	bool valueBool;
};

DialogInfo::DialogInfo()
{
}

DialogInfo::~DialogInfo()
{
	for(size_t i = 0; i < values.size(); ++i)
		delete values[i];
}


void DialogInfo::set(const char *name, const TCHAR *value)
{
	bool isVar;
	DialogInfoValue * val = findValue(name, true, &isVar);

	if (isVar)
		return;

	val->set(value);
}

void DialogInfo::set(const char *name, int value)
{
	bool isVar;
	DialogInfoValue * val = findValue(name, true, &isVar);

	if (isVar)
		return;

	val->set(value);
}

void DialogInfo::set(const char *name, double value)
{
	bool isVar;
	DialogInfoValue * val = findValue(name, true, &isVar);

	if (isVar)
		return;

	val->set(value);
}

void DialogInfo::set(const char *name, bool value)
{
	bool isVar;
	DialogInfoValue * val = findValue(name, true, &isVar);

	if (isVar)
		return;

	val->set(value);
}

void DialogInfo::remove(const char *name)
{
	size_t len = strlen(name);

	for(std::vector<DialogInfoValue *>::iterator it = values.begin(); it != values.end(); )
	{
		DialogInfoValue *val = *it;

		if (stricmp(name, val->getName()) == 0)
			it = values.erase(it);
		
		else if (strnicmp(name, val->getName(), len) == 0 && val->getName()[len] == '.')
			it = values.erase(it);
		
		else
			it++;
	}
}

DialogInfoType DialogInfo::getType(const char *name)
{
	bool isVar;
	DialogInfoValue * val = findValue(name, false, &isVar);

	if (isVar)
		return TYPE_VARIABLE;

	else if (val == NULL)
		return UNKNOWN;

	else
		return val->getType();
}


const TCHAR * DialogInfo::getAsString(const char *name)
{
	DialogInfoValue * val = findValue(name);
	if (val == NULL)
		return NULL;
	
	return val->getAsString();
}

int DialogInfo::getAsInt(const char *name)
{
	DialogInfoValue * val = findValue(name);
	if (val == NULL)
		return 0;
	
	return val->getAsInt();
}

double DialogInfo::getAsDouble(const char *name)
{
	DialogInfoValue * val = findValue(name);
	if (val == NULL)
		return 0;
	
	return val->getAsDouble();
}

bool DialogInfo::getAsBool(const char *name)
{
	DialogInfoValue * val = findValue(name);
	if (val == NULL)
		return false;
	
	return val->getAsBool();
}

DialogInfoValue * DialogInfo::findValue(const char *name, bool create, bool *isVar)
{
	size_t len = strlen(name);

	if (isVar != NULL) *isVar = false;

	for(size_t i = 0; i < values.size(); ++i)
	{
		DialogInfoValue *val = values[i];

		if (stricmp(name, val->getName()) == 0)
		{
			return val;
		}
		else if (strnicmp(name, val->getName(), len) == 0 && val->getName()[len] == '.')
		{
			if (isVar != NULL) *isVar = true;
			return val;
		}
	}

	if (create)
	{
		DialogInfoValue *ret = new DialogInfoValue(name);
		values.push_back(ret);
		return ret;
	}

	return NULL;
}
