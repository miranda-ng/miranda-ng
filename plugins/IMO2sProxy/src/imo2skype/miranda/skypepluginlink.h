typedef struct {
	int bDelayLogin;
} SKYPEPLUGINLINK_CFG;

#define SKYPE_PROTONAME	"SKYPE"
#define PSS_SKYPEAPIMSG	"/SendSkypeAPIMsg"
#define SKYPE_REGPROXY	"/RegisterProxySvc"

void SkypePluginLink_Defaults (SKYPEPLUGINLINK_CFG *pMyCfg);
IMO2SPROXY *SkypePluginLink_Init (IMO2SPROXY_CFG *pCfg, SKYPEPLUGINLINK_CFG *pMyCfg);
