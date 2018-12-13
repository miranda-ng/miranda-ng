#ifndef __templates_h__
#define __templates_h__

enum
{
	VFM_VARS,
	VFM_DESCS
};

struct TemplateVars
{
	bool del[256];
	TCHAR *val[256];
};
/*
struct VFArgs
{
	HANDLE hContact;
	DBEVENTINFO *event;
};
*/
typedef void(*VarFunc)(int mode, TemplateVars *vars, MCONTACT hContact, HistoryArray::ItemData *item);

struct TemplateInfo
{
	enum { VF_COUNT = 5 };

	char *setting;
	TCHAR *group;
	int  icon;
	TCHAR *title;
	TCHAR *defvalue;
	TCHAR *value;
	TCHAR *tmpValue;
	VarFunc vf[VF_COUNT];
};

enum
{
	TPL_TITLE,
	TPL_PASSWORDQUERY,

	TPL_MESSAGE,
	TPL_FILE,
	TPL_URL,
	TPL_SIGN,
	TPL_OTHER,

	TPL_AUTH,
	TPL_ADDED,
	TPL_DELETED,

	TPL_COPY_MESSAGE,
	TPL_COPY_FILE,
	TPL_COPY_URL,
	TPL_COPY_SIGN,
	TPL_COPY_OTHER,

	TPL_COPY_AUTH,
	TPL_COPY_ADDED,
	TPL_COPY_DELETED,

	TPL_COUNT
};

extern TemplateInfo templates[TPL_COUNT];

void LoadTemplates();
void SaveTemplates();

TCHAR *TplFormatString(int tpl, MCONTACT hContact, HistoryArray::ItemData *args);
TCHAR *TplFormatStringEx(int tpl, TCHAR *sztpl, MCONTACT hContact, HistoryArray::ItemData *args);

INT_PTR CALLBACK OptTemplatesDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif // __templates_h__