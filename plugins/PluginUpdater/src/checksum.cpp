
// Checksum Tool
// By Bio (C) 2012

#include "stdafx.h"

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

#pragma comment(lib, "version.lib")

struct MFileMapping
{
	uint8_t *ptr;
	HANDLE hMap, hFile;

	MFileMapping(const wchar_t *pwszFileName)
	{
		ptr = nullptr;
		hMap = nullptr;

		hFile = CreateFile(pwszFileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
		if (hFile != INVALID_HANDLE_VALUE)
			hMap = CreateFileMapping(hFile, nullptr, PAGE_WRITECOPY, 0, 0, nullptr);
		if (hMap)
			ptr = (uint8_t*)MapViewOfFile(hMap, FILE_MAP_COPY, 0, 0, 0);
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

static void PatchResourcesDirectory(PIMAGE_RESOURCE_DIRECTORY pIRD, uint8_t *pBase);

static void PatchResourceEntry(PIMAGE_RESOURCE_DIRECTORY_ENTRY pIRDE, uint8_t *pBase)
{
	if (pIRDE->DataIsDirectory)
		PatchResourcesDirectory(PIMAGE_RESOURCE_DIRECTORY(pBase + pIRDE->OffsetToDirectory), pBase);
}

static void PatchResourcesDirectory(PIMAGE_RESOURCE_DIRECTORY pIRD, uint8_t *pBase)
{
	UINT i;
	pIRD->TimeDateStamp = 0;

	PIMAGE_RESOURCE_DIRECTORY_ENTRY pIRDE = PIMAGE_RESOURCE_DIRECTORY_ENTRY(pIRD + 1);
	for (i = 0; i < pIRD->NumberOfNamedEntries; i++, pIRDE++)
		PatchResourceEntry(pIRDE, pBase);

	for (i = 0; i < pIRD->NumberOfIdEntries; i++, pIRDE++)
		PatchResourceEntry(pIRDE, pBase);
}

__forceinline bool Contains(PIMAGE_SECTION_HEADER pISH, uint32_t address, uint32_t size = 0)
{
	return (address >= pISH->VirtualAddress && address + size <= pISH->VirtualAddress + pISH->SizeOfRawData);
}

int CalculateModuleHash(const wchar_t *filename, char *szDest)
{
	MFileMapping map(filename);
	if (map.hFile == INVALID_HANDLE_VALUE)
		return RESULT_NOTFOUND;

	if (map.ptr == nullptr)
		return RESULT_READERROR;

	// check minimum and maximum size
	DWORD hsize = 0, filesize = GetFileSize(map.hFile, &hsize);
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
		if ((uint8_t*)pINTH + sizeof(IMAGE_NT_HEADERS) >= map.ptr + filesize)
			return RESULT_CORRUPTED;
		
		if (pINTH->Signature != IMAGE_NT_SIGNATURE)
			goto LBL_NotPE;

		uint16_t  machine = pINTH->FileHeader.Machine;
		uint32_t sections = pINTH->FileHeader.NumberOfSections;
		if (!sections)
			return RESULT_INVALID;

		PIMAGE_DATA_DIRECTORY pIDD = nullptr;
		PIMAGE_DEBUG_DIRECTORY pDBG = nullptr;
		uint8_t *pRealloc = nullptr;
		ULONGLONG base = 0;

		// try to found correct offset independent of architectures
		uint32_t offset = pIDH->e_lfanew + pINTH->FileHeader.SizeOfOptionalHeader + sizeof(IMAGE_NT_HEADERS) - sizeof(IMAGE_OPTIONAL_HEADER);

		if ((machine == IMAGE_FILE_MACHINE_I386) &&
			(pINTH->FileHeader.SizeOfOptionalHeader >= sizeof(IMAGE_OPTIONAL_HEADER32)) &&
			(pINTH->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)) {
			pIDD = (PIMAGE_DATA_DIRECTORY)((uint8_t*)pINTH + offsetof(IMAGE_NT_HEADERS32, OptionalHeader.DataDirectory));
			base = *(uint32_t*)((uint8_t*)pINTH + offsetof(IMAGE_NT_HEADERS32, OptionalHeader.ImageBase));
		}
		else if ((machine == IMAGE_FILE_MACHINE_AMD64) &&
			(pINTH->FileHeader.SizeOfOptionalHeader >= sizeof(IMAGE_OPTIONAL_HEADER64)) &&
			(pINTH->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)) {
			pIDD = (PIMAGE_DATA_DIRECTORY)((uint8_t*)pINTH + offsetof(IMAGE_NT_HEADERS64, OptionalHeader.DataDirectory));
			base = *(ULONGLONG*)((uint8_t*)pINTH + offsetof(IMAGE_NT_HEADERS64, OptionalHeader.ImageBase));
		}
		else return RESULT_CORRUPTED;

		// Debugging information entry
		uint32_t dbgAddr = pIDD[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;
		uint32_t dbgSize = pIDD[IMAGE_DIRECTORY_ENTRY_DEBUG].Size;

		// Export information entry
		uint32_t expAddr = pIDD[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
		uint32_t expSize = pIDD[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;

		// Resource directory
		uint32_t resAddr = pIDD[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;
		uint32_t resSize = pIDD[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size;

		// Reallocation information entry
		uint32_t relocAddr = pIDD[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
		uint32_t relocSize = pIDD[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;

		// verify image integrity
		for (uint32_t idx = 0; idx < sections; idx++) {
			PIMAGE_SECTION_HEADER pISH = (PIMAGE_SECTION_HEADER)(map.ptr + offset + idx * sizeof(IMAGE_SECTION_HEADER));
			if (((uint8_t*)pISH + sizeof(IMAGE_SECTION_HEADER) > map.ptr + filesize) || (pISH->PointerToRawData + pISH->SizeOfRawData > filesize))
				return RESULT_CORRUPTED;

			// erase timestamp
			if (dbgSize >= sizeof(IMAGE_DEBUG_DIRECTORY) && Contains(pISH, dbgAddr, dbgSize)) {
				uint32_t shift = dbgAddr - pISH->VirtualAddress;
				pDBG = (PIMAGE_DEBUG_DIRECTORY)(map.ptr + shift + pISH->PointerToRawData);
				for (int i = dbgSize / sizeof(IMAGE_DEBUG_DIRECTORY); i > 0; i--)
					pDBG[i - 1].TimeDateStamp = 0;
			}

			// erase export timestamp
			if (expSize >= sizeof(IMAGE_EXPORT_DIRECTORY) && Contains(pISH, expAddr, expSize)) {
				uint32_t shift = expAddr - pISH->VirtualAddress;
				PIMAGE_EXPORT_DIRECTORY pEXP = (PIMAGE_EXPORT_DIRECTORY)(map.ptr + shift + pISH->PointerToRawData);
				pEXP->TimeDateStamp = 0;
			}

			// find realocation table
			if ((relocSize >= sizeof(IMAGE_BASE_RELOCATION)) && Contains(pISH, relocAddr, relocSize)) {
				uint32_t shift = relocAddr - pISH->VirtualAddress;
				pRealloc = map.ptr + shift + pISH->PointerToRawData;
			}
		}

		for (size_t idx = 0; idx < sections; idx++) {
			PIMAGE_SECTION_HEADER pISH = (PIMAGE_SECTION_HEADER)(map.ptr + offset + idx * sizeof(IMAGE_SECTION_HEADER));

			if (((uint8_t*)pISH + sizeof(IMAGE_SECTION_HEADER) > map.ptr + filesize) || (pISH->PointerToRawData + pISH->SizeOfRawData > filesize))
				return RESULT_CORRUPTED;

			// erase debug information
			if (pDBG && pDBG->SizeOfData > 0)
				if (pDBG->PointerToRawData >= pISH->PointerToRawData && pDBG->PointerToRawData + pDBG->SizeOfData <= pISH->PointerToRawData + pISH->SizeOfRawData)
					ZeroMemory(map.ptr + pDBG->PointerToRawData, pDBG->SizeOfData);

			// patch resources
			if (resSize > 0 && Contains(pISH, resAddr, resSize)) {
				uint32_t shift = resAddr - pISH->VirtualAddress + pISH->PointerToRawData;
				IMAGE_RESOURCE_DIRECTORY *pIRD = (IMAGE_RESOURCE_DIRECTORY*)(map.ptr + shift);
				PatchResourcesDirectory(pIRD, map.ptr + shift);
			}

			// rebase to zero address
			if (pRealloc) {
				uint32_t blocklen = relocSize;
				PIMAGE_BASE_RELOCATION pIBR = (PIMAGE_BASE_RELOCATION)pRealloc;
				while (pIBR) {
					if (Contains(pISH, pIBR->VirtualAddress) && pIBR->SizeOfBlock <= blocklen) {
						uint32_t shift = pIBR->VirtualAddress - pISH->VirtualAddress + pISH->PointerToRawData;
						int len = pIBR->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION);
						PWORD pw = (PWORD)((uint8_t*)pIBR + sizeof(IMAGE_BASE_RELOCATION));

						while (len > 0) {
							uint32_t type = *pw >> 12;
							uint32_t addr = (*pw & 0x0FFF);
							uint8_t *pAddr = map.ptr + shift + addr;

							switch (type) {
							case IMAGE_REL_BASED_HIGHLOW:
								if (addr + pIBR->VirtualAddress + sizeof(uint32_t) >= pISH->VirtualAddress + pISH->SizeOfRawData) {
									len = 0;
									break;
								}
								*(PDWORD)pAddr = (uint32_t)((*(PDWORD)pAddr) - (uint32_t)base);
								break;

							case IMAGE_REL_BASED_DIR64:
								if (addr + pIBR->VirtualAddress + sizeof(ULONGLONG) >= pISH->VirtualAddress + pISH->SizeOfRawData) {
									len = 0;
									break;
								}
								*(ULONGLONG*)pAddr = (ULONGLONG)((*(ULONGLONG*)pAddr) - base);
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

							len -= sizeof(uint16_t);
							pw++;
						}
					}

					blocklen -= pIBR->SizeOfBlock;
					if (blocklen <= sizeof(IMAGE_BASE_RELOCATION))
						break;

					pIBR = (PIMAGE_BASE_RELOCATION)((uint8_t*)pIBR + pIBR->SizeOfBlock);
				}
			}

			mir_md5_append(&pms, map.ptr + pISH->PointerToRawData, pISH->SizeOfRawData);
		}
	}

	uint8_t digest[16];
	mir_md5_finish(&pms, digest);
	bin2hex(digest, sizeof(digest), szDest);
	return RESULT_OK;
}
