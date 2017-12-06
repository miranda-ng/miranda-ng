Memory_U CreateUIC(Skype_Inst *pInst, const char *pszNonce, const char *pszSalt);
int CreateUICString(Skype_Inst *pInst, const char *pszNonce, const char *pszSalt, char *pszOutUIC);
int GetCredentialsUIC(Skype_Inst *pInst, char *pszOutUIC);
