#ifndef __SKINNED_DIALOG_OPTIONS_H__
# define __SKINNED_DIALOG_OPTIONS_H__

#include <windows.h>
#include "tstring.h"
#include <vector>


enum SkinOptionType
{
	CHECKBOX,
	NUMBER,
	TEXT
};

class SkinOption;

typedef void (*SkinOptionCallback)(void *param, const SkinOption *opt);


class SkinOption
{
public:
	SkinOption(const char *name);
	~SkinOption();

	const char * getName() const;

	SkinOptionType getType() const;
	void setType(SkinOptionType type);

	const TCHAR * getDescription() const;
	void setDescription(const TCHAR * description);

	int getMax() const;
	void setMax(int max);

	int getMin() const;
	void setMin(int min);

	bool getValueCheckbox() const;
	void setValueCheckbox(bool value);

	int getValueNumber() const;
	void setValueNumber(int value);

	const TCHAR * getValueText() const;
	void setValueText(const TCHAR * value);

	void setOnChangeCallback(SkinOptionCallback cb, void *param = NULL);

private:
	std::string name;
	SkinOptionType type;
	std::tstring description;
	bool valueCheckbox;
	int valueNumber;
	int minValue;
	int maxValue;
	std::tstring valueText;

	SkinOptionCallback onChangeCallback;
	void * onChangeCallbackParam;

	void fireOnChange();
};


class SkinOptions
{
public:
	SkinOptions();
	~SkinOptions();

	bool addOption(SkinOption *opt);
	SkinOption * getOption(const char *name) const;
	SkinOption * getOption(unsigned int pos) const;
	unsigned int getNumOptions() const;
	void clearOptions();

	void setOnOptionAddRemoveCallback(SkinOptionCallback cb, void *param = NULL);
	void setOnOptionChangeCallback(SkinOptionCallback cb, void *param = NULL);

private:
	std::vector<SkinOption *> options;

	SkinOptionCallback onAddRemoveCallback;
	void * onAddRemoveCallbackParam;

	SkinOptionCallback onChangeCallback;
	void * onChangeCallbackParam;

	void fireOnAddRemove(SkinOption *opt);
};



#endif // __SKINNED_DIALOG_OPTIONS_H__
