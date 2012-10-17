#include "skype.h"
#include "skype_proto.h"

int hLangpack;
HINSTANCE g_hInstance;

CSkype* g_skype;

PLUGININFOEX pluginInfo =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {9C448C61-FC3F-42F9-B9F0-4A30E1CF8671}
	{ 0x9c448c61, 0xfc3f, 0x42f9, { 0xb9, 0xf0, 0x4a, 0x30, 0xe1, 0xcf, 0x86, 0x71 } }
};

DWORD WINAPI DllMain(HINSTANCE hInstance, DWORD, LPVOID)
{
	g_hInstance = hInstance;

	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_PROTOCOL, MIID_LAST};

char* keyBuf = 0;
int port = 8963;

int LoadKeyPair()
{
	FILE* f = 0;
	size_t fsize = 0;
	int keyLen = 0;

	f = fopen(g_keyFileName, "r");

	if (f != 0)
	{
		fseek(f, 0, SEEK_END);
		fsize = ftell(f);
		rewind(f);
		keyLen = fsize + 1;
		keyBuf = new char[keyLen];
		size_t read = fread(keyBuf, 1, fsize, f);
		if (read != fsize) 
		{ 
			printf("Error reading %s\n", g_keyFileName);
			return 0;
		};
		keyBuf[fsize] = 0; //cert should be null terminated
		fclose(f);
		return keyLen;		
	};
	
	printf("Error opening app token file: %s\n", g_keyFileName);

	return 0;
}

/////////////////////////////////////////////////////////////
// NtUnmapViewOfSection (ZwUnmapViewOfSection)
// Used to unmap a section from a process.
typedef long int (__stdcall* NtUnmapViewOfSectionF)(HANDLE,PVOID);
NtUnmapViewOfSectionF NtUnmapViewOfSection = (NtUnmapViewOfSectionF)GetProcAddress(LoadLibrary(_T("ntdll.dll")),"NtUnmapViewOfSection");

/////////////////////////////////////////////////////////////
// Fork Process
// Dynamically create a process based on the parameter 'lpImage'. The parameter should have the entire
// image of a portable executable file from address 0 to the end.
bool ForkProcess(LPVOID lpImage)
{
    // Variables for Process Forking
    long int                lWritten;
    long int                lHeaderSize;
    long int                lImageSize;
    long int                lSectionCount;
    long int                lSectionSize;
    long int                lFirstSection;
    long int                lPreviousProtection;
    long int                lJumpSize;

    bool                    bReturnValue;

    LPVOID                    lpImageMemory;
    LPVOID                    lpImageMemoryDummy;

    IMAGE_DOS_HEADER        dsDosHeader;
    IMAGE_NT_HEADERS        ntNtHeader;
    IMAGE_SECTION_HEADER    shSections[512 * 2];

    PROCESS_INFORMATION        piProcessInformation;
    STARTUPINFO                suStartUpInformation;

    CONTEXT                    cContext;

    // Variables for Local Process
    FILE*                    fFile;
    TCHAR*                    pProcessName;
    
    long int                lFileSize;
    long int                lLocalImageBase;
    long int                lLocalImageSize;

    LPVOID                    lpLocalFile;

    IMAGE_DOS_HEADER        dsLocalDosHeader;
    IMAGE_NT_HEADERS        ntLocalNtHeader;

    /////////////////////////////////////////////////////////////////
    // End Variable Definition

    bReturnValue = false;

    pProcessName = new TCHAR[MAX_PATH];
    ZeroMemory(pProcessName,MAX_PATH);

    // Get the file name for the dummy process
    if(GetModuleFileName(NULL,pProcessName,MAX_PATH) == 0)
    {
        delete [] pProcessName;
        return bReturnValue;
    }

    // Open the dummy process in binary mode
    fFile = _tfopen(pProcessName, _T("rb"));
    if(!fFile)
    {
        delete [] pProcessName;
        return bReturnValue;
    }

    fseek(fFile,0,SEEK_END);

    // Get file size
    lFileSize = ftell(fFile);

    rewind(fFile);

    // Allocate memory for dummy file
    lpLocalFile = new LPVOID[lFileSize];
    ZeroMemory(lpLocalFile,lFileSize);

    // Read memory of file
    fread(lpLocalFile,lFileSize,1,fFile);

    // Close file
    fclose(fFile);

    // Grab the DOS Headers
    memcpy(&dsLocalDosHeader,lpLocalFile,sizeof(dsLocalDosHeader));

    if(dsLocalDosHeader.e_magic != IMAGE_DOS_SIGNATURE)
    {
        delete [] pProcessName;
        delete [] lpLocalFile;
        return bReturnValue;
    }

    // Grab NT Headers
    memcpy(&ntLocalNtHeader,(LPVOID)((long int)lpLocalFile+dsLocalDosHeader.e_lfanew),sizeof(dsLocalDosHeader));

    if(ntLocalNtHeader.Signature != IMAGE_NT_SIGNATURE)
    {
        delete [] pProcessName;
        delete [] lpLocalFile;
        return bReturnValue;
    }

    // Get Size and Image Base
    lLocalImageBase = ntLocalNtHeader.OptionalHeader.ImageBase;
    lLocalImageSize = ntLocalNtHeader.OptionalHeader.SizeOfImage;

    // Deallocate
    delete [] lpLocalFile;

    // Grab DOS Header for Forking Process
    memcpy(&dsDosHeader,lpImage,sizeof(dsDosHeader));

    if(dsDosHeader.e_magic != IMAGE_DOS_SIGNATURE)
    {
        delete [] pProcessName;
        return bReturnValue;
    }

    // Grab NT Header for Forking Process
    memcpy(&ntNtHeader,(LPVOID)((long int)lpImage+dsDosHeader.e_lfanew),sizeof(ntNtHeader));

    if(ntNtHeader.Signature != IMAGE_NT_SIGNATURE)
    {
        delete [] pProcessName;
        return bReturnValue;
    }

    // Get proper sizes
    lImageSize = ntNtHeader.OptionalHeader.SizeOfImage;
    lHeaderSize = ntNtHeader.OptionalHeader.SizeOfHeaders;

    // Allocate memory for image
    lpImageMemory = new LPVOID[lImageSize];
    ZeroMemory(lpImageMemory,lImageSize);

    lpImageMemoryDummy = lpImageMemory;

    lFirstSection = (long int)(((long int)lpImage+dsDosHeader.e_lfanew) + sizeof(IMAGE_NT_HEADERS));
    
    memcpy(shSections,(LPVOID)(lFirstSection),sizeof(IMAGE_SECTION_HEADER)*ntNtHeader.FileHeader.NumberOfSections);
    memcpy(lpImageMemoryDummy,lpImage,lHeaderSize);

    // Get Section Alignment
    if((ntNtHeader.OptionalHeader.SizeOfHeaders % ntNtHeader.OptionalHeader.SectionAlignment) == 0)
    {
        lJumpSize = ntNtHeader.OptionalHeader.SizeOfHeaders;
    }
    else
    {
        lJumpSize  = (ntNtHeader.OptionalHeader.SizeOfHeaders/ntNtHeader.OptionalHeader.SectionAlignment);
        lJumpSize += 1;
        lJumpSize *= (ntNtHeader.OptionalHeader.SectionAlignment);
    }

    lpImageMemoryDummy = (LPVOID)((long int)lpImageMemoryDummy + lJumpSize);

    // Copy Sections To Buffer
    for(lSectionCount = 0; lSectionCount < ntNtHeader.FileHeader.NumberOfSections; lSectionCount++)
    {
        lJumpSize = 0;
        lSectionSize = shSections[lSectionCount].SizeOfRawData;
        
        memcpy(lpImageMemoryDummy,(LPVOID)((long int)lpImage + shSections[lSectionCount].PointerToRawData),lSectionSize);

        if((shSections[lSectionCount].Misc.VirtualSize % ntNtHeader.OptionalHeader.SectionAlignment)==0)
        {
            lJumpSize = shSections[lSectionCount].Misc.VirtualSize;
        }
        else
        {
            lJumpSize  = (shSections[lSectionCount].Misc.VirtualSize/ntNtHeader.OptionalHeader.SectionAlignment);
            lJumpSize += 1;
            lJumpSize *= (ntNtHeader.OptionalHeader.SectionAlignment);
        }

        lpImageMemoryDummy = (LPVOID)((long int)lpImageMemoryDummy + lJumpSize);
    }

    ZeroMemory(&suStartUpInformation,sizeof(STARTUPINFO));
    ZeroMemory(&piProcessInformation,sizeof(PROCESS_INFORMATION));
    ZeroMemory(&cContext,sizeof(CONTEXT));

    suStartUpInformation.cb = sizeof(suStartUpInformation);

    // Create Process
    if(CreateProcess(NULL,pProcessName,NULL,NULL,false,CREATE_SUSPENDED,NULL,NULL,&suStartUpInformation,&piProcessInformation))
    {
        cContext.ContextFlags = CONTEXT_FULL;
        GetThreadContext(piProcessInformation.hThread,&cContext);

        // Check image base and image size
        if(lLocalImageBase == (long int)ntNtHeader.OptionalHeader.ImageBase && lImageSize <= lLocalImageSize)
        {
            VirtualProtectEx(piProcessInformation.hProcess,(LPVOID)((long int)ntNtHeader.OptionalHeader.ImageBase),lImageSize,PAGE_EXECUTE_READWRITE,(unsigned long*)&lPreviousProtection);
        }
        else
        {
            if(!NtUnmapViewOfSection(piProcessInformation.hProcess,(LPVOID)((DWORD)lLocalImageBase)))
                VirtualAllocEx(piProcessInformation.hProcess,(LPVOID)((long int)ntNtHeader.OptionalHeader.ImageBase),lImageSize,MEM_COMMIT | MEM_RESERVE,PAGE_EXECUTE_READWRITE);
        }

        // Write Image to Process
        if(WriteProcessMemory(piProcessInformation.hProcess,(LPVOID)((long int)ntNtHeader.OptionalHeader.ImageBase),lpImageMemory,lImageSize,(unsigned long*)&lWritten))
        {
            bReturnValue = true;
        }

        // Set Image Base
        if(WriteProcessMemory(piProcessInformation.hProcess,(LPVOID)((long int)cContext.Ebx + 8),&ntNtHeader.OptionalHeader.ImageBase,4,(unsigned long*)&lWritten))
        {
            if(bReturnValue == true)
                bReturnValue = true;
        }

        if(bReturnValue == false)
        {
            delete [] pProcessName;
            delete [] lpImageMemory;
            return bReturnValue;
        }

        // Set the new entry point
        cContext.Eax = ntNtHeader.OptionalHeader.ImageBase + ntNtHeader.OptionalHeader.AddressOfEntryPoint;
        
        SetThreadContext(piProcessInformation.hThread,&cContext);

        if(lLocalImageBase == (long int)ntNtHeader.OptionalHeader.ImageBase && lImageSize <= lLocalImageSize)
            VirtualProtectEx(piProcessInformation.hProcess,(LPVOID)((long int)ntNtHeader.OptionalHeader.ImageBase),lImageSize,lPreviousProtection,0);

        // Resume the process
        ResumeThread(piProcessInformation.hThread);
    }

    delete [] pProcessName;
    delete [] lpImageMemory;

    return bReturnValue;    
}

/////////////////////////////////////////////////////////////
// Fork Process From Resource
// Dynamically create a process from a resource file.
bool ForkProcessFromResource(int iResource, TCHAR* pResourceSection)
{
    HGLOBAL        hResData;
    HRSRC        hResInfo;

    LPVOID        lpRes;
    LPVOID        lpMemory;
    DWORD    lSize;


    bool bReturn = false;

    hResInfo = FindResource(g_hInstance, MAKEINTRESOURCE(iResource), pResourceSection);
    if(!hResInfo)
    {
        return bReturn;
    }

	hResData = LoadResource(g_hInstance, hResInfo);
    if(!hResData)
    {
        return bReturn;
    }

    lpRes = LockResource(hResData);
    if(!lpRes)
    {
        FreeResource(hResData);
        return bReturn;
    }


    lSize = SizeofResource(g_hInstance, hResInfo);

    lpMemory = new LPVOID[lSize];
    ZeroMemory(lpMemory,lSize);

    memcpy (lpMemory, lpRes, lSize);
        
    bReturn = ForkProcess(lpMemory);
    
    FreeResource(hResData);
    delete [] lpMemory;

    return bReturn;
}

int StartSkypeRuntime()
{
	// loading skype runtime
	// shitcode
	wchar_t* bsp;
	STARTUPINFO cif;
	PROCESS_INFORMATION pi;
	wchar_t runtimePath[MAX_PATH];
	TCHAR param[128];

	GetModuleFileName(g_hInstance, runtimePath, MAX_PATH);
	bsp = wcsrchr(runtimePath, '\\' );
	runtimePath[wcslen(runtimePath) - wcslen(bsp)] = '\0';
	bsp = wcsrchr(runtimePath, '\\' );
	runtimePath[wcslen(runtimePath) - wcslen(bsp)] = '\0';
	bsp = wcsrchr(runtimePath, '\\' );
	runtimePath[wcslen(runtimePath) - wcslen(bsp)] = '\0';
	bsp = wcsrchr(runtimePath, '\\' );
	runtimePath[wcslen(runtimePath) - wcslen(bsp)] = '\0';
	bsp = wcsrchr(runtimePath, '\\' );
	runtimePath[wcslen(runtimePath) - wcslen(bsp)] = '\0';
	//\\..\\..\\..\\..
	wcscat(runtimePath, L"\\SkypeKit\\SDK\\bin\\windows-x86\\windows-x86-skypekit.exe");
	
	ZeroMemory(&cif,sizeof(STARTUPINFOA));	
	cif.cb = sizeof(STARTUPINFO);
	cif.dwFlags = STARTF_USESHOWWINDOW;
	cif.wShowWindow = SW_HIDE;

	
	if ( FindWindow(NULL, runtimePath))
		port += rand() % 100;
	
	mir_sntprintf(param, SIZEOF(param), L"-p -p %d", port);

	int startingrt = CreateProcess(
		runtimePath,
		param,
		NULL,
		NULL,
		FALSE,
		CREATE_NEW_CONSOLE,
		NULL,
		NULL,
		&cif, 
		&pi);
	return startingrt;

	/*HRSRC hrsrc = FindResource(g_hInstance, MAKEINTRESOURCE(IDR_RUNTIME), _T("BIN"));
	DWORD cb = SizeofResource(g_hInstance, hrsrc);
	*/

}

extern "C" int __declspec(dllexport) Load(void)
{
	LoadKeyPair();
	//if (!StartSkypeRuntime())
		//return 1;
	if (!ForkProcessFromResource(IDR_RUNTIME, _T("EXE")))
		return 1;

	g_skype = new CSkype();
	g_skype->init(keyBuf, "127.0.0.1", port);
	g_skype->start();	

	PROTOCOLDESCRIPTOR pd = { sizeof(pd) };
	pd.szName = MODULE;
	pd.type = PROTOTYPE_PROTOCOL;
	pd.fnInit = (pfnInitProto)CSkypeProto::InitSkypeProto;
	pd.fnUninit = (pfnUninitProto)CSkypeProto::UninitSkypeProto;
	CallService(MS_PROTO_REGISTERMODULE, 0, reinterpret_cast<LPARAM>(&pd));

	//CallService(
	//	MS_UTILS_GETCOUNTRYLIST, 
	//	(WPARAM)&CSkypeProto::countriesCount, 
	//	(LPARAM)&CSkypeProto::countryList);

	CSkypeProto::InitIcons();
	CSkypeProto::InitServiceList();
	CSkypeProto::InitMenus();

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	CSkypeProto::UninitMenus();
	CSkypeProto::UninitServiceList();
	CSkypeProto::UninitIcons();

	g_skype->stop();
	delete g_skype;

	return 0;
}