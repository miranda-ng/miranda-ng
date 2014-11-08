/*
Miranda Crash Dumper Plugin
Copyright (C) 2008 - 2012 Boris Krasnovskiy All Rights Reserved

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "dbghelp.h"

typedef struct _IMAGEHLP_MODULE64_V2 {
	DWORD    SizeOfStruct;           // set to sizeof(IMAGEHLP_MODULE64)
	DWORD64  BaseOfImage;            // base load address of module
	DWORD    ImageSize;              // virtual size of the loaded module
	DWORD    TimeDateStamp;          // date/time stamp from pe header
	DWORD    CheckSum;               // checksum from the pe header
	DWORD    NumSyms;                // number of symbols in the symbol table
	SYM_TYPE SymType;                // type of symbols loaded
	CHAR     ModuleName[32];         // module name
	CHAR     ImageName[256];         // image name
	CHAR     LoadedImageName[256];   // symbol file name
} IMAGEHLP_MODULE64_V2;

typedef struct _IMAGEHLP_MODULEW64_V2 {
	DWORD    SizeOfStruct;           // set to sizeof(IMAGEHLP_MODULE64)
	DWORD64  BaseOfImage;            // base load address of module
	DWORD    ImageSize;              // virtual size of the loaded module
	DWORD    TimeDateStamp;          // date/time stamp from pe header
	DWORD    CheckSum;               // checksum from the pe header
	DWORD    NumSyms;                // number of symbols in the symbol table
	SYM_TYPE SymType;                // type of symbols loaded
	WCHAR    ModuleName[32];         // module name
	WCHAR    ImageName[256];         // image name
	WCHAR    LoadedImageName[256];   // symbol file name
} IMAGEHLP_MODULEW64_V2;

#ifdef DBGHELP_TRANSLATE_TCHAR
#define IMAGEHLP_MODULE64_V2 IMAGEHLP_MODULEW64_V2
#endif
