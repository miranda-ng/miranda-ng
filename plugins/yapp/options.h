#ifndef _OPTIONS_INC
#define _OPTIONS_INC

typedef enum {PL_BOTTOMRIGHT=0, PL_BOTTOMLEFT=1, PL_TOPRIGHT=2, PL_TOPLEFT=3} PopupLocation;
typedef enum {PAV_NONE=0, PAV_LEFT=1, PAV_RIGHT=2} PopupAvLayout;
typedef enum {PT_NONE=0, PT_LEFT=1, PT_RIGHT=2, PT_WITHAV=3} PopupTimeLayout;
typedef struct {
	int win_width, win_max_height, av_size; //tweety
	int default_timeout;
	PopupLocation location;
	int opacity;
	bool border;
	bool round, av_round;
	bool animate;
	bool trans_bg;
	bool use_mim_monitor;
	bool right_icon;
	PopupAvLayout av_layout;
	bool disable_status[10];
	int text_indent;
	bool global_hover;
	PopupTimeLayout time_layout;
	bool disable_full_screen;
	bool drop_shadow;
	int sb_width;
	int padding, av_padding;
} Options;

extern Options options;

void InitOptions();
void LoadOptions();
void LoadModuleDependentOptions();
void DeinitOptions();

#endif
