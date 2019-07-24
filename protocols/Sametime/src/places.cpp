#include "StdAfx.h"
#include "sametime.h"

/* Stubs, NOT IMPLEMENTED NOW */

void mwServicePlace_opened(struct mwPlace*)
{
}

void mwServicePlace_closed(struct mwPlace*, guint32)
{
}

void mwServicePlace_peerJoined(struct mwPlace*, const struct mwIdBlock*)
{
}


void mwServicePlace_peerParted(struct mwPlace*, const struct mwIdBlock*)
{
}


void mwServicePlace_peerSetAttribute(struct mwPlace*, const struct mwIdBlock*, guint32, struct mwOpaque*)
{
}


void mwServicePlace_peerUnsetAttribute(struct mwPlace*, const struct mwIdBlock*, guint32)
{
}


void mwServicePlace_message(struct mwPlace*, const struct mwIdBlock*, const char*)
{
}


void mwServicePlace_clear(struct mwServicePlace*)
{
}


mwPlaceHandler mwPlace_handler = {
	mwServicePlace_opened,
	mwServicePlace_closed,
	mwServicePlace_peerJoined,
	mwServicePlace_peerParted,
	mwServicePlace_peerSetAttribute,
	mwServicePlace_peerUnsetAttribute,
	mwServicePlace_message,
	mwServicePlace_clear
};

void CSametimeProto::InitPlaces(mwSession* pSession)
{
	debugLogW(L"CSametimeProto::InitPlaces()");
	mwSession_addService(pSession, (mwService*)(service_places = mwServicePlace_new(pSession, &mwPlace_handler)));
}

void CSametimeProto::DeinitPlaces(mwSession* pSession)
{
	debugLogW(L"CSametimeProto::DeinitPlaces()");
	mwSession_removeService(pSession, mwService_PLACE);
	mwService_free((mwService*)service_places);
	service_places = nullptr;
}
