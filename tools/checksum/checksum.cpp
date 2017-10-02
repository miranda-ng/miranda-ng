
// Checksum Tool
// By Bio (C) 2012

#define _VERSION_ "3.1"

#include "commonheaders.h"

// Return codes
#define RESULT_OK			0
#define RESULT_NOTFOUND		10
#define RESULT_READERROR	20
#define RESULT_NOTPE		30
#define RESULT_CORRUPTED	40
#define RESULT_INVALID		50
#define RESULT_NONE			100

int debug = 0;

static void PatchResourcesDirectory(PIMAGE_RESOURCE_DIRECTORY pIRD, PBYTE pBase);

static void PatchResourceEntry(PIMAGE_RESOURCE_DIRECTORY_ENTRY pIRDE, PBYTE pBase)
{
	if (pIRDE->DataIsDirectory)
		PatchResourcesDirectory(PIMAGE_RESOURCE_DIRECTORY(pBase + pIRDE->OffsetToDirectory), pBase);
}

static void PatchResourcesDirectory(PIMAGE_RESOURCE_DIRECTORY pIRD, PBYTE pBase)
{
	UINT i;
	pIRD->TimeDateStamp = 0;

	PIMAGE_RESOURCE_DIRECTORY_ENTRY pIRDE = PIMAGE_RESOURCE_DIRECTORY_ENTRY(pIRD + 1);
	for (i = 0; i < pIRD->NumberOfNamedEntries; i++, pIRDE++)
		PatchResourceEntry(pIRDE, pBase);

	for (i = 0; i < pIRD->NumberOfIdEntries; i++, pIRDE++)
		PatchResourceEntry(pIRDE, pBase);
}

__forceinline bool Contains(PIMAGE_SECTION_HEADER pISH, DWORD address, DWORD size = 0)
{
	return (address >= pISH->VirtualAddress && address + size <= pISH->VirtualAddress + pISH->SizeOfRawData);
}

int PEChecksum(wchar_t *filename, BYTE digest[16])
{
	HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return RESULT_NOTFOUND;

	// check minimum and maximum size
	DWORD hsize = 0;
	DWORD filesize = GetFileSize(hFile, &hsize);
	if (!filesize || filesize == INVALID_FILE_SIZE || hsize) {
		CloseHandle(hFile);
		return RESULT_INVALID;
	}

	if (filesize < sizeof(IMAGE_DOS_HEADER) + sizeof(IMAGE_NT_HEADERS)) {
		CloseHandle(hFile);
		return RESULT_NOTPE;
	}

	PBYTE ptr = nullptr;
	HANDLE hMap = CreateFileMapping(hFile, NULL, PAGE_WRITECOPY, 0, 0, NULL);
	if (hMap)
		ptr = (PBYTE)MapViewOfFile(hMap, FILE_MAP_COPY, 0, 0, 0);

	int res = RESULT_OK;
	if (ptr) {
		PIMAGE_DOS_HEADER pIDH = (PIMAGE_DOS_HEADER)ptr;

		PIMAGE_NT_HEADERS pINTH = nullptr;
		if (pIDH->e_magic == IMAGE_DOS_SIGNATURE)
			pINTH = (PIMAGE_NT_HEADERS)(ptr + pIDH->e_lfanew);

		if (!pINTH)
			res = RESULT_NOTPE;
		else if ((PBYTE)pINTH + sizeof(IMAGE_NT_HEADERS) >= ptr + filesize)
			res = RESULT_CORRUPTED;
		else if (pINTH->Signature != IMAGE_NT_SIGNATURE)
			res = RESULT_NOTPE;
		else {
			WORD machine = pINTH->FileHeader.Machine;

			if (debug) {
				switch (machine) {
				case IMAGE_FILE_MACHINE_I386:
					fwprintf(stderr, L"Build: x86\n");
					break;
				case IMAGE_FILE_MACHINE_AMD64:
					fwprintf(stderr, L"Build: x64\n");
					break;
				case IMAGE_FILE_MACHINE_IA64:
					fwprintf(stderr, L"Build: IA64 :-)\n");
					break;
				default:
					fwprintf(stderr, L"Build: unknown :-(\n");
					break;
				}
			}

			DWORD sections = pINTH->FileHeader.NumberOfSections;
			if (!sections)
				res = RESULT_INVALID;
			else {
				PIMAGE_DATA_DIRECTORY pIDD = 0;
				PIMAGE_DEBUG_DIRECTORY pDBG = 0;
				DWORD dbgSize = 0, dbgAddr = 0;    // debug information
				DWORD expSize = 0, expAddr = 0;    // export table
				DWORD resSize = 0, resAddr = 0;    // resource directory
				DWORD relocSize = 0, relocAddr = 0; // relocation table
				PBYTE pRealloc = nullptr;
				ULONGLONG base = 0;

				// try to found correct offset independent of architectures
				DWORD offset = pIDH->e_lfanew + pINTH->FileHeader.SizeOfOptionalHeader + sizeof(IMAGE_NT_HEADERS) - sizeof(IMAGE_OPTIONAL_HEADER);

				if ((machine == IMAGE_FILE_MACHINE_I386) &&
					(pINTH->FileHeader.SizeOfOptionalHeader >= sizeof(IMAGE_OPTIONAL_HEADER32)) &&
					(pINTH->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC))
				{
					pIDD = (PIMAGE_DATA_DIRECTORY)((PBYTE)pINTH + offsetof(IMAGE_NT_HEADERS32, OptionalHeader.DataDirectory));
					base = *(DWORD*)((PBYTE)pINTH + offsetof(IMAGE_NT_HEADERS32, OptionalHeader.ImageBase));
				}
				else if ((machine == IMAGE_FILE_MACHINE_AMD64) &&
					(pINTH->FileHeader.SizeOfOptionalHeader >= sizeof(IMAGE_OPTIONAL_HEADER64)) &&
					(pINTH->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC))
				{
					pIDD = (PIMAGE_DATA_DIRECTORY)((PBYTE)pINTH + offsetof(IMAGE_NT_HEADERS64, OptionalHeader.DataDirectory));
					base = *(ULONGLONG*)((PBYTE)pINTH + offsetof(IMAGE_NT_HEADERS64, OptionalHeader.ImageBase));
				}
				else res = RESULT_CORRUPTED;

				if (debug)
					fwprintf(stderr, L"Image base is 0x%I64x \n", base);

				if (pIDD) {
					// Debugging information entry
					dbgAddr = pIDD[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;
					dbgSize = pIDD[IMAGE_DIRECTORY_ENTRY_DEBUG].Size;

					// Export information entry
					expAddr = pIDD[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
					expSize = pIDD[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;

					// Resource directory
					resAddr = pIDD[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;
					resSize = pIDD[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size;

					// Reallocation information entry
					relocAddr = pIDD[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
					relocSize = pIDD[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
				}

				// verify image integrity
				for (DWORD idx = 0; idx < sections; idx++) {
					PIMAGE_SECTION_HEADER pISH = (PIMAGE_SECTION_HEADER)(ptr + offset + idx * sizeof(IMAGE_SECTION_HEADER));
					if (((PBYTE)pISH + sizeof(IMAGE_SECTION_HEADER) > ptr + filesize) || (pISH->PointerToRawData + pISH->SizeOfRawData > filesize)) {
						res = RESULT_CORRUPTED;
						break;
					}

					// erase timestamp
					if (dbgSize >= sizeof(IMAGE_DEBUG_DIRECTORY) && Contains(pISH, dbgAddr, dbgSize)) {
						DWORD shift = dbgAddr - pISH->VirtualAddress;
						pDBG = (PIMAGE_DEBUG_DIRECTORY)(ptr + shift + pISH->PointerToRawData);
						for (int i = dbgSize / sizeof(IMAGE_DEBUG_DIRECTORY); i > 0; i--)
							pDBG[i-1].TimeDateStamp = 0;

						if (debug)
							fwprintf(stderr, L"Found debug section entry at 0x%08X (%d), data at 0x%08X (%d)\n", pISH->PointerToRawData + shift, dbgSize, pDBG->PointerToRawData, pDBG->SizeOfData);
					}

					// erase export timestamp
					if (expSize >= sizeof(IMAGE_EXPORT_DIRECTORY) && Contains(pISH, expAddr, expSize)) {
						DWORD shift = expAddr - pISH->VirtualAddress;
						PIMAGE_EXPORT_DIRECTORY pEXP = (PIMAGE_EXPORT_DIRECTORY)(ptr + shift + pISH->PointerToRawData);
						pEXP->TimeDateStamp = 0;

						if (debug)
							fwprintf(stderr, L"Found export section entry at 0x%08X\n", pISH->PointerToRawData + shift);
					}

					// find realocation table
					if ((relocSize >= sizeof(IMAGE_BASE_RELOCATION)) && Contains(pISH, relocAddr, relocSize)) {
						DWORD shift = relocAddr - pISH->VirtualAddress;
						pRealloc = ptr + shift + pISH->PointerToRawData;

						if (debug)
							fwprintf(stderr, L"Found reallocation table entry at 0x%08X (%d)\n", pISH->PointerToRawData + shift, relocSize);
					}
				}

				if (res == RESULT_OK) {
					mir_md5_state_t pms;
					mir_md5_init(&pms);

					for (size_t idx = 0; idx < sections; idx++) {
						PIMAGE_SECTION_HEADER pISH = (PIMAGE_SECTION_HEADER)(ptr + offset + idx * sizeof(IMAGE_SECTION_HEADER));

						if (((PBYTE)pISH + sizeof(IMAGE_SECTION_HEADER) > ptr + filesize) || (pISH->PointerToRawData + pISH->SizeOfRawData > filesize)) {
							res = RESULT_CORRUPTED;
							break;
						}

						// erase debug information
						if (pDBG && pDBG->SizeOfData > 0)
							if (pDBG->PointerToRawData >= pISH->PointerToRawData && pDBG->PointerToRawData + pDBG->SizeOfData <= pISH->PointerToRawData + pISH->SizeOfRawData)
								ZeroMemory(ptr + pDBG->PointerToRawData, pDBG->SizeOfData);

						// patch resources
						if (resSize > 0 && resAddr >= pISH->VirtualAddress && resAddr + resSize <= pISH->VirtualAddress + pISH->SizeOfRawData) {
							DWORD shift = resAddr - pISH->VirtualAddress + pISH->PointerToRawData;
							IMAGE_RESOURCE_DIRECTORY *pIRD = (IMAGE_RESOURCE_DIRECTORY*)(ptr + shift);
							PatchResourcesDirectory(pIRD, ptr + shift);
						}

						// rebase to zero address
						if (pRealloc) {
							DWORD blocklen = relocSize;
							PIMAGE_BASE_RELOCATION pIBR = (PIMAGE_BASE_RELOCATION)pRealloc;
							while (pIBR) {
								if (Contains(pISH, pIBR->VirtualAddress) && pIBR->SizeOfBlock <= blocklen) {
									DWORD shift = pIBR->VirtualAddress - pISH->VirtualAddress + pISH->PointerToRawData;

									int len = pIBR->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION);

									PWORD pw = (PWORD)((PBYTE)pIBR + sizeof(IMAGE_BASE_RELOCATION));
									if (debug)
										fwprintf(stderr, L"Realloc block at %08X (%d)\n", pIBR->VirtualAddress, pIBR->SizeOfBlock);

									while (len > 0) {
										DWORD type = *pw >> 12;
										DWORD addr = (*pw & 0x0FFF);
										PBYTE pAddr = ptr + shift + addr;

										switch (type) {
										case IMAGE_REL_BASED_HIGHLOW:
											if (addr + pIBR->VirtualAddress + sizeof(DWORD) >= pISH->VirtualAddress + pISH->SizeOfRawData) {
												len = 0;
												break;
											}

											if (debug && (*(PDWORD)pAddr < (DWORD)base))
												fwprintf(stderr, L"Realloc address is less than base\n");

											*(PDWORD)pAddr = (DWORD)((*(PDWORD)pAddr) - (DWORD)base);
											break;

										case IMAGE_REL_BASED_DIR64:
											if (addr + pIBR->VirtualAddress + sizeof(ULONGLONG) >= pISH->VirtualAddress + pISH->SizeOfRawData) {
												len = 0;
												break;
											}

											if (debug && (*(ULONGLONG*)pAddr < base))
												fwprintf(stderr, L"Realloc address is less than base\n");

											*(ULONGLONG*)pAddr = (ULONGLONG)((*(ULONGLONG*)pAddr) - base);
											break;

										case IMAGE_REL_BASED_ABSOLUTE:
											// stop processing
											len = 0;
											break;

										case IMAGE_REL_BASED_HIGH:
										case IMAGE_REL_BASED_LOW:
										case IMAGE_REL_BASED_HIGHADJ:
											if (debug)
												fwprintf(stderr, L"Unexpected block type %d\n", type);
											break;

										default:
											if (debug)
												fwprintf(stderr, L"Unknown block type %d\n", type);
											break;
										}

										len -= sizeof(WORD);
										pw++;
									}
								}

								blocklen -= pIBR->SizeOfBlock;
								if (blocklen > sizeof(IMAGE_BASE_RELOCATION))
									pIBR = (PIMAGE_BASE_RELOCATION)((PBYTE)pIBR + pIBR->SizeOfBlock);
								else
									break;
							}
						}

						if (debug) {
							BYTE digest2[16];
							mir_md5_state_t pms2;
							mir_md5_init(&pms2);
							mir_md5_append(&pms2, ptr + pISH->PointerToRawData, pISH->SizeOfRawData);
							mir_md5_finish(&pms2, digest2);

							fwprintf(stderr, L"%S - %08X - %d ", pISH->Name, pISH->PointerToRawData, pISH->SizeOfRawData);
							for (int i = 0; i < sizeof(digest2) / sizeof(digest2[0]); i++)
								fwprintf(stderr, L"%02X", digest2[i]);
							fwprintf(stderr, L"\n");
						}

						mir_md5_append(&pms, ptr + pISH->PointerToRawData, pISH->SizeOfRawData);
					}

					if (res == RESULT_OK)
						mir_md5_finish(&pms, digest);
				}
			}
		}
	}
	else res = RESULT_READERROR;

	if (ptr)
		UnmapViewOfFile(ptr);

	if (hMap)
		CloseHandle(hMap);

	CloseHandle(hFile);

	return res;
}

wchar_t* trtrim(wchar_t *str)
{
	if (str == NULL)
		return NULL;

	wchar_t* p = _tcschr(str, 0);
	while (--p >= str) {
		switch (*p) {
		case L' ': case L'\t': case L'\n': case L'\r':
			*p = 0; break;
		default:
			return str;
		}
	}
	return str;
}


int process(wchar_t *filename)
{
	BYTE digest[16] = { 0 };

	int res = PEChecksum(filename, digest);

	switch (res) {
	case RESULT_NOTFOUND:
		fwprintf(stderr, L"'%s'... not found!\n", filename);
		break;

	case RESULT_READERROR:
		fwprintf(stderr, L"'%s'... read error!\n", filename);
		break;

	case RESULT_NOTPE:
		fwprintf(stderr, L"'%s'... not PE type!\n", filename);
		break;

	case RESULT_CORRUPTED:
		fwprintf(stderr, L"'%s'... corrupted!\n", filename);
		break;

	case RESULT_OK:
		fwprintf(stdout, L"%s ", filename);
		for (int i = 0; i < sizeof(digest) / sizeof(digest[0]); i++)
			fwprintf(stdout, L"%02X", digest[i]);
		fwprintf(stdout, L"\n");
		break;

	default:
		break;
	}
	return res;
}


int _tmain(int argc, wchar_t *argv[])
{
	wchar_t buf[MAX_PATH];
	int res = 0;
	int cnt = 0;
	int i;

	fwprintf(stderr, L"* PE CHECKSUM TOOL * VERSION %S * by Bio (c) 2012\n\n", _VERSION_);

	if (argc > 1) {
		WIN32_FIND_DATA ffd;
		HANDLE hFind = INVALID_HANDLE_VALUE;

		for (i = 1; i < argc; i++) {
			if (!wcscmp(argv[i], L"/debug") || !wcscmp(argv[i], L"/DEBUG")) {
				debug = 1;
				break;
			}
		}

		fwprintf(stderr, L"Processing ... \n");

		for (i = 1; i < argc; i++) {
			if (!wcscmp(argv[i], L"/stdin") || !wcscmp(argv[i], L"/STDIN")) {
				while (fgetws(buf, sizeof(buf), stdin) != NULL) {
					trtrim(buf);
					res = process(buf);
					cnt++;
				}
				continue;
			}

			wchar_t *p = wcsrchr(argv[i], '\\');
			if (p) {
				*p = 0;
				SetCurrentDirectory(argv[i]);
				*p = '\\';
			}

			hFind = FindFirstFile(argv[i], &ffd);

			while (hFind != INVALID_HANDLE_VALUE) {
				if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
					res = process(ffd.cFileName);
					cnt++;
				}
				if (!FindNextFile(hFind, &ffd))
					break;
			}

			FindClose(hFind);
		}

		fwprintf(stderr, L"%d file(s) processed.\n", cnt);
	}
	else {
		fwprintf(stderr, L"Usage:    checksum.exe [/debug] [/stdin] [*.dll] ... [*.exe]\n");
		fwprintf(stderr, L"Example:  dir /b /s | checksum.exe /stdin > hashes.txt\n");
		res = RESULT_NONE;
	}

	return res;
}
