#include "skype_proto.h"

int CSkypeProto::CompareProtos(const CSkypeProto *p1, const CSkypeProto *p2)
{
	return wcscmp(p1->m_tszUserName, p2->m_tszUserName);
}

void CSkypeProto::CreateService(const char* szService, SkypeServiceFunc serviceProc)
{
	char moduleName[MAXMODULELABELLENGTH];

	::mir_snprintf(moduleName, sizeof(moduleName), "%s%s", this->m_szModuleName, szService);
	::CreateServiceFunctionObj(moduleName, (MIRANDASERVICEOBJ)*(void**)&serviceProc, this);
}

void CSkypeProto::CreateServiceParam(const char* szService, SkypeServiceFunc serviceProc, LPARAM lParam)
{
	char moduleName[MAXMODULELABELLENGTH];

	::mir_snprintf(moduleName, sizeof(moduleName), "%s%s", this->m_szModuleName, szService);
	::CreateServiceFunctionObjParam(moduleName, (MIRANDASERVICEOBJPARAM)*(void**)&serviceProc, this, lParam);
}

HANDLE CSkypeProto::CreateEvent(const char* szService)
{
	char moduleName[MAXMODULELABELLENGTH];

	::mir_snprintf(moduleName, sizeof(moduleName), "%s%s", this->m_szModuleName, szService);
	return ::CreateHookableEvent(moduleName);
}

void CSkypeProto::HookEvent(const char* szEvent, SkypeEventFunc handler)
{
	::HookEventObj(szEvent, (MIRANDAHOOKOBJ)*( void**)&handler, this);
}

int CSkypeProto::SendBroadcast(HANDLE hContact, int type, int result, HANDLE hProcess, LPARAM lParam)
{
	ACKDATA ack = {0};
	ack.cbSize = sizeof(ACKDATA);
	ack.szModule = this->m_szModuleName;
	ack.hContact = hContact;
	ack.type = type;
	ack.result = result;
	ack.hProcess = hProcess;
	ack.lParam = lParam;

	return ::CallService(MS_PROTO_BROADCASTACK, 0, (LPARAM)&ack);
}

void CSkypeProto::ForkThread(SkypeThreadFunc pFunc, void *param)
{
	UINT threadID;
	::CloseHandle((HANDLE)::mir_forkthreadowner(
		(pThreadFuncOwner)*(void**)&pFunc, 
		this, 
		param, 
		&threadID));
}

HANDLE CSkypeProto::ForkThreadEx(SkypeThreadFunc pFunc, void *param, UINT* threadID)
{
	UINT lthreadID;
	return (HANDLE)::mir_forkthreadowner(
		(pThreadFuncOwner)*(void**)&pFunc, 
		this,
		param, 
		threadID ? threadID : &lthreadID);
}