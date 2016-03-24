unsigned int	BytesSHA1(uchar *Data, uint Length);
void	GenSessionKey(uchar *Buffer, uint Size);
void	SpecialSHA(uchar *SessionKey, uint SkSz, uchar *SHAResult, uint ResSz);
uchar		*FinalizeLoginDatas(Skype_Inst *pInst, uchar *Buffer, uint *Size, uchar *Suite, int SuiteSz);
int64_t BytesSHA1I64(uchar *Data, uint Length);
int64_t BytesRandomI64();
void	BuildUnFinalizedDatas(uchar *Datas, uint Size, uchar *Result);
