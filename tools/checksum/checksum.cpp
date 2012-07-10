
// Checksum Tool 
// Version 1.0
// By Bio (C) 2012

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "../../include/m_core.h"

#define DEBUG_MSGS		1
#define DEBUG_ARCH		1 	
// #define DEBUG_SECTIONS	1


// Return codes
#define RESULT_OK			0
#define RESULT_NOTFOUND		10
#define RESULT_READERROR	20
#define RESULT_NOTPE		30
#define RESULT_CORRUPTED	40
#define RESULT_INVALID		50
#define RESULT_NONE			100


int PEChecksum(TCHAR *filename, mir_md5_byte_t digest[16])
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	HANDLE hMap;
	PBYTE ptr = 0;
    int res = RESULT_OK;
    DWORD size;
    DWORD hsize = 0;

    WORD machine = 0;
	DWORD sections = 0;

	hFile = CreateFile( filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );

	if ( hFile == INVALID_HANDLE_VALUE ) {
		return RESULT_NOTFOUND;
	}

	// check minimum and maximum size
	size = GetFileSize(hFile, &hsize);

	if ( !size || size == INVALID_FILE_SIZE || hsize )
	{
		CloseHandle( hFile );
        return RESULT_INVALID;
	}	

	if ( size < sizeof(IMAGE_DOS_HEADER) + sizeof(IMAGE_NT_HEADERS) )
	{
		CloseHandle( hFile );
        return RESULT_NOTPE;
	}

	hMap = CreateFileMapping( hFile, NULL, PAGE_READONLY, 0, 0, NULL );

	if (hMap)
		ptr = (PBYTE)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0 ,0);

	if (ptr)
	{
		PIMAGE_DOS_HEADER pIDH = 0;
		PIMAGE_NT_HEADERS pINTH = 0;
		PIMAGE_SECTION_HEADER pISH = 0;

		pIDH = (PIMAGE_DOS_HEADER)ptr;

		if ( pIDH->e_magic == IMAGE_DOS_SIGNATURE )
			pINTH = (PIMAGE_NT_HEADERS)( ptr + pIDH->e_lfanew );

		if ( !pINTH)
			res = RESULT_NOTPE;
		else
		if ( (PBYTE)pINTH + sizeof(IMAGE_NT_HEADERS) >= ptr + size )
			res = RESULT_CORRUPTED;
		else
		if ( pINTH->Signature != IMAGE_NT_SIGNATURE )
			res = RESULT_NOTPE;
		else
		{
			machine = pINTH->FileHeader.Machine;

#if defined(DEBUG_ARCH) && defined(DEBUG_MSGS)
				switch(machine) 
				{
					case IMAGE_FILE_MACHINE_I386:
						fprintf( stdout, "(x86) ");
						break;
					case IMAGE_FILE_MACHINE_AMD64:
						fprintf( stdout, "(x64) ");
						break;
					case IMAGE_FILE_MACHINE_IA64:
						fprintf( stdout, "(IA64 :-) ");
							break;
					default:
						fprintf( stdout, "(unknown) ");
						break;
				}
#endif
			sections = pINTH->FileHeader.NumberOfSections;
			
			if ( !sections )
				res = RESULT_INVALID;
			else 
			{
			    DWORD idx = 0;
			    mir_md5_state_t pms;
			    				
				mir_md5_init( &pms );

#if defined(DEBUG_SECTIONS) && defined(DEBUG_MSGS)
				fprintf( stdout, "\n" );
#endif
			    // try to found correct offset independent of architectures 
			    DWORD offset = pIDH->e_lfanew + pINTH->FileHeader.SizeOfOptionalHeader + sizeof(IMAGE_NT_HEADERS) - sizeof(IMAGE_OPTIONAL_HEADER);

			    while ( idx < sections )
			    {
			    	pISH = (PIMAGE_SECTION_HEADER)( ptr + offset + idx * sizeof(IMAGE_SECTION_HEADER) );

		    		if ( (PBYTE)pISH + sizeof(IMAGE_SECTION_HEADER) > ptr + size )
		    		{
   						res = RESULT_CORRUPTED;
   						break;
		    		}

		    		if ( pISH->PointerToRawData + pISH->SizeOfRawData > size )
		    		{
   						res = RESULT_CORRUPTED;
   						break;
		    		}

#if defined(DEBUG_SECTIONS) && defined(DEBUG_MSGS)
					fprintf( stdout, "%s - %d - %d \n", pISH->Name, pISH->PointerToRawData, pISH->SizeOfRawData);
#endif
					mir_md5_append( &pms, ptr + pISH->PointerToRawData, pISH->SizeOfRawData );

				    idx++;
				}

				if ( res == RESULT_OK ) 
					mir_md5_finish( &pms, digest );

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


int main( int argc, char** argv )
{
    mir_md5_byte_t digest[16] = {0};
    int res = 0;

	if ( argc > 1 )
	{
		for ( int i=1; i < argc; i++ )
		{
#ifdef DEBUG_MSGS
			fprintf( stdout, "Processing '%s'... ", argv[ i ] );
#endif
			res = PEChecksum( argv[ i ],  digest);

			switch(res) 
			{ 
#ifdef DEBUG_MSGS
				case RESULT_NOTFOUND:
				{
					fprintf( stdout, "not found!\n");			
					break;
				}
				case RESULT_READERROR:
				{
					fprintf( stdout, "read error!\n");			
					break;
				}
				case RESULT_NOTPE:
				{
					fprintf( stdout, "not PE type!\n");			
					break;
				}
				case RESULT_CORRUPTED:
				{
					fprintf( stdout, "corrupted\n");			
					break;
				}
#endif
				case RESULT_OK:
				{
				    int i;
					for ( i = 0; i < sizeof(digest) / sizeof(digest[0]); i++ )
						fprintf( stdout, "%02X", digest[i] );
				} // no break
				default:
				{
					fprintf( stdout, "\n");			
					break;
				}
			}
		}

	}
	else
	{
		fprintf( stderr, "PE CHECKSUM TOOL v1.0\n\n" );
		fprintf( stderr, "Usage: checksum.exe [filename] ... [filename]\n");
		fprintf( stderr, "Output: MD5 checksum of all sections in PE file.\n");
		return RESULT_NONE;
	}

	return res;
}