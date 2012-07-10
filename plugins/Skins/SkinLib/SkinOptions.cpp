#include "globals.h"
#include "SkinOptions.h"
#include <utf8_helpers.h>


SkinOption::SkinOption(const char *aName) 
		: name(aName), type(CHECKBOX), valueCheckbox(false), valueNumber(0),
		  onChangeCallback(NULL), onChangeCallbackParam(NULL),
		  minValue(MININT), maxValue(MAXINT)
{
	description = Utf8ToTchar(aName);
}

SkinOption::~SkinOption()
{
}

const char * SkinOption::getName() const
{
	return name.c_str();
}

SkinOptionType SkinOption::getType() const
{
	return type;
}

void SkinOption::setType(SkinOptionType type)
{
	if (this->type == type)
		return;

	this->type = type;
	fireOnChange();
}

const TCHAR * SkinOption::getDescription() const
{
	return description.c_str();
}

void SkinOption::setDescription(const TCHAR * description)
{
	if (this->description == description)
		return;

	this->description = description;
	fireOnChange();
}

int SkinOption::getMax() const
{
	return maxValue;
}

void SkinOption::setMax(int max)
{
	this->maxValue = max;
	setValueNumber(valueNumber);
}

int SkinOption::getMin() const
{
	return minValue;
}

void SkinOption::setMin(int min)
{
	this->minValue = min;
	setValueNumber(valueNumber);
}

bool SkinOption::getValueCheckbox() const
{
	return valueCheckbox;
}

void SkinOption::setValueCheckbox(bool value)
{
	if (valueCheckbox == value)
		return;

	valueCheckbox = value;
	fireOnChange();
}

int SkinOption::getValueNumber() const
{
	return max(minValue, min(maxValue, valueNumber));
}

void SkinOption::setValueNumber(int value)
{
	value = max(minValue, min(maxValue, value));

	if (value == valueNumber)
		return;

	valueNumber = value;
	fireOnChange();
}

const TCHAR * SkinOption::getValueText() const
{
	return valueText.c_str();
}

void SkinOption::setValueText(const TCHAR * value)
{
	if (valueText == value)
		return;

	valueText = value;
	fireOnChange();
}

void SkinOption::setOnChangeCallback(SkinOptionCallback cb, void *param /*= NULL*/)
{
	onChangeCallback = cb;
	onChangeCallbackParam = param;
}

void SkinOption::fireOnChange()
{
	if (onChangeCallback != NULL)
		onChangeCallback(onChangeCallbackParam, this);
}



SkinOptions::SkinOptions()
		: onAddRemoveCallback(NULL), onAddRemoveCallbackParam(NULL),
		  onChangeCallback(NULL), onChangeCallbackParam(NULL)
{
}

SkinOptions::~SkinOptions()
{
	for(unsigned int i = 0; i < options.size(); i++)
		delete options[i];
	options.clear();
}

bool SkinOptions::addOption(SkinOption *opt)
{
	_ASSERT(opt != NULL);
	_ASSERT(opt->getName() != NULL);

	if (getOption(opt->getName()) != NULL)
		return false;

	opt->setOnChangeCallback(onChangeCallback, onChangeCallbackParam);
	options.push_back(opt);

	fireOnAddRemove(opt);
	return true;
}

SkinOption * SkinOptions::getOption(const char *name) const
{
	_ASSERT(name != NULL);

	for(unsigned int i = 0; i < options.size(); i++)
	{
		SkinOption *opt = options[i];
		if (strcmp(opt->getName(), name) == 0)
			return opt;
	}
	return NULL;
}

SkinOption * SkinOptions::getOption(unsigned int pos) const
{
	if (pos >= options.size())
		return NULL;
	return options[pos];
}

unsigned int SkinOptions::getNumOptions() const
{
	return options.size();
}

void SkinOptions::clearOptions()
{
	if (options.size() <= 0)
		return;

	for(unsigned int i = 0; i < options.size(); i++)
	{
		fireOnAddRemove(options[i]);
		delete options[i];
	}
	options.clear();
}

void SkinOptions::setOnOptionAddRemoveCallback(SkinOptionCallback cb, void *param /*= NULL*/)
{
	onAddRemoveCallback = cb;
	onAddRemoveCallbackParam = param;
}

void SkinOptions::setOnOptionChangeCallback(SkinOptionCallback cb, void *param /*= NULL*/)
{
	onChangeCallback = cb;
	onChangeCallbackParam = param;

	for(unsigned int i = 0; i < options.size(); i++)
		options[i]->setOnChangeCallback(cb, param);
}

void SkinOptions::fireOnAddRemove(SkinOption *opt)
{
	if (onAddRemoveCallback != NULL)
		onAddRemoveCallback(onAddRemoveCallbackParam, opt);
}
