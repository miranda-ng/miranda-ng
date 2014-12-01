
// Checksum Tool
// By Bio (C) 2012

#include "common.h"

// Return codes
#define RESULT_OK			0
#define RESULT_NOTFOUND		10
#define RESULT_READERROR	20
#define RESULT_NOTPE		30
#define RESULT_CORRUPTED	40
#define RESULT_INVALID		50
#define RESULT_NONE			100

#define DEBUG_SECTIONS		1
#define DEBUG_REALLOCS		1

int debug = 0;

struct MFileMapping
{
	PBYTE  ptr;
	HANDLE hMap, hFile;

	MFileMapping(const TCHAR* ptszFileName)
	{
		ptr = NULL;
		hMap = NULL;

		hFile = CreateFile(ptszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
			hMap = CreateFileMapping( hFile, NULL, PAGE_WRITECOPY, 0, 0, NULL);
		if (hMap)
			ptr = (PBYTE)MapViewOfFile(hMap, FILE_MAP_COPY, 0, 0, 0);
	}

	~MFileMapping()
	{
		if (ptr)
			UnmapViewOfFile(ptr);

		if (hMap)
			CloseHandle(hMap);

		if (hFile != INVALID_HANDLE_VALUE)
			CloseHandle(hFile);
	}
};

static void PatchResourcesDirectory(PIMAGE_RESOURCE_DIRECTORY pIRD, BYTE *pBase);

static void PatchResourceEntry(PIMAGE_RESOURCE_DIRECTORY_ENTRY pIRDE, BYTE *pBase)
{
	if (pIRDE->DataIsDirectory)
		PatchResourcesDirectory( PIMAGE_RESOURCE_DIRECTORY(pBase + pIRDE->OffsetToDirectory), pBase);
}

static void PatchResourcesDirectory(PIMAGE_RESOURCE_DIRECTORY pIRD, PBYTE pBase)
{
	UINT i;
	pIRD->TimeDateStamp = 0;

	PIMAGE_RESOURCE_DIRECTORY_ENTRY pIRDE = PIMAGE_RESOURCE_DIRECTORY_ENTRY(pIRD+1);
	for ( i=0; i < pIRD->NumberOfNamedEntries; i++, pIRDE++ )
		PatchResourceEntry(pIRDE, pBase);

	for ( i=0; i < pIRD->NumberOfIdEntries; i++, pIRDE++ )
		PatchResourceEntry(pIRDE, pBase);
}

int CalculateModuleHash(const TCHAR *filename, char *szDest)
{
	MFileMapping map(filename);
	if (map.hFile == INVALID_HANDLE_VALUE)
		return RESULT_NOTFOUND;

	if (map.ptr == NULL)
		return RESULT_READERROR;

	// check minimum and maximum size
	DWORD hsize = 0, filesize = GetFileSize(map.hFile, &hsize );

	if (!filesize || filesize == INVALID_FILE_SIZE || hsize)
		return RESULT_INVALID;

	if (filesize < sizeof(IMAGE_DOS_HEADER) + sizeof(IMAGE_NT_HEADERS))
		return RESULT_NOTPE;

	mir_md5_state_t pms;
	mir_md5_init(&pms);

	PIMAGE_DOS_HEADER pIDH = (PIMAGE_DOS_HEADER)map.ptr;
	if (pIDH->e_magic != IMAGE_DOS_SIGNATURE) {
LBL_NotPE:
		mir_md5_append(&pms, map.ptr, filesize);
	}
	else {
		PIMAGE_NT_HEADERS pINTH = (PIMAGE_NT_HEADERS)(map.ptr + pIDH->e_lfanew);
		if ((PBYTE)pINTH + sizeof(IMAGE_NT_HEADERS) >= map.ptr + filesize )
			return RESULT_CORRUPTED;
		if (pINTH->Signature != IMAGE_NT_SIGNATURE)
			goto LBL_NotPE;

		WORD  machine = pINTH->FileHeader.Machine;
		DWORD sections = pINTH->FileHeader.NumberOfSections;
		if (!sections)
			return RESULT_INVALID;

		// try to found correct offset independent of architectures
		DWORD offset = pIDH->e_lfanew + pINTH->FileHeader.SizeOfOptionalHeader + sizeof(IMAGE_NT_HEADERS) - sizeof(IMAGE_OPTIONAL_HEADER);

		PBYTE pRealloc = 0;
		PIMAGE_DEBUG_DIRECTORY pDBG = 0;
		PIMAGE_DATA_DIRECTORY pIDD;
		ULONGLONG base;
		if ((machine == IMAGE_FILE_MACHINE_I386) &&
			(pINTH->FileHeader.SizeOfOptionalHeader >= sizeof(IMAGE_OPTIONAL_HEADER32)) &&
			(pINTH->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC))
		{
			pIDD = (PIMAGE_DATA_DIRECTORY)((PBYTE)pINTH + offsetof(IMAGE_NT_HEADERS32, OptionalHeader.DataDirectory));
			base = *(DWORD*)((PBYTE)pINTH + offsetof( IMAGE_NT_HEADERS32, OptionalHeader.ImageBase ));
		}
		else if ((machine == IMAGE_FILE_MACHINE_AMD64) &&
			(pINTH->FileHeader.SizeOfOptionalHeader >= sizeof(IMAGE_OPTIONAL_HEADER64)) &&
			(pINTH->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC))
		{
			pIDD = (PIMAGE_DATA_DIRECTORY)((PBYTE)pINTH + offsetof(IMAGE_NT_HEADERS64, OptionalHeader.DataDirectory));
			base = *(ULONGLONG*)((PBYTE)pINTH + offsetof( IMAGE_NT_HEADERS64, OptionalHeader.ImageBase ));
		}
		else return RESULT_CORRUPTED;

		// Debugging information entry
		DWORD dbgAddr = pIDD[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;
		DWORD dbgSize = pIDD[IMAGE_DIRECTORY_ENTRY_DEBUG].Size;

		// Export information entry
		DWORD expAddr = pIDD[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
		DWORD expSize = pIDD[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;

		// Resource directory
		DWORD resAddr = pIDD[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;
		DWORD resSize = pIDD[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size;

		// Reallocation information entry
		DWORD relocAddr = pIDD[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
		DWORD relocSize = pIDD[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;

		// verify image integrity
		for (DWORD idx=0; idx < sections; idx++) {
			PIMAGE_SECTION_HEADER pISH = (PIMAGE_SECTION_HEADER)(map.ptr + offset + idx * sizeof(IMAGE_SECTION_HEADER));
			if (((PBYTE)pISH + sizeof(IMAGE_SECTION_HEADER) > map.ptr + filesize) || (pISH->PointerToRawData + pISH->SizeOfRawData > filesize))
				return RESULT_CORRUPTED;

			// erase timestamp
			if (( dbgSize >= sizeof(IMAGE_DEBUG_DIRECTORY)) &&
					(dbgAddr >= pISH->VirtualAddress) &&
					(dbgAddr + dbgSize <= pISH->VirtualAddress + pISH->SizeOfRawData))
			{
				DWORD shift = dbgAddr - pISH->VirtualAddress;
				pDBG = (PIMAGE_DEBUG_DIRECTORY)(map.ptr + shift + pISH->PointerToRawData);
				pDBG->TimeDateStamp = 0;
			}

			// erase export timestamp
			if ((expSize >= sizeof(IMAGE_EXPORT_DIRECTORY)) &&
					(expAddr >= pISH->VirtualAddress) &&
					(expAddr + expSize <= pISH->VirtualAddress + pISH->SizeOfRawData))
			{
				DWORD shift = expAddr - pISH->VirtualAddress;
				PIMAGE_EXPORT_DIRECTORY pEXP = (PIMAGE_EXPORT_DIRECTORY)(map.ptr + shift + pISH->PointerToRawData);
				pEXP->TimeDateStamp = 0;
			}

			// find realocation table
			if ((relocSize >= sizeof(IMAGE_BASE_RELOCATION)) &&
					(relocAddr >= pISH->VirtualAddress) &&
					(relocAddr + relocSize <= pISH->VirtualAddress + pISH->SizeOfRawData))
			{
				DWORD shift = relocAddr - pISH->VirtualAddress;
				pRealloc = map.ptr + shift + pISH->PointerToRawData;
			}
		}

		for (size_t idx=0; idx < sections; idx++) {
			PIMAGE_SECTION_HEADER pISH = (PIMAGE_SECTION_HEADER)(map.ptr + offset + idx * sizeof(IMAGE_SECTION_HEADER));

			if (((PBYTE)pISH + sizeof(IMAGE_SECTION_HEADER) > map.ptr + filesize) || ( pISH->PointerToRawData + pISH->SizeOfRawData > filesize))
				return RESULT_CORRUPTED;

			// erase debug information
			if (pDBG && pDBG->SizeOfData > 0 &&
					pDBG->PointerToRawData >= pISH->PointerToRawData &&
					pDBG->PointerToRawData + pDBG->SizeOfData <= pISH->PointerToRawData + pISH->SizeOfRawData)
			{
				memset((map.ptr + pDBG->PointerToRawData), 0, pDBG->SizeOfData);
			}

			// patch resources
			if (resSize > 0 && resAddr >= pISH->VirtualAddress && resAddr + resSize <= pISH->VirtualAddress + pISH->SizeOfRawData) {
				DWORD shift = resAddr - pISH->VirtualAddress + pISH->PointerToRawData;
				IMAGE_RESOURCE_DIRECTORY *pIRD = (IMAGE_RESOURCE_DIRECTORY*)(map.ptr + shift);
				PatchResourcesDirectory(pIRD, map.ptr + shift);
			}

			// rebase to zero address
			if (pRealloc) {
				DWORD blocklen = relocSize;
				PIMAGE_BASE_RELOCATION pIBR = (PIMAGE_BASE_RELOCATION)pRealloc;

				while(pIBR) {
					if ((pIBR->VirtualAddress >= pISH->VirtualAddress) &&
							(pIBR->VirtualAddress < pISH->VirtualAddress + pISH->SizeOfRawData) &&
							(pIBR->SizeOfBlock <= blocklen))
					{
						DWORD shift = pIBR->VirtualAddress - pISH->VirtualAddress + pISH->PointerToRawData;
						int len = pIBR->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION);
						PWORD pw = (PWORD)((PBYTE)pIBR + sizeof(IMAGE_BASE_RELOCATION));

						while(len > 0) {
							DWORD type = *pw >> 12;
							DWORD addr = (*pw & 0x0FFF);
							PBYTE pAddr = map.ptr + shift + addr;

							switch(type) {
							case IMAGE_REL_BASED_HIGHLOW:
								if (addr + pIBR->VirtualAddress + sizeof(DWORD) >= pISH->VirtualAddress + pISH->SizeOfRawData) {
									len = 0;
									break;
								}
								*(PDWORD)pAddr = (DWORD)((*(PDWORD)pAddr) - (DWORD)base );
								break;

							case IMAGE_REL_BASED_DIR64:
								if (addr + pIBR->VirtualAddress + sizeof(ULONGLONG) >= pISH->VirtualAddress + pISH->SizeOfRawData) {
									len = 0;
									break;
								}
								*(ULONGLONG*)pAddr = (ULONGLONG)((*(ULONGLONG*)pAddr) - base );
								break;

							case IMAGE_REL_BASED_ABSOLUTE:
								// stop processing
								len = 0;
								break;

							case IMAGE_REL_BASED_HIGH:
							case IMAGE_REL_BASED_LOW:
							case IMAGE_REL_BASED_HIGHADJ:
								break;
							}

							len -= sizeof(WORD);
							pw++;
						}
					}

					blocklen -= pIBR->SizeOfBlock;
					if (blocklen <= sizeof(IMAGE_BASE_RELOCATION))
						break;

					pIBR = (PIMAGE_BASE_RELOCATION)((PBYTE)pIBR + pIBR->SizeOfBlock);
				}
			}

			mir_md5_append(&pms, map.ptr + pISH->PointerToRawData, pISH->SizeOfRawData);
		}
	}

	BYTE digest[16];
	mir_md5_finish(&pms, digest);
	bin2hex(digest, sizeof(digest), szDest);
	return RESULT_OK;
}
