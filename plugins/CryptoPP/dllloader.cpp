/*
* Loading dll from memory
*
* Written by Vitaliy Shitts (vit@shittz.ru)
* Copyright (c) 2004 Vitaliy Shittz.
*
* THIS CODE IS PROVIDED "AS IS". NO WARRANTY OF ANY KIND IS EXPRESSED
* OR IMPLIED. YOU USE AT YOUR OWN RISK. THE AUTHOR ACCEPTS NO LIABILITY 
* IF IT CAUSES ANY DAMAGE TO YOU OR YOUR COMPUTER WHATSOEVER.
*
* Beware of bugs.
*/

#include "commonheaders.h"

#ifdef DLL_FROM_RESOURCE

#ifndef MIN
#    define MIN(a,b) ((a)<(b)?(a):(b))
#endif

typedef BOOL (WINAPI *DLLMAIN)(HINSTANCE,DWORD,LPVOID);

#pragma warning (push)
#pragma warning (disable: 4311; disable: 4312; disable: 4018)

DWORD GetSectionProtection(DWORD sc)
{
    DWORD dwResult=0;
    if (sc & IMAGE_SCN_MEM_NOT_CACHED)
        dwResult |= PAGE_NOCACHE;

    if (sc & IMAGE_SCN_MEM_EXECUTE)
    {
        if (sc & IMAGE_SCN_MEM_READ)
        {
            if (sc & IMAGE_SCN_MEM_WRITE)
                dwResult |= PAGE_EXECUTE_READWRITE;
            else
                dwResult |= PAGE_EXECUTE_READ;
        }
        else 
        {
            if (sc & IMAGE_SCN_MEM_WRITE)
                dwResult |= PAGE_EXECUTE_WRITECOPY;
            else
                dwResult |= PAGE_EXECUTE;
        }
    }
    else
    {
        if (sc & IMAGE_SCN_MEM_READ)
        {
            if (sc & IMAGE_SCN_MEM_WRITE)
                dwResult|=PAGE_READWRITE;
            else
                dwResult|=PAGE_READONLY;
        }
        else 
        {
            if (sc & IMAGE_SCN_MEM_WRITE)
                dwResult|=PAGE_WRITECOPY;
            else
                dwResult|=PAGE_NOACCESS;
        }
    }
    
    return dwResult;
}

inline BOOL IsImportByOrdinal(DWORD ImportDescriptor)
{
    return (ImportDescriptor & IMAGE_ORDINAL_FLAG32)!=0;
}


HMODULE MemLoadLibrary(PBYTE data)
{
    IMAGE_FILE_HEADER        *pFileHeader        = NULL;
    IMAGE_OPTIONAL_HEADER    *pOptionalHeader    = NULL;
    IMAGE_SECTION_HEADER    *pSectionHeader        = NULL;
    IMAGE_IMPORT_DESCRIPTOR *pImportDscrtr        = NULL;
    USHORT                    e_lfanew            = *((USHORT*)(data+0x3c));
    PCHAR                    ImageBase            = NULL;
    PCHAR                    SectionBase            = NULL;

    DWORD dwSize, dwOldProt, ImageBaseDelta;
    int i;

    pFileHeader = (IMAGE_FILE_HEADER *)(data+e_lfanew+4);
    pOptionalHeader = (IMAGE_OPTIONAL_HEADER *)(data+e_lfanew+4+sizeof(IMAGE_FILE_HEADER));
    if (pOptionalHeader->Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC)
        return NULL;

    //    Let's try to reserv memory
    ImageBase = (PCHAR)VirtualAlloc(
        (PVOID)pOptionalHeader->ImageBase,
        pOptionalHeader->SizeOfImage,
        MEM_RESERVE,PAGE_NOACCESS);

    if(ImageBase==NULL)
    {
        ImageBase=(PCHAR)VirtualAlloc(NULL,
            pOptionalHeader->SizeOfImage,
            MEM_RESERVE,PAGE_NOACCESS);
        if(ImageBase==NULL)
            return NULL;
    }

    //    copy the header
    SectionBase=(PCHAR)VirtualAlloc(ImageBase,
        pOptionalHeader->SizeOfHeaders,
        MEM_COMMIT,PAGE_READWRITE);
    memcpy(SectionBase,data,pOptionalHeader->SizeOfHeaders);
    //    Do headers read-only (to be on the safe side)
    VirtualProtect(SectionBase,pOptionalHeader->SizeOfHeaders,PAGE_READONLY,&dwOldProt);

    //    find sections ...
    pSectionHeader = (IMAGE_SECTION_HEADER *)(pOptionalHeader+1);
    for (i=0; i<pFileHeader->NumberOfSections; i++)
    {
        SectionBase = (PCHAR)VirtualAlloc(
            ImageBase+pSectionHeader[i].VirtualAddress,
            pSectionHeader[i].Misc.VirtualSize,
            MEM_COMMIT,PAGE_READWRITE);
        if (SectionBase==NULL)
        {
            VirtualFree(ImageBase, 0, MEM_RELEASE);
            return NULL;
        }
        //    ... and copy initialization data
        SectionBase = ImageBase+pSectionHeader[i].VirtualAddress;
        dwSize = MIN(pSectionHeader[i].SizeOfRawData,pSectionHeader[i].Misc.VirtualSize);
        memcpy(SectionBase, data+pSectionHeader[i].PointerToRawData,dwSize);
    }

    //    check addersses
    ImageBaseDelta = (DWORD)ImageBase-pOptionalHeader->ImageBase;
    if (ImageBaseDelta!=0 && 
        pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress!=0
        )
    {
        IMAGE_BASE_RELOCATION    *pBaseReloc    = (IMAGE_BASE_RELOCATION *)(ImageBase+
            pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
        IMAGE_BASE_RELOCATION    *pBaseReloc0    = pBaseReloc;
        WORD *wPointer = NULL;
        DWORD dwModCount;
        int i;

        while ((DWORD)pBaseReloc0-(DWORD)pBaseReloc < pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size)
        {
            dwModCount = (pBaseReloc0->SizeOfBlock-sizeof(pBaseReloc))/2;
            wPointer = (WORD *)(pBaseReloc+1);
            for (i=0; i<dwModCount; i++, wPointer++)
                if ((*wPointer & 0xf000) !=0)
                {
                    PDWORD pdw = (PDWORD)(ImageBase+pBaseReloc0->VirtualAddress+((*wPointer)&0xfff));
                    (*pdw)+=ImageBaseDelta;
                }
                pBaseReloc = (IMAGE_BASE_RELOCATION *)wPointer;
        }
    }
    else if (ImageBaseDelta!=0)
    {
        VirtualFree(ImageBase, 0, MEM_RELEASE);
        return NULL;
    }

    pImportDscrtr = (IMAGE_IMPORT_DESCRIPTOR *)(ImageBase+
        pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    for (;pImportDscrtr->Name!=0; pImportDscrtr++)
    {
        PCHAR pLibName        = (PCHAR)(ImageBase+pImportDscrtr->Name);
        PCHAR pImortName    = NULL;
        HMODULE hLibModule    = LoadLibrary(pLibName);
        DWORD    *pImport    = NULL,
            *pAddress    = NULL;
        DWORD ProcAddress;

        pAddress=(DWORD *)(ImageBase+pImportDscrtr->/*Original*/FirstThunk);
        if (pImportDscrtr->TimeDateStamp==0)
            pImport=(DWORD *)(ImageBase+pImportDscrtr->FirstThunk);
        else
            pImport=(DWORD *)(ImageBase+pImportDscrtr->OriginalFirstThunk);
        for (i=0; pImport[i]!=0; i++)
        {
            if (IsImportByOrdinal(pImport[i]))
                ProcAddress=(DWORD)GetProcAddress(hLibModule, (PCHAR)(pImport[i]&0xFFFF));
            else  // import by name
            {
                pImortName=(PCHAR)(ImageBase+(pImport[i])+2);
                ProcAddress=(DWORD)GetProcAddress(hLibModule, pImortName);
            }
            pAddress[i]=ProcAddress;
        }
    }

    //    set section protection
    for (i=0; i<pFileHeader->NumberOfSections; i++)
        VirtualProtect((PVOID)(ImageBase+pSectionHeader[i].VirtualAddress),
        pSectionHeader[i].Misc.VirtualSize,
        GetSectionProtection(pSectionHeader[i].Characteristics),
        &dwOldProt);

    //    call DLLMain
    if (pOptionalHeader->AddressOfEntryPoint!=0)
    {
        DLLMAIN dllMain=(DLLMAIN)(ImageBase+pOptionalHeader->AddressOfEntryPoint);
        if (!dllMain((HMODULE)ImageBase, DLL_PROCESS_ATTACH, NULL))
        {
            VirtualFree(ImageBase, 0, MEM_RELEASE);
            return NULL;
        }
    }

    return (HMODULE)ImageBase;
}


BOOL MemFreeLibrary(HMODULE hDll)
{
    PIMAGE_DOS_HEADER        pDosHeader        = NULL;
    PIMAGE_FILE_HEADER        pFileHeader        = NULL;
    PIMAGE_OPTIONAL_HEADER    pOptionalHeader    = NULL;
    
    pDosHeader=(PIMAGE_DOS_HEADER)hDll;
    pFileHeader=(PIMAGE_FILE_HEADER)(((PBYTE)hDll)+pDosHeader->e_lfanew+4);
    pOptionalHeader=(PIMAGE_OPTIONAL_HEADER)(pFileHeader+1);

//    Call to DllMain
    if (pOptionalHeader->AddressOfEntryPoint!=0)
    {
        DLLMAIN dllMain=(DLLMAIN)((PBYTE)hDll+pOptionalHeader->AddressOfEntryPoint);
        dllMain(hDll, DLL_PROCESS_DETACH, NULL);
    }
//    free loaded librares    
    PIMAGE_IMPORT_DESCRIPTOR pImportDscrtr = (IMAGE_IMPORT_DESCRIPTOR *)((PBYTE)hDll+
        pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    for (;pImportDscrtr->Name!=0; pImportDscrtr++)
    {
        PCHAR pLibName = (PCHAR)((PBYTE)hDll+pImportDscrtr->Name);
        HMODULE hLib = GetModuleHandle(pLibName);
        FreeLibrary(hLib);
    }

    return VirtualFree((PVOID)hDll, 0, MEM_RELEASE);
}

FARPROC MemGetProcAddress(HMODULE hDll, LPCTSTR fname)
{
    PIMAGE_DOS_HEADER        pDosHeader        = NULL;
    PIMAGE_FILE_HEADER        pFileHeader        = NULL;
    PIMAGE_OPTIONAL_HEADER    pOptionalHeader    = NULL;

    pDosHeader=(PIMAGE_DOS_HEADER)hDll;
    pFileHeader=(PIMAGE_FILE_HEADER)(((PBYTE)hDll)+pDosHeader->e_lfanew+4);
    pOptionalHeader=(PIMAGE_OPTIONAL_HEADER)(pFileHeader+1);

    DWORD dwExpRVA = pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    PBYTE pb = (PBYTE)hDll;
    PIMAGE_EXPORT_DIRECTORY pExportDir=(PIMAGE_EXPORT_DIRECTORY)(pb+dwExpRVA);
    PDWORD pNamesRVA=(PDWORD)(pb+pExportDir->AddressOfNames);
    PDWORD pFuncRVA=(PDWORD)(pb+pExportDir->AddressOfFunctions);
    PWORD ord=(PWORD)(pb+pExportDir->AddressOfNameOrdinals);

    DWORD dwFunc=pExportDir->NumberOfNames;
    for (int i=0; i<dwFunc; i++)
    {
        PCHAR name =((PCHAR)(pb+pNamesRVA[i]));
        if (0==strcmp(fname, name))
            return (FARPROC)(pb+pFuncRVA[ord[i]]);
    }

    return NULL;
}

#pragma warning (pop)

#endif
