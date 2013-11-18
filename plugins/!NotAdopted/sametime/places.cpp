#include "places.h"

mwServicePlace *service_places = 0;

void mwServicePlace_opened(struct mwPlace *place) {
}

void mwServicePlace_closed(struct mwPlace *place, guint32 code) {
}

void mwServicePlace_peerJoined(struct mwPlace *place, const struct mwIdBlock *peer) {
}


void mwServicePlace_peerParted(struct mwPlace *place, const struct mwIdBlock *peer) {
}


void mwServicePlace_peerSetAttribute(struct mwPlace *place, const struct mwIdBlock *peer, guint32 attr, struct mwOpaque *o) {
}


void mwServicePlace_peerUnsetAttribute(struct mwPlace *place, const struct mwIdBlock *peer, guint32 attr) {
}


void mwServicePlace_message(struct mwPlace *place, const struct mwIdBlock *who, const char *msg) {
}


void mwServicePlace_clear(struct mwServicePlace *srvc) {
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

void InitPlaces(mwSession *session) {
	mwSession_addService(session, (mwService *)(service_places = mwServicePlace_new(session, &mwPlace_handler)));
}

void DeinitPlaces(mwSession *session) {
	mwSession_removeService(session, mwService_PLACE);
	mwService_free((mwService *)service_places);
	service_places = 0;
}
