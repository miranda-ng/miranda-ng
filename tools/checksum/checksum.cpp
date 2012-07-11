
// Checksum Tool 
// By Bio (C) 2012

#define _VERSION_ "2.0"

#include "commonheaders.h"

//#define DEBUG_SECTIONS	1

// Return codes
#define RESULT_OK			0
#define RESULT_NOTFOUND		10
#define RESULT_READERROR	20
#define RESULT_NOTPE		30
#define RESULT_CORRUPTED	40
#define RESULT_INVALID		50
#define RESULT_NONE			100


int PEChecksum( TCHAR *filename, mir_md5_byte_t digest[16] )
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

	if ( hFile == INVALID_HANDLE_VALUE ) {
		return RESULT_NOTFOUND;
	}

	// check minimum and maximum size
	filesize = GetFileSize( hFile, &hsize );

	if ( !filesize || filesize == INVALID_FILE_SIZE || hsize )
	{
		CloseHandle( hFile );
		return RESULT_INVALID;
	}	

	if ( filesize < sizeof(IMAGE_DOS_HEADER) + sizeof(IMAGE_NT_HEADERS) )
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
		if ( (PBYTE)pINTH + sizeof(IMAGE_NT_HEADERS) >= ptr + filesize )
			res = RESULT_CORRUPTED;
		else
		if ( pINTH->Signature != IMAGE_NT_SIGNATURE )
			res = RESULT_NOTPE;
		else
		{
			machine = pINTH->FileHeader.Machine;
/*
			switch(machine) 
			{
				case IMAGE_FILE_MACHINE_I386:
					_ftprintf( stderr, _T("(x86) ") );
					break;
				case IMAGE_FILE_MACHINE_AMD64:
					_ftprintf( stderr, _T("(x64) ") );
					break;
				case IMAGE_FILE_MACHINE_IA64:
					_ftprintf( stderr, _T("(IA64 :-) ") );
						break;
				default:
					_ftprintf( stderr, _T("(unknown) ") );
					break;
			}
*/
			sections = pINTH->FileHeader.NumberOfSections;
			
			if ( !sections )
				res = RESULT_INVALID;
			else 
			{
			    PIMAGE_DATA_DIRECTORY pIDD = 0;
				DWORD dbgsize = 0;
				DWORD dbgvaddr = 0; 
				DWORD expsize = 0;
				DWORD expvaddr = 0; 
			
				if ( ( machine == IMAGE_FILE_MACHINE_I386 ) && 
					( pINTH->FileHeader.SizeOfOptionalHeader >= sizeof(IMAGE_OPTIONAL_HEADER32) ) &&
					( pINTH->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC ) )
				{
				    pIDD = (PIMAGE_DATA_DIRECTORY)( (PBYTE)pINTH + offsetof( IMAGE_NT_HEADERS32, OptionalHeader.DataDirectory ) );
				}
			   	else 
				if ( ( machine == IMAGE_FILE_MACHINE_AMD64 ) && 
					( pINTH->FileHeader.SizeOfOptionalHeader >= sizeof(IMAGE_OPTIONAL_HEADER64) ) &&
					( pINTH->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC ) )
				{

				    pIDD = (PIMAGE_DATA_DIRECTORY)( (PBYTE)pINTH + offsetof( IMAGE_NT_HEADERS64, OptionalHeader.DataDirectory ) );					
				}
				else 
					res = RESULT_CORRUPTED;

				if ( pIDD )
				{	
					// Debugging information entry
					dbgvaddr = pIDD[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;
					dbgsize = pIDD[IMAGE_DIRECTORY_ENTRY_DEBUG].Size;

					// Export information entry
					expvaddr = pIDD[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
					expsize = pIDD[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
				}
			 
			    if ( res == RESULT_OK )
			    {
					PIMAGE_SECTION_HEADER pISH = 0;
		            DWORD offset;
					DWORD idx = 0;
					mir_md5_state_t pms;
								
					mir_md5_init( &pms );

					// try to found correct offset independent of architectures 
					offset = pIDH->e_lfanew + pINTH->FileHeader.SizeOfOptionalHeader + sizeof(IMAGE_NT_HEADERS) - sizeof(IMAGE_OPTIONAL_HEADER);

					while ( idx < sections )
					{
						pISH = (PIMAGE_SECTION_HEADER)( ptr + offset + idx * sizeof(IMAGE_SECTION_HEADER) );

						if ( (PBYTE)pISH + sizeof(IMAGE_SECTION_HEADER) > ptr + filesize )
						{
   							res = RESULT_CORRUPTED;
   							break;
						}

						if ( pISH->PointerToRawData + pISH->SizeOfRawData > filesize )
						{
   							res = RESULT_CORRUPTED;
   							break;
						}

#ifdef DEBUG_SECTIONS
						_ftprintf( stderr, _T("%s - %08X - %d "), pISH->Name, pISH->PointerToRawData, pISH->SizeOfRawData);

						{
							int i;			
							mir_md5_byte_t digest2[16];
							mir_md5_state_t pms2;
								
							mir_md5_init( &pms2 );
							mir_md5_append( &pms2, ptr + pISH->PointerToRawData, pISH->SizeOfRawData );
							mir_md5_finish( &pms2, digest2 );
		
							for ( i = 0; i < sizeof( digest2 ) / sizeof( digest2[0] ); i++ )
								_ftprintf( stderr, _T("%02X"), digest2[i] );
							_ftprintf( stderr, _T("\n") );
						}
#endif
						// erase timestamp and debug information
						if (( dbgsize >= sizeof( IMAGE_DEBUG_DIRECTORY ) ) && 
							( dbgvaddr >= pISH->VirtualAddress ) && 
							( dbgvaddr + dbgsize <= pISH->VirtualAddress + pISH->SizeOfRawData ) )
						{
						    DWORD shift = dbgvaddr - pISH->VirtualAddress;
						    PIMAGE_DEBUG_DIRECTORY pDBG = (PIMAGE_DEBUG_DIRECTORY)( ptr + shift + pISH->PointerToRawData ); 

							pDBG->TimeDateStamp = 0;
#ifdef DEBUG_SECTIONS
							_ftprintf( stderr, _T("found debug section entry %08X ( %d ), data %08X ( %d ) \n"), pISH->PointerToRawData + shift, dbgsize, pDBG->PointerToRawData, pDBG->SizeOfData );
#endif							
							if ( pDBG->SizeOfData > 0 && 
								( pDBG->PointerToRawData >= pISH->PointerToRawData ) && 
								( pDBG->PointerToRawData + pDBG->SizeOfData <= pISH->PointerToRawData + pISH->SizeOfRawData ) )
							{
								ZeroMemory( ptr + pDBG->PointerToRawData, pDBG->SizeOfData );
							}

							//ZeroMemory( ptr + pISH->PointerToRawData + shift, dbgsize );
						}

						// erase export timestamp
						if (( expsize >= sizeof( IMAGE_EXPORT_DIRECTORY ) ) && 
							( expvaddr >= pISH->VirtualAddress ) && 
							( expvaddr + expsize <= pISH->VirtualAddress + pISH->SizeOfRawData ) )
						{
						    DWORD shift = expvaddr - pISH->VirtualAddress;
						    PIMAGE_EXPORT_DIRECTORY pEXP = (PIMAGE_EXPORT_DIRECTORY)( ptr + shift + pISH->PointerToRawData ); 

							pEXP->TimeDateStamp = 0;
#ifdef DEBUG_SECTIONS
							_ftprintf( stderr, _T("found export section entry %08X\n"), pISH->PointerToRawData + shift );
#endif
						}

	                	mir_md5_append( &pms, ptr + pISH->PointerToRawData, pISH->SizeOfRawData );

						idx++;
					}

					if ( res == RESULT_OK ) 
						mir_md5_finish( &pms, digest );
				}
			}
		}
	}
	else
		res = RESULT_READERROR;

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
		switch ( *p ) 
		{
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
	mir_md5_byte_t digest[16] = {0};

	res = PEChecksum( filename,  digest);

	switch(res) 
	{ 
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
			_ftprintf( stdout, _T("\n") );
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

	_ftprintf( stderr, _T("* PE CHECKSUM TOOL * VERSION %s\n\n"), _VERSION_ );

	if ( argc > 1 )
	{
		WIN32_FIND_DATA ffd;
		HANDLE hFind = INVALID_HANDLE_VALUE;			

		_ftprintf( stderr, _T("Processing ... \n") );

		for ( int i=1; i < argc; i++ )
		{
			hFind = FindFirstFile( argv[i], &ffd );	

			while( hFind != INVALID_HANDLE_VALUE ) 
			{
			    if ( ! ( ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
			    {
					res = process( ffd.cFileName );
					cnt++;
				}
				if ( !FindNextFile( hFind, &ffd ) )
					break;
			}

			FindClose( hFind );
		}

   		_ftprintf( stderr, _T("%d file(s) processed.\n"), cnt );
	}
	else
	{
		while ( _fgetts( buf, sizeof( buf ), stdin ) != NULL )
		{
			if ( !cnt )
				_ftprintf( stderr, _T("Processing ... \n") );
			trtrim( buf );
			res = process( buf );
			cnt++;
		}
		
		if ( cnt )
	   		_ftprintf( stderr, _T("%d file(s) processed.\n"), cnt );
   		else
		{
			_ftprintf( stderr, _T("Usage:    checksum.exe [*.dll] ... [*.exe]\n") );
			_ftprintf( stderr, _T("Example:  dir /b /s | checksum.exe > hashes.txt\n") );
			res = RESULT_NONE;
		}
	}

	return res;
}
