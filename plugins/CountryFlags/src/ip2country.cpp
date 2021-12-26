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

#include "stdafx.h"

extern int nCountriesCount;
extern struct CountryListEntry *countries;

/************************* Bin Records ****************************/

#define DATARECORD_SIZE (sizeof(uint32_t)+sizeof(uint32_t)+sizeof(uint16_t))

// mir_free() the return value
static uint8_t* GetDataHeader(uint8_t *data, uint32_t cbDataSize, uint32_t *pnDataRecordCount)
{
	uint8_t *recordData;
	/* uncompressed size stored in first uint32_t */
	*pnDataRecordCount = (*(uint32_t*)data) / DATARECORD_SIZE;
	recordData = (uint8_t*)mir_alloc(*(uint32_t*)data);
	if (recordData != nullptr)
		Huffman_Uncompress(data + sizeof(uint32_t), recordData, cbDataSize - sizeof(uint32_t), *(uint32_t*)data);
	return recordData;
}

static int GetDataRecord(uint8_t *data, uint32_t index, uint32_t *pdwFrom, uint32_t *pdwTo)
{
	data += index * DATARECORD_SIZE;
	*pdwFrom = *(uint32_t*)data;
	data += sizeof(uint32_t);
	if (pdwTo != nullptr) *pdwTo = *(uint32_t*)data;
	data += sizeof(uint32_t);
	return (int)*(uint16_t*)data;
}

/************************* Record Cache ***************************/

static mir_cs csRecordCache;
static uint32_t nDataRecordsCount; /* protected by csRecordCache */
static uint8_t *dataRecords;       /* protected by csRecordCache */

#define UNLOADDELAY  30*1000  /* time after which the data records are being unloaded */

static void CALLBACK UnloadRecordCache(LPARAM)
{
	mir_cslock lck(csRecordCache);
	mir_free(dataRecords);
	dataRecords = nullptr;
}

// function assumes it has got the csRecordCache mutex
static BOOL EnsureRecordCacheLoaded(uint8_t **pdata, uint32_t *pcount)
{
	HRSRC hrsrc;
	uint32_t cb;
	mir_cslock lck(csRecordCache);
	if (dataRecords == nullptr) {
		/* load record data list from resources */
		hrsrc = FindResource(g_plugin.getInst(), MAKEINTRESOURCE(IDR_IPTOCOUNTRY), L"BIN");
		cb = SizeofResource(g_plugin.getInst(), hrsrc);
		dataRecords = (uint8_t*)LockResource(LoadResource(g_plugin.getInst(), hrsrc));
		if (cb <= sizeof(uint32_t) || dataRecords == nullptr)
			return FALSE;
		/* uncompress record data */
		dataRecords = GetDataHeader(dataRecords, cb, &nDataRecordsCount);
		if (dataRecords == nullptr || !nDataRecordsCount)
			return FALSE;
	}
	*pdata = dataRecords;
	*pcount = nDataRecordsCount;
	return TRUE;
}

static void LeaveRecordCache(void)
{
	/* mark for unload */
	CallFunctionBuffered(UnloadRecordCache, 0, FALSE, UNLOADDELAY);
}

/************************* Services *******************************/


INT_PTR ServiceIpToCountry(WPARAM wParam, LPARAM)
{
	uint8_t *data;
	uint32_t dwFrom, dwTo;
	uint32_t low = 0, i, high;
	int id;
	if (EnsureRecordCacheLoaded(&data, &high)) {
		/* binary search in record data */
		GetDataRecord(data, low, &dwFrom, nullptr);
		--high;
		if (wParam >= dwFrom) /* only search if wParam valid */
			while (low <= high) {
				i = low + ((high - low) / 2);
				/* analyze record */
				id = GetDataRecord(data, i, &dwFrom, &dwTo);
				if (dwFrom <= wParam && dwTo >= wParam) { LeaveRecordCache(); return id; }
				if (wParam > dwTo) low = i + 1;
				else high = i - 1;
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
} static const differentCountryNames[] = {
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
	uint8_t *buf;
	uint32_t cbLength, cbAlloced;
};

static void AppendToByteBuffer(struct ResizableByteBuffer *buffer, const void *append, uint32_t cbAppendSize)
{
	if (buffer->cbAlloced <= buffer->cbLength + cbAppendSize) {
		uint8_t* buf = (uint8_t*)mir_realloc(buffer->buf, buffer->cbAlloced + ALLOC_STEP + cbAppendSize);
		if (buf == NULL) return;
		buffer->buf = buf;
		buffer->cbAlloced += ALLOC_STEP + cbAppendSize;
		OutputDebugStringA("reallocating memory...\n"); /* all ascii */
	}
	memcpy(&buffer->buf[buffer->cbLength], append, cbAppendSize);
	buffer->cbLength += cbAppendSize;
}

static int EnumIpDataLines(const char *pszFileCSV, const char *pszFileOut)
{
	FILE *fp;
	char line[1024], out[512], *pszFrom, *pszTo, *pszTwo, *pszCountry, *buf;
	int i, j;
	uint32_t dwOut;
	uint16_t wOut;
	struct ResizableByteBuffer buffer;

	memset(&buffer, 0, sizeof(buffer));
	fp = fopen(pszFileCSV, "rt");
	if (fp != NULL) {
		OutputDebugStringA("Running IP data convert...\n"); /* all ascii */
		while (!feof(fp)) {
			if (fgets(line, sizeof(line), fp) == NULL) break;
			/* get line data */
			pszFrom = line + 1;
			pszTo = strchr(pszFrom, ',');
			*(pszTo - 1) = '\0'; pszTo += 2;
			pszTwo = strchr(pszTo, ',');
			*(pszTwo - 1) = '\0'; pszTwo += 2;
			pszCountry = strchr(pszTwo, ',') + 1;
			pszCountry = strchr(pszCountry, ',') + 2;
			buf = strchr(pszCountry, '"');
			*buf = pszTwo[2] = '\0';
			/* corrections */
			if (!mir_wstrcmpi(pszCountry, "ANTARCTICA")) continue;
			if (!mir_wstrcmpi(pszCountry, "TIMOR-LESTE")) continue;
			if (!mir_wstrcmpi(pszCountry, "PALESTINIAN TERRITORY, OCCUPIED"))
				mir_wstrcpy(pszCountry, "ISRAEL");
			else if (!mir_wstrcmpi(pszCountry, "UNITED STATES MINOR OUTLYING ISLANDS"))
				mir_wstrcpy(pszCountry, "UNITED STATES");
			else if (!mir_wstrcmpi(pszCountry, "SOUTH GEORGIA AND THE SOUTH SANDWICH ISLANDS"))
				mir_wstrcpy(pszCountry, "UNITED KINGDOM");
			else if (!mir_wstrcmpi(pszTwo, "JE")) /* map error */
				mir_wstrcpy(pszCountry, "UNITED KINGDOM");
			else if (!mir_wstrcmpi(pszTwo, "AX")) /* Еland Island belongs to Finland */
				mir_wstrcpy(pszCountry, "FINLAND");
			else if (!mir_wstrcmpi(pszTwo, "ME"))
				mir_wstrcpy(pszCountry, "MONTENEGRO");
			else if (!mir_wstrcmpi(pszTwo, "RS") || !mir_wstrcmpi(pszTwo, "CS"))
				mir_wstrcpy(pszCountry, "SERBIA");
			/* convert */
			for (i = 0; i < nCountriesCount; i++) {
				/* map different writings */
				for (j = 0; j < _countof(differentCountryNames); j++)
					if (!mir_wstrcmpi(countries[i].szName, differentCountryNames[j].szMir)) {
						buf = (char*)differentCountryNames[j].szCSV;
						break;
					}
				if (j == _countof(differentCountryNames))
					buf = (char*)countries[i].szName;
				/* check country */
				if (!mir_strcmpi(pszCountry, buf)) {
					dwOut = (uint32_t)atoi(pszFrom);
					AppendToByteBuffer(&buffer, (void*)&dwOut, sizeof(uint32_t));
					dwOut = (uint32_t)atoi(pszTo);
					AppendToByteBuffer(&buffer, (void*)&dwOut, sizeof(uint32_t));
					wOut = (uint16_t)countries[i].id;
					AppendToByteBuffer(&buffer, (void*)&wOut, sizeof(uint16_t));
					break;
				}
			}
			/* not in list */
			if (i == nCountriesCount) {
				mir_snprintf(out, "Unknown: %s-%s [%s, %s]\n", pszFrom, pszTo, pszTwo, pszCountry);
				OutputDebugStringA(out); /* all ascii */
			}
		}
		fclose(fp);
		OutputDebugStringA("Done!\n"); /* all ascii */
		if (buffer.buf != NULL) {
			HANDLE hFileOut;
			uint32_t cbWritten = 0;
			uint8_t *compressed;
			uint32_t cbCompressed;
			/* compress whole data */
			OutputDebugStringA("Compressing...\n"); /* all ascii */
			compressed = (uint8_t*)mir_alloc(buffer.cbAlloced + 384);
			if (compressed != NULL) {
				cbCompressed = Huffman_Compress(buffer.buf, compressed, buffer.cbLength);
				OutputDebugStringA("Done!\n"); /* all ascii */
				OutputDebugStringA("Writing to file...\n"); /* all ascii */
				hFileOut = CreateFile(pszFileOut, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (hFileOut != INVALID_HANDLE_VALUE) {
					/* store data length count at beginning */
					dwOut = buffer.cbLength;
					WriteFile(hFileOut, &dwOut, sizeof(uint32_t), &cbWritten, NULL);
					/* store compressed data records */
					WriteFile(hFileOut, compressed, cbCompressed, &cbWritten, NULL);
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
	if (MessageBox(NULL, _T("Looking for 'ip-to-country.csv' in current directory.\n"
		"It will be converted into 'ip-to-country.bin'.\n"
		"See debug output for more details.\n"
		"This process may take very long."), L"Bin Converter", MB_OKCANCEL | MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TOPMOST | MB_TASKMODAL) == IDOK) {
		EnumIpDataLines("ip-to-country.csv", "ip-to-country.bin");
		MessageBox(NULL, L"Done!\n'ip-to-country.bin' has been created in current directory.", L"Bin Converter", MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TOPMOST | MB_TASKMODAL);
	}
}

#endif // BINCONV 

/************************* Misc ***********************************/

void InitIpToCountry(void)
{
	nDataRecordsCount = 0;
	dataRecords = nullptr;
	/* Services */
	CreateServiceFunction(MS_FLAGS_IPTOCOUNTRY, ServiceIpToCountry);
#ifdef BINCONV
	mir_forkthread(BinConvThread);
#endif
}

void UninitIpToCountry(void)
{
	mir_free(dataRecords);
}
