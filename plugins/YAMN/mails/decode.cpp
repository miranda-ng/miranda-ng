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
	{_T("ANSI"),_T(""),TRUE,CP_ACP},
	{_T("WINDOWS-1"),_T("250"),0,1250},
	{_T("WINDOWS-1"),_T("251"),0,1251},
	{_T("WINDOWS-1"),_T("252"),0,1252},
	{_T("WINDOWS-1"),_T("253"),0,1253},
	{_T("WINDOWS-1"),_T("254"),0,1254},
	{_T("WINDOWS-1"),_T("255"),0,1255},
	{_T("WINDOWS-1"),_T("256"),0,1256},
	{_T("WINDOWS-1"),_T("257"),0,1257},
	{_T("WINDOWS-1"),_T("258"),0,1258},
	{_T("CP1"),_T("250"),0,1250},
	{_T("CP1"),_T("251"),0,1251},
	{_T("CP1"),_T("252"),0,1252},
	{_T("CP1"),_T("253"),0,1253},
	{_T("CP1"),_T("254"),0,1254},
	{_T("CP1"),_T("255"),0,1255},
	{_T("CP1"),_T("256"),0,1256},
	{_T("CP1"),_T("257"),0,1257},
	{_T("CP1"),_T("258"),0,1258},
	{_T("ANSI-1"),_T("250"),0,1250},
	{_T("ANSI-1"),_T("251"),0,1251},
	{_T("ANSI-1"),_T("252"),0,1252},
	{_T("ANSI-1"),_T("253"),0,1253},
	{_T("ANSI-1"),_T("254"),0,1254},
	{_T("ANSI-1"),_T("255"),0,1255},
	{_T("ANSI-1"),_T("256"),0,1256},
	{_T("ANSI-1"),_T("257"),0,1257},
	{_T("ANSI-1"),_T("258"),0,1258},
	{_T("KOI8"),_T("-R"),0,20866},
	{_T("KOI8"),_T(""),0,20866},
	{_T("KOI8"),_T("-U"),0,21866},
	{_T("KOI8"),_T("-RU"),0,21866},
	{_T("US-"),_T("ASCII"),0,20127},
	{_T("CP"),_T("367"),0,20127},
	{_T("ASCII"),_T(""),0,20127},
	{_T("ASCII"),_T("7"),0,20127},
	{_T("ISO-8859"),_T("-1"),0,28591},
	{_T("ISO-8859"),_T("-2"),0,28592},
	{_T("ISO-8859"),_T("-3"),0,28593},
	{_T("ISO-8859"),_T("-4"),0,28594},
	{_T("ISO-8859"),_T("-5"),0,28595},
	{_T("ISO-8859"),_T("-6"),0,28596},
	{_T("ISO-8859"),_T("-7"),0,28597},
	{_T("ISO-8859"),_T("-8"),0,28598},
	{_T("ISO-8859"),_T("-9"),0,28599},
	{_T("ISO-8859"),_T("-15"),0,28605},
	{_T("ISO_8859"),_T("-1"),0,28591},
	{_T("ISO_8859"),_T("-2"),0,28592},
	{_T("ISO_8859"),_T("-3"),0,28593},
	{_T("ISO_8859"),_T("-4"),0,28594},
	{_T("ISO_8859"),_T("-5"),0,28595},
	{_T("ISO_8859"),_T("-6"),0,28596},
	{_T("ISO_8859"),_T("-7"),0,28597},
	{_T("ISO_8859"),_T("-8"),0,28598},
	{_T("ISO_8859"),_T("-9"),0,28599},
	{_T("ISO_8859"),_T("-15"),0,28605},
	{_T("ISO-"),_T("10646-USC2"),0,1200},
	{_T("ISO-2022"),_T("/2-JP"),0,50220},
	{_T("ISO-2022"),_T("-JP"),0,50221},
	{_T("ISO-2022"),_T("/JIS-JP"),0,50222},
	{_T("ISO-2022"),_T("-KR"),0,50225},
	{_T("ISO-2022"),_T("-CH(SP)"),0,50227},
	{_T("ISO-2022"),_T("-CH(TR)"),0,50229},
	{_T("UTF-"),_T("7"),0,65000},
	{_T("UTF-"),_T("8"),0,65001},
	{_T("ARAB-"),_T("TRANSPARENT"),0,710},
	{_T("ASMO-"),_T("TRANSPARENT"),0,720},
	{_T("ASMO-"),_T("449"),0,709},
	{_T("ASMO-"),_T("708"),0,708},
	{_T("BIG5"),_T(""),0,950},
	{_T("EUC-"),_T("CH(SP)"),0,51936},
	{_T("EUC-"),_T("CH(TR)"),0,51950},
	{_T("EUC-"),_T("JP"),0,51932},
	{_T("EUC-"),_T("KR"),0,51949},
	{_T("GB-"),_T("2312"),0,20936},
	{_T("GB"),_T("2312"),0,20936},
	{_T("HZGB-"),_T("2312"),0,52936},
	{_T("IBM-"),_T("037"),0,37},
	{_T("IBM-"),_T("290"),0,290},
	{_T("IBM-"),_T("437"),0,437},
	{_T("IBM-"),_T("500"),0,500},
	{_T("IBM-"),_T("775"),0,775},
	{_T("IBM-"),_T("850"),0,850},
	{_T("IBM-"),_T("852"),0,852},
	{_T("IBM-"),_T("855"),0,855},
	{_T("IBM-"),_T("857"),0,857},
	{_T("IBM-"),_T("860"),0,860},
	{_T("IBM-"),_T("861"),0,861},
	{_T("IBM-"),_T("862"),0,862},
	{_T("IBM-"),_T("863"),0,863},
	{_T("IBM-"),_T("864"),0,864},
	{_T("IBM-"),_T("865"),0,865},
	{_T("IBM-"),_T("866"),0,866},
	{_T("IBM-"),_T("869"),0,869},
	{_T("IBM-"),_T("870"),0,870},
	{_T("IBM-"),_T("875"),0,875},
	{_T("IBM-"),_T("1026"),0,1026},
	{_T("IBM-"),_T("273"),0,20273},
	{_T("IBM-"),_T("277"),0,20277},
	{_T("IBM-"),_T("278"),0,20278},
	{_T("IBM-"),_T("280"),0,20280},
	{_T("IBM-"),_T("284"),0,20284},
	{_T("IBM-"),_T("285"),0,20285},
	{_T("IBM-"),_T("290"),0,20290},
	{_T("IBM-"),_T("297"),0,20297},
	{_T("IBM-"),_T("420"),0,20420},
	{_T("IBM-"),_T("423"),0,20423},
	{_T("IBM-"),_T("871"),0,20871},
	{_T("IBM-"),_T("880"),0,20880},
	{_T("IBM-"),_T("905"),0,20905},
	{_T("IBM-"),_T("THAI"),0,20838},
	{_T("ISCII-"),_T("DEVANAGARI"),0,57002},
	{_T("ISCII-"),_T("BENGALI"),0,57003},
	{_T("ISCII-"),_T("TAMIL"),0,57004},
	{_T("ISCII-"),_T("TELUGU"),0,57005},
	{_T("ISCII-"),_T("ASSAMESE"),0,57006},
	{_T("ISCII-"),_T("ORIYA"),0,57007},
	{_T("ISCII-"),_T("KANNADA"),0,57008},
	{_T("ISCII-"),_T("MALAYALAM"),0,57009},
	{_T("ISCII-"),_T("GUJARATI"),0,57010},
	{_T("ISCII-"),_T("PUNJABI"),0,57011},
	{_T("KOR-"),_T("JOHAB"),0,1361},
	{_T("KSC-"),_T("5601"),0,1361},
	{_T("MAC-"),_T("ROMAN"),0,10000},
	{_T("MAC-"),_T("JP"),0,10001},
	{_T("MAC-"),_T("CH(SP)(BIG5)"),0,10002},
	{_T("MAC-"),_T("KR"),0,10003},
	{_T("MAC-"),_T("AR"),0,10004}, 
	{_T("MAC-"),_T("HW"),0,10005},
	{_T("MAC-"),_T("GR"),0,10006},
	{_T("MAC-"),_T("CY"),0,10007},
	{_T("MAC-"),_T("CH(SP)(GB2312)"),0,10008},
	{_T("MAC-"),_T("ROMANIA"),0,10010},
	{_T("MAC-"),_T("UA"),0,10017},
	{_T("MAC-"),_T("TH"),0,10021},
	{_T("MAC-"),_T("LAT2"),0,10029},
	{_T("MAC-"),_T("ICE"),0,10079},
	{_T("MAC-"),_T("TR"),0,10081},
	{_T("MAC-"),_T("CR"),0,10082},
};

int CPLENALL = (sizeof(CodePageNamesAll)/sizeof(CodePageNamesAll[0]));
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

	if((size<1) || (parser=pout=new char[size+1])==NULL)
		return -1;
	while((*pin!=0) && (pin-input< (INT_PTR)size))
	{
		if ((*pin>='a') && (*pin<='z'))
			*parser++=*(pin++)-('a'-'A'); // make it capital
		//else if(*pin=='\"') // this is already done in ExtractFromContentType
		//	*pin++; //skip the quotes if any
		else 
			*parser++=*pin++;
	}

	*parser=(char)0;

#ifdef DEBUG_DECODECODEPAGE
	DebugLog(DecodeFile,"<CodePage>%s</CodePage>",pout);
#endif
	for(int i=0;i<CPLENALL;i++){
		size_t len = strlen(CodePageNamesAll[i].NameBase);
		if(0==strncmp(pout,CodePageNamesAll[i].NameBase,len)){
			if (0==strcmp(pout+len,CodePageNamesAll[i].NameSub)){
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
	if(HexValue>='0' && HexValue<='9')
	{
		*DecValue=HexValue-'0';
		return 1;
	}
	if(HexValue>='A' && HexValue<='F')
	{
		*DecValue=HexValue-'A'+10;
		return 1;
	}
	if(HexValue>='a' && HexValue<='f')
	{
		*DecValue=HexValue-'a'+10;
		return 1;
	}
	return 0;
}

int FromBase64(char Base64Value,char *DecValue)
{
	if(Base64Value>='A' && Base64Value<='Z')
	{
		*DecValue=Base64Value-'A';
		return 1;
	}
	if(Base64Value>='a' && Base64Value<='z')
	{
		*DecValue=Base64Value-'a'+26;
		return 1;
	}
	if(Base64Value>='0' && Base64Value<='9')
	{
		*DecValue=Base64Value-'0'+52;
		return 1;
	}
	if(Base64Value=='+')
	{
		*DecValue=Base64Value-'+'+62;
		return 1;
	}
	if(Base64Value=='/')
	{
		*DecValue=Base64Value-'/'+63;
		return 1;
	}
	if(Base64Value=='=')
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
	for(int Counter=0;((char)*Src!=0) && DstLen && (Counter++<DstLen);Src++,Dst++)
		if(*Src=='=')
		{
			if (!isQ){
				if (Src[1]==0x0D){
					Src++; Src++;
					if (Src[0]==0x0A) Src++;
					goto CopyCharQuotedPrintable;
				}
				if (Src[1]==0x0A){
					Src++; Src++;
					goto CopyCharQuotedPrintable;
				}
			}
			char First,Second;
			if(!FromHexa(*(++Src),&First))
			{
				*Dst++='=';Src--;
				continue;
			}
			if(!FromHexa(*(++Src),&Second))
			{
				*Dst++='=';Src--;Src--;
				continue;
			}
			*Dst=(char)(First)<<4;
			*Dst+=Second;
		}
		else if(isQ && *Src=='_')
			*Dst=' ';
		else
CopyCharQuotedPrintable: // Yeah. Bad programming stile.
			*Dst=*Src;
	*Dst=(char)0;
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
	while(*Src!=0 && DstLen && Dst!=End)
	{
		if ((*Src==0x0D)||(*Src==0x0A))	{
			Src++; 
			continue;
		}
		if((!(Result=FromBase64(*Src,MiniResult+Locator)) && (*Src==0)) || Locator++==3)	//end_of_str || end_of_4_bytes
		{
			Locator=0;									//next write to the first byte
			*Dst++=(char)((MiniResult[0]<<2) | (MiniResult[1]>>4));
			if(Dst==End) goto end;								//DstLen exceeded?
			*Dst++=(char)((MiniResult[1]<<4) | (MiniResult[2]>>2));
			if(Dst==End) goto end;								//someones don't like goto, but not me
			*Dst++=(char)((MiniResult[2]<<6) | MiniResult[3]);
			if(!Result && (*Src==0)) goto end;						//end of string?
			MiniResult[0]=MiniResult[1]=MiniResult[2]=MiniResult[3]=0;			//zero 4byte buffer for next loop
		}
		if(!Result) return 0;									//unrecognised character occured
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

	if((cp!=CP_ACP) && (cp!=CP_OEMCP) && (cp!=CP_MACCP) && (cp!=CP_THREAD_ACP) && (cp!=CP_SYMBOL) && (cp!=CP_UTF7) && (cp!=CP_UTF8) && !GetCPInfo(cp,&CPInfo))
		cp=CP_ACP;
#ifdef DEBUG_DECODECODEPAGE
	DebugLog(DecodeFile,"<CodePage #>%d</CodePage #>",cp);
#endif
		
	for(Index=0;Index<sizeof(CodePagesZeroFlags)/sizeof(CodePagesZeroFlags[0]);Index++)
		if(CodePagesZeroFlags[Index]==cp)
		{
			Index=-1;
			break;
		}
	if(Index==-1)
		streamlen=MultiByteToWideChar(cp,0,stream,-1,NULL,0);
	else
		streamlen=MultiByteToWideChar(cp,MB_USEGLYPHCHARS,stream,-1,NULL,0);

	if(*out!=NULL)
		outlen=wcslen(*out);
	else
		outlen=0;
	temp=new WCHAR[streamlen+outlen+1];

	if(*out!=NULL)
	{
		for(dest=temp;*src!=(WCHAR)0;src++,dest++)				//copy old string from *out to temp
			*dest=*src;
//		*dest++=L' ';								//add space?
		delete[] *out;
	}
	else
		dest=temp;
	*out=temp;
	
	if(Index==-1)
	{
		if(!MultiByteToWideChar(cp,0,stream,-1,dest,streamlen))
			return 0;
	}
	else
	{
		if(!MultiByteToWideChar(cp,MB_USEGLYPHCHARS,stream,-1,dest,streamlen))
			return 0;
	}
	return 1;
}

void ConvertCodedStringToUnicode(char *stream,WCHAR **storeto,DWORD cp,int mode)
{
	char *start=stream,*finder,*finderend;
	char Encoding=0;
	char *DecodedResult=NULL;

	if(stream==NULL)
		return;

	while(WS(start)) start++;
	WCHAR *tempstore=0;
	if(!ConvertStringToUnicode(stream,cp,&tempstore))return;

	size_t tempstoreLength = wcslen(tempstore);
	
	size_t outind = 0;
	while(*start!=0){
		if(CODES(start)){
			finder=start+2;finderend=finder;
			while(!CODED(finderend) && !EOS(finderend)) finderend++;
			start = finderend;
			if(CODED(finderend))
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
				if(-1==(cp=(DWORD)GetCharsetFromString(finder,finderend-finder)))
					cp=CP_ACP;
				if(Encoding!=0)
				{
					int size,codeend;
					char *pcodeend;

					finder=finderend+2;
					if(CODED(finder))
						finder++;
					while(WS(finder)) finder++;
					finderend=finder;
					while(!CODEE(finderend) && !EOS(finderend)) finderend++;
					if(codeend=CODEE(finderend))
						pcodeend=finderend;
					while(WS(finderend-1)) finderend--;
						if((mode==MIME_MAIL) && (((*finder=='"') && (*(finderend-1)=='"'))))
					{
						finder++;
						finderend--;
					}
					//*finderend=(char)0;
					char * oneWordEncoded = new char[finderend-finder+1];
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
					if(DecodedResult!=NULL)
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
					if(codeend)
						finderend=pcodeend+2;
					if(WS(finderend))	//if string continues and there's some whitespace, add space to string that is to be converted
					{
						size_t len=strlen(DecodedResult);
						DecodedResult[len]=' ';
						DecodedResult[len+1]=0;
						finderend++;
					}
					WCHAR *oneWord=0;
					if(ConvertStringToUnicode(DecodedResult,cp,&oneWord)){
						size_t len = wcslen(oneWord);
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
