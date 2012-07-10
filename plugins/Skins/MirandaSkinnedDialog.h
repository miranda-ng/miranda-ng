#ifndef __MIRANDA_SKINNED_DIALOG_H__
# define __MIRANDA_SKINNED_DIALOG_H__

#include "SkinLib\SkinnedDialog.h"

class SkinOption;
class MirandaSkinnedDialog;

typedef void (*MirandaSkinnedCallback)(void *param, const MirandaSkinnedDialog *dlg);


class MirandaSkinnedDialog : public SkinnedDialog
{
public:
	MirandaSkinnedDialog(const char *name, const char *description, const char *module);
	virtual ~MirandaSkinnedDialog();

	virtual const char * getDescription() const;
	virtual const char * getModule() const;

	virtual const TCHAR * getSkinName() const;
	virtual void setSkinName(const TCHAR *name);

	virtual bool finishedConfiguring();

	virtual void storeToDB(const SkinOptions *opts);

	virtual void setOnSkinChangedCallback(MirandaSkinnedCallback cb, void *param);

protected:
	virtual int compile();

private:
	std::string description;
	std::string module;
	std::tstring skinName;
	MirandaSkinnedCallback skinChangedCallback;
	void *skinChangedCallbackParam;

	void updateFilename();

	void loadFromDB(SkinOption *opt);
	void storeToDB(const SkinOption *opt);

	bool getSettting(const char *name, bool defVal, bool global = false);
	void setSettting(const char *name, bool val, bool global = false);
	int getSettting(const char *name, int defVal, bool global = false);
	void setSettting(const char *name, int val, bool global = false);
	void getSettting(const char *name, const WCHAR *defVal, std::wstring &ret, bool global = false);
	void setSettting(const char *name, const WCHAR *val, bool global = false);
	void getSettting(const char *name, const char *defVal, std::string &ret, bool global = false);
	void setSettting(const char *name, const char *val, bool global = false);

	inline void getSettingName(char *setting, const char * name, bool global);

	void fireOnSkinChanged();

	void onOptionChange(const SkinOption *opt);

	static void staticOnOptionChange(void *param, const SkinOption *opt);
};


#endif // __MIRANDA_SKINNED_DIALOG_H__
