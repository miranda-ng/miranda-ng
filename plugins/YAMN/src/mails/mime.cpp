/*
 * This code implements retrieving info from MIME header
 *
 * (c) majvan 2002-2004
 */

#include "../yamn.h"

//--------------------------------------------------------------------------------------------------

//Copies one string to another
// srcstart- source string
// srcend- address to the end of source string
// dest- pointer that stores new allocated string that contains copy of source string
// mode- MIME_PLAIN or MIME_MAIL (MIME_MAIL deletes '"' characters (or '<' and '>') if they are at start and end of source string
void CopyToHeader(char *srcstart,char *srcend,char **dest,int mode);

//Extracts email address (finds nick name and mail and then stores them to strings)
// finder- source string
// storeto- pointer that receives address of mail string
// storetonick- pointer that receives address of nickname
void ExtractAddressFromLine(char *finder,char **storeto,char **storetonick);

//Extracts simple text from string
// finder- source string
// storeto- pointer that receives address of string
void ExtractStringFromLine(char *finder,char **storeto);

//Extracts some item from content-type string
//Example: ContentType string: "TEXT/PLAIN; charset=US-ASCII", item:"charset=", returns: "US-ASCII"
// ContetType- content-type string
// value- string item
// returns extracted string (or NULL when not found)
char *ExtractFromContentType(char *ContentType,char *value);

//Extracts info from header text into header members
//Note that this function as well as struct CShortHeadwer can be always changed, because there are many items to extract
//(e.g. the X-Priority and Importance and so on)
// items- translated header (see TranslateHeaderFcn)
// head- header to be filled with values extracted from items
void ExtractShortHeader(struct CMimeItem *items,struct CShortHeader *head);

//Extracts header to mail using ExtractShortHeader fcn.
// items- translated header (see TranslateHeaderFcn)
// CP- codepage used when no default found
// head- header to be filled with values extracted from items, in unicode (wide char)
void ExtractHeader(struct CMimeItem *items,int &CP,struct CHeader *head);

//Deletes items in CShortHeader structure
// head- structure whose items are deleted
void DeleteShortHeaderContent(struct CShortHeader *head);

//Deletes list of YAMN_MIMENAMES structures
// Names- pointer to first item of list
void DeleteNames(PYAMN_MIMENAMES Names);

//Deletes list of YAMN_MIMESHORTNAMES structures
// Names- pointer to first item of list
void DeleteShortNames(PYAMN_MIMESHORTNAMES Names);

//Makes a string lowercase
// string- string to be lowercased
void inline ToLower(char *string);

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

void CopyToHeader(char *srcstart,char *srcend,char **dest,int mode)
{
	char *dst;

	if (dest==NULL)
		return;
	if (srcstart>=srcend)
		return;

	if ((mode==MIME_MAIL) && (((*srcstart=='"') && (*(srcend-1)=='"')) || ((*srcstart=='<') && (*(srcend-1)=='>'))))
	{
		srcstart++;
		srcend--;
	}

	if (srcstart>=srcend)
		return;

	if (NULL != *dest)
		delete[] *dest;
	if (NULL==(*dest=new char[srcend-srcstart+1]))
		return;

	dst=*dest;

	for (;srcstart<srcend;dst++,srcstart++)
	{
		if (ENDLINE(srcstart))
		{
			while(ENDLINE(srcstart) || WS(srcstart)) srcstart++;
			*dst=' ';
			srcstart--;		//because at the end of "for loop" we increment srcstart
		}
		else
			*dst=*srcstart;
	}
	*dst=0;
}

void ExtractAddressFromLine(char *finder,char **storeto,char **storetonick)
{
	if (finder==NULL)
	{
		*storeto=*storetonick=NULL;
		return;
	}
	while(WS(finder)) finder++;
	if ((*finder) != '<')
	{
		char *finderend=finder+1;
		do
		{
			if (ENDLINEWS(finderend))						//after endline information continues
				finderend+=2;
			while(!ENDLINE(finderend) && !EOS(finderend)) finderend++;		//seek to the end of line or to the end of string
		}while(ENDLINEWS(finderend));
		finderend--;
		while(WS(finderend) || ENDLINE(finderend)) finderend--;				//find the end of text, no whitespace
		if (*finderend != '>')						//not '>' at the end of line
			CopyToHeader(finder,finderend+1,storeto,MIME_MAIL);
		else								//at the end of line, there's '>'
		{
			char *finder2=finderend;
			while((*finder2 != '<') && (finder2>finder)) finder2--;		//go to matching '<' or to the start
			CopyToHeader(finder2,finderend+1,storeto,MIME_MAIL);
			if (*finder2=='<')						//if we found '<', the rest copy as from nick
			{
				finder2--;
				while(WS(finder2) || ENDLINE(finder2)) finder2--;		//parse whitespace
				CopyToHeader(finder,finder2+1,storetonick,MIME_MAIL);		//and store nickname
			}
		}
	}
	else
	{
		char *finderend=finder+1;
		do
		{
			if (ENDLINEWS(finderend))							//after endline information continues
				finderend+=2;
			while(!ENDLINE(finderend) && (*finderend != '>') && !EOS(finderend)) finderend++;		//seek to the matching < or to the end of line or to the end of string
		}while(ENDLINEWS(finderend));
		CopyToHeader(finder,finderend+1,storeto,MIME_MAIL);				//go to first '>' or to the end and copy
		finder=finderend+1;
		while(WS(finder)) finder++;								//parse whitespace
		if (!ENDLINE(finder) && !EOS(finder))					//if there are chars yet, it's nick
		{
			finderend=finder+1;
			while(!ENDLINE(finderend) && !EOS(finderend)) finderend++;	//seek to the end of line or to the end of string
			finderend--;
			while(WS(finderend)) finderend--;				//find the end of line, no whitespace
			CopyToHeader(finder,finderend+1,storetonick,MIME_MAIL);
		}
	}
}

void ExtractStringFromLine(char *finder,char **storeto)
{
	if (finder==NULL)
	{
		*storeto=NULL;
		return;
	}
	while(WS(finder)) finder++;
	char *finderend=finder;

	do
	{
		if (ENDLINEWS(finderend)) finderend++;						//after endline information continues
		while(!ENDLINE(finderend) && !EOS(finderend)) finderend++;
	}while(ENDLINEWS(finderend));
	finderend--;
	while(WS(finderend)) finderend--;				//find the end of line, no whitespace
	CopyToHeader(finder,finderend+1,storeto,MIME_PLAIN);
}

char *ExtractFromContentType(char *ContentType,char *value)
{
	char *lowered = _strdup(ContentType);
	ToLower(lowered);
	char *finder=strstr(lowered,value);
	if (finder==NULL) {
		free (lowered);
		return NULL;
	}
	finder = finder-lowered+ContentType;
	free (lowered);

	char *temp,*copier;
	char *CopiedString;

	temp=finder-1;
	while((temp>ContentType) && WS(temp)) temp--;			//now we have to find, if the word "Charset=" is located after ';' like "; Charset="
	if (*temp != ';' && !ENDLINE(temp) && temp != ContentType)
		return NULL;
	finder=finder+mir_strlen(value);						//jump over value string

	while(WS(finder)) finder++;					//jump over whitespaces
	temp=finder;
	while(*temp != 0 && *temp != ';') temp++;				//jump to the end of setting (to the next ;)
	temp--;
	while(WS(temp))	temp--;						//remove whitespaces from the end
	if (*finder=='\"') { //remove heading and tailing quotes
		finder++;
		if (*temp=='\"') temp--;
	}
	if (NULL==(CopiedString=new char[++temp-finder+1]))
		return NULL;
	for (copier=CopiedString;finder != temp;*copier++=*finder++);			//copy string
	*copier=0;						//and end it with zero character

	return CopiedString;
}

void ExtractShortHeader(struct CMimeItem *items,struct CShortHeader *head)
{
	for (;items != NULL;items=items->Next)
	{
		//at the start of line
		//MessageBox(NULL,items->value,items->name,0);
		if (0==_strnicmp(items->name,"From",4))
		{
			if (items->value==NULL)
				continue;
			#ifdef DEBUG_DECODE
			DebugLog(DecodeFile,"<Extracting from>");
			#endif
			ExtractAddressFromLine(items->value,&head->From,&head->FromNick);
			#ifdef DEBUG_DECODE
			DebugLog(DecodeFile,"</Extracting>\n");
			#endif
		}
		else if (0==_strnicmp(items->name,"Return-Path",11))
		{
			if (items->value==NULL)
				continue;
			#ifdef DEBUG_DECODE
			DebugLog(DecodeFile,"<Extracting return-path>");
			#endif
			ExtractAddressFromLine(items->value,&head->ReturnPath,&head->ReturnPathNick);
			#ifdef DEBUG_DECODE
			DebugLog(DecodeFile,"</Extracting>\n");
			#endif
		}
		else if (0==_strnicmp(items->name,"Subject",7))
		{
			if (items->value==NULL)
				continue;
			#ifdef DEBUG_DECODE
			DebugLog(DecodeFile,"<Extracting subject>");
			#endif
			ExtractStringFromLine(items->value,&head->Subject);
			#ifdef DEBUG_DECODE
			DebugLog(DecodeFile,"</Extracting>\n");
			#endif
		}
		else if (0==_strnicmp(items->name,"Body",4))
		{
			if (items->value==NULL)
				continue;
			#ifdef DEBUG_DECODE
			DebugLog(DecodeFile,"<Extracting body>");
			#endif
			ExtractStringFromLine(items->value,&head->Body);
			#ifdef DEBUG_DECODE
			DebugLog(DecodeFile,"</Extracting>\n");
			#endif
		}
		else if (0==_strnicmp(items->name,"Date",4))
		{
			if (items->value==NULL)
				continue;
			#ifdef DEBUG_DECODE
			DebugLog(DecodeFile,"<Extracting date>");
			#endif
			ExtractStringFromLine(items->value,&head->Date);
			#ifdef DEBUG_DECODE
			DebugLog(DecodeFile,"</Extracting>\n");
			#endif
		}
		else if (0==_strnicmp(items->name,"Content-Type",12))
		{
			if (items->value==NULL)
				continue;

			char *ContentType=NULL,*CharSetStr;
			#ifdef DEBUG_DECODE
			DebugLog(DecodeFile,"<Extracting Content-Type>");
			#endif
			ExtractStringFromLine(items->value,&ContentType);
			#ifdef DEBUG_DECODE
			DebugLog(DecodeFile,"</Extracting>\n");
			#endif
			ToLower(ContentType);
			if (NULL != (CharSetStr=ExtractFromContentType(ContentType,"charset=")))
			{
				head->CP=GetCharsetFromString(CharSetStr,mir_strlen(CharSetStr));
				delete[] CharSetStr;
			}
			delete[] ContentType;
		}
		else if (0==_strnicmp(items->name,"Importance",10))
		{
			if (items->value==NULL)
				continue;
			#ifdef DEBUG_DECODE
			DebugLog(DecodeFile,"<Extracting importance>");
			#endif
			if (head->Priority != -1)
			{
				if (0==strncmp(items->value,"low",3))
					head->Priority=5;
				else if (0==strncmp(items->value,"normal",6))
					head->Priority=3;
				else if (0==strncmp(items->value,"high",4))
					head->Priority=1;
			}
			#ifdef DEBUG_DECODE
			DebugLog(DecodeFile,"</Extracting>\n");
			#endif
		}
		else if (0==_strnicmp(items->name,"X-Priority",10))
		{
			if (items->value==NULL)
				continue;
			#ifdef DEBUG_DECODE
			DebugLog(DecodeFile,"<X-Priority>");
			#endif
			if ((*items->value>='1') && (*items->value<='5'))
				head->Priority=*items->value-'0';
			#ifdef DEBUG_DECODE
			DebugLog(DecodeFile,"</Extracting>\n");
			#endif
		}

	}
}

void ExtractHeader(struct CMimeItem *items,int &CP,struct CHeader *head)
{
	struct CShortHeader ShortHeader;

	memset(&ShortHeader, 0, sizeof(struct CShortHeader));
	ShortHeader.Priority=ShortHeader.CP=-1;
	#ifdef DEBUG_DECODE
	DebugLog(DecodeFile,"<Extracting header>\n");
	#endif
	ExtractShortHeader(items,&ShortHeader);

	head->Priority=ShortHeader.Priority==-1 ? 3 : ShortHeader.Priority;
	CP=ShortHeader.CP==-1 ? CP : ShortHeader.CP;
	#ifdef DEBUG_DECODE
	if (NULL != ShortHeader.From)
		DebugLog(DecodeFile,"<Decoded from>%s</Decoded)\n",ShortHeader.From);
	if (NULL != ShortHeader.FromNick)
		DebugLog(DecodeFile,"<Decoded from-nick>%s</Decoded)\n",ShortHeader.FromNick);
	if (NULL != ShortHeader.ReturnPath)
		DebugLog(DecodeFile,"<Decoded return-path>%s</Decoded)\n",ShortHeader.ReturnPath);
	if (NULL != ShortHeader.ReturnPathNick)
		DebugLog(DecodeFile,"<Decoded return-path nick>%s</Decoded)\n",ShortHeader.ReturnPathNick);
	if (NULL != ShortHeader.Subject)
		DebugLog(DecodeFile,"<Decoded subject>%s</Decoded)\n",ShortHeader.Subject);
	if (NULL != ShortHeader.Date)
		DebugLog(DecodeFile,"<Decoded date>%s</Decoded)\n",ShortHeader.Date);
	DebugLog(DecodeFile,"</Extracting header>\n");
	DebugLog(DecodeFile,"<Convert>\n");
	#endif

	ConvertCodedStringToUnicode(ShortHeader.From,&head->From,CP,MIME_PLAIN);

	#ifdef DEBUG_DECODE
	if (NULL != head->From)
		DebugLogW(DecodeFile,L"<Converted from>%s</Converted>\n",head->From);
	#endif
	ConvertCodedStringToUnicode(ShortHeader.FromNick,&head->FromNick,CP,MIME_MAIL);
	#ifdef DEBUG_DECODE
	if (NULL != head->FromNick)
		DebugLogW(DecodeFile,L"<Converted from-nick>%s</Converted>\n",head->FromNick);
	#endif
	ConvertCodedStringToUnicode(ShortHeader.ReturnPath,&head->ReturnPath,CP,MIME_PLAIN);
	#ifdef DEBUG_DECODE
	if (NULL != head->ReturnPath)
		DebugLogW(DecodeFile,L"<Converted return-path>%s</Converted>\n",head->ReturnPath);
	#endif
	ConvertCodedStringToUnicode(ShortHeader.ReturnPathNick,&head->ReturnPathNick,CP,MIME_MAIL);
	#ifdef DEBUG_DECODE
	if (NULL != head->ReturnPathNick)
		DebugLogW(DecodeFile,L"<Converted return-path nick>%s</Converted>\n",head->ReturnPathNick);
	#endif
	ConvertCodedStringToUnicode(ShortHeader.Subject,&head->Subject,CP,MIME_PLAIN);
	#ifdef DEBUG_DECODE
	if (NULL != head->Subject)
		DebugLogW(DecodeFile,L"<Converted subject>%s</Converted>\n",head->Subject);
	#endif
	ConvertCodedStringToUnicode(ShortHeader.Date,&head->Date,CP,MIME_PLAIN);
	#ifdef DEBUG_DECODE
	if (NULL != head->Date)
		DebugLogW(DecodeFile,L"<Converted date>%s</Converted>\n",head->Date);
	#endif

	ConvertCodedStringToUnicode(ShortHeader.Body,&head->Body,CP,MIME_PLAIN);
	#ifdef DEBUG_DECODE
	if (NULL != head->Body)
		DebugLogW(DecodeFile,L"<Converted Body>%s</Converted>\n",head->Body);
	#endif

	#ifdef DEBUG_DECODE
	DebugLog(DecodeFile,"</Convert>\n");
	#endif

	DeleteShortHeaderContent(&ShortHeader);

//	head->From=L"Frommmm";
//	head->Subject=L"Subject";
	return;
}

void DeleteShortHeaderContent(struct CShortHeader *head)
{
	if (head->From != NULL) delete[] head->From;
	if (head->FromNick != NULL) delete[] head->FromNick;
	if (head->ReturnPath != NULL) delete[] head->ReturnPath;
	if (head->ReturnPathNick != NULL) delete[] head->ReturnPathNick;
	if (head->Subject != NULL) delete[] head->Subject;
	if (head->Date != NULL) delete[] head->Date;
	if (head->To != NULL) DeleteShortNames(head->To);
	if (head->Cc != NULL) DeleteShortNames(head->Cc);
	if (head->Bcc != NULL) DeleteShortNames(head->Bcc);
	if (head->Body != NULL) delete[] head->Body;
}

void DeleteHeaderContent(struct CHeader *head)
{
	if (head->From != NULL) delete[] head->From;
	if (head->FromNick != NULL) delete[] head->FromNick;
	if (head->ReturnPath != NULL) delete[] head->ReturnPath;
	if (head->ReturnPathNick != NULL) delete[] head->ReturnPathNick;
	if (head->Subject != NULL) delete[] head->Subject;
	if (head->Date != NULL) delete[] head->Date;
	if (head->Body != NULL) delete[] head->Body;
	if (head->To != NULL) DeleteNames(head->To);
	if (head->Cc != NULL) DeleteNames(head->Cc);
	if (head->Bcc != NULL) DeleteNames(head->Bcc);
}

void DeleteNames(PYAMN_MIMENAMES Names)
{
	PYAMN_MIMENAMES Parser=Names,Old;
	for (;Parser != NULL;Parser=Parser->Next)
	{
		if (Parser->Value != NULL)
			delete[] Parser->Value;
		if (Parser->ValueNick != NULL)
			delete[] Parser->ValueNick;
		Old=Parser;
		Parser=Parser->Next;
		delete Old;
	}
}

void DeleteShortNames(PYAMN_MIMESHORTNAMES Names)
{
	PYAMN_MIMESHORTNAMES Parser=Names,Old;
	for (;Parser != NULL;Parser=Parser->Next)
	{
		if (Parser->Value != NULL)
			delete[] Parser->Value;
		if (Parser->ValueNick != NULL)
			delete[] Parser->ValueNick;
		Old=Parser;
		Parser=Parser->Next;
		delete Old;
	}
}


void inline ToLower(char *string)
{
	for (;*string != 0;string++)
		if (*string>='A' && *string<='Z') *string=*string-'A'+'a';
}

#define TE_UNKNOWN
#define TE_QUOTEDPRINTABLE 1
#define TE_BASE64 2
struct APartDataType
{
	char *Src;//Input
	char *ContType;
	int CodePage;
	char *TransEnc;
	BYTE TransEncType; //TE_something
	char *body;
	int bodyLen;
	WCHAR *wBody;
};


void ParseAPart(APartDataType *data)
{
	size_t len = mir_strlen(data->Src);
	try
	{
		char *finder=data->Src;
		char *prev1,*prev2,*prev3;

		while(finder<=(data->Src+len))
		{
			while(ENDLINEWS(finder)) finder++;

			//at the start of line
			if (finder>data->Src) {
				if (*(finder-2)=='\r' || *(finder-2)=='\n') 
					*(finder-2)=0;
				if (*(finder-1)=='\r' || *(finder-1)=='\n') 
					*(finder-1)=0;
			}
			prev1=finder;

			while(*finder != ':' && !EOS(finder) && !ENDLINE(finder)) finder++;
			if (ENDLINE(finder)||EOS(finder)) {
				// no ":" in the line? here the body begins;
				data->body = prev1;
				break;
			}
			prev2=finder++;

			while(WS(finder) && !EOS(finder)) finder++;
			if (!EOS(finder))
				prev3=finder;
			else
				break;

			do
			{
				if (ENDLINEWS(finder)) finder+=2;						//after endline information continues
				while(!ENDLINE(finder) && !EOS(finder)) finder++;
			}while(ENDLINEWS(finder));

			if (!_strnicmp(prev1,"Content-type",prev2-prev1)) {
				data->ContType = prev3;
			} else if (!_strnicmp(prev1,"Content-Transfer-Encoding",prev2-prev1)) {
				data->TransEnc = prev3;
			}

			if (EOS(finder))
				break;
			finder++;
			if (ENDLINE(finder)) {
				finder++;
				if (ENDLINE(finder)) {
					// end of headers. message body begins
					if (finder>data->Src) {
						if (*(finder-2)=='\r' || *(finder-2)=='\n') 
							*(finder-2)=0;
						if (*(finder-1)=='\r' || *(finder-1)=='\n') 
							*(finder-1)=0;
					}
					finder++;
					if (ENDLINE(finder))finder++;
					prev1 = finder;
					while (!EOS(finder+1))finder++;
					if (ENDLINE(finder))finder--;
					prev2 = finder;
					if (prev2>prev1) { // yes, we have body
						data->body = prev1;
					}
					break; // there is nothing else
				}
			}
		}
	}
	catch(...)
	{
		MessageBox(NULL, TranslateT("Translate header error"), _T(""), 0);
	}
	if (data->body) data->bodyLen = (int)mir_strlen(data->body);
}

//from decode.cpp
int DecodeQuotedPrintable(char *Src,char *Dst,int DstLen, BOOL isQ);
int DecodeBase64(char *Src,char *Dst,int DstLen);
int ConvertStringToUnicode(char *stream,unsigned int cp,WCHAR **out);

WCHAR *ParseMultipartBody(char *src, char *bond)
{
	char *srcback = _strdup(src);
	size_t sizebond = mir_strlen(bond);
	int numparts = 1;
	int i;
	char *courbond = srcback;
	WCHAR *dest;
	for (;(courbond=strstr(courbond,bond));numparts++,courbond+=sizebond);
	APartDataType *partData = new APartDataType[numparts];
	memset(partData, 0, sizeof(APartDataType)*numparts);
	partData[0].Src = courbond = srcback;
	for (i=1;(courbond=strstr(courbond,bond));i++,courbond+=sizebond) {
		*(courbond-2) = 0;
		partData[i].Src = courbond+sizebond;
		while (ENDLINE(partData[i].Src)) partData[i].Src++;
	}
	size_t resultSize=0;
	for (i=0;i<numparts;i++) {
		ParseAPart(&partData[i]);
		if (partData[i].body) {
			if (partData[i].TransEnc) {
				if (!_stricmp(partData[i].TransEnc,"base64")) partData[i].TransEncType=TE_BASE64;
				else if (!_stricmp(partData[i].TransEnc,"quoted-printable"))partData[i].TransEncType=TE_QUOTEDPRINTABLE;
			}
			if (partData[i].ContType) {
				char *CharSetStr;
				if (NULL != (CharSetStr=ExtractFromContentType(partData[i].ContType,"charset=")))
				{
					partData[i].CodePage=GetCharsetFromString(CharSetStr,mir_strlen(CharSetStr));
					delete[] CharSetStr;
				}
			}
			if (partData[i].ContType && !_strnicmp(partData[i].ContType,"text",4)) {
				char *localBody=0;
				switch (partData[i].TransEncType) {
					case TE_BASE64:
					{
						int size =partData[i].bodyLen*3/4+5;
						localBody = new char[size+1];
						DecodeBase64(partData[i].body,localBody,size); 
					}break;
					case TE_QUOTEDPRINTABLE:
					{
						int size = partData[i].bodyLen+2;
						localBody = new char[size+1];
						DecodeQuotedPrintable(partData[i].body,localBody,size,FALSE); 
					}break;
				}
				ConvertStringToUnicode(localBody?localBody:partData[i].body,partData[i].CodePage,&partData[i].wBody);
				if (localBody) delete[] localBody;
			} else if (partData[i].ContType && !_strnicmp(partData[i].ContType,"multipart/",10)) {
				//Multipart in mulitipart recursive? should be SPAM. Ah well
				char *bondary=NULL;
				if (NULL != (bondary=ExtractFromContentType(partData[i].ContType,"boundary=")))
				{
					partData[i].wBody = ParseMultipartBody(partData[i].body,bondary);
					delete[] bondary;
				} else goto FailBackRaw; //multipart with no boundary? badly formatted messages.
			} else {
FailBackRaw:
				ConvertStringToUnicode(partData[i].body,partData[i].CodePage,&partData[i].wBody);
			}
			resultSize += mir_wstrlen(partData[i].wBody);
		}// if (partData[i].body)
		resultSize += 100+4+3; //cr+nl+100+ 3*bullet
	}
	dest = new WCHAR[resultSize+1];
	size_t destpos = 0;
	for (i=0;i<numparts;i++) {
		if (i) { // part before first boudary should not have headers
			char infoline[1024]; size_t linesize = 0;
			mir_snprintf(infoline, SIZEOF(infoline), "%s %d", Translate("Part"), i);
			linesize = mir_strlen(infoline);
			if (partData[i].TransEnc) {
				mir_snprintf(infoline + linesize, SIZEOF(infoline) - linesize, "; %s", partData[i].TransEnc);
				linesize = mir_strlen(infoline);
			}
			if (partData[i].ContType) {
				char *CharSetStr=strchr(partData[i].ContType,';');
				if (CharSetStr) {
					CharSetStr[0]=0;
					mir_snprintf(infoline + linesize, SIZEOF(infoline) - linesize, "; %s", partData[i].ContType);
					linesize = mir_strlen(infoline);
					partData[i].ContType=CharSetStr+1;
					if (NULL != (CharSetStr=ExtractFromContentType(partData[i].ContType,"charset="))) {
						mir_snprintf(infoline + linesize, SIZEOF(infoline) - linesize, "; %s", CharSetStr);
						linesize = mir_strlen(infoline);
						delete[] CharSetStr;
					}
					if (NULL != (CharSetStr=ExtractFromContentType(partData[i].ContType,"name="))) {
						mir_snprintf(infoline + linesize, SIZEOF(infoline) - linesize, "; \"%s\"", CharSetStr);
						linesize = mir_strlen(infoline);
						delete[] CharSetStr;
					}
				}
				else {
					mir_snprintf(infoline + linesize, SIZEOF(infoline) - linesize, "; %s", partData[i].ContType);
					linesize = mir_strlen(infoline);
				}
			}
			mir_snprintf(infoline + linesize, SIZEOF(infoline) - linesize, ".\r\n");
			{
				WCHAR *temp=0;
				dest[destpos] = dest[destpos+1] = dest[destpos+2] = 0x2022; // bullet;
				destpos += 3;
				ConvertStringToUnicode(infoline,CP_ACP,&temp);
				size_t wsize = mir_wstrlen(temp);
				mir_wstrcpy(&dest[destpos],temp);
				destpos += wsize;
				delete[] temp;
			}
		} // if (i)

		if (partData[i].wBody) {
			size_t wsize = mir_wstrlen(partData[i].wBody);
			mir_wstrcpy(&dest[destpos],partData[i].wBody);
			destpos += wsize;
			delete[] partData[i].wBody;
		}
	}

	free (srcback);
	delete[] partData;
	dest[resultSize] = 0;//just in case
	return dest;
}
