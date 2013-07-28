// protocol service that sends a nudge
#define  PS_SEND_NUDGE  "/SendNudge"

// event to be create in protocol with its m_szModuleName
#define  PE_NUDGE       "/Nudge"

#define	MS_SHAKE_CLIST "SHAKE/Service/ShakeClist"
#define	MS_SHAKE_CHAT  "SHAKE/Service/ShakeChat"
#define	MS_NUDGE_SEND  "NUDGE/Send"

// Hide or Show the context menu "send nudge"
// wParam = char *szProto
// lParam = BOOL show
#define MS_NUDGE_SHOWMENU	"NudgeShowMenu"

#define MUUID_NUDGE_SEND { 0x9c66a9a, 0x57dc, 0x454d, { 0xa9, 0x30, 0xf8, 0xc0, 0x4f, 0xe2, 0x98, 0x38 } }
