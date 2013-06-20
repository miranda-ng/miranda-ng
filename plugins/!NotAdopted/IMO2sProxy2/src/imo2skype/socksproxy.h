typedef struct
{
	char *pszAuthPass;
	unsigned long int lAddr;
	short sPort;
	int iMaxConn;
} SOCKSPROXY_CFG;


void SocksProxy_Defaults (SOCKSPROXY_CFG *pMyCfg);
IMO2SPROXY *SocksProxy_Init (IMO2SPROXY_CFG *pCfg, SOCKSPROXY_CFG *pMyCfg);
