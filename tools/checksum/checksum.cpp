
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


#define DEBUG_SECTIONS		1
#define DEBUG_REALLOCS		1

int debug = 0;

static void PatchResourcesDirectory(PIMAGE_RESOURCE_DIRECTORY pIRD, BYTE* pBase);

static void PatchResourceEntry(PIMAGE_RESOURCE_DIRECTORY_ENTRY pIRDE, BYTE* pBase)
{
	if ( pIRDE->DataIsDirectory )
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

int PEChecksum( TCHAR *filename, BYTE digest[16] )
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	HANDLE hMap;
	PBYTE ptr = 0;
	int res = RESULT_OK;
	DWORD filesize;
	DWORD hsize = 0;

	WORD machine = 0;
	DWORD sections = 0;

	hFile = CreateFile( filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );

	if ( hFile == INVALID_HANDLE_VALUE )
		return RESULT_NOTFOUND;

	// check minimum and maximum size
	filesize = GetFileSize( hFile, &hsize );

	if ( !filesize || filesize == INVALID_FILE_SIZE || hsize )
	{
		CloseHandle( hFile );
		return RESULT_INVALID;
	}

	if ( filesize < sizeof(IMAGE_DOS_HEADER) + sizeof(IMAGE_NT_HEADERS))
	{
		CloseHandle( hFile );
		return RESULT_NOTPE;
	}

	hMap = CreateFileMapping( hFile, NULL, PAGE_WRITECOPY, 0, 0, NULL );

	if ( hMap )
		ptr = (PBYTE)MapViewOfFile( hMap, FILE_MAP_COPY, 0, 0 ,0 );

	if ( ptr )
	{
		PIMAGE_DOS_HEADER pIDH = 0;
		PIMAGE_NT_HEADERS pINTH = 0;

		pIDH = (PIMAGE_DOS_HEADER)ptr;

		if ( pIDH->e_magic == IMAGE_DOS_SIGNATURE )
			pINTH = (PIMAGE_NT_HEADERS)( ptr + pIDH->e_lfanew );

		if ( !pINTH)
			res = RESULT_NOTPE;
		else
		if ((PBYTE)pINTH + sizeof(IMAGE_NT_HEADERS) >= ptr + filesize )
			res = RESULT_CORRUPTED;
		else
		if ( pINTH->Signature != IMAGE_NT_SIGNATURE )
			res = RESULT_NOTPE;
		else
		{
			machine = pINTH->FileHeader.Machine;

#ifdef DEBUG_SECTIONS
			if ( debug ) {
				switch(machine) {
				case IMAGE_FILE_MACHINE_I386:
					_ftprintf( stderr, _T("Build: x86\n"));
					break;
				case IMAGE_FILE_MACHINE_AMD64:
					_ftprintf( stderr, _T("Build: x64\n"));
					break;
				case IMAGE_FILE_MACHINE_IA64:
					_ftprintf( stderr, _T("Build: IA64 :-)\n"));
					break;
				default:
					_ftprintf( stderr, _T("Build: unknown :-(\n"));
					break;
				}
			}
#endif
			sections = pINTH->FileHeader.NumberOfSections;

			if ( !sections )
				res = RESULT_INVALID;
			else
			{
				PIMAGE_DATA_DIRECTORY pIDD = 0;
				PIMAGE_DEBUG_DIRECTORY pDBG = 0;
				DWORD dbgSize = 0, dbgAddr = 0;    // debug information
				DWORD expSize = 0, expAddr = 0;    // export table
				DWORD resSize = 0, resAddr = 0;    // resource directory
				DWORD relocSize = 0, relocAddr = 0; // relocation table
				PBYTE pRealloc = 0;
				DWORD offset;
				ULONGLONG base = 0;

				// try to found correct offset independent of architectures
				offset = pIDH->e_lfanew + pINTH->FileHeader.SizeOfOptionalHeader + sizeof(IMAGE_NT_HEADERS) - sizeof(IMAGE_OPTIONAL_HEADER);

				if (( machine == IMAGE_FILE_MACHINE_I386 ) &&
					( pINTH->FileHeader.SizeOfOptionalHeader >= sizeof(IMAGE_OPTIONAL_HEADER32)) &&
					( pINTH->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC ))
				{
					pIDD = (PIMAGE_DATA_DIRECTORY)((PBYTE)pINTH + offsetof( IMAGE_NT_HEADERS32, OptionalHeader.DataDirectory ));
					base = *(DWORD*)((PBYTE)pINTH + offsetof( IMAGE_NT_HEADERS32, OptionalHeader.ImageBase ));
				}
				else if (( machine == IMAGE_FILE_MACHINE_AMD64 ) &&
					( pINTH->FileHeader.SizeOfOptionalHeader >= sizeof(IMAGE_OPTIONAL_HEADER64)) &&
					( pINTH->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC ))
				{
					pIDD = (PIMAGE_DATA_DIRECTORY)((PBYTE)pINTH + offsetof( IMAGE_NT_HEADERS64, OptionalHeader.DataDirectory ));
					base = *(ULONGLONG*)((PBYTE)pINTH + offsetof( IMAGE_NT_HEADERS64, OptionalHeader.ImageBase ));
				}
				else res = RESULT_CORRUPTED;

#ifdef DEBUG_REALLOCS
				if ( debug )
					_ftprintf( stderr, _T("Image base is 0x%I64x \n"), base );
#endif
				if ( pIDD ) {
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
				for (DWORD idx=0; idx < sections; idx++)
				{
					PIMAGE_SECTION_HEADER pISH = (PIMAGE_SECTION_HEADER)( ptr + offset + idx * sizeof(IMAGE_SECTION_HEADER));

					if (((PBYTE)pISH + sizeof(IMAGE_SECTION_HEADER) > ptr + filesize ) || ( pISH->PointerToRawData + pISH->SizeOfRawData > filesize ))
					{
						res = RESULT_CORRUPTED;
						break;
					}

					// erase timestamp
					if (( dbgSize >= sizeof( IMAGE_DEBUG_DIRECTORY )) &&
						 ( dbgAddr >= pISH->VirtualAddress ) &&
						 ( dbgAddr + dbgSize <= pISH->VirtualAddress + pISH->SizeOfRawData ))
					{
						DWORD shift = dbgAddr - pISH->VirtualAddress;
						pDBG = (PIMAGE_DEBUG_DIRECTORY)( ptr + shift + pISH->PointerToRawData );
						pDBG->TimeDateStamp = 0;

#ifdef DEBUG_SECTIONS
						if ( debug )
							_ftprintf( stderr, _T("Found debug section entry at 0x%08X (%d), data at 0x%08X (%d)\n"), pISH->PointerToRawData + shift, dbgSize, pDBG->PointerToRawData, pDBG->SizeOfData );
#endif
					}

					// erase export timestamp
					if (( expSize >= sizeof( IMAGE_EXPORT_DIRECTORY )) &&
						 ( expAddr >= pISH->VirtualAddress ) &&
						 ( expAddr + expSize <= pISH->VirtualAddress + pISH->SizeOfRawData ))
					{
						DWORD shift = expAddr - pISH->VirtualAddress;
						PIMAGE_EXPORT_DIRECTORY pEXP = (PIMAGE_EXPORT_DIRECTORY)( ptr + shift + pISH->PointerToRawData );

						pEXP->TimeDateStamp = 0;
#ifdef DEBUG_SECTIONS
						if ( debug )
							_ftprintf( stderr, _T("Found export section entry at 0x%08X\n"), pISH->PointerToRawData + shift );
#endif
					}

					// find realocation table
					if (( relocSize >= sizeof( IMAGE_BASE_RELOCATION )) &&
						 ( relocAddr >= pISH->VirtualAddress ) &&
						 ( relocAddr + relocSize <= pISH->VirtualAddress + pISH->SizeOfRawData ))
					{
						DWORD shift = relocAddr - pISH->VirtualAddress;
						pRealloc = ptr + shift + pISH->PointerToRawData;
#ifdef DEBUG_SECTIONS
						if ( debug )
							_ftprintf( stderr, _T("Found reallocation table entry at 0x%08X (%d)\n"), pISH->PointerToRawData + shift, relocSize );
#endif
					}
				}

				if ( res == RESULT_OK )
				{
					mir_md5_state_t pms;
					mir_md5_init( &pms );

					for (size_t idx=0; idx < sections; idx++)
					{
						PIMAGE_SECTION_HEADER pISH = (PIMAGE_SECTION_HEADER)( ptr + offset + idx * sizeof(IMAGE_SECTION_HEADER));

						if (((PBYTE)pISH + sizeof(IMAGE_SECTION_HEADER) > ptr + filesize ) || ( pISH->PointerToRawData + pISH->SizeOfRawData > filesize ))
						{
							res = RESULT_CORRUPTED;
							break;
						}

						// erase debug information
						if ( pDBG && pDBG->SizeOfData > 0 &&
							  pDBG->PointerToRawData >= pISH->PointerToRawData &&
							  pDBG->PointerToRawData + pDBG->SizeOfData <= pISH->PointerToRawData + pISH->SizeOfRawData )
						{
							ZeroMemory( ptr + pDBG->PointerToRawData, pDBG->SizeOfData );
						}

						// patch resources
						if ( resSize > 0 && resAddr >= pISH->VirtualAddress && resAddr + resSize <= pISH->VirtualAddress + pISH->SizeOfRawData )
						{
							DWORD shift = resAddr - pISH->VirtualAddress + pISH->PointerToRawData;
							IMAGE_RESOURCE_DIRECTORY* pIRD = (IMAGE_RESOURCE_DIRECTORY*)( ptr + shift );
							PatchResourcesDirectory(pIRD, ptr + shift);
						}

						// rebase to zero address
						if ( pRealloc )
						{
							DWORD blocklen = relocSize;
							PWORD pw;
							DWORD type;
							int   len;
							PBYTE pAddr;
							DWORD shift;
							DWORD addr;

							PIMAGE_BASE_RELOCATION pIBR = (PIMAGE_BASE_RELOCATION)pRealloc;
							while( pIBR )
							{
								if (( pIBR->VirtualAddress >= pISH->VirtualAddress ) &&
									 ( pIBR->VirtualAddress < pISH->VirtualAddress + pISH->SizeOfRawData ) &&
									 ( pIBR->SizeOfBlock <= blocklen ))
								{
									shift = pIBR->VirtualAddress - pISH->VirtualAddress + pISH->PointerToRawData;

									len = pIBR->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION);

									pw = (PWORD)((PBYTE)pIBR + sizeof(IMAGE_BASE_RELOCATION));
#ifdef DEBUG_REALLOCS
									if ( debug )
										_ftprintf( stderr, _T("Realloc block at %08X (%d)\n"), pIBR->VirtualAddress, pIBR->SizeOfBlock );
#endif

									while( len > 0 )
									{
										type = *pw >> 12;

										addr = ( *pw & 0x0FFF );

										pAddr = ptr + shift + addr;

										switch( type ) {
										case IMAGE_REL_BASED_HIGHLOW:
											if ( addr + pIBR->VirtualAddress + sizeof(DWORD) >= pISH->VirtualAddress + pISH->SizeOfRawData )
											{
												len = 0;
												break;
											}
#ifdef DEBUG_REALLOCS
											if ( debug && ( *(PDWORD)pAddr < (DWORD)base ))
												_ftprintf( stderr, _T("Realloc address is less than base\n"));
#endif
											*(PDWORD)pAddr = (DWORD)((*(PDWORD)pAddr) - (DWORD)base );
											break;

										case IMAGE_REL_BASED_DIR64:
											if ( addr + pIBR->VirtualAddress + sizeof(ULONGLONG) >= pISH->VirtualAddress + pISH->SizeOfRawData )
											{
												len = 0;
												break;
											}
#ifdef DEBUG_REALLOCS
											if ( debug && ( *(ULONGLONG*)pAddr < base ))
												_ftprintf( stderr, _T("Realloc address is less than base\n"));
#endif
											*(ULONGLONG*)pAddr = (ULONGLONG)((*(ULONGLONG*)pAddr) - base );
											break;

										case IMAGE_REL_BASED_ABSOLUTE:
											// stop processing
											len = 0;
											break;

										case IMAGE_REL_BASED_HIGH:
										case IMAGE_REL_BASED_LOW:
										case IMAGE_REL_BASED_HIGHADJ:
#ifdef DEBUG_REALLOCS
											if ( debug )
												_ftprintf( stderr, _T("Unexpected block type %d\n"), type );
#endif
											break;

										default:
#ifdef DEBUG_REALLOCS
											if ( debug )
												_ftprintf( stderr, _T("Unknown block type %d\n"), type );
#endif
											break;
										}

										len -= sizeof(WORD);
										pw++;
									}
								}

								blocklen -= pIBR->SizeOfBlock;
								if ( blocklen > sizeof(IMAGE_BASE_RELOCATION))
									pIBR = (PIMAGE_BASE_RELOCATION)((PBYTE)pIBR + pIBR->SizeOfBlock );
								else
									break;
							}
						}
#ifdef DEBUG_SECTIONS
						if ( debug )
						{
							int i;
							BYTE digest2[16];
							mir_md5_state_t pms2;

							mir_md5_init( &pms2 );
							mir_md5_append( &pms2, ptr + pISH->PointerToRawData, pISH->SizeOfRawData );
							mir_md5_finish( &pms2, digest2 );

							_ftprintf( stderr, _T("%s - %08X - %d "), pISH->Name, pISH->PointerToRawData, pISH->SizeOfRawData);

							for ( i = 0; i < sizeof( digest2 ) / sizeof( digest2[0] ); i++ )
								_ftprintf( stderr, _T("%02X"), digest2[i] );
							_ftprintf( stderr, _T("\n"));
						}
#endif

						mir_md5_append( &pms, ptr + pISH->PointerToRawData, pISH->SizeOfRawData );
					}

					if ( res == RESULT_OK )
						mir_md5_finish( &pms, digest );
				}
			}
		}
	}
	else res = RESULT_READERROR;

	if ( ptr )
		UnmapViewOfFile( ptr );

	if ( hMap )
		CloseHandle( hMap );

	CloseHandle( hFile );

	return res;
}

TCHAR* trtrim( TCHAR *str )
{
	if ( str == NULL )
		return NULL;

	TCHAR* p = _tcschr( str, 0 );
	while ( --p >= str )
	{
		switch ( *p ) {
		case L' ': case L'\t': case L'\n': case L'\r':
			*p = 0; break;
		default:
			return str;
		}
	}
	return str;
}


int process(TCHAR *filename)
{
	int res;
	BYTE digest[16] = {0};

	res = PEChecksum( filename,  digest);

	switch(res) {
		case RESULT_NOTFOUND:
			_ftprintf( stderr, _T("'%s'... not found!\n"), filename );
			break;
		case RESULT_READERROR:
			_ftprintf( stderr, _T("'%s'... read error!\n"), filename );
			break;
		case RESULT_NOTPE:
			_ftprintf( stderr, _T("'%s'... not PE type!\n"), filename );
			break;
		case RESULT_CORRUPTED:
			_ftprintf( stderr, _T("'%s'... corrupted!\n"), filename );
			break;
		case RESULT_OK:
		{
			int i;
			_ftprintf( stdout, _T("%s "), filename );
			for ( i = 0; i < sizeof( digest ) / sizeof( digest[0] ); i++ )
				_ftprintf( stdout, _T("%02X"), digest[i] );
			_ftprintf( stdout, _T("\n"));
			break;
		}
		default:
			break;
	}
	return res;
}


int _tmain( int argc, TCHAR *argv[] )
{
	TCHAR buf[ MAX_PATH ];
	int res = 0;
	int cnt = 0;
	int i;

	_ftprintf( stderr, _T("* PE CHECKSUM TOOL * VERSION %s * by Bio (c) 2012\n\n"), _VERSION_ );

	if ( argc > 1 )
	{
		WIN32_FIND_DATA ffd;
		HANDLE hFind = INVALID_HANDLE_VALUE;

		for ( i = 1; i < argc; i++ )
		{
			if ( !_tcscmp( argv[i], _T("/debug")) || !_tcscmp( argv[i], _T("/DEBUG")))
			{
				debug = 1;
				break;
			}
		}

		_ftprintf( stderr, _T("Processing ... \n"));

		for ( i = 1; i < argc; i++ )
		{
			if ( !_tcscmp( argv[i], _T("/stdin")) || !_tcscmp( argv[i], _T("/STDIN")))
			{
				while ( _fgetts( buf, sizeof( buf ), stdin ) != NULL )
				{
					trtrim( buf );
					res = process( buf );
					cnt++;
				}
				continue;
			}

			hFind = FindFirstFile( argv[i], &ffd );

			while( hFind != INVALID_HANDLE_VALUE )
			{
				if ( ! ( ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ))
				{
					res = process( ffd.cFileName );
					cnt++;
				}
				if ( !FindNextFile( hFind, &ffd ))
					break;
			}

			FindClose( hFind );
		}

		_ftprintf( stderr, _T("%d file(s) processed.\n"), cnt );
	}
	else
	{
		_ftprintf( stderr, _T("Usage:    checksum.exe [/debug] [/stdin] [*.dll] ... [*.exe]\n"));
		_ftprintf( stderr, _T("Example:  dir /b /s | checksum.exe /stdin > hashes.txt\n"));
		res = RESULT_NONE;
	}

	return res;
}
