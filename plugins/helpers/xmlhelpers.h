void AppendXMLOpeningTag(char **szBuf, char *szTag, int *depth);
void AppendXMLClosingTag(char **szBuf, char *szTag, int *depth);
void AppendXMLTag(char **szBuf, char *szTag, char *szAtts, int *depth);

char *GetSettingType(BYTE type);
void AppendXMLTagTString(char **szBuf, char *szTag, TCHAR *tszVal, int *depth);
void AppendXMLTagString(char **szBuf, char *szTag, char *szVal, int *depth);
void AppendXMLTagUtfString(char **szBuf, char *szTag, char *szVal, int *depth);
void AppendXMLTagByte(char **szBuf, char *szTag, BYTE bVal, int *depth);
void AppendXMLTagWord(char **szBuf, char *szTag, WORD wVal, int *depth);
void AppendXMLTagDword(char **szBuf, char *szTag, DWORD dVal, int *depth);
void AppendXMLTagBlob(char **szBuf, char *szTag, BYTE *pBlob, int cbBlob, int *depth);

char *XMLEncodeString(TCHAR *szSrc);
char *XMLEncodeStringA(char *tszSrc);
TCHAR *DecodeXMLString(char *szSrc);
char *DecodeXMLStringA(char *szSrc, BOOL bUtfDecode);
