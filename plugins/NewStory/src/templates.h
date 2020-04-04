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
	wchar_t* val[256];

	__forceinline wchar_t* GetVar(uint8_t id) {
		return val[id];
	}

	__forceinline void SetVar(uint8_t id, wchar_t *v, bool d) {
		if (val[id] && del[id])
			mir_free(val[id]);
		val[id] = mir_wstrdup(v);
		del[id] = d;
	}
};

typedef void(*VarFunc)(int mode, TemplateVars* vars, MCONTACT hContact, HistoryArray::ItemData* item);

struct TemplateInfo
{
	enum { VF_COUNT = 6 };

	char* setting;
	wchar_t* group;
	int  icon;
	wchar_t* title;
	wchar_t* defvalue;
	wchar_t* value;
	wchar_t* tmpValue;
	VarFunc vf[VF_COUNT];
};

enum
{
	TPL_TITLE,

	TPL_MESSAGE,
	TPL_FILE,
	TPL_SIGN,
	TPL_PRESENCE,
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
	TPL_COPY_PRESENCE,

	TPL_COUNT
};

extern TemplateInfo templates[TPL_COUNT];

void LoadTemplates();
void SaveTemplates();

wchar_t* TplFormatString(int tpl, MCONTACT hContact, HistoryArray::ItemData* args);
wchar_t* TplFormatStringEx(int tpl, wchar_t* sztpl, MCONTACT hContact, HistoryArray::ItemData* args);

#endif // __templates_h__