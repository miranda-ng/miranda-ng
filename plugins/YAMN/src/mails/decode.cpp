/*
 * This code implements decoding encoded MIME header in style
 * =?iso-8859-2?Q? "User using email in central Europe characters such as =E9" ?=
 *
 * (c) majvan 2002-2004
 */
#include "../yamn.h"
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

struct _tcptable CodePageNamesAll[]=
{
	{ "ANSI", "",TRUE,CP_ACP},
	{ "WINDOWS-1", "250",0,1250},
	{ "WINDOWS-1", "251",0,1251},
	{ "WINDOWS-1", "252",0,1252},
	{ "WINDOWS-1", "253",0,1253},
	{ "WINDOWS-1", "254",0,1254},
	{ "WINDOWS-1", "255",0,1255},
	{ "WINDOWS-1", "256",0,1256},
	{ "WINDOWS-1", "257",0,1257},
	{ "WINDOWS-1", "258",0,1258},
	{ "CP1", "250",0,1250},
	{ "CP1", "251",0,1251},
	{ "CP1", "252",0,1252},
	{ "CP1", "253",0,1253},
	{ "CP1", "254",0,1254},
	{ "CP1", "255",0,1255},
	{ "CP1", "256",0,1256},
	{ "CP1", "257",0,1257},
	{ "CP1", "258",0,1258},
	{ "ANSI-1", "250",0,1250},
	{ "ANSI-1", "251",0,1251},
	{ "ANSI-1", "252",0,1252},
	{ "ANSI-1", "253",0,1253},
	{ "ANSI-1", "254",0,1254},
	{ "ANSI-1", "255",0,1255},
	{ "ANSI-1", "256",0,1256},
	{ "ANSI-1", "257",0,1257},
	{ "ANSI-1", "258",0,1258},
	{ "KOI8", "-R",0,20866},
	{ "KOI8", "",0,20866},
	{ "KOI8", "-U",0,21866},
	{ "KOI8", "-RU",0,21866},
	{ "US-", "ASCII",0,20127},
	{ "CP", "367",0,20127},
	{ "ASCII", "",0,20127},
	{ "ASCII", "7",0,20127},
	{ "ISO-8859", "-1",0,28591},
	{ "ISO-8859", "-2",0,28592},
	{ "ISO-8859", "-3",0,28593},
	{ "ISO-8859", "-4",0,28594},
	{ "ISO-8859", "-5",0,28595},
	{ "ISO-8859", "-6",0,28596},
	{ "ISO-8859", "-7",0,28597},
	{ "ISO-8859", "-8",0,28598},
	{ "ISO-8859", "-9",0,28599},
	{ "ISO-8859", "-15",0,28605},
	{ "ISO_8859", "-1",0,28591},
	{ "ISO_8859", "-2",0,28592},
	{ "ISO_8859", "-3",0,28593},
	{ "ISO_8859", "-4",0,28594},
	{ "ISO_8859", "-5",0,28595},
	{ "ISO_8859", "-6",0,28596},
	{ "ISO_8859", "-7",0,28597},
	{ "ISO_8859", "-8",0,28598},
	{ "ISO_8859", "-9",0,28599},
	{ "ISO_8859", "-15",0,28605},
	{ "ISO-", "10646-USC2",0,1200},
	{ "ISO-2022", "/2-JP",0,50220},
	{ "ISO-2022", "-JP",0,50221},
	{ "ISO-2022", "/JIS-JP",0,50222},
	{ "ISO-2022", "-KR",0,50225},
	{ "ISO-2022", "-CH(SP)",0,50227},
	{ "ISO-2022", "-CH(TR)",0,50229},
	{ "UTF-", "7",0,65000},
	{ "UTF-", "8",0,65001},
	{ "ARAB-", "TRANSPARENT",0,710},
	{ "ASMO-", "TRANSPARENT",0,720},
	{ "ASMO-", "449",0,709},
	{ "ASMO-", "708",0,708},
	{ "BIG5", "",0,950},
	{ "EUC-", "CH(SP)",0,51936},
	{ "EUC-", "CH(TR)",0,51950},
	{ "EUC-", "JP",0,51932},
	{ "EUC-", "KR",0,51949},
	{ "GB-", "2312",0,20936},
	{ "GB", "2312",0,20936},
	{ "HZGB-", "2312",0,52936},
	{ "IBM-", "037",0,37},
	{ "IBM-", "290",0,290},
	{ "IBM-", "437",0,437},
	{ "IBM-", "500",0,500},
	{ "IBM-", "775",0,775},
	{ "IBM-", "850",0,850},
	{ "IBM-", "852",0,852},
	{ "IBM-", "855",0,855},
	{ "IBM-", "857",0,857},
	{ "IBM-", "860",0,860},
	{ "IBM-", "861",0,861},
	{ "IBM-", "862",0,862},
	{ "IBM-", "863",0,863},
	{ "IBM-", "864",0,864},
	{ "IBM-", "865",0,865},
	{ "IBM-", "866",0,866},
	{ "IBM-", "869",0,869},
	{ "IBM-", "870",0,870},
	{ "IBM-", "875",0,875},
	{ "IBM-", "1026",0,1026},
	{ "IBM-", "273",0,20273},
	{ "IBM-", "277",0,20277},
	{ "IBM-", "278",0,20278},
	{ "IBM-", "280",0,20280},
	{ "IBM-", "284",0,20284},
	{ "IBM-", "285",0,20285},
	{ "IBM-", "290",0,20290},
	{ "IBM-", "297",0,20297},
	{ "IBM-", "420",0,20420},
	{ "IBM-", "423",0,20423},
	{ "IBM-", "871",0,20871},
	{ "IBM-", "880",0,20880},
	{ "IBM-", "905",0,20905},
	{ "IBM-", "THAI",0,20838},
	{ "ISCII-", "DEVANAGARI",0,57002},
	{ "ISCII-", "BENGALI",0,57003},
	{ "ISCII-", "TAMIL",0,57004},
	{ "ISCII-", "TELUGU",0,57005},
	{ "ISCII-", "ASSAMESE",0,57006},
	{ "ISCII-", "ORIYA",0,57007},
	{ "ISCII-", "KANNADA",0,57008},
	{ "ISCII-", "MALAYALAM",0,57009},
	{ "ISCII-", "GUJARATI",0,57010},
	{ "ISCII-", "PUNJABI",0,57011},
	{ "KOR-", "JOHAB",0,1361},
	{ "KSC-", "5601",0,1361},
	{ "MAC-", "ROMAN",0,10000},
	{ "MAC-", "JP",0,10001},
	{ "MAC-", "CH(SP)(BIG5)",0,10002},
	{ "MAC-", "KR",0,10003},
	{ "MAC-", "AR",0,10004}, 
	{ "MAC-", "HW",0,10005},
	{ "MAC-", "GR",0,10006},
	{ "MAC-", "CY",0,10007},
	{ "MAC-", "CH(SP)(GB2312)",0,10008},
	{ "MAC-", "ROMANIA",0,10010},
	{ "MAC-", "UA",0,10017},
	{ "MAC-", "TH",0,10021},
	{ "MAC-", "LAT2",0,10029},
	{ "MAC-", "ICE",0,10079},
	{ "MAC-", "TR",0,10081},
	{ "MAC-", "CR",0,10082}
};

int CPLENALL = SIZEOF(CodePageNamesAll);
struct _tcptable *CodePageNamesSupp;
int CPLENSUPP = 1;

//Gets codepage ID from string representing charset such as "iso-8859-1"
// input- the string
// size- max length of input string
int GetCharsetFromString(char *input,size_t size);

//HexValue to DecValue ('a' to 10)
// HexValue- hexa value ('a')
// DecValue- poiner where to store dec value
// returns 0 if not success
int FromHexa(char HexValue,char *DecValue);

//Decodes a char from Base64
// Base64Value- input char in Base64
// DecValue- pointer where to store the result
// returns 0 if not success
int FromBase64(char Base64Value,char *DecValue);

//Decodes string in quoted printable
// Src- input string
// Dst- where to store output string
// DstLen- how max long should be output string
// isQ- if is "Q-encoding" modification. should be TRUE in headers
// always returns 1
int DecodeQuotedPrintable(char *Src,char *Dst,int DstLen, BOOL isQ);

//Decodes string in base64
// Src- input string
// Dst- where to store output string
// DstLen- how max long should be output string
// returns 0 if string was not properly decoded
int DecodeBase64(char *Src,char *Dst,int DstLen);

//Converts string to unicode from string with specified codepage
// stream- input string
// cp- codepage of input string
// out- pointer to new allocated memory that contains unicode string
int ConvertStringToUnicode(char *stream,unsigned int cp,WCHAR **out);

//Converts string from MIME header to unicode
// stream- input string
// cp- codepage of input string
// storeto- pointer to memory that contains unicode string
// mode- MIME_PLAIN or MIME_MAIL (MIME_MAIL deletes '"' from start and end of string)
void ConvertCodedStringToUnicode(char *stream,WCHAR **storeto,DWORD cp,int mode);

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

int GetCharsetFromString(char *input,size_t size)
//"ISO-8859-1" to ID from table
{
	char *pin=input;
	char *pout,*parser;

	if ((size<1) || (parser=pout=new char[size+1])==NULL)
		return -1;
	while((*pin != 0) && (pin-input< (INT_PTR)size))
	{
		if ((*pin>='a') && (*pin<='z'))
			*parser++=*(pin++)-('a'-'A'); // make it capital
		//else if (*pin=='\"') // this is already done in ExtractFromContentType
		//	*pin++; //skip the quotes if any
		else 
			*parser++=*pin++;
	}

	*parser = 0;

#ifdef DEBUG_DECODECODEPAGE
	DebugLog(DecodeFile,"<CodePage>%s</CodePage>",pout);
#endif
	for (int i=0;i<CPLENALL;i++) {
		size_t len = mir_strlen(CodePageNamesAll[i].NameBase);
		if (0==strncmp(pout,CodePageNamesAll[i].NameBase,len)) {
			if (0==mir_strcmp(pout+len,CodePageNamesAll[i].NameSub)) {
				delete[] pout;
				return CodePageNamesAll[i].CP;
			}
		}
	}
	delete[] pout;
	return -1;		//not found
}

int FromHexa(char HexValue,char *DecValue)
{
	if (HexValue>='0' && HexValue<='9')
	{
		*DecValue=HexValue-'0';
		return 1;
	}
	if (HexValue>='A' && HexValue<='F')
	{
		*DecValue=HexValue-'A'+10;
		return 1;
	}
	if (HexValue>='a' && HexValue<='f')
	{
		*DecValue=HexValue-'a'+10;
		return 1;
	}
	return 0;
}

int FromBase64(char Base64Value,char *DecValue)
{
	if (Base64Value>='A' && Base64Value<='Z')
	{
		*DecValue=Base64Value-'A';
		return 1;
	}
	if (Base64Value>='a' && Base64Value<='z')
	{
		*DecValue=Base64Value-'a'+26;
		return 1;
	}
	if (Base64Value>='0' && Base64Value<='9')
	{
		*DecValue=Base64Value-'0'+52;
		return 1;
	}
	if (Base64Value=='+')
	{
		*DecValue=Base64Value-'+'+62;
		return 1;
	}
	if (Base64Value=='/')
	{
		*DecValue=Base64Value-'/'+63;
		return 1;
	}
	if (Base64Value=='=')
	{
		*DecValue=0;
		return 1;
	}
	return 0;
}

int DecodeQuotedPrintable(char *Src,char *Dst,int DstLen, BOOL isQ)
{
#ifdef DEBUG_DECODEQUOTED
	char *DstTemp=Dst;
	DebugLog(DecodeFile,"<Decode Quoted><Input>%s</Input>",Src);
#endif
	for (int Counter=0;(*Src != 0) && DstLen && (Counter++<DstLen);Src++,Dst++)
		if (*Src=='=')
		{
			if (!isQ) {
				if (Src[1]==0x0D) {
					Src++; Src++;
					if (Src[0]==0x0A) Src++;
					goto CopyCharQuotedPrintable;
				}
				if (Src[1]==0x0A) {
					Src++; Src++;
					goto CopyCharQuotedPrintable;
				}
			}
			char First,Second;
			if (!FromHexa(*(++Src),&First))
			{
				*Dst++='=';Src--;
				continue;
			}
			if (!FromHexa(*(++Src),&Second))
			{
				*Dst++='=';Src--;Src--;
				continue;
			}
			*Dst=(char)(First)<<4;
			*Dst+=Second;
		}
		else if (isQ && *Src=='_')
			*Dst=' ';
		else
CopyCharQuotedPrintable: // Yeah. Bad programming stile.
			*Dst=*Src;
	*Dst=0;
#ifdef DEBUG_DECODEQUOTED
	DebugLog(DecodeFile,"<Output>%s</Output></Decode Quoted>",DstTemp);
#endif
	return 1;
}

int DecodeBase64(char *Src,char *Dst,int DstLen)
{
	int Result=0;
	char Locator=0,MiniResult[4];
	char *End=Dst+DstLen;

	MiniResult[0]=MiniResult[1]=MiniResult[2]=MiniResult[3]=0;

#ifdef DEBUG_DECODEBASE64
	char *DstTemp=Dst;
	DebugLog(DecodeFile,"<Decode Base64><Input>\n%s\n</Input>\n",Src);
#endif
	while(*Src != 0 && DstLen && Dst != End)
	{
		if ((*Src==0x0D)||(*Src==0x0A))	{
			Src++; 
			continue;
		}
		if ((!(Result=FromBase64(*Src,MiniResult+Locator)) && (*Src==0)) || Locator++==3)	//end_of_str || end_of_4_bytes
		{
			Locator=0;									//next write to the first byte
			*Dst++=(char)((MiniResult[0]<<2) | (MiniResult[1]>>4));
			if (Dst==End) goto end;								//DstLen exceeded?
			*Dst++=(char)((MiniResult[1]<<4) | (MiniResult[2]>>2));
			if (Dst==End) goto end;								//someones don't like goto, but not me
			*Dst++=(char)((MiniResult[2]<<6) | MiniResult[3]);
			if (!Result && (*Src==0)) goto end;						//end of string?
			MiniResult[0]=MiniResult[1]=MiniResult[2]=MiniResult[3]=0;			//zero 4byte buffer for next loop
		}
		if (!Result) return 0;									//unrecognised character occured
		Src++;
	}
end:
	*Dst=0;
#ifdef DEBUG_DECODEBASE64
	DebugLog(DecodeFile,"<Output>\n%s\n</Output></Decode Base64>",DstTemp);
#endif
	return 1;
}



int ConvertStringToUnicode(char *stream,unsigned int cp,WCHAR **out)
{
	CPINFO CPInfo;
	WCHAR *temp,*src=*out,*dest;
	size_t outlen;
	int streamlen,Index;

	//codepages, which require to have set 0 in dwFlags parameter when calling MultiByteToWideChar
	DWORD CodePagesZeroFlags[]={50220,50221,50222,50225,50227,50229,52936,54936,57002,57003,57004,57005,57006,57007,57008,57009,57010,57011,65000,65001};

	if ((cp != CP_ACP) && (cp != CP_OEMCP) && (cp != CP_MACCP) && (cp != CP_THREAD_ACP) && (cp != CP_SYMBOL) && (cp != CP_UTF7) && (cp != CP_UTF8) && !GetCPInfo(cp,&CPInfo))
		cp=CP_ACP;
#ifdef DEBUG_DECODECODEPAGE
	DebugLog(DecodeFile,"<CodePage #>%d</CodePage #>",cp);
#endif
		
	for (Index=0;Index<sizeof(CodePagesZeroFlags)/sizeof(CodePagesZeroFlags[0]);Index++)
		if (CodePagesZeroFlags[Index]==cp)
		{
			Index=-1;
			break;
		}
	if (Index==-1)
		streamlen=MultiByteToWideChar(cp,0,stream,-1,NULL,0);
	else
		streamlen=MultiByteToWideChar(cp,MB_USEGLYPHCHARS,stream,-1,NULL,0);

	if (*out != NULL)
		outlen=mir_wstrlen(*out);
	else
		outlen=0;
	temp=new WCHAR[streamlen+outlen+1];

	if (*out != NULL)
	{
		for (dest=temp;*src != (WCHAR)0;src++,dest++)				//copy old string from *out to temp
			*dest=*src;
//		*dest++=L' ';								//add space?
		delete[] *out;
	}
	else
		dest=temp;
	*out=temp;
	
	if (Index==-1)
	{
		if (!MultiByteToWideChar(cp,0,stream,-1,dest,streamlen))
			return 0;
	}
	else
	{
		if (!MultiByteToWideChar(cp,MB_USEGLYPHCHARS,stream,-1,dest,streamlen))
			return 0;
	}
	return 1;
}

void ConvertCodedStringToUnicode(char *stream,WCHAR **storeto,DWORD cp,int mode)
{
	char *start=stream,*finder,*finderend;
	char Encoding=0;
	char *DecodedResult=NULL;

	if (stream==NULL)
		return;

	while(WS(start)) start++;
	WCHAR *tempstore=0;
	if (!ConvertStringToUnicode(stream,cp,&tempstore))return;

	size_t tempstoreLength = mir_wstrlen(tempstore);
	
	size_t outind = 0;
	while(*start != 0) {
		if (CODES(start)) {
			finder=start+2;finderend=finder;
			while(!CODED(finderend) && !EOS(finderend)) finderend++;
			start = finderend;
			if (CODED(finderend))
			{
				Encoding=*(finderend+1);
				switch(Encoding)
				{
					case 'b':
					case 'B':
					case 'q':
					case 'Q':
						break;
					default:
						goto NotEncoded;
				}
				if (-1==(cp=(DWORD)GetCharsetFromString(finder,finderend-finder)))
					cp=CP_ACP;
				if (Encoding != 0)
				{
					int size,codeend;
					char *pcodeend;

					finder=finderend+2;
					if (CODED(finder))
						finder++;
					while(WS(finder)) finder++;
					finderend=finder;
					while(!CODEE(finderend) && !EOS(finderend)) finderend++;
					if (codeend=CODEE(finderend))
						pcodeend=finderend;
					while(WS(finderend-1)) finderend--;
						if ((mode==MIME_MAIL) && (((*finder=='"') && (*(finderend-1)=='"'))))
					{
						finder++;
						finderend--;
					}
					char *oneWordEncoded = new char[finderend-finder+1];
					strncpy(oneWordEncoded,finder,finderend-finder);
					oneWordEncoded[finderend-finder]=0;
					switch(Encoding)
					{
						case 'b':
						case 'B':
							size=(finderend-finder)*3/4+3+1+1;
							break;
						case 'q':
						case 'Q':
							size=finderend-finder+1+1;
							break;
					}
					if (DecodedResult != NULL)
						delete[] DecodedResult;
					DecodedResult=new char[size+1];
					switch(Encoding)
					{
						case 'q':
						case 'Q':
							DecodeQuotedPrintable(oneWordEncoded,DecodedResult,size, TRUE);
							break;
						case 'b':
						case 'B':
							DecodeBase64(oneWordEncoded,DecodedResult,size);
							break;
					}
					delete[] oneWordEncoded;
					if (codeend)
						finderend=pcodeend+2;
					if (WS(finderend))	//if string continues and there's some whitespace, add space to string that is to be converted
					{
						size_t len=mir_strlen(DecodedResult);
						DecodedResult[len]=' ';
						DecodedResult[len+1]=0;
						finderend++;
					}
					WCHAR *oneWord=0;
					if (ConvertStringToUnicode(DecodedResult,cp,&oneWord)) {
						size_t len = mir_wstrlen(oneWord);
						memcpy(&tempstore[outind],oneWord,len*sizeof(WCHAR));
						outind += len;
					}
					delete oneWord;
					oneWord = 0;
					delete[] DecodedResult; DecodedResult = 0;
					start = finderend;
				} else if (!EOS(start)) start++;
			} else if (!EOS(start)) start++;
		}else{
NotEncoded:
			tempstore[outind] = tempstore[start-stream];
			outind++;
			if (outind > tempstoreLength) break;
			start++;
		}
	}
	tempstore[outind] = 0;
	*storeto = tempstore;
}
