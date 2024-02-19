#ifndef __templates_h__
#define __templates_h__

struct TemplateVars
{
	TemplateVars();
	~TemplateVars();

	struct {
		wchar_t *val;
		bool del;
	}
		vars[256];

	__forceinline wchar_t* GetVar(uint8_t id) {
		return vars[id].val;
	}

	void SetNick(wchar_t *v);
	void SetVar(uint8_t id, wchar_t *v, bool d);
};

typedef void (*VarFunc)(TemplateVars *vars, MCONTACT hContact, ItemData *item);

struct TemplateInfo
{
	char*    setting;
	wchar_t* group;
	int      iIcon;
	wchar_t* title;
	wchar_t* defvalue;
	wchar_t* value;
	wchar_t* tmpValue;
	VarFunc vf[6];

	HICON getIcon() const;
};

enum
{
	TPL_TITLE,

	TPL_MESSAGE,
	TPL_MSG_HEAD,
	TPL_MSG_GRP,
	TPL_FILE,
	TPL_SIGN,
	TPL_PRESENCE,
	TPL_OTHER,

	TPL_AUTH,
	TPL_ADDED,
	TPL_DELETED,

	TPL_COPY_MESSAGE,
	TPL_COPY_FILE,
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

#endif // __templates_h__