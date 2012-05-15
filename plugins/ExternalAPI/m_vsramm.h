typedef struct {
	char *szProto;
	char *msg;
	char *uMsg;
	int statusMode;
} PROTOMSGINFO;

// wParam = 0
// lParam = PROTOMSGINFO *
// returns 0 on success
#define MS_VSRAMM_SETAWAYMSG		"VSRAMM/SetAwayMsg"