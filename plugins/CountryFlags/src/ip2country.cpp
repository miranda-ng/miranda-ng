/*
Miranda IM Country Flags Plugin
Copyright (C) 2006-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (Flags-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "flags.h"

extern HINSTANCE hInst;
extern int nCountriesCount;
extern struct CountryListEntry *countries;
static HANDLE hServiceIpToCountry;

/************************* Bin Records ****************************/

#define DATARECORD_SIZE (sizeof(DWORD)+sizeof(DWORD)+sizeof(WORD))

// mir_free() the return value
static BYTE* GetDataHeader(BYTE *data,DWORD cbDataSize,DWORD *pnDataRecordCount)
{
	BYTE *recordData;
	/* uncompressed size stored in first DWORD */
	*pnDataRecordCount=(*(DWORD*)data)/DATARECORD_SIZE;
	recordData=(BYTE*)mir_alloc(*(DWORD*)data);
	if (recordData != NULL)
		Huffman_Uncompress(data+sizeof(DWORD),recordData,cbDataSize-sizeof(DWORD),*(DWORD*)data);
	return recordData;
}

static int GetDataRecord(BYTE *data,DWORD index,DWORD *pdwFrom,DWORD *pdwTo)
{
	data+=index*DATARECORD_SIZE;
	*pdwFrom=*(DWORD*)data;
	data+=sizeof(DWORD);
	if (pdwTo != NULL) *pdwTo=*(DWORD*)data;
	data+=sizeof(DWORD);
	return (int)*(WORD*)data;
}

/************************* Record Cache ***************************/

static CRITICAL_SECTION csRecordCache;
static DWORD nDataRecordsCount; /* protected by csRecordCache */
static BYTE *dataRecords;       /* protected by csRecordCache */

#define UNLOADDELAY  30*1000  /* time after which the data records are being unloaded */

static void CALLBACK UnloadRecordCache(LPARAM lParam)
{
	EnterCriticalSection(&csRecordCache);
	mir_free(dataRecords);
	dataRecords=NULL;
	LeaveCriticalSection(&csRecordCache);
}

// function assumes it has got the csRecordCache mutex
static BOOL EnsureRecordCacheLoaded(BYTE **pdata,DWORD *pcount)
{
	HRSRC hrsrc;
	DWORD cb;
	EnterCriticalSection(&csRecordCache);
	if (dataRecords == NULL) {
		/* load record data list from resources */
		hrsrc=FindResource(hInst,MAKEINTRESOURCE(IDR_IPTOCOUNTRY),_T("BIN"));
		cb=SizeofResource(hInst,hrsrc);
		dataRecords=(BYTE*)LockResource(LoadResource(hInst,hrsrc));
		if (cb<=sizeof(DWORD) || dataRecords == NULL) { LeaveCriticalSection(&csRecordCache); return FALSE; }
		/* uncompress record data */
		dataRecords=GetDataHeader(dataRecords,cb,&nDataRecordsCount);
		if (dataRecords == NULL || !nDataRecordsCount) { LeaveCriticalSection(&csRecordCache); return FALSE; }
	}
	*pdata=dataRecords;
	*pcount=nDataRecordsCount;
	return TRUE;
}

static void LeaveRecordCache(void)
{
	/* mark for unload */
	CallFunctionBuffered(UnloadRecordCache,0,FALSE,UNLOADDELAY);
	LeaveCriticalSection(&csRecordCache);
}

/************************* Services *******************************/


INT_PTR ServiceIpToCountry(WPARAM wParam,LPARAM lParam)
{
	BYTE *data;
	DWORD dwFrom,dwTo;
	DWORD low=0,i,high;
	int id;
	if (EnsureRecordCacheLoaded(&data,&high)) {
		/* binary search in record data */
		GetDataRecord(data,low,&dwFrom,NULL);
		--high;
		if (wParam>=dwFrom) /* only search if wParam valid */
			while (low<=high) {
				i=low+((high-low)/2);
				/* analyze record */ 
				id=GetDataRecord(data,i,&dwFrom,&dwTo);
				if (dwFrom<=wParam && dwTo>=wParam) { LeaveRecordCache(); return id; }
				if (wParam>dwTo) low=i+1;      
				else high=i-1;
			}
		LeaveRecordCache();
	}
	return 0xFFFF; /* Unknown */
}

/************************* Bin Converter **************************/

#ifdef BINCONV
#include <stdio.h>
#include <stdlib.h>

struct {
	const char *szMir;
	const char *szCSV;
} static const differentCountryNames[]={
	{"British Virgin Islands","VIRGIN ISLANDS, BRITISH"},
	{"Brunei","BRUNEI DARUSSALAM"},
	{"Cape Verde Islands","CAPE VERDE"},
	{"Cocos-Keeling Islands","COCOS (KEELING) ISLANDS"},
	{"Chile, Republic of","CHILE"},
	{"Congo, Democratic Republic of (Zaire)","THE DEMOCRATIC REPUBLIC OF THE CONGO"},
	{"Congo, Republic of the","CONGO"},
	{"Cote d'Ivoire (Ivory Coast)","COTE D'IVOIRE"},
	{"Diego Garcia","BRITISH INDIAN OCEAN TERRITORY"},
	{"Guam, US Territory of","GUAM"},
	{"Iran (Islamic Republic of)","ISLAMIC REPUBLIC OF IRAN"},
	{"Korea, North","REPUBLIC OF KOREA"},
	{"Laos","LAO PEOPLE'S DEMOCRATIC REPUBLIC"},
	{"Reunion Island","REUNION"},
	{"Russia","RUSSIAN FEDERATION"},
	{"Saipan Island (Northern Mariana Islands)","NORTHERN MARIANA ISLANDS"},
	{"Tanzania","UNITED REPUBLIC OF TANZANIA"},
	{"USA","UNITED STATES"},
	{"Macau","MACAO"},
	{"Macedonia (F.Y.R.O.M.)","THE FORMER YUGOSLAV REPUBLIC OF MACEDONIA"},
	{"Micronesia, Federated States of","FEDERATED STATES OF MICRONESIA"},
	{"Mayotte Island","MAYOTTE"},
	{"Moldova, Republic of","REPUBLIC OF MOLDOVA"},
	{"Vietnam","VIET NAM"},
	{"Virgin Islands (USA)","VIRGIN ISLANDS, U.S."},
	{"Vatican City","HOLY SEE (VATICAN CITY STATE)"},
	{"Serbia, Republic of","SERBIA"},
	{"Montenegro, Republic of","MONTENEGRO"},
};

#define ALLOC_STEP (800*1024)  /* approx. size of data output */

struct ResizableByteBuffer {
	BYTE *buf;
	DWORD cbLength,cbAlloced;
};

static void AppendToByteBuffer(struct ResizableByteBuffer *buffer,const void *append,DWORD cbAppendSize)
{
	if (buffer->cbAlloced<=buffer->cbLength+cbAppendSize) {
		BYTE* buf=(BYTE*)mir_realloc(buffer->buf,buffer->cbAlloced+ALLOC_STEP+cbAppendSize);
		if (buf == NULL) return;
		buffer->buf=buf;
		buffer->cbAlloced+=ALLOC_STEP+cbAppendSize;
		OutputDebugStringA("reallocating memory...\n"); /* all ascii */
	}
	CopyMemory(&buffer->buf[buffer->cbLength],append,cbAppendSize);
	buffer->cbLength+=cbAppendSize;
}

static int EnumIpDataLines(const char *pszFileCSV,const char *pszFileOut)
{
	FILE *fp;
	char line[1024],out[512],*pszFrom,*pszTo,*pszTwo,*pszCountry,*buf;
	int i,j;
	DWORD dwOut;
	WORD wOut;
	struct ResizableByteBuffer buffer;

	memset(&buffer, 0, sizeof(buffer));
	fp=fopen(pszFileCSV,"rt");
	if (fp != NULL) {
		OutputDebugStringA("Running IP data convert...\n"); /* all ascii */
		while (!feof(fp)) {
			if (fgets(line,sizeof(line),fp) == NULL) break;
			/* get line data */
			pszFrom=line+1;
			pszTo=strchr(pszFrom,',');
			*(pszTo-1)='\0'; pszTo+=2;
			pszTwo=strchr(pszTo,',');
			*(pszTwo-1)='\0'; pszTwo+=2;
			pszCountry=strchr(pszTwo,',')+1;
			pszCountry=strchr(pszCountry,',')+2;
			buf=strchr(pszCountry,'"');
			*buf=pszTwo[2]='\0';
			/* corrections */
			if (!mir_tstrcmpi(pszCountry,"ANTARCTICA")) continue;
			if (!mir_tstrcmpi(pszCountry,"TIMOR-LESTE")) continue;
			if (!mir_tstrcmpi(pszCountry,"PALESTINIAN TERRITORY, OCCUPIED"))
				mir_tstrcpy(pszCountry,"ISRAEL");
			else if (!mir_tstrcmpi(pszCountry,"UNITED STATES MINOR OUTLYING ISLANDS"))
				mir_tstrcpy(pszCountry,"UNITED STATES");
			else if (!mir_tstrcmpi(pszCountry,"SOUTH GEORGIA AND THE SOUTH SANDWICH ISLANDS"))
				mir_tstrcpy(pszCountry,"UNITED KINGDOM");
			else if (!mir_tstrcmpi(pszTwo,"JE")) /* map error */
				mir_tstrcpy(pszCountry,"UNITED KINGDOM");
			else if (!mir_tstrcmpi(pszTwo,"AX")) /* Åland Island belongs to Finland */
				mir_tstrcpy(pszCountry,"FINLAND");
			else if (!mir_tstrcmpi(pszTwo,"ME"))
				mir_tstrcpy(pszCountry,"MONTENEGRO");
			else if (!mir_tstrcmpi(pszTwo,"RS") || !mir_tstrcmpi(pszTwo,"CS"))
				mir_tstrcpy(pszCountry,"SERBIA");
			/* convert */
			for(i=0;i<nCountriesCount;i++) {
				/* map different writings */
				for(j=0;j<SIZEOF(differentCountryNames);j++)
					if (!mir_tstrcmpi(countries[i].szName,differentCountryNames[j].szMir)) {
						buf=(char*)differentCountryNames[j].szCSV;
						break;
					}
				if (j == SIZEOF(differentCountryNames))
					buf=(char*)countries[i].szName;
				/* check country */
				if (!mir_strcmpi(pszCountry,buf)) {
					dwOut=(DWORD)atoi(pszFrom);
					AppendToByteBuffer(&buffer,(void*)&dwOut,sizeof(DWORD));
					dwOut=(DWORD)atoi(pszTo);
					AppendToByteBuffer(&buffer,(void*)&dwOut,sizeof(DWORD));
					wOut=(WORD)countries[i].id;
					AppendToByteBuffer(&buffer,(void*)&wOut,sizeof(WORD));
					break;
				}
			}
			/* not in list */
			if (i == nCountriesCount) {
				mir_snprintf(out, SIZEOF(out), "Unknown: %s-%s [%s, %s]\n", pszFrom, pszTo, pszTwo, pszCountry);
				OutputDebugStringA(out); /* all ascii */
			}
		}
		fclose(fp);
		OutputDebugStringA("Done!\n"); /* all ascii */
		if (buffer.buf != NULL) {
			HANDLE hFileOut;
			DWORD cbWritten=0;
			BYTE *compressed;
			DWORD cbCompressed;
			/* compress whole data */
			OutputDebugStringA("Compressing...\n"); /* all ascii */
			compressed=(BYTE*)mir_alloc(buffer.cbAlloced+384);
			if (compressed != NULL) {
				cbCompressed=Huffman_Compress(buffer.buf,compressed,buffer.cbLength);
				OutputDebugStringA("Done!\n"); /* all ascii */
				OutputDebugStringA("Writing to file...\n"); /* all ascii */
				hFileOut=CreateFile(pszFileOut,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
				if (hFileOut != INVALID_HANDLE_VALUE) {
					/* store data length count at beginning */
					dwOut=buffer.cbLength;
					WriteFile(hFileOut,&dwOut,sizeof(DWORD),&cbWritten,NULL);
					/* store compressed data records */
					WriteFile(hFileOut,compressed,cbCompressed,&cbWritten,NULL);
					CloseHandle(hFileOut);
				}
				OutputDebugStringA("Done!\n"); /* all ascii */
				mir_free(compressed);
			}
			mir_free(buffer.buf);
		}
		return 0;
	}
	return 1;
}

static void BinConvThread(void *unused)
{
	/* debug version only */
	if (MessageBox(NULL,_T("Looking for 'ip-to-country.csv' in current directory.\n"
		"It will be converted into 'ip-to-country.bin'.\n"
		"See debug output for more details.\n"
		"This process may take very long."),_T("Bin Converter"),MB_OKCANCEL|MB_ICONINFORMATION|MB_SETFOREGROUND|MB_TOPMOST|MB_TASKMODAL) == IDOK) {
		EnumIpDataLines("ip-to-country.csv","ip-to-country.bin");
		MessageBox(NULL,_T("Done!\n'ip-to-country.bin' has been created in current directory."),_T("Bin Converter"),MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND|MB_TOPMOST|MB_TASKMODAL);
	}
}

#endif // BINCONV 

/************************* Misc ***********************************/

void InitIpToCountry(void)
{
	/* Record Cache */
	InitializeCriticalSection(&csRecordCache);
	nDataRecordsCount=0;
	dataRecords=NULL;
	/* Services */
	hServiceIpToCountry=CreateServiceFunction(MS_FLAGS_IPTOCOUNTRY,ServiceIpToCountry);
#ifdef BINCONV
	mir_forkthread(BinConvThread,NULL);
#endif
}

void UninitIpToCountry(void)
{
	/* Record Cache */
	DeleteCriticalSection(&csRecordCache);
	mir_free(dataRecords); /* does NULL check */
	/* Servcies */
	DestroyServiceFunction(hServiceIpToCountry);
}
