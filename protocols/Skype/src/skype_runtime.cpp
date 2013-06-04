#include "skype_proto.h"

#include "aes\aes.h" 

#include "..\..\..\skypekit\key.h"

char *CSkypeProto::LoadKeyPair()
{
	HRSRC hResource = FindResource(g_hInstance, MAKEINTRESOURCE(IDR_KEY), L"BIN");
	if (hResource == NULL)
		return NULL;

	HGLOBAL hLoadedResource = LoadResource(g_hInstance, hResource);
	if (hLoadedResource == NULL)
		return NULL;

	LPVOID pLockedResource = LockResource(hLoadedResource);
	if (pLockedResource == NULL)
		return NULL;

	aes_context ctx;
	char *key = (char*)mir_base64_decode(MY_KEY, NULL);

	::aes_set_key(&ctx, (unsigned char *)key, 128);
	::mir_free(key);

	int length = ::SizeofResource(g_hInstance, hResource);
	if (length == 0)
		return NULL;

	char *pData = (char *)alloca(length+1);
	memcpy(pData, pLockedResource, length);
	pData[length] = 0;

	unsigned char *bufD = (unsigned char *)::mir_alloc(length * 2);
	unsigned char *tmpD = (unsigned char *)mir_base64_decode(pData, NULL);

	for (int i = 0; i < length; i += 16)
		aes_decrypt(&ctx, tmpD + i, bufD + i);

	::mir_free(tmpD);
	//bufD[length] = 0; //cert should be null terminated
	return (char *)bufD;
}

int CSkypeProto::StartSkypeRuntime(const wchar_t *profileName)
{
	STARTUPINFO cif = {0};
	cif.cb = sizeof(STARTUPINFO);
	cif.dwFlags = STARTF_USESHOWWINDOW;
	cif.wShowWindow = SW_HIDE;	

	wchar_t	fileName[MAX_PATH];
	::GetModuleFileName(g_hInstance, fileName, MAX_PATH);

	wchar_t *skypeKitPath = ::wcsrchr(fileName, '\\');
	if (skypeKitPath != NULL)
		*skypeKitPath = 0;
	::swprintf(fileName, SIZEOF(fileName), L"%s\\%s", fileName, L"SkypeKit.exe");

	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	// todo: rework
	HANDLE snapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (::Process32First(snapshot, &entry) == TRUE)
	{
		while (::Process32Next(snapshot, &entry) == TRUE)
		{
			if (::wcsicmp(entry.szExeFile, L"SkypeKit.exe") == 0 || ::wcsicmp(entry.szExeFile, L"Skype.exe") == 0)
			{
				this->skypeKitPort += rand() % 1000 + 8963;
				break;
			}
		}
	}
	::CloseHandle(snapshot);

	wchar_t param[128];
	VARST dbPath( _T("%miranda_userdata%\\SkypeKit"));
	::swprintf(param, SIZEOF(param), L"-p -P %d -f \"%s\"", this->skypeKitPort, dbPath);
	int startingrt = ::CreateProcess(
		fileName, param,
		NULL, NULL, FALSE,
		CREATE_NEW_CONSOLE,
		NULL, NULL, &cif, &this->skypeKitProcessInfo);

	return startingrt;
}

void CSkypeProto::StopSkypeRuntime()
{
	::PostThreadMessage(this->skypeKitProcessInfo.dwThreadId, WM_CLOSE, 0, 0);
	::WaitForSingleObject(this->skypeKitProcessInfo.hProcess, 1500);

	DWORD dwExitCode = 0;
	::GetExitCodeProcess(this->skypeKitProcessInfo.hProcess, &dwExitCode);
	if (dwExitCode == STILL_ACTIVE)
		::TerminateProcess(this->skypeKitProcessInfo.hProcess, 0); // Zero is the exit code

	::CloseHandle(this->skypeKitProcessInfo.hThread);
	::CloseHandle(this->skypeKitProcessInfo.hProcess);
}