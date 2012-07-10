#include "commons.h"
#include "MirandaSkinnedDialog.h"

#define SETTING_NAME_SIZE 256


MirandaSkinnedDialog::MirandaSkinnedDialog(const char *name, const char *aDescription, const char *aModule) 
		: SkinnedDialog(name), description(aDescription), module(aModule), 
		skinChangedCallback(NULL), skinChangedCallbackParam(NULL)
{
	getSettting("Skin", _T(DEFAULT_SKIN_NAME), skinName, true);
}

MirandaSkinnedDialog::~MirandaSkinnedDialog()
{
}

const char * MirandaSkinnedDialog::getDescription() const
{
	return description.c_str();
}

const char * MirandaSkinnedDialog::getModule() const
{
	return module.c_str();
}

const TCHAR * MirandaSkinnedDialog::getSkinName() const
{
	return skinName.c_str();
}

void MirandaSkinnedDialog::setSkinName(const TCHAR *name)
{
	if (skinName == name)
		return;

	skinName = name;
	setSettting("Skin", skinName.c_str(), true);
	updateFilename();
}

bool MirandaSkinnedDialog::finishedConfiguring()
{
	updateFilename();

	if (getDefaultState() == NULL || getOpts() == NULL)
		return false;

	return true;
}

void MirandaSkinnedDialog::updateFilename()
{
	std::tstring filename;
	getSkinnedDialogFilename(filename, getSkinName(), getName());
	setFilename(filename.c_str());
}

void MirandaSkinnedDialog::loadFromDB(SkinOption *opt)
{
	switch(opt->getType())
	{
		case CHECKBOX:
		{
			opt->setValueCheckbox(getSettting(opt->getName(), opt->getValueCheckbox()));
			break;
		}
		case NUMBER:
		{
			opt->setValueNumber(getSettting(opt->getName(), opt->getValueNumber()));
			break;
		}
		case TEXT:
		{
			std::tstring tmp;
			getSettting(opt->getName(), opt->getValueText(), tmp);
			opt->setValueText(tmp.c_str());
			break;
		}
	}
}

void MirandaSkinnedDialog::storeToDB(const SkinOptions *opts)
{
	for (unsigned int i = 0; i < opts->getNumOptions(); i++)
		storeToDB(opts->getOption(i));

	fireOnSkinChanged();
}

void MirandaSkinnedDialog::storeToDB(const SkinOption *opt)
{
	switch(opt->getType())
	{
		case CHECKBOX:
		{
			setSettting(opt->getName(), opt->getValueCheckbox());
			break;
		}
		case NUMBER:
		{
			setSettting(opt->getName(), opt->getValueNumber());
			break;
		}
		case TEXT:
		{
			setSettting(opt->getName(), opt->getValueText());
			break;
		}
	}
}

bool MirandaSkinnedDialog::getSettting(const char *name, bool defVal, bool global)
{
	char setting[SETTING_NAME_SIZE];
	getSettingName(setting, name, global);

	return DBGetContactSettingByte(NULL, getModule(), setting, defVal ? 1 : 0) != 0;
}

void MirandaSkinnedDialog::setSettting(const char *name, bool val, bool global)
{
	char setting[SETTING_NAME_SIZE];
	getSettingName(setting, name, global);

	DBWriteContactSettingByte(NULL, getModule(), setting, val ? 1 : 0);
}

int MirandaSkinnedDialog::getSettting(const char *name, int defVal, bool global)
{
	char setting[SETTING_NAME_SIZE];
	getSettingName(setting, name, global);

	return DBGetContactSettingDword(NULL, getModule(), setting, defVal);
}

void MirandaSkinnedDialog::setSettting(const char *name, int val, bool global)
{
	char setting[SETTING_NAME_SIZE];
	getSettingName(setting, name, global);

	DBWriteContactSettingDword(NULL, getModule(), setting, val);
}

void MirandaSkinnedDialog::getSettting(const char *name, const WCHAR *defVal, std::wstring &ret, bool global)
{
	char setting[SETTING_NAME_SIZE];
	getSettingName(setting, name, global);

	DBVARIANT dbv = {0};
	if (DBGetContactSettingWString(NULL, getModule(), setting, &dbv))
	{
		ret = defVal;
		return;
	}

	ret = dbv.pwszVal;
	DBFreeVariant(&dbv);
}

void MirandaSkinnedDialog::setSettting(const char *name, const WCHAR *val, bool global)
{
	char setting[SETTING_NAME_SIZE];
	getSettingName(setting, name, global);

	DBWriteContactSettingWString(NULL, getModule(), setting, val);
}

void MirandaSkinnedDialog::getSettting(const char *name, const char *defVal, std::string &ret, bool global)
{
	char setting[SETTING_NAME_SIZE];
	getSettingName(setting, name, global);

	DBVARIANT dbv = {0};
	if (DBGetContactSettingString(NULL, getModule(), setting, &dbv))
	{
		ret = defVal;
		return;
	}

	ret = dbv.pszVal;
	DBFreeVariant(&dbv);
}

void MirandaSkinnedDialog::setSettting(const char *name, const char *val, bool global)
{
	char setting[SETTING_NAME_SIZE];
	getSettingName(setting, name, global);

	DBWriteContactSettingString(NULL, getModule(), setting, val);
}

void MirandaSkinnedDialog::getSettingName(char *setting, const char * name, bool global)
{
	if (global)
		mir_snprintf(setting, SETTING_NAME_SIZE, "%s%s", getName(), name);
	else
		mir_snprintf(setting, SETTING_NAME_SIZE, "%s%s%s", TcharToChar(getSkinName()), getName(), name);
}

void MirandaSkinnedDialog::setOnSkinChangedCallback(MirandaSkinnedCallback cb, void *param)
{
	skinChangedCallback = cb;
	skinChangedCallbackParam = param;
}

void MirandaSkinnedDialog::fireOnSkinChanged()
{
	if (skinChangedCallback != NULL)
		skinChangedCallback(skinChangedCallbackParam, this);
}

int MirandaSkinnedDialog::compile()
{
	int ret = SkinnedDialog::compile();

	if (ret == 2)
	{
		SkinOptions * opts = getOpts();
		_ASSERT(opts != NULL);
		
		for(unsigned int i = 0; i < opts->getNumOptions(); ++i)
		{
			SkinOption *opt = opts->getOption(i);
			loadFromDB(opt);
			opt->setOnChangeCallback(&staticOnOptionChange, this);
		}

		for(unsigned int i = 0; i < getFieldCount(); ++i)
		{
			MirandaField *field = dynamic_cast<MirandaField *>(getField(i));
			field->configure();
		}

		fireOnSkinChanged();
	}

	return ret;
}

void MirandaSkinnedDialog::onOptionChange(const SkinOption *opt)
{
	storeToDB(opt);
}

void MirandaSkinnedDialog::staticOnOptionChange(void *param, const SkinOption *opt)
{
	_ASSERT(param != NULL);

	((MirandaSkinnedDialog *) param)->onOptionChange(opt);
}
