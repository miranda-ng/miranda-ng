#ifndef _OPTIONS_INC
#define _OPTIONS_INC

#include "translations.h"

#define LABEL_LEN			1024
#define VALUE_LEN			8192

#define MODULE_NAME_LEN		512
#define SETTING_NAME_LEN	512

typedef struct {
	TCHAR label[LABEL_LEN];
	TCHAR value[VALUE_LEN];
	bool line_above, value_newline;
} DisplayItem;

typedef enum {DVT_DB = 0, DVT_PROTODB = 1} DisplaySubstType;
typedef struct {
	TCHAR name[LABEL_LEN];
	DisplaySubstType type;
	char module_name[MODULE_NAME_LEN];
	char setting_name[SETTING_NAME_LEN];
	int translate_func_id;
} DisplaySubst;

struct DSListNode {
	DisplaySubst ds;
	DSListNode *next;
};

struct DIListNode {
	DisplayItem di;
	DIListNode *next;
};

typedef enum {PAV_NONE=0, PAV_LEFT=1, PAV_RIGHT=2} PopupAvLayout;
typedef enum {PTL_LEFTICON=0, PTL_RIGHTICON=1, PTL_NOICON=2, PTL_NOTITLE=3} PopupTitleLayout;
typedef enum {PP_BOTTOMRIGHT=0, PP_BOTTOMLEFT=1, PP_TOPRIGHT=2, PP_TOPLEFT=3} PopupPosition;

typedef struct {
	int win_width, win_max_height, av_size; //tweety
	int opacity;
	bool border;
	bool round, av_round;
	bool animate;
	bool drop_shadow;
	bool trans_bg;
	PopupTitleLayout title_layout;
	PopupAvLayout av_layout;
	int text_indent;
	bool show_no_focus;
	DSListNode *ds_list;
	int ds_count;
	DIListNode *di_list;
	int di_count;
	int time_in;
	int padding, av_padding, text_padding;
	PopupPosition pos;
	int min_width, min_height; // no UI for these
	int mouse_tollerance;
	bool status_bar_tips;
	int sidebar_width;
	COLORREF bg_col, border_col, div_col, bar_col, title_col, label_col, value_col, sidebar_col;
	int label_valign, label_halign, value_valign, value_halign;
	bool no_resize_av;
	TCHAR bg_fn[MAX_PATH];
	bool stretch_bg_img;
} Options;

extern Options options;

void InitOptions();
void LoadOptions();
void DeinitOptions();

#endif
