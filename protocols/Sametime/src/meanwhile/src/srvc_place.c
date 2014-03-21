
/*
  Meanwhile - Unofficial Lotus Sametime Community Client Library
  Copyright (C) 2004  Christopher (siege) O'Brien
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.
  
  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <glib.h>
#include <glib/ghash.h>
#include <glib/glist.h>

#include <stdio.h>
#include <stdlib.h>

#include "mw_channel.h"
#include "mw_common.h"
#include "mw_debug.h"
#include "mw_error.h"
#include "mw_message.h"
#include "mw_service.h"
#include "mw_session.h"
#include "mw_srvc_place.h"
#include "mw_util.h"


#define PROTOCOL_TYPE  0x00
#define PROTOCOL_VER   0x05


enum incoming_msg {
  msg_in_JOIN_RESPONSE  = 0x0000,  /* ? */
  msg_in_INFO           = 0x0002,
  msg_in_MESSAGE        = 0x0004,
  msg_in_SECTION        = 0x0014,  /* see in_section_subtype */
  msg_in_UNKNOWNa       = 0x0015,
};


enum in_section_subtype {
  msg_in_SECTION_LIST  = 0x0000,  /* list of section members */
  msg_in_SECTION_PEER  = 0x0001,  /* see in_section_peer_subtye */
  msg_in_SECTION_PART  = 0x0003,
};


enum in_section_peer_subtype {
  msg_in_SECTION_PEER_JOIN        = 0x0000,
  msg_in_SECTION_PEER_PART        = 0x0001,  /* after msg_in_SECTION_PART */
  msg_in_SECTION_PEER_CLEAR_ATTR  = 0x0003,
  msg_in_SECTION_PEER_SET_ATTR    = 0x0004,
};


enum outgoing_msg {
  msg_out_JOIN_PLACE  = 0x0000,  /* ? */
  msg_out_PEER_INFO   = 0x0002,  /* ? */
  msg_out_MESSAGE     = 0x0003,
  msg_out_OLD_INVITE  = 0x0005,  /* old-style conf. invitation */
  msg_out_SET_ATTR    = 0x000a,
  msg_out_CLEAR_ATTR  = 0x000b,
  msg_out_SECTION     = 0x0014,  /* see out_section_subtype */
  msg_out_UNKNOWNb    = 0x001e,  /* ? maybe enter stage ? */
};


enum out_section_subtype {
  msg_out_SECTION_LIST  = 0x0002,  /* req list of members */
  msg_out_SECTION_PART  = 0x0003,
};


/*
  : allocate section
  : state = NEW

  : create channel
  : state = PENDING

  : channel accepted
  : msg_out_JOIN_PLACE  (maybe create?)
  : state = JOINING

  : msg_in_JOIN_RESPONSE (contains our place member ID and section ID)
  : msg_in_INFO (for place, not peer)
  : state = JOINED

  : msg_out_SECTION_LIST (asking for all sections) (optional)
  : msg_in_SECTION_LIST (listing all sections, as requested above)

  : msg_out_PEER_INFO (with our place member ID) (optional)
  : msg_in_INFO (peer info as requested above)

  : msg_out_SECTION_LIST (with our section ID) (sorta optional)
  : msg_in_SECTION_LIST (section listing as requested above)

  : msg_out_UNKNOWNb
  : msg_in_SECTION_PEER_JOINED (empty, with our place member ID)
  : state = OPEN

  : stuff... (invites, joins, parts, messages, attr)

  : state = CLOSING
  : msg_out_SECTION_PART
  : destroy channel
  : deallocate section
*/


struct mwServicePlace {
  struct mwService service;
  struct mwPlaceHandler *handler;
  GList *places;
};


enum mwPlaceState {
  mwPlace_NEW,
  mwPlace_PENDING,
  mwPlace_JOINING,
  mwPlace_JOINED,
  mwPlace_OPEN,
  mwPlace_CLOSING,
  mwPlace_ERROR,
  mwPlace_UNKNOWN,
};


struct mwPlace {
  struct mwServicePlace *service;

  enum mwPlaceState state;
  struct mwChannel *channel;

  char *name;
  char *title;
  GHashTable *members;  /* mapping of member ID: place_member */
  guint32 our_id;       /* our member ID */
  guint32 section;      /* the section we're using */

  guint32 requests;     /* counter for requests */

  struct mw_datum client_data;
};


struct place_member {
  guint32 place_id;
  guint16 member_type;
  struct mwIdBlock idb;
  char *login_id;
  char *name;
  guint16 login_type;
  guint32 unknown_a;
  guint32 unknown_b;
};


#define GET_MEMBER(place, id) \
  (g_hash_table_lookup(place->members, GUINT_TO_POINTER(id)))


#define PUT_MEMBER(place, member) \
  (g_hash_table_insert(place->members, \
                       GUINT_TO_POINTER(member->place_id), member))


#define REMOVE_MEMBER_ID(place, id) \
  (g_hash_table_remove(place->members, GUINT_TO_POINTER(id)))


#define REMOVE_MEMBER(place, member) \
  REMOVE_MEMBER_ID(place, member->place_id)


static void member_free(struct place_member *p) {
  mwIdBlock_clear(&p->idb);
  g_free(p->login_id);
  g_free(p->name);
  g_free(p);
}

/// Miranda NG adaptation start - MSVC
/// __attribute__((used))
/// Miranda NG adaptation end
static const struct mwLoginInfo *
member_as_login_info(struct place_member *p) {
  static struct mwLoginInfo li;
  
  li.login_id = p->login_id;
  li.type = p->login_type;
  li.user_id = p->idb.user;
  li.user_name = p->name;
  li.community = p->idb.community;
  li.full = FALSE;

  return &li;
}


static const char *place_state_str(enum mwPlaceState s) {
  switch(s) {
  case mwPlace_NEW:      return "new";
  case mwPlace_PENDING:  return "pending";
  case mwPlace_JOINING:  return "joining";
  case mwPlace_JOINED:   return "joined";
  case mwPlace_OPEN:     return "open";
  case mwPlace_CLOSING:  return "closing";
  case mwPlace_ERROR:    return "error";

  case mwPlace_UNKNOWN:  /* fall-through */
  default:               return "UNKNOWN";
  }
}


static void place_state(struct mwPlace *place, enum mwPlaceState s) {
  g_return_if_fail(place != NULL);
  
  if(place->state == s) return;

  place->state = s;
  g_message("place %s state: %s", NSTR(place->name), place_state_str(s));
}


static void place_free(struct mwPlace *place) {
  struct mwServicePlace *srvc;

  if(! place) return;
  
  srvc = place->service;
  g_return_if_fail(srvc != NULL);

  srvc->places = g_list_remove_all(srvc->places, place);

  mw_datum_clear(&place->client_data);

  g_hash_table_destroy(place->members);

  g_free(place->name);
  g_free(place->title);
  g_free(place);
}


static int recv_JOIN_RESPONSE(struct mwPlace *place,
			      struct mwGetBuffer *b) {
  
  int ret = 0;
  guint32 our_id, section;

  guint32_get(b, &our_id);
  guint32_get(b, &section);

  place->our_id = our_id;
  place->section = section;

  return ret;
}


static int send_SECTION_LIST(struct mwPlace *place, guint32 section) {
  int ret = 0;
  struct mwOpaque o = {0, 0};
  struct mwPutBuffer *b;

  b = mwPutBuffer_new();
  guint16_put(b, msg_out_SECTION_LIST);
  guint32_put(b, section);
  gboolean_put(b, FALSE);
  guint32_put(b, ++place->requests);
  mwPutBuffer_finalize(&o, b);

  ret = mwChannel_send(place->channel, msg_out_SECTION, &o);
  mwOpaque_clear(&o);

  return ret;
}


static int recv_INFO(struct mwPlace *place,
		     struct mwGetBuffer *b) {

  int ret = 0;
  guint32 skip = 0;
  guint32 section = 0;

  guint32_get(b, &skip);
  guint32_get(b, &section);
  mwGetBuffer_advance(b, skip);

  if(! section) {
    /* this is a place info rather than member info */
    if(place->title) g_free(place->title);
    mwGetBuffer_advance(b, 2);
    mwString_get(b, &place->title);

    place_state(place, mwPlace_JOINED);
    ret = send_SECTION_LIST(place, place->section);
  }

  return ret;
}


static int recv_MESSAGE(struct mwPlace *place,
			struct mwGetBuffer *b) {

  struct mwServicePlace *srvc;
  guint32 pm_id;
  guint32 unkn_a, unkn_b, ign;
  struct place_member *pm;
  char *msg = NULL;
  int ret = 0;

  srvc = place->service;

  /* no messages before becoming fully open, please */
  g_return_val_if_fail(place->state == mwPlace_OPEN, -1);

  /* regarding unkn_a and unkn_b:

     they're probably a section indicator and a message count, I'm
     just not sure which is which. Until this implementation supports
     place sections in the API, it really doesn't matter. */
  
  guint32_get(b, &pm_id);
  pm = GET_MEMBER(place, pm_id);
  g_return_val_if_fail(pm != NULL, -1);

  guint32_get(b, &unkn_a);
  guint32_get(b, &ign);     /* actually an opaque length */
  
  if(! ign) return ret;

  guint32_get(b, &unkn_b);
  mwString_get(b, &msg);

  if(srvc->handler && srvc->handler->message)
    srvc->handler->message(place, &pm->idb, msg);

  g_free(msg);

  return ret;
}


static void place_opened(struct mwPlace *place) {
    struct mwServicePlace *srvc;

    place_state(place, mwPlace_OPEN);

    srvc = place->service;
    if(srvc->handler && srvc->handler->opened)
      srvc->handler->opened(place);
}


static int recv_SECTION_PEER_JOIN(struct mwPlace *place,
				  struct mwGetBuffer *b) {
  struct mwServicePlace *srvc;
  struct place_member *pm;
  guint32 section;
  int ret = 0;

  srvc = place->service;

  guint32_get(b, &section);
  if(! section) {
    /// Miranda NG adaptation start - MSVC
    ///g_info("SECTION_PEER_JOIN with section 0x00");
    g_message("SECTION_PEER_JOIN with section 0x00");
    /// Miranda NG adaptation end
    return 0;
  }

  mwGetBuffer_advance(b, 4);

  pm = g_new0(struct place_member, 1);
  guint32_get(b, &pm->place_id);
  guint16_get(b, &pm->member_type);
  mwIdBlock_get(b, &pm->idb);
  mwString_get(b, &pm->login_id);
  mwString_get(b, &pm->name);
  guint16_get(b, &pm->login_type);
  guint32_get(b, &pm->unknown_a);
  guint32_get(b, &pm->unknown_b);

  PUT_MEMBER(place, pm);
  if(srvc->handler && srvc->handler->peerJoined)
    srvc->handler->peerJoined(place, &pm->idb);

  if(pm->place_id == place->our_id)
    place_opened(place);

  return ret;
}


static int recv_SECTION_PEER_PART(struct mwPlace *place,
				  struct mwGetBuffer *b) {
  struct mwServicePlace *srvc;
  int ret = 0;
  guint32 section, id;
  struct place_member *pm;

  srvc = place->service;

  guint32_get(b, &section);
  g_return_val_if_fail(section == place->section, 0);

  guint32_get(b, &id);
  pm = GET_MEMBER(place, id);

  /* SECTION_PART may have been called already */
  if(! pm) return 0;

  if(srvc->handler && srvc->handler->peerParted)
    srvc->handler->peerParted(place, &pm->idb);

  REMOVE_MEMBER(place, pm);

  return ret;
}


static int recv_SECTION_PEER_CLEAR_ATTR(struct mwPlace *place,
					struct mwGetBuffer *b) {
  struct mwServicePlace *srvc;
  int ret = 0;
  guint32 id, attr;
  struct place_member *pm;
  
  srvc = place->service;

  guint32_get(b, &id);
  guint32_get(b, &attr);

  pm = GET_MEMBER(place, id);
  g_return_val_if_fail(pm != NULL, -1);

  if(srvc->handler && srvc->handler->peerUnsetAttribute)
    srvc->handler->peerUnsetAttribute(place, &pm->idb, attr);

  return ret;
}


static int recv_SECTION_PEER_SET_ATTR(struct mwPlace *place,
				      struct mwGetBuffer *b) {
  struct mwServicePlace *srvc;
  int ret = 0;
  guint32 id, attr;
  struct mwOpaque o = {0,0};
  struct place_member *pm;
  
  srvc = place->service;

  guint32_get(b, &id);
  mwGetBuffer_advance(b, 4);
  mwOpaque_get(b, &o);
  mwGetBuffer_advance(b, 4);
  guint32_get(b, &attr);

  pm = GET_MEMBER(place, id);
  g_return_val_if_fail(pm != NULL, -1);

  if(srvc->handler && srvc->handler->peerSetAttribute)
    srvc->handler->peerSetAttribute(place, &pm->idb, attr, &o);

  mwOpaque_clear(&o);

  return ret;
}


static int recv_SECTION_PEER(struct mwPlace *place,
			      struct mwGetBuffer *b) {
  guint16 subtype;
  int res;

  guint16_get(b, &subtype);

  g_return_val_if_fail(! mwGetBuffer_error(b), -1);

  switch(subtype) {
  case msg_in_SECTION_PEER_JOIN:
    res = recv_SECTION_PEER_JOIN(place, b);
    break;

  case msg_in_SECTION_PEER_PART:
    res = recv_SECTION_PEER_PART(place, b);
    break;

  case msg_in_SECTION_PEER_CLEAR_ATTR:
    res = recv_SECTION_PEER_CLEAR_ATTR(place, b);
    break;

  case msg_in_SECTION_PEER_SET_ATTR:
    res = recv_SECTION_PEER_SET_ATTR(place, b);
    break;

  default:
    res = -1;
  }

  return res;
}


static int recv_SECTION_LIST(struct mwPlace *place,
			     struct mwGetBuffer *b) {
  int ret = 0;
  guint32 sec, count;

  mwGetBuffer_advance(b, 4);
  guint32_get(b, &sec);

  g_return_val_if_fail(sec == place->section, -1);

  mwGetBuffer_advance(b, 8);
  guint32_get(b, &count);
  mwGetBuffer_advance(b, 8);

  while(count--) {
    struct place_member *m;

    m = g_new0(struct place_member, 1);
    mwGetBuffer_advance(b, 4);
    guint32_get(b, &m->place_id);
    guint16_get(b, &m->member_type);
    mwIdBlock_get(b, &m->idb);
    mwString_get(b, &m->login_id);
    mwString_get(b, &m->name);
    guint16_get(b, &m->login_type);
    guint32_get(b, &m->unknown_a);
    guint32_get(b, &m->unknown_b);

    PUT_MEMBER(place, m);
  }

  if(place->state != mwPlace_OPEN)
    place_opened(place);

  return ret;
}


static int recv_SECTION_PART(struct mwPlace *place,
			     struct mwGetBuffer *b) {
  /* look up user in place
     remove user from place
     trigger event */

  struct mwServicePlace *srvc;
  guint32 pm_id;
  struct place_member *pm;

  srvc = place->service;

  guint32_get(b, &pm_id);
  pm = GET_MEMBER(place, pm_id);

  /* SECTION_PEER_PART may have been called already */
  if(! pm) return 0;

  if(srvc->handler && srvc->handler->peerParted)
    srvc->handler->peerParted(place, &pm->idb);

  REMOVE_MEMBER(place, pm);

  return 0;
}


static int recv_SECTION(struct mwPlace *place, struct mwGetBuffer *b) {
  guint16 subtype;
  int res;

  guint16_get(b, &subtype);

  g_return_val_if_fail(! mwGetBuffer_error(b), -1);

  switch(subtype) {
  case msg_in_SECTION_LIST:
    res = recv_SECTION_LIST(place, b);
    break;

  case msg_in_SECTION_PEER:
    res = recv_SECTION_PEER(place, b);
    break;

  case msg_in_SECTION_PART:
    res = recv_SECTION_PART(place, b);
    break;

  default:
    res = -1;
  }

  return res;
}


static int recv_UNKNOWNa(struct mwPlace *place, struct mwGetBuffer *b) {
  int res = 0;

  if(place->state == mwPlace_JOINING) {
    ;
    /* place_state(place, mwPlace_JOINED);
       res = send_SECTION_LIST(place, place->section); */
  
  } else if(place->state == mwPlace_JOINED) {
    ;
    /* if(GET_MEMBER(place, place->our_id))
       place_opened(place); */
  }

  return res;
}


static void recv(struct mwService *service, struct mwChannel *chan,
		 guint16 type, struct mwOpaque *data) {

  struct mwPlace *place;
  struct mwGetBuffer *b;
  int res = 0;

  place = mwChannel_getServiceData(chan);
  g_return_if_fail(place != NULL);

  b = mwGetBuffer_wrap(data);
  switch(type) {
  case msg_in_JOIN_RESPONSE:
    res = recv_JOIN_RESPONSE(place, b);
    break;

  case msg_in_INFO:
    res = recv_INFO(place, b);
    break;

  case msg_in_MESSAGE:
    res = recv_MESSAGE(place, b);
    break;

  case msg_in_SECTION:
    res = recv_SECTION(place, b);
    break;

  case msg_in_UNKNOWNa:
    res = recv_UNKNOWNa(place, b);
    break;

  default:
    mw_mailme_opaque(data, "Received unknown message type 0x%x on place %s",
		     type, NSTR(place->name));
  }

  if(res) {
    mw_mailme_opaque(data, "Troubling parsing message type 0x0%x on place %s",
		     type, NSTR(place->name));
  }

  mwGetBuffer_free(b);
}


static void stop(struct mwServicePlace *srvc) {
  while(srvc->places)
    mwPlace_destroy(srvc->places->data, ERR_SUCCESS);

  mwService_stopped(MW_SERVICE(srvc));
}


static int send_JOIN_PLACE(struct mwPlace *place) {
  struct mwOpaque o = {0, 0};
  struct mwPutBuffer *b;
  int ret;

  b = mwPutBuffer_new();
  gboolean_put(b, FALSE);
  guint16_put(b, 0x01);
  guint16_put(b, 0x02); /* 0x01 */
  guint16_put(b, 0x01); /* 0x00 */

  mwPutBuffer_finalize(&o, b);

  ret = mwChannel_send(place->channel, msg_out_JOIN_PLACE, &o);

  mwOpaque_clear(&o);

  if(ret) {
    place_state(place, mwPlace_ERROR);
  } else {
    place_state(place, mwPlace_JOINING);
  }

  return ret;
}


static void recv_channelAccept(struct mwService *service,
			       struct mwChannel *chan,
			       struct mwMsgChannelAccept *msg) {
  struct mwServicePlace *srvc;
  struct mwPlace *place;
  int res;

  srvc = (struct mwServicePlace *) service;
  g_return_if_fail(srvc != NULL);

  place = mwChannel_getServiceData(chan);
  g_return_if_fail(place != NULL);

  res = send_JOIN_PLACE(place);
}


static void recv_channelDestroy(struct mwService *service,
				struct mwChannel *chan,
				struct mwMsgChannelDestroy *msg) {
  struct mwServicePlace *srvc;
  struct mwPlace *place;

  srvc = (struct mwServicePlace *) service;
  g_return_if_fail(srvc != NULL);

  place = mwChannel_getServiceData(chan);
  g_return_if_fail(place != NULL);

  place_state(place, mwPlace_ERROR);

  place->channel = NULL;

  if(srvc->handler && srvc->handler->closed)
    srvc->handler->closed(place, msg->reason);  

  mwPlace_destroy(place, msg->reason);
}


static void clear(struct mwServicePlace *srvc) {

  if(srvc->handler && srvc->handler->clear)
    srvc->handler->clear(srvc);

  while(srvc->places)
    place_free(srvc->places->data);
}


static const char *get_name(struct mwService *srvc) {
  return "Places Conferencing";
}


static const char *get_desc(struct mwService *srvc) {
  return "Barebones conferencing via Places";
}


struct mwServicePlace *
mwServicePlace_new(struct mwSession *session,
		   struct mwPlaceHandler *handler) {

  struct mwServicePlace *srvc_place;
  struct mwService *srvc;

  g_return_val_if_fail(session != NULL, NULL);
  g_return_val_if_fail(handler != NULL, NULL);

  srvc_place = g_new0(struct mwServicePlace, 1);
  srvc_place->handler = handler;

  srvc = MW_SERVICE(srvc_place);
  mwService_init(srvc, session, mwService_PLACE);
  srvc->start = NULL;
  srvc->stop = (mwService_funcStop) stop;
  srvc->recv_create = NULL;
  srvc->recv_accept = recv_channelAccept;
  srvc->recv_destroy = recv_channelDestroy;
  srvc->recv = recv;
  srvc->clear = (mwService_funcClear) clear;
  srvc->get_name = get_name;
  srvc->get_desc = get_desc;

  return srvc_place;
}


struct mwPlaceHandler *
mwServicePlace_getHandler(struct mwServicePlace *srvc) {
  g_return_val_if_fail(srvc != NULL, NULL);
  return srvc->handler;
}


const GList *mwServicePlace_getPlaces(struct mwServicePlace *srvc) {
  g_return_val_if_fail(srvc != NULL, NULL);
  return srvc->places;
}


struct mwPlace *mwPlace_new(struct mwServicePlace *srvc,
			    const char *name, const char *title) {
  struct mwPlace *place;

  g_return_val_if_fail(srvc != NULL, NULL);
  
  place = g_new0(struct mwPlace, 1);
  place->service = srvc;
  place->name = g_strdup(name);
  place->title = g_strdup(title);
  place->state = mwPlace_NEW;

  place->members = g_hash_table_new_full(g_direct_hash, g_direct_equal,
					 NULL, (GDestroyNotify) member_free);

  srvc->places = g_list_prepend(srvc->places, place);
  
  return place;
}


struct mwServicePlace *mwPlace_getService(struct mwPlace *place) {
  g_return_val_if_fail(place != NULL, NULL);
  return place->service;
}


static char *place_generate_name(const char *user) {
  /// Miranda NG adaptation start - MSVC
  ///guint a, b;
  guint a;
  guint64 b;
  /// Miranda NG adaptation end
  char *ret;
  
  user = user? user: "meanwhile";

  srand(clock() + rand());
  a = ((rand() & 0xff) << 8) | (rand() & 0xff);
  b = time(NULL);

  /// Miranda NG adaptation start - MSVC
  ///ret = g_strdup_printf("%s(%08x,%04x)", user, b, a);
  ret = g_strdup_printf("%s(%I64u,%04x)", user, b, a);
  /// Miranda NG adaptation end
  g_debug("generated random conference name: '%s'", ret);
  return ret;
}


const char *mwPlace_getName(struct mwPlace *place) {
  g_return_val_if_fail(place != NULL, NULL);

  if(! place->name) {
    struct mwSession *session;
    struct mwLoginInfo *li;

    session = mwService_getSession(MW_SERVICE(place->service));
    li = mwSession_getLoginInfo(session);

    place->name = place_generate_name(li? li->user_id: NULL);
  }

  return place->name;
}


static char *place_generate_title(const char *user) {
  char *ret;
  
  user = user? user: "Meanwhile";
  ret = g_strdup_printf("%s's Conference", user);
  g_debug("generated conference title: %s", ret);

  return ret;
}


const char *mwPlace_getTitle(struct mwPlace *place) {
  g_return_val_if_fail(place != NULL, NULL);

  if(! place->title) {
    struct mwSession *session;
    struct mwLoginInfo *li;

    session = mwService_getSession(MW_SERVICE(place->service));
    li = mwSession_getLoginInfo(session);

    place->title = place_generate_title(li? li->user_name: NULL);
  }

  return place->title;
}


int mwPlace_open(struct mwPlace *p) {
  struct mwSession *session;
  struct mwChannelSet *cs;
  struct mwChannel *chan;
  struct mwPutBuffer *b;
  int ret;

  g_return_val_if_fail(p != NULL, -1);
  g_return_val_if_fail(p->service != NULL, -1);

  session = mwService_getSession(MW_SERVICE(p->service));
  g_return_val_if_fail(session != NULL, -1);

  cs = mwSession_getChannels(session);
  g_return_val_if_fail(cs != NULL, -1);

  chan = mwChannel_newOutgoing(cs);
  mwChannel_setService(chan, MW_SERVICE(p->service));
  mwChannel_setProtoType(chan, PROTOCOL_TYPE);
  mwChannel_setProtoVer(chan, PROTOCOL_VER);

  mwChannel_populateSupportedCipherInstances(chan);

  b = mwPutBuffer_new();
  mwString_put(b, mwPlace_getName(p));
  mwString_put(b, mwPlace_getTitle(p));
  guint32_put(b, 0x00); /* ? */

  mwPutBuffer_finalize(mwChannel_getAddtlCreate(chan), b);

  ret = mwChannel_create(chan);
  if(ret) {
    place_state(p, mwPlace_ERROR);
  } else {
    place_state(p, mwPlace_PENDING);
    p->channel = chan;
    mwChannel_setServiceData(chan, p, NULL);
  }

  return ret;
}


int mwPlace_destroy(struct mwPlace *p, guint32 code) {
  int ret = 0;

  place_state(p, mwPlace_CLOSING);

  if(p->channel) {
    ret = mwChannel_destroy(p->channel, code, NULL);
    p->channel = NULL;
  }

  place_free(p);

  return ret;
}


GList *mwPlace_getMembers(struct mwPlace *place) {
  GList *l, *ll;

  g_return_val_if_fail(place != NULL, NULL);
  g_return_val_if_fail(place->members != NULL, NULL);

  ll = map_collect_values(place->members);
  for(l = ll; l; l = l->next) {
    struct place_member *pm = l->data;
    l->data = &pm->idb;
    /// Miranda NG adaptation start - MSVC
    //g_info("collected member %u: %s, %s", pm->place_id,
    //   NSTR(pm->idb.user), NSTR(pm->idb.community));
    g_message("collected member %u: %s, %s", pm->place_id, NSTR(pm->idb.user), NSTR(pm->idb.community));
    /// Miranda NG adaptation end
  }

  return ll;
}


int mwPlace_sendText(struct mwPlace *place, const char *msg) {
  struct mwOpaque o = {0,0};
  struct mwPutBuffer *b;
  int ret;

  b = mwPutBuffer_new();
  guint32_put(b, 0x01);  /* probably a message type */
  mwString_put(b, msg);
  mwPutBuffer_finalize(&o, b);

  b = mwPutBuffer_new();
  guint32_put(b, place->section);
  mwOpaque_put(b, &o);
  mwOpaque_clear(&o);
  mwPutBuffer_finalize(&o, b);

  ret = mwChannel_send(place->channel, msg_out_MESSAGE, &o);
  mwOpaque_clear(&o);
  return ret;
}


int mwPlace_legacyInvite(struct mwPlace *place,
			 struct mwIdBlock *idb,
			 const char *message) {

  struct mwOpaque o = {0,0};
  struct mwPutBuffer *b;
  int ret;

  b = mwPutBuffer_new();
  mwIdBlock_put(b, idb);
  mwString_put(b, idb->user);
  mwString_put(b, idb->user);
  mwString_put(b, message);
  gboolean_put(b, FALSE);
  mwPutBuffer_finalize(&o, b);

  ret = mwChannel_send(place->channel, msg_out_OLD_INVITE, &o);
  mwOpaque_clear(&o);
  return ret;
}


int mwPlace_setAttribute(struct mwPlace *place, guint32 attrib,
			 struct mwOpaque *data) {

  struct mwOpaque o = {0,0};
  struct mwPutBuffer *b;
  int ret;

  b = mwPutBuffer_new();
  guint32_put(b, place->our_id);
  guint32_put(b, 0x00);
  guint32_put(b, attrib);
  mwOpaque_put(b, data);
  
  ret = mwChannel_send(place->channel, msg_out_SET_ATTR, &o);
  mwOpaque_clear(&o);
  return ret;
}


int mwPlace_unsetAttribute(struct mwPlace *place, guint32 attrib) {
  struct mwOpaque o = {0,0};
  struct mwPutBuffer *b;
  int ret;

  b = mwPutBuffer_new();
  guint32_put(b, place->our_id);
  guint32_put(b, attrib);
  
  ret = mwChannel_send(place->channel, msg_out_SET_ATTR, &o);
  mwOpaque_clear(&o);
  return ret;
}


void mwPlace_setClientData(struct mwPlace *place,
			   gpointer data, GDestroyNotify clear) {

  g_return_if_fail(place != NULL);
  mw_datum_set(&place->client_data, data, clear);
}


gpointer mwPlace_getClientData(struct mwPlace *place) {
  g_return_val_if_fail(place != NULL, NULL);
  return mw_datum_get(&place->client_data);
}


void mwPlace_removeClientData(struct mwPlace *place) {
  g_return_if_fail(place != NULL);
  mw_datum_clear(&place->client_data);
}
