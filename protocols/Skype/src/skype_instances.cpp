#include "skype_proto.h"

LIST<CSkypeProto> CSkypeProto::instanceList(1, CSkypeProto::CompareProtos);

int CSkypeProto::CompareProtos(const CSkypeProto *p1, const CSkypeProto *p2)
{
	return wcscmp(p1->m_tszUserName, p2->m_tszUserName);
}

CSkypeProto* CSkypeProto::InitSkypeProto(const char* protoName, const wchar_t* userName)
{
	if (CSkypeProto::instanceList.getCount() > 0)
	{
		CSkypeProto::ShowNotification(
			::TranslateT("SkypeKit will only permit you to login to one account at a time.\nAdding multiple instances of SkypeKit is prohibited in the licence agreement and standard distribution terms."),
			MB_ICONWARNING);
		return NULL;
	}

	CSkypeProto *ppro = new CSkypeProto(protoName, userName);

	VARST profilename( _T("%miranda_profilename%"));

	if ( !ppro->StartSkypeRuntime((TCHAR *)profilename))
	{
		CSkypeProto::ShowNotification(::TranslateT("Did not unpack SkypeKit.exe."));
		return NULL;
	}

	char *keyPair = ppro->LoadKeyPair();
	if ( !keyPair)
	{
		CSkypeProto::ShowNotification(::TranslateT("Initialization key corrupted or not valid."));
		ppro->StopSkypeRuntime();
		return NULL;
	}

	TransportInterface::Status status = ppro->init(keyPair, "127.0.0.1", ppro->skypeKitPort, 0, 1);
	if (status != TransportInterface::OK)
	{
		CSkypeProto::ShowNotification(::TranslateT("SkypeKit did not initialize."));
		ppro->StopSkypeRuntime();
		return NULL;
	}

	if ( !ppro->start())
	{
		::MessageBox(NULL, TranslateT("SkypeKit did not start."), _T(MODULE), MB_OK | MB_ICONERROR);
		ppro->StopSkypeRuntime();
		return NULL;
	}

	::mir_free(keyPair);

	CSkypeProto::instanceList.insert(ppro);

	return ppro;
}

int CSkypeProto::UninitSkypeProto(CSkypeProto* ppro)
{
	ppro->stop();

	ppro->StopSkypeRuntime();

	CSkypeProto::instanceList.remove(ppro);
	delete ppro;

	return 0;
}