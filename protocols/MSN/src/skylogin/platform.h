int64_t PlatFormSpecific();
void	 InitNodeId(Skype_Inst *pInst);
void FillMiscDatas(Skype_Inst *pInst, unsigned int *Datas);
void FillRndBuffer(unsigned char *Buffer);
Memory_U Credentials_Load(char *pszUser);
int Credentials_Save(Memory_U creds, char *pszUser);
