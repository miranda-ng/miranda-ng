/*  
 * Skype Login
 * 
 * Based on:
 *   FakeSkype : Skype reverse engineering proof-of-concept client
 *               Ouanilo MEDEGAN (c) 2006   http://www.oklabs.net
 *   pyskype   : Skype login Python script by uunicorn
 *
 * Written by:   leecher@dose.0wnz.at (c) 2015 
 *
 * Module:       WINDOWS platform specific functions
 *
 */
#include <rpc.h>
#include "common.h"
#include "random.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma comment (lib,"rpcrt4.lib")

static uchar	RandomSeed[SHA_DIGEST_LENGTH] = {0};

static BOOL QueryRegValue(HKEY hKey, LPCSTR lpSubKey, LPBYTE lpValue, LPDWORD pCbValue)
{
	char *pSubKey, *pTok, szKey[256]={0};
	DWORD dwIndex;
	LONG ret;

	if ( !lpSubKey || !*lpSubKey ) return FALSE;
	if ( *lpSubKey != '*' )
	{
		for (pSubKey = (char*)lpSubKey; *pSubKey != '*'; pSubKey = pTok + 1)
			if (!(pTok = strchr(pSubKey, '\\'))) break;
		if ( pSubKey > lpSubKey )
		{
			if ( pSubKey - lpSubKey == 1 ) return FALSE;
			strncpy (szKey, lpSubKey, pSubKey - lpSubKey - 1);
			if (RegOpenKeyA (hKey, szKey, &hKey) == ERROR_SUCCESS)
			{
				ret = QueryRegValue(hKey, pSubKey, lpValue, pCbValue);
				RegCloseKey(hKey);
				return ret;
			}
			return FALSE;
		}
		if ( *lpSubKey != '*' ) return RegQueryValueExA (hKey, lpSubKey, NULL, NULL, lpValue, pCbValue) == ERROR_SUCCESS;
	}
	if (lpSubKey[1] != '\\')
		return RegQueryValueExA (hKey, lpSubKey, NULL, NULL, lpValue, pCbValue) == ERROR_SUCCESS;
	for (dwIndex = 0; (ret = RegEnumKeyA (hKey, dwIndex, szKey, sizeof(szKey))) == ERROR_SUCCESS; dwIndex++)
	{
		char szSubKey[256];

		sprintf (szSubKey, "%s%s", szKey, lpSubKey+1);
		if (QueryRegValue (hKey, szSubKey, lpValue, pCbValue)) break;
	}
	return ret == ERROR_SUCCESS;
}

int64_t PlatFormSpecific()
{
	BYTE		Buffer[0x400];
	DWORD		BufSz = 0x400;
	int			Idx, Used;

	Used = Idx = 0;

	if (QueryRegValue(HKEY_LOCAL_MACHINE, 
		"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ProductId",
		(LPBYTE)Buffer, &BufSz))
		Used += BufSz;
	BufSz = sizeof(Buffer)-Used;
	if (QueryRegValue(HKEY_LOCAL_MACHINE, 
		"HARDWARE\\DESCRIPTION\\System\\MultifunctionAdapter\\*\\DiskController\\*\\DiskPeripheral\\*\\Identifier",
		(LPBYTE)Buffer + Used, &BufSz))
		Used += BufSz;
	if (GetVolumeInformationA("C:\\", 0, 0, (LPDWORD)(Buffer + Used), 0, 0, 0, 0))
		Used+=4;
	return BytesSHA1I64(Buffer, Used);
}

void	 InitNodeId(Skype_Inst *pInst)
{
	DWORD BufSz = sizeof(pInst->NodeID);
	HKEY hKey;

	if (QueryRegValue(HKEY_LOCAL_MACHINE, 
		"SOFTWARE\\FakeSkype\\NodeId",
		(LPBYTE)&pInst->NodeID, &BufSz)) return;
	*(int64_t *)&pInst->NodeID = BytesRandomI64();
	if (RegCreateKeyA(HKEY_LOCAL_MACHINE, "SOFTWARE\\FakeSkype", &hKey) == ERROR_SUCCESS)
	{
		RegSetValueExA(hKey, "NodeId", 0, REG_BINARY, (LPBYTE)&pInst->NodeID, sizeof(pInst->NodeID));
		RegCloseKey(hKey);
	}
	
	//FIXED NODEID
	//memcpy_s(NodeID, NODEID_SZ, "\x49\x63\xff\xee\xe0\x5c\x9d\xf8", NODEID_SZ);
	//memcpy_s(NodeID, NODEID_SZ, "\x97\xca\xb1\x72\x06\xf6\x72\xb4", NODEID_SZ);
}

Memory_U Credentials_Load(char *pszUser)
{
	Memory_U creds={0};
	HKEY hKey;
	char  szKey[MAX_PATH];

	sprintf (szKey, "SOFTWARE\\FakeSkype\\%s", pszUser);
	if (RegCreateKeyA(HKEY_LOCAL_MACHINE, szKey, &hKey) == ERROR_SUCCESS)
	{
		if (RegQueryValueExA(hKey, "Credentials", NULL, NULL, NULL, &creds.MsZ) == ERROR_SUCCESS &&
			creds.MsZ)
		{
			if (!(creds.Memory = malloc(creds.MsZ)))
				creds.MsZ = 0;
			if (creds.Memory && RegQueryValueExA(hKey, "Credentials", NULL, NULL, creds.Memory, &creds.MsZ) != ERROR_SUCCESS)
			{
				free(creds.Memory);
				ZeroMemory(&creds, sizeof(creds));
			}
		}
		RegCloseKey(hKey);
	}
	return creds;
}

int Credentials_Save(Memory_U creds, char *pszUser)
{
	HKEY hKey;
	int iRet = 0;

	char  szKey[MAX_PATH];

	sprintf (szKey, "SOFTWARE\\FakeSkype\\%s", pszUser);
	if (RegCreateKeyA(HKEY_LOCAL_MACHINE, szKey, &hKey) == ERROR_SUCCESS)
	{
		iRet = RegSetValueExA(hKey, "Credentials", 0, REG_BINARY, creds.Memory, creds.MsZ) == ERROR_SUCCESS;
		RegCloseKey(hKey);
	}
	return iRet;
}

void FillMiscDatas(Skype_Inst *pInst, unsigned int *Datas)
{
	BYTE		Buffer[0x400];
	DWORD		BufSz = 0x400;
	int			ret;
	int64_t PlatForm;

	PlatForm = PlatFormSpecific();
	Datas[0] = *(unsigned int *)&PlatForm;
	Datas[1] = *(unsigned int *)&pInst->NodeID;

	if (!QueryRegValue(HKEY_LOCAL_MACHINE, 
		"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ProductId",
		(LPBYTE)Buffer, &BufSz))
		return;
	Datas[2] = BytesSHA1(Buffer, BufSz);

	BufSz = 0x400;
	if (!QueryRegValue(HKEY_LOCAL_MACHINE, 
		"HARDWARE\\DESCRIPTION\\System\\MultifunctionAdapter\\*\\DiskController\\*\\DiskPeripheral\\*\\Identifier",
		(LPBYTE)Buffer, &BufSz))
		return;
	Datas[3] = BytesSHA1(Buffer, BufSz);

	ret = GetVolumeInformationA("C:\\", 0, 0, (LPDWORD)Buffer, 0, 0, 0, 0);
	Datas[4] = BytesSHA1(Buffer, 0x04);
}

void FillRndBuffer(unsigned char *Buffer)
{
	SHA_CTX		Context;
	int			idx;

	idx = 0;
	memcpy(Buffer, RandomSeed, SHA_DIGEST_LENGTH);
	idx += sizeof(RandomSeed);
	GlobalMemoryStatus((LPMEMORYSTATUS)&Buffer[idx]);
	idx += sizeof(MEMORYSTATUS);
	UuidCreate((UUID *)&Buffer[idx]);
	idx += sizeof(UUID);
	GetCursorPos((LPPOINT)&Buffer[idx]);
	idx += sizeof(POINT);
	*(DWORD *)(Buffer + idx) = GetTickCount();
	*(DWORD *)(Buffer + idx + 4) = GetMessageTime();
	*(DWORD *)(Buffer + idx + 8) = GetCurrentThreadId();
	*(DWORD *)(Buffer + idx + 12) = GetCurrentProcessId();
	idx += 16;
	QueryPerformanceCounter((LARGE_INTEGER *)&Buffer[idx]);
	SHA1_Init(&Context);
	SHA1_Update(&Context, Buffer, 0x464);
	SHA1_Update(&Context, "additional salt...", 0x13);
	SHA1_Final(RandomSeed, &Context);
}
