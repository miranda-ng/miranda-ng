typedef struct {
	int bDelayLogin;
} W32SKYPEEMU_CFG;

void W32SkypeEmu_Defaults (W32SKYPEEMU_CFG *pMyCfg);
IMO2SPROXY *W32SkypeEmu_Init (IMO2SPROXY_CFG *pCfg, W32SKYPEEMU_CFG *pMyCfg);
